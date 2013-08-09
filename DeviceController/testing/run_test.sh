#!/bin/bash
trap "kill 0" SIGINT

PREFIX=scripts

if [ -z $1 ]; then
    echo "Usage: $0 <TestPrefix>"
    exit
fi

TEST=${1#scripts/}

for i in $PREFIX/$TEST.txt $PREFIX/$TEST[A-Z].txt; do
    if [ -f $i ]; then
        CTR=${i%.txt}
        CTR=${CTR##*[[:digit:]]}
        if [ -z $CTR ]; then
            CTR="TestController"
        fi

        PORT=53000
        if [[ $TEST == MultipleDGI* ]] && [ $CTR == "B" ]; then
            PORT=56000
        fi

        DUPLICATES=1
        if [ $TEST == "UnexpectedError5" ]; then
            DUPLICATES=2
        fi

        for (( j=1; j<=DUPLICATES; j++ )); do
            echo "Starting controller $CTR on port $PORT using script $i..."
            ../fake_controller.py -c config/controller.cfg -s $i -n $CTR -p $PORT &
        done
    fi
done

wait
