#!/usr/bin/env python2
# -*- mode: python; indent-tabs-mode: nil; tab-width: 4 -*-
###############################################################################
# @file           translator.py
#
# @author         Michael Catanzaro <michael.catanzaro@mst.edu>
#
# @project        FREEDM Device Controller
#
# @description    Intermediary between Mingkui's device controller and DGI.
#                 Transfers states faithfully, but drops all commands as the
#                 DESDs are currently uncontrollable. Since this is an
#                 abomination, this code is not for mainline plug and play.
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
import socket
import sys
import time

import device
import protocol

config = {}

with open('config/translator.cfg', 'r') as config_file:
    cp = ConfigParser.SafeConfigParser()
    cp.readfp(config_file)

config['name'] = cp.get('controller', 'name')
config['host'] = cp.get('connection', 'host')
config['port'] = int(cp.get('connection', 'port'))
config['state-timeout'] = float(cp.get('timings', 'state-timeout'))/1000.0
config['hello-timeout'] = float(cp.get('timings', 'hello-timeout'))/1000.0
config['dgi-timeout'] = float(cp.get('timings', 'dgi-timeout'))/1000.0
config['adapter-connection-retries'] = \
        int(cp.get('misc', 'adapter-connection-retries'))

desds = set()

try:
    fifo = open('sitevisitfifo2013', 'r')
except IOError:
    print "Start Mingkui's controller first"
    sys.exit(1)

reconnect = True
adaptersock = -1

while True:
    desds_present = [None, False, False, False, False, False,
                           False, False, False, False, False]

    while True:
        states = fifo.readline().strip()

        if states == 'end':
            break

        states = states.split(',')
        states = [state.partition(':') for state in states]
        states = {state[0] : float(state[2]) for state in states}

        name = int(states['Device'])
        desds_present[name] = True
        name = str(name)
	    del states['Device']

        if device.device_exists(name, desds):
            desd = device.get_device(name, desds)
            for setting, value in states.iteritems():
                desd.set_signal(setting, value)
        else:
            desd = device.Device(name, 'Desd', states)
            print 'Adding new device', name
            desds.add(desd)
            reconnect = True
    
    for i in range(1, 11):
        name = str(i)
        if not desds_present[i] and device.device_exists(name, desds):
            desds = device.remove_device(name, desds)

    if len(desds) > 0:
        if reconnect:
            if adaptersock != -1:
                protocol.polite_quit(adaptersock, desds, config['dgi-timeout'])
            adaptersock = protocol.connect(desds, config)
            reconnect = False
        protocol.work(adaptersock, desds, config['state-timeout'])
    elif adaptersock != -1:
        protocol.polite_quit(adaptersock, desds, config['dgi-timeout'])
        reconnect = True

