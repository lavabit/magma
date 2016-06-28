#include "dime/signet-resolver/mrec.h"

#include "dime/signet-resolver/cache.h"
#include "dime/signet-resolver/dns.h"
#include "dime/common/misc.h"
#include "dime/common/error.h"


/**
 * @brief   Destroy a DIME management record and its underlying data.
 * @param   record  a pointer to the DIME management record object to be destroyed.
 */
void _destroy_dime_record(dime_record_t *record) {

    if (!record) {
        _dbgprint(0, "Attempted to destroy NULL DIME record.\n");
        return;
    }

    if (record->syndicates) {
        free(record->syndicates);
    }

    _ptr_chain_free(record->pubkey);
    _ptr_chain_free(record->dx);
    _ptr_chain_free(record->tlssig);

    memset(record, 0, sizeof(dime_record_t));
    free(record);

}


/**
 * @brief   A callback handler to dump a DIME management record to the console.
 * @note    This is an internal function used by the cache management subsystem.
 * @param   fp  a pointer to the file stream that will receive the dump output.
 * @param   record  a pointer to the DIME management record structure to be dumped.
 * @param   brief   if set, only print a brief one-line description for the requested record.
 */
void _dump_dime_record_cb(FILE *fp, void *record, int brief) {

    dime_record_t *drec = (dime_record_t *)record;
    unsigned char **tlsptr;
    unsigned char **pokptr;
    const char *policy_string, *sub_string;
    char **dxptr, *tmpstr;
    size_t i;

    if (!fp) {
        return;
    }

    if (!drec) {
        fprintf(stderr, "Error: could not dump null DIME management record.\n");
        return;
    }

    if (brief) {
        fprintf(fp, "*** hashed ***");
        return;
    }

    switch(drec->policy) {
    case msg_experimental:
        policy_string = "experimental";
        break;
    case msg_mixed:
        policy_string = "mixed";
        break;
    case msg_strict:
        policy_string = "strict";
        break;
    default:
        policy_string = "[unknown]";
        break;
    }

    switch(drec->subdomain) {
    case sub_strict:
        sub_string = "strict";
        break;
    case sub_relaxed:
        sub_string = "relaxed";
        break;
    case sub_explicit:
        sub_string = "explicit";
        break;
    default:
        sub_string = "[unknown]";
        break;
    }


    fprintf(fp, "--- DIME management record for: --- hashed ---\n");
    fprintf(fp, "------ version   : %u\n", drec->version);

    if (!drec->pubkey) {
        fprintf(fp, "------ pok       : [not present (ERROR)]\n");
    } else {
        i = 1;
        pokptr = drec->pubkey;

        while (*pokptr) {

            if (!(tmpstr = _hex_encode((*pokptr), ED25519_KEY_SIZE))) {
                fprintf(stderr, "Error: could not hex encode org public key.\n");
                dump_error_stack();
                _clear_error_stack();
            } else {
                fprintf(fp, "------ pok       : %s [%zu]\n", tmpstr, i);
                free(tmpstr);
            }

            pokptr++, i++;
        }

    }

    if (!drec->tlssig) {
        fprintf(fp, "------ tlssig    : [not present]\n");
    } else {
        i = 1;
        tlsptr = drec->tlssig;

        while (*tlsptr) {

            if (!(tmpstr = _b64encode_nopad(*tlsptr, ED25519_SIG_SIZE))) {
                fprintf(stderr, "Error: could not base64 encode TLS signature.\n");
                dump_error_stack();
                _clear_error_stack();
            } else {
                fprintf(fp, "------ tlssig    : %s [%zu]\n", tmpstr, i);
                free(tmpstr);
            }

            tlsptr++, i++;
        }

    }

    fprintf(fp, "------ policy    : %s\n", policy_string);
    fprintf(fp, "------ syndicates: %s\n", drec->syndicates ? drec->syndicates : "[not present]");

    if (!drec->dx) {
        fprintf(fp, "------ dx        : [not present]\n");
    } else {
        i = 1;
        dxptr = drec->dx;

        while (*dxptr) {
            fprintf(fp, "------ dx        : %s [%zu]\n", *dxptr, i);
            dxptr++, i++;
        }

    }
    fprintf(fp, "------ expiry    : ");

    if (drec->expiry) {
        fprintf(fp, "%u days\n", (unsigned int)drec->expiry);
    } else {
        fprintf(fp, "[not present]\n");
    }

    fprintf(fp, "------ subdomain : %s\n", sub_string);

    if (drec->validated > 0) {
        fprintf(fp, "++++++ This record WAS retrieved with a valid DNSSEC signature.\n");
    } else if (!drec->validated) {
        fprintf(fp, "////// This record was NOT retrieved in a DNSSEC-protected response.\n");
    } else {
        fprintf(fp, "****** This record was retrieved with an INVALID DNSSEC signature.\n");
    }

}


/**
 * @brief   A callback handler to destroy a DIME management record.
 * @note    This is an internal function used by the cache management subsystem.
 * @param   record  a pointer to the DIME management record to be destroyed.
 */
void _destroy_dime_record_cb(void *record) {

    _destroy_dime_record((dime_record_t *)record);
}


/**
 * @brief   A callback handler to deserialize a DIME record from the persistent cache for use in memory.
 * @note    This is an internal function used by the cache management subsystem.
 * @param   data    a pointer to a serialized DIME record in binary form that will be deserialized.
 * @param   len the length, in bytes, of the buffer to be deserialized.
 * @return  a pointer to a newly allocated DIME management record structure constructed from the serialized data, or NULL on failure.
 */
void *_deserialize_dime_record_cb(void *data, size_t len) {

    dime_record_t *result;
    unsigned char *dptr = (unsigned char *)data, *dend = (unsigned char *)data + len;

    if (!data || !len) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(result = malloc(sizeof(dime_record_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for deserialized DIME record");
    }

    memset(result, 0, sizeof(dime_record_t));

    if (!_deserialize_data((unsigned char *)&(result->version), &dptr, dend, sizeof(result->version)) ||
        !_deserialize_array(&(result->pubkey), &dptr, dend, ED25519_KEY_SIZE) ||
        !_deserialize_array(&(result->tlssig), &dptr, dend, ED25519_SIG_SIZE) ||
        !_deserialize_data((unsigned char *)&(result->policy), &dptr, dend, sizeof(result->policy)) ||
        !_deserialize_string(&(result->syndicates), &dptr, dend) ||
        !_deserialize_str_array(&(result->dx), &dptr, dend) ||
        !_deserialize_data((unsigned char *)&(result->expiry), &dptr, dend, sizeof(result->expiry)) ||
        !_deserialize_data((unsigned char *)&(result->subdomain), &dptr, dend, sizeof(result->subdomain)) ||
        !_deserialize_data((unsigned char *)&(result->validated), &dptr, dend, sizeof(result->validated))) {
        free(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize DIME management record from persistent cache");
    }

    if (_validate_dime_record(result) < 0) {
        _destroy_dime_record(result);
        RET_ERROR_PTR(ERR_UNSPEC, "deserialized DIME management record failed to pass validation");
    }

    return result;
}


/**
 * @brief   A callback handler to serialize a DIME record for persistence through the data cache.
 * @note    This is an internal function used by the cache management subsystem.
 * @param   record  a pointer to a DIME management record object to be serialized for persistence.
 * @param   outlen  a pointer to a variable to receive the length of the serialized data on completion.
 * @return  a pointer to a newly allocated buffer containing the serialized DIME record data, or NULL on failure.
 */
void *_serialize_dime_record_cb(void *record, size_t *outlen) {

    dime_record_t *drec = (dime_record_t *)record;
    unsigned char *buf = NULL;

    if (!drec || !outlen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    *outlen = 0;

    if (!_mem_append(&buf, outlen, (unsigned char *)&(drec->version), sizeof(drec->version)) ||
        !_mem_append_serialized_array(&buf, outlen, (const unsigned char **)drec->pubkey, ED25519_KEY_SIZE) ||
        !_mem_append_serialized_array(&buf, outlen, (const unsigned char **)drec->tlssig, ED25519_SIG_SIZE) ||
        !_mem_append(&buf, outlen, (unsigned char *)&(drec->policy), sizeof(drec->policy)) ||
        !_mem_append_serialized_string(&buf, outlen, drec->syndicates) ||
        !_mem_append_serialized_str_array(&buf, outlen, (const char **)drec->dx) ||
        !_mem_append(&buf, outlen, (unsigned char *)&(drec->expiry), sizeof(drec->expiry)) ||
        !_mem_append(&buf, outlen, (unsigned char *)&(drec->subdomain), sizeof(drec->subdomain)) ||
        !_mem_append(&buf, outlen, (unsigned char *)&(drec->validated), sizeof(drec->validated))) {

        if (buf) {
            free(buf);
        }

        RET_ERROR_PTR(ERR_NOMEM, "unable to serialize DIME record");
    }

    return buf;
}


/**
 * @brief   Parse a data buffer into a DIME management record.
 * @note    The data passed to this function is typically retrieved from DNS via the _dx TXT record.
 * @param   txt a pointer to the data buffer to be parsed.
 * @param   len the length, in bytes, of the data buffer to be parsed.
 * @return  a pointer to the DIME management record on success, or NULL on failure.
 */
dime_record_t *_parse_dime_record(const char *txt, size_t len) {

    dime_record_t *result;
    unsigned char *keydata, *tmpdata;
    char *txtcopy, *ptr, *startopt, *startval, *tmpstr;
    size_t blen, dlen;
    int parse_dbg_level = 3;

    if (!txt || !len) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(result = malloc(sizeof(dime_record_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "unable to allocate space for DIME record");
    }

    if (!(txtcopy = malloc(len + 1))) {
        PUSH_ERROR_SYSCALL("malloc");
        free(result);
        RET_ERROR_PTR(ERR_NOMEM, "unable to allocate space for DIME record copy");
    }

    memset(txtcopy, 0, len + 1);
    memcpy(txtcopy, txt, len);

    // Fill in default values.
    memset(result, 0, sizeof(dime_record_t));

    // Fill in the defaults
    result->version = 1;
    result->policy = msg_experimental;
    result->subdomain = sub_strict;

    // Loop through the individual DIME record fields and populate their values.
    ptr = startopt = startval = txtcopy;

    _dbgprint(parse_dbg_level, "Started parsing DIME management record...\n");

    // Weird loop condition, but we want to include the null byte as a possibility.
    while (len + 1) {

        // Be careful; base64-encoded fields can end in sequences of multiple =
        if ((*ptr == '=') && (*(ptr + 1) != '=') && *(ptr + 1) && !chr_isspace(*(ptr + 1))) {
            *ptr = 0;
            startval = ptr + 1;
        } else if ((*ptr == ' ') || (*ptr == ';') || (!*ptr)) {
            *ptr = 0;

            if (!strcasecmp(startopt, "ver") || !strcasecmp(startopt, "version")) {

                if (!(result->version = atoi(startval))) {
                    _destroy_dime_record(result);
                    free(txtcopy);
                    RET_ERROR_PTR(ERR_UNSPEC, "DIME record contained invalid version field");
                }

                if (result->version != DIME_VERSION_NO) {
                    PUSH_ERROR_FMT(ERR_UNSPEC, "DIME record version must be %u; read %u", DIME_VERSION_NO, result->version);
                    _destroy_dime_record(result);
                    free(txtcopy);
                    return NULL;
                }

                _dbgprint(parse_dbg_level, "--- VERSION:    ");
            } else if (!strcasecmp(startopt, "pok") || !strcasecmp(startopt, "primary")) {

                if (strlen(startval) != ED25519_KEY_B64_SIZE) {
                    PUSH_ERROR_FMT(ERR_UNSPEC, "DIME record contained primary org key of bad length {received %zu, expected %u bytes}", strlen(startval), ED25519_KEY_B64_SIZE);
                    _destroy_dime_record(result);
                    free(txtcopy);
                    return NULL;
                }

                if (!(keydata = _b64decode_nopad(startval, strlen(startval), &blen))) {
                    _destroy_dime_record(result);
                    free(txtcopy);
                    RET_ERROR_PTR(ERR_UNSPEC, "could not base64-decode public org key data in DIME management record");
                }

                if (blen != ED25519_KEY_SIZE) {
                    _destroy_dime_record(result);
                    free(txtcopy);
                    free(keydata);
                    RET_ERROR_PTR(ERR_UNSPEC, "armored public org key in DIME management record was unexpected size");
                }

                if (!(result->pubkey = _ptr_chain_add(result->pubkey, keydata))) {
                    _destroy_dime_record(result);
                    free(txtcopy);
                    free(keydata);
                    RET_ERROR_PTR(ERR_UNSPEC, "could not add org pub key to chain");
                }

                _dbgprint(parse_dbg_level, "--- PUBLIC KEY: ");
            } else if (!strcasecmp(startopt, "tls")) {

                if (strlen(startval) != ED25519_SIG_B64_SIZE) {
                    PUSH_ERROR_FMT(ERR_UNSPEC, "DIME record contained TLS certificate signature of unexpected length {received %zu, expected %u bytes}", strlen(startval), ED25519_SIG_B64_SIZE);
                    _destroy_dime_record(result);
                    free(txtcopy);
                    return NULL;
                }

                if (!(tmpdata = _b64decode_nopad(startval, strlen(startval), &dlen))) {
                    _destroy_dime_record(result);
                    free(txtcopy);
                    RET_ERROR_PTR(ERR_UNSPEC, "could not base64-decode TLS HMAC in DIME management record");
                }

                if (dlen != ED25519_SIG_SIZE) {
                    _destroy_dime_record(result);
                    free(txtcopy);
                    free(tmpdata);
                    RET_ERROR_PTR(ERR_UNSPEC, "TLS HMAC in DIME management record was unexpected size");
                }

                if (!(result->tlssig = _ptr_chain_add(result->tlssig, tmpdata))) {
                    _destroy_dime_record(result);
                    free(txtcopy);
                    free(tmpdata);
                    RET_ERROR_PTR(ERR_UNSPEC, "could not add TLS signature to chain");
                }

                _dbgprint(parse_dbg_level, "--- TLS:        ");
            } else if (!strcasecmp(startopt, "pol") || !strcasecmp(startopt, "policy")) {

                if (!strcasecmp(startval, "experimental")) {
                    result->policy = msg_experimental;
                } else if (!strcasecmp(startval, "mixed")) {
                    result->policy = msg_mixed;
                } else if (!strcasecmp(startval, "strict")) {
                    result->policy = msg_strict;
                } else {
                    PUSH_ERROR_FMT(ERR_UNSPEC, "DIME record contained invalid message policy value: %s", startval);
                    _destroy_dime_record(result);
                    free(txtcopy);
                    return NULL;
                }

                _dbgprint(parse_dbg_level, "--- MSG POLICY: ");
            } else if (!strcasecmp(startopt, "syn") || !strcasecmp(startopt, "syndicates")) {
                _dbgprint(parse_dbg_level, "--- SYNDICATES: ");
            } else if (!strcasecmp(startopt, "dx") || !strcasecmp(startopt, "deliver")) {

                if (!(tmpstr = strdup(startval))) {
                    PUSH_ERROR_SYSCALL("strdup");
                    _destroy_dime_record(result);
                    free(txtcopy);
                    RET_ERROR_PTR(ERR_NOMEM, "could not allocate space to parse DIME record");
                }

                if (!(result->dx = _ptr_chain_add(result->dx, tmpstr))) {
                    _destroy_dime_record(result);
                    free(txtcopy);
                    free(tmpstr);
                    RET_ERROR_PTR(ERR_UNSPEC, "could not add DX server to chain");
                }

                _dbgprint(parse_dbg_level, "--- DX:         ");
            } else if (!strcasecmp(startopt, "exp") || !strcasecmp(startopt, "expiry")) {
                _dbgprint(parse_dbg_level, "--- EXPIRY:     ");
            } else if (!strcasecmp(startopt, "sub") || !strcasecmp(startopt, "subdomain")) {

                if (!strcasecmp(startval, "strict")) {
                    result->subdomain = sub_strict;
                } else if (!strcasecmp(startval, "relaxed")) {
                    result->subdomain = sub_relaxed;
                } else if (!strcasecmp(startval, "explicit")) {
                    result->subdomain = sub_explicit;
                } else {
                    PUSH_ERROR_FMT(ERR_UNSPEC, "DIME record contained invalid subdomain policy value: %s", startval);
                    _destroy_dime_record(result);
                    free(txtcopy);
                    return NULL;
                }

                _dbgprint(parse_dbg_level, "--- SUBDOMAIN:  ");
            } else {
                _dbgprint(parse_dbg_level, "* Unrecognized field: \"%s\" ", startopt);
            }

            _dbgprint(parse_dbg_level, " [%s]\n", startval);

            startopt = ptr + 1;
        }

        len--;
        ptr++;
    }

    free(txtcopy);

    // Validate the result.
    if (_validate_dime_record(result) < 0) {
        _destroy_dime_record(result);
        RET_ERROR_PTR(ERR_UNSPEC, "DIME management record validation failed");
    }

    return result;
}

/**
 * @brief   Retrieve a DIME management record for a given dark domain via DNS.
 * @param   domain      a null-terminated string containing the name of the dark domain to be queried.
 * @param   ttl     an optional pointer to a variable that will receive the current TTL value of the DIME record's TXT RR.
 * @param   use_cache   if set, use the cache as the first-line resolver; if 0, only perform live network lookups.
 * @return  NULL on failure, or a pointer to a populated dime_record_t structure on success.
 */
dime_record_t *_get_dime_record(const char *domain, unsigned long *ttl, int use_cache) {

    cached_object_t *cached, *newobj, *cloned;
    dime_record_t *result;
    char *qstr, *txtans;
    size_t qlen;
    int validated, refresh = 0;

    if (!domain) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

// TODO: This needs cleanup. We should not be using any internal functions, if possible.
    if (use_cache && (cached = _find_cached_object(domain, &(cached_stores[cached_data_drec])))) {

        // In this case, we have a DIME management record with an expired TTL, but that has not yet reached
        // its absolute expiration date. The desired behavior is to fetch the requested record again,
        // returning all supplied data, except preserving the original expiration timestamp for security purposes.
        if (!_is_object_expired(cached, &refresh) && refresh) {
            _dbgprint(1, "Attempting to refresh DIME record that exceeded TTL.\n");

            if ((result = _get_dime_record(domain, ttl, 0))) {
                // Attach the old expiration to the new record.
                result->expiry = cached->expiration;

                // If for some reason we get a cache error, report it but return the old (original value).
                if (!(newobj = _create_cached_object(cached_data_drec, (ttl ? *ttl : 0), cached->expiration, result, 1, 1))) {
                    fprintf(stderr, "Error: unable to refresh cached DIME management record entry.\n");
                    _destroy_dime_record(result);
                    return ((dime_record_t *)_get_cache_obj_data(cached));
                }

                if (!_replace_object(cached, newobj, 0)) {
                    fprintf(stderr, "Error: unable to update cached DIME management record entry.\n");
                    _destroy_cache_entry(newobj);
                    return ((dime_record_t *)_get_cache_obj_data(cached));
                }

                _dbgprint(1, "Successfully refreshed DIME record; retaining old expiry.\n");
                // TODO: does this need to be wrapped?
                return ((dime_record_t *)newobj->data);
            } else {

                if (get_last_error()) {
                    fprintf(stderr, "Error: could not refresh DIME record.\n");
                    dump_error_stack();
                    _clear_error_stack();
                } else {
                    fprintf(stderr, "Error: unable to refresh DIME record that exceeded TTL.\n");
                }

            }

        }

        _dbgprint(2, "Returning cached DIME record.\n");
        return ((dime_record_t *)_get_cache_obj_data(cached));
    } else if (use_cache && get_last_error()) {
        fprintf(stderr, "Error: could not lookup DIME record in cache.\n");
        dump_error_stack();
        _clear_error_stack();
    }

    // 2 extra bytes for the "." label separator and for the terminating null.
    qlen = strlen(domain) + strlen(DIME_RECORD_DNS_PREFIX) + 2;

    if (!(qstr = malloc(qlen))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for query string");
    }

    memset(qstr, 0, qlen);
    snprintf(qstr, qlen, "%s.%s", DIME_RECORD_DNS_PREFIX, domain);

    txtans = _get_txt_record(qstr, ttl, &validated);
    free(qstr);

    if (!txtans) {
        RET_ERROR_PTR(ERR_UNSPEC, "failed to receive TXT query answer");
    }

    if ((result = _parse_dime_record(txtans, strlen(txtans))) && use_cache) {
        free(txtans);
        result->validated = validated;

        if (!(cloned = _add_cached_object(domain, &(cached_stores[cached_data_drec]), (ttl ? *ttl : 0), result->expiry, result, 1, 1))) {
            RET_ERROR_PTR(ERR_UNSPEC, NULL);
        }

        // TODO: need to free result?
        return (_get_cache_obj_data(cloned));
    } else if (!result) {
        PUSH_ERROR(ERR_UNSPEC, "could not parse DIME management record");
    }

    free(txtans);

    return result;
}


/**
 * @brief   Validate the syntax of a DIME record for correctness.
 * @param   record  a pointer to the DIME management record to be validated.
 * @return  0 if the record was validated successfully or -1 on error.
 */
int _validate_dime_record(const dime_record_t *record) {

    if (!record) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    // Make sure that all mandatory fields are present. At the moment, there's really only one such field.
    if (!record->pubkey || !*(record->pubkey)) {
        RET_ERROR_INT(ERR_BAD_PARAM, "DIME record was missing mandatory public key field");
    }

    // Do we really want to do this?
    if (record->version != DIME_VERSION_NO) {
        RET_ERROR_INT_FMT(ERR_UNSPEC, "DIME record contained unsupported version number {ver = %u}", (unsigned int)record->version);
    }

    // It's very unlikely this will ever happen unless the record was improperly deserialized.
    if ((record->policy != msg_experimental) && (record->policy != msg_mixed) && (record->policy != msg_strict)) {
        RET_ERROR_INT(ERR_UNSPEC, "DIME record contained invalid policy value");
    }

    if ((record->subdomain != sub_strict) && (record->subdomain != sub_relaxed) && (record->subdomain != sub_explicit)) {
        RET_ERROR_INT(ERR_UNSPEC, "DIME management record contain invalid subdomain policy value");
    }

    return 0;

}


/**
 * @brief   Retrieve and parse a DIME management record from an input file.
 * @param   filename    a null-terminated string containing the filename containing the DIME management record to be parsed.
 * @param   domain      a null-terminated string containing the name of the dark domain with which the DIME record is to be associated.
 * @return  NULL on failure or a pointer to the retrieved DIME management record on success.
 */
dime_record_t *_get_dime_record_from_file(const char *filename, const char *domain) {

    dime_record_t *result = NULL;
    cached_object_t *cached;
    char buf[8192];
    FILE *fp;

    if (!filename || !domain) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(fp = fopen(filename, "r"))) {
        PUSH_ERROR_SYSCALL("fopen");
        RET_ERROR_PTR(ERR_UNSPEC, "could not open DIME record file");
    }

    memset(buf, 0, sizeof(buf));

    // Make sure we're at least reading a line that has some content.
    while (fgets(buf, sizeof(buf), fp)) {

        for (size_t i = 0; i < strlen(buf); i++) {

            if (chr_isprint(buf[i])) {

                if (buf[strlen(buf) - 1] == '\n') {
                    buf[strlen(buf) - 1] = 0;
                }

                // Since this is supplied in the file, this doesn't make it to the persistent cache. It also has an unlimited TTL.
                if ((result = _parse_dime_record(buf, strlen(buf)))) {
                    cached = _add_cached_object_forced(domain, &(cached_stores[cached_data_drec]), 0, 0, result, 0, 0);

                    if (!cached) {
                        fprintf(stderr, "Error: unable to add DIME record from file to cache:\n");
                        dump_error_stack();
                        _clear_error_stack();
                    } else {
                        result = _get_cache_obj_data(cached);
                    }

                    // TODO: memory leak from result?
                } else {
                    fclose(fp);
                    RET_ERROR_PTR(ERR_UNSPEC, "could not parse DIME management record");
                }

                fclose(fp);

                return result;
            }

        }

    }

    fclose(fp);

    return result;
}
