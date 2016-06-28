#ifndef SSL_H
#define SSL_H

#include <unistd.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ocsp.h>

#include "dime/common/error.h"


#define CA_FILE  "cacert.pem"
#define CA_DIR   "./"
#define CRL_FILE "crl.pem"


// The public interface.

// Initialization and finalization routines.
PUBLIC_FUNC_DECL(int,       ssl_initialize,           void);
PUBLIC_FUNC_DECL(void,      ssl_shutdown,             void);

// Generic TLS/SSL functionality.
PUBLIC_FUNC_DECL(SSL *,     ssl_connect_host,         const char *hostname, unsigned short port, int force_family);
PUBLIC_FUNC_DECL(void,      ssl_disconnect,           SSL *handle);
PUBLIC_FUNC_DECL(SSL_CTX *, ssl_get_client_context,   void);
PUBLIC_FUNC_DECL(SSL *,     ssl_starttls,             int fd);

// Public certificate validation functions.
PUBLIC_FUNC_DECL(int,       do_x509_validation,       X509 *cert, STACK_OF(X509) * chain);
PUBLIC_FUNC_DECL(int,       do_x509_hostname_check,   X509 *cert, const char *domain);
PUBLIC_FUNC_DECL(int,       do_ocsp_validation,       SSL *connection, int *fallthrough);
PUBLIC_FUNC_DECL(char *,    get_cert_subject_cn,      X509 *cert);

// Internal routines.
void         _ssl_fd_loop(SSL *connection);

// Internal certificate validation functions.
int          _verify_certificate_callback(int ok, X509_STORE_CTX *ctx);
int          _validate_self_signed(X509 *cert);
X509_STORE * _get_cert_store(void);
int          _domain_wildcard_check(const char *pattern, const char *domain);

// OCSP check routines.
void         _destroy_ocsp_response_cb(void *record);
int          _ocsp_response_callback(SSL *s, void *arg);
char *       _get_cache_ocsp_id(X509 *cert, OCSP_CERTID *cid, char *buf, size_t blen);
void *       _serialize_ocsp_response_cb(void *record, size_t *outlen);
void *       _deserialize_ocsp_response_cb(void *data, size_t len);
void         _dump_ocsp_response_cb(FILE *fp, void *record, int brief);

#endif
