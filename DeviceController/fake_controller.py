#!/usr/bin/env python2
# -*- mode: python; indent-tabs-mode: nil; tab-width: 4 -*-
###############################################################################
# @file           fake_controller.py
#
# @author         Michael Catanzaro <michael.catanzaro@mst.edu>
#
# @project        FREEDM Device Controller
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
import socket
import sys
import time

from device import *
from protocol import *

COPYRIGHT_YEAR = '2013'
VERSION_FILE = '../Broker/src/version.h'


def initialize_config():
    """
    Called at the start of main. Returns a dictionary of settings constructed
    by using argparse and then ConfigParser.
    """
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

    config = {}

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
        config['script'] = cp.get('fake-controller', 'script')
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

    return config


def enable_device(devices, command, protected_state_duration):
    """
    Add a new device to the list of devices based on a command from a fake
    DSP script.

    @param devices set of devices already attached to the controller
    @param command string from the DSP script
    @param protected_state_duration the new device will protect its states
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
    devices.add(Device(name, type_, signals, protected_state_duration))   


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


# set of all devices attached to this controller
devices = set()
# are we processing the very first command in the script?
first_hello = True
# socket connected
adaptersock = -1

config = initialize_config()
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
        enable_device(devices, command, config['protected-state-duration'])
        if not first_hello:
            polite_quit(adaptersock, devices, config['state-timeout'])
        adaptersock = connect(devices, config)
        if first_hello:
            first_hello = False

    elif command.find('disable') == 0 and len(command.split()) == 2:
        if first_hello:
            raise RuntimeError("Can't disable devices before first Hello")
        disable_device(devices, command)
        polite_quit(adaptersock, devices, config['state-timeout'])
        adaptersock = connect(devices, config)
        
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
                    work(adaptersock, devices, config['state-timeout'])
                except (socket.error, socket.timeout, ConnectionLostError) \
                        as e:
                    print >> sys.stderr, \
                        'DGI communication error: {0}'.format(str(e))
                    print >> sys.stderr, 'Performing impolite reconnect'
                    adaptersock.close()
                    adaptersock = connect(devices, config)
                i += 1
        elif int(duration) <= 0:
            raise ValueError('Nonsense to work for ' + duration + 's')
        else:
            for i in range(int(duration)):
                try:
                    print 'Performing work {0} of {1}\n'.format(
                            i, duration)
                    work(adaptersock, devices, config['state-timeout'])
                except (socket.error, socket.timeout, ConnectionLostError) \
                        as e:
                    print >> sys.stderr, \
                        'DGI communication error: {0}'.format(str(e))
                    print >> sys.stderr, 'Performing impolite reconnect'
                    adaptersock.close()
                    adaptersock = connect(devices, config)

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
if adaptersock != -1:
    polite_quit(adaptersock, devices, config['state-timeout'])
script.close()
