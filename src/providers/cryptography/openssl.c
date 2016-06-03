
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
pthread_mutex_t **ssl_locks = NULL;

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
		M_BIND(EC_POINT_point2oct),	M_BIND(ENGINE_cleanup),	M_BIND(ERR_error_string), M_BIND(ERR_error_string_n), M_BIND(ERR_free_strings),
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
		M_BIND(SSL_get_shutdown), M_BIND(SSL_get_wbio), M_BIND(SSL_library_init), M_BIND(SSL_load_error_strings), M_BIND(SSL_new), M_BIND(SSL_peek),
		M_BIND(SSL_read), M_BIND(SSL_set_bio), M_BIND(SSL_shutdown), M_BIND(SSLv23_client_method), M_BIND(SSLv23_server_method),
		M_BIND(SSL_version_str), M_BIND(SSL_write), M_BIND(TLSv1_server_method), M_BIND(X509_get_ext), M_BIND(X509_get_ext_count),
		M_BIND(X509_get_subject_name), M_BIND(X509_NAME_get_text_by_NID), M_BIND(EVP_MD_type)
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
 * @brief	Read data from an SSL connection.
 * @param	ssl		the SSL connection from which the data will be read.
 * @param	buffer	a pointer to the buffer where the read data will be stored.
 * @param	length	the length, in bytes, of the amount of data to be read.
 * @param	block	a boolean variable specifying whether the read operation should block.
 * @return	-1 on failure, 0 if the connection has been closed, or the number of bytes read from the connection on success.
 */
int ssl_read(SSL *ssl, void *buffer, int length, bool_t block) {

	int result = -1, sslerr;

	if (!ssl || !buffer || !length) {
		log_pedantic("Passed invalid data for the SSL_read function.");
		return -1;
	}

	if ((!block && (result = SSL_peek_d(ssl, buffer, length)) <= 0) || (result = SSL_read_d(ssl, buffer, length)) <= 0) {

		// If our socket is blocking and we get this error it's not really an error.. it just means we need to try again.
		if ((sslerr = SSL_get_error_d(ssl, result)) != SSL_ERROR_WANT_READ) {
			ERR_error_string_n_d(sslerr, bufptr, buflen);
			log_pedantic("SSL_read error. { result = %i / error = %s }", result, bufptr);
		}
		else {
			result = 0;
		}

	}
	else if (result < 0) {
		log_pedantic("SSL connection cleanly shutdown.");
	}

	return result;
}

/**
 * @brief	Write data to an open SSL connection.
 * @param	ssl		the SSL connection to which the data will be written.
 * @param	buffer	a pointer to the buffer containing the data to be written.
 * @param	length	the length, in bytes, of the data to be written.
 * @return	-1 on error, or the number of bytes written to the SSL connection.
 */
int ssl_write(SSL *ssl, const void *buffer, int length) {

	int result = -1, sslerr;

	if (!ssl || !buffer || !length) {
		log_pedantic("Passed invalid data for the SSL_write function.");
		return -1;
	}
	else if ((result = SSL_write_d(ssl, buffer, length)) < 0) {

		// This might not really be an "error" ...
		if ((sslerr = SSL_get_error_d(ssl, result)) != SSL_ERROR_WANT_WRITE) {
			log_pedantic("SSL_write error. {error = %s}", ssl_error_string(bufptr, buflen));
		}

	}
	else if (!result) {
		log_pedantic("SSL connection cleanly shutdown.");
	}

	return result;
}

/**
 * @brief	Write formatted data to an SSL connection.
 * @param	ssl		the SSL connection to which the data will be written.
 * @param	format	a format string specifying the data to be written to the SSL connection.
 * @param	va_list	a variable argument list containing the data parameters associated with the format string.
 * @return	-1 on error, or the number of bytes written to the SSL connection.
 */
int ssl_print(SSL *ssl, const char *format, va_list args) {

	int result;
	char *buffer;
	size_t length;
	va_list copy;

	if (!ssl) {
		return -1;
	} else if (!format) {
		return 0;
	}

	// We need to make a copy of the arguments list in case we need to run vsnprintf twice.
	va_copy(copy, args);

	// See if the string will fit inside the standard thread buffer.
	if ((length = vsnprintf(bufptr, buflen, format, args)) < buflen) {
		va_end(copy);
		return ssl_write(ssl, bufptr, length);
	}

	// Allocate a large enough buffer.
	else if (!(buffer = mm_alloc(length + 1))) {
		va_end(copy);
		return -1;
	}

	// Try building the string again.
	else if (vsnprintf(buffer, length, format, copy) != length) {
		mm_free(buffer);
		va_end(copy);
		return -1;
	}

	result = ssl_write(ssl, buffer, length);
	va_end(copy);
	mm_free(buffer);

	return result;
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
 * @brief	Shutdown and free an SSL connection.
 * @param	ssl		the SSL connection to be shut down.
 * @return	This function returns no value.
 *
 */
void ssl_free(SSL *ssl) {

#ifdef MAGMA_PEDANTIC
	if (!ssl) {
		log_pedantic("Passed a NULL SSL pointer.");
	}
#endif

	if (ssl) {
		ERR_remove_thread_state_d(0);
		SSL_shutdown_d(ssl);
		SSL_free_d(ssl);
	}

	return;
}

/**
 * @brief	Checks whether an SSL tunnel has been shut down or not.
 * @see		SSL_get_shutdown()
 * @param	ssl		the SSL connection to be shut down.
 * @return	0 if the connection is alive and well, or SSL_SENT_SHUTDOWN/SSL_RECEIVED_SHUTDOWN
 */
int ssl_shutdown_get(SSL *ssl) {

	int_t result = 0;
	if (ssl) {
		result = SSL_get_shutdown_d(ssl);
	}
	return result;
}

/**
 * @brief	Create an SSL session for a file descriptor, and accept the client TLS/SSL handshake.
 * @see		SSL_accept()
 * @see		BIO_new_socket()
 * @param	server	a server object which contains the underlying SSL context.
 * @param	sockd	the file descriptor of the TCP connection to be made SSL-ready.
 * @param	flags	passed to BIO_new_socket(), determines whether the socket is shut down when the BIO is freed.
 */
SSL * ssl_alloc(void *server, int sockd, int flags) {

	SSL *ssl;
	BIO *bio;
	server_t *local = server;

#ifdef MAGMA_PEDANTIC
	if (!local) {
		log_pedantic("Passed a NULL server pointer.");
	} else if (!local->ssl.context) {
		log_pedantic("Passed a NULL SSL context pointer.");
	}
	else if (sockd < 0) {
		log_pedantic("Passed an invalid socket. {sockd = %i}", sockd);
	}
#endif

	if (!local || !local->ssl.context || sockd < 0) {
		return NULL;
	} else if (!(ssl = SSL_new_d(local->ssl.context)) || !(bio = BIO_new_socket_d(sockd, flags))) {
		log_pedantic("SSL/BIO allocation error. {error = %s}", ssl_error_string(bufptr, buflen));

		if (ssl) {
			SSL_free_d(ssl);
		}

		return NULL;
	}

	SSL_set_bio_d(ssl, bio, bio);

	if (SSL_accept_d(ssl) != 1) {
		log_pedantic("SSL accept error. {error = %s}", ssl_error_string(bufptr, buflen));
		SSL_free_d(ssl);
		return NULL;
	}

	return ssl;
}

/**
 * @brief	Destroy an SSL context associated with a server.
 * @param	server	the server to be deactivated.
 * @return	This function returns no value.
 */
void ssl_server_destroy(void *server) {

	server_t *local = server;

#ifdef MAGMA_PEDANTIC
	if (!local || !local->ssl.context) {
		log_pedantic("Passed invalid data. Going to execute the function call anyways.");
	}
#endif

	SSL_CTX_free_d(local->ssl.context);

	return;
}

/**
 * @brief	Establish an SSL client wrapper around a socket descriptor.
 * @param	sockd	the file descriptor of the socket to have its transport security level upgraded.
 * @return	NULL on failure or a pointer to the SSL handle of the file descriptor if SSL negotiation was successful.
 */
void * ssl_client_create(int_t sockd) {

	BIO *bio;
	SSL *result;
	SSL_CTX *ctx = NULL;
	long options = (SSL_OP_ALL | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION | SSL_MODE_AUTO_RETRY);

	if (!(ctx = SSL_CTX_new_d(SSLv23_client_method_d()))) {
		log_pedantic("Could not create a valid SSL context. {error = %s}", ssl_error_string(MEMORYBUF(512), 512));
		return NULL;
	}
	else if ((SSL_CTX_ctrl_d(ctx, SSL_CTRL_OPTIONS, options, NULL) & options) != options) {
		log_pedantic("Could set the options mask on the SSL context. {error = %s}", ssl_error_string(MEMORYBUF(512), 512));
		SSL_CTX_free_d(ctx);
		return NULL;
	}

	/// LOW: Add requisite config options and sandbox resources to verify server SSL certificates.
	// SSL_CTX_load_verify_locations(result, SSL_CAFILE, SSL_CAPATH);
	// lookup = X509_STORE_add_lookup(SSL_CTX_get_cert_store(result.context), X509_LOOKUP_file());
	// X509_load_crl_file(lookup, SSL_CRLFILE, X509_FILETYPE_PEM);
	// X509_STORE_set_flags(SSL_CTX_get_cert_store(result.context), X509_V_FLAG_CRL_CHECK|X509_V_FLAG_CRL_CHECK_ALL);
	// mode = SSL_VERIFY_NONE or SSL_VERIFY_PEER
	// SSL_CTX_set_verify(result.context, mode, cert_verify_callback);

	else if (!(result = SSL_new_d(ctx))) {
		log_pedantic("Could create the SSL client connection context. {error = %s}", ssl_error_string(MEMORYBUF(512), 512));
		SSL_CTX_free_d(ctx);
		return NULL;
	}
	else if (!(bio = BIO_new_socket_d(sockd, BIO_NOCLOSE))) {
		log_pedantic("Could not create the SSL client BIO context. {error = %s}", ssl_error_string(MEMORYBUF(512), 512));
		SSL_free_d(result);
		SSL_CTX_free_d(ctx);
		return NULL;
	}

	SSL_set_bio_d(result, bio, bio);
	SSL_CTX_free_d(ctx);

	if (SSL_connect_d(result) != 1) {
		log_pedantic("Could not establish an SSL connection with the client. {error = %s}", ssl_error_string(MEMORYBUF(512), 512));
		SSL_free_d(result);
		return NULL;
	}

	return result;
}

/**
 * @brief	Setup an SSL CTX for a server.
 *
 * @note	The server is passed as a void pointer because the provider layer doesn't comprehend protocol specific
 * 			server instances.
 *
 * @param server_t	the SSL context will be assigned to the provided server context.
 * @param security_level	an integer which will be used to control how the SSL context is configured:
 * 							0 = accept any type of SSL or TLS protocol version, and offer broad cipher support.
 * 							1 = require TLSv1 and above, refuse SSLv2 and SSLv3 connections, use any reasonably secure cipher.
 * (reccomended)			2 = require TLSv1 and above, refuse SSLv2 and SSLv3 connections, only use ciphers which provide forward secrecy.
 * 							3 = require TLSv1.2 and limit the cipher list to ECDHE-RSA-AES256-GCM-SHA384 or ECDHE-RSA-CHACHA20-POLY1305
 * 								as required by the specifications.
 *
 */
bool_t ssl_server_create(void *server, uint_t security_level) {

	long options = 0;
	char *ciphers = NULL;
	server_t *local = server;

	// TODO: Add SSL_OP_SINGLE_ECDH_USE | SSL_OP_SINGLE_DH_USE | SSL_OP_EPHEMERAL_RSA but that means adding callbacks, and possibly updating the certificate.
	options = (SSL_OP_ALL | SSL_MODE_AUTO_RETRY | SSL_OP_TLS_ROLLBACK_BUG | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION | SSL_OP_CIPHER_SERVER_PREFERENCE);

	if (security_level == 0) {
		options = (options);
		ciphers = MAGMA_CIPHERS_GENERIC;
	}
	else if (security_level == 1) {
		options = (options | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
		ciphers = MAGMA_CIPHERS_LOW;
	}
	else if (security_level == 2) {
		options = (options | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);
		ciphers = MAGMA_CIPHERS_MEDIUM;
	}
	else if (security_level >= 3) {
		options = (options | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_COMPRESSION);
		ciphers = MAGMA_CIPHERS_HIGH;
	}

	// We use the generic SSLv23 method, which really means support SSLv2 and above, including TLSv1, TLSv1.1, etc, and then limit
	// the actual protocols the SSL context will support using the options variable configured above, and the call to SSL_CTX_ctrl() below.
	if (!(local->ssl.context = SSL_CTX_new_d(SSLv23_server_method_d()))) {
		log_critical("Could not create a valid SSL context.");
		return false;
	}
	// Set the CTX options. We use the underlying function instead of going through the macro below,
	// since macros can't be loaded at runtime.
	// #define SSL_CTX_set_options(ctx, op) SSL_CTX_ctrl((ctx),SSL_CTRL_OPTIONS,(op),NULL)
	// Pete suggested "RC4:kEDH:HIGH:!aNULL:!eNULL:!EXP:!LOW:!SSLv2:!MD5" or using SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS to workaround the Thunderbird SSL bug.
	// Otherwise the suggestion was to use "kEDH:HIGH:RC4:!aNULL:!eNULL:!EXP:!LOW:!SSLv2:!MD5" once the stream bug is fixed.
	else if ((SSL_CTX_ctrl_d(local->ssl.context, SSL_CTRL_OPTIONS, options, NULL) & options) != options) {
		log_critical("Could set the options mask on the SSL context.");
		return false;
	}
	else if (SSL_CTX_use_certificate_chain_file_d(local->ssl.context, local->ssl.certificate) != 1) {
		log_critical("Could not create a valid SSL certificate chain using the file %s.", local->ssl.certificate);
		return false;
	}
	else if (SSL_CTX_use_PrivateKey_file_d(local->ssl.context, local->ssl.certificate, SSL_FILETYPE_PEM) != 1) {
		log_critical("Could not load the private key for our SSL certificate chain using the file %s.", local->ssl.certificate);
		return false;
	}
	else if (SSL_CTX_check_private_key_d(local->ssl.context) != 1) {
		log_critical("Could not verify the SSL private key. Make sure a valid private key is in the file %s.", local->ssl.certificate);
		return false;
	}
	// We had some compatibility issues enabling PFS, so this needs to be resolved soon.
	else if (SSL_CTX_set_cipher_list_d(local->ssl.context, ciphers) != 1) {
		log_critical("Could not load the default collection of ciphers.");
		return false;
	}
	else if (SSL_CTX_ctrl_d(local->ssl.context, SSL_CTRL_SET_ECDH_AUTO, 1, NULL) != 1) {
		log_critical("Could not enable the automatic, default selection of the strongest curve.");
		return false;
	}

	SSL_CTX_set_tmp_dh_callback_d(local->ssl.context, ssl_dh_exchange_callback);

	/// TODO: The SSL_CTX_set_tmp_ecdh_callback() may no longer be needed with SSL_CTX_set_ecdh_auto(). More research is needed.
	SSL_CTX_set_tmp_ecdh_callback_d(local->ssl.context, ssl_ecdh_exchange_callback);

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

		mm_free(ssl_locks);
		ssl_locks = NULL;
	}

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
	//if ((int)file == line) {};

	// Get a lock.
	if (mode & CRYPTO_LOCK) {
		mutex_get_lock(*(ssl_locks + n));
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
 * @brief	Initialize the OpenSSL facility.
 * @note	First, this function initializes the mutexes necessary for the locking function callback that openssl uses
 * 			for shared data structures in multi-threaded applications.
 * 			Next, the DKIM key is retrieved if the magma.dkim.enabled configuration variable is set.
 * @return	true if openssl was initialized successfully, or false on failure.
 */
bool_t ssl_start(void) {

	stringer_t *keyname;

	// Thread locking setup.
	if (!(ssl_locks = mm_alloc(CRYPTO_num_locks_d() * sizeof(pthread_mutex_t *)))) {
		log_critical("Could not allocate %zu bytes for the SSL library locks.", CRYPTO_num_locks_d() * sizeof(pthread_mutex_t *));
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

	// This must be done here because we have to wait for openssl to be initialized first.
	if (magma.dkim.enabled) {
		keyname = magma.dkim.privkey;

		if (file_world_accessible(st_char_get(keyname))) {
			log_critical("Warning: DKIM private key has world-access file permissions! Please fix. { path = %s }", st_char_get(keyname));
		}

		if (!ssl_verify_privkey(st_char_get(keyname))) {
			log_critical("Unable to validate DKIM private key: %s", st_char_get(keyname));
			return false;
		} else if (!(magma.dkim.privkey = file_load(st_char_get(keyname)))) {
			log_critical("Unable to load DKIM private key contents from file: %s", st_char_get(keyname));
			return false;
		}

		st_free(keyname);
	}

	return true;
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

/**
 * @brief	Callback handler for the Diffie-Hellman parameter generation process necessary for PFS.
 * @param	ssl			the SSL session for which the callback was triggered.
 * @param	is_export	LOL. We're definitely ignoring this parameter.
 * @param	keylength	the length, in bits, of the DH key to be generated.
 * @return	a pointer to a Diffie-Hellman key of proper size with parameters generated, or NULL on failure.
 */
DH * ssl_dh_exchange_callback(SSL *ssl, int is_export, int keylength) {

	static DH *dh512 = NULL, *dh1024 = NULL, *this_dh;
	BN_GENCB cb;

	if (keylength != 512 && keylength != 1024) {
		log_error("Diffie-Hellman key generation failed; only 512/1024 bit keys are supported but %u were requested.", (unsigned int)keylength);
		return NULL;
	}

	// If the generated key was already cached, simply return it.
	if ((this_dh = (keylength == 512) ? dh512 : dh1024)) {
		return this_dh;
	}

	if (!(this_dh = DH_new_d())) {
		log_error("Unable to create new DH key.");
		return NULL;
	}

	BN_GENCB_set(&cb, ssl_dh_generate_callback, NULL);

	log_pedantic("Generating %u bit DH key parameters.", (unsigned int)keylength);

	if (!DH_generate_parameters_ex_d(this_dh, keylength, 2, &cb)) {
		log_error("Encountered error while generating Diffie-Hellman parameters.");

		if (this_dh) {
			DH_free_d(this_dh);
		}

		return NULL;
	}

	if (keylength == 512) {
		dh512 = this_dh;
	} else {
		dh1024 = this_dh;
	}

	return this_dh;
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
 * @brief	The DH param generation callback.
 *
 *
 */
int ssl_dh_generate_callback(int p, int n, BN_GENCB *cb) {

		return 1;
}
