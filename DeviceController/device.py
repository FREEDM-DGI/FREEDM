# -*- mode: python; indent-tabs-mode: nil; tab-width: 4 -*-
###############################################################################
# @file           device.py
#
# @author         Michael Catanzaro <michael.catanzaro@mst.edu>
#
# @project        FREEDM Device Controller
#
# @description    A class for storing the signals of a physical device, plus
#                 associated errors and functions.
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
import threading


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

    def __init__(self, name, type_, signals, protected_state_duration):
        """
        Creates a new device!

        @param name string containing the name of the device
        @param type_ string containing the type of the device
        @param signals dictionary of string signal names -> float values
        @param protected_state_duration how long for which to protect a
               signal's state from commands after calling set_state
        """
        self._name = name
        self._type = type_
        self._signals = signals
        self._protected_state_duration = protected_state_duration
        # dict of strings -> ints indicating num locks on the signal
        # (recently-modified signals accept no commands until locks fall to 0)
        self._protected_signals = {}
        # mutex to provide thread-safety for the protected signals set
        self._protected_signals_lock = threading.Lock()

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

    def command_signal(self, signal, value):
        """
        Set the signal to the specified value, unless it has recently changed
        independently of a command from DGI or is NaN. Call this when the state
        of a signal needs to change due to a command by DGI.

        @param signal string name of the signal to command
        @param value float the new value of this signal
        """
        if signal in self._protected_signals.keys():
            print 'Not updating ({0}, {1}): signal has {2} lock(s)'.format(
                        self._name, signal, self._protected_signals[signal])
        elif value == float(10**8): # indicates a command to be dropped
            print 'Not updating ({0}, {1}): received null command'.format(
                        self._name, signal)            
        else:
            self._signals[signal] = value

    def set_signal(self, signal, value):
        """
        Set the signal to the specified value and prevent DGI from commanding
        it for a short while. Call this when the state of a signal changes
        independently of a command from DGI.

        @param signal string name of the signal to change
        @param value float the new value of this signal
        """
        assert not math.isnan(value)
        self._signals[signal] = value
        self._protected_signals_lock.acquire()
        if signal in self._protected_signals.values():
            self._protected_signals[signal] += 1
        else:
            self._protected_signals[signal] = 1
        self._protected_signals_lock.release()
        t = threading.Timer(
                self._protected_state_duration,
                Device._unlock_signal,
                args=(self, signal))
        t.start()

    def _unlock_signal(self, signal):
        """
        Removes a lock from a signal

        @param signal string name of the signal to unlock
        """
        self._protected_signals_lock.acquire()
        self._protected_signals[signal] -= 1
        if self._protected_signals[signal] == 0:
            del self._protected_signals[signal]
        self._protected_signals_lock.release()


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
