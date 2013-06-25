#!/bin/bash

HOST=`hostname`

case "$1" in
    "Configuration1")
        ../PosixBroker -c config/freedm.cfg --factory-port -53000
        ;;
    "Configuration2")
        ../PosixBroker -c config/freedm.cfg --factory-port 0
        ;;
    "Configuration3")
        ../PosixBroker -c config/freedm.cfg --factory-port 68000
        ;;
    "Configuration4")
        ../PosixBroker -c config/freedm.cfg --factory-port 53000wq
        ;;
    "Configuration5")
        ../PosixBroker -c config/freedm.cfg
        ;;
    "dgi1")
        ../PosixBroker -c config/freedm.cfg --port 1870 --factory-port 53000 --add-host "$HOST:1871"
        ;;
    "dgi2")
        ../PosixBroker -c config/freedm.cfg --port 1871 --factory-port 56000 --add-host "$HOST:1870"
        ;;
    "default")
        ../PosixBroker -c config/freedm.cfg --factory-port 53000
        ;;
    *)
        echo "Invalid test case: $1"
        exit
        ;;
esac

