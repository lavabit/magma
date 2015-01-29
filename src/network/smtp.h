
/**
 * @file /magma/network/smtp.h
 *
 * @brief	Structures used to control SMTP connections/sessions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_OBJECTS_SMTP_H
#define MAGMA_OBJECTS_SMTP_H

enum {
	SMTP_ACTION_ERROR = -1,
	SMTP_ACTION_UNDEFINED = 0,
	SMTP_ACTION_DELETE = 1,
	SMTP_ACTION_REJECT = 2,
	SMTP_ACTION_BOUNCE = 3,
	SMTP_ACTION_MARK = 4,
	SMTP_ACTION_MARK_READ = 5,

	SMTP_MARK_NONE = 0,
	SMTP_MARK_READ = 1,
	SMTP_MARK_PHISH = 2,
	SMTP_MARK_VIRUS = 4,
	SMTP_MARK_SPAM = 8,
	SMTP_MARK_RBL = 16,
	SMTP_MARK_SPOOF = 32,
	SMTP_MARK_FILTERED = 64,

	SMTP_FILTER_TYPE_EXACT = 0,
	SMTP_FILTER_TYPE_CONTAINS = 1,
	SMTP_FILTER_TYPE_STARTS = 2,
	SMTP_FILTER_TYPE_ENDS = 4,
	SMTP_FILTER_TYPE_REGEXP = 8,

	SMTP_FILTER_LOCATION_HEADER = 1,
	SMTP_FILTER_LOCATION_BODY = 2,
	SMTP_FILTER_LOCATION_FIELD = 4,  // Requires that the field stringer is not null.
	SMTP_FILTER_LOCATION_ENTIRE = 8,

	SMTP_FILTER_ACTION_NONE = 0,
	SMTP_FILTER_ACTION_MOVE = 1, // Requires that the folder number is not null.
	SMTP_FILTER_ACTION_LABEL = 2, // Requires that the label stringer is not null.
	SMTP_FILTER_ACTION_DELETE = 4,
	SMTP_FILTER_ACTION_MARK_READ = 8,

	SMTP_OUTCOME_SUCESS = 0,
	SMTP_OUTCOME_PERM_FAILURE = 1,
	SMTP_OUTCOME_TEMP_SERVER = 2,
	SMTP_OUTCOME_TEMP_OVERQUOTA = 4,
	SMTP_OUTCOME_TEMP_LOCKED = 8,
	SMTP_OUTCOME_BOUNCE_SPF = 16,
	SMTP_OUTCOME_BOUNCE_DKIM = 32,
	SMTP_OUTCOME_BOUNCE_VIRUS = 64,
	SMTP_OUTCOME_BOUNCE_PHISH = 128,
	SMTP_OUTCOME_BOUNCE_SPAM = 256,
	SMTP_OUTCOME_BOUNCE_RBL = 512
};

typedef struct {
	placer_t to;
	placer_t date;
	placer_t from;
	placer_t subject;
	stringer_t *id;
	stringer_t *text;
	size_t header_length;
} smtp_message_t;

// A linked list of recipients.
typedef struct {
	stringer_t *address;
	struct smtp_recipients_t *next;
} smtp_recipients_t;

typedef struct {
	uint64_t foldernum, rulenum;
	unsigned location, type, action;
	stringer_t *field, *label, *expression;
} smtp_inbound_filter_t;

// The structure for storing recipient preferences on inbound data.
typedef struct {
	int_t outcome;
	size_t recv_size_limit;
	stringer_t *rcptto, *address, *domain, *forwarded, *spamsig;
	uint32_t greytime, local_size, daily_recv_limit, daily_recv_limit_ip;
	uint64_t usernum, signum, spamkey, quota, stor_size, inbox, autoreply, messagenum, foldernum;
	int_t mark, secure, rollout, spam, virus, greylist, spf, dkim, rbl, phish, overquota, bounces, spfaction, dkimaction,
		rblaction, spamaction, virusaction, phishaction, spam_checked;
	inx_t *filters;
	struct smtp_inbound_prefs_t *next;
} smtp_inbound_prefs_t;

// The structure for storing recipient preferences on outbound data.
typedef struct {
	uint64_t usernum;
	stringer_t *domain;
	int_t ssl, importance;
	uint32_t sent_today, daily_send_limit, send_size_limit;
	smtp_recipients_t *recipients;
} smtp_outbound_prefs_t;

typedef struct {
	bool_t esmtp;
	bool_t submission;
	bool_t authenticated;
	bool_t suggested_eight_bit;

	size_t max_length;
	size_t num_recipients;
	size_t suggested_length;

	stringer_t *helo;
	stringer_t *mailfrom;

	uint32_t ip_address_v4;

	struct {
		int_t rbl;
		int_t spf;
		int_t dkim;
		int_t virus;
	} checked;

	smtp_message_t *message;
	smtp_inbound_prefs_t *in_prefs;
	smtp_outbound_prefs_t *out_prefs;

	bool_t bypass;

} smtp_session_t;

#endif

