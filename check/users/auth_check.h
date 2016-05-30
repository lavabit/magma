/**
 * @file /magma/check/users/auth_check.h
 *
 * @brief The account credentials used to check the authentication functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef AUTH_CHECK_H
#define AUTH_CHECK_H

struct {
	struct {
		stringer_t *username;
		stringer_t *password;
		stringer_t *salt;
	} creds;
	struct {
		stringer_t *key;
		stringer_t *token;
	} legacy;
	struct {
		stringer_t *master;
		stringer_t *verification;
	} stacie;
} auth_accounts = {
	{
		NULLER("magma"),
		NULLER("password"),
		NULLER("XjIw2JrWPUuJ4tlYR-58TMx7pVspxrTxrZ-LDZDaatyI5xtgPbv2-IaRRAl7mwGWuMEKRO-b5zM_ROjsn-OCNVdHKCp8JLEH7t0jzORxkDFDMdemHnHWxfwkFcML8CBamX46WeY0akGHT9xS9B8OrZuw1lYGAg_fGmWjcrWtJB4")
	}, {
		NULLER("VDLnzI9DMHUEvsCTaEO4fBqnfDMhI_95e7ecS1pUsdoOAzLQ-sKCF_fyV4HIgE9OdqWBPztzzY-45lo7ttLqSw"),
		NULLER("Qqfq1lUO5SNgIioOh4NkUhbDu0relatTG-E80wYNzdCF-1NOQk4Ge8Tt_ieocnfaqtZCj-5zezrcPFVgj7I-pQ")
	}, {
		NULLER("PfiXxnTrWAqeTs4VbZYTv6HMetm8FTkNbokW0jypIAOPhIFbjfLvUDb57mJwQGeOaGd9-l2CaBrjx2EO8RLRUA"),
		NULLER("wzRp634HPvvooyY-0YWy9FPAGvbBAIrnZYTK3lUZhN-t8jGazJzqqIja4p17aMpf0cQPHiDSJLuBfPY5t_tQ1Q")
	}
};

#endif

