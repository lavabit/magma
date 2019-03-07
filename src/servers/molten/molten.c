
/**
 * @file /magma/servers/molten/molten.c
 *
 * @brief	Functions used to handle Molten commands/actions.
 */

#include "magma.h"

void molten_version(connection_t *con) {

	if (con_print(con, "VERSION magmad %s\r\n" \
		"VERSION buildstamp %s\r\n" \
		"VERSION bzip %s\r\n" \
		"VERSION clamav %s\r\n" \
		"VERSION commit %s\r\n" \
		"VERSION dkim %s\r\n" \
		"VERSION dspam %s\r\n" \
		"VERSION freetype %s\r\n" \
		"VERSION gd %s\r\n",
		build_version(),
		build_stamp(),
		lib_version_bzip(),
		lib_version_clamav(),
		lib_version_dkim(),
		lib_version_dspam(),
		build_commit(),
		lib_version_freetype(),
		lib_version_gd()) < 0) {
		enqueue(&molten_quit, con);
		return;
	}

#if defined(__GNU_LIBRARY__)
	if (con_print(con, "VERSION glibc %s\r\n", gnu_get_libc_version()) < 0) {
		enqueue(&molten_quit, con);
		return;
	}
#endif

	if (con_print(con, "VERSION jansson %s\r\n" \
		"VERSION jpeg %s\r\n" \
		"VERSION kernel %s\r\n" \
		"VERSION lzo %s\r\n" \
		"VERSION maria %s\r\n" \
		"VERSION memcached %s\r\n" \
		"VERSION openssl %s\r\n" \
		"VERSION png %s\r\n" \
		"VERSION spf %s\r\n" \
		"VERSION tokyo %s\r\n" \
		"VERSION utf8 %s\r\n" \
		"VERSION xml %s\r\n" \
		"VERSION zlib %s\r\n" \
		"END\r\n",
		lib_version_jansson(),
		lib_version_jpeg(),
		st_char_get(host_version(MANAGEDBUF(128))),
		lib_version_lzo(),
		lib_version_mysql(),
		lib_version_cache(),
		lib_version_openssl(),
		lib_version_png(),
		lib_version_spf(),
		lib_version_tokyo(),
		lib_version_utf8proc(),
		lib_version_xml(),
		lib_version_zlib()) < 0) {
		enqueue(&molten_quit, con);
		return;
	}

	enqueue(&molten_parse, con);

	return;
}

void molten_stats(connection_t *con) {

	size_t length;

	length = stats_get_count();

	for(size_t i = 0; i < length; i++) {

		if (con_print(con, "STAT %s %lu\r\n", stats_get_name(i), stats_get_value_by_num(i)) < 0) {
			enqueue(&molten_quit, con);
			return;
		}

	}

	length = stats_derived_count();

	for(size_t i = 0; i < length; i++) {

		if (con_print(con, "STAT %s %lu\r\n", stats_derived_name(i), stats_derived_value(i)) < 0) {
			enqueue(&molten_quit, con);
			return;
		}

	}

	con_write_bl(con, "END\r\n", 5) < 0 ? enqueue(&molten_quit, con) : enqueue(&molten_parse, con);

	return;
}

void molten_invalid(connection_t *con) {

	con_write_bl(con, "ERROR\r\n", 7) < 0 ? enqueue(&molten_quit, con) : enqueue(&molten_parse, con);
	return;
}

void molten_quit(connection_t *con) {

	con_destroy(con);
	return;
}

void molten_init(connection_t *con) {

	enqueue(&molten_parse, con);
	return;
}
