#!/bin/bash


ip=$(ip addr | grep -E -o '10.50.[0-9]{1,3}\.[0-9]{1,3}' | tr '\n' ' ')
hostname=$(hostname)
dt=$(date '+%Y-%m-%d %H:%M:%S')
echo "$dt: $hostname: $ip" >> $HOME/scratch/ip.txt
scp "$HOME/scratch/ip.txt" "andrew@10.50.0.174:/home/andrew/reported_ip.txt"
