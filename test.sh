#!/bin/bash

# count files in directories

IFS=$'\n'
ERR_LOG="/tmp/err.log"

exec 6>&2 2>$ERR_LOG

DIRS=`find $(readlink -f "$1") -type d -printf "%p\n"`

for i in $DIRS; do
	printf "$i `find "$i" -type f | wc -l`"
	printf '\n'
done

exec 2>&6 6>&-
sed "s/.[a-zA-Z]*:/`basename $0`:/" < $ERR_LOG 1>&2

rm $ERR_LOG

