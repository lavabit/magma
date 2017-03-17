
/**
 * @file /magma/providers/checkers/checkers.h
 *
 * @brief Functions used to scan, analyze, check, and validate data.
 */

#ifndef MAGMA_PROVIDERS_CHECKERS_H
#define MAGMA_PROVIDERS_CHECKERS_H

#define IP_RANDOMIZER_POOL 1024

#define IP_RANDOMIZER_PUSH_MIN 4
#define IP_RANDOMIZER_PUSH_MAX 16

enum {
	UNALLOCATED = 0,
	ALLOCATED = 1,
	RESERVED = 2
};

enum {
	REGISTRY = 1,
	DIRECT = 2
};

enum {
	DOMESTIC = 0,
	FOREIGN = 1
};

/// clamav.c
bool_t lib_load_clamav(void);
bool_t virus_start(void);
const char * lib_version_clamav(void);
int virus_engine_refresh(void);
int virus_check(stringer_t *data);
struct cl_engine * virus_engine_create(uint64_t *signatures);
uint64_t virus_sigs_loaded(void);
uint64_t virus_sigs_total(void);
void virus_engine_destroy(struct cl_engine **target);
void virus_stop(void);

/// dkim.c
int_t           dkim_signature_verify(stringer_t *id, stringer_t *message);
stringer_t *    dkim_signature_create(stringer_t *id, stringer_t *message);
void *          dkim_memory_alloc(void *closure, size_t nbytes);
void            dkim_memory_free(void *closure, void *ptr);
bool_t          dkim_start(void);
void            dkim_stop(void);
bool_t          lib_load_dkim(void);
const           chr_t * lib_version_dkim(void);

/// dspam.c
int_t    dspam_check(uint64_t usernum, stringer_t *message, stringer_t **signature);
bool_t   dspam_start(void);
void     dspam_stop(void);
bool_t   dspam_train(uint64_t usernum, int_t disposition, stringer_t *signature);
bool_t   lib_load_dspam(void);
chr_t *  lib_version_dspam(void);

/// spf.c
bool_t lib_load_spf(void);
const chr_t * lib_version_spf(void);
bool_t spf_start(void);
int_t spf_check(void *ip, stringer_t *helo, stringer_t *mailfrom);
void spf_stop(void);

#endif
