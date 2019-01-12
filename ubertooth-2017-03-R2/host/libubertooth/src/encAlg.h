#include <stdio.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <string.h>
#include <stdint.h>


int aesEncrypt(int8_t *guestRssi, char *data, char *encData);
int aesDecrypt(int8_t *hostRssi, char *encData, char *data);
int createRsaKey(unsigned char *privateKey, unsigned char *publicKey);
RSA *createRSA(unsigned char *key, int public);
int public_encrypt(unsigned char *data, int data_len, unsigned char *key, unsigned char *encrypted);
int private_decrypt(unsigned char *enc_data, int data_len, unsigned char *key, unsigned char *decrypted);
