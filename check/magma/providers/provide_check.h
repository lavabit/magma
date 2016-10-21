
/**
 * @file /check/providers/provide_check.h
 *
 * @brief The entry point for the provide module test suite.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef PROVIDE_CHECK_H
#define PROVIDE_CHECK_H

typedef struct {
	uint64_t tnum, onum, adler32, fletcher32, crc32, crc64, murmur32, murmur64;
} check_tank_obj_t;

typedef struct {
	uint64_t engine;
} check_tank_opt_t;

typedef struct {
	uint64_t engine;
} check_compress_opt_t;

/// symmetric_check.c
bool_t   check_symmetric_sthread(chr_t *name);

/// rand_check.c
stringer_t *  check_rand_mthread(void);
void          check_rand_mthread_wrap(void);
stringer_t *  check_rand_sthread(void);

/// tank_check.c
bool_t   check_tokyo_tank(check_tank_opt_t *opts);
bool_t   check_tokyo_tank_cleanup(inx_t *check_collection);
bool_t   check_tokyo_tank_load(char *location, inx_t *check_collection, check_tank_opt_t *opts);
bool_t   check_tokyo_tank_mthread(check_tank_opt_t *opts);
void     check_tokyo_tank_mthread_cnv(check_tank_opt_t *opts);
bool_t   check_tokyo_tank_sthread(check_tank_opt_t *opts);
bool_t   check_tokyo_tank_verify(inx_t *check_collection);

/// scramble_check.c
bool_t   check_scramble_sthread(void);

/// dspam_check.c
bool_t   check_dspam_binary_sthread(chr_t *location);
bool_t   check_dspam_mail_sthread(chr_t *location);

/// provide_check.c
Suite *      suite_check_provide(void);

/// virus_check.c
chr_t *  check_virus_sthread(chr_t *location);

/// ecies_check.c
void     check_ecies_cleanup(EC_KEY *key, cryptex_t *ciphered, stringer_t *hex_pub, stringer_t *hex_priv, unsigned char *text, unsigned char *copy, unsigned char *original);
bool_t   check_ecies_sthread(void);

/// hash_check.c
bool_t   check_hash_simple(void);
bool_t   check_hash_sthread(chr_t *name);

/// hmac_check.c
bool_t   check_hmac_simple(void);
bool_t   check_hmac_parameters(void);

/// compress_check.c
bool_t   check_compress_mthread(check_compress_opt_t *opts);
void     check_compress_mthread_cnv(check_compress_opt_t *opts);
bool_t   check_compress_sthread(check_compress_opt_t *opts);

/// stacie_check.c
bool_t   check_stacie_simple(void);
bool_t   check_stacie_parameters(void);
bool_t   check_stacie_rounds(void);
bool_t   check_stacie_determinism(void);
bool_t   check_stacie_bitflip(void);

/// prime_check.c
bool_t   check_prime_secp256k1_sthread(stringer_t *);

/// unicode_check.c
bool_t   check_unicode_invalid(stringer_t *errmsg);
bool_t   check_unicode_length(stringer_t *errmsg);
bool_t   check_unicode_valid(stringer_t *errmsg);

#endif
