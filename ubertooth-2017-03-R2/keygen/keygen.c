#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX 256
#define INTMAX 20000

float *maFilter(int *iPacket, int *rssi, int lenData, char *filename) {
	FILE *output;
	int i, j;
//	float binomialCoeff[7] = {0.0156, 0.0938, 0.2344, 0.3125, 0.2344, 0.0938, 0.0156};
	float binomialCoeff[9] = {0.0039, 0.0313, 0.1094, 0.2188, 0.2734, 0.2188, 0.1094, 0.0313, 0.0039};
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

	output = fopen(filename, "w");
	if (output == NULL) {
		printf("open failed\n");
		return 0;
	}

	for(i=0; i<lenData; i++) {
		fprintf(output, "%d %f\n", iPacket[i], rssiMA[i]);
	}
	fclose(output);
	return rssiMA;
}


int getData(char *filename, int *iPacket, int *rssi) {
	FILE *input;
	int i, j, lenData = 0, len = 0;
	char buf[MAX];

	input = fopen(filename, "r");
	if (input == NULL) {
		printf("open failed\n");
		return lenData;
	}

	while(1) {
		fgets(buf, MAX, input);
		len = strlen(buf);
		if (len == 0)
			break;
		lenData++;
		buf[0] = '\0';
	}

	fseek(input, 0, SEEK_SET);

	for (i=0; i<lenData; i++) {
		fscanf(input, "%d %d\n", &iPacket[i], &rssi[i]);
	}
	
	printf("lenData: %d\n", lenData);
	return lenData;
}

int min(int a, int b) {
	if (a < b)
		return a;
	else
		return b;
}

int main() {
	int *nP0, *rssi0, *nP1, *rssi1;
	int n0, n1, sync, end;
	int i, j;

	nP0 = (int*) malloc(sizeof(int) * INTMAX);
	rssi0 = (int*) malloc(sizeof(int) * INTMAX);
	nP1 = (int*) malloc(sizeof(int) * INTMAX);
	rssi1 = (int*) malloc(sizeof(int) * INTMAX);

	n0 = getData("log0.dat", nP0, rssi0);
	n1 = getData("log1.dat", nP1, rssi1);

	if (nP0[0] < nP1[0]) sync = nP1[0];
	else sync = nP0[0];

	if (nP0[n0] > nP1[n1]) end = nP1[n1];
	else end = nP1[n0];

	int *key0, *key1;
	key0 = (int*) malloc(sizeof(int) * min(n0,n1));
	key1 = (int*) malloc(sizeof(int) * min(n0,n1));

	float aRssi0 = 0, aRssi1 = 0, stdRssi0 = 0, stdRssi1 = 0;
	float alpha = 0.3;

	for (i=0; i<n0; i++)
		aRssi0 += rssi0[i];
	aRssi0 /= n0;
	for (i=0; i<n0; i++)
		stdRssi0 += pow(rssi0[i] - aRssi0, 2);
	stdRssi0 = sqrt(stdRssi0 / n0);

	for (i=0; i<n1; i++)
		aRssi1 += rssi1[i];
	aRssi1 /= n1;
	for (i=0; i<n1; i++)
		stdRssi1 += pow(rssi1[i] - aRssi1, 2);
	stdRssi1 = sqrt(stdRssi1 / n1);

	int lenKey = min(n0, n1);
	i = 0;
	while (nP0[i] >= sync && nP0[i] <= end) {
		if (rssi0[i] >= aRssi0 + alpha * stdRssi0)
			key0[i] = 1;
		else if (rssi0[i] < aRssi0 - alpha * stdRssi0)
			key0[i] = 0;
		else
			key0[i] = 2;
		i++;
	}
	
	i = 0;
	while (nP1[i] >= sync && nP1[i] <= end) {
		if (rssi1[i] >= aRssi1 + alpha * stdRssi1)
			key1[i] = 1;
		else if (rssi1[i] < aRssi1 - alpha * stdRssi1)
			key1[i] = 0;
		else
			key1[i] = 2;
		i++;
	}	

	i=0; j=0;
	int totalDiff = 0, nDiff = 0, nTotal = 0, eTotal = 0;
	while(i < lenKey && j < lenKey) {
		nTotal++;
		if(nP0[i] == nP1[j]) {
			eTotal++;
			printf("%d %d %d\n", nP0[i], key0[i], key1[j]);
			if (key0[i] != key1[j]) {
				nDiff++;
				totalDiff++;
			}
//			printf("%d, %d\n", rssi0[i], rssi1[i]);
			i++;
			j++;
		} else if (nP0[i] < nP1[j]) {
			printf("%d %d -1\n", nP0[i], key0[i]);
			totalDiff++;
//			printf("%d, x\n", rssi0[i]);
			i++;
		} else {
			printf("%d -1 %d\n", nP1[j], key1[j]);
			totalDiff++;
//			printf("x, %d\n", rssi1[j]);
			j++;
		}
	}

	float *rssiMA0 = (float*)malloc(sizeof(float) * n0);
	float *rssiMA1 = (float*)malloc(sizeof(float) * n1);

	rssiMA0 = maFilter(nP0, rssi0, n0, "rssiMA0.dat");
	rssiMA1 = maFilter(nP1, rssi1, n1, "rssiMA1.dat");

	printf("n0: %d, n1: %d\n", n0, n1);
	printf("aRssi0/stdRSsi0: %f/%f, aRssi1/stdRssi1: %f/%f\n", aRssi0, stdRssi0, aRssi1, stdRssi1);
	printf("Total Error: %f, matched Error: %f\n", (float)totalDiff/nTotal, (float)nDiff/eTotal);
	return 0;
}
	


