#include "dime/signet-resolver/signet-ssl.h"


int ssl_initialize(void) {
    PUBLIC_FUNC_IMPL(ssl_initialize, );
}

void ssl_shutdown(void) {
    PUBLIC_FUNC_IMPL_VOID(ssl_shutdown, );
}

SSL_CTX *ssl_get_client_context(void) {
    PUBLIC_FUNC_IMPL(ssl_get_client_context, );
}

SSL *ssl_starttls(int fd) {
    PUBLIC_FUNC_IMPL(ssl_starttls, fd);
}

SSL *ssl_connect_host(const char *hostname, unsigned short port, int force_family) {
    PUBLIC_FUNC_IMPL(ssl_connect_host, hostname, port, force_family);
}

void ssl_disconnect(SSL *handle) {
    PUBLIC_FUNC_IMPL_VOID(ssl_disconnect, handle);
}

int do_x509_validation(X509 *cert, STACK_OF(X509) *chain) {
    PUBLIC_FUNC_IMPL(do_x509_validation, cert, chain);
}

int do_x509_hostname_check(X509 *cert, const char *domain) {
    PUBLIC_FUNC_IMPL(do_x509_hostname_check, cert, domain);
}

int do_ocsp_validation(SSL *connection, int *fallthrough) {
    PUBLIC_FUNC_IMPL(do_ocsp_validation, connection, fallthrough);
}

char *get_cert_subject_cn(X509 *cert) {
    PUBLIC_FUNC_IMPL(get_cert_subject_cn, cert);
}
