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

import multiprocessing
import socket

def heartbeat(hbPort, queue):
    """
    This function runs in a separate process. It accepts TCP connections
    from the device controller and them immediately closes them. Cause the
    design doc says to!

    @param hbPort the port on which to accept heartbeat connections
    @param queue one-way communication back to the calling process
    """
    acceptorSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    acceptorSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    acceptorSocket.bind(('', hbPort))
    acceptorSocket.listen(0)
    acceptorSocket.settimeout(2)
    while True:
        try:
            hbSocket, address = acceptorSocket.accept()
            print "Heart's beating, all's good"
            hbSocket.close()
        except socket.error:
            print 'Aw man, the controller timed out :-('
            queue.put('timeout')
            break


if __name__ == '__main__':
    LISTEN_PORT = 3010
    STATE_PORT = 3011
    HEARTBEAT_PORT = 3012
    
    acceptorSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    acceptorSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    acceptorSocket.bind(('', LISTEN_PORT))
    acceptorSocket.listen(0)
    
    while True:
        print 'Waiting for the controller to send a Hello'
        clientSocket, address = acceptorSocket.accept()
        helloMsg = clientSocket.recv(2048)
        print 'Received Hello message:\n' + helloMsg
        clientSocket.close()
        
        if helloMsg.find('SessionPort') != 0 or len(helloMsg.split()) % 2 != 0:
            raise ValueError('Received malformed Hello:\n' + helloMsg)
        helloMsg = helloMsg.split()
        sessionPort = int(helloMsg[1])
        print 'Received SessionPort=%d' % sessionPort

        devices = {} # device name -> type
        helloMsg = helloMsg[2:]
        while len(helloMsg) != 0:
            if helloMsg[0] in devices:
                raise ValueError('Device ' + name + ' already exists in map')
            devices[helloMsg[0]] = helloMsg[1]
            helloMsg = helloMsg[2:]
        print 'Constructed device map:\n' + str(devices)
            
        # Set up the state connection
        stateAcceptorSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        stateAcceptorSocket.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
        stateAcceptorSocket.bind(('', STATE_PORT))
        stateAcceptorSocket.listen(0)
        stateAcceptorSocket.settimeout(1)
            
        # Now send a Start message back
        startMsg = 'StatePort: ' + str(STATE_PORT) + '\r\n'
        startMsg += 'HeartbeatPort: ' + str(HEARTBEAT_PORT) + '\r\n\r\n'
            
        sessionSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        controllerHostname, dontcarePort = address
        sessionSocket.settimeout(2)
        try:
            sessionSocket.connect((controllerHostname, sessionPort))
        except socket.error:
            print "Can't connect to controller, awaiting another Hello"
            sleep(1)
        try:
            sessionSocket.send(startMsg)
            print 'Sent startMsg to the controller:\n' + startMsg
        except socket.error:
            print 'Timeout sending startMsg, awaiting another Hello'
            continue

        # Handle heartbeats in a separate process
        queue = multiprocessing.Queue()
        p = multiprocessing.Process(target=heartbeat,
                                    args=(HEARTBEAT_PORT, queue))
        p.start()
        print 'Spawned another process to receive heartbeats'
        
        timeout = False
        while True:
            if timeout == True:
                print 'Controller is gone, awaiting another Hello'
                break

            # wait for a response from the controller
            msg = ''
            while True:
                try:
                    queue.get()
                    print 'Oh no, main has detected the controller is gone'
                    timeout = True
                    break
                except Queue.Empty:
                    print 'Main DGI process has detected no timeouts'

                # now actually get the controller's response
                try:
                    # this will timeout after 1s, as it must if we're to
                    # notice a disconnect from the hb process
                    stateSocket, address = stateAcceptorSocket.accept()
                    # be leinent, it's unlikely this will fail
                    stateSocket.settimeout(4)
                    msg = stateSocket.recv(2048)
                    break
                except socket.timeout: # not expecting socket.error
                    print "'Timed out' awaiting states from controller."
                    print "This is normal operation, I'll just keep waiting."

            # Entertain a disconnect request in an entirelly fair manner.
            if msg.find('PoliteDisconnect') == 0:
                response = 'PoliteDisconnect: '
                try:
                    if random.randint(0, 1) == 0:
                        response += 'Rejected\r\n\r\n'
                        sessionSocket.send(response)
                        print 'Rejected disconnect, expecting more states'
                    else:
                        response += 'Accepted\r\n\r\n'
                        sessionSocket.send(response)
                        print 'Accepted disconnect, awaiting another Hello'
                        break
                except socket.error:
                    # do nothing, logic in below comment applies here
                    print 'Controller timeout, sad DGI'
            # If the controller died horribly, we'll get a Hello instead
            # We can just ignore this, since we'll get a hb timeout soon
            # enough. Once we do, we'll start listening for Hellos again,
            # and the controller will keep sending in the meantime.
            elif msg.find('SessionPort') == 0:
                print 'Ignoring a Hello from a previously-dead controller'
            else:
                raise ValueError('Got nonsense from controller:\n' + msg)
