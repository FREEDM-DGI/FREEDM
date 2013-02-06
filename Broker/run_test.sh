#!/bin/bash

if [ $# -ne 1 ]
then
    echo "Usage: $0 <1|2>"
    exit
fi

HOST=`hostname`

if [ $1 -eq 1 ]
then
    ./PosixBroker -c config/testing/DGI-1.cfg --add-host "$HOST:1871"
elif [ $1 -eq 2 ]
then
    ./PosixBroker -c config/testing/DGI-2.cfg --add-host "$HOST:1870"
else
    echo "Invalid argument."
    exit
fi


