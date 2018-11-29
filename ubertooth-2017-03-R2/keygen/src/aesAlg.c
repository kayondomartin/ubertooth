#include <stdio.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <string.h>
#include <stdint.h>

#include "aesAlg.h"

#define BLOCK_SIZE 16
#define FREAD_COUNT 4096
#define KEY_BIT 128
#define IV_SIZE 16
#define RW_SIZE 1
#define SUCC 0
#define FAIL -1

AES_KEY aes_ks3;

int aesEncrypt(int *bch, char *data, char *encData) {
	int i =0, j = 0, len = 0, padding_len = 0;
	int bchPad[128] = {0, };
	unsigned char iv[IV_SIZE];
	uint8_t bch2char;
	unsigned char key16[16] = "";

	len = strlen(data) + 1;

	for(i=0; i<127; i++)
		bchPad[i] = bch[i];
	bchPad[127] = 1;

	for(i=0; i<16; i++) {
		for(j=0; j<8; j++) {
			bch2char <<= 1;
			bch2char |= bchPad[i*8 + j];
		}
		key16[i] = bch2char;
		bch2char = 0;
	}

	printf("data: %s", data);
	printf("\nkey16: ");
	for(i=0; i<16; i++)
		printf("%02x", key16[i]);
	printf("\n");

	memset(iv, 0, sizeof(iv));
	AES_set_encrypt_key(key16, KEY_BIT, &aes_ks3);

	padding_len = BLOCK_SIZE - len % BLOCK_SIZE;
	memset(encData+len, padding_len, padding_len);
	AES_cbc_encrypt(data, encData, len+padding_len, &aes_ks3, iv, AES_ENCRYPT);

	return SUCC;
}

int aesDecrypt(int *bch, char *encData, char *data) {
	int i, j, len = 0, total_size = 0, save_len = 0, w_len = 0;
	unsigned char iv[IV_SIZE];
	int bchPad[128] = {0, };
	uint8_t bch2char;
	unsigned char key16[16] = "";

	len = strlen(encData) + 1;

	for(i=0; i<127; i++)
		bchPad[i] = bch[i];
	bchPad[127] = 1;

	for(i=0; i<16; i++) {
		for(j=0; j<8; j++) {
			bch2char <<= 1;
			bch2char |= bchPad[i*8 + j];
		}
		key16[i] = bch2char;
		bch2char = 0;
	}

	printf("enc data: %s", encData);
	printf("\ndec key16: ");
	for(i=0; i<16; i++)
		printf("%02x", key16[i]);
	printf("\n");

	memset(iv, 0, sizeof(iv));
	AES_set_decrypt_key(key16, KEY_BIT, &aes_ks3);
	AES_cbc_encrypt(encData, data, len, &aes_ks3, iv, AES_DECRYPT);

	printf("dec data: %s\n", data);

	return SUCC;
}
	

