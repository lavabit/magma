
/**
 * @file /magma/check/magma/prime/prime_check.h
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_PRIME_CHECK_H
#define MAGMA_PRIME_CHECK_H

/// ed25519_check.c
bool_t   check_prime_ed25519_fixed_sthread(stringer_t *errmsg);
bool_t   check_prime_ed25519_fuzz_sthread(stringer_t *errmsg);
bool_t   check_prime_ed25519_parameters_sthread(stringer_t *errmsg);

/// prime_objects.c
bool_t   check_prime_keys_sthread(stringer_t *errmsg);
bool_t   check_prime_writers_sthread(stringer_t *errmsg);

/// secp256k1_check.c
bool_t   check_prime_secp256k1_fixed_sthread(stringer_t *errmsg);
bool_t   check_prime_secp256k1_keys_sthread(stringer_t *errmsg);
bool_t   check_prime_secp256k1_parameters_sthread(stringer_t *errmsg);

/// stacie_check.c
bool_t   check_stacie_bitflip(void);
bool_t   check_stacie_determinism(void);
bool_t   check_stacie_parameters(void);
bool_t   check_stacie_rounds(void);
bool_t   check_stacie_simple(void);

/// prime_check.c
Suite *  suite_check_prime(void);

#endif

