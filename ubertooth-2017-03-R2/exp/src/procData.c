#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

#include "procData.h"

#define FLT_MAX 3.402823466e+38F

float kMeans_clustering(int *rssi, int *cls1, int *cls2, int lenData, float *mu) {
	float threshold = 0, cost = 0;
	int i, nCls1 = 0, nCls2 = 0, sumCls1 = 0, sumCls2 = 0;
	threshold = (mu[0] + mu[1]) / 2;

	for(i=0; i<lenData; i++) {
		if (rssi[i] < threshold) {
			cls1[nCls1] = rssi[i];
			sumCls1 += rssi[i];
			cost += abs(rssi[i] - mu[0]);
			nCls1++;
		} else {
			cls2[nCls2] = rssi[i];
			sumCls2 += rssi[i];
			cost += abs(rssi[i] - mu[1]);
			nCls2++;
		}
	}

	if (nCls1 != 0)
		mu[0] = (float) sumCls1/nCls1;
	else
		mu[0] = mu[1] - 1;
	if (nCls2 != 0)
		mu[1] = (float) sumCls2/nCls2;
	else
		mu[1] = mu[0] + 1;

	return cost;
}

void quickSort(int *cls, int left, int right) {
	int L = left, R = right;
	int temp;
	int pivot = cls[(left + right)/2];

	while (L <= R) {
		while (cls[L] < pivot) L++;
		while (cls[R] > pivot) R--;

		if (L <= R) {
			if (L != R) {
				temp = cls[L];
				cls[L] = cls[R];
				cls[R] = temp;
			}
			L++; R--;
		}
	}

	if (left < R) quickSort(cls, left, R);
	if (L < right) quickSort(cls, L, right);
}

int findMedian(int *rssi, int lenData, int thr) {
	int i = 0, nCls = 0, j = 0;
	int *cls;

	for (i=0; i<lenData; i++) {
		if (rssi[i] >= thr) nCls++;
	}

	cls = malloc(sizeof(int)*nCls);
	for (i=0; i<lenData; i++) {
		if (rssi[i] >= thr)	cls[j++] = rssi[i];
	}

	// quick sort
	quickSort(cls, 0, nCls-1);
	int index = nCls / 2;
	int median = cls[index];

	// print
	printf("cls : ");
	for(i=0; i<nCls; i++)
		printf("%d ", cls[i]);
	printf("\n");

	return median;
}


	

float kMeans(int *rssi, int lenData) {
	int i, max_iter = 100, min_iter = 50;
	int *cls1, *cls2;
	float *mu;
	float cost = FLT_MAX, fcost = FLT_MAX, threshold;

	cls1 = malloc(sizeof(int)*lenData);
	cls2 = malloc(sizeof(int)*lenData);
	mu = malloc(sizeof(float)*2);

	//random initialization of mu
	mu[0] = 0;
	for (i=0; i<lenData; i++)
		mu[0] += rssi[i];
	mu[0] /= lenData;
	mu[1] = mu[0] + 1;
	//
	
	for(i=0; i<max_iter; i++) {
		fcost = kMeans_clustering(rssi, cls1, cls2, lenData, mu);
		cost = fcost;
	}
	threshold = (mu[0] + mu[1])/2;

	free(cls1); free(cls2); free(mu);

	if (threshold > -80)
		return threshold;
	else 
		return -80;
}

float *maFilter(int *rssi, int lenData) {
	// Moving average filtering
	FILE *output;
	int i, j;
	float binomialCoeff[7] = {0.0156, 0.0938, 0.2344, 0.3125, 0.2344, 0.0938, 0.0156};
	int cLen = sizeof(binomialCoeff)/sizeof(float);
	float *rssiMA = malloc(sizeof(float)*lenData);
	
	for(i=0; i<lenData; i++) {
		rssiMA[i] = 0;
		if (i >= cLen-1) {
			for(j=0; j<cLen; j++)
				rssiMA[i] = rssiMA[i] + (float)rssi[i-cLen+j] * binomialCoeff[j];
		} else {
			for(j=0; j<i+1; j++)
				rssiMA[i] = rssiMA[i] + (float)rssi[j] * binomialCoeff[j];
		}
	}
		
	// Save Moving averaged data to rssiMA.dat
	output = fopen("rssiMA.dat", "w");
	if (output == NULL)	{
		printf("open failed\n");
		return 0;
	}

	for(i=0; i<lenData; i++) {
		fprintf(output, "%f\n", rssiMA[i]);
	}
	fclose(output);	
	return rssiMA;
}

int signalDetect(int *time, int *rssi, int *eTime, int *eRssi, int lenData, float threshold, char *oFile) {
	//Detect the samples above the threshold value
	int i, j = 0;
	int nEdge = 0;
	FILE *output;

	for (i=0; i<lenData; i++) {
		if (rssi[i] >= threshold) {
//		if (rssi[i] >= -80) {
			eRssi[nEdge] = rssi[i];
			eTime[nEdge] = time[i];
			nEdge++;
		}
	}

/*
	output = fopen(oFile, "w");
	if (output == NULL) {
		printf("open failed\n");
		return 0;
	}
	for(i=0; i<nEdge; i++)
		fprintf(output, "%d %d\n", eTime[i], eRssi[i]);
	fclose(output);
*/

	return nEdge;
}

int avgPower(int *eTime, int *eRssi, int nEdge, float *avgP, char *oFile) {
	FILE *output;
	int i, j;
	int index, *nSamples;

	nSamples = malloc(sizeof(int)*26);
	for (i=0; i<26; i++) {
		nSamples[i] = 0;
		avgP[i] = 0;
	}

	for (i=0; i<nEdge; i++) {
		if (eTime[i] <1270000) {
			index = eTime[i]/5e4;
			avgP[index] += pow(10, (float)(eRssi[i]/10));
			nSamples[index] += 1;
		}
	}

	for (i=0; i<26; i++) {
		if (nSamples[i] != 0) avgP[i] = 10 * log10(avgP[i] / nSamples[i]);
		else avgP[i] = -90;
	}

	free(nSamples);

	output = fopen(oFile, "w");
	if (output == NULL) {
		printf("open failed\n");
		return 0;
	}

	for (i=0; i<26; i++)
		fprintf(output, "%f\n", avgP[i]);
	return 1;
}


int makeBarcode(int *eTime, int *eRssi, int nEdge, int *Barcode, char *oFile) {
	FILE *output;
	int i, j;
	int index;

	for (i=0; i<nEdge; i++) {
		if (eTime[i] < 1270000) {
			index = eTime[i]/1e4;
			Barcode[index] = 1;
		}
	}

	output = fopen(oFile, "w");
	if (output == NULL) {
		printf("open failed\n");
		return 0;
	}

	for (i=0; i<127; i++)
		fprintf(output, "%d\n", Barcode[i]);
	return 1;
}

int getData(char *tFile, char*rFile, int *time, int *rssi) {
	FILE *input, *output;
	int i, j, t, lenData = -1;
	int time0 = 0;

	input = fopen(tFile, "r");
	if (input == NULL) {
		printf("open failed\n");
		return 0;
	}

	while(fgetc(input) != EOF) {
		lenData++;
		fscanf(input, "%d", &t);
	}
	fseek(input, 0, SEEK_SET);

	for(i=0; i<lenData; i++) {
		fscanf(input, "%d", &t);
		time[i] = t;
		if (i == 0)
			time0 = time[0];
		time[i] -= time0;
	}

	fclose(input);
	output = fopen(tFile, "w");
	if (output == NULL) {
		printf("open failed\n");
	}

	for(i=0; i<lenData; i++) {
		fprintf(output, "%d\n", time[i]);
	}
	fclose(output);

	input = fopen(rFile, "r");
	if (input == NULL) {
		printf("open failed\n");
	}

	for(i=0; i<lenData; i++) {
		fscanf(input, "%d", &t);
		rssi[i] = t;
	}
	fclose(input);

	return lenData;
}

int *procData(char *timeFile, char *rssiFile) {
	int lenData = -1;
	int nEdge, nDiff = 0, i;
	int *rTime, *rssi;
	float thr;
	srand(time(NULL));

	rTime = malloc(sizeof(int)*2540);
	rssi = malloc(sizeof(int)*2540);
	lenData = getData(timeFile, rssiFile, rTime, rssi);

	thr = kMeans(rssi, lenData);
	// find median of samples that is over threshold derived from kmeans
//	int median = findMedian(rssi, lenData, thr);
//	printf("Median value is %d\n", median);
	//


	int *eRssi = malloc(sizeof(int)*lenData);
	int *eTime = malloc(sizeof(int)*lenData);
	nEdge = signalDetect(rTime, rssi, eTime, eRssi, lenData, thr,"signal.dat");

	int *Barcode;
	Barcode = malloc(sizeof(int)*127);
	for(i=0; i<127; i++)
		Barcode[i] = 0;
//	int r = makeBarcode(eTime, eRssi, nEdge, Barcode, "Barcode.dat");
	float *avgP = malloc(sizeof(float)*26);
	int r = avgPower(eTime, eRssi, nEdge, avgP, "Barcode.dat");

	free(rTime); free(rssi); free(eRssi); free(eTime);
	return Barcode;
}

int getAPInfo(char *APMAC, char *APSSID, char *APPWD) {
	char line[10]="";
	FILE *apSSID = popen("nm-tool | grep '*' > ap; conAP=$(sed -n '2p' < ap); APtemp=$(echo $conAP | awk '{print $1}'); APSSID=${APtemp#'*'}; APSSID=${APSSID%':'}; echo $APSSID; rm ap", "r");
	if(apSSID != NULL) {
		while(fgets(line, sizeof(10), apSSID) != NULL)
			strcat(APSSID, line);
		printf("AP ssid: %s", APSSID);
	}

	char ap[100]= "";
	int i, len;
	len = strlen(APSSID);
	for(i=0; i<len-1; i++)
		ap[i] = APSSID[i];

	char command[500] = "PWDtemp=$(sudo cat /etc/NetworkManager/system-connections/";
	strcat(command, ap);
	strcat(command, " | grep psk=); pwd=${PWDtemp#'psk='}; echo $pwd");
	FILE *apPWD = popen(command, "r");
	if(apPWD != NULL) {
		while(fgets(line, sizeof(10), apPWD) != NULL)
			strcat(APPWD, line);
		printf("AP password: %s", APPWD);
	}

	char command1[500] = "MACtemp=$(sudo cat /etc/NetworkManager/system-connections/";
	strcat(command1, ap);
	strcat(command1, " | grep mac-address=); MAC=${MACtemp#'mac-address='}; echo $MAC");
	FILE *apMAC = popen(command1, "r");
	if(apMAC != NULL) {
		while(fgets(line, sizeof(10), apMAC) != NULL)
			strcat(APMAC, line);
		printf("AP mac address: %s", APMAC);
	}
	pclose(apSSID); pclose(apPWD); pclose(apMAC);
}


int getBCHdata(char *APMAC, uint8_t *data) {
	FILE *dataFile;
	int status, len, i;
	char dd[300] = "", line[10], command[200] = "cat log | grep '", blePrefix[120] = "Data:  ", bleMac[100];
	uint8_t APMAC2Byte[6];

	if(strlen(APMAC) != 6*2+5) {
		printf("Error: MAC address is wrong length\n");
		return 0;
	}

	for (i = 0; i < 6*3; i += 3) {
		if (!isxdigit(APMAC[i]) ||
			!isxdigit(APMAC[i+1])) {
			printf("Error: MAC address contains invalid character(s)\n");
			return 0;
		}
		if (i < 5*3 && APMAC[i+2] != ':') {
			printf("Error: MAC address contains invalid character(s)\n");
			return 0;
		}
	}

	// sanity: checked; convert
	for (i = 0; i < 6; ++i) {
		unsigned byte;
		sscanf(&APMAC[i*3], "%02x",&byte);
		APMAC2Byte[i] = byte;
	}

	sprintf(bleMac, "%02x %02x %02x %02x %02x %02x", APMAC2Byte[5], APMAC2Byte[4], APMAC2Byte[3], APMAC2Byte[2], APMAC2Byte[1], APMAC2Byte[0]);
	printf("bleMac: %s\n", bleMac);

	strcat(blePrefix, bleMac);
	strcat(command, blePrefix);
	strcat(command, "'");
	dataFile = popen(command, "r");
	if(dataFile != NULL) {
		while(fgets(line, sizeof(10), dataFile) != NULL)
			strcat(dd, line);
		printf("%s", dd);
	}
	len = strlen(dd);
	for(i=0; i<len-62; i+=3) {
		unsigned byte;
		sscanf(&dd[62 + i], "%02x", &byte);
		data[i/3] = (uint8_t)byte; 
	}
	return 1;
}



