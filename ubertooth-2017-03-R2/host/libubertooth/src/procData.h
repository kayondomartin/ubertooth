#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

float kMeans_clustering(int8_t *rssi, int *cls1, int *cls2, int lenData, float *mu);
float kMeans(int8_t *rssi, int lenData);
int8_t *maFilter(int *sTime, int8_t *sRssi, int nSignal);
int signalDetect(int *time, int8_t *rssi, int *sTime, int8_t *sRssi, int lenData, float threshold, char *oFile);
int makeBarcode(int *eTime, int *eRssi, int nEdge, int *Barcode, char *oFile);
int getData(char *tFile, char*rFile, int *time, int *rssi);
int8_t *procData(int *rTime, int8_t *rssi, int lenData);
int getAPInfo(char *APMAC, char *APSSID, char *APPWD);
int startAPTx();
int getBCHdata(char *APMAC, uint8_t *data);
float getCorr(int8_t *rssi0, int8_t *rssi1, int lenData);
