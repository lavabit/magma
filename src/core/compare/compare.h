
/**
 * @file /magma/core/compare/compare.h
 *
 * @brief	Function declarations for various data and string comparison functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_CORE_COMPARE_H
#define MAGMA_CORE_COMPARE_H

/// ends.c
int_t st_cmp_ci_ends(stringer_t *s, stringer_t *ends);
int_t st_cmp_cs_ends(stringer_t *s, stringer_t *ends);

/// equal.c
int_t mm_cmp_ci_eq(void *a, void *b, size_t len);
int_t mm_cmp_cs_eq(void *a, void *b, size_t len);
int_t st_cmp_ci_eq(stringer_t *a, stringer_t *b);
int_t st_cmp_cs_eq(stringer_t *a, stringer_t *b);

/// search.c
bool_t st_search_ci(stringer_t *haystack, stringer_t *needle, size_t *location);
bool_t st_search_cs(stringer_t *haystack, stringer_t *needle, size_t *location);
bool_t st_search_chr(stringer_t *haystack, chr_t needle, size_t *location);

/// starts.c
int_t st_cmp_ci_starts(stringer_t *s, stringer_t *starts);
int_t st_cmp_cs_starts(stringer_t *s, stringer_t *starts);

#endif
