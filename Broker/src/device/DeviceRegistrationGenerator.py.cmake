#!/usr/bin/env python2
##############################################################################
# @file           DeviceRegistrationGenerator.py
#
# @author         Michael Catanzaro <michael.catanzaro@mst.edu>
# @author         Thomas Roth <tprfh7@mst.edu>
#
# @project        FREEDM DGI
#
# @description    Called at compile time to generate device registration code.
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

import glob
import os

# the templates for the generation
hppTemplate = open('${CMAKE_SOURCE_DIR}/include/device/PhysicalDeviceTypes.hpp.txt', 'r')
cppTemplate = open('${CMAKE_CURRENT_SOURCE_DIR}/PhysicalDeviceTypes.cpp.txt', 'r')

if not os.path.exists('${CMAKE_BINARY_DIR}/include/device/'):
    os.makedirs('${CMAKE_BINARY_DIR}/include/device/')

# the files to be generated
hppSource = open('${CMAKE_BINARY_DIR}/include/device/PhysicalDeviceTypes.hpp', 'w')
cppSource = open('${CMAKE_CURRENT_BINARY_DIR}/PhysicalDeviceTypes.cpp', 'w')

# get our list of devices
devices = glob.glob('${CMAKE_SOURCE_DIR}/include/device/types/CDevice*.hpp')

# get just the suffix identifier
devices = [os.path.basename(device) for device in devices]
devices = [device[len('CDevice'):-len('.hpp')] for device in devices]

# generate PhysicalDeviceTypes.hpp
for line in hppTemplate: 
    if '##INCLUDES' in line:
        for device in devices:
            hppSource.write('#include "types/CDevice' + device + '.hpp"\n')
    else:
        hppSource.write(line)

# generate PhysicalDeviceTypes.cpp
for line in cppTemplate: 
    if '##REGISTRATIONS' in line:
        for device in devices:
            cppSource.write('    REGISTER_DEVICE_PROTOTYPE(' + device + ');\n')
    elif '##INSERTIONS' in line:
        for device in devices:
            cppSource.write('        if( type == "' + device + 
                '" && device_cast<CDevice' + device + '>(it->second) )\n')
            cppSource.write('        {\n')
            cppSource.write('            result.insert(it->second);\n')
            cppSource.write('        }\n')    
    else:
        cppSource.write(line)

