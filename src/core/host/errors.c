
/**
 * @file /magma/src/core/host/errors.c
 *
 * @brief	Functions to help handle errno.
 */

#include "magma.h"

#ifdef EPERM
 #define MAGMA_EPERM EPERM
#else
 #define MAGMA_EPERM 1
#endif
#ifdef ENOENT
 #define MAGMA_ENOENT ENOENT
#else
 #define MAGMA_ENOENT 2
#endif
#ifdef ESRCH
 #define MAGMA_ESRCH ESRCH
#else
 #define MAGMA_ESRCH 3
#endif
#ifdef EINTR
 #define MAGMA_EINTR EINTR
#else
 #define MAGMA_EINTR 4
#endif
#ifdef EIO
 #define MAGMA_EIO EIO
#else
 #define MAGMA_EIO 5
#endif
#ifdef ENXIO
 #define MAGMA_ENXIO ENXIO
#else
 #define MAGMA_ENXIO 6
#endif
#ifdef E2BIG
 #define MAGMA_E2BIG E2BIG
#else
 #define MAGMA_E2BIG 7
#endif
#ifdef ENOEXEC
 #define MAGMA_ENOEXEC ENOEXEC
#else
 #define MAGMA_ENOEXEC 8
#endif
#ifdef EBADF
 #define MAGMA_EBADF EBADF
#else
 #define MAGMA_EBADF 9
#endif
#ifdef ECHILD
 #define MAGMA_ECHILD ECHILD
#else
 #define MAGMA_ECHILD 10
#endif
#ifdef EAGAIN
 #define MAGMA_EAGAIN EAGAIN
#else
 #define MAGMA_EAGAIN 11
#endif
#ifdef ENOMEM
 #define MAGMA_ENOMEM ENOMEM
#else
 #define MAGMA_ENOMEM 12
#endif
#ifdef EACCES
 #define MAGMA_EACCES EACCES
#else
 #define MAGMA_EACCES 13
#endif
#ifdef EFAULT
 #define MAGMA_EFAULT EFAULT
#else
 #define MAGMA_EFAULT 14
#endif
#ifdef ENOTBLK
 #define MAGMA_ENOTBLK ENOTBLK
#else
 #define MAGMA_ENOTBLK 15
#endif
#ifdef EBUSY
 #define MAGMA_EBUSY EBUSY
#else
 #define MAGMA_EBUSY 16
#endif
#ifdef EEXIST
 #define MAGMA_EEXIST EEXIST
#else
 #define MAGMA_EEXIST 17
#endif
#ifdef EXDEV
 #define MAGMA_EXDEV EXDEV
#else
 #define MAGMA_EXDEV 18
#endif
#ifdef ENODEV
 #define MAGMA_ENODEV ENODEV
#else
 #define MAGMA_ENODEV 19
#endif
#ifdef ENOTDIR
 #define MAGMA_ENOTDIR ENOTDIR
#else
 #define MAGMA_ENOTDIR 20
#endif
#ifdef EISDIR
 #define MAGMA_EISDIR EISDIR
#else
 #define MAGMA_EISDIR 21
#endif
#ifdef EINVAL
 #define MAGMA_EINVAL EINVAL
#else
 #define MAGMA_EINVAL 22
#endif
#ifdef ENFILE
 #define MAGMA_ENFILE ENFILE
#else
 #define MAGMA_ENFILE 23
#endif
#ifdef EMFILE
 #define MAGMA_EMFILE EMFILE
#else
 #define MAGMA_EMFILE 24
#endif
#ifdef ENOTTY
 #define MAGMA_ENOTTY ENOTTY
#else
 #define MAGMA_ENOTTY 25
#endif
#ifdef ETXTBSY
 #define MAGMA_ETXTBSY ETXTBSY
#else
 #define MAGMA_ETXTBSY 26
#endif
#ifdef EFBIG
 #define MAGMA_EFBIG EFBIG
#else
 #define MAGMA_EFBIG 27
#endif
#ifdef ENOSPC
 #define MAGMA_ENOSPC ENOSPC
#else
 #define MAGMA_ENOSPC 28
#endif
#ifdef ESPIPE
 #define MAGMA_ESPIPE ESPIPE
#else
 #define MAGMA_ESPIPE 29
#endif
#ifdef EROFS
 #define MAGMA_EROFS EROFS
#else
 #define MAGMA_EROFS 30
#endif
#ifdef EMLINK
 #define MAGMA_EMLINK EMLINK
#else
 #define MAGMA_EMLINK 31
#endif
#ifdef EPIPE
 #define MAGMA_EPIPE EPIPE
#else
 #define MAGMA_EPIPE 32
#endif
#ifdef EDOM
 #define MAGMA_EDOM EDOM
#else
 #define MAGMA_EDOM 33
#endif
#ifdef ERANGE
 #define MAGMA_ERANGE ERANGE
#else
 #define MAGMA_ERANGE 34
#endif

chr_t * errno_name(int error) {

	chr_t * string = "UNKNOWN";

	switch (error) {

		case MAGMA_EPERM:
			string = "EPERM";
			break;
		case MAGMA_ENOENT:
			string = "ENOENT";
			break;
		case MAGMA_ESRCH:
			string = "ESRCH";
			break;
		case MAGMA_EINTR:
			string = "EINTR";
			break;
		case MAGMA_EIO:
			string = "EIO";
			break;
		case MAGMA_ENXIO:
			string = "ENXIO";
			break;
		case MAGMA_E2BIG:
			string = "E2BIG";
			break;
		case MAGMA_ENOEXEC:
			string = "ENOEXEC";
			break;
		case MAGMA_EBADF:
			string = "EBADF";
			break;
		case MAGMA_ECHILD:
			string = "ECHILD";
			break;
		case MAGMA_EAGAIN:
			string = "EAGAIN";
			break;
		case MAGMA_ENOMEM:
			string = "ENOMEM";
			break;
		case MAGMA_EACCES:
			string = "EACCES";
			break;
		case MAGMA_EFAULT:
			string = "EFAULT";
			break;
		case MAGMA_ENOTBLK:
			string = "ENOTBLK";
			break;
		case MAGMA_EBUSY:
			string = "EBUSY";
			break;
		case MAGMA_EEXIST:
			string = "EEXIST";
			break;
		case MAGMA_EXDEV:
			string = "EXDEV";
			break;
		case MAGMA_ENODEV:
			string = "ENODEV";
			break;
		case MAGMA_ENOTDIR:
			string = "ENOTDIR";
			break;
		case MAGMA_EISDIR:
			string = "EISDIR";
			break;
		case MAGMA_EINVAL:
			string = "EINVAL";
			break;
		case MAGMA_ENFILE:
			string = "ENFILE";
			break;
		case MAGMA_EMFILE:
			string = "EMFILE";
			break;
		case MAGMA_ENOTTY:
			string = "ENOTTY";
			break;
		case MAGMA_ETXTBSY:
			string = "ETXTBSY";
			break;
		case MAGMA_EFBIG:
			string = "EFBIG";
			break;
		case MAGMA_ENOSPC:
			string = "ENOSPC";
			break;
		case MAGMA_ESPIPE:
			string = "ESPIPE";
			break;
		case MAGMA_EROFS:
			string = "EROFS";
			break;
		case MAGMA_EMLINK:
			string = "EMLINK";
			break;
		case MAGMA_EPIPE:
			string = "EPIPE";
			break;

		default:
			string = "ERANGE";
			break;
		}

	return string;
}

/**
 * @brief Handle different versions of strerror, which can return either an integer or a character pointer.
 * @param errnum  the error number.
 * @param buf     a pointer to the output buffer.
 * @return  NULL on failure, or a pointer to a the buffer with the error string.
 */
chr_t * errno_string(int errnum, char *buf, size_t len) {

#if defined(__USE_GNU) && __ANDROID_API__ >= 23
  return __gnu_strerror_r(errnum, buf, len);
#elif defined(__GLIBC__)
 return strerror_r(errnum, buf, len);
#else
  int_t result = 0;
  result = strerror_r(errnum, buf, len);
  return (result == 0 ? buf : NULL);
#endif

}
