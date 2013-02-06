#!/bin/bash
trap "kill 0" SIGINT

if [ -z $1 ]
then
    echo "Usage: $0 <TestPrefix>"
    exit
fi

for i in $1.txt $1[A-Z].txt; do
    if [ -f $i ]
    then
        CTR=${i%.txt}
        CTR=${CTR#$1}

        if [ $# -eq 1 ]
        then
            CFG="config/default.cfg"
        else
            CFG="$2$CTR.cfg"
        fi

        echo "Using configuration $CFG with the script $i..."
        python controller.py -c $CFG -s $i -n "TestController$CTR" &
    fi
done

wait

