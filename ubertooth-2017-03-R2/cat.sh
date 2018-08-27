#!/bin/bash
date=$1
iter=$2

mkdir data/$date

cat log0 | grep samples > data/$date/rssi0_$iter.txt
cat log0 | grep measurement > data/$date/time0_$iter.txt
cat log1 | grep samples > data/$date/rssi1_$iter.txt
cat log1 | grep measurement > data/$date/time1_$iter.txt
