#ifndef MREC_H
#define MREC_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "dime/sds/sds.h"
#include <openssl/rsa.h>
#include "dime/common/dcrypto.h"
#include "dime/common/error.h"


#define DIME_VERSION_NO        1
#define DIME_RECORD_DNS_PREFIX "_dx"


typedef enum {
    msg_experimental = 0,
    msg_mixed = 1,
    msg_strict = 2
} dime_msg_policy;

typedef enum {
    sub_strict = 0,
    sub_relaxed = 1,
    sub_explicit = 2
} dime_sub_policy;


typedef struct {
    unsigned short version;                 ///< The DIME management record syntax version.
    unsigned char **pubkey;                 ///< An array of 32-byte ED25519 organizational public key(s) (POKs).
    unsigned char **tlssig;                 ///< An array of 64-byte TLS signature(s) for MX (or DX/KS) by the POK(s).
    dime_msg_policy policy;                 ///< Policy for sending/accepting messages.
    char *syndicates;                       ///< Alternative authoritative signet lookup sources.
    char **dx;                              ///< An array of CNAME(s) for DIME delivery host (if not present, then MX).
    unsigned long expiry;                   ///< Number of days before a cached management record is discarded.
    dime_sub_policy subdomain;              ///< Determines whether subdomains will have authority over their own records.
    int validated;                          ///< 1 if validated by DNSSEC; 0 if not DNSSEC-protected; -1 if DNSSEC sig failed.
} dime_record_t;


// Public interface.
PUBLIC_FUNC_DECL(void,            destroy_dime_record,       dime_record_t *record);
PUBLIC_FUNC_DECL(dime_record_t *, parse_dime_record,         const char *txt, size_t len);
PUBLIC_FUNC_DECL(dime_record_t *, get_dime_record,           const char *domain, unsigned long *ttl, int use_cache);
PUBLIC_FUNC_DECL(int,             validate_dime_record,      const dime_record_t *record);
PUBLIC_FUNC_DECL(dime_record_t *, get_dime_record_from_file, const char *filename, const char *domain);


// Internal functions.
void  _destroy_dime_record_cb(void *record);
void  _dump_dime_record_cb(FILE *, void *record, int brief);
void *_deserialize_dime_record_cb(void *data, size_t len);
void *_serialize_dime_record_cb(void *record, size_t *outlen);

#endif
