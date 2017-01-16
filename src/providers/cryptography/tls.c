
/**
 * @file /magma/src/providers/cryptography/tls.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

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
		options = (options | SSL_OP_NO_SSLv2);
		ciphers = SSL_DEFAULT_CIPHER_LIST;
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
	if (!(local->tls.context = SSL_CTX_new_d(SSLv23_server_method_d()))) {
		log_critical("Could not create a valid SSL context.");
		return false;
	}
	// Set the CTX options. We use the underlying function instead of going through the macro below,
	// since macros can't be loaded at runtime.
	// #define SSL_CTX_set_options(ctx, op) SSL_CTX_ctrl((ctx),SSL_CTRL_OPTIONS,(op),NULL)
	// Pete suggested "RC4:kEDH:HIGH:!aNULL:!eNULL:!EXP:!LOW:!SSLv2:!MD5" or using SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS to workaround the Thunderbird SSL bug.
	// Otherwise the suggestion was to use "kEDH:HIGH:RC4:!aNULL:!eNULL:!EXP:!LOW:!SSLv2:!MD5" once the stream bug is fixed.
	else if ((SSL_CTX_ctrl_d(local->tls.context, SSL_CTRL_OPTIONS, options, NULL) & options) != options) {
		log_critical("Could set the options mask on the TLS context.");
		return false;
	}
	else if (SSL_CTX_use_certificate_chain_file_d(local->tls.context, local->tls.certificate) != 1) {
		log_critical("Could not create a valid TLS certificate chain using the file %s.", local->tls.certificate);
		return false;
	}
	else if (SSL_CTX_use_PrivateKey_file_d(local->tls.context, local->tls.certificate, SSL_FILETYPE_PEM) != 1) {
		log_critical("Could not load the private key for our TLS certificate chain using the file %s.", local->tls.certificate);
		return false;
	}
	else if (SSL_CTX_check_private_key_d(local->tls.context) != 1) {
		log_critical("Could not verify the SSL private key. Make sure a valid private key is in the file %s.", local->tls.certificate);
		return false;
	}
	// We had some compatibility issues enabling PFS, so this needs to be resolved soon.
	else if (SSL_CTX_set_cipher_list_d(local->tls.context, ciphers) != 1) {
		log_critical("Could not load the default collection of ciphers.");
		return false;
	}
	else if (SSL_CTX_ctrl_d(local->tls.context, SSL_CTRL_SET_ECDH_AUTO, 1, NULL) != 1) {
		log_critical("Could not enable the automatic, default selection of the strongest curve.");
		return false;
	}

	// High security connections get 4096 bit prime when generating a DH session key.
	else if (magma.iface.cryptography.dhparams_large_keys) {
		SSL_CTX_set_tmp_dh_callback_d(local->tls.context, dh_exchange_4096);
	}
	// Otherwise use a 2048 bit prime when generating a DH session key.
	else {
		SSL_CTX_set_tmp_dh_callback_d(local->tls.context, dh_exchange_2048);
	}

	/// TODO: The SSL_CTX_set_tmp_ecdh_callback() may no longer be needed with SSL_CTX_set_ecdh_auto(). More research is needed.
	SSL_CTX_set_tmp_ecdh_callback_d(local->tls.context, ssl_ecdh_exchange_callback);

	return true;
}

/**
 * @brief	Destroy an SSL context associated with a server.
 * @param	server	the server to be deactivated.
 * @return	This function returns no value.
 */
void ssl_server_destroy(void *server) {

	server_t *local = server;

#ifdef MAGMA_PEDANTIC
	if (!local || !local->tls.context) {
		log_pedantic("Passed invalid data. Going to execute the function call anyways.");
	}
#endif

	SSL_CTX_free_d(local->tls.context);

	return;
}

/**
 * @brief	Create an SSL session for a file descriptor, and accept the client TLS/SSL handshake.
 * @see		SSL_accept()
 * @see		BIO_new_socket()
 * @param	server	a server object which contains the underlying SSL context.
 * @param	sockd	the file descriptor of the TCP connection to be made SSL-ready.
 * @param	flags	passed to BIO_new_socket(), determines whether the socket is shut down when the BIO is freed.
 */
SSL * tls_server_alloc(void *server, int sockd, int flags) {

	SSL *tls;
	BIO *bio;
	int result = 0;
	server_t *local = server;

#ifdef MAGMA_PEDANTIC
	if (!local) {
		log_pedantic("Passed a NULL server pointer.");
	} else if (!local->tls.context) {
		log_pedantic("Passed a NULL SSL context pointer.");
	}
	else if (sockd < 0) {
		log_pedantic("Passed an invalid socket. {sockd = %i}", sockd);
	}
#endif

	if (!local || !local->tls.context || sockd < 0) {
		return NULL;
	} else if (!(tls = SSL_new_d(local->tls.context)) || !(bio = BIO_new_socket_d(sockd, flags))) {
		log_pedantic("SSL/BIO allocation error. {error = %s}", ssl_error_string(MEMORYBUF(256), 256));

		if (tls) {
			SSL_free_d(tls);
		}

		return NULL;
	}

	SSL_set_bio_d(tls, bio, bio);

	if ((result = SSL_accept_d(tls)) != 1) {
		log_pedantic("SSL accept error. { accept = %i / error = %s }", result, ssl_error_string(MEMORYBUF(256), 256));
		SSL_free_d(tls);
		return NULL;
	}

	return tls;
}

/**
 * @brief	Establish an SSL client wrapper around a socket descriptor.
 * @param	sockd	the file descriptor of the socket to have its transport security level upgraded.
 * @return	NULL on failure or a pointer to the SSL handle of the file descriptor if SSL negotiation was successful.
 */
void * tls_client_alloc(int_t sockd) {

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

	/// LOW: Add requisite config options and sandbox resources to verify server TLS certificates.
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
 * @brief	Shutdown and free an SSL connection.
 * @param	ssl		the SSL connection to be shut down.
 * @return	This function returns no value.
 *
 */
void tls_free(SSL *ssl) {

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
int tls_status(SSL *ssl) {

	int_t result = 0;
	if (ssl) {
		result = SSL_get_shutdown_d(ssl);
	}
	return result;
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

	int result = 0, sslerr;

	if (!ssl || !buffer || !length) {
		log_pedantic("Passed invalid data for the SSL_read function.");
		return -1;
	}

	if (!block) {

		// In the future, when we switch to OpenSSL v1.1.0 around the year ~2032, we should look into using an
		// asynchronous SSL context to facilitate our non-blocking read/write operations. Look to configure the
		// context using SSL_CTX_set_mode(SSL_CTX *ctx, long mode) with mode set to SSL_MODE_ASYNC.

		// Method One.
		//int fd = SSL_get_rfd_d(ssl);
		//if (recv(fd, buffer, length, MSG_PEEK | MSG_DONTWAIT) != 0) {
			result = SSL_read_d(ssl, buffer, length);
			if (result <= 0 && (sslerr = SSL_get_error_d(ssl, result)) != SSL_ERROR_WANT_READ) {
				ERR_error_string_n_d(sslerr, bufptr, buflen);
				log_pedantic("SSL_read error. { result = %i / error = %s }", result, bufptr);
			}
			else if (result < 0) {
				result = 0;
			}
		//}
		//else {
		//	result = 0;
		//}

		// Method two.
//		if (SSL_want_d(ssl) == SSL_READING || SSL_pending_d(ssl) > 0) {
//			result = SSL_read_d(ssl, buffer, length);
//		}

		// If our socket is blocking and we get this error it's not really an error.. it just means we need to try again.


	}
	else if (block && (result = SSL_read_d(ssl, buffer, length)) <= 0) {
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
	else if ((result = SSL_write_d(ssl, buffer, length)) <= 0) {

		// This might not really be an "error" ...
		if ((sslerr = SSL_get_error_d(ssl, result)) != SSL_ERROR_WANT_WRITE) {
			log_pedantic("SSL_write error. {sslerr = %i / error = %s}", sslerr, ssl_error_string(bufptr, buflen));
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
