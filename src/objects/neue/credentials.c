
/**
 * @file /magma/objects/neue/credentials.c
 *
 * @brief Functions used for track and update the object checkpoints.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

static int_t credential_calc_legacy(credential_t *cred, stringer_t *password);

static int_t credential_calc_stacie(credential_t *cred, stringer_t *password, stringer_t *salt);


/// LOW: Add a function for detecting potentially illegal username/address sequences. Valid usernames must start with an alpha character,
/// end with an alphanumeric character and not user consecutive underscores. If present, the domain portion of the username must follow the
/// applicable standard for the TLD being used.

/// LOW: Figure out how to result a credential_address into a credential_username using the Mailboxes table.


/**
 * @brief	Process a user supplied credential address to ensures it only contains valid characters.
 * @param	s	a managed string containing the user's credential address to be processed, with an optional domain suffix.
 * @note	This function duplicates the input address string, with all characters converted to lowercase, and whitespace removed.
 * 			'.' and '-' are also converted to '_' in the username, and if there is a '+' in the username portion of the
 * 			credential address, all subsequent characters in that username will be ignored.
 * @return	NULL on failure, or a managed string containing the validated credential address on success.
 */
stringer_t * credential_address(stringer_t *s) {

	size_t len;
	chr_t *p, *w, *tip = NULL;
	stringer_t *output, *handle, *domain = NULL;

	if (st_empty_out(s, (uchr_t **)&p, &len) || !(output = st_alloc_opts(MANAGED_T | CONTIGUOUS | SECURE, len + 1))) {
		return NULL;
	}

	w = st_char_get(output);

	for (size_t i = 0; i < len; i++) {

		// If an @ symbol is encountered, record its position
		if (!tip && *p == '@') {
			tip = w;
		}

		// Non white space characters are copied in lower case form
		if (!chr_whitespace(*p)) {
			*w++ = lower_chr(*p++);
		}

		// Just advance past white space
		else {
			p++;
		}

	}

	st_length_set(output, w - st_char_get(output));

	// If an @ symbol was encountered, save the handle and domain portions separately since they use different rules.
	if (tip) {
		handle = PLACER(st_char_get(output), tip - st_char_get(output));
		domain = PLACER(tip, st_length_get(output) - st_length_get(handle));
	}
	else {
		handle = PLACER(st_char_get(output), st_length_get(output));
	}

	p = st_char_get(handle);
	len = st_length_get(handle);
	tip = NULL;

	for (size_t i = 0; !tip && i < len; i++) {

		// Save the location of a plus sign (+) so it can be be stripped from the handle section.
		if (*p == '+') {
			tip = p;
		}
		// Translate periods and dashes to underscores allowing them to be used interchanged if the user desires.
		else if (*p == '.' || *p == '-') {
			*p++ = '_';
		}
		else {
			p++;
		}

	}

	// If a plus sign was found in the handle, trim the handle and if necessary shift the domain portion to accommodate.
	if (tip && domain) {
		w = st_char_get(domain);
		len = st_length_get(domain);

		for (size_t i = 0; i < len; i++) {
			*p++ = *w++;
		}

		st_length_set(output, (size_t)(p - st_char_get(output)));
	}
	else if (tip) {
		st_length_set(output, (size_t)(p - st_char_get(output)));
	}

	return output;
}

/**
 * @brief	Get the valid credential username portion of a fully qualified user address.
 * @param	s	a managed string containing the user's credential address to be processed, with an optional domain suffix.
 * @return	NULL on failure, or a managed string containing the credential username portion of the supplied address.
 */
stringer_t * credential_username(stringer_t *s) {

	size_t at;
	stringer_t *output, *domain = NULL;

	if (!(output = credential_address(s))) {
		return NULL;
	}

	// If an @ symbol was encountered, check the domain portion to see if it matches magma.system.domain and if so strip it off.
	if (st_search_cs(output, PLACER("@", 1), &at) && (st_length_get(output) - at) > 1) {
		domain = PLACER(st_char_get(output) + at + 1, st_length_get(output) - at - 1);
		if (!st_cmp_cs_eq(domain, magma.system.domain)) {
			st_length_set(output, at);
		}
	}

	return output;
}

/**
 * @brief	Free a user credential.
 * @param	cred	a pointer to the user credential object to be freed.
 * @return	This function returns no value.
 */
void credential_free(credential_t *cred) {

	if (cred) {

		if (cred->type == CREDENTIAL_AUTH) {
			st_cleanup(cred->auth.username);
			st_cleanup(cred->auth.domain);
			st_cleanup(cred->auth.password);
			st_cleanup(cred->auth.key);
		}

		else if (cred->type == CREDENTIAL_MAIL) {
			st_cleanup(cred->mail.address);
			st_cleanup(cred->mail.domain);
		}

		mm_free(cred);
	}

	return;
}

/**
 * @brief	Get a user mail credential for anonymous use within the mail subsystem.
 * @note	This function is used by the smtp service to fetch the smtp inbound preferences of a recipient user.
 * @param	address		a managed string containing the email address of the user account.
 * @return	NULL on failure, or a pointer to the requested user's mail credentials on success.
 */
credential_t * credential_alloc_mail(stringer_t *address) {

	size_t at;
	credential_t *result = NULL;

	if (!(result = mm_alloc(sizeof(credential_t)))) {
		return NULL;
	}

	result->authentication = NONE;

	// Boil the address
	if ((result->type = CREDENTIAL_MAIL) != CREDENTIAL_MAIL || !(result->mail.address = credential_address(address)) ||
		// QUESTION: What is mail.domain being used for?
		!(result->mail.domain = st_alloc_opts(PLACER_T | JOINTED | SECURE | FOREIGNDATA, 0))) {
		credential_free(result);
		return NULL;
	}

	// If applicable, track the domain portion of the username
	else if (st_search_cs(result->mail.address, PLACER("@", 1), &at) && at != 0 && (st_length_get(result->mail.address) - at) > 2) {
		st_data_set(result->auth.domain, st_data_get(result->auth.username) + at + 1);
		st_length_set(result->auth.domain, st_length_get(result->auth.username) - at - 1);
	}

	// Reject addresses without both a mailbox and domain by returning NULL.
	else {
		credential_free(result);
		return NULL;
	}

	return result;
}

/**
 * @brief	Construct a user credential object from supplied username.
 * @note	The credential's auth.key field becomes a single pass hash of the password, while auth.password is created from a three-time hash.
 * @param	username	the input username.
 * @return	NULL on failure, or a pointer to the requested user's auth credentials on success.
 */
credential_t * credential_alloc_auth(stringer_t *username) {

	credential_t *cred;
	size_t at_offset;
	stringer_t *sanitized;

	if(st_empty(username)) {
		log_pedantic("NULL or empty username passed.");
		goto error;
	}

	if(!(cred = mm_alloc(sizeof(credential_t)))) {
		log_error("Failed to allocate memory for credentials object.");
		goto error;
	}

	if(!(sanitized = credential_address(username))) {
		log_error("Failed to sanitize username.");
		goto cleanup_cred;
	}

	cred->type = CREDENTIAL_AUTH;
	cred->auth.username = credential_username(sanitized);
	st_free(sanitized);

	if(!cred->auth.username) {
		log_error("Failed to boil the username from address.");
		goto cleanup_cred;
	}

	if(st_search_cs(cred->auth.username, PLACER("@", 1), &at_offset) && (st_length_get(cred->auth.username) - at_offset) > 1) {

		if(!(cred->auth.domain = st_alloc_opts((MANAGED_T | JOINTED | SECURE), st_length_get(cred->auth.username) - (at_offset + 1)))) {
			log_error("Failed to allocate email domain stringer.");
			goto cleanup_cred;
		}

		st_copy_in(cred->auth.domain, st_data_get(cred->auth.username) + at_offset + 1, st_length_get(cred->auth.username) - (at_offset + 1));
	}

	if (!(cred->auth.domain = st_merge_opts((MANAGED_T | JOINTED | SECURE), "s", magma.system.domain))) {
		log_error("Failed to create default magma domain stringer.");
		goto cleanup_cred;
	}

	return cred;

cleanup_cred:
	credential_free(cred);
error:
	return NULL;
}


/**
 * @brief	Generates a new salt value.
 * @return	Stringer containing a newly generated salt.
 */
stringer_t *    credential_salt_generate(void) {

	size_t salt_len;
	stringer_t *result;

/// FIXME TODO: We need a configuration line in our config file that specifies our server's salt length.

	salt_len = 128;

	if(!(result = st_alloc_opts((MANAGED_T | CONTIGUOUS | SECURE), salt_len))) {
		log_error("Failed to allocate secure stringer for user salt.");
		goto error;
	}

	if(salt_len != rand_write(result)) {
		log_error("Failed to write random bytes into user salt stringer.");
		goto cleanup_result;
	}

	return result;

cleanup_result:
	st_free(result);
error:
	return NULL;
}


/**
 * @brief	Initializes an already allocated credential objects with appropriate values for the specified inputs.
 * @param	cred		Newly allocated credential_t object to be initialized.
 * @param	password	Stringer containing password.
 * @param	salt		Stringer containing salt, or NULL if no salt was available.
 * @return	1 on success, 0 on failure.
 */
int_t credential_calc_auth(credential_t *cred, stringer_t *password, stringer_t *salt) {

	int_t result;

	if(!cred) {
		log_pedantic("NULL credential object was passed in.");
		goto error;
	}

	if(st_empty(password)) {
		log_pedantic("NULL or empty password was passed in.");
		goto error;
	}

	if(cred->type != CREDENTIAL_AUTH) {
		log_error("Invalid credential type object was passed in.");
		goto error;
	}

	if(st_empty(cred->auth.username)) {
		log_error("Credential object has NULL or empty username field.");
		goto error;
	}

	if(st_empty(cred->auth.domain)) {
		log_error("Credential object has NULL or empty domain field.");
		goto error;
	}

	if(salt == NULL) {
		result = credential_calc_legacy(cred, password);
	}
	else {
		result = credential_calc_stacie(cred, password, salt);
	}

	return result;

error:
	return 0;
}

/**
 * @brief	Initializes the provided credential object with values according to legacy authorization mechanic.
 * @param	cred		Credential_t object to be populated.
 * @param	password	Stringer containing password.
 * @return	1 on success, 0 on failure.
 */
static int_t credential_calc_legacy(credential_t *cred, stringer_t *password) {

	stringer_t *binary, *combo;

	cred->authentication = LEGACY;

	if(!(binary = st_alloc_opts((BLOCK_T | CONTIGUOUS | SECURE), 64))) {
		log_error("Failed to allocate block for binary data.");
		goto error;
	}

	if(!(combo = st_merge_opts((MANAGED_T | CONTIGUOUS | SECURE), "sss", cred->auth.username, magma.secure.salt, password))) {
		log_error("Failed to merge authentication information for hashing key.");
		goto cleanup_binary;
	}

	binary = hash_sha512(combo, binary);
	st_free(combo);

	if(!binary) {
		log_error("Failed to hash authentication information.");
		goto cleanup_binary;
	}

	if(!(cred->auth.key = hex_encode_opts(binary, (MANAGED_T | CONTIGUOUS | SECURE)))) {
		log_error("Failed to encode authentication key");
		goto cleanup_binary;
	}

	if(!(combo = st_merge_opts((MANAGED_T | CONTIGUOUS | SECURE), "ss", password, binary))) {
		log_error("Failed to merge authentication information for first round password hashing.");
		goto cleanup_binary;
	}

	binary = hash_sha512(combo, binary);
	st_free(combo);

	if(!binary) {
		log_error("Failed first round of password hashing.");
		goto cleanup_binary;
	}

	if(!(combo = st_merge_opts((MANAGED_T | CONTIGUOUS | SECURE), "ss", password, binary))) {
		log_error("Failed to merge authentication information for second round password hashing.");
		goto cleanup_binary;
	}

	binary = hash_sha512(combo, binary);
	st_free(combo);

	if(!binary) {
		log_error("Failed second round of password hashing.");
		goto cleanup_binary;
	}

	if(!(cred->auth.password = hex_encode_opts(binary, (MANAGED_T | CONTIGUOUS | SECURE)))) {
		log_error("Failed to encode password hash.");
		goto cleanup_binary;
	}

	st_free(binary);

	return 1;

cleanup_binary:
	st_free(binary);
error:
	return 0;
}

/**
 * @brief	Initializes the provided credential object with values according to STACIE spec.
 * @param	cred		Credential_t object to be populated.
 * @param	password	Stringer containing password.
 * @param	salt		User specific stacie salt.
 * @return	1 on success, 0 on failure.
 */
static int_t credential_calc_stacie(credential_t *cred, stringer_t *password, stringer_t *salt) {

	uint_t rounds;
	stringer_t *seed, *passkey, *temp;

	cred->authentication = STACIE;

	//FIXME TODO: ADD magma.secure.bonus value to magma config file and make sure it gets imported correctly.
	if(!(rounds = stacie_rounds_calculate(password, 0 /* should be magma.secure.bonus */ ))) {
		log_error("Failed to calculate STACIE rounds number.");
		goto error;
	}

	if(!(seed = stacie_entropy_seed_derive(rounds, password, salt))) {
		log_error("Failed STACIE entropy seed extraction.");
		goto error;
	}

	temp = stacie_hashed_key_derive(seed, rounds, cred->auth.username, password, salt);
	st_free(seed);

	if(!temp) {
		log_error("Failed STACIE master key derivation.");
		goto error;
	}

	if(!(cred->auth.key = hex_encode_opts(temp, (MANAGED_T | CONTIGUOUS | SECURE)))) {
		log_error("Failed to hex encode STACIE master key.");
		goto cleanup_temp;
	}

	passkey = stacie_hashed_key_derive(temp, rounds, cred->auth.username, password, salt);
	st_free(temp);

	if(!passkey) {
		log_error("Failed STACIE password key derivation.");
		goto error;
	}

	temp = stacie_hashed_token_derive(passkey, cred->auth.username, salt, NULL);
	st_free(passkey);

	if(!temp) {
		log_error("Failed STACIE verification token derivation.");
		goto error;
	}

	cred->auth.password = hex_encode_opts(temp, (MANAGED_T | CONTIGUOUS | SECURE));
	st_free(temp);

	if(!cred->auth.password) {
		log_error("Failed to hex encode STACIE verification token.");
		goto error;
	}

	return 1;

cleanup_temp:
	st_free(temp);
error:
	return 0;
}

