
/**
 * @file /magma/providers/providers.h
 *
 * @brief The entry point for the provider modules.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_PROVIDERS_H
#define MAGMA_PROVIDERS_H

typedef struct {
	const chr_t *name;
	void **pointer;
} symbol_t;

// Functions used to load external symbols
bool_t lib_load(void);
void lib_unload(void);
bool_t lib_symbols(size_t count, symbol_t symbols[]);

#include "database/database.h"
#include "consumers/consumers.h"
#include "checkers/checkers.h"
#include "compress/compress.h"
#include "cryptography/cryptography.h"
#include "prime/prime.h"
#include "parsers/parsers.h"
#include "storage/storage.h"
#include "images/images.h"
//#include "deprecated/deprecated.h"

#endif
