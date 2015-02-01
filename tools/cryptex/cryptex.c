/**
 * @file /cryptex/example.c
 *
 * @brief Example code for using/testing/benchmarking the provided ECIES code to encrypt/decrypt data.
 *
 * $Author: Ladar Levison $
 * $Website: http://lavabit.com $
 * $Date: 2010/08/11 23:45:52 $
 * $Revision: 63466342d54a6106a3eb4361aece44458a018a49 $
 *
 */

#include "cryptex.h"

void processor_cleanup(EC_KEY *key, secure_t *ciphered, char *hex_pub, char *hex_priv, unsigned char *text, unsigned char *copy, unsigned char *original) {

	if (key) {
		ecies_key_free(key);
	}

	if (ciphered) {
		secure_free(ciphered);
	}

	if (hex_pub) {
		OPENSSL_free(hex_pub);
	}

	if (hex_priv) {
		OPENSSL_free(hex_priv);
	}

	if (text) {
		free(text);
	}

	if (copy) {
		free(copy);
	}

	if (original) {
		free(original);
	}

	return;
}

typedef char make_sure_rand_max_is_large_enough[(RAND_MAX < 10LL << 20) ? -1 : 1];

int processor(int iteration) {

	int tlen;
	size_t olen;
	EC_KEY *key = NULL;
	secure_t *ciphered = NULL;
	char *hex_pub = NULL, *hex_priv = NULL;
	unsigned char *text = NULL, *copy = NULL, *original = NULL;

	// Generate random size for the block of data were going to encrypt. Use a min value of 1 MB and a max of 10 MB.
	do {
		tlen = (rand() % (1024 * 1024 * 10));
	} while (tlen < (1024 * 1024));

	if (!(text = malloc(tlen + 1)) || !(copy = malloc(tlen + 1))) {
		printf("Memory error.\n");
		processor_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
		return -1;
	}

	// Wipe and then fill the data blocks with random data.
	memset(copy, 0, tlen + 1);
	memset(text, 0, tlen + 1);
	for (uint64_t j = 0; j < tlen; j++) {
		copy[j] = text[j] = rand() % 256;
	}

	// Generate a key for our theoretical user.
	if (!(key = ecies_key_create())) {
		printf("Key creation failed.\n");
		processor_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
		return -1;
	}

	// Since we'll store the keys as hex values in reali life, extract the appropriate hex values and release the original key structure.
	if (!(hex_pub = ecies_key_public_get_hex(key)) || !(hex_priv = ecies_key_private_get_hex(key))) {
		printf("Serialization of the key to a pair of hex strings failed.\n");
		processor_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
		return -1;
	}

	if (!(ciphered = ecies_encrypt(hex_pub, text, tlen))) {
		printf("The encryption process failed!\n");
		processor_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
		return -1;
	}

	if (!(original = ecies_decrypt(hex_priv, ciphered, &olen))) {
		printf("The decryption process failed!\n");
		processor_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
		return -1;
	}

	if (olen != tlen || memcmp(original, copy, tlen)) {
		printf("Comparison failure.\n");
		processor_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
		return -1;
	}

	processor_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
	printf(" ... %i ... %i\n", iteration + 1, tlen);

	return 0;
}

void main_cleanup(void) {

	ecies_group_free();

	// As a child I was taught that your done eating until your plate is completely clean.
	// The following should release _all_ of the memory allocated by the OpenSSL functions used.
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	ERR_free_strings();
	ERR_remove_thread_state(NULL);
	sk_pop_free((_STACK *)SSL_COMP_get_compression_methods(), CRYPTO_free);

	return;
}

int main() {

	SSL_library_init();
	SSL_load_error_strings();

	// Initializing the group once up front cut execution time in half! However the code should function without a reusable group.
	ecies_group_init();

	// Comment this line out if you want the program to execute consistently each time.
	srand(time(NULL));

	for (uint64_t i = 0; i < 1000; i++) {
		if (processor(i)) {
			main_cleanup();
			return 1;
		}
	}

	printf("Finished.\n");
	main_cleanup();

	return 0;
}
