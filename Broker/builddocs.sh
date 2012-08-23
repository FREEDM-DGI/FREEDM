#!/bin/sh
scriptdir=`dirname $0`
cd $scriptdir/doc
echo 'Running Doxygen on Broker...'
doxygen freedm.dxy
