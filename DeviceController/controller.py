#!/usr/bin/env python2
# -*- mode: python; indent-tabs-mode: nil; tab-width: 4 -*-
###############################################################################
# @file           controller.py
#
# @author         Michael Catanzaro <michael.catanzaro@mst.edu>
#
# @project        FREEDM Fake Device Controller
#
# @description    A fake ARM controller for FREEDM devices.
#
# These source code files were created at Missouri University of Science and
# Technology, and are intended for use in teaching or research. They may be
# freely copied, modified, and redistributed as long as modified versions are
# clearly marked as such and this notice is not removed. Neither the authors
# nor Missouri S&T make any warranty, express or implied, nor assume any legal
# responsibility for the accuracy, completeness, or usefulness of these files
# or any information distributed with these files.
#
# Suggested modifications or questions about these files can be directed to
# Dr. Bruce McMillin, Department of Computer Science, Missouri University of
# Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
###############################################################################

import ConfigParser
import argparse
import datetime
import math
import os
import socket
import string
import sys
import threading
import time


############### WHERE DO THESE GO? ################

# can't be global like this anymore
config = {}

COPYRIGHT_YEAR = '2013'
# wow this is brittle ...
VERSION_FILE = '../Broker/src/version.h'


################## DEVICE CLASS ##################

class NoSuchSignalError(Exception):
    """
    An exception to be raised when a signal of a device does not exist.
    """
    pass


class NoSuchDeviceError(Exception):
    """
    An exception to be raised when a device of a given name does not exist.
    """
    pass    

    
def get_device(name, devices):
    """
    Loops through a set of devices and finds the device with the requested name

    @param name the name of the device to find
    @param devices the set of devices to look in

    @ErrorHandling throws NoSuchDeviceError if the device does not exist

    @return the desired device
    """
    for device in devices:
        if device.get_name() == name:
            return device
    else:
        raise NoSuchDeviceError('Device ' + name + ' not found in set')

   
class Device(object):
    """
    Represents a physical device.
    """
    def __init__(self, name, type_, signals):
        """
        Creates a new device!

        @param name string containing the name of the device
        @param type_ string containing the type of the device
        @param signals dictionary of string signal names -> float values
        """
        # string - name of the device
        self._name = name
        # string - type of the device
        self._type = type_
        # strings (signal names) -> floats (signal values)
        self._signals = signals
        # dict of strings -> ints indicating num locks on the signal
        # (recently-modified signals accept no commands until locks fall to 0)
        self._protected_signals = {}
        # mutex to provide thread-safety for the protected signals set
        self._protected_signals_lock = threading.Lock()

        
    def get_name(self):
        """
        @return string the name of this device
        """
        return self._name

        
    def get_type(self):
        """
        @return string the type of this device
        """
        return self._type

        
    def get_signals(self):
        """
        @return a set of strings naming the signals of this device
        """
        return self._signals.keys()
    

    def get_signal(self, signal):
        """
        @param signal string containing the name of the desired signal

        @ErrorHandling could raise NoSuchSignalError

        @return float value of the signal
        """
        if signal in self._signals:
            return self._signals[signal]
        else:
            raise NoSuchSignalError('Invalid signal: ' + signal)


    def command_signal(self, signal, value):
        """
        Set the signal to the specified value, unless it has recently changed
        independently of a command from DGI or is NaN. Call this when the state
        of a signal needs to change due to a command by DGI.

        @param signal string name of the signal to command
        @param value float the new value of this signal
        """
        if signal in self._protected_signals.keys():
            print 'Not updating ({0}, {1}): signal has {2} lock(s)'.format(
                        self._name, signal, self._protected_signals[signal])
        elif math.isnan(value):
            print 'Not updating ({0}, {1}): received NaN command'.format(
                        self._name, signal)            
        else:
            self._signals[signal] = value


    def set_signal(self, signal, value):
        """
        Set the signal to the specified value and prevent DGI from commanding
        it for a short while. Call this when the state of a signal changes
        independently of a command from DGI.

        @param signal string name of the signal to change
        @param value float the new value of this signal
        """
        assert not math.isnan(value)
        self._signals[signal] = value
        self._protected_signals_lock.acquire()
        if signal in self._protected_signals.values():
            self._protected_signals[signal] += 1
        else:
            self._protected_signals[signal] = 1
        self._protected_signals_lock.release()
        t = threading.Timer(
                config['protected-state-duration'],
                self._unlock_signal,
                args=(signal))
        t.start()


    def _unlock_signal(self, signal):
        """
        Removes a lock from a signal

        @param signal string name of the signal to unlock
        """
        self._protected_signals_lock.acquire()
        self._protected_signals[signal] -= 1
        if self._protected_signals[signal] == 0:
            del self._protected_signals[signal]
        self._protected_signals_lock.release()


################## PNP PROTOCOL LOGIC ##################

class ConnectionLostError(Exception):
    """
    An exception to be raised when the DGI disappears.
    """
    pass


def send_all(socket, msg):
    """
    Ensures the entirety of msg is sent over the socket.

    @param socket the connected stream socket to send to
    @param msg the message to be sent

    @ErrorHandling Could raise socket.error or socket.timeout if the data
                   cannot be sent in time. Raises ConnectionLostError if the
                   connection to the DGI is closed.
    """
    assert len(msg) != 0
    sent_bytes = socket.send(msg)
    while (sent_bytes != len(msg)):
        assert sent_bytes < len(msg)
        msg = msg[sent_bytes:]
        assert len(msg) != 0
        sent_bytes = socket.send(msg)
        if sent_bytes == 0:
            raise ConnectionLostError('Connection to DGI unexpectedly lost')


def recv_all(socket):
    """
    Receives all the data that has been sent from DGI until \r\n\r\n is
    reached.
    
    @param socket the connected stream socket to receive from

    @ErrorHandling Will raise a RuntimeError if there is data after \r\n\r\n,
                   since that delimits the end of a message and the DGI is not
                   allowed to send multiple messages in a row. Also raises
                   runtime errors if there is no \r\n\r\n at all in the packet,
                   or if it doesn't occur at the very end of the packet. Will
                   raise ConnectionLostError if the DGI times out, or
                   RuntimeError if the DGI sends a malformed packet.

    @return the data that has been read
    """
    msg = socket.recv(1024)
    while len(msg)%1024 == 0 and len(msg) != 0:
        msg += socket.recv(1024)
    if len(msg) == 0:
        raise ConnectionLostError('Connection to DGI unexpectedly lost')
    if msg.find('\r\n\r\n') != len(msg)-4:
        raise RuntimeError('Malformed message from DGI:\n' + msg)
    return msg


def handle_bad_request(msg):
    """
    Terminate immediately if the DGI says we sent a bad request.

    @param msg string the message sent by DGI
    """
    msg = msg.replace('BadRequest', '', 1)
    raise RuntimeError('Sent bad request to DGI: ' + msg)


def handle_dgi_error(msg):
    """
    If the DGI reports an on its end, print the error, wait for the hello
    timeout, then try to reconnect.

    @param msg string the message sent by DGI
    """
    msg = msg.replace('Error', '', 1)
    print >> sys.stderr, 'Received an error from DGI: ' + msg
    time.sleep(config['hello-timeout'])
    return connect(device_types)


def send_states(adaptersock, devices):
    """
    Sends updated device states to the DGI.

    @param adaptersock the connected stream socket to send the states on
    @param devices set of devices attached to this controller

    @ErrorHandling caller must check for socket.timeout
    """
    msg = 'DeviceStates\r\n'
    for device in devices:
        for signal in device.get_signals():
            msg += '{0} {1} {2}\r\n'.format(device.get_name(),
                                            signal, 
                                            device.get_signal(signal))
    msg += '\r\n'
    print 'Sending states to DGI:\n' + msg.strip()
    send_all(adaptersock, msg)
    print 'Sent states to DGI\n'


def receive_commands(adaptersock, devices):
    """
    Receives new commands from the DGI and then implements them by modifying
    the devices map. We're basically the best controller ever since we
    generally satify the DGI instantanously. But sometimes we ignore the DGI:

    * The DGI will send a NaN command initially (when it doesn't yet know our
      state) to indicate we should ignore the command.

    * DGI might be operating based on old state info if the state suddenly
      changes sharply based on a command in the dsp simulation script.
      In this case we shall ignore commands on this state for some configurable
      amount of time (protected-state-duration).

    @param adaptersock the connected stream socket to receive commands from
    @param devices set of devices attached to this controller

    @ErrorHandling caller must check for socket.timeout
    """
    print 'Awaiting commands from DGI...'
    msg = recv_all(adaptersock)
    print 'Received commands from DGI:\n' + msg.strip()
    if msg.find('DeviceCommands\r\n') != 0:
        raise RuntimeError('Malformed command packet:\n' + msg)
    for line in msg.split('\r\n'):
        if line.find('DeviceCommands') == 0:
            continue
        command = line.split()
        if len(command) == 0:
            continue
        if len(command) != 3:
            raise RuntimeError('Malformed command in packet:\n' + line)        
        try:
            name = command[0]
            signal = command[1]
            value = float(command[2])
            get_device(name, devices).command_signal(signal, value)
        except NoSuchSignalError as e:
            raise RuntimeError('Packet contains invalid signal: '
                    + str(e))
    print 'Device states have been updated\n'


def work(adaptersock, devices):
    """
    Send states, receive commands, then sleep for a bit.
    
    @param adaptersock the connected stream socket to use
    @param devices set of devices attached to this controller

    @ErrorHandling caller must check for socket.timeout, socket.error, and
                   ConnectionLostError
    """
    global config
    send_states(adaptersock, devices)
    receive_commands(adaptersock, devices)
    time.sleep(config['state-timeout'])


def connect(devices):
    """
    Sends a Hello message to DGI, and receives its Start message in response.
    If there is a failure, tries again until it works.

    @param devices set of devices attached to this controller
    
    @return a stream socket connected to a DGI adapter
    """
    global config
    
    hello_packet = 'Hello\r\n'
    hello_packet += config['name'] + '\r\n'
    for device in devices:
        hello_packet += '{0} {1}\r\n'.format(device.get_type(),
                                             device.get_name())
    hello_packet += '\r\n'
    
    msg = ''
    while True:
        factorysock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        factorysock.settimeout(config['dgi-timeout'])
        try:
            print 'Attempting to connect to AdapterFactory...'
            factorysock.connect((config['host'], config['port']))
        except (socket.error, socket.herror, socket.gaierror, socket.timeout) \
                as e:
            print >> sys.stderr, \
                'Fail connecting to AdapterFactory: {0}'.format(str(e))
            time.sleep(config['hello-timeout'])
            print >> sys.stderr, 'Attempting to reconnect...'
            continue
        else:
            print 'Connected to AdapterFactory'

        try:
            send_all(factorysock, hello_packet)
            print '\nSent Hello message:\n' + hello_packet.strip()
            msg = recv_all(factorysock)
        except (socket.error, socket.timeout, ConnectionLostError) as e:
            print >> sys.stderr, \
                'AdapterFactory communication failure: {0}'.format(str(e))
            factorysock.close()
            time.sleep(config['hello-timeout'])
            print >> sys.stderr, 'Attempting to reconnect...'
        else:
            break

    if msg.find('Error') == 0:
        return handle_dgi_error(msg)
    elif msg.find('BadRequest') == 0:
        handle_bad_request(msg)
    else:
        msg = msg.split()
        if len(msg) != 1 or msg[0] != 'Start':
            raise RuntimeError('DGI sent malformed Start:\n' + ' '.join(msg))
        else:
            print '\nReceived Start message:\n' + ' '.join(msg)

    # Note at this point, we're communicating with an adapter, not the factory
    return factorysock


def polite_quit(adaptersock, devices):
    """
    Sends a quit request to DGI and returns once the request has been approved.
    If the request is not initially approved, continues to send device states
    and receive device commands until the request is approved or the DGI times
    out, at which point control returns to the script.

    @param adaptersock the connected stream socket which wants to quit
                       THIS SOCKET WILL BE CLOSED!!
    @param devices set of devices attached to this controller
    """
    while True:
        try:
            print 'Sending PoliteDisconnect request to DGI'
            send_all(adaptersock, 'PoliteDisconnect\r\n\r\n')
            msg = recv_all(adaptersock)
        except (socket.error, socket.timeout, ConnectionLostError) as e:
            print >> sys.stderr, \
                'DGI communication error: {0}'.format(str(e))
            print >> sys.stderr, 'Closing connection impolitely'
            adaptersock.close()
            return

        if msg.find('BadRequest') == 0:
            handle_bad_request(msg)

        msg = msg.split()
        if len(msg) != 2 or msg[0] != 'PoliteDisconnect' or \
                (msg[1] != 'Accepted' and msg[1] != 'Rejected'):
            raise RuntimeError('Got bad disconnect response:\n' \
                    + ' '.join(msg))

        if msg[1] == 'Accepted':
            print 'PoliteDisconnect accepted by DGI'
            adaptersock.close()
            return
        else:
            assert msg[1] == 'Rejected'
            print 'PoliteDisconnect rejected, performing another round of work'
            try:
                work(adaptersock,devices)
                # Loop again
            except (socket.error, socket.timeout, ConnectionLostError) as e:
                print >> sys.stderr, \
                    'DGI communication error: {0}'.format(str(e))
                print >> sys.stderr, 'Closing connection impolitely'
                adaptersock.close()
                return


################## FAKE DSP LOGIC ##################

def initialize_config():
    """
    Called at the start of main to initialize the global configuration using
    ConfigParser and argparse.
    """
    global config

    desc = \
'''A fake FREEDM device controller that interfaces with the DGI via the FREEDM
plug and play protocol and pretends to immediately implement DGI commands.'''

    epi = 'Batteries not included.'

    parser = argparse.ArgumentParser(description=desc, epilog=epi)
    parser.add_argument('-c', '--config', default='config/controller.cfg',
                        help='file to use for additional configuration')
    parser.add_argument('-g', '--host',
                        help='hostname of the DGI to connect to')
    parser.add_argument('-n', '--name',
                        help='unique identifier of this controller')
    parser.add_argument('-p', '--port', type=int,
                        help="the DGI AdapterFactory's listening port")
    parser.add_argument('-V', '--version', action='store_true',
                        help='print version info and exit')
    parser.add_argument('-s', '--script', help='the DSP script file to run')
    args = parser.parse_args()

    if args.version:
        version = '(Unknown Version)'
        try:
            with open(VERSION_FILE, 'r') as version_file:
                version_line = version_file.readlines()[2]
                version_start = version_line.index('"')
                version_end = version_line.rindex('"')
                version = version_line[version_start+1:version_end]
        except:
            pass
        print 'FREEDM Fake Device Controller Revision', version
        print 'Copyright (C)', COPYRIGHT_YEAR, 'NSF FREEDM Systems Center'
        sys.exit(0)

    if args.name:
        config['name'] = args.name
    if args.script:
        config['script'] = args.script
    if args.host:
        config['host'] = args.host
    if args.port:
        config['port'] = args.port

    if args.config:
        config_filename = args.config

    # read connection settings from the config file
    with open(config_filename, 'r') as config_file:
        cp = ConfigParser.SafeConfigParser()
        cp.readfp(config_file)

    if args.name == None:
        config['name'] = cp.get('controller', 'name')
    if args.script == None:
        config['script'] = cp.get('controller', 'script')
    if args.host == None:
        config['host'] = cp.get('connection', 'host')
    if args.port == None:
        config['port'] = int(cp.get('connection', 'port'))
    config['state-timeout'] = float(cp.get('timings', 'state-timeout'))/1000.0
    config['hello-timeout'] = float(cp.get('timings', 'hello-timeout'))/1000.0
    config['dgi-timeout'] = float(cp.get('timings', 'dgi-timeout'))/1000.0
    config['custom-timeout'] = float(cp.get(
            'timings', 'custom-timeout'))/1000.0
    config['protected-state-duration'] = float(cp.get(
            'timings', 'protected-state-duration'))/1000.0
    config['adapter-connection-retries'] = \
            int(cp.get('misc', 'adapter-connection-retries'))


def enable_device(devices, command):
    """
    Add a new device to the list of devices based on a command from a fake
    DSP script.

    @param devices set of devices already attached to the controller
    @param command string from the DSP script
    """
    command = command.split()
    assert command[0] == 'enable'
    assert (len(command)-3)%2 == 0 and len(command) >= 5
    type_ = command[1]
    name = command[2]
    try:
        get_device(name, devices)
        raise AssertionError('Device ' + name + ' is already enabled!')
    except NoSuchDeviceError:
        pass
    signals = {}        
    for i in range(3, len(command), 2):
        signals[command[i]] = float(command[i+1])
    devices.add(Device(name, type_, signals))   


def disable_device(devices, command):
    """
    Remove a device from the list of devices based on a command from a fake
    DSP script.

    @param devices set of devices already attached to the controller
    @param command string from the DSP script
    """
    command = command.split()
    assert command[0] == 'disable'
    assert len(command) == 2
    name = command[1]
    devices.remove(get_device(name, devices))

    
if __name__ == '__main__': 
    # set of all devices attached to this controller
    devices = set()
    # are we processing the very first command in the script?
    first_hello = True
    # socket connected
    adaptersock = -1

    initialize_config()
    script = open(config['script'], 'r')
    for command in script:
        # If we ever need to break connection, we will reconnect immediately.
        # So after the first Hello, adaptersock will ALWAYS be valid.
        if not first_hello:
            assert adaptersock.fileno() > 1

        if command[0] == '#' or command.isspace(): 
            continue

        print '\nProcessing command: ' + command.strip()

        if command.find('enable') == 0 and (len(command.split())-3)%2 == 0:
            enable_device(devices, command)
            if not first_hello:
                polite_quit(adaptersock, devices)
            adaptersock = connect(devices)
            if first_hello:
                first_hello = False

        elif command.find('disable') == 0 and len(command.split()) == 2:
            if first_hello:
                raise RuntimeError("Can't disable devices before first Hello")
            disable_device(devices, command)
            polite_quit(adaptersock, devices)
            adaptersock = connect(devices)
            
        elif command.find('change') == 0 and len(command.split()) == 4:
            if first_hello:
                raise RuntimeError("Can't change a signal before first Hello")
            command = command.split()
            name = command[1]
            signal = command[2]
            value = float(command[3])
            get_device(name, devices).set_signal(signal, value)
            print 'Changed', name, signal, 'to', str(value)

        elif command.find('dieHorribly') == 0 and len(command.split()) == 1:
            print 'I have died horribly, goodbye'
            adaptersock.this_is_a_syntax_error()

        elif command.find('work') == 0 and len(command.split()) == 2:
            if first_hello:
                raise RuntimeError("Can't work before the first Hello")
            duration = command.split()[1]
            if duration == 'forever':
                i = 0
                while True:
                    try:
                        print 'Working forever, as ordered captain:', i
                        work(adaptersock, devices)
                    except (socket.error, socket.timeout, ConnectionLostError) \
                            as e:
                        print >> sys.stderr, \
                            'DGI communication error: {0}'.format(str(e))
                        print >> sys.stderr, 'Performing impolite reconnect'
                        adaptersock.close()
                        adaptersock = connect(devices)
                    i += 1
            elif int(duration) <= 0:
                raise ValueError('Nonsense to work for ' + duration + 's')
            else:
                for i in range(int(duration)):
                    try:
                        print 'Performing work {0} of {1}\n'.format(
                                i, duration)
                        work(adaptersock, devices)
                    except (socket.error, socket.timeout, ConnectionLostError) \
                            as e:
                        print >> sys.stderr, \
                            'DGI communication error: {0}'.format(str(e))
                        print >> sys.stderr, 'Performing impolite reconnect'
                        adaptersock.close()
                        adaptersock = connect(devices)

        elif command.find('sendtofactory') == 0 and len(command.split()) == 2:
            filename = command.split()[1]
            msg = ''
            response = ''
            with open(filename, 'r') as packet:
                msg = packet.read()
            print 'Going to send to adapter factory:\n' + msg.replace('\r', '')
            factorysock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            factorysock.settimeout(config['dgi-timeout'])
            try:
                factorysock.connect((config['host'], config['port']))
                send_all(factorysock, msg)
                response = recv_all(factorysock)
                factorysock.close()
            except (socket.error, socket.herror, socket.gaierror,
                    socket.timeout, ConnectionLostError) as e:
                print >> sys.stderr, \
                'sendtofactory failed: {0}'.format(str(e))
            if response.find('BadRequest') == 0:
                handle_bad_request(response)
            else:
                print 'Factory responded:\n' + response
            time.sleep(config['custom-timeout'])

        elif command.find('sendtoadapter') == 0 and len(command.split()) == 2:
            if first_hello:
                raise RuntimeError("Can't sendtoadapter before first Hello")
            filename = command.split()[1]
            msg = ''
            response = ''
            with open(filename, 'r') as packet:
                msg = packet.read()
            print 'Going to send to adapter:\n' + msg.replace('\r', '')
            try:
                send_all(adaptersock, msg)
                response = recv_all(adaptersock)
            except (socket.error, socket.timeout, ConnectionLostError) as e:
                print >> sys.stderr, \
                'sendtoadapter failed: {0}'.format(str(e))
            if response.find('BadRequest') == 0:
                handle_bad_request(response)
            else:
                print 'Adapter responded:\n' + response
            time.sleep(config['custom-timeout'])

        elif command.find('sleep') == 0 and len(command.split()) == 2:
            duration = int(command.split()[1])
            print "I'm going to sleep for {0} seconds".format(duration)
            time.sleep(duration)

        else:
            raise RuntimeError('Read invalid script command:\n' + command)

    print 'That seems to be the end of my script, disconnecting now...'
    if adaptersock != -1: # crash when doing only sendtofactory
        polite_quit(adaptersock, devices)
    script.close()
