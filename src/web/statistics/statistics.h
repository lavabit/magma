
/**
 * @file /magma/web/statistics/statistics.h
 *
 * @brief	Code for dynamically generating the portal statistics page.
 */

#ifndef MAGMA_WEB_STATISTICS_H
#define MAGMA_WEB_STATISTICS_H

enum {
	portal_stat_total_users = 0,
	portal_stat_users_checked_email_today = 1,
	portal_stat_users_checked_email_week = 2,
	portal_stat_users_sent_email_today = 3,
	portal_stat_users_sent_email_week = 4,
	portal_stat_emails_received_today = 5,
	portal_stat_emails_received_week = 6,
	portal_stat_emails_sent_today = 7,
	portal_stat_emails_sent_week = 8,
	portal_stat_users_registered_today = 9,
	portal_stat_users_registered_week = 10,
	portal_stat_users_registered_total = 11
};

typedef struct {
	MYSQL_STMT **stmt;
	uint64_t val;
} statistics_vp_t;


/// statistics.c
void   statistics_process(connection_t *con);

/// datatier.c
void	statistics_init(void);
bool_t	statistics_refresh(void);

#endif

