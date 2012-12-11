#!/usr/bin/env python2
##############################################################################
# @file main.py
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
##############################################################################

import ConfigParser
import socket
import time

def enableDevice(devices, command):
    """
    Add a new device to the list of devices. Takes a list of device
    (type, name) pairs and adds the new device to the list.

    @param devices the list of previously-enabled devices
    @param command string of the format "enable type name"

    @return nothing, just appends to devices list
    """
    command = command.split()
    devices.append((command[1], command[2]))


def disableDevice(devices, command):
    """
    Remove a device from the list of devices. Takes a list of device
    (type, name) pairs and removes the specified device from the list. Then,
    attempts to terminate connection with DGI. This function will not return
    until the connection is cleanly closed.

    @param devices the list of previously-enabled devices
    @param command string of the format "disable type name"

    @return nothing, just appends to devices list
    """
    command = command.split()
    devices.remove((command[1], command[2]))


def reconnect(dgiHostname, dgiPort, listenPort, devices):
    """
    Sends a Hello message to DGI, and receives its Start message in response.
    This function encompasses both the Start and Init states of the transition
    diagram. (The controller will always need to send a Hello before awaiting
    a Start message.)

    @param dgiHostname the hostname of the DGI to connect to
    @param dgiPort the port on the DGI to connect to to connect to
    @param listenPort the port on which to listen for a response from the DGI
    @param devices the list of device (type, name) pairs to send to DGI
    
    @return StatePort and HeartbeatPort
    """
    # Prepare to receive a response from DGI
    acceptorSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    acceptorSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    acceptorSocket.bind(('', int(listenPort)))
    acceptorSocket.listen(0)

    # Connect to DGI on preconfigured port
    initiationSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    initiationSocket.connect((dgiHostname, int(dgiPort)))

    # Construct the Hello message
    devicePacket = 'SessionPort: ' + listenPort + '\r\n'
    for devicePair in devices:
        deviceType, deviceName = devicePair
        devicePacket += deviceType + ' ' + deviceName + '\r\n'
    devicePacket += '\r\n'
    
    # Send Hello to DGI, as many times as necessary until response received
    startMsg = ''
    while True:
        initiationSocket.send(devicePacket)
        print 'Sent Hello message:\n' + devicePacket    
        # Receive the Start message from DGI
        try:
            clientSocket, address = acceptorSocket.accept()
            acceptorSocket.close()
            startMsg = clientSocket.recv(64)
            print 'Received Start message:\n' + msg
            initiationSocket.close()
            clientSocket.close()
            break
        except socket.timeout:
            print 'Timeout awaiting Start from DGI, resending Hello'

    # Return (StatePort, HeartbeatPort) as specified by DGI
    if not 'StatePort' in startMsg or not 'HeartbeatPort' in startMsg:
        raise ValueError('Received malformed start message from DGI:\n'
                         + startMsg)
    startMsg = startMsg.split()
    print 'Received StatePort=%s, HeartbeatPort=%s' % startMsg[1], startMsg[3]
    return (startMsg[1], startMsg[3])


def politeQuit(statePort, listenPort):
    """
    Sends a quit request to DGI and returns once the request has been approved.
    If the request is not initially approved, continues to send device states
    until the request is approved.
    """
    accepted = False

    while not accepted:
        # TODO - we don't actually have device states yet, yo.
        # So just keep trying to quit until it works.
        acceptorSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        acceptorSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        acceptorSocket.bind(('', int(listenPort)))
        acceptorSocket.listen(0)
        
        print 'Sending PoliteDisconnect request to DGI'
        dgiStatePort.send('PoliteDisconnect\r\n\r\n')
        
        responseSocket, address = acceptorSocket.accept()
        acceptorSocket.close()
        msg = responseSocket.recv(32)
        
        if 'Accepted' in msg:
            accepted = True
            print 'Disconnect request accepted, yay!'
        elif 'Rejected' in msg:
            print 'Disconnect request rejected, booo'
        else:
            raise ValueError('Disconnect request response malformed:\n' + msg)


def heartbeat(dgiHostname, hbPort):
    """
    A heartbeat must be sent to DGI each second. Right now this is done by
    initiating a new TCP connection. Definitely not an appropriate long-term
    solution. Calling this function causes a second of sleep, but will probably
    take more than one second due to network delays.

    @param dgiHostname the host to send the heartbeat request to
    @param hbPort the port to send the heartbeat request to
    """
    print 'Sending heartbeat to' + dgiHostname + ' ' + hbPort
    hbSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    hbSocket.connect((dgiHostname, int(hbPort)))
    time.sleep(0.5)
    hbSocket.close()
    time.sleep(0.5)


if __name__ == '__main__':
    # read connection settings from the config file
    with open('controller.cfg', 'r') as f:
        config = ConfigParser.SafeConfigParser()
        config.readfp(f)
        
    # the hostname and port of the DGI this controller must connect to
    dgiHostname = config.get('connection', 'dgi-hostname')
    dgiPort = config.get('connection', 'dgi-port')

    # the port on which this controller will receive from the DGI
    listenPort = config.get('connection', 'listen-port')

    # run the dsp-script
    script = open('dsp-script.txt', 'r')
    devices = []
    dgiStatePort = -1
    hbPort = -1

    for command in script:
        if dgiStatePort < -1 or dgiStatePort > 65535:
            raise ValueError('StatePort ' + str(dgiStatePort) + ' not sensible')
        elif hbPort < -1 or hbPort > 65535:
            raise ValueError('HeartbeatPort ' + str(hbPort) + 'not sensible')

        if command[0] == '#' or command.isspace():
            continue # do not send heartbeat!
        else:
            print 'Processing command ' + command[:-1] # don't print \n

        if command.find('enable') == 0:
            enableDevice(devices, command)
            if dgiStatePort >= 0:
                politeQuit(dgiStatePort, listenPort)
            dgiStatePort, hbPort = reconnect(dgiHostname, dgiPort, listenPort,
                                          devices)
        elif command.find('disable') == 0:
            disableDevice(devices, command)
            if dgiStatePort >= 0:
                politeQuit(dgiStatePort, listenPort)
            dgiStatePort, hbPort = reconnect(dgiHostname, dgiPort, listenPort,
                                          devices)
        elif command.find('dieHorribly') == 0:
            duration = int(command.split()[1])
            if duration < 0:
                raise ValueError("It's nonsense to die for " + duration + "s")
            time.sleep(duration)
            dgiStatePort, hbPort = reconnect(dgiHostname, dgiPort, listenPort,
                                             devices)
        elif command.find('sleep') == 0:
            duration = int(command.split()[1])
            if duration <= 0:
                raise ValueError('Nonsense to sleep for ' + duration + 's')
            for i in range(duration):
                if hbPort >= 0:
                    print 'Sleep ' + str(i)
                    try:
                        heartbeat(dgiHostname, hbPort)
                    except socket.timeout:
                        print 'Timeout from DGI, sending a new Hello...'
                        dgiStatePort, hbPort = reconnect(dgiHostname, dgiPort,
                                                      listenPort, devices)
        else:
            raise ValueError('Found weird command ' + command)

        if hbPort >= 0:
            try:
                heartbeat(dgiHostname, hbPort)
            except socket.timeout:
                print 'Timeout from DGI, sending a new Hello...'
                dgiStatePort, hbPort = reconnect(dgiHostname, dgiPort,
                                                 listenPort, devices)

    close(script)
