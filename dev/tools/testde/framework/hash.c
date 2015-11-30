
#include "framework.h"

extern global_config_t config;
extern const EVP_MD * (*EVP_sha512_d)(void);
extern int (*EVP_DigestInit_d)(EVP_MD_CTX *ctx, const EVP_MD *type);
extern int (*EVP_DigestUpdate_d)(EVP_MD_CTX *ctx, const void *d, unsigned int cnt);
extern int (*EVP_DigestFinal_d)(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s);

int hash_sha512_password(hashed_password_t *hashed, stringer_t *username, stringer_t *password) {
	
	int increment;
	char final[129];
	char passkey[129];
	int out_len = 0;
	EVP_MD_CTX ctx;
	stringer_t *final_out;
	stringer_t *passkey_out;
	const EVP_MD *algorithim;
	unsigned char result[EVP_MAX_MD_SIZE];
	
	if (!hashed || !username || !password) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Received NULL input value.");
		#endif
		return -1;
	}
	
	hashed->passkey = NULL;
	hashed->final = NULL;
	
	if (used_st(config.salt) == 0|| used_st(password) == 0 || used_st(username) == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Received an input of zero length.");
		#endif
		return -1;
	}
	
	algorithim = EVP_sha512_d();
	if (!algorithim) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not load the SHA 512 algorithim. EVP_sha512 = NULL");
		#endif
		return -1;
	}
	
	if (!EVP_DigestInit_d(&ctx, algorithim)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not initialize the SHA context.");
		#endif
		return -1;
	}
	
	// The first round gives us the passkey.
	if (!EVP_DigestUpdate_d(&ctx, data_st(username), used_st(username))) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not update the SHA context.");
		#endif
		return -1;
	}
	
	if (!EVP_DigestUpdate_d(&ctx, data_st(config.salt), used_st(config.salt))) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not update the SHA context.");
		#endif
		return -1;
	}
	
	if (!EVP_DigestUpdate_d(&ctx, data_st(password), used_st(password))) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not update the SHA context.");
		#endif
		return -1;
	}
	
	// Store the passkey.
	if (!EVP_DigestFinal_d(&ctx, &result[0], &out_len)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not update the SHA context.");
		#endif
		return -1;
	}
	
	if (out_len != 64) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The result appears to be an invalid length.");
		#endif
		return -1;
	}
	
	// This will be the OTP for the private ECC key.		
	for(increment = 0; increment < out_len; increment++) {
		sprintf(&passkey[0] + (increment * 2), "%02x", result[increment]);
	}
	
	// Round two.
	if (!EVP_DigestInit_d(&ctx, algorithim)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not update the SHA context.");
		#endif
		return -1;
	}
	
	if (!EVP_DigestUpdate_d(&ctx, data_st(password), used_st(password))) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not update the SHA context.");
		#endif
		return -1;
	}
	
	if (!EVP_DigestUpdate_d(&ctx, &result[0], out_len)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not update the SHA context.");
		#endif
		return -1;
	}
	
	// Store round two.
	if (!EVP_DigestFinal_d(&ctx, &result[0], &out_len)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not update the SHA context.");
		#endif
		return -1;
	}
	
	if (out_len != 64) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The result appears to be an invalid length.");
		#endif
		return -1;
	}
	
	// Round three, the final.
	if (!EVP_DigestInit_d(&ctx, algorithim)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not update the SHA context.");
		#endif
		return -1;
	}
	
	if (!EVP_DigestUpdate_d(&ctx, data_st(password), used_st(password))) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not update the SHA context.");
		#endif
		return -1;
	}
	
	if (!EVP_DigestUpdate_d(&ctx, &result[0], out_len)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not update the SHA context.");
		#endif
		return -1;
	}
	
	// Store round three.
	if (!EVP_DigestFinal_d(&ctx, &result[0], &out_len)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not update the SHA context.");
		#endif
		return -1;
	}
	
	if (out_len != 64) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The result appears to be an invalid length.");
		#endif
		return -1;
	}
	
	// This will be what we check the DB for.	
	for(increment = 0; increment < 64; increment++) {
		sprintf(&final[0] + (increment * 2), "%02x", result[increment]);
	}
	
	// Allocate memory.
	final_out = import_bl(final, 128);
	passkey_out = import_bl(passkey, 128);
	
	if (final_out == NULL || passkey_out == NULL) {
		if (final_out == NULL) {
			free_st(final_out);
		}
		if (passkey_out == NULL) {
			free_st(passkey_out);
		}
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unabled to store the hashed values in stringers.");
		#endif
		return -1;
	}
	
	hashed->passkey = passkey_out;
	hashed->final = final_out;
	
	// Don't let things fall into the wrong hands.
	clear_bl(passkey, 129);
	clear_bl(final, 129);

	return 1;
}
