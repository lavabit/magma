
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

	prime_t *holder = NULL;
	stringer_t *packed = NULL, *key = MANAGEDBUF(64);

	// Create a STACIE realm key.
	rand_write(key);

	// Allocate an org key.
	if (!(holder = prime_alloc(PRIME_ORG_KEY, NONE))) {
		st_sprint(errmsg, "Organizational key allocation failed.");
		return false;
	}

	prime_free(holder);

	// Generate an org key.
	if (!(holder = prime_key_generate(PRIME_ORG_KEY))) {
		st_sprint(errmsg, "Organizational key generation failed.");
		return false;
	}

	// Serialize the org key.
	else if (!(packed = prime_get(holder, BINARY, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "Organizational key serialization failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Unpack the serialized org key.
	if (!(holder = prime_set(packed, BINARY, NONE))) {
		st_sprint(errmsg, "Organizational key parsing failed.");
		return false;
	}

	// Encrypt the org key.
	else if (!(packed = prime_key_encrypt(key, holder, BINARY, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "Organizational key encryption failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Decrypt the org key.
	if (!(holder = prime_key_decrypt(key, packed, BINARY, NONE))) {
		st_sprint(errmsg, "Encrypted organizational key parsing failed.");
		return false;
	}

	prime_free(holder);

	// Perform the same checks, but this time make the functions
	// allocate memory for the output. Generate an org key.
	if (!(holder = prime_key_generate(PRIME_ORG_KEY))) {
		st_sprint(errmsg, "Organizational key generation failed.");
		return false;
	}

	// Serialize the org key.
	else if (!(packed = prime_get(holder, BINARY, NULL))) {
		st_sprint(errmsg, "Organizational key serialization failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Unpack the serialized org key.
	if (!(holder = prime_set(packed, BINARY, NONE))) {
		st_sprint(errmsg, "Organizational key parsing failed.");
		st_free(packed);
		return false;
	}

	st_free(packed);

	// Encrypt the org key.
	if (!(packed = prime_key_encrypt(key, holder, BINARY, NULL))) {
		st_sprint(errmsg, "Organizational key encryption failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Decrypt the org key.
	if (!(holder = prime_key_decrypt(key, packed, BINARY, NONE))) {
		st_sprint(errmsg, "Encrypted organizational key parsing failed.");
		st_free(packed);
		return false;
	}

	prime_free(holder);
	st_free(packed);

	return true;
}

bool_t check_prime_user_keys_sthread(stringer_t *errmsg) {

	prime_t *holder = NULL;
	stringer_t *packed = NULL, *key = MANAGEDBUF(64);

	// Create a STACIE realm key.
	rand_write(key);

	// Allocate a user key.
	if (!(holder = prime_alloc(PRIME_USER_KEY, NONE))) {
		st_sprint(errmsg, "User key allocation failed.");
		return false;
	}

	prime_free(holder);

	// Generate a user key.
	if (!(holder = prime_key_generate(PRIME_USER_KEY))) {
		st_sprint(errmsg, "User key generation failed.");
		return false;
	}

	// Serialize the user key.
	else if (!(packed = prime_get(holder, BINARY, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "User key serialization failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Unpack the serialized user key.
	if (!(holder = prime_set(packed, BINARY, NONE))) {
		st_sprint(errmsg, "User key parsing failed.");
		return false;
	}

	// Encrypt the user key.
	else if (!(packed = prime_key_encrypt(key, holder, BINARY, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "User key encryption failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Decrypt the user key.
	if (!(holder = prime_key_decrypt(key, packed, BINARY, NONE))) {
		st_sprint(errmsg, "Encrypted user key parsing failed.");
		return false;
	}

	prime_free(holder);

	// Perform the same checks, but this time make the functions
	// allocate memory for the output. Generate a new user key.
	if (!(holder = prime_key_generate(PRIME_USER_KEY))) {
		st_sprint(errmsg, "User key generation failed.");
		return false;
	}

	// Serialize the user key.
	else if (!(packed = prime_get(holder, BINARY, NULL))) {
		st_sprint(errmsg, "User key serialization failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Unpack the serialized user key.
	if (!(holder = prime_set(packed, BINARY, NONE))) {
		st_sprint(errmsg, "User key parsing failed.");
		st_free(packed);
		return false;
	}

	st_free(packed);

	// Encrypt the user key.
	if (!(packed = prime_key_encrypt(key, holder, BINARY, NULL))) {
		st_sprint(errmsg, "User key encryption failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Decrypt the user key.
	if (!(holder = prime_key_decrypt(key, packed, BINARY, NONE))) {
		st_sprint(errmsg, "Encrypted user key parsing failed.");
		st_free(packed);
		return false;
	}

	prime_free(holder);
	st_free(packed);

	return true;
}

bool_t check_prime_parameters_sthread(stringer_t *errmsg) {

	prime_t *holder = NULL;

	// Attempt allocation of a non-key type using the key allocation function.
	if ((holder = prime_alloc(PRIME_ORG_KEY_ENCRYPTED, NONE)) ||
		(holder = prime_alloc(PRIME_USER_KEY_ENCRYPTED, NONE)) ||
		(holder = prime_alloc(PRIME_MESSAGE_ENCRYPTED, NONE))) {
		st_sprint(errmsg, "Allocation parameter checks failed.");
		prime_free(holder);
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

bool_t check_prime_armor_sthread(stringer_t *errmsg) {

	prime_t *object1 = NULL, *object2 = NULL, *object3 = NULL;
	stringer_t *pem1 = NULL, *pem2 = NULL, *pem_encrypted1 = NULL, *pem_encrypted2 = NULL,
		*object_encrypted1 = NULL, *object_encrypted2 = NULL, *object_encrypted3 = NULL,
		*binary1 = NULL, *binary2 = NULL, *binary3 = NULL,
		*protect = MANAGEDBUF(64);

	// Create a random STACIE realm key.
	rand_write(protect);

	// Generate an org key.
	if (!(object1 = prime_key_generate(PRIME_ORG_KEY)) ||
		!(binary1 = prime_get(object1, BINARY, MANAGEDBUF(256))) ||
		!(pem1 = prime_get(object1, ARMORED, MANAGEDBUF(512))) ||
		!(pem2 = prime_pem_wrap(binary1, MANAGEDBUF(512))) ||
		!(binary2 = prime_pem_unwrap(pem1, MANAGEDBUF(512))) ||
		!(object2 = prime_set(pem2, ARMORED, NONE)) ||
		!(binary3 = prime_get(object2, BINARY, MANAGEDBUF(256))) ||
		st_cmp_cs_eq(pem1, pem2) ||
		st_cmp_cs_eq(binary1, binary2) ||
		st_cmp_cs_eq(binary1, binary3)) {

		st_sprint(errmsg, "An error occurred while trying to armor an organizational key.");
		prime_cleanup(object1);
		prime_cleanup(object2);
		return false;
	}

	prime_free(object2);
	object2 = NULL;

	// Encrypt/decrypt armor/dearmor an org key.
	if (!(object_encrypted1 = prime_key_encrypt(protect, object1, BINARY, MANAGEDBUF(512))) ||
		!(pem_encrypted1 = prime_pem_wrap(object_encrypted1, MANAGEDBUF(512))) ||
		!(object_encrypted2 =  prime_pem_unwrap(pem_encrypted1, MANAGEDBUF(512))) ||
		!(pem_encrypted2 = prime_key_encrypt(protect, object1, ARMORED, MANAGEDBUF(512))) ||
		!(object2 = prime_key_decrypt(protect, pem_encrypted2, ARMORED, NONE)) ||
		!(binary2 = prime_get(object2, BINARY, MANAGEDBUF(256))) ||
		!(object_encrypted3 =  prime_pem_unwrap(pem_encrypted2, MANAGEDBUF(512))) ||
		!(object3 = prime_key_decrypt(protect, object_encrypted3, BINARY, NONE)) ||
		!(binary3 = prime_get(object3, BINARY, MANAGEDBUF(256))) ||
		st_cmp_cs_eq(object_encrypted1, object_encrypted2) ||
		st_cmp_cs_eq(binary1, binary2) ||
		st_cmp_cs_eq(binary1, binary3)) {

		st_sprint(errmsg, "An error occurred while trying to armor an encrypted organizational key.");
		prime_free(object1);
		prime_cleanup(object2);
		prime_cleanup(object3);
		return false;
	}

	prime_free(object1);
	prime_free(object2);
	prime_free(object3);

	object1 = NULL;
	object2 = NULL;
	object3 = NULL;

//	log_unit("%.*s", st_length_int(pem1), st_char_get(pem1));
//	log_unit("%.*s", st_length_int(pem_encrypted1), st_char_get(pem_encrypted1));

	// Generate a user key.
	if (!(object1 = prime_key_generate(PRIME_USER_KEY)) ||
		!(binary1 = prime_get(object1, BINARY, MANAGEDBUF(256))) ||
		!(pem1 = prime_get(object1, ARMORED, MANAGEDBUF(512))) ||
		!(pem2 = prime_pem_wrap(binary1, MANAGEDBUF(512))) ||
		!(binary2 = prime_pem_unwrap(pem1, MANAGEDBUF(512))) ||
		!(object2 = prime_set(pem2, ARMORED, NONE)) ||
		!(binary3 = prime_get(object2, BINARY, MANAGEDBUF(256))) ||
		st_cmp_cs_eq(pem1, pem2) ||
		st_cmp_cs_eq(binary1, binary2) ||
		st_cmp_cs_eq(binary1, binary3)) {

		st_sprint(errmsg, "An error occurred while trying to armor a user key.");
		prime_cleanup(object1);
		prime_cleanup(object2);
		return false;
	}

	prime_free(object2);
	object2 = NULL;

	// Encrypt/decrypt armor/dearmor a user key.
	if (!(object_encrypted1 = prime_key_encrypt(protect, object1, BINARY, MANAGEDBUF(512))) ||
		!(pem_encrypted1 = prime_pem_wrap(object_encrypted1, MANAGEDBUF(512))) ||
		!(object_encrypted2 =  prime_pem_unwrap(pem_encrypted1, MANAGEDBUF(512))) ||
		!(pem_encrypted2 = prime_key_encrypt(protect, object1, ARMORED, MANAGEDBUF(512))) ||
		!(object2 = prime_key_decrypt(protect, pem_encrypted2, ARMORED, NONE)) ||
		!(binary2 = prime_get(object2, BINARY, MANAGEDBUF(256))) ||
		!(object_encrypted3 =  prime_pem_unwrap(pem_encrypted2, MANAGEDBUF(512))) ||
		!(object3 = prime_key_decrypt(protect, object_encrypted3, BINARY, NONE)) ||
		!(binary3 = prime_get(object3, BINARY, MANAGEDBUF(256))) ||
		st_cmp_cs_eq(object_encrypted1, object_encrypted2) ||
		st_cmp_cs_eq(binary1, binary2) ||
		st_cmp_cs_eq(binary1, binary3)) {

		st_sprint(errmsg, "An error occurred while trying to armor an encrypted user key.");
		prime_free(object1);
		prime_cleanup(object2);
		prime_cleanup(object3);
		return false;
	}

	prime_free(object1);
	prime_free(object2);
	prime_free(object3);

	object1 = NULL;
	object2 = NULL;
	object3 = NULL;

//	log_unit("%.*s", st_length_int(pem1), st_char_get(pem1));
//	log_unit("%.*s", st_length_int(pem_encrypted1), st_char_get(pem_encrypted1));

	return true;
}
