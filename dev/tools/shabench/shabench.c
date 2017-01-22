/**
 * @file /shabench/shabench.c
 *
 * @note gcc -Iopenssl/ -lcrypto -lssl -std=gnu99 -O3 -o shabench shabench.c
 *
 */

#include "shabench.h"

int main() {

	size_t len;
	char t[1024];
	time_t moment;
	unsigned char md[SHA512_DIGEST_LENGTH + sizeof(uint64_t)];
	struct tm localtime;

	SSL_library_init();
	SSL_load_error_strings();

	memset(&localtime, 0, sizeof(struct tm));

	if ((moment = time(NULL)) == ((time_t) -1) || !localtime_r(&moment, &localtime)) {
		return -1;
	}

	else if ((len = strftime(t, 1024, "%Y-%m-%d %T %Z", &localtime)) <= 0) {
		return -1;
	}

	printf("Start Time: %s\n", t);

	memset(md, 0, SHA512_DIGEST_LENGTH + sizeof(uint64_t));

	for (uint64_t i = 0; i < UINT32_MAX; i++) {
		if (i % 0x00FFFFFFL == 0 && i != 0) printf("%u (%u/255)iterations completed...\n", i, i/0x00FFFFFFL);
		memcpy(md + SHA512_DIGEST_LENGTH, &i, sizeof(uint64_t));
		SHA512(md, SHA512_DIGEST_LENGTH + sizeof(uint64_t), md);
	}

	memset(&localtime, 0, sizeof(struct tm));

	if ((moment = time(NULL)) == ((time_t) -1) || !localtime_r(&moment, &localtime)) {
		return -1;
	}

	else if ((len = strftime(t, 1024, "%Y-%m-%d %T %Z", &localtime)) <= 0) {
		return -1;
	}

	printf("End Time: %s\n", t);

	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	ERR_free_strings();
	ERR_remove_thread_state(NULL);
	sk_pop_free((_STACK *)SSL_COMP_get_compression_methods(), CRYPTO_free);

	return 0;
}
