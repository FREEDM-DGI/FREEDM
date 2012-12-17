"""
@file         parameters.py

@author       Stephen Jackson <scj7t4@mst.edu>

@project      FREEDM DGI

@description  Defines the parameters used to fill in the profiles

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

PARAMETERS = {
    # Time to acquire the channel
    'tp': 2,
    # Time to write the message to the channel
    'tm': 2, 
    # Time to process the outgoing message for sending
    'ts_proc': 2,
    # Time to process the outgoing message for recieving
    'tr_proc': 2,
    # The maximum size of groups that should be formed
    'n': 5,
    # The number of rounds between clock synchronizations.
    'rounds_per_sync': 10,
    # The number of load balancings that can be attempted per round
    'lb_per_phase': 2
}    
