
/**
 * @file /magma/src/providers/prime/primitives/primitives.h
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef PRIME_PRIMITIVES_H
#define PRIME_PRIMITIVES_H

#define PRIME_MAX_1_BYTE 255
#define PRIME_MAX_2_BYTE 65535
#define PRIME_MAX_3_BYTE 16777215
#define PRIME_MAX_4_BYTE 4294967295

typedef uint8_t prime_field_type_t;

/// objects.c
size_t   prime_object_size_max(prime_type_t type);
size_t   prime_object_size_min(prime_type_t type);
chr_t *  prime_object_type(prime_type_t type);

/// headers.c
size_t        prime_header_length(prime_type_t type);
stringer_t *  prime_header_encrypted_org_key_write(size_t size, stringer_t *output);
stringer_t *  prime_header_org_key_write(size_t size, stringer_t *output);
stringer_t *  prime_header_org_signet_write(size_t size, stringer_t *output);
stringer_t *  prime_header_encrypted_user_key_write(size_t size, stringer_t *output);
stringer_t *  prime_header_user_key_write(size_t size, stringer_t *output);
stringer_t *  prime_header_user_signet_write(size_t size, stringer_t *output);
stringer_t *  prime_header_user_signing_request_write(size_t size, stringer_t *output);
stringer_t *  prime_header_write(prime_type_t type, size_t size, stringer_t *output);
stringer_t *  prime_header_encrypted_message_write(size_t size, stringer_t *output);

/// fields.c
size_t        prime_field_size_length(prime_type_t type, prime_field_type_t field);
size_t        prime_field_size_max(prime_type_t type, prime_field_type_t field);
stringer_t *  prime_field_write(prime_type_t type, prime_field_type_t field, size_t size, stringer_t *data, stringer_t *output);

#endif

