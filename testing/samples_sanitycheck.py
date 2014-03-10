#!/usr/bin/env python2
# -*- mode: python; indent-tabs-mode: nil; tab-width: 4 -*-
###############################################################################
# @file           samples_sanitycheck.py
#
# @author         Michael Catanzaro <michael.catanzaro@mst.edu>
#
# @project        FREEDM DGI
#
# @description    Make sure the sample configs don't crash DGI.
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

import os
import subprocess
import sys
import time

# TODO: Too many relative paths in here, won't work unless the executables are
#       in exactly the right place.  Unfortunately not simple to fix....

SAMPLES_PATH = '../config/samples/'
SUCCESS_AFTER_SECONDS = 2

# List of all general configuration files to test
config_files = ['freedm.cfg']

# List of all adapter specifications to test, and corresponding simserv configs
adapter_files = {'adapter.xml':'rtds.xml'}

# List of all timings configuration files to test
timings_files = ['timings-p4-3.cfg', 'timings-mamba-5.cfg', \
                 'timings-ts7800-3.cfg', 'timings-ts7800-6.cfg']

# List of all logger configuration files to test
logger_files = ['logger.cfg']

# List of samples files to not test
ignored_files = ['network.xml']

# Make sure the input lists are reasonable

if len(config_files) == 0 or len(adapter_files) == 0 \
        or len(timings_files) == 0 or len(logger_files) == 0:
    raise RuntimeError('At least one of each configuration must be tested')

for sample in os.listdir(SAMPLES_PATH):
    if sample not in config_files and sample not in adapter_files and \
            sample not in timings_files and sample not in logger_files \
            and sample not in ignored_files:
        raise RuntimeError(sample + ' is not listed to be tested!')

# Now let's run the DGI once for each possible combination of config files.
# If the DGI runs for SUCCESS_AFTER_SECONDS or longer, we claim success.

passes = 0
fails = 0

for general_config in config_files:
    for adapter_config in adapter_files:
        for timings_config in timings_files:
            for logger_config in logger_files:
                simserv = subprocess.Popen(['../../PSCAD-Interface/driver',
                        '--xml', '../../PSCAD-Interface/config/samples/' +
                        adapter_files[adapter_config], '--config',
                        '../../PSCAD-Interface/config/samples/simserv.cfg',
                        '--logger',
                        '../../PSCAD-Interface/config/samples/logger.cfg',
                        '--verbose', '0'])
                dgi = subprocess.Popen(['../PosixBroker', '--config',
                        SAMPLES_PATH + general_config, '--adapter-config',
                        SAMPLES_PATH + adapter_config, '--timings-config',
                        SAMPLES_PATH + timings_config, '--logger-config',
                        SAMPLES_PATH + logger_config, '--verbose', '0'])
                time.sleep(SUCCESS_AFTER_SECONDS)
                if dgi.poll() != None:
                    fails += 1
                    print 'FAILED: ' + general_config + ' ' + adapter_config + \
                            ' ' + timings_config + ' ' + logger_config
                else:
                    dgi.terminate()
                    passes += 1
                    print 'PASSED: ' + general_config + ' ' + adapter_config + \
                            ' ' + timings_config + ' ' + logger_config
                simserv.terminate()

print 'Passed: %d\nFailed: %d' % (passes, fails)
