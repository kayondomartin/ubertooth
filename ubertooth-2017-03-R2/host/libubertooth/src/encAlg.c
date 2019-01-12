#include <stdio.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <string.h>
#include <stdint.h>

#include "encAlg.h"

#define BLOCK_SIZE 16
#define FREAD_COUNT 4096
#define KEY_BIT 128
#define IV_SIZE 16
#define RW_SIZE 1
#define SUCC 0
#define FAIL -1

AES_KEY aes_ks3;

int aesEncrypt(int8_t *guestRssi, char *data, char *encData) {
	int i =0, j = 0, len = 0, padding_len = 0;
	unsigned char iv[IV_SIZE];
	unsigned char key16[16] = ""; // 128 AES encryption

	len = strlen(data) + 1;

	for(i=0; i<16; i++) {
		key16[i] = (unsigned char) guestRssi[i];
	}

//	printf("data: %s", data);
//	printf("\nkey16: ");
//	for(i=0; i<16; i++)
//		printf("%02x", key16[i]);
//	printf("\n");

	memset(iv, 0, sizeof(iv));
	AES_set_encrypt_key(key16, KEY_BIT, &aes_ks3);

	padding_len = BLOCK_SIZE - len % BLOCK_SIZE;
	memset(encData+len, padding_len, padding_len);
	AES_cbc_encrypt(data, encData, len+padding_len, &aes_ks3, iv, AES_ENCRYPT);

	return SUCC;
}

int aesDecrypt(int8_t *hostRssi, char *encData, char *data) {
	int i, j, len = 0, total_size = 0, save_len = 0, w_len = 0;
	unsigned char iv[IV_SIZE];
	unsigned char key16[16] = "";

	len = strlen(encData) + 1;

	for(i=0; i<16; i++)
		key16[i] = hostRssi[i];

//	printf("enc data: %s", encData);
//	printf("\ndec key16: ");
//	for(i=0; i<16; i++)
//		printf("%02x", key16[i]);
//	printf("\n");

	memset(iv, 0, sizeof(iv));
	AES_set_decrypt_key(key16, KEY_BIT, &aes_ks3);
	AES_cbc_encrypt(encData, data, len, &aes_ks3, iv, AES_DECRYPT);

//	printf("dec data: %s\n", data);

	return SUCC;
}

int createRsaKey(unsigned char *privateKey, unsigned char *publicKey) {
	FILE *rsaKey;
	int keyLen = 0, count;

	system("openssl genrsa -out private.pem 1024");
	system("openssl rsa -in private.pem -out public.pem -outform PEM -pubout");

	rsaKey = fopen("private.pem", "r");
	if (rsaKey == NULL) {
		printf("open failed\n");
		return 0;
	}

	fseek(rsaKey, 0, SEEK_END);
	keyLen = ftell(rsaKey);

	memset(privateKey, 0, sizeof(unsigned char) * keyLen + 1);
	fseek(rsaKey, 0, SEEK_SET);
	count = fread(privateKey, keyLen, 1, rsaKey);
	fclose(rsaKey);

	rsaKey = fopen("public.pem", "r");
	if (rsaKey == NULL) {
		printf("open failed\n");
		return 0;
	}

	fseek(rsaKey, 0, SEEK_END);
	keyLen = ftell(rsaKey);

	memset(publicKey, 0, sizeof(unsigned char) * keyLen + 1);
	fseek(rsaKey, 0, SEEK_SET);
	count = fread(publicKey, keyLen, 1, rsaKey);
	fclose(rsaKey);

	system("rm private.pem; rm public.pem");

	return keyLen;
}

RSA *createRSA(unsigned char *key, int public) {
	RSA *rsa = NULL;
	BIO *keybio;
	keybio = BIO_new_mem_buf(key, -1);
	if (keybio == NULL) {
		printf("Failed to create key BIO");
		return 0;
	}
	if (public) {
		rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
	} else {
		rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
	}

	return rsa;
}

int public_encrypt(unsigned char *data, int data_len, unsigned char *key, unsigned char *encrypted) {
	RSA *rsa = createRSA(key, 1);
	int result = RSA_public_encrypt(data_len, data, encrypted, rsa, RSA_PKCS1_PADDING);
	return result;
}

int private_decrypt(unsigned char *enc_data, int data_len, unsigned char *key, unsigned char *decrypted) {
	RSA *rsa = createRSA(key, 0);
	int result = RSA_private_decrypt(data_len, enc_data, decrypted, rsa, RSA_PKCS1_PADDING);
	return result;
}
