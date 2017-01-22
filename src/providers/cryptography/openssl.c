
/**
 * @file /magma/providers/cryptography/openssl.c
 *
 * @brief	The interface to OpenSSL routines.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

char ssl_version[16];
extern DH *dh2048, *dh4096;
pthread_mutex_t **ssl_locks = NULL;
extern pthread_mutex_t dhparam_lock;

/**
 * @brief	Return the version string of the OpenSSL library.
 * @return	a pointer to a character string containing the libopenssl version information.
 */
const char * lib_version_openssl(void) {

	return ssl_version;
}

/**
 * @brief	Initialize the OpenSSL library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_openssl(void) {

	placer_t parse;
	symbol_t openssl[] = {
		M_BIND(ASN1_STRING_TABLE_cleanup), M_BIND(BIO_new_socket), M_BIND(BIO_sock_cleanup), M_BIND(BIO_vprintf), M_BIND(BN_bin2bn),
		M_BIND(BN_bn2bin), M_BIND(BN_bn2hex), M_BIND(BN_free), M_BIND(BN_hex2bn), M_BIND(BN_num_bits), M_BIND(COMP_zlib_cleanup),
		M_BIND(CONF_modules_unload), M_BIND(CRYPTO_cleanup_all_ex_data), M_BIND(CRYPTO_free), M_BIND(CRYPTO_num_locks),
		M_BIND(CRYPTO_set_id_callback), M_BIND(CRYPTO_set_locking_callback), M_BIND(DH_free), M_BIND(DH_generate_parameters_ex), M_BIND(DH_new),
		M_BIND(ECDH_compute_key), M_BIND(EC_GROUP_free), M_BIND(EC_GROUP_new_by_curve_name), M_BIND(EC_GROUP_precompute_mult),
		M_BIND(EC_GROUP_set_point_conversion_form), M_BIND(EC_KEY_check_key), M_BIND(EC_KEY_free), M_BIND(EC_KEY_generate_key),
		M_BIND(EC_KEY_get0_group),M_BIND(EC_KEY_get0_private_key), M_BIND(EC_KEY_get0_public_key), M_BIND(EC_KEY_new),
		M_BIND(EC_KEY_new_by_curve_name), M_BIND(EC_KEY_set_group), M_BIND(EC_KEY_set_private_key), M_BIND(EC_KEY_set_public_key),
		M_BIND(EC_POINT_free), M_BIND(EC_POINT_hex2point), M_BIND(EC_POINT_new), M_BIND(EC_POINT_oct2point), M_BIND(EC_POINT_point2hex),
		M_BIND(EC_POINT_point2oct),	M_BIND(ENGINE_cleanup), M_BIND(ERR_error_string_n), M_BIND(ERR_free_strings),
		M_BIND(ERR_get_error), M_BIND(ERR_remove_thread_state), M_BIND(EVP_CIPHER_block_size),	M_BIND(EVP_CIPHER_CTX_block_size),
		M_BIND(EVP_CIPHER_CTX_cleanup),	M_BIND(EVP_CIPHER_CTX_init), M_BIND(EVP_CIPHER_CTX_iv_length), M_BIND(EVP_CIPHER_CTX_key_length),
		M_BIND(EVP_CIPHER_CTX_set_padding),	M_BIND(EVP_CIPHER_iv_length), M_BIND(EVP_CIPHER_key_length), M_BIND(EVP_CIPHER_nid),
		M_BIND(EVP_cleanup), M_BIND(EVP_DecryptFinal_ex), M_BIND(EVP_DecryptInit_ex), M_BIND(EVP_DecryptUpdate), M_BIND(EVP_Digest),
		M_BIND(EVP_DigestFinal), M_BIND(EVP_DigestFinal_ex), M_BIND(EVP_DigestInit), M_BIND(EVP_DigestInit_ex), M_BIND(EVP_DigestUpdate),
		M_BIND(EVP_EncryptFinal_ex), M_BIND(EVP_EncryptInit_ex), M_BIND(EVP_EncryptUpdate),	M_BIND(EVP_get_cipherbyname),
		M_BIND(EVP_get_digestbyname), M_BIND(EVP_md4), M_BIND(EVP_md5),	M_BIND(EVP_MD_CTX_cleanup),	M_BIND(EVP_MD_CTX_init),
		M_BIND(EVP_MD_size), M_BIND(EVP_ripemd160),	M_BIND(EVP_sha), M_BIND(EVP_sha1),	M_BIND(EVP_sha224),	M_BIND(EVP_sha256),
		M_BIND(EVP_sha384),	M_BIND(EVP_sha512),	M_BIND(HMAC_CTX_cleanup), M_BIND(HMAC_CTX_init), M_BIND(HMAC_Final), M_BIND(HMAC_Init_ex),
		M_BIND(HMAC_Update), M_BIND(OBJ_cleanup), M_BIND(OBJ_NAME_cleanup),	M_BIND(OBJ_nid2sn),	M_BIND(OPENSSL_add_all_algorithms_noconf),
		M_BIND(RAND_bytes),	M_BIND(RAND_cleanup), M_BIND(RAND_load_file), M_BIND(RAND_status), M_BIND(sk_pop_free),	M_BIND(SSL_accept),
		M_BIND(SSL_COMP_get_compression_methods), M_BIND(SSL_connect), M_BIND(SSL_CTX_check_private_key), M_BIND(SSL_CTX_ctrl),
		M_BIND(SSL_CTX_free), M_BIND(SSL_CTX_load_verify_locations), M_BIND(SSL_CTX_new), M_BIND(SSL_CTX_set_cipher_list),
		M_BIND(SSL_CTX_set_tmp_dh_callback), M_BIND(SSL_CTX_set_tmp_ecdh_callback), M_BIND(SSL_CTX_use_certificate_chain_file),
		M_BIND(SSL_CTX_use_PrivateKey_file), M_BIND(SSLeay_version), M_BIND(SSL_free), M_BIND(SSL_get_error), M_BIND(SSL_get_peer_certificate),
		M_BIND(SSL_get_shutdown), M_BIND(SSL_get_wbio), M_BIND(SSL_library_init), M_BIND(SSL_load_error_strings), M_BIND(SSL_new),
		M_BIND(SSL_read), M_BIND(SSL_set_bio), M_BIND(SSL_shutdown), M_BIND(SSLv23_client_method), M_BIND(SSLv23_server_method),
		M_BIND(SSL_version_str), M_BIND(SSL_write), M_BIND(TLSv1_server_method), M_BIND(X509_get_ext), M_BIND(X509_get_ext_count),
		M_BIND(X509_get_subject_name), M_BIND(X509_NAME_get_text_by_NID), M_BIND(EVP_MD_type), M_BIND(SSL_pending), M_BIND(SSL_want),
		M_BIND(SSL_get_rfd), M_BIND(EVP_CIPHER_CTX_ctrl), M_BIND(EVP_CIPHER_CTX_flags), M_BIND(EVP_CIPHER_flags), M_BIND(X509_STORE_CTX_new),
		M_BIND(sk_pop), M_BIND(i2d_OCSP_RESPONSE), M_BIND(ECDSA_do_sign), M_BIND(ECDSA_SIG_free), M_BIND(i2d_OCSP_CERTID), M_BIND(d2i_OCSP_RESPONSE),
		M_BIND(OCSP_REQUEST_new), M_BIND(OCSP_BASICRESP_free), M_BIND(i2d_X509), M_BIND(SSL_CTX_callback_ctrl), M_BIND(SSL_ctrl),
		M_BIND(X509_NAME_ENTRY_get_data), M_BIND(ASN1_INTEGER_to_BN), M_BIND(BIO_new_fp), M_BIND(X509_NAME_oneline), M_BIND(OCSP_response_status_str),
		M_BIND(X509_verify_cert_error_string), M_BIND(EVP_aes_256_cbc), M_BIND(d2i_ECPrivateKey), M_BIND(o2i_ECPublicKey), M_BIND(d2i_ECDSA_SIG),
		M_BIND(EVP_CIPHER_CTX_new), M_BIND(EVP_PKEY_new), M_BIND(ASN1_GENERALIZEDTIME_print), M_BIND(BIO_free), M_BIND(ECDSA_do_verify),
		M_BIND(EVP_PKEY_set1_RSA), M_BIND(EVP_VerifyFinal), M_BIND(i2d_ECDSA_SIG), M_BIND(i2d_ECPrivateKey), M_BIND(i2o_ECPublicKey),
		M_BIND(OCSP_basic_verify), M_BIND(OCSP_check_nonce), M_BIND(OCSP_check_validity), M_BIND(OCSP_parse_url), M_BIND(OCSP_REQ_CTX_add1_header),
		M_BIND(OCSP_REQ_CTX_set1_req), M_BIND(OCSP_request_add1_nonce), M_BIND(OCSP_REQUEST_print), M_BIND(OCSP_resp_find_status),
		M_BIND(OCSP_RESPONSE_print), M_BIND(OCSP_response_status), M_BIND(OCSP_sendreq_nbio), M_BIND(SHA1_Final), M_BIND(SHA1_Init),
		M_BIND(SHA1_Update), M_BIND(SHA256_Final), M_BIND(SHA256_Init), M_BIND(SHA256_Update), M_BIND(SHA512_Final), M_BIND(SHA512_Init),
		M_BIND(SHA512_Update), M_BIND(sk_num), M_BIND(SSL_get_fd), M_BIND(SSL_set_fd), M_BIND(X509_check_host), M_BIND(X509_check_issued),
		M_BIND(X509_NAME_get_index_by_NID), M_BIND(X509_STORE_CTX_get_error), M_BIND(X509_STORE_CTX_get_error_depth), M_BIND(X509_STORE_CTX_init),
		M_BIND(X509_STORE_load_locations), M_BIND(X509_STORE_set_flags), M_BIND(X509_verify_cert), M_BIND(OCSP_sendreq_new), M_BIND(RSA_new),
		M_BIND(RSAPublicKey_dup), M_BIND(BUF_strlcat), M_BIND(X509_get1_ocsp), M_BIND(SSL_get_peer_cert_chain), M_BIND(ASN1_STRING_data),
		M_BIND(SHA512), M_BIND(ERR_peek_error_line_data), M_BIND(BIO_free_all), M_BIND(EC_GROUP_clear_free), M_BIND(ERR_load_crypto_strings),
		M_BIND(ERR_print_errors_fp), M_BIND(EVP_CIPHER_CTX_free), M_BIND(OCSP_REQUEST_free), M_BIND(OCSP_RESPONSE_free), M_BIND(RSA_free),
		M_BIND(SSL_CTX_set_verify), M_BIND(X509_email_free), M_BIND(X509_STORE_CTX_free), M_BIND(X509_STORE_CTX_set_chain), M_BIND(X509_STORE_free),
		M_BIND(OCSP_cert_to_id), M_BIND(OCSP_request_add0_id), M_BIND(OCSP_response_get1_basic), M_BIND(sk_value), M_BIND(X509_STORE_CTX_get_current_cert),
		M_BIND(X509_STORE_add_lookup), M_BIND(X509_LOOKUP_file), M_BIND(X509_NAME_get_entry), M_BIND(X509_STORE_new), M_BIND(ERR_clear_error),
		M_BIND(ERR_put_error), M_BIND(EVP_aes_256_gcm), M_BIND(EC_KEY_get_conv_form), M_BIND(EC_KEY_set_conv_form), M_BIND(BN_bn2mpi),
		M_BIND(BN_mpi2bn), M_BIND(BN_bn2dec), M_BIND(EC_POINT_mul), M_BIND(BN_CTX_new), M_BIND(BN_CTX_start), M_BIND(BN_CTX_free),
		M_BIND(EC_POINT_cmp), M_BIND(BN_cmp), M_BIND(ED25519_keypair), M_BIND(ED25519_sign), M_BIND(ED25519_verify), M_BIND(ED25519_keypair_from_seed),
		M_BIND(CRYPTO_set_mem_functions), M_BIND(CRYPTO_set_locked_mem_functions), M_BIND(DH_check)
	};

	if (!lib_symbols(sizeof(openssl) / sizeof(symbol_t), openssl)) {
		return false;
	}

	// Parse the version string. Expecting "OpenSSL VERSION date" or "OpenSSL 1.0.2-x 15 Jul 2016"
	if (tok_get_ns(*SSL_version_str_d, ns_length_get(*SSL_version_str_d), ' ', 1, &parse) >= 0) {
		snprintf(ssl_version, 16, "%.*s", pl_length_int(parse), pl_char_get(parse));
	}
	else {
		snprintf(ssl_version, 16, "unknown");
	}

	return true;
}

/**
 * @brief	Initialize the OpenSSL facility.
 * @note	First, this function initializes the mutexes necessary for the locking function callback that openssl uses
 * 			for shared data structures in multi-threaded applications.
 * 			Next, the DKIM key is retrieved if the magma.dkim.enabled configuration variable is set.
 * @return	true if openssl was initialized successfully, or false on failure.
 */
bool_t ssl_start(void) {

	// DH Parameter Generator Lock
	if (mutex_init(&dhparam_lock, NULL)) {
		log_critical("Could not initialize DH parameter mutex.");
		return false;
	}

	// Thread locking setup.
	else if (!(ssl_locks = mm_alloc(CRYPTO_num_locks_d() * sizeof(pthread_mutex_t *)))) {
		log_critical("Could not allocate %zu bytes for the TLS library locks.", CRYPTO_num_locks_d() * sizeof(pthread_mutex_t *));
		return false;
	}

	// Get the SSL library setup.
	SSL_load_error_strings_d();
	SSL_library_init_d();

	// Initialize all of the mutexes.
	for (uint64_t i = 0; i < CRYPTO_num_locks_d(); i++) {

		if (!(*(ssl_locks + i) = mm_alloc(sizeof(pthread_mutex_t)))) {
			log_critical("Could not allocate %zu bytes for SSL lock %lu.", sizeof(pthread_mutex_t), i);
			return false;
		}
		else if (mutex_init(*(ssl_locks + i), NULL)) {
			log_critical("Could not initialize SSL mutex %lu.", i);
			return false;
		}

	}

	// Ensure all of the ciphers are available.
	OPENSSL_add_all_algorithms_noconf_d();

	CRYPTO_set_id_callback_d(&ssl_thread_id_callback);
	CRYPTO_set_locking_callback_d(&ssl_locking_callback);

	// Note these functions get replaced by the _secure_ functions in OpenSSL 1.1.0, where we simply need to call
	// CRYPTO_secure_malloc_init() and then when we shutdown, call CRYPTO_secure_malloc_done();
	//CRYPTO_set_mem_functions_d(&mm_alloc, &mm_realloc, &mm_free);
	CRYPTO_set_locked_mem_functions_d(&mm_sec_alloc, &mm_sec_free);

	return true;
}

/**
 * @brief	Stop the openssl library and cleanup all associated data structures.
 * @return	This function returns no values.
 */
void ssl_stop(void) {

	if (ssl_locks) {
		ERR_remove_thread_state_d(0);
		COMP_zlib_cleanup_d();
		CONF_modules_unload_d(1);
		OBJ_cleanup_d();
		OBJ_NAME_cleanup_d(-1);
		BIO_sock_cleanup_d();
		EVP_cleanup_d();
		ENGINE_cleanup_d();
		CRYPTO_cleanup_all_ex_data_d();
		ERR_free_strings_d();
		ASN1_STRING_TABLE_cleanup_d();
		CRYPTO_set_id_callback_d(NULL);
		CRYPTO_set_locking_callback_d(NULL);

		// The SSL compression method stack doesn't get freed properly by any of the functions above.
		// This was necessary as of 1.0.0b, but may be fixed.
		sk_pop_free_d((_STACK *)SSL_COMP_get_compression_methods_d(), mm_free);

		// Destroy and free all of the mutexes.
		for (uint64_t i = 0; i < CRYPTO_num_locks_d(); i++) {
			mutex_destroy(*(ssl_locks + i));
			mm_free(*(ssl_locks + i));
		}

		mutex_destroy(&dhparam_lock);
		mm_free(ssl_locks);
		ssl_locks = NULL;
	}

	return;
}

/**
 * @brief	Free the calling thread's SSL error queue.
 * @see		ssl_thread_stop()
 * @return	This function returns no value.
 */
void ssl_thread_stop(void) {
	ERR_remove_thread_state_d(NULL);
	return;
}

/**
 * @brief	The locking function callback necessary for all multi-threaded applications using OpenSSL.
 * @see		CRYPTO_set_locking_callback()
 * @param	mode	a bitmask specifying the requested openssl locking operation (CRYPTO_LOCK, CRYPTO_WRITE, CRYPTO_READ, or CRYPTO_UNLOCK).
 * @param	n		the zero-based index of the lock that is the target of the requested operation.
 * @param	file	a null-terminated string containing the filename of the function setting the lock, for debugging purposes.
 * @param	line	the line number of the function setting the lock, for debugging purposes.
 * @return	This function returns no value.
 */
void ssl_locking_callback(int mode, int n, const char *file, int line) {

	// Do a comparison. This eliminates the compiler warning about unused variables.
	// if ((int)file == line) {};

	// if (!st_cmp_cs_eq(NULLER((char *)file), PLACER("ex_data.c", 9)))
	//	log_pedantic("mode = %i, num = %i, file = %s, line = %i", mode, n, file, line);

	// Get a lock.
	if (mode & CRYPTO_LOCK) {
		mutex_lock(*(ssl_locks + n));
	}
	// Release a lock.
	else {
		mutex_unlock(*(ssl_locks + n));
	}

	return;
}

/**
 * @brief	The thread id callback necessary for all multi-threaded applications using OpenSSL.
 * @see		CRYPTO_set_id_callback()
 * @return	the id of the calling thread.
 */
unsigned long ssl_thread_id_callback(void) {

	return (unsigned long)thread_get_thread_id();
}

/**
 * @brief	Callback handler for the EC Diffie-Hellman parameter generation process necessary for PFS.
 * @param	ssl			the SSL session for which the callback was triggered.
 * @param	is_export	around here all we export is freedom.
 * @param	keylength	the length, in bits, of the ECCH key to be generated.
 * @return	a pointer to a ECDH key of proper size with parameters generated, or NULL on failure.
 */
EC_KEY * ssl_ecdh_exchange_callback(SSL *ssl, int is_export, int keylength) {

	static EC_KEY *ecdh512 = NULL, *ecdh1024 = NULL, *this_ecdh = NULL;

	if (keylength != 512 && keylength != 1024) {
		log_error("ECDH key generation failed; only 512/1024 bit keys are supported but %u were requested.", (unsigned int)keylength);
		return NULL;
	}

	// If the generated key was already cached, simply return it.
	if ((this_ecdh = (keylength == 512) ? ecdh512 : ecdh1024)) {
		return this_ecdh;
	}

	if (!(this_ecdh = EC_KEY_new_by_curve_name_d(NID_X9_62_prime256v1))) {
		log_error("Error encountered while creating new EC key.");
	}

	EC_GROUP_set_point_conversion_form_d((EC_GROUP *)EC_KEY_get0_group_d(this_ecdh), POINT_CONVERSION_COMPRESSED);

	if (keylength == 512) {
		ecdh512 = this_ecdh;
	} else {
		ecdh1024 = this_ecdh;
	}

	return this_ecdh;
}

/**
 * @brief	Get a textual representation of the last OpenSSL error message.
 * @param	buffer	a buffer that will receive the last OpenSSL error message.
 * @param	length	the size, in bytes, of the buffer that will contain the last OpenSSL error message.
 * @return	NULL on failure, or a pointer to the buffer where the last OpenSSL error message has been stored.
 */
char * ssl_error_string(chr_t *buffer, int_t length) {

	if (!buffer) {
		return NULL;
	}

	if (length < 120) {
		log_pedantic("The buffer created to hold the SSL error string should be at least 120 bytes.");
	}

	ERR_error_string_n_d(ERR_get_error_d(), buffer, length);

	return buffer;
}

/**
 * @brief	Verify that the filename contains a valid private key in PEM format.
 * @param	keyfile		the pathname of the private key.
 * @return	true if the the key is valid, or false if it is not.
 */
bool_t ssl_verify_privkey(const char *keyfile) {

    SSL_CTX *ctx = NULL;

    if (!(ctx = SSL_CTX_new_d(SSLv23_client_method_d()))) {
    	log_pedantic("Could not create a valid SSL context. {error = %s}", ssl_error_string(MEMORYBUF(512), 512));
    	return false;
    } else if (!SSL_CTX_use_PrivateKey_file_d(ctx, keyfile, SSL_FILETYPE_PEM)) {
    	log_pedantic("Could not load private key file {error = %s}", ssl_error_string(MEMORYBUF(512), 512));
		return false;
	}

    if (ctx) {
    	SSL_CTX_free_d(ctx);
    }

	return true;

}
