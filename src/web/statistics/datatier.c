
/**
 * @file /magma/web/statistics/datatier.c
 *
 * @brief	Interface for harvesting statistics from the database for the portal statistics app.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"


#define PORTAL_STATISTICS_TIMEOUT	300 /* check if more than 5 minutes old */

pthread_mutex_t portal_statistics_mutex = PTHREAD_MUTEX_INITIALIZER;
time_t statistics_last_updated = 0;


statistics_vp_t portal_stats[portal_stat_users_num_statements];


/**
 * @brief	Initialize the prepared sql statements used by the portal statistics page.
 * @return	This function returns no value.
 */
void statistics_init(void) {

	mm_wipe(&portal_stats,sizeof(portal_stats));

	portal_stats[portal_stat_total_users].stmt = stmts.statistics_get_total_users;
	portal_stats[portal_stat_users_checked_email_today].stmt = stmts.statistics_get_users_checked_email_today;
	portal_stats[portal_stat_users_checked_email_week].stmt = stmts.statistics_get_users_checked_email_week;
	portal_stats[portal_stat_users_sent_email_today].stmt = stmts.statistics_get_users_sent_email_today;
	portal_stats[portal_stat_users_sent_email_week].stmt = stmts.statistics_get_users_sent_email_week;
	portal_stats[portal_stat_emails_received_today].stmt = stmts.statistics_get_emails_received_today;
	portal_stats[portal_stat_emails_received_week].stmt = stmts.statistics_get_emails_received_week;
	portal_stats[portal_stat_emails_sent_today].stmt = stmts.statistics_get_emails_sent_today;
	portal_stats[portal_stat_emails_sent_week].stmt = stmts.statistics_get_emails_sent_week;
	portal_stats[portal_stat_users_registered_today].stmt = stmts.statistics_get_users_registered_today;
	portal_stats[portal_stat_users_registered_week].stmt = stmts.statistics_get_users_registered_week;
	portal_stats[portal_stat_users_num_statements].stmt = stmts.statistics_get_total_users;
}

/**
 * @brief	Refresh all portal statistics from the database, if they haven't been updated recently.
 * @return	true if all statistics were refreshed successfully, or false if they were not.
 */
bool_t statistics_refresh(void) {

	row_t *row;
	table_t *table;
	bool_t result = true;

	portal_stats[0].stmt = stmts.statistics_get_total_users;

	// If we don't need to refresh from the database, then don't do anything.
	if (statistics_last_updated && (time(NULL) - statistics_last_updated) < PORTAL_STATISTICS_TIMEOUT) {
		return true;
	}

	mutex_lock (&portal_statistics_mutex);

	// Check one more time, because maybe somebody just updated the statistics stamp for us.
	if (statistics_last_updated && (time(NULL) - statistics_last_updated) < PORTAL_STATISTICS_TIMEOUT) {
		mutex_unlock (&portal_statistics_mutex);
		return true;
	} else if (!statistics_last_updated) {
		statistics_init();
	}

	for (int i = 0; (i < sizeof(portal_stats) / sizeof(statistics_vp_t)); i++) {

		if (!(table = stmt_get_result(portal_stats[i].stmt, NULL))) {
			result = false;
			continue;
		}

		// Get only the first row.
		if (!(row = res_row_next(table))) {
			log_pedantic("Error encountered processing portal statistics { index = %u }", i);
			res_table_free(table);
			continue;
		}

		// Store the result.
		portal_stats[i].val = res_field_uint64(row, 0);
		res_table_free(table);
	}

	statistics_last_updated = time(NULL);
	mutex_unlock (&portal_statistics_mutex);

	return result;
}
