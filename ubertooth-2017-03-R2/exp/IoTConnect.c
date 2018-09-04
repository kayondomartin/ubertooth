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

#include "src/procData.h"
#include "src/syncStart.h"
#include "src/bchEnc.h"

int main() {
	int status, i, nCor;
	int *Barcode, *bch;
	char *timeFile, *rssiFile, *macAP;

	timeFile = "time.dat";
	rssiFile = "rssi.dat";
	macAP = "ec:55:f9:12:7c:c9";

	Barcode = malloc(sizeof(int)*127);
	bch = malloc(sizeof(int)*127);
	
	while(1) {
		status = syncStart(macAP);
		if(status < 0)
			return 0;
		
		Barcode = procData(timeFile, rssiFile);
		printf("\nBarcode: ");
		for(i=0; i<127; i++)
			printf("%d ", Barcode[i]);
		printf("\n");

		nCor = 0;
		status = makeBCH(Barcode, bch, nCor);
		printf("\nBCH encoded Barcode: ");
		for(i=0; i<127; i++)
			printf("%d ", bch[i]);
		printf("\n");

		sleep(1);
	}

	return 0;
}
