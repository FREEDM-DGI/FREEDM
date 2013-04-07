#!/bin/bash
trap "kill 0" SIGINT

if [ -z $1 ]; then
    echo "Usage: $0 <TestPrefix> [ConfigPrefix]"
    exit
fi

for i in $1.txt $1[A-Z].txt; do
    if [ -f $i ]; then
        CTR=${i%.txt}
        CTR=${CTR#$1}

        if [ $# == "1" ]; then
            CFG="config/controller$CTR.cfg"
        elif [ $# == "@" ]; then
            CFG="config/$2$CTR.cfg"
        fi

        echo "Using configuration $CFG with the script $i..."
        ../controller.py -c $CFG -s $i &
    fi
done

wait
