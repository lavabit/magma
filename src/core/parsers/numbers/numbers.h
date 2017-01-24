
/**
 * @file /magma/core/parsers/numbers/numbers.h
 *
 * @brief	Function declarations for the number conversion functions.
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

/// clamp.c
int16_t    int16_clamp(int16_t min, int16_t max, int16_t number);
int32_t    int32_clamp(int32_t min, int32_t max, int32_t number);
int64_t    int64_clamp(int64_t min, int64_t max, int64_t number);
int8_t     int8_clamp(int8_t min, int8_t max, int8_t number);
uint16_t   uint16_clamp(uint16_t min, uint16_t max, uint16_t number);
uint32_t   uint32_clamp(uint32_t min, uint32_t max, uint32_t number);
uint64_t   uint64_clamp(uint64_t min, uint64_t max, uint64_t number);
uint8_t    uint8_clamp(uint8_t min, uint8_t max, uint8_t number);

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

stringer_t * uint32_put_no(uint32_t val);
stringer_t * uint24_put_no(uint32_t val);
stringer_t * uint16_put_no(uint16_t val);
uint32_t uint32_get_no(stringer_t *s);
uint32_t uint24_get_no(stringer_t *s);
uint16_t uint16_get_no(stringer_t *s);

#endif
