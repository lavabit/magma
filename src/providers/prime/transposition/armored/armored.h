
/**
 * @file /magma/src/providers/prime/formats/armored/armored.h
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef PRIME_AMORED_H
#define PRIME_AMORED_H

/// pem.c
stringer_t *  prime_pem_begin(prime_type_t type);
stringer_t *  prime_pem_end(prime_type_t type);
stringer_t *  prime_pem_unwrap(stringer_t *pem, stringer_t *output);
stringer_t *  prime_pem_wrap(stringer_t *object, stringer_t *output);





#endif

