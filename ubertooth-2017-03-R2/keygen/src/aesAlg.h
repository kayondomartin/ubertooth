#include <stdio.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <string.h>
#include <stdint.h>


int aesEncrypt(int *bch, char *data, char *encData);
int aesDecrypt(int *bch, char *encData, char *data);
