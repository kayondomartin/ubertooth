#!/bin/bash 

cd ./firmware/
make bluetooth_rxtx
sleep 1
ubertooth-dfu -d bluetooth_rxtx/bluetooth_rxtx.dfu -r -U 0
sleep 1
cd ../
./stop.sh
./tx_start.sh 7
