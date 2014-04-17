#!/usr/bin/env python2
###############################################################################
# @file             dxyfilter.py
#
# @author           Michael Catanzaro <michael.catanzaro@mst.edu>
#
# @project          FREEDM DGI
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
# Science and Technology, Rolla, MO 65409 <ff@mst.edu>
###############################################################################

"""
Translate a FREEDM source file into Doxygen-friendly form.

This primarily entails throwing out the list of functions from the top of each
source file, as well as the copyright notice that appears at the top of the
file. While this text doesn't affect the class documentation, it does ruin the
file documentation, and this script will get rid of it before Doxygen ever
sees it.

This file is a Doxygen input filter, configured in freedm.dxy. Doxygen will
call this script every time it parses a FREEDM source file, and will pipe the
text of that source file to this script's stdin. Doxygen will then run on this
script's stdout, instead of the original source file. That is, the output of
this script is what Doxygen actually sees.
"""

import fileinput
import os
import sys

# We'll make this really simple. Print the line to stdout if we're in
# KEEP_LINE_STATE or KEEP_REMAINING_LINES_STATE. Go on to the next line if
# we're in DISCARD_LINE_STATE.

KEEP_LINE_STATE = 0
DISCARD_LINE_STATE = 1
KEEP_REMAINING_LINES_STATE = 2

if not os.path.isfile(sys.argv[1]):
    sys.exit('Oh no, %s is not a valid file!' % sys.argv[1])

state = KEEP_LINE_STATE

def stateTransition(state, line):
    """
    stateTransition( int, string ) -> string

    Start throwing out lines when we reach @functions or the license. Stop
    throwing out lines when we reach the end of the comment header. The return
    value of this function must be assigned to the state global
    """
    if state < 0 or state > 2:
        raise RuntimeError('Fatal: in unknown state %d' % state)
    elif state == KEEP_LINE_STATE:
        if '@functions' in line:
            return DISCARD_LINE_STATE
        elif 'These source code files' in line:
            return DISCARD_LINE_STATE
        else:
            return KEEP_LINE_STATE
    elif state == DISCARD_LINE_STATE:
        if '/////////////////' in line:
            return KEEP_REMAINING_LINES_STATE
        else:
            return DISCARD_LINE_STATE
    elif state == KEEP_REMAINING_LINES_STATE:
        return KEEP_REMAINING_LINES_STATE

for line in fileinput.input():
    state = stateTransition(state, line)
    # Could result in lost documentation
    if '@function' in line or '@fn' in line or state == DISCARD_LINE_STATE:
        # if we actually discard the line, we'll mess up line numbering
        sys.stdout.write('\n')
    elif state == KEEP_REMAINING_LINES_STATE or state == KEEP_LINE_STATE:
        sys.stdout.write(line)
    else:
        raise RuntimeError('Fatal: in unknown state ' + str(state))
