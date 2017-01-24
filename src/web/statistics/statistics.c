
/**
 * @file /magma/web/statistics/statistics.c
 *
 * @brief	Generate a dynamic web page with statistics on server performance.
 */

#include "magma.h"

extern statistics_vp_t portal_stats[portal_stat_users_num_statements];

/**
 * @brief	Display the statistics page to the requesting connection.
 * @param	con		a pointer to the connection object across which the server statistics will be transmitted.
 * @return	This function returns no value.
 */
void statistics_process(connection_t *con) {

	time_t sm_time;
	chr_t buffer[256];
	stringer_t *raw;
	struct tm tm_time;
	http_page_t *page;

	if (!(page = http_page_get("statistics/statistics"))) {
		http_print_500(con);
		return;
	}

	statistics_refresh();

	if ((sm_time = time(NULL)) == ((time_t)-1) || !localtime_r(&sm_time, &tm_time) ||
		strftime(buffer, 256, "These statistics were last updated %A, %B %e, %Y at %I:%M:%S %p %Z.", &tm_time) <= 0) {
		log_pedantic("Unable to build the time string.");
	}
	else {
		// Update the time.
		xml_set_xpath_ns(page->xpath_ctx, (xmlChar *)"//xhtml:p[@id='time']", (uchr_t *)buffer);
	}

	xml_set_xpath_uint64(page->xpath_ctx, (xmlChar *)"//xhtml:td[@id='total_users']", portal_stats[portal_stat_total_users].val);
	xml_set_xpath_uint64(page->xpath_ctx, (xmlChar *)"//xhtml:td[@id='checked_email_today']", portal_stats[portal_stat_users_checked_email_today].val);
	xml_set_xpath_uint64(page->xpath_ctx, (xmlChar *)"//xhtml:td[@id='checked_email_week']", portal_stats[portal_stat_users_checked_email_week].val);
	xml_set_xpath_uint64(page->xpath_ctx, (xmlChar *)"//xhtml:td[@id='sent_email_today']", portal_stats[portal_stat_users_sent_email_today].val);
	xml_set_xpath_uint64(page->xpath_ctx, (xmlChar *)"//xhtml:td[@id='sent_email_week']", portal_stats[portal_stat_users_sent_email_week].val);
	xml_set_xpath_uint64(page->xpath_ctx, (xmlChar *)"//xhtml:td[@id='emails_received_today']", portal_stats[portal_stat_emails_received_today].val);
	xml_set_xpath_uint64(page->xpath_ctx, (xmlChar *)"//xhtml:td[@id='emails_received_week']", portal_stats[portal_stat_emails_received_week].val);
	xml_set_xpath_uint64(page->xpath_ctx, (xmlChar *)"//xhtml:td[@id='emails_sent_today']", portal_stats[portal_stat_emails_sent_today].val);
	xml_set_xpath_uint64(page->xpath_ctx, (xmlChar *)"//xhtml:td[@id='emails_sent_week']", portal_stats[portal_stat_emails_sent_week].val);
	xml_set_xpath_uint64(page->xpath_ctx, (xmlChar *)"//xhtml:td[@id='users_registered_today']", portal_stats[portal_stat_users_registered_today].val);
	xml_set_xpath_uint64(page->xpath_ctx, (xmlChar *)"//xhtml:td[@id='users_registered_week']", portal_stats[portal_stat_users_registered_week].val);

	if (!(raw = xml_dump_doc(page->doc_obj))) {
		http_print_500(con);
		http_page_free(page);
		return;
	}

	http_response_header(con, 200, page->content->type, st_length_get(raw));
	con_write_st(con, raw);
	http_page_free(page);
	st_free(raw);

	return;
}
