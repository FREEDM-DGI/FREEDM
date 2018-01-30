#!/bin/sh -e

host=`hostname`
exec='../PosixBroker'

if [ $# -gt 3 ] || [ $# -lt 2 ]; then
    echo "Incorrect number of arguments! Tee-hee!"
    exit
fi

if [ $# -eq 3 ]; then
    exec=$3
fi

case $1 in
    "pnp")
        case $2 in
            "Configuration1")
                $exec -c config/freedm.cfg --factory-port -53000
                ;;
            "Configuration2")
                $exec -c config/freedm.cfg --factory-port 0
                ;;
            "Configuration3")
                $exec -c config/freedm.cfg --factory-port 68000
                ;;
            "Configuration4")
                $exec -c config/freedm.cfg --factory-port 53000wq
                ;;
            "Configuration5")
                $exec -c config/freedm.cfg
                ;;
            "dgi1")
                $exec -c config/freedm.cfg --port 51870 --factory-port 53000 --add-host "$host:51871"
                ;;
            "dgi2")
                $exec -c config/freedm.cfg --port 51871 --factory-port 56000 --add-host "$host:51870"
                ;;
            "default")
                $exec -c config/freedm.cfg --factory-port 53000
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
                $exec -v3 -c config/freedm.cfg --adapter-config config/adapters/$2.xml
            ;;
            "SingleDgi")
                $exec -v3 -c config/freedm.cfg --adapter-config config/adapters/$2.xml &
                sleep 15
                killall PosixBroker
                sleep 1
            ;;
            "MultipleDgi")
                $exec -v3 -c config/freedm.cfg --adapter-config config/adapters/"$2"A.xml --port 51870 --add-host "$host:51871" --add-host "$host:51872" --add-host "$host:51873" --add-host "$host:51874" &
                $exec -v3 -c config/freedm.cfg --adapter-config config/adapters/"$2"B.xml --port 51871 --add-host "$host:51870" --add-host "$host:51872" --add-host "$host:51873" --add-host "$host:51874" &
                $exec -v3 -c config/freedm.cfg --adapter-config config/adapters/"$2"C.xml --port 51872 --add-host "$host:51871" --add-host "$host:51870" --add-host "$host:51873" --add-host "$host:51874" &
                $exec -v3 -c config/freedm.cfg --adapter-config config/adapters/"$2"D.xml --port 51873 --add-host "$host:51871" --add-host "$host:51872" --add-host "$host:51870" --add-host "$host:51874" &
                $exec -v3 -c config/freedm.cfg --adapter-config config/adapters/"$2"E.xml --port 51874 --add-host "$host:51871" --add-host "$host:51872" --add-host "$host:51873" --add-host "$host:51870" &
                sleep 40
                killall PosixBroker
                sleep 1
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

