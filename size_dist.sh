#!/bin/bash
[ -z "$1" ] && echo "Usage: $0 folder_or_txt [interval]"
[ -z "$1" ] && exit 1

[ -f "$1" -o -d "$1" ] || echo "Invalid file or folder!" 
[ -f "$1" -o -d "$1" ] || exit 2

if [ -f "$1" ]
then
	build/incremental_wc $2 < $1
else
	cat $1/* | lz4 -dc | build/incremental_wc $2
fi
