
/**
 * @file /magma/engine/status/status.h
 *
 * @brief	Functions involved with tracking system status and performance.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_ENGINE_STATUS_H
#define MAGMA_ENGINE_STATUS_H

/************  BUILD  ************/
const char * build_stamp(void);
const char * build_version(void);
/************  BUILD  ************/

/************  PERFORMANCE  ************/
uint64_t perf_rdtsc(void);
/************  PERFORMANCE  ************/

/// status.c
bool_t     status(void);
int        status_get(void);
uint64_t   status_pid(void);
void       status_process(void);
void       status_set(int value);
uint64_t   status_startup(void);

/************  STATISTICS  ************/
uint64_t derived_count (void);
char *derived_name (uint64_t position);
uint64_t derived_value (uint64_t position);

bool_t stats_init(void);
void stats_shutdown(void);

uint64_t stats_get_count(void);
char * stats_get_name(uint64_t position);

uint64_t stats_get_value_by_name(char *name);
uint64_t stats_get_value_by_num(uint64_t position);

void stats_decrement_by_name(char *name);
void stats_decrement_by_num(uint64_t position);

void stats_increment_by_name(char *name);
void stats_increment_by_num(uint64_t position);

void stats_set_by_name(char *name, uint64_t value);
void stats_set_by_num(uint64_t position, uint64_t value);

void stats_adjust_by_name(char *name, int32_t value);
void stats_adjust_by_num(uint64_t position, int32_t value);
/************  STATISTICS  ************/

stringer_t *  host_platform(stringer_t *output);
stringer_t *  host_version(stringer_t *output);

#endif
