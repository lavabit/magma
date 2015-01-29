
#include "framework.h"

stringer_t *ssl_version = NULL;
static pthread_mutex_t **ssl_mutex;

// Functions.
extern void *lavalib;
void (*RAND_cleanup_d)(void) = NULL;
void (*SSL_free_d)(SSL *ssl) = NULL;
int (*SSL_accept_d)(SSL *ssl) = NULL;
int (*SSL_shutdown_d)(SSL *ssl) = NULL;
int (*CRYPTO_num_locks_d)(void) = NULL;
void (*ERR_free_strings_d)(void) = NULL;
void (*SSL_library_init_d)(void) = NULL;
SSL * (*SSL_new_d)(SSL_CTX *ctx) = NULL;
const EVP_MD * (*EVP_sha512_d)(void) = NULL;
void (*SSL_CTX_free_d)(SSL_CTX *ctx) = NULL;
void (*SSL_load_error_strings_d)(void) = NULL;
const char * (*SSLeay_version_d)(int t) = NULL;
SSL_METHOD * (*SSLv23_server_method_d)(void) = NULL;
SSL_CTX * (*SSL_CTX_new_d)(SSL_METHOD *method) = NULL;
int (*SSL_read_d)(SSL *ssl, void *buf, int num) = NULL;
int (*SSL_CTX_check_private_key_d)(SSL_CTX *ctx) = NULL;
int (*RAND_bytes_d)(unsigned char *buf, int num) = NULL;
BIO * (*BIO_new_socket_d)(int sock, int close_flag) = NULL;
void (*SSL_set_bio_d)(SSL *ssl, BIO *rbio, BIO *wbio) = NULL;
int (*SSL_write_d)(SSL *ssl, const void *buf, int num) = NULL;
int (*EVP_DigestInit_d)(EVP_MD_CTX *ctx, const EVP_MD *type) = NULL;
int (*RAND_load_file_d)(const char *filename, long max_bytes) = NULL;
long (*SSL_CTX_ctrl_d)(SSL_CTX *ctx, int cmd, long larg, void *parg) = NULL;
void (*CRYPTO_set_id_callback_d)(unsigned long (*id_function)(void)) = NULL;
int (*EVP_DigestUpdate_d)(EVP_MD_CTX *ctx, const void *d, unsigned int cnt) = NULL;
int (*SSL_CTX_use_certificate_chain_file_d)(SSL_CTX *ctx, const char *file) = NULL;
int (*EVP_DigestFinal_d)(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s) = NULL;
int (*SSL_CTX_use_PrivateKey_file_d)(SSL_CTX *ctx, const char *file, int type) = NULL;
void (*CRYPTO_set_locking_callback_d)(void (*locking_function)(int mode, int n, const char *file, int line));

int load_symbols_ssl(void) {

	if (lavalib == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The lava library pointer was NULL.");
		#endif
		return 0;
	}

	EVP_DigestFinal_d = dlsym(lavalib, "EVP_DigestFinal");
	if (EVP_DigestFinal_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function EVP_DigestFinal.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	EVP_DigestUpdate_d = dlsym(lavalib, "EVP_DigestUpdate");
	if (EVP_DigestUpdate_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function EVP_DigestUpdate.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	EVP_DigestInit_d = dlsym(lavalib, "EVP_DigestInit");
	if (EVP_DigestInit_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function EVP_DigestInit.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	BIO_new_socket_d = dlsym(lavalib, "BIO_new_socket");
	if (BIO_new_socket_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function BIO_new_socket.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_accept_d = dlsym(lavalib, "SSL_accept");
	if (SSL_accept_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_accept.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_free_d = dlsym(lavalib, "SSL_free");
	if (SSL_free_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_free.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_set_bio_d = dlsym(lavalib, "SSL_set_bio");
	if (SSL_set_bio_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_set_bio.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_read_d = dlsym(lavalib, "SSL_read");
	if (SSL_read_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_read.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_write_d = dlsym(lavalib, "SSL_write");
	if (SSL_write_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_write.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_CTX_free_d = dlsym(lavalib, "SSL_CTX_free");
	if (SSL_CTX_free_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_CTX_free.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_shutdown_d = dlsym(lavalib, "SSL_shutdown");
	if (SSL_shutdown_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_shutdown.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_load_error_strings_d = dlsym(lavalib, "SSL_load_error_strings");
	if (SSL_load_error_strings_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_load_error_strings.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_library_init_d = dlsym(lavalib, "SSL_library_init");
	if (SSL_library_init_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_library_init.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	ERR_free_strings_d = dlsym(lavalib, "ERR_free_strings");
	if (ERR_free_strings_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function ERR_free_strings.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	CRYPTO_num_locks_d = dlsym(lavalib, "CRYPTO_num_locks");
	if (CRYPTO_num_locks_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function CRYPTO_num_locks.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	CRYPTO_set_id_callback_d = dlsym(lavalib, "CRYPTO_set_id_callback");
	if (CRYPTO_set_id_callback_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function CRYPTO_set_id_callback.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	CRYPTO_set_locking_callback_d  = dlsym(lavalib, "CRYPTO_set_locking_callback");
	if (CRYPTO_num_locks_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function CRYPTO_set_locking_callback.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	RAND_load_file_d = dlsym(lavalib, "RAND_load_file");
	if (RAND_load_file_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function RAND_load_file.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_CTX_new_d = dlsym(lavalib, "SSL_CTX_new");
	if (SSL_CTX_new_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_CTX_new.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_CTX_check_private_key_d = dlsym(lavalib, "SSL_CTX_check_private_key");
	if (SSL_CTX_check_private_key_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_CTX_check_private_key.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSLv23_server_method_d = dlsym(lavalib, "SSLv23_server_method");
	if (SSLv23_server_method_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSLv23_server_method.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_CTX_ctrl_d = dlsym(lavalib, "SSL_CTX_ctrl");
	if (SSL_CTX_ctrl_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_CTX_ctrl_d.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_CTX_use_certificate_chain_file_d = dlsym(lavalib, "SSL_CTX_use_certificate_chain_file");
	if (SSL_CTX_use_certificate_chain_file_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_CTX_use_certificate_chain_file.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_CTX_use_PrivateKey_file_d = dlsym(lavalib, "SSL_CTX_use_PrivateKey_file");
	if (SSL_CTX_use_PrivateKey_file_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_CTX_use_PrivateKey_file.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSL_new_d = dlsym(lavalib, "SSL_new");
	if (SSL_new_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function SSL_new.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	EVP_sha512_d = dlsym(lavalib, "EVP_sha512");
	if (EVP_sha512_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function EVP_sha512.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	SSLeay_version_d = dlsym(lavalib, "SSLeay_version");
	if (SSLeay_version_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the variable SSLeay_version.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	RAND_cleanup_d = dlsym(lavalib, "RAND_cleanup");
	if (RAND_cleanup_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the variable RAND_cleanup.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	RAND_bytes_d = dlsym(lavalib, "RAND_bytes");
	if (RAND_bytes_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the variable RAND_bytes.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	#ifdef DEBUG_FRAMEWORK
	lavalog("The OpenSSL library symbols have been loaded.");
	#endif

	return 1;
}

stringer_t * version_ssl(void) {

	const char *holder;

	if (ssl_version != NULL) {
		return ssl_version;
	}

	holder = SSLeay_version_d(SSLEAY_VERSION);
	ssl_version = get_token_ns(holder, size_ns(holder), ' ', 2);

	return ssl_version;
}

inline void ssl_set_bio(SSL *ssl, BIO *rbio, BIO *wbio) {

	#ifdef DEBUG_FRAMEWORK
	if (ssl == NULL || rbio == NULL || wbio == NULL) {
		lavalog("Passed invalid data. Going to excute the function call anyways.");
	}
	#endif

	SSL_set_bio_d(ssl, rbio, wbio);

	return;
}

inline void ssl_free(SSL *ssl) {

	#ifdef DEBUG_FRAMEWORK
	if (ssl == NULL) {
		lavalog("Passed invalid data. Going to excute the function call anyways.");
	}
	#endif

	SSL_free_d(ssl);

	return;

}

inline int ssl_accept(SSL *ssl) {

	#ifdef DEBUG_FRAMEWORK
	if (ssl == NULL) {
		lavalog("Passed invalid data. Going to excute the function call anyways.");
	}
	#endif

	return SSL_accept_d(ssl);
}

inline BIO * ssl_bio_new_socket(int sock, int close_flag) {

	#ifdef DEBUG_FRAMEWORK
	if (sock < 0) {
		lavalog("Passed an invalid socket. Going to excute the function call anyways.");
	}
	#endif

	return BIO_new_socket_d(sock, close_flag);
}

inline SSL * ssl_new(SSL_CTX *ctx) {

	#ifdef DEBUG_FRAMEWORK
	if (ctx == NULL) {
		lavalog("Passed invalid data. Going to excute the function call anyways.");
	}
	#endif

	return SSL_new_d(ctx);
}

inline int ssl_shutdown(SSL *ssl) {

	#ifdef DEBUG_FRAMEWORK
	if (ssl == NULL) {
		lavalog("Passed invalid data. Going to excute the function call anyways.");
	}
	#endif

	return SSL_shutdown_d(ssl);
}

inline int ssl_read(SSL *ssl, void *buffer, int length) {

	#ifdef DEBUG_FRAMEWORK
	if (ssl == NULL || buffer == NULL || length == 0) {
		lavalog("Passed invalid data. Going to excute the function call anyways.");
	}
	#endif

	return SSL_read_d(ssl, buffer, length);
}

inline int ssl_write(SSL *ssl, const void *buffer, int length) {

	#ifdef DEBUG_FRAMEWORK
	if (ssl == NULL || buffer == NULL || length == 0) {
		lavalog("Passed invalid data. Going to excute the function call anyways.");
	}
	#endif

	return SSL_write_d(ssl, buffer, length);
}

inline void ssl_ctx_free(SSL_CTX *ctx) {

	#ifdef DEBUG_FRAMEWORK
	if (ctx == NULL) {
		lavalog("Passed invalid data. Going to excute the function call anyways.");
	}
	#endif

	SSL_CTX_free_d(ctx);

	return;
}

// Call back function used by OpenSSL to enforce thread concurrency.
void ssl_locking_callback(int mode, int type, char *file, int line) {

	// Do a comparison. This eliminates the compiler warning about unused variables.
	//if ((int)file == line) {};

	// Get a lock.
	if (mode & CRYPTO_LOCK) {
		pthread_mutex_lock(*(ssl_mutex + type));
	}
	// Release a lock.
	else {
		pthread_mutex_unlock(*(ssl_mutex + type));
	}
}

// Returns a unique thread id.
unsigned long ssl_thread_id(void) {
	return pthread_self();
}

// Initialize the SSL library.
int initialize_ssl(void) {

	int increment;

	// Get the SSL library setup.
	SSL_load_error_strings_d();
	SSL_library_init_d();

	// Thread locking setup.
	ssl_mutex = malloc(CRYPTO_num_locks_d() * sizeof(pthread_mutex_t *));
	if (ssl_mutex == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %i bytes.", CRYPTO_num_locks_d() * sizeof(pthread_mutex_t *));
		#endif
		return 0;
	}

	// Initialize all of the mutexes.
	for (increment = 0; increment < CRYPTO_num_locks_d(); increment++) {
		*(ssl_mutex + increment) = malloc(sizeof(pthread_mutex_t));
		if (*(ssl_mutex + increment) == NULL) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Could not allocate %i bytes.", sizeof(pthread_mutex_t));
			#endif
			return 0;
		}
		pthread_mutex_init(*(ssl_mutex + increment), NULL);
	}

	CRYPTO_set_id_callback_d((unsigned long (*)())ssl_thread_id);
	CRYPTO_set_locking_callback_d((void (*)())ssl_locking_callback);

	// Seed the random number generator.
	RAND_load_file_d("/dev/random", 128);

	#ifdef DEBUG_FRAMEWORK
	lavalog("SSL library initialized.");
	#endif

	return 1;
}

// Initialize for a given process. In a perfect world we call this after forking.
inline int reseed_ssl(void) {

	// Seed the random number generator. We seed again, so every process is random.
	RAND_load_file_d("/dev/random", 128);

	return 1;
}

// Free's the SSL context.
void free_ssl(void) {

	int increment;

	RAND_cleanup_d();
	ERR_free_strings_d();

	// Destroy and free all of the mutexes.
	for (increment = 0; increment < CRYPTO_num_locks_d(); increment++) {
		pthread_mutex_destroy(*(ssl_mutex + increment));
		free(*(ssl_mutex + increment));
	}

	free(ssl_mutex);

	if (ssl_version != NULL) {
		free_st(ssl_version);
		ssl_version = NULL;
	}

	#ifdef DEBUG_FRAMEWORK
	lavalog("SSL library shutdown complete.");
	#endif

}

// Setup an SSL CTX for a server.
int initialize_ssl_server(server_config_t *server) {

	server->ssl_ctx = SSL_CTX_new_d(SSLv23_server_method_d());
	if (server->ssl_ctx == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not create a valid SSL context.");
		#endif
		return 0;
	}

	// Set the CTX options. Use the underlying function call instead of SSL_CTX_set_options.
	SSL_CTX_ctrl_d(server->ssl_ctx, SSL_CTRL_OPTIONS, SSL_OP_ALL | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION | SSL_MODE_AUTO_RETRY, NULL);

	if (SSL_CTX_use_certificate_chain_file_d(server->ssl_ctx, server->certificate) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not create a valid SSL certificate chain.");
		#endif
		return 0;
	}

	if (SSL_CTX_use_PrivateKey_file_d(server->ssl_ctx, server->certificate, SSL_FILETYPE_PEM) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not associate a private key with our SSL certificate chain.");
		#endif
		return 0;
	}

	if (SSL_CTX_check_private_key_d(server->ssl_ctx) != 1) {
		lavalog("Could not verify the SSL private key. Make sure a valid key is in the configuration file.");
		return 0;
	}

	return 1;
}
