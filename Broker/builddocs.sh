#!/bin/sh
scriptdir=`dirname $0`
cd $scriptdir/doc
doxygen freedm.dxy
