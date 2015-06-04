
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

/**
 * @brief	Calculate hmacs with a constant key and input string and compare to pre-calculated values.
 * @return	True on successful comparisons, false if at least one failed.
*/
bool_t check_hmac_simple(void) {

	digest_t *digest;
	stringer_t *hash, *hex,
	*digest_list[] = {
		PLACER("MD4", 3), 
		PLACER("MD5", 3),
		PLACER("SHA", 3),
		PLACER("SHA1", 4),
		PLACER("SHA224", 6),
		PLACER("SHA256", 6),
		PLACER("SHA384", 6),
		PLACER("SHA512", 6),
		PLACER("RIPEMD160", 9)
	},
	*key = PLACER("key", 3),
	*result_list[] = {
		PLACER("9b74896f0d315106c754a6c98d5602df", 32),
		PLACER("bc6a30c3fb714a626f2579eb17996408", 32),
		PLACER("dd3856b9637c715a17baa22ddaefbe8d5950ad38", 40),
		PLACER("0f296eadc8f232211a6f5a80427c3abb6490d392", 40),
		PLACER("162d0318e6a8ca179f11ea8811a03d0187e0fb77f553f650b88d945a", 56),
		PLACER("c68e4d52fc7380d5910f95008ceced2eb543bde28446eacb095bfc2993ea0457", 64),
		PLACER("ff9120f9fb89d833c30ab360e0f86291bdc56b3b6d61f9223556fdc7e90e8baf630668234cec76ca18cc780a868bab10", 96),
		PLACER("add0b1c73df30f0b9b968a20dd3cb4c7b24c88d47147fc13bac811db852a2883b71904f81d0dc03c2738c7e9e380a6436cd972ca3964f9e921cb983e9d3e5035", 128),
		PLACER("f83f31af3bd50fd2e85925e25472d0b0e0879c34", 40)
	};

	for (uint64_t i = 0; status() && i < (sizeof(digest_list) / sizeof(chr_t *)); ++i) {

		if (!(digest = hash_name(digest_list[i])) || !(hash = hmac_digest(digest, digest_list[i], key, NULL))) {
			return false;
		}
		else if (!(hex = hex_encode_st(hash, NULL))) {
			st_free(hash);
			return false;
		}

		if (st_cmp_cs_eq(hex, result_list[i] )) {
			st_free(hash);
			st_free(hex);
			return false;
		}

		st_free(hash);
		st_free(hex);
	}

	return true;
}

bool_t check_hmac_parameters(void) {

	digest_t *temp_dig;
	stringer_t *temp_st, *res;

	temp_dig = hash_name("SHA512");
	temp_st = NULLER("temp_string");
	
	if((res = hmac_digest(NULL, temp_st, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_digest(temp_dig, NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_digest(temp_dig, temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_multi_digest(0, NULL, temp_st, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_multi_digest(111, NULL, temp_st, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_multi_digest(0, temp_dig, NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_multi_digest(111, temp_dig, NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_multi_digest(0, temp_dig, temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_multi_digest(111, temp_dig, temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_md4(NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_md4(temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_md5(NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_md5(temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_sha(NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_sha(temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_sha1(NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_sha1(temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_sha224(NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_sha224(temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_sha256(NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_sha256(temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_sha384(NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_sha384(temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_sha512(NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_sha512(temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_ripemd160(NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_ripemd160(temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_multi_sha512(0, NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_multi_sha512(0, NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_multi_sha512(0, temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}
	else if((res = hmac_multi_sha512(0, temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}

	return true;
}
