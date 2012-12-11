#!/usr/bin/env python2
##############################################################################
# @file dgi.py
#
# @author Michael Catanzaro <michael.catanzaro@mst.edu>
#
# @project FREEDM Fake Device Controller
#
# @description A fake DGI for testing the fake device controller.
#
#                       Don't think about it too hard!
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

import socket

LISTEN_PORT = 3010
STATE_PORT = 3011
HEARTBEAT_PORT = 3012

acceptorSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
acceptorSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
acceptorSocket.bind(('', LISTEN_PORT))
acceptorSocket.listen(1)

while True:
    # wait for the controller to send a Hello
    clientSocket, address = acceptorSocket.accept()
    helloMsg = clientSocket.recv(2048)
    print 'Received Hello message:\n' + helloMsg
    clientSocket.close()
    
    if helloMsg.find(SessionPort) is not 0 or len(helloMsg.split())%2 is not 0:
        raise ValueError('Received malformed Hello:\n' + helloMsg)
    helloMsg = helloMsg.split()
    sessionPort = helloMsg[1]
    print 'Received SessionPort=' + sessionPort

    devices = {} # device name -> type
    helloMsg = helloMsg[2:]
    while len(helloMsg) is not 0:
        if helloMsg[0] in devices:
            raise ValueError('Device ' + name + ' already exists in map')
        devices[helloMsg[0]] = helloMsg[1]
        helloMsg = helloMsg[2:]
    print 'Constructed device map:\n' + devices

    # Set up the state connection
    stateSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    stateSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    stateSocket.bind(('', STATE_PORT))
    stateSocket.listen(1)

    # Now send a Start message back
    startMsg = 'StatePort: ' + str(STATE_PORT) + '\r\n'
    startMsg += 'HeartbeatPort: ' + str(HEARTBEAT_PORT) + '\r\n\r\n'

    sessionSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sessionSocket.connect((address, sessionPort))
    sessionSocket.send(startMsg)

    # TODO - we need to handle heartbeats.
    # TODO - just accept and then close connections on the hbport.

    while True:
        # what does the controller say??
        msg = stateSocket.recv(2048)
        
        # TODO - if the controller dies horribly, you'll get a Hello here
        # TODO - handle dieHorribly !!!
        
        # Or you might get a disconnect request.
        # Which we will entertain in a completely fair manner.
        if msg.find('PoliteDisconnect') is 0:
            response = 'PoliteDisconnect: '
        if random.randint(0, 1) is 0:
            response += 'Rejected\r\n\r\n'
            sessionSocket.send(response)        
        else:
            response += 'Accepted\r\n\r\n'
            sessionSocket.send(response)
            break
            
