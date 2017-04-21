
/**
 * @file /check/magma/core/core_check.h
 *
 * @brief The entry point for the core module test suite.
 */

#ifndef CORE_CHECK_H
#define CORE_CHECK_H

typedef struct {
	inx_t *inx;
	uint64_t type;
} check_inx_opt_t;

struct check_mi_t {
	int nr;
	char *name;
};

extern stringer_t *string_check_constant;

/// clamp_check.c
chr_t * check_clamp_max(void);
chr_t * check_clamp_min(void);
chr_t * check_clamp_randomizer(void);
chr_t * check_clamp_min_max_equal(void);
chr_t * check_clamp_min_max_invalid(void);

/// string_check.c
bool_t   check_string_alloc(uint32_t check);
bool_t   check_string_dupe(uint32_t check);
bool_t   check_string_import(void);
bool_t   check_string_merge(void);
bool_t   check_string_print(void);
bool_t   check_string_write(void);
bool_t   check_string_realloc(uint32_t check);

/// qp_check.c
bool_t   check_encoding_qp(void);

/// inx_check.c
bool_t    check_inx_cursor_mthread(check_inx_opt_t *opts);
void		  check_inx_cursor_mthread_cnv(check_inx_opt_t *opts);
bool_t    check_inx_cursor_sthread(check_inx_opt_t *opts);
bool_t    check_inx_mthread(check_inx_opt_t *opts);
void		  check_inx_mthread_cnv(check_inx_opt_t *opts);
bool_t    check_inx_sthread(check_inx_opt_t *opts);
bool_t	  check_inx_append_helper(inx_t *);
void	  check_inx_append_test(inx_t *);
bool_t 	  check_inx_append_sthread(MAGMA_INDEX, stringer_t*);
bool_t 	  check_inx_append_mthread(MAGMA_INDEX, stringer_t*);

/// ip_check.c
bool_t check_uint16_to_hex_st(uint16_t val, stringer_t *buff);
bool_t check_ip_private_sthread(stringer_t *errmsg);
bool_t check_ip_localhost_sthread(stringer_t *errmsg);

/// linked_check.c
bool_t   check_indexes_linked_cursor(char **errmsg);
bool_t   check_indexes_linked_cursor_compare(uint64_t values[], inx_cursor_t *cursor);
bool_t   check_indexes_linked_simple(char **errmsg);

/// hex_check.c
bool_t   check_encoding_hex(void);

/// url_check.c
bool_t   check_encoding_url(void);

/// core_check.c
Suite *                    suite_check_core(void);

/// base64_check.c
bool_t   check_encoding_base64(bool_t secure_on);
bool_t   check_encoding_base64_mod(bool_t secure_on);

/// hashed_check.c
bool_t   check_indexes_hashed_cursor(char **errmsg);
bool_t   check_indexes_hashed_cursor_compare(uint64_t values[], inx_cursor_t *cursor);
bool_t   check_indexes_hashed_simple(char **errmsg);

/// system_check.c
bool_t   check_system_errnonames(void);
bool_t   check_system_signames(void);

/// tree_check.c
bool_t   check_indexes_tree_cursor(char **errmsg);
bool_t   check_indexes_tree_cursor_compare(uint64_t values[], inx_cursor_t *cursor);
bool_t   check_indexes_tree_simple(char **errmsg);

/// qsort_check.c
int   check_bsearch_compare(const void *m1, const void *m2);
int   check_bsearch_months(int num, char *name);

/// zbase32_check.c
bool_t   check_encoding_zbase32(void);

/// nbo_check.c
bool_t   check_nbo_simple(void);
bool_t   check_nbo_parameters(void);

/// bitwise_check.c
bool_t   check_bitwise_parameters(void);
bool_t   check_bitwise_determinism(void);
bool_t   check_bitwise_simple(void);

/// checksum_check.c
bool_t check_checksum_fuzz_sthread(void);
bool_t check_checksum_fixed_sthread(void);
bool_t check_checksum_loop_sthread(void);

/// address_check.c
void check_address_octet_s (int _i CK_ATTRIBUTE_UNUSED);
void check_address_presentation_s (int _i CK_ATTRIBUTE_UNUSED);
void check_address_reversed_s (int _i CK_ATTRIBUTE_UNUSED);
void check_address_segment_s (int _i CK_ATTRIBUTE_UNUSED);
void check_address_standard_s (int _i CK_ATTRIBUTE_UNUSED);
void check_address_subnet_s (int _i CK_ATTRIBUTE_UNUSED);
bool_t check_ip_private_scheck(stringer_t *errmsg);
bool_t check_ip_localhost_scheck(stringer_t *errmsg);

#endif
