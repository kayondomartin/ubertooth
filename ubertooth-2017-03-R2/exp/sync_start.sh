#!/bin/bash
u=$1

# ssh mwnl1@192.168.86.24 "cd ~/JWHUR/ubertooth/ubertooth-2017-03-R2/ && ./rx_start.sh 1" 

ubertooth-btle -s ec:55:f9:12:7c:c9 -S -A 39 -U $u >& log$u
sleep 0.1
scp mwnl1@192.168.86.24:~/JWHUR/ubertooth/ubertooth-2017-03-R2/log1 ./

catTime=$(sed -n -e 's/^.*measurement : //p' log0)
catRssi=$(sed -n -e 's/^.*samples : //p' log0)

echo $catTime > time
echo $catRssi > rssi
tr -s ' ' '\n'< time > time0.dat
tr -s ' ' '\n'< rssi > rssi0.dat

rm time
rm rssi

catTime=$(sed -n -e 's/^.*measurement : //p' log1)
catRssi=$(sed -n -e 's/^.*samples : //p' log1)

echo $catTime > time
echo $catRssi > rssi
tr -s ' ' '\n'< time > time1.dat
tr -s ' ' '\n'< rssi > rssi1.dat

rm time
rm rssi


./a.out
paste time0.dat rssi0.dat > exp0.dat
paste time1.dat rssi1.dat > exp1.dat

gnuplot -e "
set multiplot layout 2,1 columnsfirst; 
set xrange [0:1e+6];
set grid ytics lc 7 lw 1 lt 0;
set grid xtics lc 7 lw 1 lt 0;
plot 'exp0.dat' with lines lc 1, 'edge0.dat' with points lc 1, 'exp1.dat' with lines lc 3, 'edge1.dat' with points lc 3;
set xrange [0:100];
plot 'Barcode0.dat' with impulses lw 2 lc 1, 'Barcode0.dat' with points pt 7 lc 1, 'Barcode1.dat' with impulses lw 2 lc 3, 'Barcode1.dat' with points pt 7 lc 3;
unset multiplot;
pause -1"

