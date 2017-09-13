
/**
 * @file /magma/core/host/host.h
 *
 * @brief	Provide access to system interfaces.
 */

#ifndef MAGMA_CORE_SYSTEM_H
#define MAGMA_CORE_SYSTEM_H

#define MAGMA_COLOR_RESET "\033[m"

#define MAGMA_COLOR_RED "\033[0;31m"
#define MAGMA_COLOR_GREEN "\033[0;32m"
#define MAGMA_COLOR_YELLOW "\033[0;33m"
#define MAGMA_COLOR_BLUE "\033[0;34m"
#define MAGMA_COLOR_PURPLE "\033[0;35m"
#define MAGMA_COLOR_CYAN "\033[0;36m"
#define MAGMA_COLOR_WHITE "\033[0;37m"

#define MAGMA_COLOR_RED_BOLD "\033[1;31m"
#define MAGMA_COLOR_GREEN_BOLD "\033[1;32m"
#define MAGMA_COLOR_YELLOW_BOLD "\033[1;33m"
#define MAGMA_COLOR_BLUE_BOLD "\033[1;34m"
#define MAGMA_COLOR_PURPLE_BOLD "\033[1;35m"
#define MAGMA_COLOR_CYAN_BOLD "\033[1;36m"
#define MAGMA_COLOR_WHITE_BOLD "\033[1;37m"

#define MAGMA_COLOR_RED_UNDERLINE "\033[4;31m"
#define MAGMA_COLOR_GREEN_UNDERLINE "\033[4;32m"
#define MAGMA_COLOR_YELLOW_UNDERLINE "\033[4;93m"
#define MAGMA_COLOR_BLUE_UNDERLINE "\033[4;34m"
#define MAGMA_COLOR_PURPLE_UNDERLINE "\033[4;35m"
#define MAGMA_COLOR_CYAN_UNDERLINE "\033[4;36m"
#define MAGMA_COLOR_WHITE_UNDERLINE "\033[4;37m"

#define MAGMA_COLOR_RED_INTENSE "\033[0;91m"
#define MAGMA_COLOR_GREEN_INTENSE "\033[0;92m"
#define MAGMA_COLOR_YELLOW_INTENSE "\033[0;93m"
#define MAGMA_COLOR_BLUE_INTENSE "\033[0;94m"
#define MAGMA_COLOR_PURPLE_INTENSE "\033[0;95m"
#define MAGMA_COLOR_CYAN_INTENSE "\033[0;96m"
#define MAGMA_COLOR_WHITE_INTENSE "\033[0;97m"

#define MAGMA_COLOR_RED_INTENSE_BOLD "\033[1;91m"
#define MAGMA_COLOR_GREEN_INTENSE_BOLD "\033[1;92m"
#define MAGMA_COLOR_YELLOW_INTENSE_BOLD "\033[1;93m"
#define MAGMA_COLOR_BLUE_INTENSE_BOLD "\033[1;94m"
#define MAGMA_COLOR_PURPLE_INTENSE_BOLD "\033[1;95m"
#define MAGMA_COLOR_CYAN_INTENSE_BOLD "\033[1;96m"
#define MAGMA_COLOR_WHITE_INTENSE_BOLD "\033[1;97m"

#define MAGMA_PROC_PATH "/proc"

/**
 * @typedef octet_t
 */
typedef int16_t octet_t;

/**
 * @typedef segment_t
 */
typedef int32_t segment_t;

/**
 * @typedef ip_t
 */
typedef struct {
	sa_family_t family;
	union {
		struct in_addr ip4;
		struct in6_addr ip6;
		void *ip;
	};
} ip_t;

/**
 * @typedef subnet_t
 */
typedef struct {
	uint32_t mask;
	ip_t address;
} subnet_t;

// The spool_start function uses a for loop to validate the spool directory tree. If additional
// spool locations are enumerated, make sure that function is updated.
enum {
	MAGMA_SPOOL_BASE = 0,
	MAGMA_SPOOL_DATA = 1,
	MAGMA_SPOOL_SCAN = 2
};

/// color.c
const    chr_t * color_blue(void);
const    chr_t * color_blue_bold(void);
const    chr_t * color_blue_intense(void);
const    chr_t * color_blue_intense_bold(void);
const    chr_t * color_blue_underline(void);
const    chr_t * color_cyan(void);
const    chr_t * color_cyan_bold(void);
const    chr_t * color_cyan_intense(void);
const    chr_t * color_cyan_intense_bold(void);
const    chr_t * color_cyan_underline(void);
const    chr_t * color_green(void);
const    chr_t * color_green_bold(void);
const    chr_t * color_green_intense(void);
const    chr_t * color_green_intense_bold(void);
const    chr_t * color_green_underline(void);
const    chr_t * color_purple(void);
const    chr_t * color_purple_bold(void);
const    chr_t * color_purple_intense(void);
const    chr_t * color_purple_intense_bold(void);
const    chr_t * color_purple_underline(void);
const    chr_t * color_red(void);
const    chr_t * color_red_bold(void);
const    chr_t * color_red_intense(void);
const    chr_t * color_red_intense_bold(void);
const    chr_t * color_red_underline(void);
const    chr_t * color_reset(void);
bool_t   color_supported(void);
const    chr_t * color_white(void);
const    chr_t * color_white_bold(void);
const    chr_t * color_white_intense(void);
const    chr_t * color_white_intense_bold(void);
const    chr_t * color_white_underline(void);
const    chr_t * color_yellow(void);
const    chr_t * color_yellow_bold(void);
const    chr_t * color_yellow_intense(void);
const    chr_t * color_yellow_intense_bold(void);
const    chr_t * color_yellow_underline(void);

/// files.c
stringer_t *  file_load(const char *name);
int_t         file_read(const char *name, stringer_t *output);
int_t         file_temp_handle(chr_t *pdir, stringer_t **tmpname);
bool_t        file_accessible(const chr_t *path);
bool_t        file_readwritable(const chr_t *path);
bool_t        file_world_accessible(const chr_t *path);

/// tcp.c
ip_t *        tcp_addr_ip(int sockd, ip_t *output);
stringer_t *  tcp_addr_st(int sockd, stringer_t *output);
int           tcp_continue(int sockd, int result, int syserror);
int_t         tcp_error(int error);
int           tcp_read(int sockd, void *buffer, int length, bool_t block);
int_t         tcp_status(int sockd);
int           tcp_wait(int sockd);
int           tcp_write(int sockd, const void *buffer, int length, bool_t block);

/// host.c
stringer_t *  host_platform(stringer_t *output);
stringer_t *  host_version(stringer_t *output);

/// errors.c
chr_t *  errno_name(int error);

/// signals.c
chr_t *  signal_name(int signal, char *buffer, size_t length);

/// folder.c
int_t   folder_count(stringer_t *path, bool_t recursive, bool_t strict);
int_t   folder_exists(stringer_t *path, bool_t create);

/// process.c
pid_t   process_find_pid(stringer_t *name);
int_t   process_kill(stringer_t *name, int_t signum, int_t wait);
pid_t   process_my_pid(void);

/// spool.c
int_t         spool_check(stringer_t *path);
int_t         spool_check_file(const char *file, const struct stat *info, int type);
int_t         spool_cleanup(void);
uint64_t      spool_error_stats(void);
int_t         spool_mktemp(int_t spool, chr_t *prefix);
stringer_t *  spool_path(int_t spool);
bool_t        spool_start(void);
void          spool_stop(void);

/// ip.c
bool_t        ip_addr_eq(ip_t *ip1, ip_t *ip2);
ip_t *        ip_copy(ip_t *dst, ip_t *src);
int_t         ip_family(ip_t *address);
bool_t        ip_localhost(ip_t *address);
bool_t        ip_matches_subnet(subnet_t *subnet, ip_t *addr);
octet_t       ip_octet(ip_t *address, int_t position);
stringer_t *  ip_presentation(ip_t *address, stringer_t *output);
bool_t        ip_private(ip_t *address);
stringer_t *  ip_reversed(ip_t *address, stringer_t *output);
segment_t     ip_segment(ip_t *address, int_t position);
stringer_t *  ip_standard(ip_t *address, stringer_t *output);
bool_t        ip_addr_st(chr_t *ipstr, ip_t *out);
bool_t        ip_subnet_st(chr_t *substr, subnet_t *out);
stringer_t *  ip_subnet(ip_t *address, stringer_t *output);
int8_t        ip_type(ip_t *address);
uint32_t      ip_word(ip_t *address, int_t position);

#endif

