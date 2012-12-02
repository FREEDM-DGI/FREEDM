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

# read settings from the config file
with open('controller.cfg', 'r') as f:
    config = ConfigParser.SafeConfigParser()
    config.readfp(f)
    # the hostname and port of the DGI this controller must connect to
    dgiHostname = config.get('connection', 'dgi-hostname')
    dgiPort = config.get('connection', 'dgi-port')
    # the port on which this controller will receive from the DGI
    listenPort = config.get('connection', 'listen-port')
    # the type of each device under this controller
    devices = [y for (x, y) in config.items('devices')]

# Prepare to receive a response from DGI
listenSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listenSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
listenSocket.bind(('', int(listenPort)))
listenSocket.listen(1)

# ARM connects to dgi-hostname:dgi-port as soon as it parses the configuration
initiationSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
initiationSocket.connect((dgiHostname, int(dgiPort)))

# ARM sends a packet with the format: server-port deviceA deviceB deviceC...
initiationSocket.send(listenPort + ' ' + ' '.join(devices) + '\r\n')
initiationSocket.close()

# ARM receives a packet with the format: DGI_Adapter_Port
client, address = listenSocket.accept()
adapterPort = client.recv(8)
client.close()
listenSocket.close() # for now, all this socket does is get that port number

# ARM connects to dgi-hostname:DGI_Adapter_Port every second (but sends no data)
while True:
    adapterSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    adapterSocket.connect((dgiHostname, int(adapterPort)))
    time.sleep(0.5)
    adapterSocket.close()
    time.sleep(0.5)

