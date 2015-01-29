
/**
 * @file /magma/core/host/host.h
 *
 * @brief	Provide access to system interfaces.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_CORE_SYSTEM_H
#define MAGMA_CORE_SYSTEM_H

#define MAGMA_PROC_PATH "/proc"

// The spool_start function uses a for loop to validate the spool directory tree. If additional spool locations are enumerated, make sure that function is updated.
enum {
	MAGMA_SPOOL_BASE = 0,
	MAGMA_SPOOL_DATA = 1,
	MAGMA_SPOOL_SCAN = 2
};

/// files.c
stringer_t *  file_load(char *name);
int_t         file_read(char *name, stringer_t *output);
int_t         get_temp_file_handle(chr_t *pdir, stringer_t **tmpname);
bool_t        file_accessible(const chr_t *path);
bool_t        file_readwritable(const chr_t *path);
bool_t        file_world_accessible(const chr_t *path);


/// host.c
stringer_t *  host_platform(stringer_t *output);
stringer_t *  host_version(stringer_t *output);

/// mappings.c
chr_t *  errno_name(int errnum, char *buffer, size_t length);
chr_t *  signal_name(int signal, char *buffer, size_t length);

/// folder.c
int_t   folder_exists(stringer_t *path, bool_t create);

/// process.c
int_t   process_kill(stringer_t *name, int_t signal, int_t wait);

/// spool.c
int_t         spool_check(stringer_t *path);
int_t         spool_check_file(const char *file, const struct stat *info, int type);
int_t         spool_cleanup(void);
uint64_t      spool_error_stats(void);
int_t         spool_mktemp(int_t spool, chr_t *prefix);
stringer_t *  spool_path(int_t spool);
bool_t        spool_start(void);
void          spool_stop(void);

#endif

