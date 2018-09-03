/***************************
  JWHUR, I implemented the overall BleIoT connection protocol in this code
***************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "src/procData.h"
#include "src/syncStart.h"

int main() {
	int status, i;
	int *Barcode;
	char *timeFile, *rssiFile, *macAP;

	timeFile = "time.dat";
	rssiFile = "rssi.dat";
	macAP = "ec:55:f9:12:7c:c9";

	Barcode = malloc(sizeof(int)*100);
	
	while(1) {
		status = syncStart(macAP);
		if(status < 0)
			return 0;
		
		Barcode = procData(timeFile, rssiFile);
		printf("\nBarcode: ");
		for(i=0; i<100; i++)
			printf("%d ", Barcode[i]);
		printf("\n");
		sleep(1);
	}

	return 0;
}
