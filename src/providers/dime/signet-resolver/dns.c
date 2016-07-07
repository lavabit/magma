#include "dime/signet-resolver/dns.h"

#include "dime/signet-resolver/cache.h"
#include "dime/common/misc.h"
#include "dime/common/error.h"

#include "providers/symbols.h"

#define INITIALIZE_DNS() { if (!_dns_initialized && (_initialize_resolver() < 0)) { RET_ERROR_PTR(ERR_UNSPEC, "failed to initialize DNS resolver"); } }

static int _dns_initialized = 0;


/**
 * @brief   Append a DNS label in uncompressed, canonical format to a dynamic buffer.
 * @note    This function should be called for the first time with *buf and *blen set to NULL.
 * @param   buf a pointer to the address of the output buffer that will be resized to hold the result, or NULL to allocate one.
 * @param   blen    a pointer to a variable that will both hold the initial size of and receive the newly resized output buffer
 *                      following the completion of the append operation.
 * @param       name    a null-terminated string containing the domain label to be stored in the buffer in canonical form.
 * @return  the new size of the (re)allocated output buffer, or 0 on failure.
 *
 */
size_t _mem_append_canon(unsigned char **buf, size_t *blen, const char *name) {

    unsigned char *outbuf, *outptr;
    size_t tmpsize, nlen, llen, marker, outlen, i;

    if (!buf || !blen) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    tmpsize = (nlen = strlen(name)) * 2;

    if (!(outbuf = malloc(tmpsize))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_UINT(ERR_NOMEM, "could not allocate space to pack canonical DNS name");
    }

    memset(outbuf, 0, tmpsize);
    outptr = outbuf;
    i = marker = 0;

    // Split the domain name into labels of a maximum of 63 bytes each.
    while (i <= nlen) {

        // Either we hit a period or we hit the end of the string and have a remaining substring to be stuffed in a label.
        if (!name[i] || name[i] == '.') {
            llen = i - marker;

            if (llen > NS_MAXLABEL) {
                free(outbuf);
                RET_ERROR_UINT(ERR_UNSPEC, "unable to pack canonical name; label would exceed maximum size");
            }

            *outptr++ = llen;
            memcpy(outptr, &(name[marker]), llen);
            outptr += llen;
            marker = ++i;
        }

        i++;
    }

    *outptr++ = 0;

    // Make sure the resulting packed domain doesn't exceed 255 bytes.
    if (((outlen = outptr - outbuf)) > NS_MAXCDNAME) {
        free(outbuf);
        RET_ERROR_UINT(ERR_UNSPEC, "unable to pack canonical name; domain would exceed maximum size");
    }

    // Make one final sweep; all letters are expected to be in lowercase format.
    for (i = 0; i < outlen; i++) {

        if (isupper(outbuf[i])) {
            outbuf[i] = tolower(outbuf[i]);
        }

    }

    // Special case: zero-size label (".")
    if (!nlen || ((nlen == 1) && (*name == '.'))) {
        outlen = 1;
    }

    i = _mem_append(buf, blen, outbuf, outlen);
    free(outbuf);

    if (!i) {
        RET_ERROR_UINT(ERR_NOMEM, NULL);
    }

    return i;
}


/**
 * @brief   Calculate the keytag value for a DNSKEY resource record.
 * @note    This value is not necessarily unique, but should distinguish between keys of different owners/algorithms.
 *              As per RFC 4034, this applies to all algorithms except of type 1.
 * @param   rdata   a pointer to the start of the rdata buffer of the DNSKEY RR.
 * @param   rdlen   the rdlength (total size, in bytes) of the DNSKEY RR.
 * @return  a keytag value for the specified DNSKEY RR to be referenced in RRSIG (or DS) records.
 */
unsigned int _get_keytag(const unsigned char *rdata, size_t rdlen) {

    unsigned long result;
    size_t i;

    if (!rdata || !rdlen) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    for (result = 0, i = 0; i < rdlen; i++) {
        result += (i & 1) ? rdata[i] : rdata[i] << 8;
    }

    result += (result >> 16) & 0xffff;

    return (result & 0xffff);
}


/**
 * @brief   Extract an RSA public key from a DNSKEY resource record.
 * @note    The parameters to this function are not the same as the RR's rdata/rdlen, but rather, the
 *              RSA key data that usually begins 4 bytes into this buffer.
 * @param   rdptr   a pointer to the start of the raw RSA key data inside the DNSKEY RR's rdata.
 * @param   rdlen   the length, in bytes, of the encoded RSA public key.
 * @return  NULL on failure, or a pointer to a newly allocated RSA public key on success.
 */
RSA *_get_rsa_dnskey(const unsigned char *rdptr, size_t rdlen) {

    RSA *result;
    BIGNUM *mod, *exp;
    uint16_t explen;
    const unsigned char *ptr = rdptr;
    size_t len = rdlen, keysize;

    if (!rdptr || !rdlen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    explen = *ptr++;
    len--;

    // If this first byte is > 0, then it's the entire length as a one byte field.
    // Otherwise, if it's zero, the following 2 bytes store the exponent length.
    if (!explen) {

        if (len < NS_INT16SZ) {
            RET_ERROR_PTR(ERR_UNSPEC, "DNSKEY rdata didn't contain enough data");
        }

        NS_GET16(explen, ptr);
        ptr += NS_INT16SZ;
        len -= NS_INT16SZ;
    }

    if (len < explen) {
        RET_ERROR_PTR(ERR_UNSPEC, "DNSKEY rdata terminated before end of exponent");
    }

    if (!(exp = BN_bin2bn_d(ptr, explen, NULL))) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "unable to read exponent from DNSKEY rdata");
    }

    ptr += explen;
    len -= explen;

    // The remainder is the modulus.
    if (!(mod = BN_bin2bn_d(ptr, len, NULL))) {
        PUSH_ERROR_OPENSSL();
        BN_free_d(exp);
        RET_ERROR_PTR_FMT(ERR_UNSPEC, "unable to extract modulus from DNSKEY rdata {%u bytes}", (unsigned int)len);
    }

    // TODO: We need more validation in the form of an algorithm passed into this function.
    // RSA/SHA-256 keeps have a minimum size of 512 bits, while RSA/SHA-512 requires at least 1024.

    // The number of bits in the modulus determines the public key size;
    // RFC 3110 demands that this value should not be less than 512 bits or greater than 4096 bits.
    if (((keysize = BN_num_bits_d(mod)) < 512) || (keysize > 4096)) {
        PUSH_ERROR_OPENSSL();
        BN_free_d(exp);
        BN_free_d(mod);
        RET_ERROR_PTR(ERR_UNSPEC, "modulus doesn't fall in mandatory length range of 512-4096 bits");
    }

    if (!(result = RSA_new_d())) {
        PUSH_ERROR_OPENSSL();
        BN_free_d(exp);
        BN_free_d(mod);
        RET_ERROR_PTR(ERR_UNSPEC, "unable to allocate new RSA public key holder");
    }

    result->n = mod;
    result->e = exp;
    result->d = NULL;
    result->p = NULL;
    result->q = NULL;

    //printf("MOD dec = [%s], hex = [%s]\n", BN_bn2dec(mod), BN_bn2hex_d(mod));
    //printf("EXP dec = [%s], hex = [%s]\n", BN_bn2dec(exp), BN_bn2hex_d(exp));
    _dbgprint(3, "Read DNSKEY, exp. len: %u, total raw pubkey len: %u\n", explen, (unsigned int)len);

    return result;
}

/**
 * @brief   Verify that an RRSIG record has been signed properly using a given RSA public key.
 * @note    According to RFC 4034, the signature calculation is computed over the following data:
 *              RRSIG_RDATA | RR(1) | RR(2) | RR(s)
 *              where RRSIG_RDATA is the wire format of the RRSIG RDATA with the signer's name in canonical form and NO signature field
 *              RR(s) is the RRset of all matching RR's.
 *              RR(x): owner | type | class | ttl (original ttl) | rdlen | rdata

 *          These fields of each RR and the RRSIG RR must match: owner, class, type=covered, ttl=original ttl
 *      The RRSET must be in canonical order, and the DNS names in the RDATA must be in canonical form.

 *      Each domain name in an RR must be fully expanded and qualified, and in lowercase.
 *      Finally, the RR's need to be sorted in canonical order.
 * @param   label       a pointer to the label of the RRSIG record being verified.
 * @param   algorithm   the value of the algorithm used to produce the signature (ex. NS_ALG_RSASHA1).
 * @param   pubkey      a pointer to the RSA public key that will be used to verify the specified signature.
 * @param   rrsig       a pointer to the beginning of the RRSIG record's rdata content.
 * @param   sigbuf      a pointer to the start of the input buffer containing the RR signature data.
 * @param   siglen      the length, in bytes, of the RR signature data.
 * @param   dhandle     a pointer to the handle of the DNS answer message containing the RRSIG record being verified.
 * @return  -1 on general error, 0 if the RSA signature value did not match the data, or 1 if the signature was valid.
 */
int _rsa_verify_record(const char *label, unsigned char algorithm, RSA *pubkey, const unsigned char *rrsig, const unsigned char *sigbuf, size_t siglen, ns_msg *dhandle) {

    EVP_MD_CTX ctx;
    const EVP_MD *digtype;
    EVP_PKEY *pkey;
    ns_rr rr;
    rrsig_rr_t *rrsigrr = (rrsig_rr_t *)rrsig;
    unsigned char *hdata = NULL;
    char nbuf[MAXDNAME];
    size_t hlen = 0, nanswers, nmatched = 0;
    uint16_t u16;
    int result, *corder;

    // TODO: We have dhandle... so is label necessary; also, if we have rrsig, are algorithm+sigbuf+siglen therefore necessary parameters?

    if (!pubkey || !rrsig || !sigbuf || !siglen || !dhandle) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    // Make sure this is an algorithm we know how to handle.
    if (algorithm != NS_ALG_RSASHA1 && algorithm != NS_ALG_RSASHA256 && algorithm != NS_ALG_RSASHA512) {
        RET_ERROR_INT_FMT(ERR_UNSPEC, "could not sign RRSIG because of unsupported requested algorithm: %u", algorithm);
    }

    // First we pack the RRSIG rdata, up until the signer name (the very last byte of the structure).
    if (!_mem_append(&hdata, &hlen, rrsig, sizeof(rrsig_rr_t) - 1)) {
        RET_ERROR_INT(ERR_NOMEM, "unable to pack RRSIG RDATA for verification");
    }

    // Then we add the signer name, but in canonical form.
    memset(nbuf, 0, sizeof(nbuf));

    // First grab the uncompressed name of the signer.
    if (ns_name_uncompress(ns_msg_base(*dhandle), ns_msg_end(*dhandle) + 1, rrsig + sizeof(rrsig_rr_t) - 1, (char *)nbuf, sizeof(nbuf)) == -1) {
        PUSH_ERROR_RESOLVER("ns_name_uncompress");
        free(hdata);
        RET_ERROR_INT(ERR_UNSPEC, "could not sign RRSIG: signer name field was invalid");
    }

    // Append the signer's canonical name to the previous contents of the rdata.
    if (!_mem_append_canon(&hdata, &hlen, (char *)nbuf)) {
        free(hdata);
        RET_ERROR_INT(ERR_UNSPEC, "could not pack canonical signing name for verification");
    }

    // Now it's time to find all of our matching resource records.
    nanswers = ns_msg_count(*dhandle, ns_s_an);

    if (!(corder = malloc(nanswers * sizeof(int)))) {
        PUSH_ERROR_SYSCALL("malloc");
        free(hdata);
        RET_ERROR_INT(ERR_NOMEM, "could not create buffer to sort RRs in canonical order");
    }

    if (_sort_rrs_canonical(dhandle, corder, nanswers) < 0) {
        free(hdata);
        free(corder);
        RET_ERROR_INT(ERR_UNSPEC, "could not sort RRs into canonical order for signature verification");
    }

    for(size_t i = 0; i < nanswers; i++) {

        if (ns_parserr(dhandle, ns_s_an, corder[i], &rr) < 0) {
            PUSH_ERROR_RESOLVER("ns_parserr");
            fprintf(stderr, "Error occurred parsing answer!\n");
            continue;
        }

        // The RRSIG and covered RR must be of the same class.
        if (ns_rr_class(rr) != ns_c_in) {
            _dbgprint(1, "Skipped over answer with class != IN ...\n");
            continue;
        }

        // The RRSIG and covered RR must be of the same type.
        if (ns_rr_type(rr) != htons(rrsigrr->covered)) {
            _dbgprint(5, "Skipped over answer not covered by RRSIG ...\n");
            continue;
        }

        // Compare the owner of the covered RR and RRSIG to make sure they're equal.
        // Not entirely sure this is mandated...
        if (strcmp(ns_rr_name(rr), label)) {
            _dbgprint(5, "Skipped over RR: did not match owner of RRSIG [%s] vs [%s]...\n", ns_rr_name(rr), label);
            continue;
        }

        // TODO: According to RFC 4034, the RRSIG RR must have the same TTL value as the RRSET is covers. This check needs to be added.
/*      if (0 != ns_rr_ttl(rr)) {
                        _dbgprint(5, "Skipped over RR: did not match ttl of RRSIG [%u] vs [%u]...\n", 0, ns_rr_ttl(rr));
                        continue;
                } */

        // We've found a matching RR. Now we have to pack the appropriate fields of it into our signed buffer.
        // First start with the owner.
        if (!_mem_append_canon(&hdata, &hlen, label)) {
            free(hdata);
            free(corder);
            RET_ERROR_INT(ERR_UNSPEC, "could not sign RRSIG: error packing canonical form of owner");
        }

        // Next, the type.
        u16 = htons(ns_rr_type(rr));
        _mem_append(&hdata, &hlen, (unsigned char *)&u16, sizeof(u16));
        // Then the class.
        u16 = htons(ns_rr_class(rr));
        _mem_append(&hdata, &hlen, (unsigned char *)&u16, sizeof(u16));
        // The TTL is actually the original TTL taken from the RRSIG record.
        _mem_append(&hdata, &hlen, (unsigned char *)&(rrsigrr->ottl), 4);
        // The second-to-last part of the RR to pack is the rdlength.
        u16 = htons(ns_rr_rdlen(rr));
        _mem_append(&hdata, &hlen, (unsigned char *)&u16, sizeof(u16));
        // And finally, the rdata itself.
        // TODO: Technically, DNS names in the RDATA field must be in canonical form.
        // BUT, for our current purposes we're not really interested in any such RR, since all we're ever expecting to
        // see signed is either a TXT, DNSKEY, or DS record, and none of these contain name labels.
        _mem_append(&hdata, &hlen, (unsigned char *)ns_rr_rdata(rr), ns_rr_rdlen(rr));
        nmatched++;

        //printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ [%s] len %u had rr name [%s]\n", nbuf2, hlen, nbuf);
    }

    free(corder);

    if (!nmatched) {
        free(hdata);
        RET_ERROR_INT(ERR_UNSPEC, "could not validate RRSIG SHA hash: no matching RRs were found");
    }

    if (!(pkey = EVP_PKEY_new_d())) {
        PUSH_ERROR_OPENSSL();
        free(hdata);
        RET_ERROR_INT(ERR_UNSPEC, "an unexpected error occurred while verifying RR signature");
    }

    if (EVP_PKEY_set1_RSA_d(pkey, pubkey) != 1) {
        PUSH_ERROR_OPENSSL();
        free(hdata);
        RET_ERROR_INT(ERR_UNSPEC, "could not set key for RR signature verification");
    }

    EVP_MD_CTX_init_d(&ctx);

    if (algorithm == NS_ALG_RSASHA1) {
        digtype = EVP_sha1_d();
    } else if (algorithm == NS_ALG_RSASHA256) {
        digtype = EVP_sha256_d();
    } else {
        digtype = EVP_sha512_d();
    }

    if (EVP_DigestInit_d(&ctx, digtype) != 1) {
        PUSH_ERROR_OPENSSL();
        free(hdata);
        RET_ERROR_INT(ERR_UNSPEC, "could not set digest for RR signature verification");
    }

    if (EVP_DigestUpdate_d(&ctx, hdata, hlen) != 1) {
        PUSH_ERROR_OPENSSL();
        free(hdata);
        RET_ERROR_INT(ERR_UNSPEC, "could not read hash for RR signature verification");
    }

    result = EVP_VerifyFinal_d(&ctx, sigbuf, siglen, pkey);
    free(hdata);

    if (result == 1) {
        _dbgprint(1, "Signature verification succeeded (signed = %s, keytag = %u, covered = %u, alg = %u)\n",
                  label, ntohs(rrsigrr->key_tag), ntohs(rrsigrr->covered), rrsigrr->algorithm);
    } else if (!result) {
        fprintf(stderr, "Signature verification failed (signed = %s, keytag = %u, covered = %u, alg = %u)\n",
                label, ntohs(rrsigrr->key_tag), ntohs(rrsigrr->covered), rrsigrr->algorithm);
    } else {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "error verifying RRSIG record");
    }

//ALERT_PRINT(stderr, _dump_cache(cached_data_dnskey, 1, 1));
//ALERT_PRINT(stderr, _dump_cache(cached_data_ds, 1, 1));

    return result;
}


/**
 * @brief   Load a collection of DNS key(s) from a config file (into the object cache).
 * @note    Any DNSKEY entry loaded from this file will be treated as verified and having final authority.
 *              This function will fail if there are no root [.] DNSKEY entries supplied in the file.
 * @param   filename    a null-terminated string containing the name of the file containing the DNS key entries.
 * @return  0 if key(s) were successfully loaded from the file or -1 on failure.
 */
int _load_dnskey_file(const char *filename) {

    FILE *fp;
    dnskey_t *dk;
    RSA *pubkey;
    dnskey_t cmp;
    unsigned char *rawkey, *rdata = NULL;
    char dname[MAXDNAME], line[4096], *lptr, *optr;
    unsigned int flags, protocol, algorithm, keytag;
    size_t rawlen, slen, rdlen = 0;
    uint16_t tmp16;
    uint8_t tmp8;

    if (!filename) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!(fp = fopen(filename, "r"))) {
        PUSH_ERROR_SYSCALL("fopen");
        RET_ERROR_INT_FMT(ERR_UNSPEC, "could not load root key config file: %s", filename);
    }

    while (!feof(fp)) {
        memset(line, 0, sizeof(line));

        if (!fgets(line, sizeof(line), fp)) {
            break;
        }

        if ((slen = strlen(line)) && ((line[slen - 1] == '\n') || (line[slen - 1] == '\r'))) {
            line[slen - 1] = 0;
        }

        lptr = line;

        while (chr_isspace(*lptr)) {
            lptr++;
        }

        if (*lptr == '#') {
            continue;
        }

        memset(dname, 0, sizeof(dname));

        if (sscanf(lptr, "%s initial-key %u %u %u ", dname, &flags, &protocol, &algorithm) != 4) {
            continue;
        }

        if (flags & ~(DNSKEY_RR_FLAG_ZK | DNSKEY_RR_FLAG_SEP)) {
            fclose(fp);
            RET_ERROR_INT_FMT(ERR_UNSPEC, "encountered unexpected key flags in DNS key file: %u", flags);
        }

        if (protocol != DNSKEY_RR_PROTO) {
            fclose(fp);
            RET_ERROR_INT_FMT(ERR_UNSPEC, "encountered unexpected protocol in DNS key file: %u", protocol);
        }

        if (algorithm != NS_ALG_RSASHA1 && algorithm != NS_ALG_RSASHA256 && algorithm != NS_ALG_RSASHA512) {
            fclose(fp);
            RET_ERROR_INT_FMT(ERR_UNSPEC, "encountered unexpected algorithm: %u", algorithm);
        }

        while (*lptr && *lptr != '"') {
            lptr++;
        }

        if (!*lptr++) {
            fclose(fp);
            RET_ERROR_INT(ERR_UNSPEC, "encountered unexpected end of line");
        }

        optr = lptr;

        while (*lptr && *lptr != '"') {
            lptr++;
        }

        if (!*lptr) {
            fclose(fp);
            RET_ERROR_INT(ERR_UNSPEC, "encountered unexpected end of line");
        }

        *lptr++ = 0;

        while (chr_isspace(*lptr)) {
            lptr++;
        }

        if (*lptr != ';') {
            fclose(fp);
            RET_ERROR_INT(ERR_UNSPEC, "encountered unexpected end of line");
        }

        /* Get rid of any spaces */
        for (lptr = optr; *lptr; lptr++) {

            if (chr_isspace(*lptr)) {
                memmove(lptr, lptr + 1, strlen(lptr));
            }

        }

        if (!(rawkey = _b64decode(optr, strlen(optr), &rawlen))) {
            fclose(fp);
            RET_ERROR_INT(ERR_UNSPEC, "failed to extract base64 encoded public key from root key config file");
        }

        if (!(pubkey = _get_rsa_dnskey(rawkey, rawlen))) {
            fclose(fp);
            RET_ERROR_INT(ERR_UNSPEC, "failed to extract public key from DNSKEY RR");
        }

        // Before we add the DNSKEY entry, we need to calculate its keytag by constructing a dummy DNSKEY RR.
        tmp16 = htons(flags);
        _mem_append(&rdata, &rdlen, (unsigned char *)&tmp16, sizeof(tmp16));
        tmp8 = protocol;
        _mem_append(&rdata, &rdlen, &tmp8, sizeof(tmp8));
        tmp8 = algorithm;
        _mem_append(&rdata, &rdlen, &tmp8, sizeof(tmp8));
        _mem_append(&rdata, &rdlen, rawkey, rawlen);

        if (!(keytag = _get_keytag(rdata, rdlen))) {
            fclose(fp);
            RET_ERROR_INT(ERR_UNSPEC, "could not get tag value for DNSKEY");
        }

        // The ttl is unlimited and we don't want to save this entry to the cache.
        if (!(dk = _add_dnskey_entry_rsa(dname, flags, algorithm, pubkey, keytag, rdata, rdlen, 0, 0, 1))) {
            fclose(fp);
            RET_ERROR_INT(ERR_UNSPEC, "unable to import DNS root key entry");
        }

        // Any key from a local file is automatically validated.
        dk->validated = 1;

        free(rdata);
        free(rawkey);
        rdata = NULL;
        rdlen = 0;
    }

    fclose(fp);

    // Make sure that we got our root key(s).
    memset(&cmp, 0, sizeof(cmp));
    cmp.label = (char *)".";

    if (!_find_cached_object_cmp(&cmp, &(cached_stores[cached_data_dnskey]), &_dnskey_domain_comparator)) {
        RET_ERROR_INT(ERR_UNSPEC, "config file did not contain any root DNSKEY entries");
    }

    return 0;
}


/**
 * @brief   Trace a DNSKEY backwards through its DS entry to the root, to see if it is validated.
 * @note    Chain validation works recursively on a starting DNSKEY in the following way:
 *              1. For each of the DNSKEYs that were used to sign this DNSKEY record,
 *                 2. For each of the signing DNSKEY's DS records,
 *                    3. Validate the signing DNSKEY of the DS record.
 *              In the end, a DNSKEY can only be considered if its validated flag has been set, or
 *              a key by which it has been signed has also been validated. Only the keys supplied from
 *              the function _load_dnskey_file() carry the validation flag by default.
 * @param   dk  a pointer to the DNSKEY to be validated.
 * @return  -1 on general error, 0 if the specified key could not be validated, or 1 if it is.
 */
int _is_validated_key(dnskey_t *dk) {

    dnskey_t **dkptr, **dkptr2;
    ds_t **dsptr;

    if (!dk) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (dk->validated) {
        return 1;
    }
    // Root keys are the end of the line for our recursion.
    else if (IS_ROOT_LABEL(dk->label)) {
        return 0;
    }
    // If this key isn't validated and isn't backed up by any DS records, it's a failure.
    else if (!dk->signkeys) {
        return 0;
    }

    for (dkptr = dk->signkeys; *dkptr; dkptr++) {

        if ((*dkptr)->dse) {

            for (dsptr = (*dkptr)->dse; *dsptr; dsptr++) {

                if ((*dsptr)->signkeys) {

                    for (dkptr2 = (*dsptr)->signkeys; *dkptr2; dkptr2++) {

                        // Since the validation hasn't already occurred
                        if (_is_validated_key(*dkptr2)) {
                            dk->validated = 1;
                            return 1;
                        }

                    }

                }

            }

        }

    }

    return 0;
}

/**
 * @brief   Perform a SHA-family hash against a DNSKEY public key.
 * @see     compuute_sha_hash()
 * @note    This hash value is the data that is used to link a DNSKEY to its corresponding DS record.
 * @note    According to section 5.1.4 of RFC 4034, the digest is taken over the concatenation of:
 *              the owner name of the DNSKEY record in canonical form, with the DSNKEY rdata appended.
 * @param   key a pointer to the DNSKEY record to be hashed.
 * @param   nbits   the number of bits to be used for the SHA hash (160, 256, or 512).
 * @param   outbuf  a pointer to the output buffer of size nbits bits to receive the DNSKEY hash value.
 * @return  -1 on general error or 0 on success.
 */
int _compute_dnskey_sha_hash(const dnskey_t *key, size_t nbits, unsigned char *outbuf) {

    unsigned char *hashbuf = NULL;
    size_t hlen = 0;
    int result;

    if (!key || !outbuf) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!_mem_append_canon(&hashbuf, &hlen, key->label)) {
        RET_ERROR_INT(ERR_NOMEM, "unable to derive canonical name of DNSKEY owner");
    }

    if (!_mem_append(&hashbuf, &hlen, (unsigned char *)key->rdata, key->rdlen)) {
        RET_ERROR_INT(ERR_NOMEM, NULL);
    }

    result = _compute_sha_hash(nbits, hashbuf, hlen, outbuf);
    free(hashbuf);

    if (result < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error computing SHA hash of DNSKEY data");
    }

    return result;
}


/**
 * @brief   Create and add a new RSA DNSKEY entry to the object cache.
 * @param   label       a pointer to a null-terminated string containing the name of the domain supplying the specified key.
 * @param   flags       a bitmask of flags corresponding to the DNSKEY's RR flags field.
 * @param   algorithm   the value of the DNSKEY's RR algorithm field.
 * @param   pubkey      a pointer to an RSA public key used to validate signatures made by this key.
 * @param   keytag      a numerical value linking this to other RRSIG or DS records.
 * @param   rdata       a pointer to the beginning of this DNSKEY's RR rdata.
 * @param   rdlen       the length, in bytes, of the DNSKEY's RR rdata.
 * @param   ttl     the TTL value indicated by the DNSKEY resource record.
 * @param   do_cache    if set, the new object cache entry for this DNSKEY can be persisted to the cache file.
 * @param   forced      if set, this entry will forcibly replace (overshadow) any clashing or matching existing DNSKEY entry
 *                              inside the object cache.
 * @return  NULL on failure, or a pointer to the new DNSKEY object cache entry on success.
 */
dnskey_t *_add_dnskey_entry_rsa(const char *label, uint16_t flags, unsigned char algorithm, RSA *pubkey, unsigned int keytag, const unsigned char *rdata,
                                size_t rdlen, unsigned long ttl, unsigned int do_cache, int forced) {

    // TODO: Some of these parameters are redundant.

    dnskey_t *dnskey;
    char *cacheid;
    size_t cidlen;
    cached_object_t *result;

    if (!label || !pubkey || !rdata) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    // The first check: the RFC states that if the zone bit is not set then the key cannot be used to verify RRSIGs over RRsets.
    if (!(flags & DNSKEY_RR_FLAG_ZK)) {
        RET_ERROR_PTR(ERR_UNSPEC, "DNSKEY without zone bit cannot be used to verify RRSIGs");
    }

    if (!(dnskey = malloc(sizeof(dnskey_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "unable to allocate space for DNSKEY entry");
    }

    memset(dnskey, 0, sizeof(dnskey_t));

    if (!(dnskey->rdata = malloc(rdlen))) {
        PUSH_ERROR_SYSCALL("malloc");
        _destroy_dnskey(dnskey);
        RET_ERROR_PTR(ERR_NOMEM, "unable to allocate space for DNSKEY rdata");
    }

    memcpy(dnskey->rdata, rdata, rdlen);
    dnskey->rdlen = rdlen;

    if (!(dnskey->label = strdup(label))) {
        PUSH_ERROR_SYSCALL("strdup");
        _destroy_dnskey(dnskey);
        RET_ERROR_PTR(ERR_NOMEM, "unable to allocate space for DNSKEY label");
    }

    dnskey->algorithm = algorithm;

    if (flags & DNSKEY_RR_FLAG_ZK) {
        dnskey->is_zone = 1;
    }

    if (flags & DNSKEY_RR_FLAG_SEP) {
        dnskey->is_sep = 1;
    }

    dnskey->pubkey = pubkey;
    dnskey->keytag = keytag;
    dnskey->do_cache = do_cache;

    // Our cached ID needs to be unique, so since we may have multiple keys cached from the same signer, the id must include the keytag.
    cidlen = strlen(label) + 32;

    if (!(cacheid = malloc(cidlen))) {
        PUSH_ERROR_SYSCALL("malloc");
        _destroy_dnskey(dnskey);
        RET_ERROR_PTR(ERR_NOMEM, "unable to add DNSKEY entry to object cache because of memory allocation error");
    }

    memset(cacheid, 0, cidlen);
    snprintf(cacheid, cidlen, "%s-%u", label, keytag);

    // Our newly created object doubles as the key for comparison checks.
    // By default we don't persist the key; we wait for it to be validated first.
    if (forced) {
        result = _add_cached_object_cmp_forced(cacheid, dnskey, &(cached_stores[cached_data_dnskey]), ttl, 0, dnskey, 0, 0, _dnskey_tag_comparator);
    } else {
        result = _add_cached_object_cmp(cacheid, dnskey, &(cached_stores[cached_data_dnskey]), ttl, 0, dnskey, 0, 0, _dnskey_tag_comparator);
    }

    if (!result) {
        _destroy_dnskey(dnskey);
        free(cacheid);
        RET_ERROR_PTR(ERR_UNSPEC, "could not add DNSKEY entry to object cache");
    }

    free(cacheid);

    return (_get_cache_obj_data(result));
}


/**
 * @brief   Create and add a new DS entry to the object cache.
 * @param   label       a pointer to a null-terminated string containing the name of the domain supplying the specified DS record.
 * @param   keytag      a numerical value identifying the DNSKEY entry linked to this record.
 * @param   algorithm   the value of the DS's RR algorithm field.
 * @param   digest_type the value of the DS's RR digest type field.
 * @param   digest      a pointer to a buffer containing the DS record's digest data.
 * @param   dlen        the size, in bytes, of the DS record's digest.
 * @param   ttl     the TTL value indicated by the DS resource record.
 * @return  NULL on failure, or a pointer to the new DS object cache entry on success.
 */
ds_t *_add_ds_entry(const char *label, unsigned int keytag, unsigned char algorithm, unsigned char digest_type, const unsigned char *digest, size_t dlen, unsigned long ttl) {

    cached_object_t *result;
    ds_t *ds;
    char *cacheid;
    size_t cidlen;

    if (!label || !digest || !dlen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(ds = malloc(sizeof(ds_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "unable to allocate space for DS record");
    }

    memset(ds, 0, sizeof(ds_t));

    if (!(ds->label = strdup(label))) {
        PUSH_ERROR_SYSCALL("strdup");
        _destroy_ds(ds);
        RET_ERROR_PTR(ERR_NOMEM, "unable to allocate space for DS label");
    }

    ds->keytag = keytag;
    ds->algorithm = algorithm;
    ds->digest_type = digest_type;
    ds->diglen = dlen;

    if (!(ds->digest = malloc(dlen))) {
        PUSH_ERROR_SYSCALL("malloc");
        _destroy_ds(ds);
        RET_ERROR_PTR(ERR_NOMEM, "unable to allocate space for DS digest");
    }

    memcpy(ds->digest, digest, dlen);

    // Our cached ID needs to be unique, so since we may have multiple DS records cached from the same signer, the id must include a couple of things.
    cidlen = strlen(label) + 64;

    if (!(cacheid = malloc(cidlen))) {
        PUSH_ERROR_SYSCALL("malloc");
        _destroy_ds(ds);
        RET_ERROR_PTR(ERR_NOMEM, "unable to add DS entry to object cache because of memory allocation error");
    }

    memset(cacheid, 0, cidlen);
    snprintf(cacheid, cidlen, "%s-%u-%u-%u", label, keytag, algorithm, digest_type);

    // Our newly created object doubles as the comparator key.
    if (!(result = _add_cached_object_cmp(cacheid, ds, &(cached_stores[cached_data_ds]), ttl, 0, ds, 1, 0, _ds_comparator))) {
        _destroy_ds(ds);
        free(cacheid);
        RET_ERROR_PTR(ERR_UNSPEC, "could not add DS entry to object cache");
    }

    free(cacheid);

    _fixup_ds_links();

    return (_get_cache_obj_data(result));
}


/**
 * @brief   Find a DNSKEY entry by its keytag value.
 * @param   tag     the semi-unique keytag value identifying the target DNSKEY entry.
 * @param   signer      a null-terminated string containing the name of the signing domain that owns the DNSKEY entry.
 * @param   force_lookup    if set, perform a live-lookup of any DNSKEY entry that could not be located by keytag;
 *                              otherwise, lookups will be restricted only to the object cache.
 * @return  a pointer to the specified DNSKEY record if it was found, or NULL on error or if it wasn't.
 */
dnskey_t *_get_dnskey_by_tag(unsigned int tag, const char *signer, int force_lookup) {

    dnskey_t cmp;
    cached_object_t *obj;

    if (!signer) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    memset(&cmp, 0, sizeof(cmp));
    cmp.label = (char *)signer; /* won't be deallocated */
    cmp.keytag = tag;

    if ((obj = _find_cached_object_cmp(&cmp, &(cached_stores[cached_data_dnskey]), &_dnskey_tag_comparator))) {
        return ((dnskey_t *)_get_cache_obj_data(obj));
    } else {
        _clear_error_stack();
    }

    if (force_lookup) {

        if (IS_ROOT_LABEL(signer)) {
            _dbgprint(2, "Stopped lookup chain; hit root.\n");
        } else {
            _dbgprint(1, "Could not find key for signer [%s]... looking it up.\n", signer);
            _lookup_dnskey(signer);
            _lookup_ds(signer);
            // Preemptive cleanup in case the error stack is set by _lookup_dnskey() or _lookup_ds().
            _clear_error_stack();
            _dbgprint(2, "Returned from forced lookup.\n");
            return _get_dnskey_by_tag(tag, signer, 0);
        }

    }

    return NULL;
}


/**
 * @brief   Look up a DS record by its matching DNSKEY record.
 * @param   key a pointer to the DNSKEY record that will be used to find the matching DS record.
 * @return  a pointer to the matching DS record on success or NULL on failure.
 */
ds_t *_get_ds_by_dnskey(const dnskey_t *key) {

    ds_t cmp;
    cached_object_t *obj;
    unsigned char hashbuf[64];

    if (!key) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    memset(&cmp, 0, sizeof(cmp));
    cmp.label = key->label;
    cmp.algorithm = key->algorithm;
    cmp.keytag = key->keytag;

    // Set up the hash information for the DS entry.
    memset(&hashbuf, 0, sizeof(hashbuf));
    cmp.digest = hashbuf;

    if (key->algorithm == NS_ALG_RSASHA1) {
        cmp.digest_type = DNSSEC_DIGEST_SHA1;
        cmp.diglen = SHA_160_SIZE;
    } else if (key->algorithm == NS_ALG_RSASHA256) {
        cmp.digest_type = DNSSEC_DIGEST_SHA256;
        cmp.diglen = SHA_256_SIZE;
    } else {
        RET_ERROR_PTR_FMT(ERR_UNSPEC, "could not map DNSKEY to DS record of unsupported digest type {alg = %u}", key->algorithm);
    }

    if (_compute_dnskey_sha_hash(key, cmp.diglen * 8, hashbuf) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "unable to compute SHA hash of DNSKEY");
    }

    if ((obj = _find_cached_object_cmp(&cmp, &(cached_stores[cached_data_ds]), &_ds_comparator))) {
        return ((ds_t *)_get_cache_obj_data(obj));
    } else {
        _clear_error_stack();
    }

    return NULL;
}


/**
 * @brief   An object cache callback for comparing two DNSKEY records based on tag value.
 * @note    This function also checks to make sure that the domain labels match as well.
 * @param   object  a pointer to the DNSKEY cache entry to be compared against the key.
 * @param   key a pointer to a key value to be compared against the object.
 * @return  -1 if a < b, 1 if b > a, or 0 if the records were equivalent.
 */
int _dnskey_tag_comparator(const void *object, const void *key) {

    dnskey_t *a = (dnskey_t *)object, *b = (dnskey_t *)key;
    int result;

    if (!a) {
        return -1;
    } else if (!b) {
        return -1;
    }

    if (!a->label) {
        return -1;
    } else if (!b->label) {
        return 1;
    }

    // Although we're really searching by keytag, the domain labels still need to match.
    if ((result = strcasecmp(a->label, b->label))) {
        return result;
    }

    if (a->keytag == b->keytag) {
        return 0;
    }

    return ((a->keytag < b->keytag) ? -1 : 1);
}


/**
 * @brief   An object cache callback for comparing two DNSKEY records based on domain label.
 * @return  -1 if a < b, 1 if b > a, or 0 if the records were equivalent.
 */
int _dnskey_domain_comparator(const void *object, const void *key) {

    dnskey_t *a = (dnskey_t *)object, *b = (dnskey_t *)key;

    if (!a) {
        return -1;
    } else if (!b) {
        return -1;
    }

    if (!a->label) {
        return -1;
    } else if (!b->label) {
        return 1;
    }

    return (strcasecmp(a->label, b->label));
}



/**
 * @brief   An object cache callback for comparing two DS records for general equivalence.
 * @note    This function is used to map a DNSKEY record to a DS record.
 */
int _ds_comparator(const void *object, const void *key) {

    ds_t *a = (ds_t *)object, *b = (ds_t *)key;
    int result;

    // First make sure the labels match.
    if (!a) {
        return -1;
    } else if (!b) {
        return -1;
    }

    if (!a->label) {
        return -1;
    } else if (!b->label) {
        return 1;
    }

    // Although we're really searching by keytag, the domain labels still need to match.
    if ((result = strcmp(a->label, b->label))) {
        return result;
    }

    if (a->keytag < b->keytag) {
        return -1;
    } else if (a->keytag > b->keytag) {
        return 1;
    }

    if (!a->digest) {
        return -1;
    } else if (!b->digest) {
        return 1;
    }

    if (a->diglen < b->diglen) {
        return -1;
    } else if (a->diglen > b->diglen) {
        return 1;
    }

    return (memcmp(a->digest, b->digest, a->diglen));
}



/**
 * @brief   Free a DNSKEY object and its underlying data.
 * @param   key a pointer to the DNSKEY object to be destroyed.
 */
void _destroy_dnskey(dnskey_t *key) {

    if (!key) {
        return;
    }

    if (key->label) {
        free(key->label);
    }

    if (key->pubkey) {
        RSA_free_d(key->pubkey);
    }

    if (key->rdata) {
        free(key->rdata);
    }

    // Don't use _ptr_chain_free() because these chains are shallow copies.
    if (key->signkeys) {
        free(key->signkeys);
    }

    if (key->dse) {
        free(key->dse);
    }

    memset(key, 0, sizeof (dnskey_t));
    free(key);

}


/**
 * @brief   Free a DS object and its underlying data.
 * @param   ds  a pointer to the DS object to be destroyed.
 */
void _destroy_ds(ds_t *ds) {

    if (!ds) {
        return;
    }

    if (ds->label) {
        free(ds->label);
    }

    if (ds->digest) {
        free(ds->digest);
    }

    // Don't use _ptr_chain_free() because this chain is a shallow copy.
    if (ds->signkeys) {
        free(ds->signkeys);
    }

    memset(ds, 0, sizeof(ds_t));
    free(ds);

}


/**
 * @brief   Create and add a new DNSKEY entry to the object cache.
 * @param   label   a pointer to a null-terminated string containing the name of the domain supplying the specified key.
 * @param   buf a pointer to the beginning of the DNSKEY's RR rdata.
 * @param   len the length, in bytes, of the DNSKEY RR's rdata.
 * @param   ttl the TTL value indicated by the DNSKEY resource record.
 */
dnskey_t *_add_dnskey_entry(const char *label, const unsigned char *buf, size_t len, unsigned long ttl) {

    dnskey_rr_flags_t *rrflags;
    RSA *pubkey;
    dnskey_t *result;
    const unsigned char *bptr = buf;
    unsigned int keytag;
    uint16_t *flags;
    uint8_t proto, alg;

    if (!label || !buf) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    } else if (len <= 4) {
        RET_ERROR_PTR(ERR_BAD_PARAM, "DNSKEY RR length was too short");
    }

    if (!(keytag = _get_keytag(buf, len))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not get tag value for DNSKEY");
    }

    rrflags = (dnskey_rr_flags_t *)buf;
    flags = (uint16_t *)rrflags;
    bptr += 2;
    proto = *bptr++;
    alg = *bptr++;

    _dbgprint(3, "  keytag = %u, flags = %x, proto = %d, alg = %d, rdlen = %u\n", keytag, *flags, proto, alg, len);

    if (proto != DNSKEY_RR_PROTO) {
        RET_ERROR_PTR_FMT(ERR_UNSPEC, "DNSKEY RR contained bad protocol: %u", proto);
    }

    // The only one we support right now.
    if (alg != NS_ALG_RSASHA1 && alg != NS_ALG_RSASHA256 && alg != NS_ALG_RSASHA512) {
        RET_ERROR_PTR_FMT(ERR_UNSPEC, "DNSKEY RR contained unsupported algorithm: %u", alg);
    }

    if (!rrflags->zone_key) {
        RET_ERROR_PTR(ERR_UNSPEC, "DNSKEY RR was not a zone key");
    }

    _dbgprint(3, "  flags reserved1 = %u, zk = %u, reserved2 = %u, sep = %u\n", rrflags->reserved1, rrflags->zone_key, rrflags->reserved2, rrflags->sep);

    if (!(pubkey = _get_rsa_dnskey(bptr, len - 4))) {
        RET_ERROR_PTR(ERR_UNSPEC, "unable to extract public key from DNSKEY RR");
    }

    // TODO: Should this really be called from here?
    if (!(result = _add_dnskey_entry_rsa(label, ntohs(*flags), alg, pubkey, keytag, buf, len, ttl, 1, 0))) {
        RET_ERROR_PTR(ERR_UNSPEC, "unable to add DNSKEY entry to cache");
    }

    _fixup_dnskey_validation();

    return result;
}


// TODO: The callers of this function also need to establish signing for multiple records in an RRSET. Right now they are called in a way so that only the most recent record will be signed.
/**
 * @brief   Verify the signature provided by an RRSIG record.
 * @see     rsa_verify_record()
 * @return  1 if the RRSIG signature was correct for the RR data, 0 if it was not, or -1 on general error.
 */
int _validate_rrsig_rr(const char *label, ns_msg *dhandle, unsigned short covered, const unsigned char *rdata, size_t rdlen, dnskey_t **outkey) {

    rrsig_rr_t *rrsig;
    dnskey_t *signing_key;
    const unsigned char *strptr = rdata;
    char nbuf[MAXDNAME];
    char *inception_timestr, *expiration_timestr, *now_timestr;
    time_t tnow;
    size_t rdleft = rdlen;
    int lsignlen, result;

    if (!label || !dhandle || !rdata) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    } else if (rdlen < sizeof(rrsig_rr_t)) {
        RET_ERROR_INT(ERR_BAD_PARAM, "RRSIG did not meet minimum required length");
    }

    rdleft -= sizeof(rrsig_rr_t) - 1;
    strptr += sizeof(rrsig_rr_t) - 1;

    rrsig = (rrsig_rr_t *)rdata;

    if (rrsig->covered != htons(covered)) {
        RET_ERROR_INT_FMT(ERR_UNSPEC, "RRSIG record did not cover correct right resource type {got %u, expected %u}", ntohs(rrsig->covered), covered);
    } else if (rrsig->algorithm != NS_ALG_RSASHA1 && rrsig->algorithm != NS_ALG_RSASHA256 && rrsig->algorithm != NS_ALG_RSASHA512) {
        RET_ERROR_INT_FMT(ERR_UNSPEC, "RRSIG record was not encoded with RSASHA1 (%u)", rrsig->algorithm);
    }

    inception_timestr = _get_chr_date(ntohl(rrsig->inception), 1);
    expiration_timestr = _get_chr_date(ntohl(rrsig->expiration), 1);
    _dbgprint(3, "  algorithm: %u, covered: %u, labels: %u, Original ttl: %d, key tag: %d, expiration: %s, inception: %s\n",
              rrsig->algorithm, ntohs(rrsig->covered), rrsig->labels, ntohl(rrsig->ottl), ntohs(rrsig->key_tag),
              expiration_timestr ? expiration_timestr : "[error]", inception_timestr ? inception_timestr : "[error]");
    free(inception_timestr);
    free(expiration_timestr);

    if (time(&tnow) == ((time_t)-1)) {
        PUSH_ERROR_SYSCALL("time");
        RET_ERROR_INT(ERR_UNSPEC, "unable to get current time");
    }

    now_timestr = _get_chr_date(tnow, 1);
    _dbgprint(5, "  NOW: %s\n", now_timestr ? now_timestr : "[error]");
    free(now_timestr);

    if (tnow < ntohl(rrsig->inception)) {
        RET_ERROR_INT(ERR_UNSPEC, "current time is before RR inception time");
    } else if (tnow > ntohl(rrsig->expiration)) {
        RET_ERROR_INT(ERR_UNSPEC, "current time is after RR expiration time");
    }

    memset(nbuf, 0, sizeof(nbuf));

    if ((lsignlen = ns_name_uncompress(ns_msg_base(*dhandle), (unsigned char *)ns_msg_end(*dhandle) + 1, (unsigned char *)&(rrsig->signame), nbuf, sizeof(nbuf))) == -1) {
        PUSH_ERROR_RESOLVER("ns_name_uncompress");
        RET_ERROR_INT(ERR_UNSPEC, "signer name field was invalid");
    }

    // Right now, strptr and rdleft are updated to point to the end of the key tag.
    // The only fields left in the rdata are the signer's name and signature.
    if ((size_t)lsignlen >= rdleft) {
        RET_ERROR_INT(ERR_UNSPEC, "error parsing signer name label");
    }

    strptr += lsignlen;
    rdleft -= lsignlen;

    _dbgprint(3, "  Signer name: [%s], len = %u, nbytes = %u\n", nbuf, lsignlen, rdleft);

    if (_verbose >= 4) {
        _dbgprint(4, "Signature buffer:\n");
        _dump_buf_outer((unsigned char *)strptr, rdleft, 5, 1);
    }

    if (!(signing_key = _get_dnskey_by_tag(ntohs(rrsig->key_tag), nbuf, 1))) {
        RET_ERROR_INT_FMT(ERR_UNSPEC, "could not locate signing key %u for signing name: %s", ntohs(rrsig->key_tag), &(rrsig->signame));
    }

    if (outkey) {
        *outkey = signing_key;
    }

    if ((result = _rsa_verify_record(label, rrsig->algorithm, signing_key->pubkey, rdata, strptr, rdleft, dhandle)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, NULL);
    }

    return result;
}

/**
 * @brief   Dump the contents of a DNS reply header.
 * @param   handle  a pointer to the DNS reply to have its header dumped to the console.
 */
void _dump_dns_header(ns_msg *handle) {

    _dbgprint(3, "DNS reply header[1] - msg id: %.2x, qr flag: %u, opcode: %u, rcode: %u, aa: %u, ad: %u\n",
              ns_msg_id(*handle), ns_msg_getflag(*handle, ns_f_qr),   ns_msg_getflag(*handle, ns_f_opcode),
              ns_msg_getflag(*handle, ns_f_rcode), ns_msg_getflag(*handle, ns_f_aa), ns_msg_getflag(*handle, ns_f_ad));
    _dbgprint(3, "DNS reply header[2] - q count: %u, answer count: %u, additional count: %u, ns count: %u\n",
              ns_msg_count(*handle, ns_s_qd), ns_msg_count(*handle, ns_s_an), ns_msg_count(*handle, ns_s_ar), ns_msg_count(*handle, ns_s_ns));

}


// TODO: needs lots of cleanup. Needs to return values, for one.
void *_lookup_dnskey(const char *label) {

    ns_msg handle;
    ns_rr rr;
    dnskey_t *dnskey, *skey, **dptr, **allkeys = NULL;
    unsigned char resbuf[4096];
    int nread;
    uint16_t nanswers, rrtype;

    if ((nread = res_query(label, ns_c_in, T_DNSKEY, resbuf, sizeof(resbuf))) < 0) {
        PUSH_ERROR_RESOLVER("res_query");
        RET_ERROR_PTR(ERR_UNSPEC, "error occurred in sending DNSKEY record query");
    }

    if (ns_initparse(resbuf, nread, &handle) < 0) {
        PUSH_ERROR_RESOLVER("ns_initparse");
        RET_ERROR_PTR(ERR_UNSPEC, "error in parsing DNSKEY request answer");
    }

    nanswers = ns_msg_count(handle, ns_s_an);
    _dump_dns_header(&handle);

    // During the first pass we pick out all the DNSKEYs.
    for(size_t i = 0; i < nanswers; i++) {

        if (ns_parserr(&handle, ns_s_an, i, &rr) < 0) {
            PUSH_ERROR_RESOLVER("ns_parserr");
            RET_ERROR_PTR(ERR_UNSPEC, "error in parsing DNSKEY request answers [1]");
        }

        if (ns_rr_class(rr) != ns_c_in) {
            _dbgprint(3, "Skipped over answer with class != IN ...\n");
            continue;
        }

        rrtype = ns_rr_type(rr);

        if (rrtype != T_DNSKEY && rrtype != T_RRSIG) {
            _dbgprint(2, "Skipped over answer with type != DNSKEY: %u ...\n", ns_rr_type(rr));
            continue;
        } else if (rrtype != T_DNSKEY) {
            continue;
        }

        _dbgprint(2, "Parsing DNSKEY lookup request answer #%u/%u - rr name: [%s], type %u, class %u, ttl %u, rdlen %u\n",
                  i + 1, nanswers, ns_rr_name(rr), ns_rr_type(rr), ns_rr_class(rr), ns_rr_ttl(rr), ns_rr_rdlen(rr));
        //_dump_buf((char *)ns_rr_rdata(rr), ns_rr_rdlen(rr));

        if ((dnskey = _add_dnskey_entry(ns_rr_name(rr), ns_rr_rdata(rr), ns_rr_rdlen(rr), ns_rr_ttl(rr)))) {

            if (!(allkeys = _ptr_chain_add(allkeys, dnskey))) {
                RET_ERROR_PTR(ERR_UNSPEC, "could not add DNSKEY RR to chain");
            }
        } else {
            fprintf(stderr, "Error adding DNSKEY entry for RRSIG verification.\n");
        }

    }

    // For the second pass, we see if the RRSIGs over the DNSKEYs check out.
    for(size_t i = 0; i < nanswers; i++) {

        if (ns_parserr(&handle, ns_s_an, i, &rr) < 0) {
            PUSH_ERROR_RESOLVER("ns_parserr");

            if (allkeys) {
                free(allkeys);
            }

            RET_ERROR_PTR(ERR_UNSPEC, "error in parsing DNSKEY request answers [2]");
        }

        if ((ns_rr_class(rr) != ns_c_in) || (ns_rr_type(rr) != T_RRSIG)) {
            continue;
        }


        if (allkeys) {
            // The validation process works over the entire set of records.
            if (_validate_rrsig_rr(ns_rr_name(rr), &handle, T_DNSKEY, ns_rr_rdata(rr), ns_rr_rdlen(rr), &skey) < 0) {
                fprintf(stderr, "Error: could not validate RRSIG over DNSKEY record:\n");
                dump_error_stack();
                _clear_error_stack();
            }

            // So once we get the first one it's just a matter of copying it over to the rest.
            for (dptr = allkeys; *dptr; dptr++) {
                // We ignore this potential error. Should we?
                if (!((*dptr)->signkeys = _ptr_chain_add((*dptr)->signkeys, skey))) {
                    fprintf(stderr, "Error: could not add signing key to pointer chain.\n");
                    dump_error_stack();
                    _clear_error_stack();
                }

            }

        }

    }

    if (allkeys) {
        free(allkeys);
    }

    // TODO: this needs to be changed
    return NULL;
}


/**
 * @brief   Lookup the DS record for a specified domain.
 * @param   label   a null-terminated string containing the name of the domain to have its DS record queried.
 */
void *_lookup_ds(const char *label) {

    ns_msg handle;
    ns_rr rr;
    ds_rr_t *dsr;
    dnskey_t *dnskey, *skey;
    ds_t *ds, **dsptr, **allds = NULL;
    unsigned char resbuf[4096], hashbuf[64];
    int nread;
    uint16_t nanswers, rrtype;
    size_t hsize;

    if (!label) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    _dbgprint(1, "Looking up DS record for [%s]\n", label);

    if ((nread = res_query(label, ns_c_in, T_DS, resbuf, sizeof(resbuf))) < 0) {
        PUSH_ERROR_RESOLVER("res_query");
        RET_ERROR_PTR(ERR_UNSPEC, "error occurred in sending DS record query");
    }

    if (ns_initparse(resbuf, nread, &handle) < 0) {
        PUSH_ERROR_RESOLVER("ns_initparse");
        RET_ERROR_PTR(ERR_UNSPEC, "error in parsing DS request answer");
    }

    nanswers = ns_msg_count(handle, ns_s_an);
    _dump_dns_header(&handle);

    // During the first pass we pick out all the DS records.
    for(size_t i = 0; i < nanswers; i++) {

        if (ns_parserr(&handle, ns_s_an, i, &rr) < 0) {
            PUSH_ERROR_RESOLVER("ns_parserr");
            RET_ERROR_PTR(ERR_UNSPEC, "error in parsing DS request answers [1]");
        }

        if (ns_rr_class(rr) != ns_c_in) {
            _dbgprint(3, "Skipped over answer with class != IN ...\n");
            continue;
        }

        rrtype = ns_rr_type(rr);

        if (rrtype != T_DS && rrtype != T_RRSIG) {
            _dbgprint(3, "Skipped over answer with type != DS: %u ...\n", ns_rr_type(rr));
            continue;
        } else if (rrtype != T_DS) {
            continue;
        }

        _dbgprint(2, "Parsing DS lookup request answer #%u/%u: rr name: [%s], type %u, class %u, ttl %u, rdlen %u\n",
                  (unsigned int)i + 1, nanswers, ns_rr_name(rr), ns_rr_type(rr), ns_rr_class(rr), ns_rr_ttl(rr), ns_rr_rdlen(rr));
        //_dump_buf((char *)ns_rr_rdata(rr), ns_rr_rdlen(rr));
        //

        if (ns_rr_rdlen(rr) < sizeof(ds_rr_t)) {
            _dbgprint(2, "Skipped over DS record: rdata failed to meet minimum length requirement.\n");
            continue;
        }

        dsr = (ds_rr_t *)ns_rr_rdata(rr);

        if (dsr->algorithm != NS_ALG_RSASHA1 && dsr->algorithm != NS_ALG_RSASHA256 && dsr->algorithm != NS_ALG_RSASHA512) {
            _dbgprint(2, "Skipped over DS record; encountered unsupported algorithm: %u\n", dsr->algorithm);
            continue;
        }

        if (dsr->digest_type == DNSSEC_DIGEST_SHA1) {
            hsize = 160;
        } else if (dsr->digest_type == DNSSEC_DIGEST_SHA256) {
            hsize = 256;
        } else {
            _dbgprint(2, "Skipped over DS record; encountered unsupported digest type: %u\n", dsr->digest_type);
            continue;
        }

        // Make sure that the digest length (at rdata+4) is exactly as long as it should be
        if (ns_rr_rdlen(rr) - 4U != hsize / 8) {
            _dbgprint(1, "Error: DS record contained a digest of unexpected length.\n");
            continue;
        }

        _dbgprint(3, "DS key_tag = %u, algorithm: %u, digest type: %u, rdlen: %u, remainder: %u\n",
                  ntohs(dsr->key_tag), dsr->algorithm, dsr->digest_type, ns_rr_rdlen(rr), ns_rr_rdlen(rr) - 4);

        if (!(ds = _add_ds_entry(ns_rr_name(rr), ntohs(dsr->key_tag), dsr->algorithm, dsr->digest_type, ns_rr_rdata(rr) + 4, (ns_rr_rdlen(rr) - 4), ns_rr_ttl(rr)))) {
            fprintf(stderr, "Error: Unable to generate DS entry for record.\n");
            dump_error_stack();
            continue;
        }

        if (!(allds = _ptr_chain_add(allds, ds))) {
            RET_ERROR_PTR(ERR_UNSPEC, "could not add DS RR to chain");
        }

        // See if there is a DNSKEY entry that this DS record validates.
        if ((dnskey = _get_dnskey_by_tag(ntohs(dsr->key_tag), ns_rr_name(rr), 0))) {
            _compute_dnskey_sha_hash(dnskey, hsize, hashbuf);

            // Compare the hashed DNSKEY against the digest field that's 4 bytes into the DS RR.
            if (memcmp(hashbuf, ns_rr_rdata(rr) + 4, hsize / 8)) {
                fprintf(stderr, "Error: DNSKEY hash provided by this DS record did not match up!\n");
            } else {
                // Link this DS record to the DNSKEY record that it covers.
                // Note that each DNSKEY record can be covered by multiple DS records.
                if (!(dnskey->dse = _ptr_chain_add(dnskey->dse, ds))) {
                    fprintf(stderr, "Error: could not link DS records to DNSKEY entry.\n");
                }

            }

        } else {
            fprintf(stderr, "XXXXXXXXXXXXX: we need to handle this\n");
        }

    }

    // For the second pass, we see if the RRSIGs over the DS check out.
    for(size_t i = 0; i < nanswers; i++) {

        if (ns_parserr(&handle, ns_s_an, i, &rr) < 0) {
            PUSH_ERROR_RESOLVER("ns_parserr");

            if (allds) {
                free(allds);
            }

            RET_ERROR_PTR(ERR_UNSPEC, "error in parsing DS request answers [2]");
        }

        if ((ns_rr_class(rr) != ns_c_in) || (ns_rr_type(rr) != T_RRSIG)) {
            continue;
        }


        if (allds) {
            // The validation process works over the entire set of records.
            if (_validate_rrsig_rr(ns_rr_name(rr), &handle, T_DS, ns_rr_rdata(rr), ns_rr_rdlen(rr), &skey) < 0) {
                fprintf(stderr, "Error: could not validate RRSIG over DS record:\n");
                dump_error_stack();
                _clear_error_stack();
            }

            // So once we get the first one it's just a matter of copying it over to the rest.
            for (dsptr = allds; *dsptr; dsptr++) {
                // We ignore this error. Should we?
                if (!((*dsptr)->signkeys = _ptr_chain_add((*dsptr)->signkeys, skey))) {
                    fprintf(stderr, "Error: could not add signing key to pointer chain.\n");
                    dump_error_stack();
                    _clear_error_stack();
                }

            }

        }

    }

    if (allds) {
        free(allds);
    }

    // TODO: This needs to change.
    return NULL;
}


/**
 * @brief   Get the answer to a DNS TXT record query.
 * @param   qstring     a pointer to a null-terminated string containing the DNS query string.
 * @param   ttl     if not NULL, an optional pointer to a value that will store the TTL of the retrieved record on success.
 * @param   validated
 */
char *_get_txt_record(const char *qstring, unsigned long *ttl, int *validated) {

    ns_msg handle;
    ns_rr rr;
    dnskey_t *signing_key;
    const unsigned char *strptr;
    unsigned char resbuf[4096];
    const char *lname;
    char *result = NULL;
    int nread, vval, dnssec_rrs = 0;
    size_t nanswers, nadditional, strsize, rdleft, rsize = 0;
    uint16_t rrtype, z;
    uint8_t ercode, version;

    if (!qstring) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    INITIALIZE_DNS();

    // The default state, unless there's an error or a validation failure, is unvalidated.
    if (validated) {
        *validated = 0;
    }

    if ((nread = res_query(qstring, ns_c_in, ns_t_txt, resbuf, sizeof(resbuf))) < 0) {
        PUSH_ERROR_RESOLVER("res_query");
        RET_ERROR_PTR(ERR_UNSPEC, "unable to send TXT record query");
    }

    if (ns_initparse(resbuf, nread, &handle) < 0) {
        PUSH_ERROR_RESOLVER("ns_initparse");
        RET_ERROR_PTR(ERR_UNSPEC, "unable to parse TXT record answer");
    }

    // This should probably never happen, but it obviously MUST be a response.
    if (!ns_msg_getflag(handle, ns_f_qr)) {
        RET_ERROR_PTR(ERR_UNSPEC, "received a query instead of a DNS response");
    }
    // Nor is this likely
    else if (ns_msg_getflag(handle, ns_f_opcode)) {
        RET_ERROR_PTR(ERR_UNSPEC, "DNS response contained unexpected opcode");
    } else if (ns_msg_getflag(handle, ns_f_rcode)) {
        RET_ERROR_PTR_FMT(ERR_UNSPEC, "DNS response rcode indicates an error: %u", ns_msg_getflag(handle, ns_f_rcode));
    }

    nanswers = ns_msg_count(handle, ns_s_an);
    nadditional = ns_msg_count(handle, ns_s_ar);
    _dump_dns_header(&handle);

    if (!nanswers) {
        RET_ERROR_PTR(ERR_UNSPEC, "DNS response contained no answers");
    } else if (!nadditional) {
        RET_ERROR_PTR(ERR_UNSPEC, "DNS response contained no additional RRs");
    }

    for(size_t i = 0; i < nanswers; i++) {

        if (ns_parserr(&handle, ns_s_an, i, &rr) < 0) {
            PUSH_ERROR_RESOLVER("ns_parserr");

            if(result) {
                free(result);
            }

            RET_ERROR_PTR(ERR_UNSPEC, "error occurred parsing TXT record answer");
        }

        if (ns_rr_class(rr) != ns_c_in) {
            _dbgprint(3, "Skipped over answer with class != IN ...\n");
            continue;
        }

        rrtype = ns_rr_type(rr);

        if ((rrtype != ns_t_txt) && (rrtype != T_RRSIG)) {
            _dbgprint(3, "Skipped over answer with type != TXT or RRSIG ...\n");
            continue;
        }

        _dbgprint(2, "Parsing TXT query answer #%u/%u - rr name: [%s], type %u, class %u, ttl %u, rdlen %u\n", i + 1, nanswers, ns_rr_name(rr), ns_rr_type(rr), ns_rr_class(rr), ns_rr_ttl(rr), ns_rr_rdlen(rr));

        rdleft = ns_rr_rdlen(rr);
        strptr = ns_rr_rdata(rr);

        // TODO: make sure that the RRSIG covers the actual TXT record.
        // TODO: make sure the TXT record response is for the exact requested record (_dx.domain.com)
        if (rrtype == T_RRSIG) {
            // Either a validation failure or a general failure shoudl be considered a flat-out dnssec validation failure.
            vval = _validate_rrsig_rr(ns_rr_name(rr), &handle, ns_t_txt, strptr, rdleft, &signing_key);

            if (vval <= 0) {
                *validated = -1;

                if (vval < 0) {
                    fprintf(stderr, "Error: could not validate RRSIG over TXT record.\n");
                    dump_error_stack();
                    _clear_error_stack();
                }

            } else {
                // There's a chance that our key was validated but the ultimate chain of custody still can't be completed.
                if (_is_validated_key(signing_key)) {
                    *validated = 1;
                } else {
                    *validated = -1;
                }

                _fixup_dnskey_validation();
            }

        } else if (rrtype == ns_t_txt) {

            free(result);
            if (!(result = malloc(rdleft + 1))) {
                PUSH_ERROR_SYSCALL("malloc");
                RET_ERROR_PTR(ERR_NOMEM, "unable to allocate space for TXT record answer");
            }

            memset(result, 0, rdleft + 1);

            while (rdleft) {
                strsize = *((unsigned char *)strptr++);
                rdleft--;

                if (strsize > rdleft) {
                    _dbgprint(2, "Encountered premature end of data while reading TXT record rdata.\n");
                    continue;
                }

                //printf("strsize = %u\n", strsize);
                if (_verbose >= 2) {
                    _dbgprint(2, "TXT record answer:\n");
                    _dump_buf(strptr, strsize, 0);
                }

                memcpy(&(result[rsize]), strptr, strsize);
                rsize += strsize;

                //_parse_dime_record(strptr, strsize);
                strptr += strsize;
                rdleft -= strsize;

                if (ttl) {
                    *ttl = ns_rr_ttl(rr);
                }

            }

        }

    }

    for(size_t i = 0; i < nadditional; i++) {
        _dbgprint(4, "Parsing additional RR #%u/%u ...\n", (unsigned int)i + 1, nadditional);

        if (ns_parserr(&handle, ns_s_ar, i, &rr) < 0) {
            PUSH_ERROR_RESOLVER("ns_parserr");

            if(result) {
                free(result);
            }

            RET_ERROR_PTR(ERR_UNSPEC, "error occurred parsing TXT record answer");
        }

        if (ns_rr_type(rr) != ns_t_opt) {
            _dbgprint(4, "Skipped over additional record with type != OPT ...\n");
            continue;
        }

        // Get the corresponding EDNS0 data fields.
        lname = ns_rr_name(rr);
        ercode = (ns_rr_ttl(rr) & 0xff000000) >> 24;
        version = (ns_rr_ttl(rr) & 0x00ff0000) >> 16;
        z = ns_rr_ttl(rr) & 0x0000ffff;

        _dbgprint(4, "  ercode = %x, version = %x, z = %x\n", ercode, version, z);

        if (!IS_ROOT_LABEL(lname)) {
            _dbgprint(4, "Skipped over OPT record with unexpected non-empty root name.\n");
            continue;
        }

        _dbgprint(4, "  len = %d, orig ttl = %.8lx, ercode = %.8lx\n", (int)strlen(ns_rr_name(rr)), (long unsigned int)ns_rr_ttl(rr), (long unsigned int)ercode);

        if (!(z & NS_OPT_DNSSEC_OK)) {
            _dbgprint(4, "Skipped over OPT record missing DNSSEC OK flag.\n");
            continue;
        }

        _dbgprint(4, "  rr name: [%s], type %u, udp payload size %u, ttl %u, rdlen %u\n", ns_rr_name(rr), ns_rr_type(rr), ns_rr_class(rr), ns_rr_ttl(rr), ns_rr_rdlen(rr));
        rdleft = ns_rr_rdlen(rr);
        strptr = ns_rr_rdata(rr);

        while (rdleft) {
            strsize = *((unsigned char *)strptr++);
            rdleft--;

            if (strsize > rdleft) {
                _dbgprint(4, "Encountered premature end of data while reading TXT record rdata.\n");
                continue;
            }

            _dbgprint(4, "strsize = %u\n", (unsigned int)strsize);

            if (_verbose >= 3) {
                _dump_buf(strptr, strsize, 0);
            }

            strptr += strsize;
            rdleft -= strsize;
        }

        dnssec_rrs++;
    }

    // If we get an RRSIG answer but the packet isn't marked as +dnssec, then we should discard the validation.
    if (!dnssec_rrs) {
        _dbgprint(1, "Error: Received RRSIG response but no DNSSEC flag in response. Discarding DNSSEC validation.\n");

        if (validated) {
            *validated = 0;
        }

    }

    if (!result) {
        _dbgprint(0, "Error: Did not find requested TXT record.\n");
    }

    return result;
}


/**
 * @brief   Free a collection of MX records returned by _get_mx_records().
 * @param   mxs the array of MX records to be freed.
 */
void _free_mx_records(mx_record_t **mxs) {

    mx_record_t **ptr = mxs;

    while (*ptr) {
        free((*ptr)->name);
        free(*ptr);
        ptr++;
    }

    free(mxs);

}


/**
 * @brief   Retrieve the collection of MX records for a given domain.
 * @param   qstring     a null-terminated string containing the domain name to be queried via DNS.
 * @return  NULL on failure, or a pointer to a null-entry terminated array of mx_record pointers describing the domain on success.
 */
mx_record_t **_get_mx_records(const char *qstring) {

    ns_msg handle;
    ns_rr rr;
    mx_record_t **result = NULL, **rptr, *rentry, *oentry;
    unsigned char resbuf[4096];
    char nbuf[MAXDNAME], *strptr;
    int nread;
    size_t nanswers, rsize = 0;
    uint16_t rrtype, pref;

    if (!qstring) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    INITIALIZE_DNS();

    if ((nread = res_query(qstring, ns_c_in, ns_t_mx, resbuf, sizeof(resbuf))) < 0) {
        PUSH_ERROR_RESOLVER("res_query");
        RET_ERROR_PTR(ERR_UNSPEC, "error occurred in sending MX record query");
    }

    if (ns_initparse(resbuf, nread, &handle) < 0) {
        PUSH_ERROR_RESOLVER("ns_initparse");
        RET_ERROR_PTR(ERR_UNSPEC, "error in parsing MX request answer");
    }

    // This should probably never happen, but it obviously MUST be a response.
    if (!ns_msg_getflag(handle, ns_f_qr)) {
        RET_ERROR_PTR(ERR_UNSPEC, "received a query instead of a DNS response");
    }
    // Nor is this likely
    else if (ns_msg_getflag(handle, ns_f_opcode)) {
        RET_ERROR_PTR(ERR_UNSPEC, "DNS response contained unexpected opcode");
    } else if (ns_msg_getflag(handle, ns_f_rcode)) {
        RET_ERROR_PTR_FMT(ERR_UNSPEC, "DNS response rcode indicates an error: %u", ns_msg_getflag(handle, ns_f_rcode));
    }

    nanswers = ns_msg_count(handle, ns_s_an);
    _dump_dns_header(&handle);

    if (!nanswers) {
        RET_ERROR_PTR(ERR_UNSPEC, "DNS response contained no answers");
    }

    for(size_t i = 0; i < nanswers; i++) {
        _dbgprint(1, "---\nParsing answer #%u ...\n", (unsigned int)i + 1);

        if (ns_parserr(&handle, ns_s_an, i, &rr) < 0) {
            PUSH_ERROR_RESOLVER("ns_parserr");
            RET_ERROR_PTR(ERR_UNSPEC, "error in parsing MX request answers");
        }

        if (ns_rr_class(rr) != ns_c_in) {
            _dbgprint(1, "Skipped over answer with class != IN ...\n");
            continue;
        }

        rrtype = ns_rr_type(rr);

        if (rrtype != ns_t_mx) {
            _dbgprint(1, "Skipped over answer with type != MX ...\n");
            continue;
        }

        _dbgprint(1, "--- rr name: [%s], type %u, class %u, ttl %u, rdlen %u\n", ns_rr_name(rr), ns_rr_type(rr), ns_rr_class(rr), ns_rr_ttl(rr), ns_rr_rdlen(rr));

        strptr = (char *)ns_rr_rdata(rr);

        // The MX rdata wire format is a 16 bit preference number followed by the domain name of the exchange.
        // Lower values are preferred.
        pref = ntohs(*((uint16_t *)strptr));
        strptr += sizeof(pref);

        memset(nbuf, 0, sizeof(nbuf));

        if (ns_name_uncompress(ns_msg_base(handle), ns_msg_end(handle) + 1, (unsigned char *)strptr, nbuf, sizeof(nbuf)) == -1) {
            PUSH_ERROR_RESOLVER("ns_name_uncompress");
            RET_ERROR_PTR(ERR_UNSPEC, "could not uncompress name buffer in MX response");
        }

        if (!(strptr = strdup(nbuf))) {
            PUSH_ERROR_SYSCALL("strdup");
            RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for MX result");
        }

        if (!(rentry = malloc(sizeof(mx_record_t)))) {
            PUSH_ERROR_SYSCALL("malloc");
            free(strptr);
            RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for MX result");
        }

        memset(rentry, 0, sizeof(mx_record_t));
        rentry->name = strptr;
        rentry->pref = pref;

        // Append this string to the end of the array of null-terminated strings returned as the answer.
        _mem_append((unsigned char **)&result, &rsize, (unsigned char *)&rentry, sizeof(rentry));

        _dbgprint(1, "--- mx pref: %u, val: %s\n", pref, nbuf);
    }

    if (!result) {
        fprintf(stderr, "Error: Did not find requested MX record.\n");
    }
    // The array should be terminated with a null pointer.
    else {
        strptr = 0;
        _mem_append((unsigned char **)&result, &rsize, (unsigned char *)&strptr, sizeof(strptr));

        // Finally, our result set should be sorted by order of priority. A bubble sort will do.
        rptr = result;

        while (*rptr && *(rptr + 1)) {

            // Lower prefs come first.
            if ((*rptr)->pref > (*(rptr + 1))->pref) {
                oentry = *rptr;
                *rptr = *(rptr + 1);
                *(rptr + 1) = oentry;
                rptr = result;
                continue;
            }

            rptr++;
        }

    }

    return result;
}


/**
 * @brief   Initialize the DNS resolver subsystem.
 * @return  -1 on failure or 0 on success.
 */
int _initialize_resolver(void) {

    char *root_key_path;

    if (!(root_key_path = _get_dime_dir_location(ROOT_KEY_FILE))) {
        RET_ERROR_INT(ERR_UNSPEC, "unable to get location of root anchor file");
    }

    if (_load_dnskey_file(root_key_path) < 0) {
        PUSH_ERROR_FMT(ERR_UNSPEC, "could not load root public key from config file: %s", root_key_path);
        free(root_key_path);
        return -1;
    }

    free(root_key_path);

    if (res_init() < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "unexpected error occurred in res_init()");
    }

    _dbgprint(5, "Resolver options: retransmit: %u, retry: %u, options: %lu, nscount: %u, id: %u\n", _res.retrans, _res.retry, _res.options, _res.nscount, _res.id);

    _res.options |= (RES_USE_INET6 | RES_USE_DNSSEC);
//  _res.options &= ~(/*RES_USEVC |*/ RES_STAYOPEN | RES_DEFNAMES | RES_DNSRCH);

    // TODO: We probably want the CD bit set but not sure how to do this.
    // res_nmkquery
    //
    //
    return 0;
}


/**
 * @brief   A callback handler to destroy a DNSKEY record.
 * @note    This is an internal function used by the cache management subsystem.
 * @param   record  a pointer to the DNSKEY structure to be destroyed.
 */
void _destroy_dnskey_record_cb(void *record) {

    _destroy_dnskey((dnskey_t *)record);
}


/**
* @brief    A callback handler to dump a DNSKEY record to the console.
* @note     This is an internal function used by the cache management subsystem.
* @param    fp  a pointer to the file stream that will receive the dump output.
* @param    record  a pointer to the DNSKEY structure to be dumped.
* @param    brief   if set, only print a brief one-line description for the requested record.
*/
void _dump_dnskey_record_cb(FILE *fp, void *record, int brief) {

    dnskey_t *key = (dnskey_t *)record;
    ds_t **dsptr;
    unsigned char hashbuf[64], h160 = 0, h256 = 0, h512 = 0;
    char *signbuf = NULL, dsbuf[32];
    int ds_count = 0;

    if (!fp || !key) {
        return;
    }

    if (brief) {
        fprintf(fp, "%s", key->label);
        return;
    }

    signbuf = _get_signing_key_names(key->signkeys);
    ds_count = _count_ptr_chain(key->dse);

    memset(dsbuf, 0, sizeof(dsbuf));

    if (ds_count > 0) {
        snprintf(dsbuf, sizeof(dsbuf), "yes (%d)", ds_count);
    } else {
        snprintf(dsbuf, sizeof(dsbuf), "no");

        if (ds_count < 0) {
            _clear_error_stack();
        }

    }

    fprintf(fp, "--- DNSKEY: [%s]: keytag = %u, alg = %u, zone = %u, sep = %u, rdlen = %u, has_ds = %s, signed by %s, validated = %s\n",
            key->label, key->keytag, key->algorithm, key->is_zone, key->is_sep,
            (unsigned int)key->rdlen, dsbuf, (signbuf ? signbuf : "[error]"), (_is_validated_key(key) == 1 ? "yes" : "no"));

    // First check to see if we're attached to any DS records. If so, dump the corresponding hashes.
    if (key->dse) {

        for (dsptr = key->dse; *dsptr; dsptr++) {

            if ((*dsptr)->digest_type == DNSSEC_DIGEST_SHA1) {
                h160 = 1;
            } else if ((*dsptr)->digest_type == DNSSEC_DIGEST_SHA256) {
                h256 = 1;
            }

        }

    } else {

        switch(key->algorithm) {
        case NS_ALG_RSASHA1:
            h160 = 1;
            break;
        case NS_ALG_RSASHA256:
            h256 = 1;
            break;
        case NS_ALG_RSASHA512:
            h512 = 1;
            break;
        default:
            break;
        }

    }

    if (h160) {
        _compute_dnskey_sha_hash(key, 160, hashbuf);
        fprintf(fp, "------ DNSKEY SHA1 hash: ");
        _dump_buf(hashbuf, SHA_160_SIZE, 1);
    }

    if (h256) {
        _compute_dnskey_sha_hash(key, 256, hashbuf);
        fprintf(fp, "------ DNSKEY SHA256 hash: ");
        _dump_buf(hashbuf, SHA_256_SIZE, 1);
    }

    if (h512) {
        _compute_dnskey_sha_hash(key, 512, hashbuf);
        fprintf(fp, "------ DNSKEY SHA512 hash: ");
        _dump_buf(hashbuf, SHA_512_SIZE, 1);
    }

    if (!h160 && !h256 && !h512) {
        fprintf(fp, "------ [unknown digest type]\n");
    }

    if (signbuf) {
        free(signbuf);
    }

}


/**
 * @brief   A callback handler to destroy a DS record.
 * @note    This is an internal function used by the cache management subsystem.
 * @param   record  a pointer to the DS structure to be destroyed.
 */
void _destroy_ds_record_cb(void *record) {

    _destroy_ds((ds_t *)record);
}


/**
* @brief    A callback handler to dump a DS record to a file.
* @note     This is an internal function used by the cache management subsystem.
* @param    fp  the file to which the DS record is dumped.
* @param    record  a pointer to the DS structure to be dumped.
* @param    brief   if set, only print a brief one-line description for the requested record.
*/
void _dump_ds_record_cb(FILE *fp, void *record, int brief) {

    ds_t *ds = (ds_t *)record;
    char *signbuf = NULL;

    if (!ds) {
        return;
    }

    if (brief) {
        fprintf(fp, "%s", ds->label);
        return;
    }

    signbuf = _get_signing_key_names(ds->signkeys);

    fprintf(fp, "--- DS: [%s], alg = %u, digest = %u, keytag = %u, signed by %s, len = %u",
            ds->label, ds->algorithm, ds->digest_type, ds->keytag, (signbuf ? signbuf : "[error]"), (unsigned int)ds->diglen);

    if (ds->validated) {
        fprintf(fp, " [VALIDATED]\n");
    } else {
        fprintf(fp, "\n");
    }

    _dump_buf(ds->digest, ds->diglen, 1);

    if (signbuf) {
        free(signbuf);
    }

}


/**
 * @brief   A callback handler to deserialize a DS record from the object cache.
 */
void *_deserialize_ds_record_cb(void *data, size_t len) {

    unsigned char *dptr = (unsigned char *)data, *dend = (unsigned char *)data + len;
    ds_t *result;

    if (!data || !len) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(result = malloc(sizeof(ds_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "unable to allocate space for deserialized DS record");
    }

    memset(result, 0, sizeof(ds_t));

    if (!_deserialize_string(&(result->label), &dptr, dend) ||
        !_deserialize_data(&(result->algorithm), &dptr, dend, sizeof(result->algorithm)) ||
        !_deserialize_data(&(result->digest_type), &dptr, dend, sizeof(result->digest_type)) ||
        !_deserialize_data((unsigned char *)&(result->keytag), &dptr, dend, sizeof(result->keytag)) ||
        (!(result->digest = _deserialize_vardata(&dptr, dend, &(result->diglen)))) ||
        !_deserialize_data(&(result->validated), &dptr, dend, sizeof(result->validated))) {
        _destroy_ds(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize DS record from persistent cache");
    }

    // TODO: validation function?

    return result;
}


/**
 * @brief   A callback record to serialize a DS record for storage in the object cache.
 */
void *_serialize_ds_record_cb(void *record, size_t *outlen) {

    ds_t *ds = (ds_t *)record;
    unsigned char *buf = NULL;

    if (!record || !outlen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    *outlen = 0;

    if (!_mem_append_serialized_string(&buf, outlen, ds->label) ||
        !_mem_append(&buf, outlen, &(ds->algorithm), sizeof(ds->algorithm)) ||
        !_mem_append(&buf, outlen, &(ds->digest_type), sizeof(ds->digest_type)) ||
        !_mem_append(&buf, outlen, (unsigned char *)&(ds->keytag), sizeof(ds->keytag)) ||
        !_mem_append_serialized(&buf, outlen, ds->digest, ds->diglen) ||
        !_mem_append(&buf, outlen, &(ds->validated), sizeof(ds->validated))) {
        RET_ERROR_PTR(ERR_NOMEM, "unable to deserialize DS record");
    }

    return buf;
}


/**
 * @brief   A callback handler to deserialize a DNSKEY record.
 * @note    This is an internal function used by the cache management subsystem.
 * @param   data    a pointer to a buffer containing the data to be deserialized.
 * @param   len the length, in bytes, of the data buffer to be deserialized.
 * @return  a pointer to a newly allocated dnskey_t structure on success, or NULL on failure.
 */
void *_deserialize_dnskey_record_cb(void *data, size_t len) {

    unsigned char *dptr = (unsigned char *)data, *dend = (unsigned char *)data + len;
    unsigned char *keydata;
    dnskey_t *result;
    size_t olen;

    if (!data || !len) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(result = malloc(sizeof(dnskey_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for deserialized DNSKEY");
    }

    memset(result, 0, sizeof(dnskey_t));

    if (!_deserialize_string(&(result->label), &dptr, dend) ||
        !_deserialize_data(&(result->algorithm), &dptr, dend, sizeof(result->algorithm)) ||
        !_deserialize_data(&(result->is_zone), &dptr, dend, sizeof(result->is_zone)) ||
        !_deserialize_data(&(result->is_sep), &dptr, dend, sizeof(result->is_sep))) {
        _destroy_dnskey(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize DNSKEY record from persistent cache");
    }

    if (!(keydata = _deserialize_vardata(&dptr, dend, &olen))) {
        _destroy_dnskey(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize DNSKEY record public key from persistent cache");
    }

    if (!(result->pubkey = _decode_rsa_pubkey(keydata, olen))) {
        _destroy_dnskey(result);
        free(keydata);
        RET_ERROR_PTR(ERR_UNSPEC, "could not decode DNSKEY record public key from persistent cache");
    }

    free(keydata);

    // We deserialize the rdlen to determine exactly how much rdata we need to read.
    if (!_deserialize_data((unsigned char *)&(result->keytag), &dptr, dend, sizeof(result->keytag)) ||
        !_deserialize_data((unsigned char *)&(result->rdlen), &dptr, dend, sizeof(result->rdlen))) {
        _destroy_dnskey(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize DNSKEY record from persistent cache");
    }

    if (!(result->rdata = malloc(result->rdlen))) {
        PUSH_ERROR_SYSCALL("malloc");
        _destroy_dnskey(result);
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for DNSKEY rdata");
    }

    if (!_deserialize_data(result->rdata, &dptr, dend, result->rdlen)) {
        _destroy_dnskey(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize DNSKEY record rdata from persistent cache");
    }

    if (!_deserialize_data((unsigned char *)&(result->validated), &dptr, dend, sizeof(result->validated))) {
        _destroy_dnskey(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize DNSKEY record from persistent cache");
    }


    // TODO: validation function?
    // TODO: link up dse field to DS key

    return result;
}


/**
 * @brief   A callback handler to serialize a DNSKEY record.
 * @note    This is an internal function used by the cache management subsystem.
 * @param   record  a pointer to the DNSKEY structure to be serialized.
 * @param   outlen  a pointer to a variable that will receive the length of the serialized DNSKEY.
 * @return  a pointer to a newly allocated buffer holding the serialized DNSKEY on success, or NULL on failure.
 */
void *_serialize_dnskey_record_cb(void *record, size_t *outlen) {

    dnskey_t *key = (dnskey_t *)record;
    unsigned char *buf = NULL, *keydata;
    size_t klen;

    if (!key || !outlen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    } else if (!key->pubkey) {
        RET_ERROR_PTR(ERR_BAD_PARAM, "record was missing pubkey");
    }

    *outlen = 0;

    if (!(keydata = _encode_rsa_pubkey(key->pubkey, &klen))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not serialize DNSKEY record public key");
    }

    if (!_mem_append_serialized_string(&buf, outlen, key->label) ||
        !_mem_append(&buf, outlen, &(key->algorithm), sizeof(key->algorithm)) ||
        !_mem_append(&buf, outlen, &(key->is_zone), sizeof(key->is_zone)) ||
        !_mem_append(&buf, outlen, &(key->is_sep), sizeof(key->is_sep)) ||
        !_mem_append_serialized(&buf, outlen, keydata, klen) ||
        !_mem_append(&buf, outlen, (unsigned char *)&(key->keytag), sizeof(key->keytag)) ||
        !_mem_append(&buf, outlen, (unsigned char *)&(key->rdlen), sizeof(key->rdlen)) ||
        !_mem_append(&buf, outlen, key->rdata, key->rdlen) ||
        !_mem_append(&buf, outlen, (unsigned char *)&(key->validated), sizeof(key->validated))) {
        free(keydata);

        if (buf) {
            free(buf);
        }

        RET_ERROR_PTR(ERR_NOMEM, "could not allocate memory for DNSKEY serialization");
    }

    free(keydata);

    return buf;
}


// TODO: Will this be use used in the future? For the moment it is not.
void *_clone_dnskey_record_cb(void *record) {

    dnskey_t *result, *original = (dnskey_t *)record;

    if (!record) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(result = malloc(sizeof(dnskey_t)))) {
        RET_ERROR_PTR(ERR_NOMEM, NULL);
    }

    memset(result, 0, sizeof(dnskey_t));

    if (original->label && (!(result->label = strdup(original->label)))) {
        PUSH_ERROR_SYSCALL("strdup");
        _destroy_dnskey(result);
        RET_ERROR_PTR(ERR_NOMEM, "could not clone DNSKEY label");
    }

    result->algorithm = original->algorithm;
    result->is_zone = original->is_zone;
    result->is_sep = original->is_sep;
    result->keytag = original->keytag;
    result->rdlen = original->rdlen;
    result->validated = original->validated;

    if (original->rdata && (!(result->rdata = malloc(result->rdlen)))) {
        _destroy_dnskey(result);
        RET_ERROR_PTR(ERR_NOMEM, "could not clone DNSKEY rdata");
    }

    memcpy(result->rdata, original->rdata, result->rdlen);

    if (original->pubkey && (!(result->pubkey = RSAPublicKey_dup_d(original->pubkey)))) {
        _destroy_dnskey(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not clone DNSKEY RSA key");
    }

    // We can clone these pointer chains because they consist of lists of shallow copies we don't own.
    if (original->signkeys && (!(result->signkeys = _ptr_chain_clone(original->signkeys)))) {
        _destroy_dnskey(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not clone DNSKEY signing key records");
    }

    if (original->dse && (!(result->dse = _ptr_chain_clone(original->dse)))) {
        _destroy_dnskey(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not clone DNSKEY DS records");
    }

    return result;
}


/**
 * @brief   Link any DNSKEY entries in the cache with dangling DS pointers to the right place.
 * @note    This function should be used after the retrieval of new records, or after a cache load.
 */
void _fixup_ds_links(void) {

/*  cached_store_t *store = &(cached_stores[cached_data_dnskey]);
        cached_object_t *ptr = store->head;
        dnskey_t *key; */


//printf("XXX: fixing up ds links\n");

/*  while (ptr) {
                key = (dnskey_t *)ptr->data;

                if (key && !key->dse) {
                        key->dse = _get_ds_by_dnskey(key);
                        printf("XXX: dse was null, now = %lx [%s]\n", (unsigned long)key->dse, key->label);
                } else { }

                ptr = ptr->next;
        } */

}


/**
 * @brief   Mark as validated any DNSKEY entries in the object cache that should be but aren't.
 * @note    The retrieval of a new, validated DNSKEY can mean that another DNSKEY in the cache that
 *              was signed by it will now be validated by virtue of transitivity.
 */
void _fixup_dnskey_validation(void) {

    cached_store_t *store = &(cached_stores[cached_data_dnskey]);
    cached_object_t *ptr;
    dnskey_t *key;
    int res;

    _lock_cache_store(store);
    ptr = store->head;

    while (ptr) {
        key = (dnskey_t *)ptr->data;

        if (key && (!key->validated)) {

            // If the DNSKEY can be validated, and it's cacheable, it should be persisted.
            if ((res = _is_validated_key(key)) == 1) {

                if (key->do_cache) {
                    ptr->persists = 1;
                }
                // Otherwise it shouldn't be persisted in any case if it's not validated.
            } else {
                ptr->persists = 0;
            }
            // If the key was already validated and it's cacheable, make certain that it is persisted.
        } else if (key && key->validated && key->do_cache) {
            ptr->persists = 1;
        }

        ptr = ptr->next;
    }

    _unlock_cache_store(store);

}


/**
 * @brief   Sort the RRs in an RRSET into canonical order for DNSSEC signature verification.
 * @note    This function does not actually sort the targeted RR's, but rather produces an output buffer
 *      containing an array of the sorted RR indices.
 * @param   dhandle     the handle of the DNS reply containing the answer RRs to be sorted.
 * @param   ordbuf      a pointer to an array of integers of size nanswers that will be updated to
 *                              contain the new zero-indexed RR indices in proper canonically sorted order.
 * @param   nanswers    the number of answers in the DNS reply to be sorted (must match dhandle).
 * @return  -1 on error or 0 on success.
 */
int _sort_rrs_canonical(ns_msg *dhandle, int *ordbuf, size_t nanswers) {

    ns_rr rr1, rr2;
    const unsigned char *rrdata1, *rrdata2;
    size_t i, rrlen1, rrlen2, swap;

    if (!dhandle || !ordbuf || !nanswers) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (nanswers != ns_msg_count(*dhandle, ns_s_an)) {
        RET_ERROR_INT(ERR_UNSPEC, "expected number of answers in response does not match packet");
    }

    for (i = 0; i < nanswers; i++) {
        ordbuf[i] = i;
    }

    i = 0;

    while (i < (nanswers - 1)) {

        if ((ns_parserr(dhandle, ns_s_an, ordbuf[i], &rr1) < 0) || (ns_parserr(dhandle, ns_s_an, ordbuf[i + 1], &rr2) < 0)) {
            PUSH_ERROR_RESOLVER("ns_parserr");
            RET_ERROR_INT(ERR_UNSPEC, "encountered error parsing answer for canonical sort");
        }

        rrlen1 = ns_rr_rdlen(rr1);
        rrlen2 = ns_rr_rdlen(rr2);
        rrdata1 = ns_rr_rdata(rr1);
        rrdata2 = ns_rr_rdata(rr2);

        if (_compare_rdata(rrdata1, rrlen1, rrdata2, rrlen2) > 0) {
            swap = ordbuf[i];
            ordbuf[i] = ordbuf[i + 1];
            ordbuf[i + 1] = swap;
            i = 0;
        } else {
            i++;
        }

    }

    return 0;
}


/** @brief  Compare two RR's, for the purpose of allowing for canonical ordering within an RRSET.
  * @note       This function is used to sort resource records for the reliable validation of RRSIG records.
  * @note   According to Section 6.3 of RFC 4034 ("Canonical RR Ordering within an RRSET"):
  *     This is treated as a left-justified octet sequence where the absence of an octet sorts before a zero octet.
  * @param  rdata1  a pointer to the beginning of the first resource record's rdata.
  * @param  rdlen1  the length, in bytes, of the first resource record's rdata.
  * @param  rdata2  a pointer to the beginning of the second resource record's rdata.
  * @param  rdlen2  the length, in bytes, of the second resource record's rdata.
  * @return -1 if rdata1 < rdata2, 1 if rdata2 > rdata1, or 0 if the two resource records were equal.
  */
int _compare_rdata(const unsigned char *rdata1, size_t rdlen1, const unsigned char *rdata2, size_t rdlen2) {

    size_t minlen;

    if (!rdata1 || !rdlen1) {
        return -1;
    } else if (!rdata2 || !rdlen2) {
        return 1;
    }

    minlen = (rdlen1 < rdlen2) ? rdlen1 : rdlen2;

    for (size_t i = 0; i < minlen; i++) {

        if (rdata1[i] > rdata2[i]) {
            return 1;
        } else if (rdata1[i] < rdata2[i]) {
            return -1;
        }

    }

    if (rdlen1 < rdlen2) {
        return -1;
    } else if (rdlen1 > rdlen2) {
        return 1;
    }

    return 0;
}


/**
 * @brief   Get the names of a collection of signing DNSKEYs for human display.
 * @param   keys    a pointer to an array of DNSKEY entries to be printed to a human-readable string.
 * @return  NULL on failure, or a pointer to a newly allocated null-terminated string containing the
 *              DNSKEY collection info on success.
 */
char *_get_signing_key_names(dnskey_t **keys) {

    dnskey_t **dkptr;
    char *result = NULL;
    int res;

    if (!keys || !*keys) {

        if (!(result = strdup("[none]"))) {
            PUSH_ERROR_SYSCALL("strdup");
            RET_ERROR_PTR(ERR_NOMEM, NULL);
        }

        return result;
    }

    for (dkptr = keys; *dkptr; dkptr++) {

        if (!result) {
            res = _str_printf(&result, "%s[%u:%u]", (*dkptr)->label, (*dkptr)->keytag, (*dkptr)->algorithm);
        } else {
            res = _str_printf(&result, "/%s[%u:%u]", (*dkptr)->label, (*dkptr)->keytag, (*dkptr)->algorithm);
        }

        if (!res) {
            RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for signing keys names");
        }

    }

    return result;
}
