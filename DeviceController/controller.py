#!/usr/bin/env python2
################################################################################
# @file controller.py
#
# @author Michael Catanzaro <michael.catanzaro@mst.edu>
#
# @project FREEDM Fake Device Controller
#
# @description A fake ARM controller for FREEDM devices.
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
################################################################################

import ConfigParser
import socket
import sys
import time


def sendAll(socket, msg):
    """
    Ensures the entirety of msg is sent over the socket.

    @param socket the connected stream socket to send to
    @param msg the message to be sent

    @ErrorHandling Could raise socket.error or socket.timeout if the data
                   cannot be sent in time.
    """
    assert len(msg) != 0
    sentBytes = socket.send(msg)
    while (sentBytes != len(msg)):
        assert sentBytes < len(msg)
        msg = msg[sentBytes:]
        assert len(msg) != 0
        sentBytes = socket.send(msg)
        if sentBytes == 0:
            raise socket.error('Connection to DGI unexpectedly closed')


def recvAll(socket):
    """
    Receives all the data that has been sent from DGI until \r\n\r\n is reached.
    
    @param socket the connected stream socket to receive from

    @ErrorHandling Will raise a RuntimeError if there is data after \r\n\r\n,
                   since that delimits the end of a message and the DGI is not
                   allowed to send multiple messages in a row. Or if there is
                   no \r\n\r\n at all in the packet, or if it doesn't occur at
                   the very end of the packet. Will raise socket.timeout if
                   the DGI times out, or socket.error if the connection is
                   closed.

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


def enableDevice(deviceTypes, deviceSignals, command):
    """
    Add a new device to the list of devices. Takes a map that represents the
    previous devices in the system and a command from the DSP script, and adds
    the new device and its signals to the map.

    @param deviceTypes dict of names -> types
    @param deviceSignals dict of device (name, signal) -> float
    @param command string from the dsp-script
    """
    command = command.split()
    assert command[0] == 'enable'
    assert (len(command)-3)%2 == 0
    name = command[2]
    deviceTypes[name] = command[1]
    for i in range (3, len(command), 2):
        deviceSignals[(name, command[i])] = float(command[i+1])


def disableDevice(deviceTypes, deviceSignals, command):
    """
    Remove a device from the list of devices. Operates just like enableDevice
    (see above), except in reverse.

    @param deviceTypes dict of names -> types
    @param deviceSignals dict of device (name, signal) -> float
    @param command string from the dsp-script
    """
    command = command.split()
    assert command[0] == 'disable'
    assert len(command) == 2

    assert deviceTypes.has_key(command[1])
    del deviceTypes[command[1]]

    removedSomething = False
    for name, signal in deviceSignals.keys():
        if name == command[1]:
            del deviceSignals[(name, signal)]
            removedSomething = True
    assert removedSomething


def sendStates(adapterSock, deviceSignals):
    """
    Sends updated device states to the DGI.

    @param adapterSock the connected stream socket to send the states on
    @param deviceSignals dict of (name, signal) pairs to floats

    @ErrorHandling caller must check for socket.timeout
    """
    msg = 'DeviceStates\r\n'
    for (name, signal) in deviceSignals.keys():
        msg += name + ' ' + signal + ' ' + deviceSignals[(name, signal)]
    msg += '\r\n'
    print 'Sending states to DGI:\n' + msg
    sendAll(adapterSock, msg)
    print 'Sent states to DGI'


def receiveCommands(adapterSock, deviceSignals):
    """
    Receives new commands from the DGI and then implements them by modifying
    the devices map.

    @param adapterSock the connected stream socket to receive commands from
    @param deviceSignals dict of (name, signal) pairs to floats

    @ErrorHandling caller must check for socket.timeout
    """
    print 'Awaiting commands from DGI...'
    msg = recvAll(adapterSock)
    print 'Received states from DGI:\n' + msg
    if msg.find('DeviceCommands\r\n') != 0:
        raise RuntimeError('Malformed command packet:\n' + msg)
    for line in msg:
        if line.find('DeviceCommands\r\n') == 0:
            continue
        command = line.split()
        if len(command) != 3:
            raise RuntimeError('Malformed command in packet:\n' + line)
        if not deviceSignals.has_key((command[0], command[1])):
            raise RuntimeError('Unrecognized command in packet:\n' + line)
        deviceSignals[(command[0], command[1])] = command[2]
    print 'Device states have been updated'


def work(adapterSock, deviceSignals, stateTimeout):
    """
    Send states, receive commands, then sleep for a bit.
    
    @param adapterSock the connected stream socket to use
    @param deviceSignals dict of (device, name) pairs to floats
    @param stateTimeout how long to sleep for

    @ErrorHandling caller must check for socket.timeout
    """
    sendStates(adapterSock, deviceSignals)
    receiveCommands(adapterSock, deviceSignals)
    time.sleep(stateTimeout)


def reconnect(deviceTypes, config):
    """
    Sends a Hello message to DGI, and receives its Start message in response.
    If there is a failure, tries again until it works.

    @param deviceTypes dict of device names -> types
    @param config all the settings from the configuration file
    
    @return a stream socket connected to a DGI adapter
    """
    dgiHostname = config.get('connection', 'dgi-hostname')
    dgiPort = int(config.get('connection', 'dgi-port'))
    helloTimeout = float(config.get('timings', 'hello-timeout'))/1000.0
    dgiTimeout = float(config.get('timings', 'dgi-timeout'))/1000.0
    adapterConnRetries = int(config.get('misc', 'adapter-connection-retries'))

    devicePacket = 'Hello\r\n'
    for deviceName, deviceType in deviceTypes.items():
        devicePacket += deviceType + ' ' + deviceName + '\r\n'
    devicePacket += '\r\n'
    
    msg = ''
    while True:
        adapterFactorySock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        adapterFactorySock.settimeout(dgiTimeout)
        try:
            print 'Attempting to connect to AdapterFactory...'
            adapterFactorySock.connect((dgiHostname, dgiPort))
        except (socket.error, socket.herror, socket.gaierror, socket.timeout) \
                as e:
            print >> sys.stderr, \
                'Fail connecting to AdapterFactory: {0}'.format(e.strerror)
            time.sleep(helloTimeout)
            print >> sys.stderr, 'Attempting to reconnect...'
            continue
        else:
            print 'Connected to AdapterFactory'

        try:
            sendAll(adapterFactorySock, devicePacket)
            print 'Sent Hello message:\n' + devicePacket
            msg = recvAll(adapterFactorySock)
            print 'Received Start message:\n' + msg
        except (socket.error, socket.timeout) as e:
            print >> sys.stderr, \
                'AdapterFactory communication failure: {0}'.format(e.strerror)
            adapterFactorySock.close()
            time.sleep(helloTimeout)
            print >> sys.stderr, 'Attempting to reconnect...'
        else:
            break

    if msg.find('BadRequest: ') == 0:
        msg.replace('BadRequest: ', '', 1)
        raise ValueError('Sent bad request to DGI: ' + msg)
    else:
        msg = msg.split()
        if len(msg) != 3 or msg[0] != 'Start' or msg[1] != 'StatePort':
            raise RuntimeError('DGI sent malformed Start:\n' + ''.join(msg))
    
    statePort = int(msg[2])
    if 0 < statePort < 1024:
        raise ValueError('DGI wants to use DCCP well known port ' + statePort)
    elif statePort < 0 or statePort > 65535:
        raise ValueError('DGI sent a nonsense statePort ' + dgiport)

    for i in range(0, adapterConnRetries):
        try:
            adapterSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            adapterSocket.connect(dgiHostname, statePort)
        except (socket.error, socket.herror, socket.gaierror, socket.timeout) \
                as e:
            print >> sys.stderr, \
                'Error connecting to DGI adapter: {0}'.format(e.strerror)
            sleep(stateTimeout)
            if i == 9:
                print >> sys.stderr, \
                    'Giving up on the adapter, sending a new Hello'
                return reconnect(deviceTypes, config)
            else:
                print >> sys.stderr, 'Trying again...'
        else:
            adapterSocket.setTimeout(dgiTimeout)
            return adapterSocket

def politeQuit(adapterSock, deviceSignals, stateTimeout):
    """
    Sends a quit request to DGI and returns once the request has been approved.
    If the request is not initially approved, continues to send device states
    and receive device commands until the request is approved or the DGI times
    out, at which point control returns to the script.

    @param adapterSock the connected stream socket which wants to quit
                       THIS SOCKET WILL BE CLOSED!!
    @param deviceSignals dict of device (name, signal) pairs -> floats
    @param stateTimeout how long to wait between working
    """
    while True:
        try:
            print 'Sending PoliteDisconnect request to DGI'
            sendAll(adapterSock, 'PoliteDisconnect\r\n\r\n')
            msg = recvAll(adapterSock)
        except (socket.error, socket.timeout) as e:
            print >> sys.stderr, \
                'DGI communication error: {0}'.format(e.strerror)
            print >> sys.stderr, 'Closing connection impolitely'
            adapterSock.close()
            return

        if msg.find('BadRequest: ') == 0:
            msg.replace('BadRequest: ', '', 1)
            raise ValueError('Sent bad request to DGI: ' + msg)
        
        msg = msg.split()
        if len(msg) != 2 or msg[0] != 'PoliteDisconnect' or \
                (msg[1] != 'Accepted' and msg[1] != 'Rejected'):
            raise RuntimeError('Got bad disconnect response:\n' + ''.join(msg))

        if msg[1] == 'Accepted':
            adapterSock.close()
            return
        else:
            assert msg[1] == 'Rejected'
            print 'Performing another round of work'
            try:
                work(adapterSock, deviceSignals, stateTimeout)
                # Loop again
            except (socket.error, socket.timeout) as e:
                print >> sys.stderr, \
                    'DGI communication error: {0}'.format(e.strerror)
                print >> sys.stderr, 'Closing connection impolitely'
                adapterSock.close()
                return


if __name__ == '__main__':
    # read connection settings from the config file
    with open('controller.cfg', 'r') as configFile:
        config = ConfigParser.SafeConfigParser()
        config.readfp(configFile)
        
    # the name of the DSP script to run
    dspScript = config.get('script', 'dsp-script')
    # how long to wait between work
    stateTimeout = float(config.get('timings', 'state-timeout'))/1000.0
    # dict of names -> types
    deviceTypes = {}
    # dict of (name, signal) pairs -> floats
    deviceSignals = {}
    # are we processing the very first command in the script?
    firstHello = True
    # socket connected
    adapterSock = -1
 
    script = open(dspScript, 'r')
    for command in script:
        # These dictionaries must be kept in parallel
        assert len(deviceTypes) == len(deviceSignals)

        # If we ever need to break connection, we will reconnect immediately.
        # So after the first Hello, adapterSock will ALWAYS be valid.
        if not firstHello:
            assert adapterSock.fileno() > 1

        if command[0] == '#' or command.isspace(): 
            continue

        print 'Processing command ' + command[:-1] # don't print \n

        if command.find('enable') == 0 and (len(command.split())-3)%2 == 0:
            enableDevice(deviceTypes, deviceSignals, command)
            if not firstHello:
                politeQuit(adapterSock, deviceSignals, stateTimeout)
            adapterSock = reconnect(deviceTypes, config)
            if firstHello:
                firstHello = False

        elif command.find('disable') == 0 and len(command.split()) == 2:
            if firstHello:
                raise RuntimeError("Can't disable devices before first Hello")
            disableDevice(deviceTypes, deviceSignals, command)
            politeQuit(adapterSock, deviceSignals, stateTimeout)
            adapterSock = reconnect(deviceTypes, config)

        elif command.find('dieHorribly') == 0 and len(command.split()) == 2:
            if firstHello:
                raise RuntimeError("Can't die before first Hello")
            duration = int(command.split()[1])
            if duration < 0:
                raise ValueError("It's nonsense to die for " + duration + "s")
            time.sleep(duration)
            adapterSock.close()
            adapterSock = reconnect(deviceTypes, config)

        elif command.find('work') == 0 and len(command.split()) == 2:
            if firstHello:
                raise RuntimeError("Can't work before the first Hello")
            duration = int(command.split()[1])
            if duration <= 0:
                raise ValueError('Nonsense to work for ' + duration + 's')
            for i in range(duration):
                try:
                    work(adapterSock, deviceSignals, stateTimeout)
                except (socket.error, socket.timeout) as e:
                    print >> sys.stderr, \
                        'DGI communication error: {0}'.format(e.strerror)
                    print >> sys.stderr, 'Performing impolite reconnect'
                    adapterSock.close()
                    adapterSock = reconnect(deviceTypes, config)

        else:
            raise RuntimeError('Read invalid script command:\n' + command)

    close(script)
