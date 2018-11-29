#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "bleControl.h"

int syncStart(char *macAP) {
	int status;
	char *pre_com, *post_com, command[100] = "";

	pre_com = "ubertooth-btle -s ";
	post_com = " -S -U 0 > log";

	strcat(command, pre_com);
	strcat(command, macAP);
	strcat(command, post_com);
	
	status = system(command); // ubertooth-btle -s macAP -s -U 0 >& log
	status = system("catTime=$(sed -n -e 's/^.*measurement : //p' log); echo $catTime > time; tr -s ' ' '\n'< time > time.dat; rm time");
	status = system("catRssi=$(sed -n -e 's/^.*samples : //p' log); echo $catRssi > rssi; tr -s ' ' '\n'< rssi > rssi.dat; rm rssi");

	return status;
}

int dataTx(char *macAP, int *bch, int rLen, char *encPwd, int txDur) {
	int status, i, pwdLen;
	uint32_t parity = 0x00000000;
	uint8_t errCap = 0x00, *parityByte;
	int *eccInt;
	char eccChar[10], pwdChar[10];
	char *pre_com, *mid1_com, *mid2_com, *post_com, eccHdr[50] = "", data[100] = "", command[300] = "", dur[12];

	parityByte = (uint8_t *)&parity;

	if (rLen == 0)
		errCap = 0x00;
	else if (rLen == 14)
		errCap = 0x02;
	else
		errCap = 0x04;

	eccInt = (int*)malloc(sizeof(int)*rLen);
	for(i=0; i<rLen; i++) {
		eccInt[i] = bch[127 - rLen + i];
	}
	for(i=0; i<rLen; i++) {
		parity <<= 1;
		parity |= eccInt[i];
	}

	sprintf(eccChar, "%02x", errCap);
	strcat(eccHdr, eccChar);
	for(i=1; i<5; i++) {
		sprintf(eccChar, "%02x", parityByte[4-i]);
		strcat(eccHdr, eccChar);
	}

	
	/**********************
	  BLE packet format for BLE IoT authentication association protocol

	  BLE header - Eddystone header - Eddystone URL header - protocol preamble (aa) - ECC header - enc pwd preamble (cc) - enc. pwd
	
	 *********************/

	strcat(data, eccHdr);	// Add ecc Header
	strcat(data, "cc");		// Add preamble for encrypted password

	pwdLen = strlen(encPwd);
	for(i=0; i<pwdLen; i++) {
		sprintf(pwdChar,"%02x", (uint8_t)encPwd[i]);
		strcat(data, pwdChar);
	}
	printf("encPwd: %s\n", encPwd);

	pre_com = "ubertooth-btle -s ";
	mid1_com = " -d ";
	mid2_com = " -T ";
	post_com = " -U 0";
	sprintf(dur, "%d", txDur);

	strcat(command, pre_com);
	strcat(command, macAP);
	strcat(command, mid1_com);
	strcat(command, data);
	strcat(command, mid2_com);
	strcat(command, dur);
	strcat(command, post_com);

	status = system(command);
	free(eccInt);
	return status;
}

int startRx(int rxDur) {
	int status;
	char *pre_com, *post_com, command[100] = "", dur[12];
	pre_com = "ubertooth-btle -f -T ";
	post_com = " -U 0 > log"; 
	sprintf(dur, "%d", rxDur);

	strcat(command, pre_com);
	strcat(command, dur);
	strcat(command, post_com);
	printf("%s\n", command);

	status = system(command);
	return status;
}

int syncReceived(char *APMAC) {
	FILE *syncReceived, *apMACADDR;
	int status;
	char line[10] = "", syncFlag;
	char command[] = "grep -q 'SYNC received' log; [ $? -eq 0 ] && echo '1' || echo '0'";

	syncReceived = popen(command, "r");
	if(syncReceived != NULL) {
		while(fgets(line, sizeof(10), syncReceived) != NULL);
	}
	pclose(syncReceived);
	syncFlag = line[0];
	if(syncFlag == '1') {
		printf("Sync received\n");
		apMACADDR = popen("line=$(cat log | grep 'SYNC received'); macAddr=$(echo $line | awk '{print $6}'); echo $macAddr", "r");
		if(apMACADDR != NULL) {
			while(fgets(line, sizeof(10), apMACADDR) != NULL)
				strcat(APMAC, line);
			printf("AP mac address: %s", APMAC);
		}
		status = system("catTime=$(sed -n -e 's/^.*measurement : //p' log); echo $catTime > time; tr -s ' ' '\n'< time > time.dat; rm time");
		status = system("catRssi=$(sed -n -e 's/^.*samples : //p' log); echo $catRssi > rssi; tr -s ' ' '\n'< rssi > rssi.dat; rm rssi");
		pclose(apMACADDR);
		return 1;
	} else 
		return 0;
}



