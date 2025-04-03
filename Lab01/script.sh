#!/bin/bash

for (( i=1; i<=20; i++ ))
do
	echo "$i times:"
    rm ./flag.txt
    sudo timeout 5 tcpdump port 10001 and -v ip -A and 'ip[8]>150' > flag.txt & 
    sudo timeout 5 nc inp111.zoolab.org 10001  > tmp.txt &
    wait
	
    if [ $(wc -c <"flag.txt") -le 100 ] || [ $(wc -c <"flag.txt") -ge 1000 ]; then
        continue
    fi

    echo "ans:"
    sed '/^$/d' flag.txt | sed '1,3d' | cut -c 9- | base64 -d | awk -F " " '{print $4}'
    break
done


