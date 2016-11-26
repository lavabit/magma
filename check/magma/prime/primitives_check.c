
/**
 * @file /magma/check/magma/prime/prime_objects.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

bool_t check_prime_org_keys_sthread(stringer_t *errmsg) {

	prime_key_t *holder = NULL;
	stringer_t *packed = NULL, *key = MANAGEDBUF(64);

	// Create a STACIE realm key.
	rand_write(key);

	// Allocate an org key.
	if (!(holder = prime_key_alloc(PRIME_ORG_KEY))) {
		st_sprint(errmsg, "Organizational key allocation failed.");
		return false;
	}

	prime_key_free(holder);

	// Generate an org key.
	if (!(holder = prime_key_generate(PRIME_ORG_KEY))) {
		st_sprint(errmsg, "Organizational key generation failed.");
		return false;
	}

	// Serialize the org key.
	else if (!(packed = prime_key_get(holder, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "Organizational key serialization failed.");
		prime_key_free(holder);
		return false;
	}

	prime_key_free(holder);

	// Unpack the serialized org key.
	if (!(holder = prime_key_set(packed))) {
		st_sprint(errmsg, "Organizational key parsing failed.");
		return false;
	}

	// Encrypt the org key.
	else if (!(packed = prime_encrypted_key_get(key, holder, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "Organizational key encryption failed.");
		prime_key_free(holder);
		return false;
	}

	prime_key_free(holder);

	// Decrypt the org key.
	if (!(holder = prime_encrypted_key_set(key, packed))) {
		st_sprint(errmsg, "Encrypted organizational key parsing failed.");
		return false;
	}

	prime_key_free(holder);

	// Perform the same checks, but this time make the functions
	// allocate memory for the output. Generate an org key.
	if (!(holder = prime_key_generate(PRIME_ORG_KEY))) {
		st_sprint(errmsg, "Organizational key generation failed.");
		return false;
	}

	// Serialize the org key.
	else if (!(packed = prime_key_get(holder, NULL))) {
		st_sprint(errmsg, "Organizational key serialization failed.");
		prime_key_free(holder);
		return false;
	}

	prime_key_free(holder);

	// Unpack the serialized org key.
	if (!(holder = prime_key_set(packed))) {
		st_sprint(errmsg, "Organizational key parsing failed.");
		st_free(packed);
		return false;
	}

	st_free(packed);

	// Encrypt the org key.
	if (!(packed = prime_encrypted_key_get(key, holder, NULL))) {
		st_sprint(errmsg, "Organizational key encryption failed.");
		prime_key_free(holder);
		return false;
	}

	prime_key_free(holder);

	// Decrypt the org key.
	if (!(holder = prime_encrypted_key_set(key, packed))) {
		st_sprint(errmsg, "Encrypted organizational key parsing failed.");
		st_free(packed);
		return false;
	}

	prime_key_free(holder);
	st_free(packed);

	return true;
}

bool_t check_prime_user_keys_sthread(stringer_t *errmsg) {

	prime_key_t *holder = NULL;
	stringer_t *packed = NULL, *key = MANAGEDBUF(64);

	// Create a STACIE realm key.
	rand_write(key);

	// Allocate a user key.
	if (!(holder = prime_key_alloc(PRIME_USER_KEY))) {
		st_sprint(errmsg, "User key allocation failed.");
		return false;
	}

	prime_key_free(holder);

	// Generate a user key.
	if (!(holder = prime_key_generate(PRIME_USER_KEY))) {
		st_sprint(errmsg, "User key generation failed.");
		return false;
	}

	// Serialize the user key.
	else if (!(packed = prime_key_get(holder, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "User key serialization failed.");
		prime_key_free(holder);
		return false;
	}

	prime_key_free(holder);

	// Unpack the serialized user key.
	if (!(holder = prime_key_set(packed))) {
		st_sprint(errmsg, "User key parsing failed.");
		return false;
	}

	// Encrypt the user key.
	else if (!(packed = prime_encrypted_key_get(key, holder, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "User key encryption failed.");
		prime_key_free(holder);
		return false;
	}

	prime_key_free(holder);

	// Decrypt the user key.
	if (!(holder = prime_encrypted_key_set(key, packed))) {
		st_sprint(errmsg, "Encrypted user key parsing failed.");
		return false;
	}

	prime_key_free(holder);

	// Perform the same checks, but this time make the functions
	// allocate memory for the output. Generate a new user key.
	if (!(holder = prime_key_generate(PRIME_USER_KEY))) {
		st_sprint(errmsg, "User key generation failed.");
		return false;
	}

	// Serialize the user key.
	else if (!(packed = prime_key_get(holder, NULL))) {
		st_sprint(errmsg, "User key serialization failed.");
		prime_key_free(holder);
		return false;
	}

	prime_key_free(holder);

	// Unpack the serialized user key.
	if (!(holder = prime_key_set(packed))) {
		st_sprint(errmsg, "User key parsing failed.");
		st_free(packed);
		return false;
	}

	st_free(packed);

	// Encrypt the user key.
	if (!(packed = prime_encrypted_key_get(key, holder, NULL))) {
		st_sprint(errmsg, "User key encryption failed.");
		prime_key_free(holder);
		return false;
	}

	prime_key_free(holder);

	// Decrypt the user key.
	if (!(holder = prime_encrypted_key_set(key, packed))) {
		st_sprint(errmsg, "Encrypted user key parsing failed.");
		st_free(packed);
		return false;
	}

	prime_key_free(holder);
	st_free(packed);

	return true;
}

bool_t check_prime_parameters_sthread(stringer_t *errmsg) {

	prime_key_t *holder = NULL;

	// Attempt allocation of a non-key type using the key allocation function.
	if ((holder = prime_key_alloc(PRIME_ORG_SIGNET)) || (holder = prime_key_alloc(PRIME_USER_SIGNET)) || (holder = prime_key_alloc(PRIME_USER_SIGNING_REQUEST))) {
		st_sprint(errmsg, "Key parameter checks failed.");
		prime_key_free(holder);
		return false;
	}

	return true;

}

bool_t check_prime_writers_sthread(stringer_t *errmsg) {

	// The minimum valid key length is 68, so lets try that.
	if (status() && st_cmp_cs_eq(prime_header_org_key_write(68, MANAGEDBUF(5)), hex_decode_st(NULLER("07a0000044"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for an org key.");
		return false;
	}

	else if (status() && st_cmp_cs_eq(prime_header_user_key_write(68, MANAGEDBUF(5)), hex_decode_st(NULLER("07dd000044"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for a user key.");
		return false;
	}

	// We don't have minimums setup yet, so we're using 1024 for the length.
	else if (status() && st_cmp_cs_eq(prime_header_org_signet_write(1024, MANAGEDBUF(5)), hex_decode_st(NULLER("06f0000400"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for an org signet.");
		return false;
	}
	else if (status() && st_cmp_cs_eq(prime_header_encrypted_org_key_write(1024, MANAGEDBUF(5)), hex_decode_st(NULLER("079b000400"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for an encrypted org key.");
		return false;
	}
	else if (status() && st_cmp_cs_eq(prime_header_user_signet_write(1024, MANAGEDBUF(5)), hex_decode_st(NULLER("06fd000400"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for a user signet.");
		return false;
	}
	else if (status() && st_cmp_cs_eq(prime_header_user_signing_request_write(1024, MANAGEDBUF(5)), hex_decode_st(NULLER("04bf000400"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for a user signing request.");
		return false;
	}
	else if (status() && st_cmp_cs_eq(prime_header_encrypted_user_key_write(1024, MANAGEDBUF(5)), hex_decode_st(NULLER("07b8000400"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for an encrypted user key.");
		return false;
	}
	else if (status() && st_cmp_cs_eq(prime_header_encrypted_message_write(1024, MANAGEDBUF(6)), hex_decode_st(NULLER("073700000400"), MANAGEDBUF(6)))) {
		st_sprint(errmsg, "Invalid PRIME header for an encrypted message.");
		return false;
	}

	// Try creating objects that are intentionally too small.
	if (status() && prime_header_org_key_write(34, MANAGEDBUF(5))) {
		st_sprint(errmsg, "PRIME header returned for invalid org key size.");
		return false;
	}

	else if (status() && prime_header_user_key_write(34, MANAGEDBUF(5))) {
		st_sprint(errmsg, "PRIME header returned for invalid user key size.");
		return false;
	}

	return true;
}

bool_t check_prime_unpacker_sthread(stringer_t *errmsg) {

	log_enable();
	prime_object_t *object = NULL;
	stringer_t *user_key = base64_decode(NULLER("B90AAEQBIAzFqb5wsMLLwJV1uUfVecHirAQVnHZbvlDqDqkwGZwzAiAk/epj8HtmvA/VUnMC9TfWwh1veCK9Bp+uExSfeuHCug=="), MANAGEDBUF(76)),
		*org_key = base64_decode(NULLER("B6AAAEQBIJw2BXyqCKDFsosdWHuGxUpD7CDNyYCCjtjKuS1zHUOAAyD0I69V/DkTLv/g9Maesc9Vs2Ssef8ao4ZTzDk7e+Nf1g=="), MANAGEDBUF(128)),
		*user_signing_request = base64_decode(NULLER("BL8AAIcBIUCrKVU0/tyKEetnxfXBVdYlws1cof0DJ/obRCg/QPT2pAIhA8SQ44pss9J5vxHp4jbCS9poZv0JlFSCQxH2t"
			"aDhfsMnBcU1Wa162o4FpEqAzcxETbD71HO27lRlzE8/Yd+RZT7KNROA4e/TAmphORiH7KK3yS6DnuSoSt7w/oFeYq180Aw="), MANAGEDBUF(1024)),
		*user_signet = base64_decode(NULLER("Bv0AAW4BIUCrKVU0/tyKEetnxfXBVdYlws1cof0DJ/obRCg/QPT2pAIhA8SQ44pss9J5vxHp4jbCS9poZv0JlFSCQxH2taDhfsMnBcU1Wa162o4FpEqAzcxETbD71HO2"
			"7lRlzE8/Yd+RZT7KNROA4e/TAmphORiH7KK3yS6DnuSoSt7w/oFeYq180AwGUtVYvtAR4szKkh3QHNSJ/Sqh1xvWmSEdm9RxDxcxf+bxS8G79PwJDHiFV+rW+5uetx+W"
			"sQTfhHDZQBZTNPdQARAQdXNlckBleGFtcGxlLmNvbf0HkBxZvkoQG6rwhGZTxd6LUZNiJ6aAfu9cxw3kaceoG5F3Mcz4WoNSwTOK76Z7a7pujX5eoOSKZgLvt7QPMCIA"
			"/hB1c2VyQGV4YW1wbGUuY29t//OjJlv2ra+bT+cE37Aoufn8ThUrIVaCEnb47JRITh3zs72KESap140Jr0vKzAm80NtLEqgQ0IGYH4NkCf5GsQw="), MANAGEDBUF(1024)),
		*org_signet = base64_decode(NULLER("BvAAAWMBIUA84E+XlnQm2rf/xArwWRYoCWRHKfLYnR1epjfGHSLeawMhA48f04SsJOtgZvIAaAMHb0lAU6JGmKWQBYZRdFuIOOtkBHsafjgMKnfEfasVTBpjbnon81tt"
			"nuJNYxIioSBbXMHoouchSKOm9elbyB7W8hva2ONjByYvGI/dgHU3+OgrrAgQC0V4YW1wbGUgTExDERgxNjAwIFBlbm5zeWx2YW5pYSBBdmUgTlcTDVVuaXRlZCBTdGF0"
			"ZXMUBTIwNTAwFQ4oMjAyKSA0NTYtMTExMf107N0qlZxpbixIWPYfWcKrk82Ma6jxFCKsU2Om8p3P7uXRDxXOsKjl+exCMLdWcCQf7Za4t+0qmidb3LsobAAO/gtleGFt"
			"cGxlLmNvbf9w6PnxWe+gVy8HeK2hyyKVMVd57LXbmYlmqsRegg2FAI8su/QqYyPoogxRXKjOaeKExRhPe+L1onefH3JBA0IK"), MANAGEDBUF(1024));

	if (!(object = prime_unpack(user_key))) {
		st_sprint(errmsg, "Failed to unpack the user key.");
		return false;
	}

//	log_unit("Type [%i] = %s\n", object->type, prime_object_type(object->type));
//
//	for (int i = 0; i < object->count; i++) {
//		if (object->fields[i].type < 16 || object->fields[i].type == 253 || object->fields[i].type == 255) {
//			stringer_t *payload_b64 = base64_encode_mod(&(object->fields[i].payload), MANAGEDBUF(1024));
//			log_unit("Field [%i] = %.*s\n", object->fields[i].type, st_length_int(payload_b64), st_char_get(payload_b64));
//		}
//		else {
//			log_unit("Field [%i] = %.*s\n", object->fields[i].type, st_length_int(&(object->fields[i].payload)), st_char_get(&(object->fields[i].payload)));
//		}
//	}

	prime_object_free(object);

	if (!(object = prime_unpack(org_key))) {
		st_sprint(errmsg, "Failed to unpack the organizational key.");
		return false;
	}

//	log_unit("Type [%i] = %s\n", object->type, prime_object_type(object->type));
//
//	for (int i = 0; i < object->count; i++) {
//		if (object->fields[i].type < 16 || object->fields[i].type == 253 || object->fields[i].type == 255) {
//			stringer_t *payload_b64 = base64_encode_mod(&(object->fields[i].payload), MANAGEDBUF(1024));
//			log_unit("Field [%i] = %.*s\n", object->fields[i].type, st_length_int(payload_b64), st_char_get(payload_b64));
//		}
//		else {
//			log_unit("Field [%i] = %.*s\n", object->fields[i].type, st_length_int(&(object->fields[i].payload)), st_char_get(&(object->fields[i].payload)));
//		}
//	}
//
	prime_object_free(object);

	if (!(object = prime_unpack(org_signet))) {
		st_sprint(errmsg, "Failed to unpack the org signet.");
		return false;
	}

//	log_unit("Type [%i] = %s\n", object->type, prime_object_type(object->type));
//
//	for (int i = 0; i < object->count; i++) {
//		if (object->fields[i].type < 16 || object->fields[i].type == 253 || object->fields[i].type == 255) {
//			stringer_t *payload_b64 = base64_encode_mod(&(object->fields[i].payload), MANAGEDBUF(1024));
//			log_unit("Field [%i] = %.*s\n", object->fields[i].type, st_length_int(payload_b64), st_char_get(payload_b64));
//		}
//		else {
//			log_unit("Field [%i] = %.*s\n", object->fields[i].type, st_length_int(&(object->fields[i].payload)), st_char_get(&(object->fields[i].payload)));
//		}
//	}

	prime_object_free(object);

	if (!(object = prime_unpack(user_signing_request))) {
		st_sprint(errmsg, "Failed to unpack the user signing request.");
		return false;
	}

//	log_unit("Type [%i] = %s\n", object->type, prime_object_type(object->type));
//
//	for (int i = 0; i < object->count; i++) {
//		if (object->fields[i].type < 16 || object->fields[i].type == 253 || object->fields[i].type == 255) {
//			stringer_t *payload_b64 = base64_encode_mod(&(object->fields[i].payload), MANAGEDBUF(1024));
//			log_unit("Field [%i] = %.*s\n", object->fields[i].type, st_length_int(payload_b64), st_char_get(payload_b64));
//		}
//		else {
//			log_unit("Field [%i] = %.*s\n", object->fields[i].type, st_length_int(&(object->fields[i].payload)), st_char_get(&(object->fields[i].payload)));
//		}
//	}

	prime_object_free(object);

	if (!(object = prime_unpack(user_signet))) {
		st_sprint(errmsg, "Failed to unpack the user signet.");
		return false;
	}

//	log_unit("Type [%i] = %s\n", object->type, prime_object_type(object->type));
//
//	for (int i = 0; i < object->count; i++) {
//		if (object->fields[i].type < 16 || object->fields[i].type == 253 || object->fields[i].type == 255) {
//			stringer_t *payload_b64 = base64_encode_mod(&(object->fields[i].payload), MANAGEDBUF(1024));
//			log_unit("Field [%i] = %.*s\n", object->fields[i].type, st_length_int(payload_b64), st_char_get(payload_b64));
//		}
//		else {
//			log_unit("Field [%i] = %.*s\n", object->fields[i].type, st_length_int(&(object->fields[i].payload)), st_char_get(&(object->fields[i].payload)));
//		}
//	}

	prime_object_free(object);

	return true;
}
