
/**
 * @file /magma/src/objects/auth/legacy.c
 *
 * @brief Functions needed to support/convert password hashes using the deprecated legacy strategy.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"


/****
 * Removing Legacy Password Support
 *
 * - Database
 * 		Remove the legacy field from the Users table.
 *
 * - Config
 * 		Remove the "salt" parameter from the global config.
 *
 * - Queries
 * 		Remove/update queries which reference the legacy field.
 *
 * - Auth
 * 		Remove the legacy struct from the auth_t typedef.
 *
 * - Code
 * 		Remove the legacy routines and any code which references the auth->legacy struct.
 */
