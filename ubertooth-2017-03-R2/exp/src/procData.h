#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

float kMeans_clustering(int *rssi, int *cls1, int *cls2, int lenData, float *mu);
float kMeans(int *rssi, int lenData);
float *maFilter(int *rssi, int lenData);
int signalDetect(int *time, int *rssi, int *eTime, int *eRssi, int lenData, float threshold, char *oFile);
int makeBarcode(int *eTime, int nEdge, int *Barcode, char *oFile);
int getData(char *tFile, char*rFile, int *time, int *rssi);
int *procData(char *timeFile, char *rssiFile);
int getAPInfo(char *APMAC, char *APSSID, char *APPWD);
int getBCHdata(char *APMAC, uint8_t *data);
