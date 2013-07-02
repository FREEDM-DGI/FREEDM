"""
@file         profiles.py

@author       Stephen Jackson <scj7t4@mst.edu>

@project      FREEDM DGI

@description  Defines the functions that are used in the variables file.

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

#This is magic constant that makes it so an event that fires once per round will do so.
ONE_PER_ROUND = 10

def round_trip_time(*args,**kwargs):
    tp = kwargs.get('tp')
    tm = kwargs.get('tm')
    tr_proc = kwargs.get('tr_proc')
    ts_proc = kwargs.get('ts_proc')
    return 2*(tr_proc + tp + tm + ts_proc)

def sc_phase_time(*args,**kwargs):
    tp = kwargs.get('tp')
    tm = kwargs.get('tm')
    tr_proc = kwargs.get('tr_proc')
    ts_proc = kwargs.get('ts_proc')
    n = kwargs.get('n')
    part1 = ts_proc * (n-1) * (tp+tm)
    part2 = tr_proc + ts_proc + (n-1) * (n-1) * (tp+tm)
    part3 = tr_proc + ts_proc + (n-1) * (tp + tm)
    part4 = tr_proc + ts_proc + (n-1) * (tp + tm)
    return part1 + part2 + part3 + part4

def lb_state_timer(*args,**kwargs):
    return lb_phase_time(*args,**kwargs)+ONE_PER_ROUND

def lb_global_timer(*args,**kwargs):
    tp = kwargs.get('tp')
    tm = kwargs.get('tm')
    tr_proc = kwargs.get('tr_proc')
    ts_proc = kwargs.get('ts_proc')
    n = kwargs.get('n')
    part1 = ts_proc + (n-1)/2 * (tp+tm)
    part2 = tr_proc + (n-1)/4 * (tp+tm)
    part3 = tr_proc + ts_proc + (tp+tm)
    part4 = part3
    return part1 + part2 + part3 + part4

def lb_phase_time(*args,**kwargs):
    c = kwargs.get('lb_per_phase')
    # Add 10ms of leeway of LB will one one time fewer than expected
    return c * lb_global_timer(*args,**kwargs) + 10

def gm_phase_time(*args,**kwargs):
    tp = kwargs.get('tp')
    tm = kwargs.get('tm')
    tr_proc = kwargs.get('tr_proc')
    ts_proc = kwargs.get('ts_proc')
    n = kwargs.get('n')
    t  = ts_proc + (n-1) * (tp+tm)
    t += ts_proc + (n-1) * (tp+tm)
    t += ts_proc + (n-1) * (tp+tm) + gm_premerge_max_timeout(*args, **kwargs)
    t += ts_proc + (n-1) * (tp+tm)
    t += tr_proc + ts_proc + (n-1) * (tp+tm)
    t += tr_proc
    t += ts_proc + (n-1) * (tp+tm)
    t += tr_proc + gm_invite_response_timeout(*args,**kwargs) + ts_proc + (n-1) * (tp + tm) + ts_proc    
    return t

def gm_premerge_min_timeout(*args,**kwargs):
    return gm_premerge_granularity(*args,**kwargs)

def gm_premerge_max_timeout(*args, **kwargs):
    n = kwargs.get('n')
    return gm_premerge_granularity(*args,**kwargs)*(n+1)

def gm_premerge_granularity(*args, **kwargs):
    return round_trip_time(*args,**kwargs)

def gm_invite_response_timeout(*args, **kwargs):
    n = kwargs.get('n')
    return (n+1) * round_trip_time(*args,**kwargs)

def gm_check_timeout(*args,**kwargs):
    return gm_phase_time(*args,**kwargs)+ONE_PER_ROUND

def gm_timeout_timeout(*args,**kwargs):
    return gm_phase_time(*args, **kwargs)+ONE_PER_ROUND

def gm_global_timeout(*args, **kwargs):
    return gm_phase_time(*args, **kwargs)+ONE_PER_ROUND

def gm_fid_timeout(*args, **kwargs):
    return gm_phase_time(*args, **kwargs)+ONE_PER_ROUND

def gm_ayt_response_timeout(*args, **kwargs):
    tp = kwargs.get('tp')
    tm = kwargs.get('tm')
    tr_proc = kwargs.get('tr_proc')
    ts_proc = kwargs.get('ts_proc')
    n = kwargs.get('n')
    return ts_proc + (tp+tm) + tr_proc + ts_proc + (tp+tm) * (n) + tr_proc

def csrc_resend_time(*args, **kwargs):
    return round_trip_time(*args,**kwargs)

def csuc_resend_time(*args, **kwargs):
    return round_trip_time(*args,**kwargs)

def csrc_default_timeout(*args,**kwargs):
    return max(gm_phase_time(*args, **kwargs),sc_phase_time(*args,**kwargs),lb_phase_time(*args,**kwargs))

def cs_exchange_time(*args,**kwargs):
    rounds_per_sync = kwargs.get('rounds_per_sync')
    return rounds_per_sync * (gm_phase_time(*args, **kwargs) + sc_phase_time(*args,**kwargs) + lb_phase_time(*args,**kwargs))

def dev_rtds_delay(*args, **kwargs):
    return 50

def lb_sc_query_time(*args, **kwargs):
    #This can be magic because the SC query should only generate a local message.
    return 20

def dev_socket_timeout(*args, **kwargs):
    return 1000

def dev_pnp_heartbeat(*args, **kwargs):
    return 2000
