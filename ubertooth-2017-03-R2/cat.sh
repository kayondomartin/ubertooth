#!/bin/bash
iter=$1

cat log0 | grep samples > data/0820/rssi0_$iter.txt
cat log0 | grep measurement > data/0820/time0_$iter.txt
cat log1 | grep samples > data/0820/rssi1_$iter.txt
cat log1 | grep measurement > data/0820/time1_$iter.txt
