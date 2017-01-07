
/**
 * @file /magma/src/providers/prime/signets/orgs.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"


// Note that we need 1 extra byte for the 0x04 prefix taken from proposed RFC for adding EdDSA to the OpenPGP schema.
// https://tools.ietf.org/id/draft-koch-eddsa-for-openpgp.txt
void org_signet_free(prime_org_signet_t *org) {

	if (org) {
		if (org->signing) ed25519_free(org->signing);
		if (org->encryption) secp256k1_free(org->encryption);
		mm_free(org);
	}

	return;
}

prime_org_signet_t * org_signet_alloc(void) {

	prime_org_signet_t *org = NULL;

	if (!(org = mm_alloc(sizeof(prime_org_signet_t)))) {
		log_pedantic("PRIME organizational signet allocation failed.");
		return NULL;
	}

	mm_wipe(org, sizeof(prime_org_signet_t));

	return org;
}

