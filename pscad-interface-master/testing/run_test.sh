#!/bin/sh -e

config_dir=config
xml_dir=$config_dir/xml

if [ -z $1 ]; then
	../driver -c $config_dir/simserv.cfg --xml $xml_dir/default.xml
else
	xml_file=$xml_dir/$1.xml
	if [ -e $xml_file ]; then
		../driver -c config/simserv.cfg --xml $xml_file
	else
		echo "Can't find $xml_file"
	fi
fi
