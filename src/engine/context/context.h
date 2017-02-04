
/**
 * @file /magma/engine/context/context.h
 *
 * @brief	Functions involved in initializing, manipulating and verifying the operating context for the system.
 */

#ifndef MAGMA_ENGINE_CONTEXT_H
#define MAGMA_ENGINE_CONTEXT_H

/// args.c
void	display_usage(void);
bool_t	args_parse(int argc, char *argv[]);

/************  SANITY  ************/
bool_t sanity_check(void);
/************  SANITY  ************/

/************ STARTUP/SHUTDOWN ************/
void process_stop(void);
bool_t process_start(void);
/************ STARTUP/SHUTDOWN ************/

/// signal.c
bool_t signal_start(void);
bool_t signal_thread_start(void);
void signal_segfault(int signal);
void signal_shutdown(int signal);
void signal_status(int signal);

/// system.c
bool_t     system_change_root_directory(void);
bool_t     system_fork_daemon(void);
bool_t     system_init_core_dumps(void);
bool_t     system_init_impersonation(void);
bool_t     system_init_resource_limits(void);
bool_t     system_init_umask(void);
uint64_t   system_oslimit_max(int_t control, int_t resource);
uint64_t   system_ulimit_cur(int_t resource);
uint64_t   system_ulimit_max(int_t resource);

/// thread.c
bool_t thread_start(void);
void thread_stop(void);

#endif
