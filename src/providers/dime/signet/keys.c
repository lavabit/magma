
#include "dime/common/misc.h"
#include "dime/signet/keys.h"
#include "dime/signet/signet.h"

static EC_KEY * keys_enckey_fetch(char const *filename);

static EC_KEY * keys_enckey_from_binary(unsigned char const *bin_keys, size_t len);

static int keys_file_create(keys_type_t type, ED25519_KEY *sign_key, EC_KEY *enc_key, char const *filename);

static unsigned char * keys_file_serialize(char const *filename, size_t *len);

static int keys_length_check(unsigned char const *in, size_t in_len);

static ED25519_KEY * keys_signkey_fetch(char const *filename);

static ED25519_KEY * keys_signkey_from_binary(unsigned char const *bin_keys, size_t len);

static keys_type_t keys_type_get(unsigned char const *bin_keys, size_t len);

static int keys_generate(keys_type_t type, char **signet_pem, char **key_pem);

// TODO - keys files not currently encrypted
// TODO - private key serialization currently occurs into DER encoded format
// which is long, therefore we have 2 bytes for private key length
//static int
//keys_file_add_sok(
//  ED25519_KEY *sok,
//  const char *filename);

/* PRIVATE FUNCTIONS */

/**
 * @brief
 *  Checks the size of the keys buffer for consistency.
 * @param in
 *  Keys buffer.
 * @param in_len
 *  Keys buffer size.
 * @return
 *  0 if the length checks pass, -1 if they do not.
 */
static int keys_length_check(unsigned char const *in, size_t in_len) {
	uint32_t signet_length;

	if (!in || (in_len < SIGNET_HEADER_SIZE)) {
		RET_ERROR_INT(ERR_BAD_PARAM, NULL);
	}
	signet_length = _int_no_get_3b((void *)(in + 2));
	if ((in_len - SIGNET_HEADER_SIZE) != signet_length) {
		RET_ERROR_INT(ERR_UNSPEC, "length does not match input size");
	}
	return 0;
}

/**
 * @brief
 *  Retrieves the keys type (user or organizational) from the keys binary.
 * @param bin_keys
 *  Pointer to the keys buffer.
 * @param len
 *  Length of the keys buffer.
 * @return
 *  Keys type on success, KEYS_TYPE_ERROR on error.
 */
static keys_type_t keys_type_get(unsigned char const *bin_keys, size_t len) {
	dime_number_t number;

	if (!bin_keys) {
		RET_ERROR_CUST(KEYS_TYPE_ERROR, ERR_BAD_PARAM, NULL);
	}
	else if (keys_length_check(bin_keys, len) < 0) {
		RET_ERROR_CUST(KEYS_TYPE_ERROR, ERR_BAD_PARAM, NULL);
	}
	number = (dime_number_t)_int_no_get_2b((void *)bin_keys);
	if (number == DIME_ORG_KEYS) {
		return KEYS_TYPE_ORG;
	}
	else if (number == DIME_USER_KEYS) {
		return KEYS_TYPE_USER;
	}
	RET_ERROR_CUST(KEYS_TYPE_ERROR, ERR_UNSPEC, "DIME number is not keys file type");
}

/**
 * @brief
 *  Retrieves the encryption key from the keys binary.
 * @param bin_keys
 *  Pointer to the keys buffer.
 * @param len
 *  Length of the keys buffer.
 * @return
 *  Pointer to elliptic curve key, NULL if an error occurred.
 * @free_using{free_ec_key}
 */
static EC_KEY * keys_enckey_from_binary(unsigned char const *bin_keys, size_t len) {
	unsigned char enc_fid;
	size_t at = 0, privkeylen;
	EC_KEY *enc_key = NULL;

	if (!bin_keys) {
		RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
	}
	else if (keys_length_check(bin_keys, len) < 0) {
		RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
	}
	switch (keys_type_get(bin_keys, len)) {

	case KEYS_TYPE_ORG:
		enc_fid = KEYS_ORG_PRIVATE_ENC;
		break;
	case KEYS_TYPE_USER:
		enc_fid = KEYS_USER_PRIVATE_ENC;
		break;
	default:
		RET_ERROR_PTR(ERR_UNSPEC, "invalid keys type");
		break;

	}
	at = KEYS_HEADER_SIZE;

	while (bin_keys[at++] != enc_fid) {
		at += bin_keys[at] + 1;
		if (len <= at) {
			RET_ERROR_PTR(ERR_UNSPEC, "no private encryption key in keys file");
		}
	}

	privkeylen = _int_no_get_2b(bin_keys + at);
	at += 2;
	if (at + privkeylen > len) {
		RET_ERROR_PTR(ERR_UNSPEC, "invalid encryption key size");
	}
	if (!(enc_key = _deserialize_ec_privkey(bin_keys + at, privkeylen, 0))) {
		RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize private EC encryption key");
	}
	return enc_key;
}

/**
 * @brief
 *  Retrieves the signing key from the keys binary.
 * @param bin_keys
 *  Pointer to the keys buffer.
 * @param len
 *  Length of the keys buffer.
 * @return
 *  Pointer to ed25519 signing key, NULL if an error occurred.
 * @free_using{free_ed25519_key}
 */
static ED25519_KEY * keys_signkey_from_binary(unsigned char const *bin_keys, size_t len) {
	unsigned char sign_fid;
	unsigned int at = 0;
	ED25519_KEY *sign_key;

	if (!bin_keys) {
		RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
	}
	else if (keys_length_check(bin_keys, len) < 0) {
		RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
	}
	else if (len < KEYS_HEADER_SIZE + 2 + ED25519_KEY_SIZE) {
		RET_ERROR_PTR(ERR_BAD_PARAM, "keys buffer too small for signing key");
	}
	switch (keys_type_get(bin_keys, len)) {

	case KEYS_TYPE_ORG:
		sign_fid = KEYS_ORG_PRIVATE_POK;
		break;
	case KEYS_TYPE_USER:
		sign_fid = KEYS_USER_PRIVATE_SIGN;
		break;
	default:
		RET_ERROR_PTR(ERR_UNSPEC, "invalid keys type");
		break;

	}
	at = KEYS_HEADER_SIZE;
	if (bin_keys[at++] != sign_fid) {
		RET_ERROR_PTR(ERR_UNSPEC, "no signing key was found");
	}
	if (bin_keys[at++] != ED25519_KEY_SIZE) {
		RET_ERROR_PTR(ERR_UNSPEC, "invalid size of signing key");
	}
	if (!(sign_key = _deserialize_ed25519_privkey(bin_keys + at))) {
		RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize ed25119 signing key");
	}
	return sign_key;
}

/**
 * @brief
 *  Retrieves the keys binary from the keys file.
 * @param filename
 *  Null terminated string containing specified filename.
 * @param len
 *  Pointer to the length of the output.
 * @return
 *  Pointer to the keys binary string, this memory needs to be wipe before
 *  being freed. NULL on error.
 * @free_using{free}
 */
static unsigned char * keys_file_serialize(char const *filename, size_t *len) {
	char *b64_keys = NULL;
	unsigned char *serial_keys = NULL;

	if (!filename || !len) {
		RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
	}
	if (!(b64_keys = _read_pem_data(filename, SIGNET_KEY_USER, 1)) && !(b64_keys = _read_pem_data(filename, SIGNET_KEY_ORG, 1))) {
		RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve keys from PEM file");
	}
	if (!(serial_keys = _b64decode(b64_keys, strlen(b64_keys), len))) {
		free(b64_keys);
		RET_ERROR_PTR(ERR_UNSPEC, "could not base64 decode the keys");
	}
	free(b64_keys);

	return serial_keys;
}

/**
 * @brief
 *  Creates a keys file with specified signing and encryption keys.
 * @param type
 *  Type of keys file, whether the keys correspond to a user or organizational
 *  signet.
 * @param sign_key
 *  Pointer to the specified ed25519 key, the private portion of which will be
 *  stored in the keys file as the signing key.
 * @param enc_key
 *  Pointer to the specified elliptic curve key, the private portion of which
 *  will be stored in the keys file as the encryption key.
 * @param filename
 *  Pointer to the NULL terminated string containing the filename for the keys
 *  file.
 * @return  0 on success, -1 on failure.
 */
static int keys_file_create(keys_type_t type, ED25519_KEY *sign_key, EC_KEY *enc_key, char const *filename) {
	char *b64_keys = NULL, *b64_crc_keys = NULL, holder[16];
	int res;
	uint32_t crc;
	size_t serial_size = 0, enc_size = 0, at = 0;
	unsigned char *serial_keys = NULL, *serial_enc = NULL, serial_sign[ED25519_KEY_SIZE], sign_fid, enc_fid, be[3];
	dime_number_t number;

	if (!sign_key || !enc_key || !filename) {
		RET_ERROR_INT(ERR_BAD_PARAM, NULL);
	}
	switch (type) {

	case KEYS_TYPE_ORG:
		number = DIME_ORG_KEYS;
		sign_fid = KEYS_ORG_PRIVATE_POK;
		enc_fid	 = KEYS_ORG_PRIVATE_ENC;
		break;
	case KEYS_TYPE_USER:
		number = DIME_USER_KEYS;
		sign_fid = KEYS_USER_PRIVATE_SIGN;
		enc_fid	 = KEYS_USER_PRIVATE_ENC;
		break;
	default:
		RET_ERROR_INT(ERR_BAD_PARAM, NULL);
		break;

	}
	memcpy(serial_sign, sign_key->private_key, ED25519_KEY_SIZE);
	if (!(serial_enc = _serialize_ec_privkey(enc_key, &enc_size))) {
		_secure_wipe(serial_sign, ED25519_KEY_SIZE);
		RET_ERROR_INT(ERR_UNSPEC, "could not serialize private key");
	}
	serial_size = KEYS_HEADER_SIZE + 1 + 1 + ED25519_KEY_SIZE + 1 + 2 + enc_size;
	if (!(serial_keys = malloc(serial_size))) {
		PUSH_ERROR_SYSCALL("malloc");
		_secure_wipe(serial_sign, ED25519_KEY_SIZE);
		_secure_wipe(serial_enc, enc_size);
		free(serial_enc);
		RET_ERROR_INT(ERR_NOMEM, NULL);
	}
	memset(serial_keys, 0, serial_size);
	_int_no_put_2b(serial_keys, (uint16_t)number);
	_int_no_put_3b(serial_keys + 2, (uint32_t)(serial_size - KEYS_HEADER_SIZE));
	at = KEYS_HEADER_SIZE;
	serial_keys[at++] = sign_fid;
	serial_keys[at++] = ED25519_KEY_SIZE;
	memcpy(serial_keys + at, serial_sign, ED25519_KEY_SIZE);
	at += ED25519_KEY_SIZE;
	_secure_wipe(serial_sign, ED25519_KEY_SIZE);
	serial_keys[at++] = enc_fid;
	_int_no_put_2b(serial_keys + at, (uint16_t)enc_size);
	at += 2;
	memcpy(serial_keys + at, serial_enc, enc_size);
	_secure_wipe(serial_enc, enc_size);
	free(serial_enc);

	crc = _compute_crc24_checksum(serial_keys, serial_size);
	b64_keys = _b64encode(serial_keys, serial_size);

	be[0] = ((unsigned char *)&crc)[2];
	be[1] = ((unsigned char *)&crc)[1];
	be[2] = ((unsigned char *)&crc)[0];

	b64_crc_keys = _b64encode((unsigned char *)&be, (size_t)3);
	_secure_wipe(serial_keys, serial_size);
	free(serial_keys);
	if (!b64_keys || !b64_crc_keys) {
		if (b64_keys) {free(b64_keys); }
		if (b64_crc_keys) {free(b64_crc_keys); }
		RET_ERROR_INT(ERR_UNSPEC, "could not base64 encode the keys");
	}
	if (snprintf(holder, 16, "\n=%s", b64_crc_keys) != 6) {
		free(b64_keys);
		free(b64_crc_keys);
		RET_ERROR_INT(ERR_UNSPEC, "could not armor the keys");
	}
	res = _write_pem_data(b64_keys, holder, type == KEYS_TYPE_USER ? SIGNET_KEY_USER : SIGNET_KEY_ORG, filename);
	_secure_wipe(b64_keys, strlen(b64_keys));
	free(b64_keys);
	free(b64_crc_keys);
	if (res < 0) {
		RET_ERROR_INT(ERR_UNSPEC, "could not store keys in PEM file.");
	}
	return 0;
}

static int keys_generate(keys_type_t type, char **signet_pem, char **key_pem) {

	int res;
	uint32_t crc, signet_len;
	signet_t *signet;
	dime_number_t number;
	EC_KEY *enc_key = NULL;
	ED25519_KEY *sign_key = NULL;
	size_t serial_size = 0, enc_size = 0, at = 0;
	char *b64_keys = NULL, *b64_crc_keys = NULL, *b64_signet = NULL, *b64_crc_signet = NULL, *result = NULL;
	unsigned char *serial_keys = NULL, *serial_signet = NULL, *serial_enc = NULL,
		serial_sign[ED25519_KEY_SIZE], sign_fid, enc_fid, be[3];

	if (!(sign_key = _generate_ed25519_keypair())) {
		RET_ERROR_INT(ERR_UNSPEC, "could not generate ed25519 key pair");
	}

	if (!(enc_key = _generate_ec_keypair())) {
		_free_ed25519_key(sign_key);
		RET_ERROR_INT(ERR_UNSPEC, "could not generate elliptic curve key pair");
	}

	switch (type) {

	case KEYS_TYPE_ORG:
		number = DIME_ORG_KEYS;
		sign_fid = KEYS_ORG_PRIVATE_POK;
		enc_fid	 = KEYS_ORG_PRIVATE_ENC;
		break;
	case KEYS_TYPE_USER:
		number = DIME_USER_KEYS;
		sign_fid = KEYS_USER_PRIVATE_SIGN;
		enc_fid	 = KEYS_USER_PRIVATE_ENC;
		break;
	default:
		RET_ERROR_INT(ERR_BAD_PARAM, NULL);
		break;

	}

	if (!(signet = dime_sgnt_signet_create(type))) {
		_free_ec_key(enc_key);
		_free_ed25519_key(sign_key);
		RET_ERROR_INT(ERR_UNSPEC, "could not create signet object");
	}

	res = dime_sgnt_signkey_set(signet, sign_key, SIGNKEY_DEFAULT_FORMAT);
	res = dime_sgnt_enckey_set(signet, enc_key, 0);

	serial_signet = dime_sgnt_signet_binary_serialize(signet, &signet_len);
	crc = _compute_crc24_checksum(serial_signet, signet_len);

	be[0] = ((unsigned char *)&crc)[2];
	be[1] = ((unsigned char *)&crc)[1];
	be[2] = ((unsigned char *)&crc)[0];

	b64_crc_signet = _b64encode((unsigned char *)&be, (size_t)3);
	b64_signet = _b64encode_w_lineseperators(serial_signet, signet_len);

	res = str_printf(&result, "-----BEGIN %s-----\n%s\n=%s\n-----END %s-----\n",
		type == KEYS_TYPE_USER ? SIGNET_USER : SIGNET_ORG, b64_signet, b64_crc_signet,
		type == KEYS_TYPE_USER ? SIGNET_USER : SIGNET_ORG);

	*signet_pem = result;
	result = NULL;

	memcpy(serial_sign, sign_key->private_key, ED25519_KEY_SIZE);
	if (!(serial_enc = _serialize_ec_privkey(enc_key, &enc_size))) {
		_secure_wipe(serial_sign, ED25519_KEY_SIZE);
		RET_ERROR_INT(ERR_UNSPEC, "could not serialize private key");
	}
	serial_size = KEYS_HEADER_SIZE + 1 + 1 + ED25519_KEY_SIZE + 1 + 2 + enc_size;
	if (!(serial_keys = malloc(serial_size))) {
		PUSH_ERROR_SYSCALL("malloc");
		_secure_wipe(serial_sign, ED25519_KEY_SIZE);
		_secure_wipe(serial_enc, enc_size);
		free(serial_enc);
		RET_ERROR_INT(ERR_NOMEM, NULL);
	}
	memset(serial_keys, 0, serial_size);
	_int_no_put_2b(serial_keys, (uint16_t)number);
	_int_no_put_3b(serial_keys + 2, (uint32_t)(serial_size - KEYS_HEADER_SIZE));
	at = KEYS_HEADER_SIZE;
	serial_keys[at++] = sign_fid;
	serial_keys[at++] = ED25519_KEY_SIZE;
	memcpy(serial_keys + at, serial_sign, ED25519_KEY_SIZE);
	at += ED25519_KEY_SIZE;
	_secure_wipe(serial_sign, ED25519_KEY_SIZE);
	serial_keys[at++] = enc_fid;
	_int_no_put_2b(serial_keys + at, (uint16_t)enc_size);
	at += 2;
	memcpy(serial_keys + at, serial_enc, enc_size);
	_secure_wipe(serial_enc, enc_size);
	free(serial_enc);

	crc = _compute_crc24_checksum(serial_keys, serial_size);
	b64_keys = _b64encode_w_lineseperators(serial_keys, serial_size);

	be[0] = ((unsigned char *)&crc)[2];
	be[1] = ((unsigned char *)&crc)[1];
	be[2] = ((unsigned char *)&crc)[0];

	b64_crc_keys = _b64encode((unsigned char *)&be, (size_t)3);
	_secure_wipe(serial_keys, serial_size);
	free(serial_keys);
	if (!b64_keys || !b64_crc_keys) {
		if (b64_keys) {free(b64_keys); }
		if (b64_crc_keys) {free(b64_crc_keys); }
		RET_ERROR_INT(ERR_UNSPEC, "could not base64 encode the keys");
	}

	res = str_printf(&result, "-----BEGIN %s-----\n%s\n=%s\n-----END %s-----\n",
		type == KEYS_TYPE_USER ? SIGNET_KEY_USER : SIGNET_KEY_ORG, b64_keys, b64_crc_keys,
		type == KEYS_TYPE_USER ? SIGNET_KEY_USER : SIGNET_KEY_ORG);

	_secure_wipe(b64_keys, strlen(b64_keys));
	free(b64_keys);
	free(b64_crc_keys);
	if (res < 0) {
		RET_ERROR_INT(ERR_UNSPEC, "could not generate keys in PEM format.");
	}

	*key_pem = result;
	return 0;

}

/**
 * @brief
 *  Retrieves the signing key from the keys file.
 * @param filename
 *  Null terminated filename string.
 * @return
 *  Pointer to the ed25519 signing key.
 * @free_using{free_ed25519_key}
 */
static ED25519_KEY * keys_signkey_fetch(char const *filename) {
	size_t keys_len;
	unsigned char *keys_bin;
	ED25519_KEY *key;

	if (!filename) {
		RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
	}
	else if (!strlen(filename)) {
		RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
	}
	if (!(keys_bin = keys_file_serialize(filename, &keys_len))) {
		RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve keys binary string");
	}
	key = keys_signkey_from_binary(keys_bin, keys_len);
	_secure_wipe(keys_bin, keys_len);
	free(keys_bin);
	if (!key) {
		RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve ed25519 signing key");
	}
	return key;
}

/**
 * @brief
 *  Retrieves the encryption key from the keys file.
 * @param filename
 *  Null terminated filename string.
 * @return
 *  Pointer to the elliptic curve encryption key.
 * @free_using{free_ec_key}
 */
static EC_KEY * keys_enckey_fetch(char const *filename) {
	size_t keys_len;
	unsigned char *keys_bin;
	EC_KEY *key;

	if (!filename) {
		RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
	}
	else if (!strlen(filename)) {
		RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
	}
	if (!(keys_bin = keys_file_serialize(filename, &keys_len))) {
		RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve keys binary string");
	}
	key = keys_enckey_from_binary(keys_bin, keys_len);
	_secure_wipe(keys_bin, keys_len);
	free(keys_bin);
	if (!key) {
		RET_ERROR_PTR_FMT(ERR_UNSPEC, "could not retrieve ed25519 signing key from %s", filename);
	}
	return key;
}

/* PUBLIC FUNCTIONS */

/**
 * @brief
 *  Creates a keys file with specified signing and encryption keys.
 * @param type
 *  Type of keys file, whether the keys correspond to a user or organizational
 *  signet.
 * @param sign_key
 *  Pointer to the specified ed25519 key, the private portion of which will be
 *  stored in the keys file as the signing key.
 * @param enc_key
 *  Pointer to the specified elliptic curve key, the private portion of which
 *  will be stored in the keys file as the encryption key.
 * @param filename
 *  Pointer to the NULL terminated string containing the filename for the keys
 *  file.
 * @return  0 on success, -1 on failure.
 */
int dime_keys_file_create(keys_type_t type, ED25519_KEY *sign_key, EC_KEY *enc_key, const char *filename) {
	PUBLIC_FUNCTION_IMPLEMENT(keys_file_create, type, sign_key, enc_key, filename);
}

// Not implemented yet
//int
//dime_keys_file_sok_add(
//    ED25519_KEY *sok,
//    sok_permissions_t perm,
//    const char *filename)
//{
//    PUBLIC_FUNCTION_IMPLEMENT(
//        keys_file_add_sok,
//        sok,
//        filename);
//}
//
//ED25519_KEY *
//dime_keys_sok_fetch(
//    char const *filename,
//    unsigned int num)
//{
//    PUBLIC_FUNCTION_IMPLEMENT(keys_sok_fetch, sok, num);
//}

/**
 * @brief
 *  Retrieves the encryption key from the keys file.
 * @param filename
 *  Null terminated filename string.
 * @return
 *  Pointer to the elliptic curve encryption key.
 * @free_using{free_ec_key}
 */
ED25519_KEY * dime_keys_signkey_fetch(char const *filename) {
	PUBLIC_FUNCTION_IMPLEMENT(keys_signkey_fetch, filename);
}

/**
 * @brief
 *  Retrieves the signing key from the keys file.
 * @param filename
 *  Null terminated filename string.
 * @return
 *  Pointer to the ed25519 signing key.
 * @free_using{free_ed25519_key}
 */
EC_KEY * dime_keys_enckey_fetch(char const *filename) {
	PUBLIC_FUNCTION_IMPLEMENT(keys_enckey_fetch, filename);
}

int dime_keys_generate(keys_type_t type, char **signet_pem, char **key_pem) {
	PUBLIC_FUNCTION_IMPLEMENT(keys_generate, type, signet_pem, key_pem);
}
