# -*- mode: python; indent-tabs-mode: nil; tab-width: 4 -*-
###############################################################################
# @file           device.py
#
# @author         Michael Catanzaro <michael.catanzaro@mst.edu>
#
# @project        FREEDM Device Controller
#
# @description    A class for storing the signals of a physical device, plus
#                 associated errors and functions.3
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

import math


class NoSuchSignalError(Exception):
    """
    An exception to be raised when a signal of a device does not exist.
    """
    pass


class NoSuchDeviceError(Exception):
    """
    An exception to be raised when a device of a given name does not exist.
    """
    pass    


class Device(object):
    """
    Represents a physical device.
    """

    def __init__(self, name, type_, signals):
        """
        Creates a new device!

        @param name string containing the name of the device
        @param type_ string containing the type of the device
        @param signals dictionary of string signal names -> float values
        """
        self._name = name
        self._type = type_
        self._signals = signals

    def get_name(self):
        """
        @return string the name of this device
        """
        return self._name

    def get_type(self):
        """
        @return string the type of this device
        """
        return self._type

    def get_signals(self):
        """
        @return a set of strings naming the signals of this device
        """
        return self._signals.keys()

    def get_signal(self, signal):
        """
        @param signal string containing the name of the desired signal

        @ErrorHandling could raise NoSuchSignalError

        @return float value of the signal
        """
        if signal in self._signals:
            return self._signals[signal]
        else:
            raise NoSuchSignalError('Invalid signal: ' + signal)

    def set_signal(self, signal, value):
        """
        Set the signal to the specified value
        @param signal string name of the signal to command
        @param value float the new value of this signal
        """
        if math.isnan(value):
            print 'Not updating ({0}, {1}): received NaN command'.format(
                        self._name, signal)            
        else:
            self._signals[signal] = value

    def __repr__(self):
        """This is probably unholy"""
        return self.get_name()

def get_device(name, devices):
    """
    Loops through a set of devices and finds the device with the requested name

    @param name the name of the device to find
    @param devices the set of devices to look in

    @ErrorHandling throws NoSuchDeviceError if the device does not exist

    @return the desired device
    """
    for device in devices:
        if device.get_name() == name:
            return device
    else:
        raise NoSuchDeviceError('Device ' + name + ' not found in set')

def remove_device(name, devices):
    """
    Loops through a set of devices and removes the device with the requested name

    @param name the name of the device to find
    @param devices the set of devices to look in

    @ErrorHandling throws NoSuchDeviceError if the device does not exist

    @return A NEW SET
    """
    for device in devices:
        if device.get_name() == name:
            print devices
            devices.remove(device)
            print 'REMOVED', name
            print devices
            return devices
    else:
        raise NoSuchDeviceError('Device ' + name + ' not found in set')


def device_exists(name, devices):
    """
    like get_device but returns true/false
    """
    for device in devices:
        if device.get_name() == name:
            return True
    else:
        return False
