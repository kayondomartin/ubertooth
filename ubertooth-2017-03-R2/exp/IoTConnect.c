/***************************
  JWHUR, I implemented the overall BleIoT connection protocol in this code

  OvERALL PROTOCOL
  
  Smartphone   -------->     IoT device
  
  send sync packet --->      receive sync packet
  wait 100ms                 wait 100ms
  RSSI sampling 100ms        RSSI sampling 100ms
  Make barcode
  Make BCH code
  send BCH code, enc. pwd--->receive BCH code
  Make AES key               make AES key
                             decrypt the message and connect to the AP
  
***************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "src/procData.h"
#include "src/bleControl.h"
#include "src/bchEcc.h"
#include "src/aesAlg.h"

#define BLOCK_SIZE 16
#define FREAD_COUNT 4096

int main() {
	int status, rLen, nCor, txDur, rxDur, i;
	int *Barcode, *bch;
	char *timeFile, *rssiFile;
	char encPwd[FREAD_COUNT + BLOCK_SIZE], decPwd[FREAD_COUNT + BLOCK_SIZE]; 
	char APSSID[100] = "", APPWD[100] = "", APMAC[17]="", macAP[17]="";

	struct timespec tspec;
	uint64_t start, now;

	srand(time(NULL));

	timeFile = "time.dat";
	rssiFile = "rssi.dat";

	Barcode = malloc(sizeof(int)*127);
	bch = malloc(sizeof(int)*127);

	status = getAPInfo(APMAC, APSSID, APPWD);

	for(i=0; i<17; i++)
		macAP[i] = APMAC[i];

	clock_gettime(CLOCK_MONOTONIC, &tspec);
	start = (tspec.tv_sec)*1000 + (tspec.tv_nsec)/1000000;
	now = start;
	while(now - start < 3000) {
		status = syncStart(macAP);
		if(status < 0)
			return 0;
		
		Barcode = procData(timeFile, rssiFile);
		printf("\nBarcode : ");
		for(i=0; i<127; i++)
			printf("%d ", Barcode[i]);
		printf("\n");

		nCor = 0;
		rLen = makeBCH(Barcode, bch);
		printf("\nBCH enc.: ");
		for(i=0; i<127; i++)
			printf("%d ", bch[i]);
		printf("\n");

		status = aesEncrypt(bch, APPWD, encPwd);
		txDur = 50 + (rand()%10 - 10);
		rxDur = 100 - txDur;
		status = dataTx(macAP, bch, rLen, encPwd, txDur);
		status = startRx(rxDur);

		clock_gettime(CLOCK_MONOTONIC, &tspec);
		now = (tspec.tv_sec)*1000 + (tspec.tv_nsec)/1000000;

	}

	return 0;
}
