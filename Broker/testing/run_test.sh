#!/bin/sh -e

host=`hostname`

case $1 in
    "pnp")
        case $2 in
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
                ../PosixBroker -c config/freedm.cfg --port 1870 --factory-port 53000 --add-host "$host:1871"
                ;;
            "dgi2")
                ../PosixBroker -c config/freedm.cfg --port 1871 --factory-port 56000 --add-host "$host:1870"
                ;;
            "default")
                ../PosixBroker -c config/freedm.cfg --factory-port 53000
                ;;
            *)
                echo "Invalid test case: $2"
                exit
        esac
        ;;
    "rtds")
        rtds_test_type=`echo $2 | tr --delete '[:digit:]'`
        case $rtds_test_type in
            "BadXml")
                ../PosixBroker -v3 -c config/freedm.cfg --adapter-config config/adapters/$2.xml
            ;;
            "SingleDgi")
                ../PosixBroker -v3 -c config/freedm.cfg --adapter-config config/adapters/$2.xml &
                sleep 3
                killall PosixBroker
            ;;
            "MultiDgi")
                ../PosixBroker -v3 -c config/freedm.cfg --adapter-config config/adapters/$2A.xml &
                ../PosixBroker -v3 -c config/freedm.cfg --adapter-config config/adapters/$2B.xml &
                sleep 3
                killall PosixBroker
            ;;
            *)
                echo "Invalid RTDS test type: $rtds_test_type"
                exit
        esac
        ;;
    *)
        echo "Invalid test category: $1"
        exit
esac

