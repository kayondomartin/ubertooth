#!/bin/bash

#ubertooth-btle -s ec:55:f9:7c:c9:ff -S -U 0 >& log0
#scp mwnl1@192.168.70.201:~/JWHUR/ubertooth/ubertooth-2017-03-R2/exp/log1 ./
#scp mwnl1@192.168.86.24:~/JWHUR/ubertooth/ubertooth-2017-03-R2/exp/log1 ./

catTime=$(sed -n -e 's/^.*measurement : //p' log0)
echo $catTime > time
tr -s ' ' '\n'< time > time.dat
rm time

numLine=$(wc -l < "time.dat")

catRssi=$(sed -n -e 's/^.*samples : //p' log0)
echo $catRssi > rssi
tr -s ' ' '\n'< rssi > rssi.dat
rm rssi

./procData.out
paste time.dat rssi.dat > exp0.dat
rm time.dat rssi.dat log0
mv signal.dat signal0.dat

catTime=$(sed -n -e 's/^.*measurement : //p' log1)
echo $catTime > time
tr -s ' ' '\n'< time > time.dat
#rm time

numLine=$(wc -l < "time.dat")

catRssi=$(sed -n -e 's/^.*samples : //p' log1)
echo $catRssi > rssi
tr -s ' ' '\n'< rssi > rssi.dat
rm rssi

./procData.out
paste time.dat rssi.dat > exp1.dat
rm time.dat rssi.dat
mv signal.dat signal1.dat

#cmp -l Barcode0.dat Barcode1.dat > diff.dat

#printf "\nBit diff: "
#echo $(wc -l < diff.dat)
#rm diff.dat

gnuplot -e "set multiplot layout 2,1; set xrange [0:1000000]; set yrange [-100:-10]; set grid; plot 'exp0.dat' with lines linestyle 1; set xrange [0:1000000]; set yrange [0:1]; plot 'signal.dat' with lines  lw 2, 'signal.dat' with points pt 7 lc 1; unset multiplot; pause -1"
# gnuplot -e "set multiplot layout 2,1; set xrange [0:1e+6]; set yrange [-100:-20]; set grid; plot 'exp0.dat' with lines linestyle 1; set xrange [0:100]; set yrange [0:1]; plot 'Barcode0.dat' with impulses  lw 2, 'Barcode0.dat' with points pt 7 lc 1; unset multiplot; pause -1"
