#!/usr/bin/python2
from fabric.api import *
from optparse import OptionParser
from os.path import expanduser
import subprocess

import network
import fabfile
import experiment

def generate_parser():
    parser = OptionParser()
    parser.add_option("-f","--hostname-file",dest="hostfile",help="File containing Hostnames's used in the test, one per line.")
    parser.add_option("-d","--dry-run",action="store_true",dest="dryrun",default=False,help="Don't actually issue the run experiment commands, simply prepare the experiments.")
    parser.add_option("-n","--hostname",dest="hostnames",action="append",help="UUIDs to use for this experiment set.")
    parser.add_option("-o","--output-file",dest="outputfile",default="exp.dat",help="File to write the experiment table to, if not provided, defaults to exp.dat")
    parser.add_option("-g","--granularity",dest="granularity",default=5,type="int",help="The simulator will run experiments with a step up or down in the granularity for each network connection.")
    parser.add_option("-t","--time",dest="time",default="10m",help="The option that will be provided to the unix command timeout to terminate cases.")
    return parser

def disconnect_all2():
    from fabric.state import connections
    for key in connections.keys():
        connections[key].close()
        del connections[key]

def map_hosts_to_uuids(hostlist):
    uuids = dict()
    for host in hostlist:
        with settings(host_string=host):
            uuids.setdefault(host,fabfile.get_uuid())
    return uuids

if __name__ == "__main__":
    parser = generate_parser()
    (options,args) = parser.parse_args()
    if options.hostfile != None:
        f = open(options.hostfile)
        options.hostnames = f.readlines()
        options.hostnames = [ x.strip() for x in options.hostnames ]
        f.close()
    host2uuid = map_hosts_to_uuids(options.hostnames)
    #PREPARE THE EXPERIMENT
    exp = experiment.Experiment(host2uuid,options.granularity)
    f = open(options.outputfile,'w',0)
    f.write(exp.tsv_head()+"\n")
    while 1:
        f.write(exp.tsv_entry()+"\n")
        hostlist = exp.generate_files()
        for (host,fd) in hostlist.iteritems():
            with settings(host_string=host):
                fabfile.setup_sim(fd)
      
        for (host,fd) in hostlist.iteritems():
            with settings(host_string=host):
                if not options.dryrun:
                    fabfile.start_sim(options.time)
                else:
                    print "Skipping start_sim; dry run."
        with settings(host_string='localhost'):
            if not options.dryrun:
                fabfile.wait_sim(options.time)
                print "Sending TERM"
        
        for (host,fd) in hostlist.iteritems():
            with settings(host_string=host):
                if not options.dryrun:
                    fabfile.end_sim()
                
        with settings(host_string='localhost'):
            if not options.dryrun:
                print "Waiting...",
                fabfile.wait_sim('10s')    
                print "Done!"    
 
        if exp.next() == None:
            break
    disconnect_all2()
