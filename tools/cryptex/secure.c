/**
 * @file /cryptex/secure.c
 *
 * @brief Functions for handling the secure data type.
 *
 * $Author: Ladar Levison $
 * $Website: http://lavabit.com $
 * $Date: 2010/08/11 23:45:52 $
 * $Revision: 63466342d54a6106a3eb4361aece44458a018a49 $
 *
 */

#include "cryptex.h"

uint64_t secure_key_length(secure_t *cryptex) {
	secure_head_t *head = (secure_head_t *)cryptex;
	return head->length.key;
}

uint64_t secure_mac_length(secure_t *cryptex) {
	secure_head_t *head = (secure_head_t *)cryptex;
	return head->length.mac;
}

uint64_t secure_body_length(secure_t *cryptex) {
	secure_head_t *head = (secure_head_t *)cryptex;
	return head->length.body;
}

uint64_t secure_orig_length(secure_t *cryptex) {
	secure_head_t *head = (secure_head_t *)cryptex;
	return head->length.orig;
}

uint64_t secure_total_length(secure_t *cryptex) {
	secure_head_t *head = (secure_head_t *)cryptex;
	return sizeof(secure_head_t) + (head->length.key + head->length.mac + head->length.body);
}

void * secure_key_data(secure_t *cryptex) {
	return (char *)cryptex + sizeof(secure_head_t);
}

void * secure_mac_data(secure_t *cryptex) {
	secure_head_t *head = (secure_head_t *)cryptex;
	return (char *)cryptex + (sizeof(secure_head_t) + head->length.key);
}

void * secure_body_data(secure_t *cryptex) {
	secure_head_t *head = (secure_head_t *)cryptex;
	return (char *)cryptex + (sizeof(secure_head_t) + head->length.key + head->length.mac);
}

void * secure_alloc(uint64_t key, uint64_t mac, uint64_t orig, uint64_t body) {
	secure_t *cryptex = malloc(sizeof(secure_head_t) + key + mac + body);
	secure_head_t *head = (secure_head_t *)cryptex;
	head->length.key = key;
	head->length.mac = mac;
	head->length.orig = orig;
	head->length.body = body;
	return cryptex;
}

void secure_free(secure_t *cryptex) {
	free(cryptex);
	return;
}
