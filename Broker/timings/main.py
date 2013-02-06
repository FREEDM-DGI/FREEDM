#!/usr/bin/env python2
"""
@file         main.py

@author       Stephen Jackson <scj7t4@mst.edu>

@project      FREEDM DGI

@description  Controls the program flow and outputs the results.

These source code files were created at Missouri University of Science and
Technology, and are intended for use in teaching or research. They may be
freely copied, modified, and redistributed as long as modified versions are
clearly marked as such and this notice is not removed. Neither the authors
nor Missouri S&T make any warranty, express or implied, nor assume any legal
responsibility for the accuracy, completeness, or usefulness of these files
or any information distributed with these files.

Suggested modifications or questions about these files can be directed to
Dr. Bruce McMillin, Department of Computer Science, Missouri University of
Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
"""

from variables import VARIABLES
from parameters import PARAMETERS 
from templates import *

def main():
    print PARAMETERS
    generated = {}
    for (k,v) in VARIABLES.iteritems():
        generated[k] = v(**PARAMETERS)
    cfg_file('timings.cfg',generated)
    hpp_file('CTimings.hpp',generated)
    cpp_file('CTimings.cpp',generated)

if __name__ == "__main__":
    main()
