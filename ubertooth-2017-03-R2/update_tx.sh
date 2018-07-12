#!/bin/bash 
device=$1

cd ./firmware/
make bluetooth_rxtx
sleep 1
ubertooth-dfu -d bluetooth_rxtx/bluetooth_rxtx.dfu -r -U $device
sleep 1
cd ../
./stop.sh
