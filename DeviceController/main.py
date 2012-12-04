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
    (type, name) pairs and removes the specified device from the list.

    @param devices the list of previously-enabled devices
    @param command string of the format "disable type name"

    @return nothing, just appends to devices list
    """
    command = command.split()
    devices.remove((command[1], command[2]))


def reinitialize(dgiHostname, dgiPort, listenPort, devices):
    """
    Disconnect from the DGI at the session layer, then reconnect using a
    different set of devices.

    @param dgiHostname the hostname of the DGI to connect to
    @param dgiPort the port on the DGI to connect to to connect to
    @param listenPort the port on which to listen for a response from the DGI
    @param devices the list of device (type, name) pairs to send to DGI
    
    @return string containing the port at which to send heartbeats
    """
# Prepare to receive a response from DGI
    acceptorSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    acceptorSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    acceptorSocket.bind(('', int(listenPort)))
    acceptorSocket.listen(1)

# ARM connects to dgi-hostname:dgi-port as soon as it parses the configuration
    initiationSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    initiationSocket.connect((dgiHostname, int(dgiPort)))
    
# ARM tells DGI which devices it will have
    devicePacket = listenPort + '\r\n'
    for devicePair in devices:
        deviceType, deviceName = devicePair
        devicePacket += deviceType + ' ' + deviceName + '\r\n'
    devicePacket += '\r\n'
    initiationSocket.send(devicePacket)
    initiationSocket.close()

    print 'Sent packet:'
    print devicePacket

# ARM receives a packet with the format: DGI_Adapter_Port
    clientSocket, address = acceptorSocket.accept()
    acceptorSocket.close()
    adapterPort = clientSocket.recv(8)
    print 'Received packet: ' + adapterPort
    clientSocket.close()

    return adapterPort


def heartbeat(dgiHostname, hbPort):
    """
    A heartbeat must be sent to DGI each second. Right now this is done by
    initiating a new TCP connection. Definitely not an appropriate long-term
    solution. Calling this function takes one second.

    @param dgiHostname the host to send the heartbeat request to
    @param hbPort the port to send the heartbeat request to
    """
    print 'Sending heartbeat to ' + dgiHostname + ' ' + hbPort
    hbSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    hbSocket.connect((dgiHostname, int(hbPort)))
    time.sleep(0.5)
    hbSocket.close()
    time.sleep(0.5)


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
devices = []
with open('dsp-script.txt') as script:
    hbPort = -1
    for command in script:
        if command[0] is '#':
            continue # do not send heartbeat!
        else:
            print 'Processing command ' + command[:-1] # don't print \n
        if 'enable' in command:
            enableDevice(devices, command)
        elif 'disable' in command:
            disableDevice(devices, command)
        elif 'reconnect' in command:
            time.sleep(6) # give DGI a bit of time to kill its adapter
            hbPort = reinitialize(dgiHostname, dgiPort, listenPort, devices)
        elif 'sleep' in command:
            for i in range(int(command.split()[1])):
                if hbPort > 0:
                    print 'Sleep ' + str(i)
                    heartbeat(dgiHostname, hbPort)
        if hbPort > 0:
            heartbeat(dgiHostname, hbPort)
