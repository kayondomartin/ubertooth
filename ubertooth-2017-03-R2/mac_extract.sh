#!/bin/bash
date=$1
iter=$2
loc=$3

cat log$3 | grep 'AdvA' > data/$date/log_adva
cat log$3 | grep 'rssi' > data/$date/log_rssi

nline=$(cat data/$date/log_adva | wc -l)
touch data/$date/mac_rssi_${iter}_${loc}.txt

n=1
for i in $(seq 1 $nline)
do
	line=$(awk -v n=$i 'NR == n' data/$date/log_adva)
	macAddr=${line:10:18}
	
	line=$(awk -v n=$i 'NR == n' data/$date/log_rssi)
	sRSSI=`expr length "$line" - 3`
	rssi=${line:$sRSSI:4}
	
	nmac=$(cat data/$date/mac_rssi_${iter}_${loc}.txt | wc -l)
	exist=0
	for j in $(seq 1 $nmac)
	do
		line=$(awk -v n=$j 'NR == n' data/$date/mac_rssi_${iter}_${loc}.txt)
		eMac=${line:0:17}
		if [ $macAddr = $eMac ]
		then
			exist=1
			var=$(echo rssi_$j)
			eval rssi_"$j"=`expr ${!var} + $rssi`
			var=$(echo num_$j)
			eval num_"$j"=`expr ${!var} + 1`
			break
		fi
	done

	if [ $exist -eq 0 ]
	then
		eval rssi_"$n"=$rssi
		eval num_"$n"=1
		echo $macAddr >> data/$date/mac_rssi_${iter}_${loc}.txt
		n=`expr $n + 1`
	fi
done

nmac=$(cat data/$date/mac_rssi_${iter}_${loc}.txt | wc -l)
for j in $(seq 1 $nmac)
do
	sumRssi=$(echo rssi_$j)
	num=$(echo num_$j)
	eval avgRssi_$j=$(echo "${!sumRssi}/${!num}" | bc -l)
	var=$(echo avgRssi_$j)
	echo ${!var} >> data/$date/mac_rssi_${iter}_${loc}.txt
done
