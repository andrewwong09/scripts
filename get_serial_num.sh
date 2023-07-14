#!/bin/bash

video_devices=$(ls /dev/video*)
echo $video_devices

serial_array=()

for dev in $video_devices
do
	serial_num=$(/bin/udevadm info --name=$dev | grep SERIAL_SHORT | grep -o -E '[0-9]{8}')
	echo "$dev: $serial_num"
	if [[ ${serial_array[@]} =~ $serial_num ]]
	then
		echo "Found $serial_num"
	else
		serial_array+=($serial_num)
	fi
done
