#include <openssl/x509v3.h>

#include "dime/signet-resolver/dmtp.h"
#include "dime/signet-resolver/cache.h"
#include "dime/signet-resolver/dns.h"
#include "dime/signet-resolver/mrec.h"

#include "dime/common/network.h"
#include "dime/common/misc.h"
#include "dime/common/error.h"


signet_t *_get_signet(const char *name, const char *fingerprint, int use_cache) {

    dmtp_session_t *session;
    signet_t *result, *org_signet = NULL;
    cached_object_t *cached;
    const char *org;
    char *line;
    int res, is_org = 0;

    if (!name) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(org = strchr(name, '@'))) {
        org = name;
        is_org = 1;
    } else {
        org++;

        // The org address cannot also contain a '@' ...
        if (strchr(org, '@')) {
            RET_ERROR_PTR(ERR_BAD_PARAM, "invalid organizational signet address");
        }

    }

    // TODO: Need to check fingerprint of cached object, if it is found.
    // If we're told to use the cache, first check to see if the signet is already in it.
    if (use_cache && ((cached = _find_cached_object(name, &(cached_stores[cached_data_signet]))))) {
        result = ((signet_t *)_get_cache_obj_data(cached));
        return result;
        // In case this returned an error.
    } else if (use_cache) {

        if (get_last_error()) {
            dump_error_stack();
        }

        _clear_error_stack();
    }

    // If not, we have to do a lookup via DMTP.
    if (!(session = _sgnt_resolv_dmtp_connect(org, 0))) {
        RET_ERROR_PTR(ERR_UNSPEC, "unable to connect to DX server");
    }

    if ((res = _verify_dx_certificate(session)) < 0) {
        _sgnt_resolv_destroy_dmtp_session(session);
        RET_ERROR_PTR(ERR_UNSPEC, "error encoutered during the DX certificate verification process");
    } else if (!res) {
        _sgnt_resolv_destroy_dmtp_session(session);
        RET_ERROR_PTR(ERR_UNSPEC, "DX certificate verification failed");
    }

    // If we're requesting a user signet, then we need to fetch the org signet first and verify against it.
    if (!is_org) {

        if (!(line = _sgnt_resolv_dmtp_get_signet(session, org, NULL))) {
            _sgnt_resolv_destroy_dmtp_session(session);
            RET_ERROR_PTR(ERR_UNSPEC, "org signet retrieval for user signet verification failed");
        }

        if (!(org_signet = dime_sgnt_signet_b64_deserialize(line))) {
            free(line);
            _sgnt_resolv_destroy_dmtp_session(session);
            RET_ERROR_PTR(ERR_UNSPEC, "org signet deserialization for user signet verification failed");
        }

        free(line);

        if (dime_sgnt_validate_all(org_signet, NULL, NULL, (const unsigned char **)session->drec->pubkey) != SS_FULL) {
            _sgnt_resolv_destroy_dmtp_session(session);
            RET_ERROR_PTR(ERR_UNSPEC, "org signet could not be verified against DIME management record POK");
        }

        _dbgprint(1, "Org signet validation succeeded for: %s\n", org);
    }

    if (!(line = _sgnt_resolv_dmtp_get_signet(session, name, fingerprint))) {
        _sgnt_resolv_destroy_dmtp_session(session);
        RET_ERROR_PTR(ERR_UNSPEC, "signet retrieval failed");
    }

    if (!(result = dime_sgnt_signet_b64_deserialize(line))) {
        free(line);
        _sgnt_resolv_destroy_dmtp_session(session);
        RET_ERROR_PTR(ERR_UNSPEC, "unable to decode signet received from server");
    }

    free(line);

//  if (is_org && (sig_pok_compare(result, (const unsigned char **)session->drec->pubkey) < 0)) {
    if (is_org && (dime_sgnt_validate_all(result, NULL, NULL, (const unsigned char **)session->drec->pubkey) != SS_FULL)) {
        _sgnt_resolv_destroy_dmtp_session(session);
        dime_sgnt_signet_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "org signet could not be verified against DIME management record POK");
    } else if (is_org) {
        _dbgprint(1, "Org signet validation succeeded for: %s\n", name);
    } else if (!is_org && (dime_sgnt_validate_all(result, NULL, org_signet, NULL) != SS_FULL)) {
        _sgnt_resolv_destroy_dmtp_session(session);
        dime_sgnt_signet_destroy(result);
        dime_sgnt_signet_destroy(org_signet);
        RET_ERROR_PTR(ERR_UNSPEC, "user signet could not be verified against org signet");
    } else if (!is_org) {
        _dbgprint(1, "User signet validation succeeded for: %s\n", name);
    }

    _sgnt_resolv_destroy_dmtp_session(session);

    if (org_signet) {
        dime_sgnt_signet_destroy(org_signet);
    }

    if (use_cache) {

        if (!(cached = _add_cached_object(name, &(cached_stores[cached_data_signet]), 0, 0, result, 1, 0))) {
            fprintf(stderr, "Error adding signet to object cache: %s", name);
            dump_error_stack();
            _clear_error_stack();
            return result;
        }

        if (_save_cache_contents() < 0) {
            fprintf(stderr, "Error: could not save cache contents.\n");
            dump_error_stack();
            _clear_error_stack();
        }

        result = _get_cache_obj_data(cached);
    }

    return result;
}

/**
 * @brief   Establish a DMTP connection to the DX server of a provided dark domain.
 * @note    This function automatically queries the DIME management record of the domain to determine
 *              the appropriate way to establish a connection to the domain's DX server.
 *              This is the function that should be used by all general callers.
 * @param   domain  a null-terminated string containing the specified dark domain.
 * @param   force_family an optional address family (AF_INET or AF_INET6) to force the TCP connection to take.
 * @return  NULL if unable to establish a DMTP connection successfully, or a pointer to the DMTP session on success.
 */
dmtp_session_t *_sgnt_resolv_dmtp_connect(const char *domain, int force_family) {

    dmtp_session_t *result = NULL;
    dime_record_t *drec;
    mx_record_t **mxs, **mxptr;
    unsigned long ttl;
    char **dxptr;

    if (!domain) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(drec = _get_dime_record(domain, &ttl, 1))) {
        RET_ERROR_PTR(ERR_UNSPEC, "DIME management record could not be retrieved");
    }

    // We only continue if DNSSEC validation succeeded, or DNSSEC was not in place at all.
    // DNSSEC signature failure, on the other hand, is a fatal error.
    if (drec->validated < 0) {
        _destroy_dime_record(drec);
        RET_ERROR_PTR(ERR_UNSPEC, "could not establish DMTP connection to host: DIME management record DNSSEC signature was invalid");
    }

    // There are 3 possible ways this will turn out.
    // 1. The DIME management record has a dx field and we will connect to this server on the standard DMTP port.
    // 2. There is no dx field but the domain has an MX record. We will attempt to connect to this host first
    //    over standard DMTP, and then fall back to dual mode on the SMTP port if unsuccessful.
    // 3. There is no DX field or MX record for the domain. We make an attempt to connect to the standard DMTP port.

    // Case 1: Our record has a DX field.
    if (drec->dx) {
        size_t i = 1;
        dxptr = drec->dx;

        /* We must try each possible DX server in order. */
        while (*dxptr) {
            _dbgprint(1, "Attempting DMTP connection to DIME record-supplied DX server #%u at %s:%u ...\n", i, *dxptr, DMTP_PORT);

            if ((result = _dx_connect_standard(*dxptr, domain, force_family, drec))) {
                break;
            }

            dxptr++, i++;
        }

        // Case 2: There are MX record(s) for our domain.
    } else {

        if ((mxptr = mxs = _get_mx_records(domain))) {

            // Try a maximum of the first 3 MX records.
            for (int i = 0; (i < DMTP_MAX_MX_RETRIES) && *mxptr; i++, mxptr++) {
                _dbgprint(1, "Attempting DMTP connection to MX hostname at %s:%u [pref %u] ...\n", (*mxptr)->name, DMTP_PORT, (*mxptr)->pref);

                if (!(result = _dx_connect_standard((*mxptr)->name, domain, force_family, drec))) {
                    _dbgprint(1, "Re-attempting dual-mode DMTP connection to MX hostname at %s:%u ...\n", (*mxptr)->name, DMTP_PORT_DUAL);
                    result = _dx_connect_dual((*mxptr)->name, domain, force_family, drec, 1);
                }

                if (result) {
                    break;
                }

            }

            free(mxs);
        }

        // Case 3 (final): There is no DX field or MX record for this domain. We try a standard DMTP connection.
        // This is actually executed as failover from Case #2 if it completes unsuccessfully.
        if (!result) {
            _dbgprint(1, "Attempting DMTP connection to assumed DX server at %s:%u ...\n", domain, DMTP_PORT);
            result = _dx_connect_standard(domain, domain, force_family, drec);
        }

    }

    if (!result) {
        RET_ERROR_PTR(ERR_UNSPEC, "connection to DX server failed");
    }

    return result;
}

/**
 * @brief   Destroy a DMTP session and its underlying data.
 * @param   session     a pointer to the DMTP session to be destroyed/disconnected.
 */
void _sgnt_resolv_destroy_dmtp_session(dmtp_session_t *session) {

    if (!session) {
        return;
    }

    if (session->domain) {
        memset(session->domain, 0, strlen(session->domain));
        free(session->domain);
    }

    if (session->dx) {
        memset(session->dx, 0, strlen(session->dx));
        free(session->dx);
    }

    if (session->con) {
        _ssl_disconnect(session->con);
    }

    if (session->_fd >= 0) {
        close(session->_fd);
    }

    if (session->drec) {
        _destroy_dime_record(session->drec);
    }

    memset(session->_inbuf, 0, sizeof(session->_inbuf));

    free(session);

}

/**
 * @brief   Establish (force) a DMTP connection to a specified DX server on tcp port 26/ssl.
 * @note    This function should only be called externally with care.
 * @param   host        the hostname of the DX server to which the DMTP connection will be established.
 * @param   domain      the dark domain which the DX server is servicing.
 * @param   force_family    an optional address family (AF_INET or AF_INET6) to force the TCP connection to take.
 * @param   dimerec     an optional pointer to a DIME management record to be attached to the session.
 * @return  NULL on failure, or a pointer to a newly established DMTP session on success.
 */
dmtp_session_t *_dx_connect_standard(const char *host, const char *domain, int force_family, dime_record_t *dimerec) {

    dmtp_session_t *result;
    SSL *connection;

    if (!host || !domain) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(connection = _ssl_connect_host(host, DMTP_PORT, force_family))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not establish standard DMTP connection to host");
    }

    if (!(result = malloc(sizeof(dmtp_session_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        _ssl_disconnect(connection);
        RET_ERROR_PTR(ERR_NOMEM, "could not establish DMTP session because of memory allocation problem");
    }

    memset(result, 0, sizeof(dmtp_session_t));
    result->con = connection;
    result->mode = dmtp_mode_dmtp;
    result->_fd = -1;
    result->drec = dimerec;

    if ((!(result->domain = strdup(domain))) || (!(result->dx = strdup(host)))) {
        PUSH_ERROR_SYSCALL("strdup");
        _sgnt_resolv_destroy_dmtp_session(result);
        RET_ERROR_PTR(ERR_NOMEM, "could not establish DMTP session because of memory allocation problem");
    }

    if (_sgnt_resolv_dmtp_expect_banner(result) < 0) {
        _sgnt_resolv_destroy_dmtp_session(result);
        RET_ERROR_PTR(ERR_UNSPEC, "received incompatible DMTP banner from server");
    }

    return result;
}

/**
 * @brief   Establish a DMTP connection to a specified DX server that is running DMTP in dual mode (port 25).
 * @note    This function should only be called externally with care.
 * @param   host        the hostname of the DX server to which the DMTP connection will be established.
 * @param   domain      the dark domain which the DX server is servicing.
 * @param   force_family    an optional address family (AF_INET or AF_INET6) to force the TCP connection to take.
 * @param   dimerec     an optional pointer to a DIME management record to be attached to the session.
 * @param   failover    if set, attempt to retry failed connections on the backup port (587).
 * @return  NULL on failure, or a pointer to a newly established DMTP session on success.
 */
dmtp_session_t *_dx_connect_dual(const char *host, const char *domain, int force_family, dime_record_t *dimerec, int failover) {

    dmtp_session_t *result;
    int fd;

    if (!host || !domain) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    // Connect to the remote end and set up a stub DMTP session.
    if ((fd = _connect_host(host, DMTP_PORT_DUAL, force_family)) < 0) {

        if (failover) {
            _dbgprint(1, "Retrying unsuccessful dual mode connection on port 587 ...\n");
            fd = _connect_host(host, 587, force_family);
        }

        if (fd < 0) {
            RET_ERROR_PTR_FMT(ERR_UNSPEC, "unable to connect to dual mode DMTP server  at %s:%u", host, (failover ? 587 : DMTP_PORT_DUAL));
        }

    }

    if (!(result = malloc(sizeof(dmtp_session_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        close(fd);
        RET_ERROR_PTR(ERR_NOMEM, "could not establish DMTP session because of memory allocation problem");
    }

    memset(result, 0, sizeof(dmtp_session_t));
    result->_fd = fd;

    if ((!(result->domain = strdup(domain))) || (!(result->dx = strdup(host)))) {
        PUSH_ERROR_SYSCALL("strdup");
        _sgnt_resolv_destroy_dmtp_session(result);
        RET_ERROR_PTR(ERR_NOMEM, "could not establish DMTP session because of memory allocation problem");
    }

    result->mode = dmtp_mode_dual;

    // Read in the DMTP banner and make sure everything is good.
    if (_sgnt_resolv_dmtp_expect_banner(result) < 0) {
        _sgnt_resolv_destroy_dmtp_session(result);
        RET_ERROR_PTR(ERR_UNSPEC, "received incompatible DMTP banner from server");
    }

    if (_sgnt_resolv_dmtp_initiate_starttls(result, host) != dmtp_mode_dmtp) {
        _sgnt_resolv_destroy_dmtp_session(result);
        RET_ERROR_PTR(ERR_UNSPEC, "failed to initiate TLS session over dual mode server");
    }

    result->active = 1;
    result->drec = dimerec;

    return result;
}

/**
 * @brief   Compare the TLS certificate presented by a remote host against the TLS certificate signature field of a DIME management record.
 * @note    This function accomplishes a number of steps in the following order:
 *              1. If the DIME record has a TLS certificate signature field, perform a comparison of it against the remote peer's certificate.
 *              2. If necessary, perform x509 chain validation against the certificate.
 *              3. If necessary, verify that the certificate has not been revoked by issuing an OCSP request.
 * @param   session     a pointer to the DMTP session to have its TLS certificate verified.
 * @return  -1 on general error, 0 if the connection's TLS certificate failed verification, and 1 if the certificate passed verification.
 *
 */
int _verify_dx_certificate(dmtp_session_t *session) {

    X509 *cert;
    unsigned char **tlsptr;
    ED25519_KEY key;
    unsigned char hashbuf[SHA_512_SIZE], **pokptr;
    int selfsign = 0, tlsmatch = 0, vres;

    if (!session || !(session->drec) || (session->drec->validated < 0) || !(session->con)) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!(cert = SSL_get_peer_certificate(session->con))) {
        PUSH_ERROR_OPENSSL();
        _dbgprint(1, "DX TLS certificate verification failed: could not get peer certificate.\n");
        RET_ERROR_INT(ERR_UNSPEC, "could not get peer certificate");
    }

    if (X509_check_issued(cert, cert) == X509_V_OK) {
        _dbgprint(2, "Continuing verification of self-signed DX TLS certificate ...\n");
        selfsign = 1;
    }

    // TODO:
    // Do we want to verify the self signed certificate using _validate_self_signed even if it matches the TLS signature
    // in order to confirm that it's well formed?

    // The TLS certificate comparison will only take place if the DIME record has a TLS signature field (it's optional).
    if (session->drec->tlssig) {
        memset(hashbuf, 0, sizeof(hashbuf));

        if (_get_x509_cert_sha_hash(cert, 512, hashbuf) < 0) {
            RET_ERROR_INT(ERR_UNSPEC, "unable to compute SHA-512 hash of TLS certificate");
        }

        tlsptr = session->drec->tlssig;
        pokptr = session->drec->pubkey;
        memset(&key, 0, sizeof(key));

        // We must try to verify this server's TLS signature against each of the one(s) we have in our DIME record.
        while (*tlsptr) {

            // Additionally, we must "brute force" each signature by making sure it checks out against each POK, since
            // we're not exactly sure which is the correct one.
            while (*pokptr) {
                // Set up a dummy key for signature validation.
                memcpy(key.public_key, *pokptr, sizeof(key.public_key));

                if ((vres = _ed25519_verify_sig(hashbuf, SHA_512_SIZE, &key, *tlsptr)) == 1) {
                    tlsmatch = 1;
                    break;
                } else if (vres < 0) {
                    RET_ERROR_INT(ERR_UNSPEC, "error verifying TLS signature against POK");
                }

                pokptr++;
            }

            tlsptr++;
        }

        // No TLS certificate match is always the cause of failure.
        if (!tlsmatch) {
            return 0;
        } else {
            _dbgprint(1, "DX TLS certificate matched DIME record signature.\n");
        }

        // If the signature matches, we continue. Unless +dnssec is in place - this means automatic validation.
        if (session->drec->validated == 1) {
            _dbgprint(1, "DX TLS certificate verification succeeded automatically (TLS cert match + dnssec).\n");
            return 1;
        }

    }
    // If the DIME record had no TLS signature field, then there's no way we can accept a self-signed certificate.
    else if (selfsign) {
        _dbgprint(0, "Error verifying DX certificate: self-signed certificates require the DIME management record TLS signature field.\n");
        return 0;
    }

    // Before continue, make sure that this certificate even matches the expected hostname of the DX.
    // Our version of OpenSSL doesn't support X509_check_host() so it looks like this is what we need to do:
    // The hostname is either 1. the dnsName field of the subjectAlternativeName extension, or 2. the CN field of the subject.
    switch (_do_x509_hostname_check(cert, session->dx)) {

    case 0:
        _dbgprint(1, "DX TLS certificate failed x509 hostname check.\n");
        return 0;
        break;
    case 1:
        // do nothing
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "error encountered during x509 hostname check");
        break;

    }

    switch(_do_x509_validation(cert, SSL_get_peer_cert_chain(session->con))) {

    case 0:
        _dbgprint(1, "DX TLS certificate failed x509 chain validation.\n");
        return 0;
        break;
    case 1:
        // do nothing.
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "error encountered during x509 validation");
        break;

    }

    // If the x509 chain verification passed AND the TLS certificate signature matched, we skip the OCSP check.
    if (tlsmatch) {
        _dbgprint(1, "Skipping OCSP check on certificate (x509 check and cert signature check passed).\n");
        return 1;
    }

    // The following remaining cases undergo OCSP validation: x509 chain validation succeeded, but DIME record had no tlssig field.
    switch(_do_ocsp_validation(session->con, NULL)) {

    case 0:
        _dbgprint(1, "DX TLS certificate failed OCSP validation.\n");
        return 0;
        break;
    case 1:
        // do nothing.
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "error encountered during OCSP validation");
        break;

    }

    return 1;
}

/**
 * @brief   Look up a named user or organizational signet.
 * @note    If the fingerprint parameter is omitted, the signet record returned by the
 *              server will be only for the current certificate.
 * @param   session
 * @param   signame     the name of the requested organizational or user signet.
 * @param   fingerprint if not NULL, an optional fingerprint that the returned signet MUST match.
 */
char *_sgnt_resolv_dmtp_get_signet(dmtp_session_t *session, const char *signame, const char *fingerprint) {

    char *response, *request, *rptr, *result;
    size_t reqlen;
    unsigned short rcode;

    if (!session || !signame) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    reqlen = strlen(signame) + 32 + (fingerprint ? strlen(fingerprint) : 0);

    if (!(request = malloc(reqlen))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "could not retrieve signet because of memory allocation error");
    }

    memset(request, 0, reqlen);

    if (fingerprint) {
        snprintf(request, reqlen, "SGNT <%s> [%s]\r\n", signame, fingerprint);
    } else {
        snprintf(request, reqlen, "SGNT <%s>\r\n", signame);
    }

    response = _sgnt_resolv_dmtp_send_and_read(session, request, &rcode);
    free(request);

    // We can get two sorts of failures: a lower-level networking failure, or a response code from the DMTP server that indicates failure.
    if (!response) {
        RET_ERROR_PTR(ERR_UNSPEC, "signet retrieval from remote host failed");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "signet lookup failed: %s", response);
        free(response);
        return NULL;
    }

    rptr = response;

    while (chr_isspace(*rptr)) {
        rptr++;
    }

    if (!*rptr || (*rptr != 'O') || !*(rptr + 1) || (*(rptr + 1) != 'K')) {
        free(response);
        RET_ERROR_PTR(ERR_UNSPEC, "received malformed signet response from server");
    }

    rptr += 2;

    while (chr_isspace(*rptr)) {
        rptr++;
    }

    if ((*rptr++ != '[') || (rptr[strlen(rptr) - 1] != ']')) {
        free(response);
        RET_ERROR_PTR(ERR_UNSPEC, "received malformed signet response from server");
    }

    rptr[strlen(rptr) - 1] = 0;

    if (!(result = strdup(rptr))) {
        PUSH_ERROR_SYSCALL("strdup");
        free(response);
        RET_ERROR_PTR(ERR_NOMEM, NULL);
    }

    free(response);

    return result;
}

/**
 * @brief   Issue an EHLO command to the remote DMTP server.
 * @param   session     a pointer to the DMTP session across which the EHLO command will be issued.
 * @param   domain      a null-terminated string containing the domain name that is the parameter to the EHLO command.
 * @return  0 if the EHLO command was issued and received successfully, or -1 on failure.
 */
int _sgnt_resolv_dmtp_ehlo(dmtp_session_t *session, const char *domain) {

    char *cmd = NULL;
    char *response;
    unsigned short rcode;

    if (!session || !domain) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!_str_printf(&cmd, "EHLO <%s>\r\n", domain)) {
        RET_ERROR_INT(ERR_NOMEM, "unable to construct EHLO request");
    }

    // This command can result in a multi-line response.
    if (_sgnt_resolv_dmtp_issue_command(session, cmd) < 0) {
        free(cmd);
        RET_ERROR_INT(ERR_UNSPEC, "EHLO command failed on remote server");
    }

    free(cmd);

    if (!(response = _sgnt_resolv_read_dmtp_multiline(session, NULL, &rcode))) {
        RET_ERROR_INT(ERR_UNSPEC, "EHLO command failed on remote server");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "EHLO command returned error: %u: %s", rcode, response);
        free(response);
        return -1;
    }

    free(response);

    return 0;
}

/**
 * @brief   Issue a MAIL FROM command to the remote DMTP server.
 */
int _sgnt_resolv_dmtp_mail_from(dmtp_session_t *session, const char *origin, size_t msgsize, dmtp_mail_rettype_t rettype, dmtp_mail_datatype_t dtype) {

    char *cmd = NULL;
    const char *retstr, *datastr;
    char *response;
    unsigned short rcode;

    if (!session || !origin || !msgsize) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    switch(rettype) {

    case return_type_default:
        retstr = "";
        break;
    case return_type_full:
        retstr = " RETURN=FULL";
        break;
    case return_type_display:
        retstr = " RETURN=DISPLAY";
        break;
    case return_type_header:
        retstr = " RETURN=HEADER";
        break;
    default:
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
        break;
    }

    switch(dtype) {

    case data_type_default:
        datastr = "";
        break;
    case data_type_7bit:
        datastr = " DATA=7BIT";
        break;
    case data_type_8bit:
        datastr = " DATA=8BIT";
        break;
    default:
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
        break;
    }

    if (!_str_printf(&cmd, "MAIL FROM: <%s> [%s] SIZE=%lu%s%s\r\n", origin, "fingerprint", msgsize, retstr, datastr)) {
        RET_ERROR_INT(ERR_NOMEM, "unable to construct MAIL FROM request");
    }

    response = _sgnt_resolv_dmtp_send_and_read(session, cmd, &rcode);
    free(cmd);

    if (!response) {
        RET_ERROR_INT(ERR_UNSPEC, "MAIL FROM command failed on remote server");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "MAIL FROM command returned error: %u: %s", rcode, response);
        free(response);
        return -1;
    }

    free(response);

    return 0;
}

/**
 * @brief   Issue an RCPT TO command to the remote DMTP server.
 * @param   session     a pointer to the DMTP session across which the RCPT TO command will be issued.
 * @param   domain      a null-terminated string containing the domain name that is the parameter to the RCPT TO command.
 * @return  0 if the RCPT TO command was issued and received successfully, or -1 on failure.
 */
int _sgnt_resolv_dmtp_rcpt_to(dmtp_session_t *session, const char *domain) {

    char *cmd = NULL;
    char *response;
    unsigned short rcode;

    if (!session || !domain) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!_str_printf(&cmd, "RCPT TO: <%s> [%s]\r\n", domain, "fingerprint")) {
        RET_ERROR_INT(ERR_NOMEM, "unable to construct RCPT TO request");
    }

    response = _sgnt_resolv_dmtp_send_and_read(session, cmd, &rcode);
    free(cmd);

    if (!response) {
        RET_ERROR_INT(ERR_UNSPEC, "RCPT TO command failed on remote server");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "RCPT TO command returned error: %u: %s", rcode, response);
        free(response);
        return -1;
    }

    free(response);

    return 0;
}

/**
 * @brief   Issue an DATA command to the remote DMTP server.
 */
char *_sgnt_resolv_dmtp_data(dmtp_session_t *session, void *msg, size_t msglen) {

    char cmd[128];
    char *response, *token, *tokens, *commit_hash, *result;
    unsigned short rcode;

    if (!session || !msg || !msglen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "DATA [%s]\r\n", "fingerprint");

    // The DATA command is special because the first response is expected to be in the 3xx numeric code range.
    if (!(response = _sgnt_resolv_dmtp_send_and_read(session, cmd, &rcode))) {
        RET_ERROR_PTR(ERR_UNSPEC, "DATA command failed on remote server");
    } else if ((rcode < 300) || (rcode >= 400)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "DATA command returned error: %u: %s", rcode, response);
        free(response);
        return NULL;
    }

    // The first response token should be "CONTINUE".
    if ((!(token = strtok_r(response, " \t", &tokens))) || (strcasecmp(token, "CONTINUE"))) {
        free(response);
        RET_ERROR_PTR(ERR_UNSPEC, "server DATA response was in unexpected format");
    }

    // The only following mandatory piece of data is the hash.
    if ((!(token = strtok_r(NULL, " \t", &tokens))) || (*token != '[') || (token[strlen(token) - 1] != ']')) {
        free(response);
        RET_ERROR_PTR(ERR_UNSPEC, "server DATA response was in unexpected format");
    }

    token[strlen(token) - 1] = 0;

    if (!(commit_hash = strdup(token + 1))) {
        PUSH_ERROR_SYSCALL("strdup");
        free(response);
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for commit hash of DATA command");
    }

    free(response);

    // Write the raw data in our message.
    if ((_sgnt_resolv_dmtp_write_data(session, msg, msglen) < 0) || (_sgnt_resolv_dmtp_write_data(session, "\r\n", 2) < 0)) {
        free(commit_hash);
        RET_ERROR_PTR(ERR_UNSPEC, "unable to write DATA buffer to remote server");
    }

    // Now we get the final server response, which is hopefully an "OK" accompanied by a transaction ID.
    if (!(response = _sgnt_resolv_read_dmtp_line(session, NULL, &rcode, 0))) {
        free(commit_hash);
        RET_ERROR_PTR(ERR_UNSPEC, "DATA command failed on remote server");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "DATA command returned error: %u: %s", rcode, response);
        free(commit_hash);
        free(response);
        return NULL;
    }

    // The first response token should be "CONTINUE".
    if ((!(token = strtok_r(response, " \t", &tokens))) || (strcasecmp(token, "OK"))) {
        free(commit_hash);
        free(response);
        RET_ERROR_PTR(ERR_UNSPEC, "server DATA response continuation was in unexpected format");
    }

    // The only following mandatory piece of data is the hash.
    if ((!(token = strtok_r(NULL, " \t", &tokens))) || (*token != '[') || (token[strlen(token) - 1] != ']')) {
        free(commit_hash);
        free(response);
        RET_ERROR_PTR(ERR_UNSPEC, "server DATA response continuation was in unexpected format");
    }

    token[strlen(token) - 1] = 0;

    if (!(result = strdup(token + 1))) {
        PUSH_ERROR_SYSCALL("strdup");
        free(commit_hash);
        free(response);
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for transaction ID returned by DATA command");
    }

    free(commit_hash);

    return result;
}

/**
 * @brief   Verify whether a named user or org signet is current or not, given both its name and fingerprint.
 * @param   session     a pointer to the DMTP session across which the VRFY command will be issued.
 * @param   signame     the name of the user or organizational signet to be verified.
 * @param   fingerprint the fingerprint of the signet to be checked for updates.
 * @param   newprint    an optional pointer to a string which will be updated to point to the latest
 *                              fingerprint of the specified signet, if it is out of date.
 * @return  -1 on general failure, 0 if the signet fingerprint was out of date, or 1 if it is the most current one.
 */
int _sgnt_resolv_dmtp_verify_signet(dmtp_session_t *session, const char *signame, const char *fingerprint, char **newprint) {

    char *response, *request, *status, *tokens, *nfp;
    size_t reqlen;
    unsigned short rcode;

    if (!session || !signame || !fingerprint) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    reqlen = strlen(signame) + strlen(fingerprint) + 32;

    if (!(request = malloc(reqlen))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_INT(ERR_UNSPEC, "could not verify signet because of memory allocation error");
    }

    memset(request, 0, reqlen);
    snprintf(request, reqlen, "VRFY <%s> [%s]\r\n", signame, fingerprint);

    response = _sgnt_resolv_dmtp_send_and_read(session, request, &rcode);
    free(request);

    // We can get two sorts of failures: a lower-level networking failure, or a response code from the DMTP server that indicates failure.
    if (!response) {
        RET_ERROR_INT(ERR_UNSPEC, "signet verification on remote host failed");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "signet verification failed: %s", response);
        free(response);
        return -1;
    }

    // The first VRFY response parameter must be either "CURRENT" or "UPDATE"
    if ((!(status = strtok_r(response, " \t", &tokens)))) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "VRFY reply was in unexpected format: %s", response);
        free(response);
        return -1;
        // If the specified signet is current then there's really nothing much else to do.
    } else if (!strcasecmp(status, "CURRENT")) {
        free(response);
        return 1;
    } else if (strcasecmp(status, "UPDATE")) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "VRFY reply was in unexpected format: %s", response);
        free(response);
        return -1;
    }

    // If we're here it's because the named signet needs to be updated. This is the next parameter of the reply.
    if (!(nfp = strtok_r(NULL, " \t", &tokens))) {
        free(response);
        RET_ERROR_INT(ERR_UNSPEC, "VRFY reply returned UPDATE without the corresponding fingerprint");
    }

    if (newprint && (!(*newprint = strdup(nfp)))) {
        PUSH_ERROR_SYSCALL("strdup");
        free(response);
        RET_ERROR_INT(ERR_UNSPEC, "signet verification failed due to memory allocation problem");
    }

    // Return code for signet that needs updating.
    return 0;
}

/**
 * @brief   Retrieve the history (key chain) of a named user or organizational signet.
 * @param   session     a pointer to the DMTP session across which the HIST command will be issued.
 * @param   signame     the name of the user or organizational signet to have its history retrieved.
 * @param   startfp     the starting fingerprint of the signet in the queried chain of custody.
 * @param   endfp       the ending fingerprint of the signet in the queried chain of custody.
 * @return  NULL on failure, or a pointer to a string containing the signet's history on success.
 */
char *_sgnt_resolv_dmtp_history(dmtp_session_t *session, const char *signame, const char *startfp, const char *endfp) {

    char *response, *request;
    unsigned short rcode;
    size_t reqlen;

    if (!session || !signame || !startfp) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    reqlen = strlen(signame) + strlen(startfp) + 32;

    if (endfp) {
        reqlen += strlen(endfp);
    }

    if (!(request = malloc(reqlen))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_UNSPEC, "could not query signet history because of memory allocation error");
    }

    memset(request, 0, reqlen);
    snprintf(request, reqlen, "HIST <%s> [%s]", signame, startfp);

    if (endfp) {
        strcat(request, " [");
        strcat(request, endfp);
        strcat(request, "]");
    }

    strcat(request, "\r\n");

    if (_sgnt_resolv_dmtp_issue_command(session, request) < 0) {
        free(request);
        RET_ERROR_PTR(ERR_UNSPEC, "HIST command failed on remote server");
    }

    free(request);

    if (!(response = _sgnt_resolv_read_dmtp_multiline(session, NULL, &rcode))) {
        RET_ERROR_PTR(ERR_UNSPEC, "HIST command failed on remote server");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "HIST command returned error: %u: %s", rcode, response);
        free(response);
        return NULL;
    }

    return response;
}

char *_sgnt_resolv_dmtp_stats(dmtp_session_t *session, const unsigned char *secret __attribute__((__unused__))) {

    const char *stats_cmd = "STATS\r\n";
    char *response, *rptr, *endptr, *nonce_req = NULL;
    unsigned short rcode;

    if (!session) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (_sgnt_resolv_dmtp_issue_command(session, stats_cmd) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "STATS command failed on remote server");
    }

    if (!(response = _sgnt_resolv_read_dmtp_multiline(session, NULL, &rcode))) {
        RET_ERROR_PTR(ERR_UNSPEC, "STATS command failed on remote server");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "STATS command returned error: %u: %s", rcode, response);
        free(response);
        return NULL;
    }

    rptr = response;

    while (*rptr && chr_isspace(*rptr)) {
        rptr++;
    }

    // Check to see if a nonce was returned. If it was not, we can return right away.
    if (strncasecmp(rptr, "ONCE", 4)) {
        return response;
    }

    rptr += 4;

    if (!chr_isspace(*rptr)) {
        return response;
    }

    // If we've gotten this far, we can assume that the server intended to return a nonce.
    while (chr_isspace(*rptr)) {
        rptr++;
    }

    if (*rptr++ != '[') {
        free(response);
        RET_ERROR_PTR(ERR_UNSPEC, "STAT command returned unrecognized reply");
    }

    endptr = rptr + strlen(rptr);
    endptr--;

    if ((endptr <= rptr) || (*endptr != ']')) {
        free(response);
        RET_ERROR_PTR(ERR_UNSPEC, "STAT command returned unrecognized reply");
    }

    *endptr = 0;

    // Retrieve our nonce and reissue the command.
    memmove(response, rptr, (size_t)(endptr - rptr + 1));

    if (!_str_printf(&nonce_req, "STATS [%s]\r\n", response)) {
        free(response);
        RET_ERROR_PTR(ERR_UNSPEC, "unable to construct STATS request with nonce");
    }

    free(response);

    if (_sgnt_resolv_dmtp_issue_command(session, nonce_req) < 0) {
        free(nonce_req);
        RET_ERROR_PTR(ERR_UNSPEC, "STATS command with nonce failed on remote server");
    }

    free(nonce_req);

    if (!(response = _sgnt_resolv_read_dmtp_multiline(session, NULL, &rcode))) {
        RET_ERROR_PTR(ERR_UNSPEC, "STATS command with nonce failed on remote server");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "STATS command with nonce returned error: %u: %s", rcode, response);
        free(response);
        return NULL;
    }

    return response;
}

/**
 * @brief   Return the DMTP mode corresponding to a DMTP mode string.
 * @param   modestr     a null-terminated string containing the human-readable name of the mode.
 * @return  the DMTP mode type corresponding to the supplied mode string, or dmtp_mode_unknown otherwise.
 */
dmtp_mode_t _sgnt_resolv_dmtp_str_to_mode(const char *modestr) {

    if (!modestr) {
        return dmtp_mode_dmtp;
    }

    if (!strcasecmp(modestr, "DMTPv1")) {
        return dmtp_mode_dmtp;
    } else if (!strcasecmp(modestr, "SMTP")) {
        return dmtp_mode_smtp;
    } else if (!strcasecmp(modestr, "ESMTP")) {
        return dmtp_mode_esmtp;
    }

    return dmtp_mode_unknown;
}

/**
 * @brief   Get the current DMTP server mode.
 * @param   session     a pointer to the DMTP session across which the MODE command will be issued.
 * @return  the value of the current DMTP mode on success, or dmtp_mode_unknown on failure.
 */
dmtp_mode_t _sgnt_resolv_dmtp_get_mode(dmtp_session_t *session) {

    dmtp_mode_t result = dmtp_mode_unknown;
    const char *mode_cmd = "MODE\r\n";
    unsigned short rcode;
    char *response, *token, *tokens;

    if (!session) {
        RET_ERROR_CUST(dmtp_mode_unknown, ERR_BAD_PARAM, NULL);
    }

    if (!(response = _sgnt_resolv_dmtp_send_and_read(session, mode_cmd, &rcode))) {
        RET_ERROR_CUST(dmtp_mode_unknown, ERR_UNSPEC, "MODE command failed on remote server");
        return dmtp_mode_unknown;
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "MODE command returned error: %u: %s", rcode, response);
        free(response);
        return dmtp_mode_unknown;
    }

    // The first MODE response parameter must be "OK"
    if ((!(token = strtok_r(response, " \t", &tokens))) || strcmp(token, "OK")) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "MODE reply was in unexpected format: %s", response);
        free(response);
        return dmtp_mode_unknown;
    }

    // There should be one and ONLY one more response parameter.
    token = strtok_r(NULL, " \t", &tokens);

    if (!token || strtok_r(NULL, " \t", &tokens)) {
        free(response);
        RET_ERROR_CUST(dmtp_mode_unknown, ERR_UNSPEC, "MODE reply was in unexpected format");
    }

    result = _sgnt_resolv_dmtp_str_to_mode(token);
    free(response);

    return result;
}

/**
 * @brief   Issue a no-operation command to the remote DMTP server.
 * @param   session     a pointer to the DMTP session across which the NOOP command will be issued.
 * @return  -1 on failure or 0 if the command was successfully executed.
 */
int _sgnt_resolv_dmtp_noop(dmtp_session_t *session) {

    const char *noop_cmd = "NOOP\r\n";
    char *response;
    unsigned short rcode;

    if (!session) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!(response = _sgnt_resolv_dmtp_send_and_read(session, noop_cmd, &rcode))) {
        RET_ERROR_INT(ERR_UNSPEC, "NOOP command failed on remote server");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "NOOP command returned error: %u: %s", rcode, response);
        free(response);
        return -1;
    }

    free(response);

    return 0;
}

/**
 * @brief   Issue a reset command to the remote DMTP server.
 * @param   session     a pointer to the DMTP session across which the RSET command will be issued.
 * @return  -1 on failure or 0 if the command was successfully executed.
 */
int _sgnt_resolv_dmtp_reset(dmtp_session_t *session) {

    const char *noop_cmd = "RSET\r\n";
    char *response;
    unsigned short rcode;

    if (!session) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!(response = _sgnt_resolv_dmtp_send_and_read(session, noop_cmd, &rcode))) {
        RET_ERROR_INT(ERR_UNSPEC, "RSET command failed on remote server");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "RSET command returned error: %u: %s", rcode, response);
        free(response);
        return -1;
    }

    free(response);

    return 0;
}

/**
 * @brief   Get a list of supported DMTP server commands by issuing the HELP command.
 * @param   session     a pointer to the DMTP session across which the HELP command will be issued.
 * @return  NULL on failure or a pointer to a string containing the full server help table on success.
 */
char *_sgnt_resolv_dmtp_help(dmtp_session_t *session) {

    const char *help_cmd = "HELP\r\n";
    char *response;
    unsigned short rcode;

    if (!session) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (_sgnt_resolv_dmtp_issue_command(session, help_cmd) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "HELP command failed on remote server");
    }

    if (!(response = _sgnt_resolv_read_dmtp_multiline(session, NULL, &rcode))) {
        RET_ERROR_PTR(ERR_UNSPEC, "HELP command failed on remote server");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "HELP command returned error: %u: %s", rcode, response);
        free(response);
        return NULL;
    }

    return response;
}

/**
 * @brief   Terminate an active DMTP session by issuing the QUIT command.
 * @param   session     a pointer to the DMTP session across which the QUIT command will be issued.
 * @param   do_close    currently unused
 * @return  -1 on failure or 0 if the command was successfully executed.
 */
int _sgnt_resolv_dmtp_quit(dmtp_session_t *session, int do_close) {

    const char *quit_cmd = "QUIT\r\n";
    char *response;
    unsigned short rcode;
    int result = -1;

    (void)do_close; /* TODO: actually use this parameter */

    if (!session) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!(response = _sgnt_resolv_dmtp_send_and_read(session, quit_cmd, &rcode))) {
        PUSH_ERROR(ERR_UNSPEC, "QUIT command failed on remote server");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "RSET command returned error: %u: %s\n", rcode, response);
    } else {
        result = 0;
    }

    if (response) {
        free(response);
    }

    if (session->con) {
        _ssl_disconnect(session->con);
        session->con = NULL;
    } else if (session->_fd >= 0) {

        if (close(session->_fd) < 0) {
            perror("close");
        }

        session->_fd = -1;
    }

    session->active = 0;

    return result;
}

/**
 * @brief   Read a CR/LF-terminated line of input from an active DMTP session.
 * @param   session     a pointer to the DMTP session from which the response line will be read.
 * @param   overflow    an optional pointer to a variable that will be set if the read operation
 *                              exceeds the size of the internal line buffer.
 * @param   rcode       an optional pointer to a variable that will receive the numeric response
 *                              code of the DMTP reply that was just received.
 * @param   multiline   an optional parameter that if set will permit multiline DMTP responses. If
 *                              this was the final line of a multiline response, the value will be set to 1
 *                              when the function returns, or 0 if there is more content to follow.
 * @return  NULL on failure or a pointer to a string containing the next line(s) of output from the DMTP server.
 */
char *_sgnt_resolv_read_dmtp_line(dmtp_session_t *session, int *overflow, unsigned short *rcode, int *multiline) {

    char *result = NULL, *lbreak = NULL, *line;
    size_t nleft;
    int nread;

    if (!session) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    // TODO: This needs to be removed or corrected.
/*  if (!session->active) {
                RET_ERROR_PTR(ERR_UNSPEC, "cannot read network line from inactive DMTP session");
        } */

    // Try to read some more into the buffer until we get enough or an error occurs.
    nleft = sizeof(session->_inbuf) - session->_inpos - 1;

    // As long as we can read more data and we still don't have a CRLF...
    while (nleft && (!(lbreak = strstr((char *)session->_inbuf, "\r\n")))) {

        // Determine whether we're reading from a vanilla socket or an SSL connection.
        if (session->con) {
            nread = SSL_read(session->con, &(session->_inbuf[session->_inpos]), nleft);
        } else {
            nread = recv(session->_fd, &(session->_inbuf[session->_inpos]), nleft, 0);
        }

        if (nread <= 0) {

            if (session->mode == dmtp_mode_dmtp) {

                if (nread < 0) {
                    PUSH_ERROR_OPENSSL();
                }

                _ssl_disconnect(session->con);
                session->con = NULL;
            } else {

                if (nread < 0) {
                    perror("recv");

                }

                close(session->_fd);
                session->_fd = -1;
            }

            session->active = 0;

            // If there's something in the buffer, return it.
            if (session->_inpos) {

                if (!(result = strdup((char *)session->_inbuf))) {
                    perror("strdup");
                }

            } else {
                result = NULL;
            }

            memset(session->_inbuf, 0, sizeof(session->_inbuf));
            session->_inpos = 0;

            if (overflow) {
                *overflow = 0;
            }

            break;
        }

        // We did get some data. But is it enough for a full line?
        //
        // This should never happen but you can never be too safe.
        if ((size_t)nread > nleft) {
            RET_ERROR_PTR(ERR_UNSPEC, "unexpected error occurred in SSL line read operation");
        }

        session->_inpos += nread;
        nleft -= nread;
    }

    // There's a good chance that we're here before we got our CRLF
    // If we got what we were looking for:
    if (lbreak) {
        *lbreak++ = 0;
        *lbreak++ = 0;
        result = strdup((char *)session->_inbuf);

        // Move the remainder of the buffer to the front of the buffer.
        nleft = (unsigned long)&(session->_inbuf[sizeof(session->_inbuf)]) - (unsigned long)lbreak;
        memmove(session->_inbuf, lbreak, nleft);
        // Zero out the trailing buffer.
        memset(&(session->_inbuf[nleft]), 0, sizeof(session->_inbuf) - nleft);
        // Reset the buffer pointer.
        session->_inpos -= (unsigned long)lbreak - (unsigned long)session->_inbuf;

        if (overflow) {
            *overflow = 0;
        }

        // Otherwise, we can only return what we have.
    } else if (session->_inpos) {
        // Simple strdup() because our buffer has an extra character that is always = \x00.
        result = strdup((char *)session->_inbuf);
        memset(session->_inbuf, 0, sizeof(session->_inbuf));
        session->_inpos = 0;

        if (overflow) {
            *overflow = 1;
        }

    }

    if (!result && (lbreak || session->_inpos)) {
        PUSH_ERROR_SYSCALL("strdup");
        // If the caller requested line code parsing, perform this operation and free the original result.
    } else if (rcode) {

        if (!(line = _sgnt_resolv_parse_line_code(result, rcode, multiline))) {
            free(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not parse DMTP line response code");
        }

        if (!(line = strdup(line))) {
            PUSH_ERROR_SYSCALL("strdup");
            free(result);
            RET_ERROR_PTR(ERR_NOMEM, "unable to allocate temporary buffer for response");
        }

        free(result);
        result = line;
    }

    _dbgprint(5, "DMTP < %s\n", result);

    return result;
}

/**
 * @brief   Read a multiple-line response from the DMTP server.
 * @note    A multiline response is designated by the presence of a hyphen between the response code and the
 *              respones text. A regular response, or the final line of a multiline response will instead have a space.
 * @param   session     a pointer to the DMTP session from which the response line(s) will be read.
 * @param   overflow    an optional pointer to a variable that will be set if the read operation
 *                              exceeds the size of the internal line buffer.
 * @param   rcode       an optional pointer to a variable that will receive the numeric response
 *                              code of the DMTP reply that was just received.
 * @return  NULL on failure or a pointer to a string containing the next response from the DMTP server on success.
 */
char *_sgnt_resolv_read_dmtp_multiline(dmtp_session_t *session, int *overflow, unsigned short *rcode) {

    char *line, *result = NULL, *reall_res = NULL;
    unsigned short rc, firstrc = 0;
    int of, ml, first = 1;
    size_t rsize = 1;

    if (!session) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    while ((line = _sgnt_resolv_read_dmtp_line(session, &of, &rc, &ml))) {

        // This shouldn't happen, but we need to make sure that all the response codes were of the same sequence.
        if (!first && (firstrc != rc)) {
            free(line);
            free(result);
            RET_ERROR_PTR(ERR_BAD_PARAM, "multiline DMTP response returned unexpected response code");
        }

        // The result needs to have the new line followed by \n appended to it (if it is not the last line).
        rsize += strlen(line) + 1;

        if (!(reall_res = realloc(result, rsize))) {
            PUSH_ERROR_SYSCALL("realloc");

            if(result) {
                free(result);
            }

            free(line);
            RET_ERROR_PTR(ERR_NOMEM, "could not read multiline DMTP response because of memory allocation problem");
        }

        result = reall_res;
        reall_res = NULL;

        // We need to make sure our result buffer is null-terminated at least the first time it's allocated.
        if (first) {
            memset(result, 0, rsize);
            first = 0;
            firstrc = rc;
        }

        strcat(result, line);

        // No new-line terminator if it's the last line.
        if (!ml) {
            strcat(result, "\n");
        }

        free(line);

        // If the multiline variable was set to non-zero, then this is the last line of the series.
        if (ml) {
            break;
        }

    }

    // These value should be set for the caller.
    if (rcode) {
        *rcode = rc;
    }

    if (overflow) {
        *overflow = of;
    }

    return result;
}

/**
 * @brief   Parse a DMTP line into a numerical code and the trailing text.
 * @param   line        a pointer to a null-terminated string to be parsed as a DMTP response line or lines.
 * @param   rcode       an opointer to a variable that will receive the numeric response code of the DMTP reply.
 * @param   multiline   an optional parameter that if set will permit multiline DMTP responses. If
 *                              this was the final line of a multiline response, the value will be set to 1
 *                              when the function returns, or 0 if there is more content to follow.
 * @return  NULL on failure or a pointer to a string containing the next line(s) of output from the DMTP server.
 */
char * _sgnt_resolv_parse_line_code(const char *line, unsigned short *rcode, int *multiline) {

    char numbuf[8];
    const char *ptr = line;
    size_t nbytes;

    if (!line) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    while (*ptr) {

        // The word before the first space is (should be) the numerical code.
        // Or in multiline mode, the response code is separated from the rest of the line by a hyphen.
        if (chr_isspace(*ptr) || (multiline && (*ptr == '-'))) {
            memset(numbuf, 0, sizeof(numbuf));
            nbytes = (unsigned long)ptr - (unsigned long)line;
            nbytes = nbytes < sizeof(numbuf) ? nbytes : sizeof(numbuf) - 1;
            strncpy(numbuf, line, nbytes);

            if (!(*rcode = atoi(numbuf))) {
                RET_ERROR_PTR_FMT(ERR_UNSPEC, "DMTP response line contained invalid numerical response code {code = %s}", numbuf);
            }

            // In multiline, what comes after the hyphen is the rest of the line buffer.
            if (multiline && (*ptr == '-')) {
                ptr++;

                // This isn't the final line of the multi-line response.
                *multiline = 0;
            } else {

                // Otherwise, what comes after the last space is the rest of the line.
                while (*ptr && chr_isspace(*ptr)) {
                    ptr++;
                }

                // This is the final line of a multi-line response.
                if (multiline) {
                    *multiline = 1;
                }

            }

            return (char *)ptr;
        }

        ptr++;
    }

    RET_ERROR_PTR(ERR_UNSPEC, NULL);
}

/**
 * @brief   Initiate a DMTP session over a plain TCP connection with the DMTP STARTTLS command.
 * @note    The dxname argument will be used as a parameter to the STARTTLS command for requesting a TLS
 *              certificate explcitly by hostname.
 * @param   session     a pointer to the DMTP session to have its transport security upgraded with TLS.
 * @param   dxname      the name of the DX server providing the TLS service of this DMTP session.
 * @return  dmtp_mode_unknown on failure or the active mode of the DMTP session as reported by the server on success.
 */
dmtp_mode_t _sgnt_resolv_dmtp_initiate_starttls(dmtp_session_t *session, const char *dxname) {

    dmtp_mode_t result;
    char cmdbuf[512], *response, *rptr;
    unsigned short rcode;

    if (!session | !dxname) {
        RET_ERROR_CUST(dmtp_mode_unknown, ERR_BAD_PARAM, NULL);
    }

    if (session->con) {
        RET_ERROR_CUST(dmtp_mode_unknown, ERR_UNSPEC, "cannot initiate STARTTLS if a TLS session already exists");
    } else if (session->_fd < 0) {
        RET_ERROR_CUST(dmtp_mode_unknown, ERR_UNSPEC, "cannot initiate STARTTLS without a valid network file descriptor");
    }

    memset(cmdbuf, 0, sizeof(cmdbuf));
    snprintf(cmdbuf, sizeof(cmdbuf), "STARTTLS <%s> MODE=DMTPv1\r\n", dxname);

    if ((size_t)send(session->_fd, cmdbuf, strlen(cmdbuf), 0) != strlen(cmdbuf)) {
        RET_ERROR_CUST(dmtp_mode_unknown, ERR_UNSPEC, "unable to issue STARTTLS command to DMTP server");
    }

    // This is one of the odd commands that should be prepared to read a multi-line response.
    if (!(response = _sgnt_resolv_read_dmtp_multiline(session, NULL, &rcode))) {
        RET_ERROR_CUST(dmtp_mode_unknown, ERR_UNSPEC, "STARTTLS negotiation received unexpected server response");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "STARTTLS command returned error: %u: %s\n", rcode, response);
        free(response);
        return dmtp_mode_unknown;
    }

    free(response);

    if (!(session->con = _ssl_starttls(session->_fd))) {
        RET_ERROR_CUST(dmtp_mode_unknown, ERR_UNSPEC, "TLS negotiation in DMTP session failed");
    }

    // Finally, hide the file descriptor. Network operations only occur via SSL now.
    session->_fd = -1;
    session->mode = dmtp_mode_dmtp;

    // Now we need to process the final response to the STARTTLS command and see if the server recognizes it as successful.
    if (!(response = _sgnt_resolv_read_dmtp_line(session, NULL, &rcode, NULL))) {
        RET_ERROR_CUST(dmtp_mode_unknown, ERR_UNSPEC, "STARTTLS negotiation failed on server for unknown reason");
    } else if ((rcode < 200) || (rcode >= 300)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "STARTTLS command failed on server with code %u: %s", rcode, response);
        free(response);
        return dmtp_mode_unknown;
    }

    // The response is "OK" followed by the mode.
    rptr = response + 2;

    if (strncmp(response, "OK", 2)) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "STARTTLS server response was of unrecognized format: %s", response);
        free(response);
        return dmtp_mode_unknown;
    }

    while (chr_isspace(*rptr)) {
        rptr++;
    }

    if (!*rptr) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "STARTTLS server response was of unrecognized format: %s", response);
        free(response);
        return dmtp_mode_unknown;
    }

    result = _sgnt_resolv_dmtp_str_to_mode(rptr);
    free(response);

    return result;
}


/**
 * @brief   Attempt to receive a valid DMTP banner immediately after a connection is established.
 * @param   session     a pointer to the DMTP session to have the server banner read.
 * @return  -1 on failure or 0 if a banner was succesfully received advertising DMTPv1 compatibility.
 */
int _sgnt_resolv_dmtp_expect_banner(dmtp_session_t *session) {

    char *banner, *token, *tokens;
    unsigned short bcode;
    int result = -1;

    if (!session) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!(banner = _sgnt_resolv_read_dmtp_line(session, NULL, &bcode, 0))) {
        RET_ERROR_INT(ERR_UNSPEC, "unable to read DMTP banner");
    }

    _dbgprint(4, "DMTP banner: %s\n", banner);

    if (bcode != 220) {
        free(banner);
        RET_ERROR_INT(ERR_UNSPEC, "DMTP banner had bad status code");
    }

    // Skip the first token, which sould be the server hostname.
    if (!(token = strtok_r(banner, " \t", &tokens))) {
        free(banner);
        RET_ERROR_INT(ERR_UNSPEC, "DMTP banner contained unexpected format");
    }

    // Check the remaining tokens to see if one of them is DMTP.
    while ((token = strtok_r(NULL, " \t", &tokens))) {

        if (!strcmp(token, "DMTPv1")) {
            result = 0;
            break;
        }

    }

    free(banner);

    return result;
}

/**
 * @brief   Issue a command to the remote server in a DMTP session.
 * @param   session     a pointer to the DMTP session across which the command will be issued.
 * @param   cmd     a null-terminated string containing the value of the specified DMTP command.
 * @return  -1 on failure or 0 if the command was successfully sent.
 */
int _sgnt_resolv_dmtp_issue_command(dmtp_session_t *session, const char *cmd) {

    int nwritten;

    if (!session) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    _dbgprint(5, "DMTP > %s", cmd);

    // Figure out if this is being written plain text or through an established SSL connection.
    if (session->con) {

        if ((nwritten = SSL_write(session->con, cmd, strlen(cmd))) <= 0) {
            PUSH_ERROR_OPENSSL();
        }

    } else if (session->_fd >= 0) {

        if ((nwritten = send(session->_fd, cmd, strlen(cmd), 0)) < 0) {
            PUSH_ERROR_SYSCALL("send");
        }

    } else {
        RET_ERROR_INT(ERR_UNSPEC, "unable to issue command; session was in a bad state");
    }

    if ((size_t)nwritten != strlen(cmd)) {
        RET_ERROR_INT(ERR_UNSPEC, "unable to issue DMTP command");
    }

    return 0;
}

/**
 * @brief   Issue a command to a DMTP server and get the response.
 * @param   session     a pointer to the DMTP session across which the command will be issued.
 * @param   cmd     a null-terminated string containing the value of the specified DMTP command.
 * @param   rcode
 * @return  NULL on failure or a pointer to a string containing the next line(s) of output from the DMTP server on success.
 */
char *_sgnt_resolv_dmtp_send_and_read(dmtp_session_t *session, const char *cmd, unsigned short *rcode) {

    char *result;

    if (!session || !cmd) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (_sgnt_resolv_dmtp_issue_command(session, cmd) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "unable to issue DMTP command");
    }

    if (!(result = _sgnt_resolv_read_dmtp_line(session, NULL, rcode, 0))) {
        RET_ERROR_PTR(ERR_UNSPEC, NULL);
    }

    return result;
}


/**
 * @brief   Write raw data to the remote end of a DMTP session.
 * @param   session     a pointer to the DMTP session to which the data will be written.
 * @param   buf     a pointer to the buffer containing the raw data to be written.
 * @param   buflen      the size, in bytes, of the data buffer to be written to the DMTP session.
 * @return  0 if all requested bytes were written successfully to the connection, or -1 on failure.
 */
int _sgnt_resolv_dmtp_write_data(dmtp_session_t *session, const void *buf, size_t buflen) {

    unsigned char *dataptr = (unsigned char *)buf;
    int nwritten;

    if (!session || !buf || !buflen) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    _dbgprint(5, "Attempting to write %zu bytes of data to server...\n", buflen);

    while (buflen) {

        // Figure out if this is being written plain text or through an established SSL connection.
        if (session->con) {

            if ((nwritten = SSL_write(session->con, dataptr, buflen)) <= 0) {
                PUSH_ERROR_OPENSSL();
                RET_ERROR_INT(ERR_UNSPEC, "could not write all data in buffer to remove server");
            }

        } else if (session->_fd >= 0) {

            if ((nwritten = send(session->_fd, dataptr, buflen, 0)) <= 0) {

                if (nwritten < 0) {
                    PUSH_ERROR_SYSCALL("send");
                }

                RET_ERROR_INT(ERR_UNSPEC, "could not write all data in buffer to remove server");
            }

        } else {
            RET_ERROR_INT(ERR_UNSPEC, "could not write data; session was ain a bad state");
        }

        buflen -= nwritten;
    }

    _dbgprint(5, "Finished writing data to server.\n");

    return 0;
}
