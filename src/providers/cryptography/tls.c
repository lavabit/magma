
/**
 * @file /magma/providers/cryptography/tls.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 */

#include "magma.h"

/**
 * @brief	Setup an TLS CTX for a server.
 *
 * @note	The server is passed as a void pointer because the provider layer doesn't comprehend protocol specific
 * 			server instances.
 *
 * @param server_t	the TLS context will be assigned to the provided server context.
 * @param security_level	an integer which will be used to control how the TLS context is configured:
 * 							0 = accept any type of SSL or TLS protocol version, and offer broad cipher support.
 * 							1 = require TLSv1 and above, refuse SSLv2 and SSLv3 connections, use any reasonably secure cipher.
 * (reccomended)			2 = require TLSv1 and above, refuse SSLv2 and SSLv3 connections, only use ciphers which provide forward secrecy.
 * 							3 = require TLSv1.2 and limit the cipher list to ECDHE-RSA-AES256-GCM-SHA384 or ECDHE-RSA-CHACHA20-POLY1305
 * 								as required by the specifications.
 */
bool_t tls_server_create(void *server, uint_t security_level) {

	long options = 0;
	char *ciphers = NULL;
	server_t *local = server;

	if (security_level == 0) {
		options = (SSL_OP_ALL | \
			SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION | SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_MODE_AUTO_RETRY);
		ciphers = MAGMA_CIPHERS_GENERIC;
	}
	else if (security_level == 1) {
		options = (SSL_OP_ALL | SSL_OP_NO_SSLv2 | \
			SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION | SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_MODE_AUTO_RETRY);
		ciphers = SSL_DEFAULT_CIPHER_LIST;
	}
	else if (security_level == 2) {
		options = (SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | \
			SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION | SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_MODE_AUTO_RETRY | \
			SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS | SSL_OP_NO_COMPRESSION | SSL_OP_NO_TICKET);
		// SSL_OP_ALL | SSL_MODE_AUTO_RETRY | SSL_OP_TLS_ROLLBACK_BUG | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION | SSL_OP_CIPHER_SERVER_PREFERENCE
		// SSL_OP_NO_SSLv2 | SSL_OP_NO_TICKET | SSL_OP_NO_COMPRESSION | SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS
		ciphers = MAGMA_CIPHERS_MEDIUM;
	}
	else if (security_level >= 3) {
		options = (SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | \
			SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION | SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_MODE_AUTO_RETRY | \
			SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS | SSL_OP_NO_TICKET | SSL_OP_NO_COMPRESSION);
		ciphers = MAGMA_CIPHERS_HIGH;
	}

	// We use the generic SSLv23 method, which really means support SSLv2 and above, including TLSv1, TLSv1.1, etc, and then limit
	// the actual protocols the SSL context will support using the options variable configured above, and the call to SSL_CTX_ctrl() below.
	if (!(local->tls.context = SSL_CTX_new_d(SSLv23_server_method_d()))) {
		log_critical("Could not create a valid SSL context.");
		return false;
	}
	// Set the CTX options using the control function (in lieu of macros) so we can load/bind the necessary function at runtime.
	else if ((SSL_CTX_ctrl_d(local->tls.context, SSL_CTRL_OPTIONS, options, NULL) & options) != options) {
		log_critical("Could set the options mask on the TLS context.");
		return false;
	}
	// Use the cipher list selected above.
	else if (SSL_CTX_set_cipher_list_d(local->tls.context, ciphers) != 1) {
		log_critical("Could not load the default collection of ciphers.");
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

	// High security connections get 4096 bit prime when generating a DH session key.
	else if (magma.iface.cryptography.dhparams_rotate && magma.iface.cryptography.dhparams_large_keys) {
		SSL_CTX_set_tmp_dh_callback_d(local->tls.context, dh_exchange_4096);
	}
	// Otherwise use a 2048 bit prime when generating a DH session key.
	else if (magma.iface.cryptography.dhparams_rotate && !magma.iface.cryptography.dhparams_large_keys) {
		SSL_CTX_set_tmp_dh_callback_d(local->tls.context, dh_exchange_2048);
	}
	// Other use the static DH paramters.
	else if (!magma.iface.cryptography.dhparams_rotate && magma.iface.cryptography.dhparams_large_keys) {
		SSL_CTX_set_tmp_dh_callback_d(local->tls.context, dh_static_4096);
	}
	else {
		SSL_CTX_set_tmp_dh_callback_d(local->tls.context, dh_static_2048);
	}

	// Automatically select the appropriate and ECDH curve. Note the SSL_CTRL_SET_ECDH_AUTO and SSL_CTX_set_tmp_ecdh_callback()
	// configured below are no longer necessary, or relevant once we upgrade to OpenSSL 1.1.0. Curve configuration in 1.1.10
	// occurs automatically, with control over which curves are available being provided by the SSL_CTX_set1_curves interface.
//	if (SSL_CTX_ctrl_d(local->tls.context, SSL_CTRL_SET_ECDH_AUTO, 1, NULL) != 1) {
//		log_critical("Could not enable the automatic selection of ellipitical curves.");
//		return false;
//	}

	// Like the SSL_CTRL_SET_ECDH_AUTO, this function will no longer be needed when we switch to OpenSSL 1.1.0.
	SSL_CTX_set_tmp_ecdh_callback_d(local->tls.context, ssl_ecdh_exchange_callback);

	// We don't support authentication using client certificates, so we set the verify flag to NONE. This will prevent the server
	// from sending a client certificate request.
	SSL_CTX_set_verify_d(local->tls.context, SSL_VERIFY_NONE, NULL);

	// Enabling the ellipitical curve single use will improve the forward secreecy for ecdh keys.
//	else if (SSL_CTX_ctrl_d(local->tls.context, SSL_OP_SINGLE_ECDH_USE, 1, NULL) != 1) {
//		log_critical("Could not enable single use elliptical curve.");
//		return false;
//	}

	// Enable read ahead to allow for more efficient pipeline processing.
//	else if (SSL_CTX_ctrl_d(local->tls.context, SSL_CTRL_SET_READ_AHEAD, 1, NULL) != 1) {
//		log_critical("Could not enable the use of read ahead for encrypted connections.");
//		return false;
//	}

	return true;
}

/**
 * @brief	Destroy an TLS context associated with a server.
 * @param	server	the server to be deactivated.
 * @return	This function returns no value.
 */
void tls_server_destroy(void *server) {

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
 * @brief	Create a TLS session for a file descriptor, and accept the client TLS/SSL handshake.
 * @see		SSL_accept()
 * @see		BIO_new_socket()
 * @param	server	a server object which contains the underlying SSL context.
 * @param	sockd	the file descriptor of the TCP connection to be made SSL-ready.
 * @param	flags	passed to BIO_new_socket(), determines whether the socket is shut down when the BIO is freed.
 */
TLS * tls_server_alloc(void *server, int sockd, int flags) {

	SSL *tls;
	BIO *bio;
	server_t *local = server;
	int_t error = 0, result = 0, counter = 0;

	// Clear the error state, so we get accurate indications of a problem.
	errno = 0;
	ERR_clear_error_d();

#ifdef MAGMA_PEDANTIC
	if (!local) {
		log_pedantic("Passed a NULL server pointer.");
	}
	else if (!local->tls.context) {
		log_pedantic("Passed a NULL SSL context pointer.");
	}
	else if (sockd < 0) {
		log_pedantic("Passed an invalid socket. { sockd = %i }", sockd);
	}
#endif

	if (!local || !local->tls.context || sockd < 0) {
		return NULL;
	}
	else if (!(tls = SSL_new_d(local->tls.context)) || !(bio = BIO_new_socket_d(sockd, flags))) {
		log_pedantic("TLS/BIO allocation error. { error = %s }", ssl_error_string(MEMORYBUF(256), 256));

		if (tls) {
			SSL_free_d(tls);
		}

		return NULL;
	}

	SSL_set_bio_d(tls, bio, bio);
	SSL_set_accept_state_d(tls);

	// If the result code indicates a handshake error, but the TCP connection is still alive, we retry the handshake.
	do {
		// Attempt the server connection setup.
		if ((result = SSL_accept_d(tls)) <= 0 && status()) {

			switch ((error = SSL_get_error_d(tls, result))) {

				// Log these errors with extra information.
				case (SSL_ERROR_SSL):
					log_pedantic("TLS accept error. { accept = %i / error = SSL_ERROR_SSL, message = %s }", result,
						ssl_error_string(MEMORYBUF(512), 512));
					break;
				case (SSL_ERROR_SYSCALL):
					log_pedantic("TLS accept error. { accept = %i / error = SSL_ERROR_SYSCALL / errno = %i / message = %s }", result,
						errno, errno_name(errno));
					break;

				// A zero return indicates a socket shutdown. The latter should never happen.
				case (SSL_ERROR_ZERO_RETURN):
				case (SSL_ERROR_NONE):
					break;

				default:
					log_pedantic("TLS accept error. { accept = %i / error = %i }", result, error);
					break;
			}
		}

	} while (result < 0 && counter++ < 10);

	if (result != 1) {
		SSL_free_d(tls);
		return NULL;
	}

	return tls;
}

/**
 * @brief	Establish an TLS client wrapper around a socket descriptor.
 * @param	sockd	the file descriptor of the socket to have its transport security level upgraded.
 * @return	NULL on failure or a pointer to the SSL handle of the file descriptor if SSL negotiation was successful.
 */
void * tls_client_alloc(int_t sockd) {

	BIO *bio;
	SSL *tls;
	int_t result = 0, counter = 0;
	SSL_CTX *ctx = NULL;
	long options = (SSL_OP_ALL | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION | SSL_MODE_AUTO_RETRY);

	// Clear the error state, so we get accurate indications of a problem.
	errno = 0;
	ERR_clear_error_d();

	if (!(ctx = SSL_CTX_new_d(SSLv23_client_method_d()))) {
		log_pedantic("Could not create a valid TLS context. { error = %s }", ssl_error_string(MEMORYBUF(512), 512));
		return NULL;
	}
	else if ((SSL_CTX_ctrl_d(ctx, SSL_CTRL_OPTIONS, options, NULL) & options) != options) {
		log_pedantic("Could set the options mask on the TLS context. { error = %s }", ssl_error_string(MEMORYBUF(512), 512));
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

	// We don't bother with server certificate verification, just yet.
	SSL_CTX_set_verify_d(ctx, SSL_VERIFY_NONE, NULL);

	if (!(tls = SSL_new_d(ctx))) {
		log_pedantic("Could create the TLS client connection context. { error = %s }", ssl_error_string(MEMORYBUF(512), 512));
		SSL_CTX_free_d(ctx);
		return NULL;
	}
	else if (!(bio = BIO_new_socket_d(sockd, BIO_NOCLOSE))) {
		log_pedantic("Could not create the TLS client BIO context. { error = %s }", ssl_error_string(MEMORYBUF(512), 512));
		SSL_CTX_free_d(ctx);
		SSL_free_d(tls);
		return NULL;
	}

	SSL_set_bio_d(tls, bio, bio);
	SSL_set_connect_state_d(tls);

	SSL_CTX_free_d(ctx);

	do {

		// Attempt the connection. Retry if the error indicates a retryable error.
		if ((result = SSL_connect_d(tls)) < 0) {
			log_pedantic("Could not establish a TLS client connection, but the result indicates we should retry. { error = %i }", SSL_get_error_d(tls, result));
		}

		else if (result < 1) {
			log_pedantic("Could not establish a TLS client connection. { error = %s }", ssl_error_string(MEMORYBUF(512), 512));
		}

	} while (result < 0 && counter++ < 10);

	if (result != 1) {
		SSL_free_d(tls);
		return NULL;
	}

	return tls;
}

/**
 * @brief	Shutdown and free an TLS connection.
 * @param	TLS		the TLS connection to be shut down.
 * @return	This function returns no value.
 */
void tls_free(TLS *tls) {

#ifdef MAGMA_PEDANTIC
	if (!tls) {
		log_pedantic("Passed an invalid TLS pointer.");
	}
#endif

	if (tls) {
		ERR_remove_thread_state_d(0);
		SSL_shutdown_d(tls);
		SSL_free_d(tls);
	}

	return;
}

/**
 * @brief	Return the name of the TLS cipher suite assoicated with a connection.
 * @see		SSL_CIPHER_get_name()
 * @param 	tls the TLS connection being inspected.
 * @param	output	a managed string to receive the encoded output; if passed as NULL, an output buffer will be allocated
 *				which must be freed by the caller.
 * @return	a pointer to the result, or NULL if an error occurs.
 */
 stringer_t * tls_cipher(TLS *tls, stringer_t *output) {

	 SSL_CIPHER *cipher = NULL;
	 stringer_t *result = NULL;
	 chr_t *version = NULL, *name = NULL;

	 if (!(cipher = (SSL_CIPHER *)SSL_get_current_cipher_d(tls))) {
		 return NULL;
	 }
	 else if (!(version = (chr_t *)SSL_get_version_d(tls))) {
		 return NULL;
	 }
	 else if (!(name = (chr_t *)SSL_CIPHER_get_name_d(cipher))) {
		 return NULL;
	 }
	 else if (!(result = st_output(output, 128))) {
		 return NULL;
	 }

	 st_sprint(result, "version = %s / suite = %s", (st_cmp_cs_eq(NULLER(version), PLACER("TLSv1", 5)) ? version : "TLSv1.0"), name);

	 return result;
 }

/**
 * @brief	Provide the number of secret bits, and thus strength, of the TLS connection cipher suite.
 * @see		SSL_CIPHER_get_bits()
 */
 int_t tls_bits(TLS *tls) {

	 int_t bits = 0;
	 SSL_CIPHER *cipher = NULL;

	 if (!tls || !(cipher = (SSL_CIPHER *)SSL_get_current_cipher_d(tls)) || !SSL_CIPHER_get_bits_d(cipher, &bits)) {
		 return 0;
	 }

	 return bits;
 }

/**
 * @brief	Provide the SSL/TLS/DTLS version as a string constant.
 * @see		SSL_get_version()
 */
chr_t * tls_version(TLS *tls) {

	chr_t *version = NULL;
	SSL_CIPHER *cipher = NULL;

	if (!tls || !(cipher = (SSL_CIPHER *)SSL_get_current_cipher_d(tls)) || !(version = (chr_t *)SSL_get_version_d(tls))) {
		return NULL ;
	}

	return (st_cmp_cs_eq(NULLER(version), PLACER("TLSv1", 5)) ? version : "TLSv1.0");
 }

/**
 * @brief	Provide the SSL/TLS/DTLS cipher suite as a string constant.
 * @see		SSL_CIPHER_get_name()
 * @note	The RFC version of the TLS cipher suite is available through the SSL_CIPHER_standard_name() function.
 *
 */
chr_t * tls_suite(TLS *tls) {

	chr_t *suite = NULL;
	SSL_CIPHER *cipher = NULL;

	if (!tls || !(cipher = (SSL_CIPHER *)SSL_get_current_cipher_d(tls)) || !(suite = (chr_t *)SSL_CIPHER_get_name_d(cipher))) {
		 return NULL;
	}

	return suite;
 }

/**
 * @brief	Checks whether a TLS connection has been shut down or not.
 * @see		SSL_get_shutdown()
 * @param	tls		the TLS connection to be shut down.
 * @return	0 if the connection is alive and well, or SSL_SENT_SHUTDOWN/SSL_RECEIVED_SHUTDOWN
 */
int tls_status(TLS *tls) {

	int_t result = 0;

	// Look for a clean shut down of the TLS connection.
	if (tls) {
		result = SSL_get_shutdown_d(tls);
	}

	return result;
}

/**
 * @brief	Consolidate the complicated logic associated with handling SSL_read/SSL_write calls which result in 0, or a negative number.
 */
stringer_t * tls_error(TLS *tls, int_t code, stringer_t *output) {

	int tlserr = 0, syserr = 0;
	chr_t *message = MEMORYBUF(1024);
	stringer_t *result = NULL;

	// Check the SSL_get_error() function. If any of the codes listed here (0, 2 or 3) are returned, then the SSL input/output is
	// still valid, and may be used for future read and write operations.
	if ((tlserr = SSL_get_error_d(tls, code)) == SSL_ERROR_NONE || tlserr == SSL_ERROR_WANT_READ || tlserr == SSL_ERROR_WANT_WRITE) {
		return NULL;
	}

	// If we make it past here, then there should be an error message in the buffer by the time we exit this function.
	else if (!(result = st_output(output, 128))) {
		log_pedantic("Unable to record the TLS error because the output buffer is invalid.");
		return NULL;
	}

	// We need to record the errno locally to prevent any future function call from clobbering the relevant value.
	syserr = errno;

	// Note that if errno is non-zero, then the SSL error was probably just an indication we had a system level event, like a peer disconnect.
	if (syserr != 0) {
		st_sprint(result, "error = %i / errno = %i / message = %s", tlserr, syserr, errno_string(syserr, message, 1024));
	}
	// If the operation returned a negative value, but errno indicate no particular problem, then we record that scenario here.
	else if (code < 0 && syserr == 0) {
		st_sprint(result, "error = %i / errno = 0", tlserr);
	}

	// SSL error code number 1, an SSL library error occurred, usually because of a protocol problem, and thus the OpenSSL
	// error queue should hold more information about the fault.
	else if (tlserr == SSL_ERROR_SSL) {
		ERR_error_string_n_d(ERR_get_error_d(), message, 1024);
		st_sprint(result, "error = %i / errno = 0 / message = %s", tlserr, message);
	}

	// SSL error code number 5, a non-recoverable I/O error occurred. Usually errno will provide a clue, but not always.
	else if (tlserr == SSL_ERROR_SYSCALL) {
		ERR_error_string_n_d(ERR_get_error_d(), message, 1024);
		st_sprint(result, "error = %i / errno = 0 / message = %s", tlserr, message);
	}

	// SSL error code number 6, which seems to indicate the remote host shut down the connection.
	else if (tlserr == SSL_ERROR_ZERO_RETURN) {
		st_sprint(result, "error = %i / message = Connection shut down.", tlserr);
	}

	// We end up here if the result was zero, and errno was zero, and we didn't recognize the SSL error code directly.
	else {
		st_sprint(result, "error = %i / errno = 0", tlserr);
	}

	return result;
}

/**
 * @brief	Return -1 if the connection is invalid, 0 if the operation should be retried, or a positive number indicating the
 * 			number of bytes processed.
 */
int tls_continue(TLS *tls, int result, int syserror) {

	int holder = 0;
	unsigned long tlserror = 0;
	chr_t *message = MEMORYBUF(1024);

	// Check that the daemon hasn't initiated a shutdown.
	if (!status()) return -1;

	// Data was processed, so there is no need to retry the operation.
	else if (result > 0) return result;

	// Switch statement will process neutral/negative result codes.
	switch ((holder = SSL_get_error_d(tls, result))) {

		// This result is expected when no more data is expected, such as when the end-of-file terminator ir reached.
		case SSL_ERROR_ZERO_RETURN:
			// This indicates a non-error occurred, such as a timeout lapse, or shutdown/close notifcation is reccieved.
		case SSL_ERROR_NONE:
			result = -1;
			break;

			// This indicates the operation should be retried, possibly because of a renegotiation, or other out-of-band
			// interrupted the operation.
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_READ:
			result = 0;
			break;

			// A TLS error ocurred, check the error stack to find out more.
		case SSL_ERROR_SSL:
			ERR_error_string_n_d((tlserror = ERR_get_error_d()), message, 1024);
			log_pedantic("A TLS error occurred. { error = %lu / message = %s", tlserror, message);
			result = -1;
			break;

			// Indicates the call returned because of a transport error. Check errno for more information.
		case SSL_ERROR_SYSCALL:
			log_pedantic("A TCP error occurred. { errno = %i / error = %s / message = %s }", syserror, errno_name(syserror),
				strerror_r(syserror, message, 1024));
			result = -1;
			break;

		default:
			log_pedantic("An unexpected TLS error result was encountered. { error = %i }", holder);
			result = 0;
			break;
	}

	return result;
}

/**
 * @brief	Read data from a TLS connection.
 * @param	tls		the TLS connection from which the data will be read.
 * @param	buffer	a pointer to the buffer where the read data will be stored.
 * @param	length	the length, in bytes, of the amount of data to be read.
 * @param	block	a boolean variable specifying whether the read operation should block.
 * @return	-1 on failure, 0 if the connection has been closed, or the number of bytes read from the connection on success.
 */
int tls_read(TLS *tls, void *buffer, int length, bool_t block) {

	int result = 0, counter = 0;

	if (!tls || !buffer || !length) {
		log_pedantic("Passed invalid parameters for a call to the TLS read function.");
		return -1;
	}

	// In the future, when we switch to OpenSSL v1.1.0 around the year ~2032, we should look into using an
	// asynchronous SSL context to facilitate our non-blocking read/write operations. Look to configure the
	// context using SSL_CTX_set_mode(SSL_CTX *ctx, long mode) with mode set to SSL_MODE_ASYNC.
	if (!block) log_pedantic("Non-blocking TLS read calls have not been fully implemented yet.");

	do {

		errno = 0;
		ERR_clear_error_d();

		// Consult SSL_peek / SSL_want / SSL_get_read_ahead / SSL_set_read_ahead
		// if (!block) SSL_peek_d(tld...);

		result = SSL_read_d(tls, buffer, length);

	} while (block && counter++ < 8 && !(result = tls_continue(tls, result, errno)));

	return result;
}

/**
 * @brief	Write data to an open TLS connection.
 * @param	tls		the TLS connection to which the data will be written.
 * @param	buffer	a pointer to the buffer containing the data to be written.
 * @param	length	the length, in bytes, of the data to be written.
 * @return	-1 on error, or the number of bytes written to the TLS connection.
 */
int tls_write(TLS *tls, const void *buffer, int length, bool_t block) {

	int result = 0, counter = 0;

	if (!tls || !buffer || !length) {
		log_pedantic("Passed invalid parameters for a call to the TLS write function.");
		return -1;
	}

	if (!block) log_pedantic("Non-blocking TLS write calls have not been fully implemented yet.");

	do {

		errno = 0;
		ERR_clear_error_d();

		// Consult SSL_peek / SSL_want / SSL_get_read_ahead / SSL_set_read_ahead
		// if (!block) SSL_peek_d(tld...);

		result = SSL_write_d(tls, buffer, length);

	} while (block && counter++ < 8 && !(result = tls_continue(tls, result, errno)));

	return result;
}

/**
 * @brief	Write formatted data to an TLS connection.
 * @param	tls		the SSL connection to which the data will be written.
 * @param	format	a format string specifying the data to be written to the SSL connection.
 * @param	va_list	a variable argument list containing the data parameters associated with the format string.
 * @return	-1 on error, or the number of bytes written to the SSL connection.
 */
int tls_print(TLS *tls, const char *format, va_list args) {

	va_list copy;
	size_t length = 0;
	chr_t *buffer = NULL;
	int result = 0, counter = 0, bytes = 0, position = 0;

	if (!tls || !format) {
		return (!tls ? -1 : 0);
	}

	// We need to make a copy of the arguments so we can run vsnprintf twice.
	va_copy(copy, args);

	// Calculate the length of the result so we can allocate an appropriately sized buffer.
	length = vsnprintf(NULL, 0, format, copy);
	va_end(copy);

	// Make sure the print function succeeded.
	if (length <= 0) {
		return length;
	}

	// Allocate a large enough buffer.
	else if (!(buffer = mm_alloc(length + 1))) {
		return -1;
	}

	// Build the output string.
	else if (vsnprintf(buffer, length + 1, format, args) != length) {
		mm_free(buffer);
		return -1;
	}

	do {

		if ((bytes = tls_write(tls, buffer + position, length - position, true)) < 0) {
			mm_free(buffer);
			return -1;
		}

		position += bytes;

	} while (position != length && counter++ < 128 && status());

	mm_free(buffer);

	return result;
}
