"""
@file         variables.py

@author       Stephen Jackson <scj7t4@mst.edu>

@project      FREEDM DGI

@description  The variables that should be produced into the timing.cfg
    This file also defines the functions that will produce timing values.

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

from profiles import *

def dummy(*args, **kwargs):
    return 0

VARIABLES = {
    # GM VARIABLES
    'GM_PHASE_TIME': gm_phase_time,
    'GM_CHECK_TIMEOUT': gm_check_timeout,
    'GM_TIMEOUT_TIMEOUT': gm_timeout_timeout,
    'GM_GLOBAL_TIMEOUT': gm_global_timeout,
    'GM_FID_TIMEOUT': gm_fid_timeout,
    'GM_PREMERGE_MAX_TIMEOUT': gm_premerge_max_timeout,
    'GM_PREMERGE_MIN_TIMEOUT': gm_premerge_min_timeout,
    'GM_PREMERGE_GRANULARITY': gm_premerge_granularity,
    'GM_INVITE_RESPONSE_TIMEOUT': gm_invite_response_timeout,
    'GM_AYT_RESPONSE_TIMEOUT': gm_ayt_response_timeout,
    'GM_AYC_RESPONSE_TIMEOUT': gm_invite_response_timeout,
    # SC VARIABLES
    'SC_PHASE_TIME': sc_phase_time,
    # LB VARIABLES
    'LB_PHASE_TIME': lb_phase_time,
    'LB_GLOBAL_TIMER': lb_global_timer,
    'LB_STATE_TIMER': lb_state_timer,
    # CSRC VARIABLES
    'CSRC_RESEND_TIME': csrc_resend_time,
    'CSRC_DEFAULT_TIMEOUT': csrc_default_timeout,
    # CSUC VARIABLES
    'CSUC_RESEND_TIME': csuc_resend_time,
    # CS VARIABLES
    'CS_EXCHANGE_TIME': cs_exchange_time,
    # DEV VARIABLES
    'RTDS_RUN_DELAY': rtds_run_delay,
}
