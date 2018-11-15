#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

float kMeans_clustering(int *rssi, int *cls1, int *cls2, int lenData, float *mu);
void quickSort(int *cls, int left, int right);
int findMedian(int *rssi, int lenData, int thr);
float kMeans(int *rssi, int lenData);
float *maFilter(int *rssi, int lenData);
int signalDetect(int *time, int *rssi, int *eTime, int *eRssi, int lenData, float threshold, char *oFile);
int avgPower(int *time, int *rssi, int lenData, float *avgP, char *oFile);
int makeBarcode(int *eTime, int *eRssi, int nEdge, int *Barcode, char *oFile);
int getData(char *tFile, char*rFile, int *time, int *rssi);
int *procData(char *timeFile, char *rssiFile);
int getAPInfo(char *APMAC, char *APSSID, char *APPWD);
int getBCHdata(char *APMAC, uint8_t *data);
