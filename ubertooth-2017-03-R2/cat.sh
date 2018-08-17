#!/bin/bash

cat log0 | grep rssi > data/0730/rssi0.txt
cat log1 | grep rssi > data/0730/rssi1.txt
cat log2 | grep rssi > data/0730/rssi2.txt

cat log0 | grep measurement > data/0730/time0.txt
cat log1 | grep measurement > data/0730/time1.txt
cat log2 | grep measurement > data/0730/time2.txt
