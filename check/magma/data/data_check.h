
/**
 * @file /check/magma/data/data_check.h
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 */

#ifndef CHECK_DATA_H
#define CHECK_DATA_H

/// data_check.c
bool_t        check_message_dkim_sign(uint32_t index);
bool_t        check_message_dkim_verify(uint32_t index);
stringer_t *  check_message_get(uint32_t index);
uint32_t      check_message_max(void);

#endif

