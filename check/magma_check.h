
/**
 * @file /check/magma_check.h
 *
 * @brief The entry point for accessing the modules involved with executing unit tests.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_CHECK_H
#define MAGMA_CHECK_H

#include <magma.h>

#include <ctype.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <check.h>
#include <valgrind/valgrind.h>

// Normally the START_TEST macro creates a static testcase function. Unfortunately we can't
// find those symbols using dlsym() so we can't dynamically select individual test cases at
// runtime. This redefines the macro without using the static keyword to workaround this problem.
#undef START_TEST
#define START_TEST(__testname)\
void __testname (int _i CK_ATTRIBUTE_UNUSED)\
{\
  tcase_fn_start (""# __testname, __FILE__, __LINE__);

#include "core/core_check.h"
#include "providers/provide_check.h"
#include "network/network_check.h"
#include "objects/objects_check.h"
#include "users/users_check.h"

extern int case_timeout;

void     log_test(chr_t *test, stringer_t *error);
#define log_unit(...) log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_LINE_FEED_DISABLE | M_LOG_TIME_DISABLE | M_LOG_FILE_DISABLE | M_LOG_LINE_DISABLE | M_LOG_FUNCTION_DISABLE | M_LOG_STACK_TRACE_DISABLE, __VA_ARGS__)
#define testcase(s, tc, name, func) tcase_add_test((tc = tcase_create(name)), func); tcase_set_timeout(tc, case_timeout); suite_add_tcase(s, tc)

//! Quick Test
#if 1
#define RUN_TEST_CASE_TIMEOUT 100
#define PROFILE_TEST_CASE_TIMEOUT 1000

#define INX_CHECK_MTHREADS 0
#define INX_CHECK_OBJECTS 1024

#define TREE_INSERTS_CHECK 128
#define TREE_CURSORS_CHECK 128
#define LINKED_INSERTS_CHECK 128
#define LINKED_CURSORS_CHECK 128
#define HASHED_INSERTS_CHECK 128
#define HASHED_CURSORS_CHECK 128

#define QP_CHECK_SIZE 1024
#define URL_CHECK_SIZE 1024
#define HEX_CHECK_SIZE 1024
#define BASE64_CHECK_SIZE 1024
#define ZBASE32_CHECK_SIZE 1024

#define QP_CHECK_ITERATIONS 16
#define URL_CHECK_ITERATIONS 16
#define HEX_CHECK_ITERATIONS 16
#define BASE64_CHECK_ITERATIONS 16
#define ZBASE32_CHECK_ITERATIONS 16

#define TANK_CHECK_DATA_HNUM 1L
#define TANK_CHECK_DATA_UNUM 1L
#define TANK_CHECK_DATA_MTHREADS 2 // Disabled
#define TANK_CHECK_DATA_CLEANUP true
#define TANK_CHECK_DATA_PATH "dev/corpus/"

#define DSPAM_CHECK_SIZE_MIN 1024
#define DSPAM_CHECK_SIZE_MAX (2 * 1024)
#define DSPAM_CHECK_DATA_UNUM 1L
#define DSPAM_CHECK_ITERATIONS 128
#define DSPAM_CHECK_DATA_PATH "dev/corpus/"

#define VIRUS_CHECK_DATA_PATH "dev/corpus/"

// Controls the size of the compression test block.
#define COMPRESS_CHECK_SIZE_MIN 1024 // 1 kilobyte
#define COMPRESS_CHECK_SIZE_MAX (2 * 1024) // 2 kilobytes
#define COMPRESS_CHECK_MTHREADS 2 // Disabled
#define COMPRESS_CHECK_ITERATIONS 16

#define RAND_CHECK_SIZE_MIN 64
#define RAND_CHECK_SIZE_MAX 128
#define RAND_CHECK_ITERATIONS 128
#define RAND_CHECK_MTHREADS 2

#define SCRAMBLE_CHECK_SIZE_MIN (1024) // 1 kilobyte
#define SCRAMBLE_CHECK_SIZE_MAX (2 * 1024) // 2 kilobytes
#define SCRAMBLE_CHECK_ITERATIONS 16

#define DIGEST_CHECK_SIZE 1024
#define DIGEST_CHECK_ITERATIONS 16

#define SYMMETRIC_CHECK_SIZE_MIN 64
#define SYMMETRIC_CHECK_SIZE_MAX 1024
#define SYMMETRIC_CHECK_ITERATIONS 16

#define ECIES_CHECK_SIZE_MIN (1024) // 1 kilobyte
#define ECIES_CHECK_SIZE_MAX (2 * 1024) // 2 kilobytes
#define ECIES_CHECK_ITERATIONS 16

#define OBJECT_CHECK_ITERATIONS 16

//! Exhaustive Test
#else

// Maximum number of seconds for a given test case.
#define RUN_TEST_CASE_TIMEOUT 86400
#define PROFILE_TEST_CASE_TIMEOUT 864000

#define INX_CHECK_MTHREADS 8
#define INX_CHECK_OBJECTS 8192

#define TREE_INSERTS_CHECK 8192
#define TREE_CURSORS_CHECK 8192
#define LINKED_INSERTS_CHECK 8192
#define LINKED_CURSORS_CHECK 8192
#define HASHED_INSERTS_CHECK 8192
#define HASHED_CURSORS_CHECK 8192

#define QP_CHECK_SIZE 8192
#define URL_CHECK_SIZE 8192
#define HEX_CHECK_SIZE 8192
#define BASE64_CHECK_SIZE 8192
#define ZBASE32_CHECK_SIZE 8192

#define QP_CHECK_ITERATIONS 8192
#define URL_CHECK_ITERATIONS 8192
#define HEX_CHECK_ITERATIONS 8192
#define BASE64_CHECK_ITERATIONS 8192
#define ZBASE32_CHECK_ITERATIONS 8192

#define TANK_CHECK_DATA_HNUM 1L
#define TANK_CHECK_DATA_UNUM 1L
#define TANK_CHECK_DATA_MTHREADS 8
#define TANK_CHECK_DATA_CLEANUP true
#define TANK_CHECK_DATA_PATH "dev/corpus"

#define DSPAM_CHECK_DATA_UNUM 1L
#define DSPAM_CHECK_ITERATIONS 8192
#define DSPAM_CHECK_SIZE_MIN 1024
#define DSPAM_CHECK_SIZE_MAX (16 * 1024)
//#define DSPAM_CHECK_SIZE_MAX (1 * 1024 * 1024) // 1 megabyte
#define DSPAM_CHECK_DATA_PATH "dev/corpus"

#define VIRUS_CHECK_DATA_PATH "dev/corpus"

#define COMPRESS_CHECK_MTHREADS 8
#define COMPRESS_CHECK_ITERATIONS 256
#define COMPRESS_CHECK_SIZE_MIN 1024 // 1 kilobyte
#define COMPRESS_CHECK_SIZE_MAX (16 * 1024)
//#define COMPRESS_CHECK_SIZE_MAX (1 * 1024 * 1024) // 1 megabyte

#define RAND_CHECK_MTHREADS 8
#define RAND_CHECK_ITERATIONS 256
#define RAND_CHECK_SIZE_MIN 1024 // 1 kilobyte
#define RAND_CHECK_SIZE_MAX (16 * 1024)
//#define RAND_CHECK_SIZE_MAX (1 * 1024 * 1024) // 1 megabyte

#define ECIES_CHECK_ITERATIONS 256
#define ECIES_CHECK_SIZE_MIN 1024 // 1 kilobyte
#define ECIES_CHECK_SIZE_MAX (16 * 1024)
//#define ECIES_CHECK_SIZE_MAX (1 * 1024 * 1024) // 1 megabyte

#define SCRAMBLE_CHECK_ITERATIONS 256
#define SCRAMBLE_CHECK_SIZE_MIN 1024 // 1 kilobyte
#define SCRAMBLE_CHECK_SIZE_MAX (16 * 1024)
//#define SCRAMBLE_CHECK_SIZE_MAX (1 * 1024 * 1024) // 1 megabyte

#define DIGEST_CHECK_ITERATIONS 256
#define DIGEST_CHECK_SIZE (16 * 1024)
//#define DIGEST_CHECK_SIZE (1 * 1024 * 1024) // 1 megabyte

#define SYMMETRIC_CHECK_ITERATIONS 256
#define SYMMETRIC_CHECK_SIZE_MIN 1024 // 1 kilobyte
#define SYMMETRIC_CHECK_SIZE_MAX (16 * 1024)
//#define SYMMETRIC_CHECK_SIZE_MAX (1 * 1024 * 1024) // 1 megabyte

#define OBJECT_CHECK_ITERATIONS 256

#endif

#endif
