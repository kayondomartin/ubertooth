#!/bin/bash

data=$(cat $1)
tx_level=$1

echo $data

ubertooth-btle -s ec:55:f9:12:7c:c9 -d $data -l $tx_level -U 0
