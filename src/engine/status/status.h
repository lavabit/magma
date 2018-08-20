
/**
 * @file /magma/engine/status/status.h
 *
 * @brief	Functions involved with tracking system status and performance.
 */

#ifndef MAGMA_ENGINE_STATUS_H
#define MAGMA_ENGINE_STATUS_H

/// statistics.c
void       stats_adjust_by_name(char *name, int32_t value);
void       stats_adjust_by_num(uint64_t position, int32_t value);
void       stats_decrement_by_name(char *name);
void       stats_decrement_by_num(uint64_t position);
uint64_t   stats_derived_count(void);
char *     stats_derived_name(uint64_t position);
uint64_t   stats_derived_value(uint64_t position);
uint64_t   stats_get_count(void);
char *     stats_get_name(uint64_t position);
uint64_t   stats_get_name_pos(char *name);
uint64_t   stats_get_value_by_name(char *name);
uint64_t   stats_get_value_by_num(uint64_t position);
void       stats_increment_by_name(char *name);
void       stats_increment_by_num(uint64_t position);
bool_t     stats_init(void);
void       stats_set_by_name(char *name, uint64_t value);
void       stats_set_by_num(uint64_t position, uint64_t value);
void       stats_shutdown(void);
uint64_t   stats_sum_errors(void);

/// performance.c
uint64_t   perf_rdtsc(void);

/// build.c
const      char * build_commit(void);
const      char * build_stamp(void);
const      char * build_version(void);
uint64_t   build_version_major(void);
uint64_t   build_version_minor(void);
uint64_t   build_version_patch(void);

/// status.c
bool_t     status(void);
int        status_get(void);
uint64_t   status_pid(void);
void       status_process(void);
void       status_set(int value);
void       status_signal(void);
uint64_t   status_startup(void);

#endif
