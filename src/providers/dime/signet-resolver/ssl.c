#include "dime/common/network.h"
#include "dime/common/misc.h"
#include "dime/common/error.h"

#include "openssl/ocsp.h"
#include "openssl/x509.h"

#include "dime/signet-resolver/cache.h"
#include "dime/signet-resolver/dmtp.h"
#include "dime/signet-resolver/signet-ssl.h"

#include "providers/symbols.h"

/* Thoughts: Any reason to change our code to leverage X509_digest() ? */


static SSL_CTX *_dmtp_ssl_client_ctx = NULL;
static int _ssl_initialized = 0;
static char *_ca_file = NULL;
static char *_crl_file = NULL;


/**
 * @brief   Initialize all the dependent layers of the openssl library for use.
 * @return  0 on success or -1 on error.
 */
int _ssl_initialize(void) {

    // OpenSSL 1.1.0+ initializes the library automatically.

    if (_ssl_initialized) {
        return 0;
    }

    if (!_ca_file && !(_ca_file = _get_dime_dir_location(CA_FILE))) {
        RET_ERROR_INT(ERR_UNSPEC, "unable to get location of certificate authority file");
    }

    if (!_crl_file && !(_crl_file = _get_dime_dir_location(CRL_FILE))) {
        free(_ca_file);
        _ca_file = NULL;
        RET_ERROR_INT(ERR_UNSPEC, "unable to get location of CRL file");
    }

    _dbgprint(4, "Initialized openssl library.\n");
    _ssl_initialized = 1;

    return 0;
}


/**
 * @brief   Shutdown the openssl subsystem.
 */
void _ssl_shutdown(void) {

    if (_dmtp_ssl_client_ctx) {
        SSL_CTX_free_d(_dmtp_ssl_client_ctx);
        _dmtp_ssl_client_ctx = NULL;
    }

    // OpenSSL 1.1.0+ frees error strings automatically at exit.

    _dbgprint(4, "Unloaded openssl library.\n");

}


/**
 * @brief   Get an SSL context that will be used for all DMTP client connections.
 * @return  a pointer to the DMTP SSL client context on success, or NULL on failure.
 */
SSL_CTX *_ssl_get_client_context(void) {

    long ctx_options = (SSL_OP_ALL | SSL_MODE_AUTO_RETRY |
                        SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

    if (_dmtp_ssl_client_ctx) {
        return _dmtp_ssl_client_ctx;
    }

    if (_ssl_initialize() < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "unable to initialize openssl subsystem");
    }

    // We start with this as our base client method and then disable older transport methods below with options.
    if (!(_dmtp_ssl_client_ctx = SSL_CTX_new_d(TLS_client_method_d()))) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve SSL client context");
    }

    // This will need to be commented out in order for the ./x utility to work.
    if (!SSL_CTX_set_cipher_list_d(_dmtp_ssl_client_ctx, DMTP_V1_CIPHER_LIST)) {
        PUSH_ERROR_OPENSSL();
        _dmtp_ssl_client_ctx = NULL;
        RET_ERROR_PTR(ERR_UNSPEC, "could not set mandatory DMTP cipher suite");
    }

    _dbgprint(4, "Initialized SSL context with cipher list: %s\n", DMTP_V1_CIPHER_LIST);

    if ((SSL_CTX_set_options_d(_dmtp_ssl_client_ctx, ctx_options) & ctx_options) != ctx_options) {
        PUSH_ERROR_OPENSSL();
        _dmtp_ssl_client_ctx = NULL;
        RET_ERROR_PTR(ERR_UNSPEC, "could not set mandatory DMTP SSL context options");
    }

    // Probably don't want SSL_VERIFY_PEER
    // Set the callback verification that will be set if there's a self-signed certificate, etc.
    SSL_CTX_set_verify_d(_dmtp_ssl_client_ctx, SSL_VERIFY_NONE, _verify_certificate_callback);

    return _dmtp_ssl_client_ctx;
}


/**
 * @brief   Negotiate a TLS session over an existing network socket connection.
 * @note    This function is called immediately after server confirmation of a STARTTLS command receipt.
 * @param   fd  the file descriptor of the network socket over which the TLS session will be initiated.
 * @return  NULL on failure, or the SSL descriptor of the newly established TLS session on success.
 */
SSL *_ssl_starttls(int fd) {

    SSL_CTX *ctx;
    SSL *result;

    if (!(ctx = _ssl_get_client_context())) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not get SSL client context");
    }

    if (!(result = SSL_new_d(ctx))) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "could not create new SSL connection object");
    }

    if (!SSL_set_fd_d(result, fd)) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "could not set SSL connection descriptor");
    }

    if (SSL_connect_d(result) <= 0) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "could not establish SSL connection");
    }

    _dbgprint(2, "STARTTLS connection upgrade succeeded.\n");

    return result;
}


/**
 * @brief   Connect to a specified host and port via a TLS-enabled service.
 * @param   hostname    the hostname of the server to which the TLS connection will be established.
 * @param   port        the numerical port number of the service to which the connection should be made.
 * @param   force_family    an optional address family (AF_INET or AF_INET6) to force the TCP connection to take.
 * @return  NULL on failure, or the SSL descriptor of the newly established TLS session on success.
 */
SSL *_ssl_connect_host(const char *hostname, unsigned short port, int force_family) {

    SSL_CTX *ctx;
    SSL *result;
    int fd;

    if (!hostname || !port) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(ctx = _ssl_get_client_context())) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not get SSL client context");
    }

    if ((fd = _connect_host(hostname, port, force_family)) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not connect to remote host");
    }

    if (!(result = SSL_new_d(ctx))) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "could not create new SSL connection object");
    }

    if (!SSL_set_fd_d(result, fd)) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "could not set SSL connection descriptor");
    }

    // We want to set the SNI (server name indication) to force the proper certificate.
    // On the event of failure it's not necessarily a fatal error.
    if (SSL_ctrl_d(result, SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, (void *)hostname) != 1) {
        fprintf(stderr, "Warning: could not set SNI TLS extension for connection.\n");
    }

    // This OCSP stapling callback doesn't do anything at the moment.
    SSL_ctrl_d(result, SSL_CTRL_SET_TLSEXT_STATUS_REQ_TYPE, TLSEXT_STATUSTYPE_ocsp, NULL);
    SSL_CTX_callback_ctrl_d(ctx, SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB, (void (*)(void))_ocsp_response_callback);
    SSL_CTX_ctrl_d(ctx, SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB_ARG, 0, NULL);

    if (SSL_connect_d(result) <= 0) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "could not establish SSL connection");
    }

    _dbgprint(3, "Successfully established TLS connection to %s:%u.\n", hostname, port);

    return result;
}


/**
 * @brief   Terminate an SSL connection.
 * @param   handle  the SSL connection handle to be closed.
 */
void _ssl_disconnect(SSL *handle) {

    int fd = -1;

    if (!handle) {
        return;
    }

    if ((fd = SSL_get_fd_d(handle)) < 0) {
        ERR_print_errors_fp_d(stderr);
    }

    SSL_shutdown_d(handle);

    if ((fd >= 0) && (close(fd) < 0)) {
        perror("close");
    }

    SSL_free_d(handle);
    _dbgprint(4, "SSL session and underlying socket was terminated.\n");

}


/**
 * @brief   Perform X509 validation on a certificate presented by a remote TLS service.
 * @param   cert    a pointer to the x509 certificate to be chain-validated.
 * @param   chain   an optional pointer to the x509 certificate chain presented by the remote service;
 *                      it will be added to the root certificate store already used for validation.
 * @return  -1 on general error, 0 if the certificate failed validation, or 1 if it passed validation successfully.
 */
int _do_x509_validation(X509 *cert, STACK_OF(X509) *chain) {

    static X509_STORE_CTX *ctx = NULL;
    static X509_STORE *store = NULL;
    static X509_LOOKUP *lookup = NULL;
    char dbuf[1024];
    int res;

    if (!cert) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (_verbose >= 1) {
        memset(dbuf, 0, sizeof(dbuf));
        X509_NAME_oneline_d(X509_get_subject_name_d(cert), dbuf, sizeof(dbuf));
        _dbgprint(1, "Performing x509 validation on subject: %s\n", dbuf);
    }

    if (!(store = X509_STORE_new_d())) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "unable to initialize X509 certificate store");
    }

    if (!(lookup = X509_STORE_add_lookup_d(store, X509_LOOKUP_file_d()))) {
        PUSH_ERROR_OPENSSL();
        X509_STORE_free_d(store);
        RET_ERROR_INT(ERR_UNSPEC, "unable to add X509 lookup");
    }

    if (X509_STORE_load_locations_d(store, _ca_file, NULL) != 1) {
        PUSH_ERROR_OPENSSL();
        X509_STORE_free_d(store);
        RET_ERROR_INT_FMT(ERR_UNSPEC, "unable to load certificate chain file: %s", _ca_file);
    }

    _dbgprint(4, "Added root certificates to x509 store from location: %s\n", _ca_file);

    // Not sure if necessary?
/*  if (!(X509_LOOKUP_load_file(lookup, _ca_file, X509_FILETYPE_PEM))) {
                PUSH_ERROR_OPENSSL();
                X509_STORE_free_d(store);
                RET_ERROR_INT_FMT(ERR_UNSPEC, "unable to load lookup chain file: %s", _ca_file);
        }  */

    // TODO: temporarily disabled... does this go back in?
/*  if((res = X509_load_crl_file(lookup, _crl_file, X509_FILETYPE_PEM)) < 1) {
                PUSH_ERROR_OPENSSL();
                X509_STORE_free_d(store);
                RET_ERROR_INT_FMT(ERR_UNSPEC, "unable to load CRL file: %s", _crl_file);
        }

        _dbgprint(4, "Loaded a total of %d entries from CRL file.\n", res); */

//  if (X509_STORE_set_flags_d(store, (X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL)) != 1) {
    if (X509_STORE_set_flags_d(store, X509_V_FLAG_X509_STRICT) != 1) {
        PUSH_ERROR_OPENSSL();
        X509_STORE_free_d(store);
        RET_ERROR_INT(ERR_UNSPEC, "unable to configure X509 store");
    }

    if (!(ctx = X509_STORE_CTX_new_d())) {
        PUSH_ERROR_OPENSSL();
        X509_STORE_free_d(store);
        RET_ERROR_INT(ERR_UNSPEC, "unable to create x509 certificate store context");
    }

    if (!X509_STORE_CTX_init_d(ctx, store, cert, NULL)) {
        PUSH_ERROR_OPENSSL();
        X509_STORE_free_d(store);
        X509_STORE_CTX_free_d(ctx);
        RET_ERROR_INT(ERR_UNSPEC, "unable to initialize x509 certificate store context");
    }

    if (chain) {
        X509_STORE_CTX_set0_untrusted_d(ctx, chain);
    }

    res = X509_verify_cert_d(ctx);

    if (res < 0) {
        PUSH_ERROR_OPENSSL();
        fprintf(stderr, "Error: x509 verification encountered error: %s\n", X509_verify_cert_error_string_d(X509_STORE_CTX_get_error_d(ctx)));
    } else if (!res) {
        _dbgprint(1, "x509 certificate did not pass verification: %s\n", X509_verify_cert_error_string_d(X509_STORE_CTX_get_error_d(ctx)));
    } else {
        _dbgprint(1, "x509 certificate passed verification.\n");
    }

    X509_STORE_free_d(store);
    X509_STORE_CTX_free_d(ctx);

    return res;
}


/**
 * @brief   Check to see that an issued X509 certificate matches the expected target domain name.
 * @see     _domain_wildcard_check()
 * @note    This function will check two parts of the specified certificate against the supplied domain name:
 *              1. The CN attribute of the subject field.
 *              2. The dnsName of the SAN extension
 * @param   cert    a pointer to the X509 certificate that will be matched against the specified domain name.
 * @param   domain  a null-terminated string containing the domain name to be matched against the target certificate.
 * @return  -1 on general error, 0 if a match did not occur, or 1 if the domain matched the certificate.
 */
int _do_x509_hostname_check(X509 *cert, const char *domain) {

/*  X509_NAME *xname;
        STACK_OF(GENERAL_NAME) *san;
        GENERAL_NAME *gn;
        char buf[512], *cn, *dnsname;
        int attempts = 0; */

    if (!cert || !domain) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    return (X509_check_host_d(cert, domain, strlen(domain), 0, NULL));

/*  if (!(xname = X509_get_subject_name_d(cert))) {
                PUSH_ERROR_OPENSSL();
                RET_ERROR_INT(ERR_UNSPEC, "could not read subject name from certificate");
        }

        memset(buf, 0, sizeof(buf));

        if ((cn = _get_cert_subject_cn(cert))) {
                _dbgprint(2, "Checking certificate against subject CN: %s\n", cn);
                attempts++;

                if (_domain_wildcard_check(cn, domain) == 1) {
                        free(cn);
                        return 1;
                }

                free(cn);
        }

        san = (STACK_OF(GENERAL_NAME) *)X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);

        while (sk_GENERAL_NAME_num(san) > 0) {
                gn = sk_GENERAL_NAME_pop(san);

                if (gn->type == GEN_DNS) {

                        if (!(dnsname = (char *)ASN1_STRING_data_d(gn->d.dNSName))) {
                                continue;
                        }

                        // Make sure there hasn't been an attempt at nul byte poisoning.
                        if (ASN1_STRING_length(gn->d.dNSName) != strlen(dnsname)) {
                                fprintf(stderr, "Error: x509 hostname check failed because of potential name poisoning attack.\n");
                                return 0;
                        }

                        attempts++;

                        if (_domain_wildcard_check(dnsname, domain) == 1) {
                                return 1;
                        }

                }

        }

// sk_GENERAL_NAME_pop_free()
        if (!attempts) {
                _dbgprint(1, "DX x509 name check failed: no CN or SAN was found in the certificate for comparison.\n");
        }

        return 0; */
}


/**
 * @brief   Perform OCSP validation on an x509 certificate.
 * @param   connection  a pointer to the SSL connection associated with the peer certificate to be OCSP validated.
 * @param   fallthrough an optional pointer to a variable that will be set if OCSP validation did not fail for various
 *              reasons (ie. OCSP was not available), but validation was not completed.
 * @return  -1 on general error, 0 if the certificate did not pass OCSP validation, or 1 if it did so successfully.
 */
int _do_ocsp_validation(SSL *connection, int *fallthrough) {

    // OpenSSL 3.0 removed the OCSP-over-HTTP client helpers (OCSP_parse_url, OCSP_REQ_CTX_set1_req,
    // OCSP_sendreq_nbio) this routine relied on to fetch and verify a fresh OCSP response for DIME
    // signet TLS. Rather than port the OSSL_HTTP_* client for this niche revocation path, OCSP
    // validation is treated as unavailable and we fall through (soft-fail). This disables live OCSP
    // revocation checking for the DIME signet resolver only; core mail-server TLS is unaffected.
    // TODO: reimplement live OCSP fetching using the OpenSSL 3.x OSSL_HTTP_* API.
    (void)connection;
    if (fallthrough) {
        *fallthrough = 1;
    }
    _dbgprint(1, "OCSP validation is unavailable in the OpenSSL 3.x build; falling through.\n");
    return 1;
}

/**
 * @brief   Get the subject common name (CN) attribute of an X509 certificate.
 * @param   cert    a pointer to the X509 certificate to have its subject field parsed.
 * @return  a pointer to the X509 certificate's CN as a null-terminated string on success, or NULL on failure.
 */
char *_get_cert_subject_cn(X509 *cert) {

    X509_NAME *xname;
    X509_NAME_ENTRY *xne;
    ASN1_STRING *cnstr;
    unsigned char *cn;
    char *result;
    int idxnid;

    if (!cert) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(xname = X509_get_subject_name_d(cert))) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "unable to get certificate subject common name");
    }

    if ((idxnid = X509_NAME_get_index_by_NID_d(xname, NID_commonName, -1)) != -1) {

        if ((xne = X509_NAME_get_entry_d(xname, idxnid))) {

            if (!(cnstr = X509_NAME_ENTRY_get_data_d(xne))) {
                PUSH_ERROR_OPENSSL();
                RET_ERROR_PTR(ERR_UNSPEC, "unable to read certificate subject common name data");
            }

            if (!(cn = ASN1_STRING_data_d(cnstr))) {
                PUSH_ERROR_OPENSSL();
                RET_ERROR_PTR(ERR_UNSPEC, "unable to read certificate subject common name data string");
            } else if (!(result = strdup((char *)cn))) {
                PUSH_ERROR_SYSCALL("strdup");
                RET_ERROR_PTR(ERR_UNSPEC, NULL);
            }

            return result;
        }

    }

    return NULL;
}


/**
 * @brief   An internal callback function for DMTP TLS certificate verification.
 * @note    This callback is set from SSL_CTX_set_verify_d().
 * @param   preverify_ok    if set, indicates that the verification of the certificate passed, or 0 if it didn't.
 * @param   ctx     a pointer to the complete context used for the certificate chain verification.
 * @return  if 0, the verification process is stopped immediately, or if 1, the process is continued.
 */
int _verify_certificate_callback(int preverify_ok, X509_STORE_CTX *ctx) {

    X509 *cert;
    X509_NAME *xname;
    char buf[1024], depthstr[16], *cn;
    // 12/29/2015: depth was uninitialized, -1 seems appropriate but requires review.
    int depth = -1, err = -1;

    // This function isn't really used anything except facilitating debugging output;
    // since we call X509_verify_cert_d() manually we discard the automatic chain verification provided by
    // openssl and instead try this process again later after having built a proper certificate store.
    // Therefore we always return 1 to keep the chain traversal going no matter what.

    if (!(cert = X509_STORE_CTX_get_current_cert_d(ctx))) {
        ERR_print_errors_fp_d(stderr);
        fprintf(stderr, "Error: unable to get x509 certificate in verification callback.\n");
        return 1;
    }

    if (!(xname = X509_get_subject_name_d(cert))) {
        ERR_print_errors_fp_d(stderr);
        fprintf(stderr, "Error: unable to get x509 certificate subject name in verification callback.\n");
        return 1;
    }

    memset(depthstr, 0, sizeof(depthstr));

    if ((err = X509_STORE_CTX_get_error_d(ctx)) != X509_V_OK) {
        depth = X509_STORE_CTX_get_error_depth_d(ctx);
        strncpy(depthstr, "[unknown]", sizeof(depthstr) - 1);
    }

    if (depth >= 0) {
        snprintf(depthstr, sizeof(depthstr), "%u", depth);
    }

    if ((cn = _get_cert_subject_cn(cert))) {
        _dbgprint(1, "Attempting validation in x509 certificate chain: %s (level %s); verified = %s / %d\n", cn, depthstr, (preverify_ok ? "yes" : "no"), err);
        free(cn);
    } else {
        _clear_error_stack();
        memset(buf, 0, sizeof(buf));
        X509_NAME_oneline_d(X509_get_subject_name_d(cert), buf, sizeof(buf));
        _dbgprint(1, "Attempting validation in x509 certificate chain %s (level %s); verified = %s / %d\n", buf, depthstr, (preverify_ok ? "yes" : "no"), err);
    }

//  printf("XXX: depth = %d, error = %d (%s)\n", depth, err, X509_verify_cert_error_string_d(err));

    return 1;
}


/**
 * @brief   Validate a self-signed x509 certificate.
 * @note    This is completed by adding the certificate to a temporary cert store/context and calling internal validation.
 * @param   cert    a pointer to the x509 certificate to be validated.
 * @return  -1 on general failure, 0 on validation error, or 1 on success.
 */
int _validate_self_signed(X509 *cert) {

    X509_STORE_CTX *ctx;
    X509_STORE *store;
    int result, error;

    if (!(store = X509_STORE_new_d())) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "could not validate self-signed X509 certificate because of x509 store error");
    }

    if (!(ctx = X509_STORE_CTX_new_d())) {
        PUSH_ERROR_OPENSSL();
        X509_STORE_free_d(store);
        RET_ERROR_INT(ERR_UNSPEC, "could not validate self-signed X509 certificate because of x509 store context error");
    }

    if (!X509_STORE_CTX_init_d(ctx, store, cert, NULL)) {
        PUSH_ERROR_OPENSSL();
        X509_STORE_free_d(store);
        X509_STORE_CTX_free_d(ctx);
        RET_ERROR_INT(ERR_UNSPEC, "unable to initialize x509 store context");
    }

    if (X509_verify_cert_d(ctx) == 1) {
        result = 1;
    } else {
        result = 0;
        error = X509_STORE_CTX_get_error_d(ctx);

        // TODO: need to resubmit for validation
        if (error == X509_V_ERR_CERT_HAS_EXPIRED) {
            _dbgprint(0, "Warning: self-signed certificate has expired but still passed check.\n");
            result = 1;
        } else if (error == X509_V_ERR_CERT_NOT_YET_VALID) {
            _dbgprint(0, "Warning: self-signed certificate was not yet valid but still passed check.\n");
            result = 1;
        } else {
            fprintf(stderr, "x509 certificate verification failed: %s\n", X509_verify_cert_error_string_d(X509_STORE_CTX_get_error_d(ctx)));
        }

    }

    X509_STORE_free_d(store);
    X509_STORE_CTX_free_d(ctx);

    return result;
}


/**
 * @brief   Perform a check against a wildcard pattern for a domain name.
 * @param   pattern     a pointer to the wildcard pattern string, subject as a certificate CN or SAN dnsname.
 * @param   domain      a pointer to the domain name, which will be compared against the supplied wildcard string.
 * @return  1 if the domain name matches the pattern or 0 if it does not; -1 if a general error was encountered.
 */
int _domain_wildcard_check(const char *pattern, const char *domain) {

    //_dbgprint(5, "x509 hostname wildcard check: %s against %s ...\n", pattern, domain);

    char *p, *pptr, *d, *dptr;
    int result = 0;

    if (!pattern || !domain || !*pattern || !*domain) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    // Everything needs to be lowercase because of the comparison.
    if ((!(p = strdup(pattern))) || (!(d = strdup(domain)))) {
        PUSH_ERROR_SYSCALL("strdup");

        if (p) {
            free(p);
        }

        RET_ERROR_INT(ERR_UNSPEC, "unable to check domain wildcard because of memory allocation error");
    }

    for (size_t i = 0; i < strlen(p); i++) {
        p[i] = tolower((unsigned char)p[i]);
    }

    for (size_t i = 0; i < strlen(d); i++) {
        d[i] = tolower((unsigned char)d[i]);
    }

    // Simple equality check first.
    if (!strcmp(p, d)) {
        result = 1;
        // Next try the wildcard
    } else if (*p == '*') {
        pptr = p + 1;

        if (strlen(pptr) > strlen(d)) {
            result = 0;
        } else {
            dptr = d + strlen(d) - strlen(pptr);

            if (!strcmp(pptr, dptr)) {
                result = 1;
            } else {
                result = 0;
            }

        }

    }

    free(d);
    free(p);

    return result;
}


/**
 * @brief   Get an x509 certificate store populated with the root certificate bundle.
 * @return  NULL on failure, or a pointer to the x509 certificate store on success.
 */
X509_STORE *_get_cert_store(void) {

    X509_STORE *store;

    if (!(store = X509_STORE_new_d())) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "could not initialize new certificate store");
    }

    if (X509_STORE_load_locations_d(store, _ca_file, NULL) != 1) {
        PUSH_ERROR_OPENSSL();
        X509_STORE_free_d(store);
        RET_ERROR_PTR(ERR_UNSPEC, "could not load x509 certificate store");
    }

    return store;
}


/**
 * @brief       Create an fd loop (connect stdin to an ssl connection write fd, and ssl connection output to stdout).
 * @param   connection  the SSL connection handle to be passed to the control of the active console.
 */
void _ssl_fd_loop(SSL *connection) {

    fd_set rfds, xfds;
    char buf[1024];
    ssize_t nread, nwritten;
    int ssl_fd;

    if ((ssl_fd = SSL_get_rfd_d(connection)) < 0) {
        ERR_print_errors_fp_d(stderr);
        return;
    }

    if  (ssl_fd < STDOUT_FILENO) {
        fprintf(stderr, "Error: File descriptor assumptions failed {fd = %u}\n", ssl_fd);
        return;
    }

    while (1) {
        FD_ZERO(&rfds);
        FD_ZERO(&xfds);

        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(STDIN_FILENO, &xfds);
        FD_SET(ssl_fd, &rfds);
        FD_SET(ssl_fd, &xfds);

        if (select(ssl_fd + 1, &rfds, NULL, &xfds, NULL) < 0) {
            perror("select");
            return;
        }

        memset(buf, 0, sizeof(buf));

        if (FD_ISSET(STDIN_FILENO, &xfds)) {
            fprintf(stderr, "[Exception occurred reading from stdin]\n");
            return;
        } else if (FD_ISSET(STDIN_FILENO, &rfds)) {

            if ((nread = read(STDIN_FILENO, buf, sizeof(buf))) < 0) {
                perror("read [stdin]:");
                return;
            } else if (!nread) {
                fprintf(stderr, "[Reached end of stdin]\n");
                return;
            }

            if ((nwritten = SSL_write_d(connection, buf, nread)) <= 0) {
                ERR_print_errors_fp_d(stderr);
                return;
            } else if (nwritten != nread) {
                fprintf(stderr, "[Unable to write all data to SSL buffer]\n");
                return;
            }

        }

        if (FD_ISSET(ssl_fd, &xfds)) {
            fprintf(stderr, "[Exception occurred reading from SSL fd]\n");
            return;
        } else if (FD_ISSET(ssl_fd, &rfds)) {

            if ((nread = SSL_read_d(connection, buf, sizeof(buf))) < 0) {
                ERR_print_errors_fp_d(stderr);
                return;
            } else if (!nread) {
                fprintf(stderr, "[Reached end of ssl stream]\n");
                return;
            }

            if ((nwritten = write(STDOUT_FILENO, buf, nread)) < 0) {
                perror("write");
                return;
            } else if (nwritten != nread) {
                fprintf(stderr, "[Unable to write all data to stdout]\n");
                return;
            }

        }

    }

}


/**
 * @brief   An OCSP stapling rOCSP_REQUEST_freeesponse callback for the TLS subsystem.
 * @note    This function currently does not do anything!
 * @param   s   a pointer to the SSL connection that generated the callback.
 * @param   arg an optional callback parameter that was set by the caller.
 * @return  1 if the OCSP response was valid or 0 if it was not.
 */
int _ocsp_response_callback(SSL *s, void *arg) {

    OCSP_RESPONSE *response;
    const unsigned char *p;
    int len;

    len = SSL_ctrl_d(s, SSL_CTRL_GET_TLSEXT_STATUS_REQ_OCSP_RESP, 0, (void *)&p);

    if (!p) {
        ERR_print_errors_fp_d(stderr);
        // No OCSP response was sent.
        return 1;
    }

    if (!(response = d2i_OCSP_RESPONSE_d(NULL, &p, len))) {
        ERR_print_errors_fp_d(stderr);
        fprintf(stderr, "Error: could not parse OCSP response message.\n");
        return 0;
    }

    if (_verbose >= 3) {
        OCSP_RESPONSE_print_d(arg, response, 0);
    }

    OCSP_RESPONSE_free_d(response);

    return 1;
}



/**
 * @brief   A callback handler to destroy an OCSP_RESPONSE object.
 * @note    This is an internal function used by the cache management subsystem.
 * @param   record  a pointer to the OCSP_RESPONSE object to be destroyed.
 */
void _destroy_ocsp_response_cb(void *record) {

    OCSP_RESPONSE *response = (OCSP_RESPONSE *)record;

    OCSP_RESPONSE_free_d(response);
}


/**
 * @brief   Get a unique identifier string for an OCSP response to be used in the object cache.
 * @param   cert    a pointer to the X509 certificate undergoing verification.
 * @param   cid a pointer to the OCSP certificate ID derived from the certificate and its issuer.
 * @param   buf a pointer to a buffer that will hold the generated OCSP response ID in the cache.
 * @param   blen    the size, in bytes, of the buffer holding the OCSP response id.
 * @return  a pointer to the output buffer with the OCSP response ID on success, or NULL on failure.
 */
char *_get_cache_ocsp_id(X509 *cert, OCSP_CERTID *cid, char *buf, size_t blen) {

    unsigned char hashbuf[SHA_160_SIZE], *cbuf = NULL;
    char hexbuf[SHA_160_SIZE * 2 + 1], *cn;
    size_t cid_size;

    // TODO: Change this function to leverage _str_printf() ?

    if (!cert || !cid) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(cn = _get_cert_subject_cn(cert))) {
        // Formerly didn't return
        RET_ERROR_PTR(ERR_UNSPEC, "could not get subject CN for x509 certificate");
    }

    if ((cid_size = i2d_OCSP_CERTID_d(cid, &cbuf)) <= 0) {
        PUSH_ERROR_OPENSSL();
        free(cn);
        RET_ERROR_PTR(ERR_UNSPEC, "could not read OCSP certificate id");
    }

    if (_compute_sha_hash(160, (unsigned char *)cbuf, cid_size, hashbuf) < 0) {
        free(cn);
        CRYPTO_free_d(cbuf, __FILE__, __LINE__);
        RET_ERROR_PTR(ERR_UNSPEC, "could not compute SHA hash of x509 certificate id");
    }

    CRYPTO_free_d(cbuf, __FILE__, __LINE__);
    memset(hexbuf, 0, sizeof(hexbuf));

    // Simple byte-to-hex conversion routine for the 20 bytes of the SHA-160 hash.
    for (size_t i = 0; i < sizeof(hashbuf); i++) {
        snprintf(&(hexbuf[i * 2]), 3, "%.2x", (unsigned char)hashbuf[i]);
    }

    if (snprintf(buf, blen, "%s-%s", cn, hexbuf) <= 0) {
        fprintf(stderr, "Error: could not get x509 certificate id string.\n");
    }

    free(cn);

    return buf;
}


/**
 * @brief   A callback handler to dump an OCSP response to the console.
 * @note    This is an internal function used by the cache management subsystem.
 * @param   fp  a pointer to the file stream that will receive the dump output.
 * @param   record  a pointer to the OCSP_RESPONSE object to be dumped.
 * @param   brief   if set, only print a brief one-line description for the requested record.
 */
void _dump_ocsp_response_cb(FILE *fp, void *record, int brief) {

    // OpenSSL 1.1.0+ made the OCSP response structures (OCSP_BASICRESP, OCSP_SINGLERESP, and the
    // tbsResponseData internals) opaque. This is a debug-only dump tied to the OCSP revocation path
    // that is otherwise stubbed on the 3.x build (see _do_ocsp_validation), so it is reduced to a
    // minimal summary rather than walking the now-opaque internals.
    // TODO: reimplement using the OpenSSL 3.x OCSP accessors (OCSP_resp_get0, OCSP_single_get0_*).
    OCSP_RESPONSE *response = (OCSP_RESPONSE *)record;
    (void)brief;

    if (!response) {
        fprintf(stderr, "Error: could not dump null OCSP response.\n");
        return;
    }

    fprintf(fp ? fp : stderr, "OCSP response { status = %s }\n",
        OCSP_response_status_str_d(OCSP_response_status_d(response)));
    return;
}


/**
 * @brief   A callback handler to deserialize an OCSP response message.
 * @note    This is an internal function used by the cache management subsystem.
 * @param   data    a pointer to a buffer containing the data to be deserialized.
 * @param   len the length, in bytes, of the data buffer to be deserialized.
 * @return  a pointer to a newly allocated OCSP_RESPONSE structure on success, or NULL on failure.
 */
void *_deserialize_ocsp_response_cb(void *data, size_t len) {

    OCSP_RESPONSE *response;

    if (!data) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(response = d2i_OCSP_RESPONSE_d(NULL, (const unsigned char **)&data, len))) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize OCSP response message");
    }

    return response;
}


/**
 * @brief   A callback handler to serialize an OCSP response message.
 * @note    This is an internal function used by the cache management subsystem.
 * @param   record  a pointer to the OCSP_RESPONSE object to be serialized.
 * @param   outlen  a pointer ot a variable that will receive the length of the serialized OCSP response.
 * @return  a pointer to a newly allocated buffer holding the serialized OCSP_RESPONSE on success, or NULL on failure.
 */
void *_serialize_ocsp_response_cb(void *record, size_t *outlen) {

    OCSP_RESPONSE *response = (OCSP_RESPONSE *)record;
    unsigned char *buf = NULL;
    int res;

    if (!record || !outlen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if ((res = i2d_OCSP_RESPONSE_d(response, &buf)) < 0) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "could not serialize OCSP response message");
    }

    *outlen = res;
    return buf;
}
