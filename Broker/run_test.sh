#!/bin/bash

if [ $# -ne 1 ]
then
    echo "Usage: $0 <1|2|3>"
    exit
fi

HOST=`hostname`

if [ $1 -eq 1 ]
then
    ./PosixBroker -c config/testing/DGI-1.cfg --add-host "$HOST:1871" --add-host "$HOST:1872"
elif [ $1 -eq 2 ]
then
    ./PosixBroker -c config/testing/DGI-2.cfg --add-host "$HOST:1870" --add-host "$HOST:1872"
elif [ $1 -eq 3 ]
then
    ./PosixBroker -c config/testing/DGI-3.cfg --add-host "$HOST:1870" --add-host "$HOST:1871"
else
    echo "Invalid argument."
    exit
fi


