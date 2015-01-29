
/**
 * @file /magma/providers/symbols.c
 *
 * @brief Functions used to load the external library symbols.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

void *lib_magma = NULL;

/**
 * @brief	Initialize and bind an import symbol table.
 * @see		dlsym()
 * @param	count	the number of symbols in the table.
 * @param	symbols	the symbol table to be patched.
 * @return	true on success or false on failure.
 */
bool_t lib_symbols(size_t count, symbol_t symbols[]) {

	if (!count || !lib_magma) {
		log_critical("An invalid request was made.");
		return false;
	}

	// Scans the symbols array to ensure none of the symbols/pointers have been referenced twice.
	for (size_t i = 0; i < count; i++) {
		for (size_t j = i + 1; j < count; j++) {
			if (symbols[i].pointer == symbols[j].pointer) {
				log_critical("A dynamic function pointer has been referenced twice. {name = %s / pointer = %p}", symbols[i].name, symbols[i].pointer);
				return false;
			}
			else if (!st_cmp_cs_eq(NULLER(symbols[i].name), NULLER(symbols[j].name))) {
				log_critical("A dynamically loaded symbol has been referenced twice. {name = %s / pointer = %p}", symbols[i].name, symbols[i].pointer);
				return false;
			}
		}
	}

	// Loop through and setup the function pointers.
	for (size_t i = 0; i < count; i++) {
		if ((*(symbols[i].pointer) = dlsym(lib_magma, symbols[i].name)) == NULL) {
			log_critical("Unable to establish a pointer to the function %s.", symbols[i].name);
			log_critical("%s", dlerror());
			return false;
		}
	}

	return true;
}

/**
 * @brief	Unload libmagma from memory.
 * @return	This function returns no value.
 */
void lib_unload(void) {

	if (magma.library.unload) {
		dlclose(lib_magma);
	}

	return;
}

/**
 * @brief	Load libmagma dynamically and resolve all external dependencies from 3rd party providers.
 * @return	false on failure or true on success.
 */
bool_t lib_load(void) {

	if (!magma.config.file) {
		log_critical("The Magma shared library path is empty.");
		return false;
	}
	else if (!st_cmp_ci_eq(NULLER(magma.library.file), CONSTANT("NULL"))) {
		lib_magma = dlopen(RTLD_DEFAULT, RTLD_NOW | RTLD_GLOBAL);
	}
	else {
		lib_magma = dlopen(magma.library.file, RTLD_NOW | RTLD_GLOBAL);
	}

	if (!lib_magma) {
		log_critical("%s", dlerror());
		return false;
	}

	else if (!lib_load_bzip() || !lib_load_cache() || !lib_load_clamav() || !lib_load_dkim() || !lib_load_dspam() ||
		!lib_load_freetype() || !lib_load_gd() || !lib_load_jansson() || !lib_load_jpeg() || !lib_load_lzo() ||
		!lib_load_mysql() || !lib_load_openssl() || !lib_load_png() || !lib_load_spf() || !lib_load_tokyo() || !lib_load_xml() ||
		!lib_load_zlib()) {
		return false;
	}

	log_pedantic("-------------------------------- VERSIONS --------------------------------\n\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n" \
		"%-10.10s %63.63s\n%-10.10s %63.63s\n%-10.10s %63.63s\n",
			"MAGMA:", build_version(),
			"BUILD:", build_stamp(),
			"PLATFORM:", st_char_get(host_platform(MANAGEDBUF(128))),
			"KERNEL:", st_char_get(host_version(MANAGEDBUF(128))),
			"DATABASE:", serv_type_mysql(),
			"BUILD:", serv_version_mysql(),
			"SCHEMA:", serv_schema_mysql(),
			"ENCODING:", serv_charset_mysql(),
			"BZIP:", lib_version_bzip(),
			"CLAMAV:", lib_version_clamav(),
			"DKIM:", lib_version_dkim(),
			"DSPAM:", lib_version_dspam(),
			"FREETYPE", lib_version_freetype(),
			"GD", lib_version_gd(),
			"GLIBC:", gnu_get_libc_version(),
			"JANSSON:", lib_version_jansson(),
			"JPEG", lib_version_jpeg(),
			"LZO:", lib_version_lzo(),
			"MEMCACHED:", lib_version_cache(),
			"MYSQL:", lib_version_mysql(),
			"OPENSSL:", lib_version_openssl(),
			"PNG", lib_version_png(),
			"SPF:", lib_version_spf(),
			"TOKYO:", lib_version_tokyo(),
			"XML:",	lib_version_xml(),
			"ZLIB:", lib_version_zlib());
	return true;
}
