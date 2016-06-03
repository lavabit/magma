
/**
 * @file /magma/providers/checkers/dspam.c
 *
 * @brief DSPAM interface functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

extern pool_t *sql_pool;

/**
 * @brief	Return the version string of the dspam library.
 * @return	a pointer to a character string containing the dspam library version information.
 */
chr_t * lib_version_dspam(void) {
	return (chr_t *)dspam_version_d();
}

/**
 * @brief	Initialize the dspam library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_dspam(void) {

	symbol_t dspam[] = {
		M_BIND(dspam_attach), M_BIND(dspam_create), M_BIND(dspam_destroy), M_BIND(dspam_detach), M_BIND(dspam_init_driver),
		M_BIND(dspam_process), M_BIND(dspam_shutdown_driver), M_BIND(dspam_version)
	};

	if (lib_symbols(sizeof(dspam) / sizeof(symbol_t), dspam) != 1) {
		return false;
	}

	return true;
}

bool_t dspam_start(void) {
	if (dspam_init_driver_d(NULL) != 0) {
		return false;
	}
	return true;
}

void dspam_stop(void) {
	dspam_shutdown_driver_d(NULL);
	return;
}

/// HIGH: Return a result structure with the disposition, confidence, probability and signature data. Then store the analysis result with the message. Either in the DB or
/// by adding a custom header to the message. Return codes should become 0 for success, or a negative integer for errors. Add statistical updates to check and train functions.
/// Result: result=\"%s\"; class=\"%s\"; probability=%01.4f; confidence=%02.2f; signature=%lu; key=%lu;
/// Layman's terms: How spammy is this message
int_t dspam_check(uint64_t usernum, stringer_t *message, stringer_t **signature) {

	DSPAM_CTX *ctx;
	chr_t unum[20];
	uint32_t connection;
	int_t result = 2, ret;
	struct _mysql_drv_dbh dbh;
	stringer_t *tmpdir, *output;

	// Generate a string version of the dispatch number.
	if (snprintf(unum, 20, "%lu", usernum) <= 0 || !(tmpdir = spool_path(MAGMA_SPOOL_DATA))) {
		log_pedantic("Context setup error.");
		return -1;
	}
	// Initialize the DSPAM context.
	else if (!(ctx = dspam_create_d(unum, NULL, st_char_get(tmpdir), DSM_PROCESS, DSF_SIGNATURE | DSF_NOISE | DSF_WHITELIST))) {
		log_pedantic("An error occurred inside the DSPAM library. {dspam_create = NULL}");
		st_free(tmpdir);
    return -1;
 	}

	st_free(tmpdir);

	if (pool_pull(sql_pool, &connection) != PL_RESERVED) {
		log_info("Unable to get an available connection for the query.");
		dspam_destroy_d(ctx);
		return -1;
	}
	else if (sql_ping(connection) < 0 || !stmt_rebuild(connection)) {
		log_info("The database connection has been lost and the reconnection attempt failed.");
		dspam_destroy_d(ctx);
		return -1;
	}

	// Setup the database handle in a structure format.
	dbh.dbh_read = pool_get_obj(sql_pool, connection);
	dbh.dbh_write = pool_get_obj(sql_pool, connection);

	if ((ret = dspam_attach_d(ctx, &dbh))) {
   	if (dspam_detach_d(ctx) != 0) {
			log_pedantic("Could not detach the DB connection.");
		}
		pool_release(sql_pool, connection);
		log_pedantic("An error occurred while attaching to the statistical database. {dspam_attach = %i}", ret);
		dspam_destroy_d(ctx);
		return -1;
  }

	// Tokenization method and statistical algorithm.
	ctx->algorithms = DSA_GRAHAM | DSA_BURTON | DSP_GRAHAM;
	ctx->tokenizer = DSZ_CHAIN;

	// To prevent the message tokens from being stored in the database we disable training.
	ctx->training_mode = DST_NOTRAIN;

	// This actually processes the message.
	if ((ret = dspam_process_d(ctx, st_char_get(message)))) {
		if (dspam_detach_d(ctx) != 0) {
			log_pedantic("Could not detach the DB connection.");
		}
		pool_release(sql_pool, connection);
		log_pedantic("An error occurred while analyzing an email with DSPAM. {dspam_process = %i}", ret);
		dspam_destroy_d(ctx);
    return -1;
	}

	// We assume that the SQL connection will no longer be needed.
	if ((ret = dspam_detach_d(ctx))) {
		log_pedantic("Could not detach the DB connection. {dspam_detach = %i}", ret);
		pool_release(sql_pool, connection);
		dspam_destroy_d(ctx);
    return -1;
	}

	// Return the connection to our pool.
	pool_release(sql_pool, connection);

	// Check to see if the message is junk mail.
	if (ctx->result == DSR_ISSPAM) {
		result = -2;
	}
	else {
		result = 1;
	}

	//log_pedantic("Probability: %2.4f Confidence: %2.4f, Result: %s", ctx->probability, ctx->confidence,
 	//	(ctx->result == DSR_ISSPAM) ? "JUNK" : "INNOCENT");

	// See what happens if we don't get a signature back.
	if (ctx->signature == NULL) {
		log_error("DSPAM did not return a signature. {ctx->signature = NULL}");
		dspam_destroy_d(ctx);
    return result;
  }

	// Copy over the signature.
	if (!(output = st_import(ctx->signature->data, ctx->signature->length))) {
		log_pedantic("Could not import the statistical signature. {length = %lu}", ctx->signature->length);
		dspam_destroy_d(ctx);
		return result;
  }

	if (signature) {
		*signature = output;
	}

	// Destroy the context and return the result.
	dspam_destroy_d(ctx);
	return result;
}

/**
 *  @note	The disposition parameter marks whether or not the signature is currently marked as junk.
 *  		So training the signature means that it will toggle its status.
 *  @param	usernum			the numerical id of the user making the spam training request.
 *  @param	disposition		if 0, dspam will mark the signature as junk; otherwise, mark as OK.
 *  @param	signature		a managed string containing the spam signature to be trained.
 *  @return	true on success or false on failure.
 */
bool_t dspam_train(uint64_t usernum, int_t disposition, stringer_t *signature) {

	int_t ret;
	DSPAM_CTX *ctx;
	chr_t unum[20];
	stringer_t *tmpdir;
	uint32_t connection;
	struct _mysql_drv_dbh dbh;
	struct _ds_spam_signature sig;

	// Generate a string version of the dispatch number.
	if (snprintf(unum, 20, "%lu", usernum) <= 0 || !(tmpdir = spool_path(MAGMA_SPOOL_DATA))) {
		log_pedantic("Context setup error.");
		return false;
	}
	// Initialize the DSPAM context.
	else if (!(ctx = dspam_create_d(unum, NULL, st_char_get(tmpdir), DSM_PROCESS, DSF_SIGNATURE | DSF_NOISE | DSF_WHITELIST))) {
		log_pedantic("An error occurred inside the DSPAM library. {dspam_create = NULL}");
		st_free(tmpdir);
		return false;
 	}

	st_free(tmpdir);

	// Get a DB connection.
	if (pool_pull(sql_pool, &connection) != PL_RESERVED) {
		log_info("Unable to get an available connection for the query.");
		dspam_destroy_d(ctx);
		return false;
	}
	else if (sql_ping(connection) < 0 || !stmt_rebuild(connection)) {
		log_info("The database connection has been lost and the reconnection attempt failed.");
		dspam_destroy_d(ctx);
		return false;
	}

	// Setup the database handle in a structure format.
	dbh.dbh_read = pool_get_obj(sql_pool, connection);
	dbh.dbh_write = pool_get_obj(sql_pool, connection);

	if ((ret = dspam_attach_d(ctx, &dbh))) {

		if (dspam_detach_d(ctx) != 0) {
			log_pedantic("Could not detach the DB connection.");
		}

		pool_release(sql_pool, connection);
		log_pedantic("An error occurred while attaching to the statistical database. {dspam_attach = %i}", ret);
		dspam_destroy_d(ctx);
		return false;
	}

	// Tokenization method and statistical algorithm.
	ctx->algorithms = DSA_GRAHAM | DSA_BURTON | DSP_GRAHAM;
	ctx->tokenizer = DSZ_CHAIN;

	// Setup the classification as opposite of what the original was.
	ctx->classification = disposition ? DSR_ISINNOCENT : DSR_ISSPAM;

	// Set up the context for error correction.
	ctx->source = DSS_ERROR;

	// Setup the signature.
	sig.length = st_length_get(signature);
	sig.data = st_char_get(signature);
	ctx->signature = &sig;

	// Call DSPAM, and then destroy the context.
	ret = dspam_process_d(ctx, NULL);
	dspam_detach_d(ctx);
	dspam_destroy_d(ctx);
	pool_release(sql_pool, connection);

	if (ret) {
		log_pedantic("An error occurred while training message signature. {dspam_process = %i}", ret);
		return false;
	}

	return true;
}
