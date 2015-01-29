
/**
 * @file /magma/core/classify/classify.h
 *
 * @brief	Functions used to classify characters into specific classes.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_CORE_CLASSIFY_H
#define MAGMA_CORE_CLASSIFY_H

/// ascii.c
bool_t chr_alphanumeric(uchr_t c);
bool_t chr_ascii(uchr_t c);
bool_t chr_blank(uchr_t c);
bool_t chr_lower(uchr_t c);
bool_t chr_numeric(uchr_t c);
bool_t chr_printable(uchr_t c);
bool_t chr_punctuation(uchr_t c);
bool_t chr_upper(uchr_t c);
bool_t chr_whitespace(uchr_t c);
bool_t chr_is_class(uchr_t c, uchr_t *chrs, size_t chrlen);

#endif

