
/**
 * @file /magma.check/providers/digest_check.c
 *
 * @brief The logic used to test the digest ciphers.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

bool_t check_hash_simple(void) {

	digest_t *digest;
	stringer_t *hash, *hex;
	chr_t *digest_list[] = {
		"MD4", "MD5", "SHA", "SHA1", "SHA224", "SHA256", "SHA384", "SHA512", "RIPEMD160"
	}, *result_list[] = {
		"66f1f59819d52476f328839e34101d0f",
		"7215ee9c7d9dc229d2921a40e899ec5f",
		"bce965d4e985f27d988262331c0427909417cd90",
		"b858cb282617fb0956d960215c8e84d1ccf909c6",
		"ca17734c016e36b898af29c1aeb142e774abf4b70bac55ec98a27ba8",
		"36a9e7f1c95b82ffb99743e0c5c4ce95d83c9a430aac59f84ef3cbfab6145068",
		"588016eb10045dd85834d67d187d6b97858f38c58c690320c4a64e0c2f92eebd9f1bd74de256e8268815905159449566",
		"f90ddd77e400dfe6a3fcf479b00b1ee29e7015c5bb8cd70f5f15b4886cc339275ff553fc8a053f8ddc7324f45168cffaf81f8c3ac93996f6536eef38e5e40768",
		"ac53a3aea6835b5ec12054e12d41d392e9d57b72"
	};

	// Loop through and hash a single space using each digest method. Then check whether we got what we expected.
	for (uint64_t i = 0; status() && i < (sizeof(digest_list) / sizeof(chr_t *)); i++) {

		if (!(digest = hash_name(NULLER(digest_list[i]))) || !(hash = hash_digest(digest, PLACER(" ", 1), NULL))) {
			return false;
		}
		else if (!(hex = hex_encode_st(hash, NULL))) {
			st_free(hash);
			return false;
		}
		else if (st_cmp_cs_eq(hex, NULLER(result_list[i]) )) {
			st_free(hash);
			st_free(hex);
			return false;
		}

		st_free(hash);
		st_free(hex);
	}

	return true;
}

bool_t check_hash_sthread(chr_t *name) {

	stringer_t *hash;
	digest_t *digest;
	byte_t buffer[DIGEST_CHECK_SIZE];

	for (uint64_t i = 0; status() && i < DIGEST_CHECK_ITERATIONS; i++) {

		// Fill the buffer with random data and convert the buffer to encrypted.
		if (rand_write(PLACER(buffer, DIGEST_CHECK_SIZE)) != DIGEST_CHECK_SIZE) {
			return false;
		}
		//else if (!(digest = digest_name(NULLER(name))) || !(hash = hash_digest(digest, PLACER(buffer, DIGEST_CHECK_SIZE), NULL))) {
		else if (!(digest = hash_name(NULLER(name))) || !(hash = hash_digest(digest, PLACER(" ", 1), NULL))) {
			return false;
		}

		//stringer_t *hex[2] = { NULL, hex_encode_st(hash, NULL) };
		//log_pedantic("%-15.15s = %.*s", "plain", st_length_int(hex[0]), st_char_get(hex[0]));
		//log_pedantic("%-15.15s = %s / %.*s", "hash", name, st_length_int(hex[1]), st_char_get(hex[1]));
		//st_free(hex[0]); st_free(hex[1]);

		st_free(hash);
	}

	return true;
}
