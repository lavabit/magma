
/**
 * @file /magma/core/parsers/numbers/numbers.h
 *
 * @brief	Function declarations for the number conversion functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_CORE_PARSERS_NUMBERS_H
#define MAGMA_CORE_PARSERS_NUMBERS_H

/// digits.c
size_t int16_digits(int16_t number);
size_t int32_digits(int32_t number);
size_t int64_digits(int64_t number);
size_t int8_digits(int8_t number);
size_t uint16_digits(uint16_t number);
size_t uint32_digits(uint32_t number);
size_t uint64_digits(uint64_t number);
size_t uint8_digits(uint8_t number);

/// numbers.c
bool_t double_conv(stringer_t *s, double_t *number);
bool_t float_conv(stringer_t *s, float_t *number);
bool_t int16_conv_bl(void *block, size_t length, int16_t *number);
bool_t int16_conv_ns(char *string, int16_t *number);
bool_t int16_conv_st(stringer_t *string, int16_t *number);
bool_t int32_conv_bl(void *block, size_t length, int32_t *number);
bool_t int32_conv_ns(char *string, int32_t *number);
bool_t int32_conv_st(stringer_t *string, int32_t *number);
bool_t int64_conv_bl(void *block, size_t length, int64_t *number);
bool_t int64_conv_ns(char *string, int64_t *number);
bool_t int64_conv_st(stringer_t *string, int64_t *number);
bool_t int8_conv_bl(void *block, size_t length, int8_t *number);
bool_t int8_conv_ns(char *string, int8_t *number);
bool_t int8_conv_st(stringer_t *string, int8_t *number);
bool_t size_conv_bl(void *block, size_t length, size_t *number);
bool_t ssize_conv_bl(void *block, size_t length, ssize_t *number);
bool_t uint16_conv_bl(void *block, size_t length, uint16_t *number);
bool_t uint16_conv_ns(char *string, uint16_t *number);
bool_t uint16_conv_st(stringer_t *string, uint16_t *number);
bool_t uint32_conv_bl(void *block, size_t length, uint32_t *number);
bool_t uint32_conv_ns(char *string, uint32_t *number);
bool_t uint32_conv_st(stringer_t *string, uint32_t *number);
bool_t uint64_conv_bl(void *block, size_t length, uint64_t *number);
bool_t uint64_conv_ns(char *string, uint64_t *number);
bool_t uint64_conv_pl(placer_t string, uint64_t *number);
bool_t uint64_conv_st(stringer_t *string, uint64_t *number);
bool_t uint8_conv_bl(void *block, size_t length, uint8_t *number);
bool_t uint8_conv_ns(char *string, uint8_t *number);
bool_t uint8_conv_st(stringer_t *string, uint8_t *number);

#endif
