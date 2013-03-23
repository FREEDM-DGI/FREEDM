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

config = {}

COPYRIGHT_YEAR = '2013'
ERROR_FILE = 'ERRORS'
VERSION_FILE = '../Broker/src/version.h'


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
    parser.add_argument('-c', '--config', default='controller.cfg',
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

    config_filename = 'controller.cfg'
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


def send_all(socket, msg):
    """
    Ensures the entirety of msg is sent over the socket.

    @param socket the connected stream socket to send to
    @param msg the message to be sent

    @ErrorHandling Could raise socket.error or socket.timeout if the data
                   cannot be sent in time.
    """
    assert len(msg) != 0
    sent_bytes = socket.send(msg)
    while (sent_bytes != len(msg)):
        assert sent_bytes < len(msg)
        msg = msg[sent_bytes:]
        assert len(msg) != 0
        sent_bytes = socket.send(msg)
        if sent_bytes == 0:
            raise socket.error('Connection to DGI unexpectedly closed')


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
                   raise socket.timeout if the DGI times out, or socket.error
                   if the connection is closed.

    @return the data that has been read
    """
    msg = socket.recv(1024)
    while len(msg)%1024 == 0:
        msg += socket.recv(1024)
    if len(msg) == 0:
        raise socket.error('Connection to DGI unexpectedly closed')
    if msg.find('\r\n\r\n') != len(msg)-4:
        raise RuntimeError('Malformed message from DGI:\n' + msg)
    return msg


def handle_bad_request(msg):
    """
    If the DGI says we sent a bad request, log the error and try to reconnect.
    You probably need to return immediately after calling this.

    @param msg string the message sent by DGI
    """
    msg = msg.replace('BadRequest', '', 1)
    errormsg = 'Sent bad request to DGI: ' + msg
    print >> sys.stderr, errormsg
    with open(ERROR_FILE, 'a') as errorfile:
        errorfile.write(str(datetime.datetime.now()) + '\n')
        errorfile.write(errormsg)


def enable_device(device_types, device_signals, command):
    """
    Add a new device to the list of devices. Takes a map that represents the
    previous devices in the system and a command from the DSP script, and adds
    the new device and its signals to the map.

    @param device_types dict of names -> types
    @param device_signals dict of device (name, signal) -> float
    @param command string from the dsp-script
    """
    command = command.split()
    assert command[0] == 'enable'
    assert (len(command)-3)%2 == 0 and len(command) >= 5
    name = command[2]
    if name in device_types:
        raise AssertionError('Device ' + name + ' is already enabled!')
    device_types[name] = command[1]
    for i in range (3, len(command), 2):
        device_signals[(name, command[i])] = command[i+1]


def disable_device(device_types, device_signals, command):
    """
    Remove a device from the list of devices. Operates just like enable_device
    (see above), except in reverse.

    @param device_types dict of names -> types
    @param device_signals dict of device (name, signal) -> float
    @param command string from the dsp-script
    """
    command = command.split()
    assert command[0] == 'disable'
    assert len(command) == 2

    assert command[1] in device_types
    del device_types[command[1]]

    removedSomething = False
    for name, signal in device_signals.keys():
        if name == command[1]:
            del device_signals[(name, signal)]
            removedSomething = True
    assert removedSomething


def send_states(adaptersock, device_signals):
    """
    Sends updated device states to the DGI.

    @param adaptersock the connected stream socket to send the states on
    @param device_signals dict of (name, signal) pairs to floats

    @ErrorHandling caller must check for socket.timeout
    """
    msg = 'DeviceStates\r\n'
    for (name, signal) in device_signals.keys():
        msg += name + ' ' + signal + ' ' + str(device_signals[(name, signal)])
        msg += '\r\n'
    msg += '\r\n'
    print 'Sending states to DGI:\n' + msg.strip()
    send_all(adaptersock, msg)
    print 'Sent states to DGI\n'


def receive_commands(adaptersock, device_signals, protected_signals):
    """
    Receives new commands from the DGI and then implements them by modifying
    the devices map. We're basically the best controller ever since we
    generally satify the DGI instantanously. However, we do ignore the DGI:

    * The DGI will send a NaN command initially (when it doesn't yet know our
      state) to indicate we should ignore the command.

    * DGI might be operating based on old state info if the state suddenly
      changes sharply based on a command in the dsp simulation script.
      In this case we shall ignore commands on this state for some configurable
      amount of time (protected-state-duration).

    @param adaptersock the connected stream socket to receive commands from
    @param device_signals dict of (name, signal) pairs to floats
    @param protected_signals dict of (device, signal) pairs to ints; the int
                             represents the number of locks on the signal

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
        if (command[0], command[1]) not in device_signals:
            raise RuntimeError('Unrecognized command in packet:\n' + line)
        if math.isnan(float(command[2])):
            print 'Not updating ({0}, {1}): received NaN command'.format(
                    command[0], command[1])
        elif (command[0], command[1]) in protected_signals:
            print 'Not updating ({0}, {1}): signal has {2} lock(s)'.format(
                    command[0], command[1],
                    protected_signals[(command[0], command[1])])
        else:
            device_signals[(command[0], command[1])] = command[2]
    print 'Device states have been updated\n'


def work(adaptersock, device_signals, protected_signals):
    """
    Send states, receive commands, then sleep for a bit.
    
    @param adaptersock the connected stream socket to use
    @param device_signals dict of (device, signal) pairs to floats
    @param protected_signals dict of (device, signal) pairs to ints; the int
                             represents the number of locks on the signal

    @ErrorHandling caller must check for socket.timeout
    """
    global config
    send_states(adaptersock, device_signals)
    receive_commands(adaptersock, device_signals, protected_signals)
    time.sleep(config['state-timeout'])


def reconnect(device_types):
    """
    Sends a Hello message to DGI, and receives its Start message in response.
    If there is a failure, tries again until it works.

    @param device_types dict of device names -> types
    
    @return a stream socket connected to a DGI adapter
    """
    global config
    
    devicePacket = 'Hello\r\n'
    devicePacket += config['name'] + '\r\n'
    for deviceName, deviceType in device_types.items():
        devicePacket += deviceType + ' ' + deviceName + '\r\n'
    devicePacket += '\r\n'
    
    msg = ''
    while True:
        adapterFactorySock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        adapterFactorySock.settimeout(config['dgi-timeout'])
        try:
            print 'Attempting to connect to AdapterFactory...'
            adapterFactorySock.connect((config['host'], config['port']))
        except (socket.error, socket.herror, socket.gaierror, socket.timeout) \
                as e:
            print >> sys.stderr, \
                'Fail connecting to AdapterFactory: {0}'.format(e.strerror)
            time.sleep(config['hello-timeout'])
            print >> sys.stderr, 'Attempting to reconnect...'
            continue
        else:
            print 'Connected to AdapterFactory'

        try:
            send_all(adapterFactorySock, devicePacket)
            print '\nSent Hello message:\n' + devicePacket.strip()
            msg = recv_all(adapterFactorySock)
            print '\nReceived Start message:\n' + msg.strip()
        except (socket.error, socket.timeout) as e:
            print >> sys.stderr, \
                'AdapterFactory communication failure: {0}'.format(e.strerror)
            adapterFactorySock.close()
            time.sleep(config['hello-timeout'])
            print >> sys.stderr, 'Attempting to reconnect...'
        else:
            break

    if msg.find('BadRequest') == 0:
        handle_bad_request(msg)
        time.sleep(config['hello-timeout'])
        return reconnect(device_types)
    else:
        msg = msg.split()
        if len(msg) != 3 or msg[0] != 'Start' or msg[1] != 'StatePort:':
            raise RuntimeError('DGI sent malformed Start:\n' + ' '.join(msg))

    adapterPort = int(msg[2])
    if 0 < adapterPort < 1024:
        raise ValueError('DGI wants to use DCCP well known port ' \
                + adapterPort)
    elif adapterPort < 0 or adapterPort > 65535:
        raise ValueError('DGI sent a nonsense statePort ' + config['port'])

    for i in range(0, config['adapter-connection-retries']):
        try:
            adaptersocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            adaptersocket.connect((config['host'], adapterPort))
        except (socket.error, socket.herror, socket.gaierror, socket.timeout) \
                as e:
            print >> sys.stderr, \
                'Error connecting to DGI adapter: {0}'.format(e.strerror)
            time.sleep(config['state-timeout'])
            if i == 9:
                print >> sys.stderr, \
                    'Giving up on the adapter, sending a new Hello'
                return reconnect(device_types)
            else:
                print >> sys.stderr, 'Trying again...'
        else:
            # FIXME for some reason this doesn't prevent hangs when dgi dies
            # Figuring out why would be good. But none of the other code ever
            # touches anything besides this socket. =/
            adaptersocket.settimeout(config['dgi-timeout'])
            return adaptersocket

def polite_quit(adaptersock, device_signals, protected_signals):
    """
    Sends a quit request to DGI and returns once the request has been approved.
    If the request is not initially approved, continues to send device states
    and receive device commands until the request is approved or the DGI times
    out, at which point control returns to the script.

    @param adaptersock the connected stream socket which wants to quit
                       THIS SOCKET WILL BE CLOSED!!
    @param device_signals dict of device (name, signal) pairs -> floats
    @param protected_signals dict of device (name, signal) pairs -> ints; the
                             int represents the number of locks on the signal
    """
    global config

    while True:
        try:
            print 'Sending PoliteDisconnect request to DGI'
            send_all(adaptersock, 'PoliteDisconnect\r\n\r\n')
            msg = recv_all(adaptersock)
        except (socket.error, socket.timeout) as e:
            print >> sys.stderr, \
                'DGI communication error: {0}'.format(e.strerror)
            print >> sys.stderr, 'Closing connection impolitely'
            adaptersock.close()
            return

        if msg.find('BadRequest') == 0:
            handle_bad_request(msg)
            return

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
                work(adaptersock, device_signals, config['state-timeout'],
                     protected_signals)
                # Loop again
            except (socket.error, socket.timeout) as e:
                print >> sys.stderr, \
                    'DGI communication error: {0}'.format(e.strerror)
                print >> sys.stderr, 'Closing connection impolitely'
                adaptersock.close()
                return


def unlock_signal(device_signal_pair, protected_signals,
                  protected_signals_lock):
    """
    Removes a (device, signal) pair from a set of locked signals

    @param device_signal_pair a (device, signal) on which to release a lock
    @param protected_signals dict of (device, signal) -> ints; the int
                             represents the number of locks on the signal
    @param protected_signals_lock mutex for the protected_signals dictionary
    """
    protected_signals_lock.acquire()
    protected_signals[device_signal_pair] -= 1
    if protected_signals[device_signal_pair] == 0:
        del protected_signals[device_signal_pair]
    protected_signals_lock.release()


if __name__ == '__main__': 
    # FIXME this got a bit out of hand, we need devices to be a class
    # dict of names -> types
    device_types = {}
    # dict of (name, signal) pairs -> strings (representing floats :S)
    device_signals = {}
    # dict of (name, signal) pairs -> int (number of locks on the signal)
    protected_signals = {}
    # ensure only one thread touches the set of protected signals at a time
    protected_signals_lock = threading.Lock()
    # are we processing the very first command in the script?
    first_hello = True
    # socket connected
    adaptersock = -1

    # we don't want to see errors from previous runs
    try:
        os.remove(ERROR_FILE)
    except OSError:
        # no errors, yay
        pass
    
    initialize_config() 
    script = open(config['script'], 'r')
    for command in script:
        # These dictionaries must be kept in parallel
        assert len(device_types) == len(device_signals)

        # If we ever need to break connection, we will reconnect immediately.
        # So after the first Hello, adaptersock will ALWAYS be valid.
        if not first_hello:
            assert adaptersock.fileno() > 1

        if command[0] == '#' or command.isspace(): 
            continue

        print '\nProcessing command: ' + command.strip()

        if command.find('enable') == 0 and (len(command.split())-3)%2 == 0:
            enable_device(device_types, device_signals, command)
            if not first_hello:
                polite_quit(adaptersock, device_signals, protected_signals)
            adaptersock = reconnect(device_types)
            if first_hello:
                first_hello = False

        elif command.find('disable') == 0 and len(command.split()) == 2:
            if first_hello:
                raise RuntimeError("Can't disable devices before first Hello")
            disable_device(device_types, device_signals, command)
            polite_quit(adaptersock, device_signals, protected_signals)
            adaptersock = reconnect(device_types)
            
        elif command.find('change') == 0 and len(command.split()) == 4:
            if first_hello:
                raise RuntimeError("Can't change a signal before first Hello")
            command = command.split()
            device_signals[(command[1], command[2])] = command[3]
            protected_signals_lock.acquire()
            if (command[1], command[2]) in protected_signals:
                protected_signals[(command[1], command[2])] += 1
            else:
                protected_signals[(command[1], command[2])] = 1
            protected_signals_lock.release()
            t = threading.Timer(
                    config['protected-state-duration'], unlock_signal,
                    args=((command[1], command[2]), protected_signals,
                          protected_signals_lock))
            t.start()
            print 'Changed', command[1], command[2], 'to', command[3]

        elif command.find('dieHorribly') == 0 and len(command.split()) == 2:
            if first_hello:
                raise RuntimeError("Can't die before first Hello")
            duration = command.split()[1]
            if duration == 'forever':
                print 'I have died horribly forever, goodbye'
                script.close()
                sys.exit(0)
            duration = int(duration)
            if duration < 0:
                raise ValueError("It's nonsense to die for " + duration + "s")
            time.sleep(duration)
            print 'Back to life!'
            adaptersock.close()
            adaptersock = reconnect(device_types)

        elif command.find('work') == 0 and len(command.split()) == 2:
            if first_hello:
                raise RuntimeError("Can't work before the first Hello")
            duration = command.split()[1]
            if duration == 'forever':
                while True:
                    try:
                        print 'Working forever, as ordered captain!'
                        work(adaptersock, device_signals, protected_signals)
                    except (socket.error, socket.timeout) as e:
                        print >> sys.stderr, \
                            'DGI communication error: {0}'.format(e.strerror)
                        print >> sys.stderr, 'Performing impolite reconnect'
                        adaptersock.close()
                        adaptersock = reconnect(device_types)
            elif int(duration) <= 0:
                raise ValueError('Nonsense to work for ' + duration + 's')
            else:
                for i in range(int(duration)):
                    try:
                        print 'Performing work {0} of {1}\n'.format(
                                i, duration)
                        work(adaptersock, device_signals, protected_signals)
                    except (socket.error, socket.timeout) as e:
                        print >> sys.stderr, \
                            'DGI communication error: {0}'.format(e.strerror)
                        print >> sys.stderr, 'Performing impolite reconnect'
                        adaptersock.close()
                        adaptersock = reconnect(device_types)

        elif command.find('sendtofactory') == 0 and len(command.split()) == 2:
            filename = command.split()[1]
            msg = ''
            response = ''
            with open(filename, 'r') as packet:
                msg = packet.read()
            print 'Going to send to adapter factory:\n' + msg
            factorySock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            factorySock.settimeout(config['dgi-timeout'])
            try:
                factorySock.connect((config['host'], config['port']))
                send_all(factorySock, msg)
                response = recv_all(factorySock)
                factorySock.close()
            except (socket.error, socket.herror, socket.gaierror,
                    socket.timeout) as e:
                print >> sys.stderr, \
                'sendtofactory failed: {0}'.format(e.strerror)
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
            print 'Going to send to adapter:\n' + msg
            try:
                send_all(adaptersock, msg)
                response = recv_all(adaptersock)
            except (socket.error, socket.timeout) as e:
                print >> sys.stderr, \
                'sendtoadapter failed: {0}'.format(e.strerror)
            if response.find('BadRequest') == 0:
                handle_bad_request(response)
            else:
                print 'Adapter responded:\n' + response
            time.sleep(config['custom-timeout'])

        else:
            raise RuntimeError('Read invalid script command:\n' + command)

    print 'That seems to be the end of my script, disconnecting now...'
    polite_quit(adaptersock, device_signals, protected_signals)
    # FIXME what if a timer hasn't expired yet -> should be stopped
    script.close()
