#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int syncStart(char *macAP);
int dataTx(char *macAP, int *bch, int rLen, char *encPwd, int txDur);
int startRx(int rxDur);

