/**
 * @file /cryptex/ecies.h
 *
 * @brief ECIES module functions.
 *
 * $Author: Ladar Levison $
 * $Website: http://lavabit.com $
 * $Date: 2010/08/18 20:23:20 $
 * $Revision: 3484fa2110fb03859e4d31f8278578dac79d9d91 $
 *
 */

#ifndef LAVABIT_CRYPTEX_H
#define LAVABIT_CRYPTEX_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/stack.h>

#define ECIES_CURVE NID_secp521r1
#define ECIES_CIPHER EVP_aes_256_cbc()
#define ECIES_HASHER EVP_sha512()

typedef struct {

	struct {
		uint64_t key;
		uint64_t mac;
		uint64_t orig;
		uint64_t body;
	} length;

} __attribute__ ((packed)) secure_head_t;

typedef char * secure_t;

void secure_free(secure_t *cryptex);
void * secure_key_data(secure_t *cryptex);
void * secure_mac_data(secure_t *cryptex);
void * secure_body_data(secure_t *cryptex);
uint64_t secure_key_length(secure_t *cryptex);
uint64_t secure_mac_length(secure_t *cryptex);
uint64_t secure_body_length(secure_t *cryptex);
uint64_t secure_orig_length(secure_t *cryptex);
uint64_t secure_total_length(secure_t *cryptex);
void * secure_alloc(uint64_t key, uint64_t mac, uint64_t orig, uint64_t body);

void ecies_group_init(void);
void ecies_group_free(void);
EC_GROUP * ecies_group(void);

void ecies_key_free(EC_KEY *key);

EC_KEY * ecies_key_create(void);
EC_KEY * ecies_key_create_public_hex(char *hex);
EC_KEY * ecies_key_create_private_hex(char *hex);
EC_KEY * ecies_key_create_public_octets(unsigned char *octets, size_t length);

char * ecies_key_public_get_hex(EC_KEY *key);
char * ecies_key_private_get_hex(EC_KEY *key);

secure_t * ecies_encrypt(char *key, unsigned char *data, size_t length);
unsigned char * ecies_decrypt(char *key, secure_t *cryptex, size_t *length);

#endif
