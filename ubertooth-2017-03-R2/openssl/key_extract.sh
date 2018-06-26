#!/bin/bash

infile=$1
outfile=$2

numl=$(cat $infile | wc -l)
last=`expr $numl - 1`

sed -n '2,'$last'p' $infile > temp
cat temp | tr -d '\n' > $outfile

rm temp
