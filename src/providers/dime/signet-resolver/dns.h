#ifndef DNS_H
#define DNS_H

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser_compat.h>
#include <netdb.h>
#include <resolv.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include "dime/common/error.h"

// TODO: Does DNSKEY RR have "key revoked" bit?
//       SEP bit indicates KSK vs. 0=ZSK?



#define ROOT_KEY_FILE "root-anchor.key"

#define IS_ROOT_LABEL(lname) (!lname || !strlen(lname) || *lname == '.')



#ifndef T_DNSKEY
#define T_DNSKEY 48
#endif

#ifndef T_DS
#define T_DS     43
#endif

#ifndef T_RRSIG
#define T_RRSIG  46
#endif

#define DNSKEY_RR_LEN      4
#define DNSKEY_RR_FLAG_ZK  0x100
#define DNSKEY_RR_FLAG_SEP 0x001
#define DNSKEY_RR_PROTO    3

#define NS_ALG_RSASHA1   5                      /* Mandatory */
#define NS_ALG_RSASHA256 8
#define NS_ALG_RSASHA512 10

#define DNSSEC_DIGEST_SHA1   1
#define DNSSEC_DIGEST_SHA256 2


typedef struct  {
#if BYTE_ORDER == BIG_ENDIAN
    unsigned int reserved1 : 7;
    unsigned int zone_key : 1;
    unsigned int reserved2 : 7;
    unsigned int sep : 1;
#else
    unsigned int zone_key : 1;
    unsigned int reserved1 : 7;
    unsigned int sep : 1;
    unsigned int reserved2 : 7;
#endif
} __attribute__((__packed__)) dnskey_rr_flags_t;

typedef struct {
    uint16_t covered;               ///< The type of the rr set covered by this record.
    uint8_t algorithm;              ///< The numerical id of the cryptographic algorithm used to generate the signature.
    uint8_t labels;                 ///< The number of labels in the original RRSIG RR owner name.
    uint32_t ottl;                  ///< The original TTL of the covered rrset as it appears in the authoritative zone.
    uint32_t expiration;            ///< The expiration date after which the RRSIG record must not be used for authentication.
    uint32_t inception;             ///< The inception date before which the RRSIG record must not be used for authentication.
    uint16_t key_tag;               ///< A tag (selector) identifying the corresponding DNSKEY RR (which isn't necessarily unique) that validates this record.
    unsigned char signame;          ///< A label identifying the owner name of the DNSKEY RR validating this signature (must not use name compression).
    // Signame is followed by the actual signature
} __attribute__((__packed__)) rrsig_rr_t;

typedef struct {
    uint16_t key_tag;               ///< The keytag of the DNSKEY RR to which this record refers.
    uint8_t algorithm;              ///< The algorithm used by this RR's referenced DNSKEY.
    uint8_t digest_type;            ///< The type of the digest algorithm used by the digest field.
    unsigned char digest;           ///< A digest of this RR's referenced DNSKEY.
} __attribute__((__packed__)) ds_rr_t;


typedef struct ds ds_t;
typedef struct dnskey dnskey_t;

/** Our own internal representation of certain RRs. */
struct ds {
    char *label;
    unsigned char algorithm;
    unsigned char digest_type;
    unsigned int keytag;
    unsigned char *digest;
    size_t diglen;
    unsigned char validated;        ///< This is for the cache.
    dnskey_t **signkeys;            ///< The DNSKEY(s) that were used to sign the RRSIG guaranteeing this DS.
};

struct dnskey {
    char *label;
    unsigned char algorithm;
    unsigned char is_zone;
    unsigned char is_sep;
    RSA *pubkey;
    unsigned int keytag;
    size_t rdlen;
    unsigned char *rdata;
    dnskey_t **signkeys;            ///< The DNSKEY(s) that were used to sign the RRSIG guaranteeing this DNSKEY.
    ds_t **dse;                     ///< The DS RR(s) that match this DNSKEY record.
    unsigned int validated;         ///< Has this key been validated?
    unsigned int do_cache;          ///< If set, this DNSKEY will be persisted IFF it has been validated.
};


typedef struct {
    unsigned short pref;
    char *name;
} mx_record_t;



// Public DNS interface.
PUBLIC_FUNC_DECL(int,            load_dnskey_file,        const char *filename);
PUBLIC_FUNC_DECL(int,            is_validated_key,        dnskey_t *dk);

PUBLIC_FUNC_DECL(int,            compute_dnskey_sha_hash, const dnskey_t *key, size_t nbits, unsigned char *outbuf);
PUBLIC_FUNC_DECL(dnskey_t *,     get_dnskey_by_tag,       unsigned int tag, const char *signer, int force_lookup);
PUBLIC_FUNC_DECL(ds_t *,         get_ds_by_dnskey,        const dnskey_t *key);
PUBLIC_FUNC_DECL(unsigned int,   get_keytag,              const unsigned char *rdata, size_t rdlen);

PUBLIC_FUNC_DECL(void,           destroy_dnskey,          dnskey_t *key);
PUBLIC_FUNC_DECL(void,           destroy_ds,              ds_t *ds);

PUBLIC_FUNC_DECL(int,            rsa_verify_record,       const char *label, unsigned char algorithm, RSA *pubkey, const unsigned char *rrsig, const unsigned char *sigbuf, size_t siglen, ns_msg *dhandle);
PUBLIC_FUNC_DECL(int,            validate_rrsig_rr,       const char *label, ns_msg *dhandle, unsigned short covered, const unsigned char *rdata, size_t rdlen, dnskey_t **outkey);
PUBLIC_FUNC_DECL(void *,         lookup_dnskey,           const char *label);
PUBLIC_FUNC_DECL(void *,         lookup_ds,               const char *label);
PUBLIC_FUNC_DECL(char *,         get_txt_record,          const char *qstring, unsigned long *ttl, int *validated);
PUBLIC_FUNC_DECL(mx_record_t **, get_mx_records,          const char *qstring);
PUBLIC_FUNC_DECL(void,           free_mx_records,         mx_record_t **mxs);


// Internal routines
int        _initialize_resolver(void);

dnskey_t *_add_dnskey_entry(const char *label, const unsigned char *buf, size_t len, unsigned long ttl);
dnskey_t *_add_dnskey_entry_rsa(const char *label, uint16_t flags, unsigned char algorithm, RSA *pubkey, unsigned int keytag, const unsigned char *rdata,
                                size_t rdlen, unsigned long ttl, unsigned int do_cache, int forced);
ds_t *_add_ds_entry(const char *label, unsigned int keytag, unsigned char algorithm, unsigned char digest_type, const unsigned char *digest, size_t dlen, unsigned long ttl);
RSA * _get_rsa_dnskey(const unsigned char *rdptr, size_t rdlen);

void       _destroy_dnskey_record_cb(void *record);
void       _destroy_ds_record_cb(void *record);

void *_serialize_dnskey_record_cb(void *record, size_t *outlen);
void *_serialize_ds_record_cb(void *record, size_t *outlen);
void *_deserialize_dnskey_record_cb(void *data, size_t len);
void *_deserialize_ds_record_cb(void *data, size_t len);
void *_clone_dnskey_record_cb(void *record);

void      _dump_dns_header(ns_msg *handle);
void      _dump_dnskey_record_cb(FILE *fp, void *record, int brief);
void      _dump_ds_record_cb(FILE *fp, void *record, int brief);

void      _fixup_ds_links(void);
void      _fixup_dnskey_validation(void);
int       _sort_rrs_canonical(ns_msg *dhandle, int *ordbuf, size_t nanswers);
int       _compare_rdata(const unsigned char *rdata1, size_t rdlen1, const unsigned char *rdata2, size_t rdlen2);
char *    _get_signing_key_names(dnskey_t **keys);
int       _ds_comparator(const void *object, const void *key);
int       _dnskey_tag_comparator(const void *object, const void *key);
int       _dnskey_domain_comparator(const void *object, const void *key);

size_t   _mem_append_canon(unsigned char **buf, size_t *blen, const char *name);

#endif
