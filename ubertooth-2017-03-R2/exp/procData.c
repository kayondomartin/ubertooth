#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

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
	if (nCls2 != 0)
		mu[1] = (float) sumCls2/nCls2;
	
	return cost;
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
	mu[0] = rand()%100 - 100;
	mu[1] = rand()%100 - 100;
	//
	
	for(i=0; i<max_iter; i++) {
		fcost = kMeans_clustering(rssi, cls1, cls2, lenData, mu);
		cost = fcost;
	}
	threshold = (mu[0] + mu[1])/2;
	return threshold;
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

int edgeDetect(int *time, int *rssi, int *eTime, int *eRssi, int lenData, float threshold, char *oFile) {
	//Detect the peak of rssi samples
	int i, j = 0;
	int nEdge = 0;
	FILE *output;

	for (i=1; i<lenData; i++) {
		if (rssi[i-1] < threshold && rssi[i] >= threshold) {
			eRssi[nEdge] = rssi[i];
			eTime[nEdge] = time[i];
			nEdge++;
		}
	}

	output = fopen(oFile, "w");
	if (output == NULL) {
		printf("open failed\n");
		return 0;
	}
	for(i=0; i<nEdge; i++)
		fprintf(output, "%d %d\n", eTime[i], eRssi[i]);
	fclose(output);

	return nEdge;
}

int makeBarcode(int *eTime, int nEdge, int *Barcode, char *oFile) {
	FILE *output;
	int i, j;
	int index;

	for (i=0; i<nEdge; i++) {
		if (eTime[i] < 1e6) {
			index = eTime[i]/1e4;
			Barcode[index] = 1;
		}
	}

	output = fopen(oFile, "w");
	if (output == NULL) {
		printf("open failed\n");
		return 0;
	}
	for (i=0; i<100; i++)
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
int main() {
	int lenData0 = -1, lenData1 = -1;
	int nEdge0, nEdge1, nDiff = 0, i;
	int *rTime0, *rssi0, *rTime1, *rssi1;
	float thr0, thr1;
	srand(time(NULL));

	rTime0 = malloc(sizeof(int)*2000);
	rssi0 = malloc(sizeof(int)*2000);
	lenData0 = getData("time0.dat", "rssi0.dat", rTime0, rssi0);

	thr0 = kMeans(rssi0, lenData0);

	int *eRssi0 = malloc(sizeof(int)*lenData0);
	int *eTime0 = malloc(sizeof(int)*lenData0);
	nEdge0 = edgeDetect(rTime0, rssi0, eTime0, eRssi0, lenData0, thr0,"edge0.dat");

	int Barcode0[100] = {0, };
	int r = makeBarcode(eTime0, nEdge0, Barcode0, "Barcode0.dat");

	rTime1 = malloc(sizeof(int)*2000);
	rssi1 = malloc(sizeof(int)*2000);
	lenData1 = getData("time1.dat", "rssi1.dat", rTime1, rssi1);
	
	thr1 = kMeans(rssi1, lenData1);

	int *eRssi1 = malloc(sizeof(int)*lenData1);
	int *eTime1 = malloc(sizeof(int)*lenData1);
	nEdge1 = edgeDetect(rTime1, rssi1, eTime1, eRssi1, lenData1, thr1,"edge1.dat");

	int Barcode1[100] = {0, };
	r = makeBarcode(eTime1, nEdge1, Barcode1, "Barcode1.dat");

	for(i=0; i<100; i++) {
		if(Barcode0[i] != Barcode1[i])
			nDiff++;
	}
	printf("The number of different bits: %d\n", nDiff);
	return 0;
}
