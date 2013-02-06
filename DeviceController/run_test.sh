#!/bin/bash
trap "kill 0" SIGINT

CFG="config/default.cfg"

if [ $# -eq 2 ]
then
    CFG_PREFIX=$2
elif [ $# -ne 1 ]
then
    echo "Usage: $0 <TestPrefix> <OptionalConfigPrefix>"
    exit
fi

for i in $1*.txt; do
    CTR_SUFFIX=${i%.txt}
    CTR_SUFFIX=${CTR_SUFFIX#$1}
    CTR="TestController$CTR_SUFFIX"

    if [ $# -eq 2 ]
    then
        CFG="$CFG_PREFIX$CTR_SUFFIX.cfg"
    fi

    echo "Starting $CTR using the script $i with the config $CFG..."
    python controller.py -c $CFG -s $i -n $CTR &
done

wait

