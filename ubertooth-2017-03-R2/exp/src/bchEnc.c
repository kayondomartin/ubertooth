// BCH encoder for m = 7 //

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bchEnc.h"

/*****
nCor : correction capability
nCor == 0 -> n = 127, k = 127
nCor == 2 -> n = 127, k = 113
nCor == 4 -> n = 127, k = 99
*****/

int makeBCH(int *data, int *bch) {
	int rLen, i, j, k, n = 127, genLen, nCor = 0;
	int nBit0 = 0, nBit2 = 0;
	int corThr0 = 4, corThr1 = 8;
	int primPol[8] = {1, 1, 0, 0, 0, 0, 0, 1};
	int *temp;

	temp = malloc(sizeof(int)*127);
	for(i=0; i<127; i++) temp[i] = data[i];

	for(i=0; i<127; i++)
		nBit0 += data[i];
	for(i=0; i<113; i++)
		nBit2 += data[i];

	// The value of threshold need to be considered
	if (nBit0 < corThr0)
		nCor = 0;
	else if (nBit2 >= corThr0 && nBit2 < corThr1)
		nCor = 2;
	else 
		nCor = 4;

	if(nCor == 0) {
		for(i=0; i<127; i++) 
			bch[i] = data[i];
		rLen = 0;
		return rLen;
	} else if (nCor == 2) {
		int genPol[15] = {1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1};
		int parPol[14] = {0, };
		genLen = 15;
		k = 113;
		
		for(i=k; i<n; i++) temp[i] = 0;
		for(i=0; i<k; i++) {
		if(temp[i] == 1) {
			for(j=0; j<genLen; j++)
				temp[j+i] = temp[j+i]^genPol[j];
			}
		}
		rLen = 14;
	} else if (nCor == 4) {
		int genPol[29] = {1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1};
		int parPil[28] = {0, };
		genLen = 29;
		k = 99;
		for(i=k; i<n; i++) temp[i] = 0;
		for(i=0; i<k; i++) {
		if(temp[i] == 1) {
			for(j=0; j<genLen; j++)
				temp[j+i] = temp[j+i]^genPol[j];
			}
		}
		rLen = 28;
	}

	for(i=0; i<k; i++) bch[i] = data[i];
	for(i=k; i<n; i++) bch[i] = temp[i];

	return rLen;
}
