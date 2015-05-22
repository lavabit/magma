
/**
 * @file /magma.check/providers/hmac_check.c
 *
 * @brief The logic used to test the hmac algorithm interface.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

bool_t check_hmac_simple(void) {

	digest_t *digest;
	stringer_t *hash, *hex;
	chr_t *digest_list[] = {
		"MD4", "MD5", "SHA", "SHA1", "SHA224", "SHA256", "SHA384", "SHA512", "RIPEMD160"
	},
	*key_list[] = {
		"abcdefghKEY1KEY2ijklmnopKEY3KEY4",
		"qrstuvwxKEY5KEY6yzABCDEFKEY7KEY8",
		"GHIJKLMNKEY9KEY0OPQRSTUVKEY1KEY2WXYZabcd",
		"KEY3KEY4efghijklKEY5KEY6mnopqrstKEY7KEY8",
		"uvwxyzABKEY9KEY0CDEFGHIJKEY1KEY2KLMNOPQRKEY3KEY4STUVWXYZ",
		"KEY5KEY6abcdefghKEY7KEY8ijklmnopKEY9KEY0qrstuvwxKEY1KEY2yzABCDEF",
		"KEY3KEY4GHIJKLMNKEY5KEY6OPQRSTUVKEY7KEY8WXYZabcdKEY9KEY0efghijklKEY1KEY2mnopqrstKEY3KEY4uvwxyzAB",
		"CDEFGHIJKEY5KEY6KLMNOPQRKEY7KEY8STUVWXYZKEY9KEY0abcdefghKEY1KEY2ijklmnopKEY3KEY4qrstuvwxKEY5KEY6yzABCDEFKEY7KEY8GHIJKLMNKEY9KEY0",
		"OPQRSTUVKEY1KEY2WXYZabcdKEY3KEY4efghijkl"
	},
	*result_list[] = {
		"ca0ac03c32316b2025e1732b0eb1377c",
		"f0e5f1e5a93f1f051ccc80852568c895",
		"1ca147ab57ba0d0d44f47804074e9156f108efd7",
		"7843bc098af2279a631dcd5cdc4d9caefb37d3b9",
		"1d1e789e4d0cdd7ba892287412710513723394799900a963851568f0",
		"0e99f21ba0df76417e9db5ca1b9635679b72fa3c9b4ad7012d946bee6ca27074",
		"958b3e11796cd6139720901106be3541371fc17f7de7d90c2d92531e18c3d050f00bc3e9b241da6c65fbccaa7ee5b130",
		"5c8d649316694980857c4f7052e1464d49166586968b7149a239d0855e3ba24531d446659a9cd0f6fa4a03a583bbca14e9e4e8a1e3aa35e999138abfdc55bf42",
		"0f6ffd4b9e9c65ada6e006c1343d927b36ee7195"
	};

	for (uint64_t i = 0	; status() && i < (sizeof(digest_list) / sizeof(chr_t *)); ++i) {

		if (!(digest = hash_name(NULLER(digest_list[i]))) || !(hash = hmac_digest(digest, PLACER(" ", 1), NULLER(key_list[i]), NULL))) {
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
