#!/bin/bash

data=$(cat $1)
u=$2

echo $data

ubertooth-btle -s ec:55:f9:12:7c:c9 -S $data -U $u
