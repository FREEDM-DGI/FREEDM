#!/usr/bin/python2
from fabric.api import *
from optparse import OptionParser
from os.path import expanduser
import subprocess
import time

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
    env.hosts = options.hostnames

    host2uuid = map_hosts_to_uuids(options.hostnames)
    #PREPARE THE EXPERIMENT
    exp = experiment.Experiment(host2uuid,options.granularity)
    #Hack until I fix stuff
    exp.fix_edge(options.hostnames[0],options.hostnames[2],100)
    exp.fix_edge(options.hostnames[1],options.hostnames[3],100)
    f = open(options.outputfile,'w',0)
    f.write(exp.tsv_head()+"\n")
    hs = ",".join(options.hostnames)
    while 1:
        print exp.expcounter
        f.write(exp.tsv_entry()+"\n")
        hostlist = exp.generate_files()
        for (host,fd) in hostlist.iteritems():
            with settings(host_string=host):
                fabfile.setup_sim(fd)
    
        if not options.dryrun:
            #This simplifies the operations and lets us "spy" on the running tasks.
            cmd = ['fab','-H', ",".join(options.hostnames), "start_sim:%s" % options.time, "--linewise" ]
            #Uses suprocess to run the sim
            subprocess.call(cmd)
        else:
            print "Skipping start_sim; dry run."

        print "All runs should now be dead. Sleeping to let everything clean."
        time.sleep(10)
 
        if exp.next() == None:
            break
    disconnect_all2()
