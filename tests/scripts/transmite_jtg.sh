#!/bin/sh

#Signal Handler
trap "exit" 1 2 3

packet_size="1300 1200 1100 1000 900 800 700 600 500 400 300"

for i in $packet_size; do
	for ((n=0;n<10;n++)); do
		./jtg -u -l 1200 -b 100000000 -d 10 10.1.1.12
	done
done
