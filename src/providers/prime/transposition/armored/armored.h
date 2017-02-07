
/**
 * @file /magma/providers/prime/transposition/armored/armored.h
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 */

#ifndef PRIME_AMORED_H
#define PRIME_AMORED_H

/// pem.c
stringer_t *  prime_pem_begin(uint16_t type);
stringer_t *  prime_pem_end(uint16_t type);
stringer_t *  prime_pem_unwrap(stringer_t *pem, stringer_t *output);
stringer_t *  prime_pem_wrap(stringer_t *object, stringer_t *output);





#endif

