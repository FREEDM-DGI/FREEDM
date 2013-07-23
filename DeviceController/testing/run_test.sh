#!/bin/bash
trap "kill 0" SIGINT

PREFIX=scripts

if [ -z $1 ]; then
    echo "Usage: $0 <TestPrefix>"
    exit
fi

for i in $PREFIX/$1.txt $PREFIX/$1[A-Z].txt; do
    if [ -f $i ]; then
        CTR=${i%.txt}
        CTR=${CTR#$1}
        if [ -z $CTR ]; then
            CTR="TestController"
        fi
    
        PORT=53000
        if [[ $1 == MultipleDGI* ]] && [ $CTR == "B" ]; then
            PORT=56000
        fi

        echo "Starting controller $CTR on port $PORT using script $i..."
        ../fake_controller.py -c config/controller.cfg -s $i -n $CTR -p $PORT &
    fi
done

wait
