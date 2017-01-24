
/**
 * @file /magma/core/parsers/parsers.h
 *
 * @brief The function decelerations and types needed by the generic parsing functions.
 */

#ifndef MAGMA_CORE_PARSERS_H
#define MAGMA_CORE_PARSERS_H

typedef struct {
	char token;
	char *block, *position;
	size_t length, remaining;
} tok_state_t;

/// time.c
uint64_t      time_datestamp(void);
stringer_t *  time_print_gmt(stringer_t *s, chr_t *format, time_t moment);
stringer_t *  time_print_local(stringer_t *s, chr_t *format, time_t moment);
uint64_t      time_till_midnight(void);

// Lowercase
uchr_t lower_chr(uchr_t c);
stringer_t * lower_st(stringer_t *s);

// Uppercase
uchr_t upper_chr(uchr_t c);
stringer_t * upper_st(stringer_t *s);

/************  TOKENS  ************/
// Token counting
uint64_t tok_get_count_st(stringer_t *string, char token);
uint64_t tok_get_count_bl(void *block, size_t length, char token);
uint64_t str_tok_get_count_bl(void *block, size_t length, chr_t *token, size_t toklen);

// Token extraction
int tok_get_pl(placer_t string, char token, uint64_t fragment, placer_t *value);
int tok_get_st(stringer_t *string, char token, uint64_t fragment, placer_t *value);
int tok_get_bl(void *block, size_t length, char token, uint64_t fragment, placer_t *value);
int tok_get_ns(char *string, size_t length, char token, uint64_t fragment, placer_t *value);
int str_tok_get_bl(char *block, size_t length, chr_t *token, size_t toklen, uint64_t fragment, placer_t *value);

// Token popping
int tok_pop(tok_state_t *state, placer_t *value);
void tok_pop_init_st(tok_state_t *state, stringer_t *string, char token);
void tok_pop_init_bl(tok_state_t *state, void *block, size_t length, char token);

// Other splitting
bool_t pl_skip_characters (placer_t *place, char *skipchars, size_t nchars);
bool_t pl_skip_to_characters (placer_t *place, char *skiptochars, size_t nchars);
bool_t pl_shrink_before_characters (placer_t *place, char *shrinkchars, size_t nchars);
bool_t pl_get_embraced (placer_t str, placer_t *out, unsigned char opening, unsigned char closing, bool_t required);
bool_t pl_update_start (placer_t *place, size_t nchars, bool_t more);
/************  TOKENS  ************/

/************  LINE  ************/
placer_t line_pl_ns(char *string, uint64_t number);
placer_t line_pl_pl(placer_t string, uint64_t number);
placer_t line_pl_st(stringer_t *string, uint64_t number);
placer_t line_pl_bl(char *block, size_t length, uint64_t number);
/************  LINE  ************/

/// trim.c
placer_t pl_trim(placer_t place);
placer_t pl_trim_end(placer_t place);
placer_t pl_trim_start(placer_t place);
void st_trim(stringer_t *string);

/// bitwise.c
stringer_t * st_xor(stringer_t *a, stringer_t *b, stringer_t *outcome);
stringer_t * st_and(stringer_t *a, stringer_t *b, stringer_t *outcome);
stringer_t * st_or(stringer_t *a, stringer_t *b, stringer_t *outcome);
stringer_t * st_not(stringer_t *s, stringer_t *outcome);


#include "numbers/numbers.h"
#include "formats/formats.h"
#include "special/special.h"

#endif
