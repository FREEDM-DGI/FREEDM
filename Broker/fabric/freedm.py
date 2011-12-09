#!/usr/bin/python2
from fabric.api import *
from optparse import OptionParser,OptionGroup
import ConfigParser
from os.path import expanduser
import subprocess
import time

import network
import fabfile
import experiment

def generate_parser():
    parser = OptionParser()
    parser.add_option("-f","--config-file",dest="configfile",
        default="dgi.cfg", help="Config file to use.")
    parser.add_option("-d","--dry-run",action="store_true",dest="dryrun",
                      default=False,help="Don't actually issue the run"
                      "commands.")
    return parser

def disconnect_all2():
    from fabric.state import connections
    for key in connections.keys():
        connections[key].close()
        del connections[key]

def map_hosts_to_uuids(path,hostlist):
    uuids = dict()
    for host in hostlist:
        with settings(host_string=host):
            uuids.setdefault(host,fabfile.get_uuid(path))
    return uuids

if __name__ == "__main__":
    parser = generate_parser()
    (options,args) = parser.parse_args()

    config = ConfigParser.ConfigParser()
    config.read(options.configfile)

    hostnames = []
    line = config.getboolean('lineserver','enable')
    """
    if line:
        linehost = config.get('lineserver','host')
        tmp = linehost.split(":")
        if len(tmp) != 2:
            print "Line host needs to be in the form hostname:port"
            exit(1)
        linehost = (tmp[0],tmp[1])
    """
    exp = config.getboolean('networkexp','enable')
    if exp:
        granularity = config.get('networkexp','granularity')
        outputfile = config.get('networkexp','outputfile')

    path = config.get('options','path')
    #default_port = config.get('options','default_port')
    runtime = config.get('options','runtime')

    for i in xrange(20000):
        section = "host"+str(i)
        if not config.has_section(section):
            break
        tmp = config.get(section,'host',0)
        tmp3 = tmp.split(":")
        if len(tmp3) == 1:
            tmp3.append(1870)
        if len(tmp3) == 0:
            print "Need a hostname/port for entry %s" % section
            exit(1)
        r = (tmp3[0],tmp3[1])
        hostnames.append(r)

    if line:
        print "I am going to launch the LineServer. When the simulation has",
        print "connected to the line server, press ENTER."

        print "Starting LineServer in..."
        
        c = range(5)
        c.reverse()

        for i in c:
            print "%s..." % (i+1)
            time.sleep(1)

        cmd = ['fab','-H',"linehost","start_linehost:%s" % path, "--linewise"]
        if not options.dryrun:
            x = subprocess.Popen(cmd)

        raw_input("Press Enter to continue...")

    host2uuid = map_hosts_to_uuids(path,[ x[0] for x in hostnames ])
    #PREPARE THE EXPERIMENT
    if exp:
        exp = experiment.Experiment(host2uuid,options.granularity)
        #Hack until I fix stuff
        #exp.fix_edge(options.hostnames[0],options.hostnames[2],100)
        #exp.fix_edge(options.hostnames[1],options.hostnames[3],100)
        f = open(outputfile,'w',0)
        f.write(exp.tsv_head()+"\n")
    while 1:
        #Setup network.xml
        if exp:
            print exp.expcounter
            f.write(exp.tsv_entry()+"\n")
            hostlist = exp.generate_files()
            for (host,fd) in hostlist.iteritems():
                with settings(host_string=host):
                    fabfile.setup_sim(fd)
 
        if not options.dryrun:
            #This simplifies the operations and lets us "spy" on the running tasks.
            cmd = ['fab','-H', ",".join([ x[0] for x in hostnames]),
                   "start_sim:%s,%s" % (path,runtime), "--linewise" ]
            #Uses suprocess to run the sim
            subprocess.call(cmd)
        else:
            print "Skipping start_sim; dry run."

        print "All runs should now be dead. Sleeping to let everything clean."
        time.sleep(10)
 
        if not exp or exp.next() == None:
            break
    disconnect_all2()
