#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

#include "src/procData.h"
#include "src/bleControl.h"
#include "src/bchEcc.h"
#include "src/aesAlg.h"

int main() {
	int status, rxDur, txDur, i;
	int *Barcode, *bch;
	uint8_t data[200];
	char *timeFile, *rssiFile;
	char APSSID[100] = "", APPWD[100] = "", APMAC[17] = "", apMAC[17] = "";

	struct timespec tspec;
	uint64_t start, now;

	timeFile = "time.dat";
	rssiFile = "rssi.dat";

	rxDur = 3000;
//	status = startRx(rxDur);
	status = syncReceived(apMAC);
	if(status == 1) {
		for(i=0; i<17; i++)
			APMAC[i] = apMAC[i];
		Barcode = procData(timeFile, rssiFile);
		printf("\nBarcode : ");
		for(i=0; i<127; i++)
			printf("%d ", Barcode[i]);
		printf("\n");

		rxDur = 100;
		status = getBCHdata(APMAC, data);
		//status = startRx(rxDur);

	}

	return 0;
}
