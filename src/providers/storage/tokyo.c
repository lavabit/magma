
/**
 * @file /magma/providers/storage/tokyo.c
 *
 * @brief Tokyo Cabinet symbols.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */


#include "magma.h"

const char * lib_version_tokyo(void) {
	return *tcversion_d;
}

/**
 * @brief	Initialize the Tokyo Cabinet library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_tokyo(void) {

	symbol_t tokyo[] = {
			M_BIND(tcndbdel), M_BIND(tcndbnew2), M_BIND(tcndbfwmkeys), M_BIND(tcndbget), M_BIND(tclistdel),	M_BIND(tclistnum),
			M_BIND(tclistval), M_BIND(tcndbputkeep), M_BIND(tcndbout), M_BIND(tcndbrnum), M_BIND(tctreekeys), M_BIND(tctreevals),
			M_BIND(tcndbgetboth), M_BIND(tchdbnew), M_BIND(tchdbdel), M_BIND(tchdbecode), M_BIND(tchdbsync), M_BIND(tchdbclose),
			M_BIND(tchdberrmsg), M_BIND(tchdbtune), M_BIND(tchdbputasync), M_BIND(tchdbopen), M_BIND(tchdbsetmutex), M_BIND(tchdbout),
			M_BIND(tchdbpath), M_BIND(tchdbget), M_BIND(tcfree), M_BIND(tchdbrnum),	M_BIND(tchdbfsiz), M_BIND(tchdbsetdfunit),
			M_BIND(tchdbdefrag), M_BIND(tchdboptimize),	M_BIND(tcndbget3), M_BIND(tcndbiternext2), M_BIND(tcndbiterinit), M_BIND(tcndbdup),
			M_BIND(tcversion)
	};

	if (!lib_symbols(sizeof(tokyo) / sizeof(symbol_t), tokyo)) {
		return false;
	}

	return true;
}
