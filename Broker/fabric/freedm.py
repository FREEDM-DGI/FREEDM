#!/usr/bin/python2
from optparse import OptionParser
from os.path import expanduser
import subprocess

import network

def generate_parser():
    parser = OptionParser()
    parser.add_option("-f","--hostname-file",dest="uuidfile",help="File containing Hostnames's used in the test, one per line.")
    parser.add_option("-d","--dry-run",dest="dryrun",default=False,help="Don't actually issue the run experiment commands, simply prepare the experiments.")
    parser.add_option("-n","--hostname",dest="hostnames",action="append",help="UUIDs to use for this experiment set.")
    parser.add_option("-o","--output-file",dest="outputfile",default="exp.dat",help="File to write the experiment table to, if not provided, defaults to exp.dat")
    parser.add_option("-g","--granularity",dest="granularity",default=5,type="int",help="The simulator will run experiments with a step up or down in the granularity for each network connection.")
    parser.add_option("-t","--time",dest="time",default="10m",help="The option that will be provided to the unix command timeout to terminate cases.")
    return parser


if __name__ == "__main__":
    parser = generate_parser()
    (options,args) = parser.parse_args()
    map_hosts_to_uuids(["google.com","amazon.com"])
