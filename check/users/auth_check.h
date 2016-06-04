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
		stringer_t *nonce;
	} creds;
	struct {
		stringer_t *key;
		stringer_t *token;
	} legacy;
	struct {
		stringer_t *master;
		stringer_t *ephemeral;
		stringer_t *verification;
	} stacie;
} auth_accounts = {
	{
		NULLER("magma"),
		NULLER("password"),
		NULLER("XjIw2JrWPUuJ4tlYR-58TMx7pVspxrTxrZ-LDZDaatyI5xtgPbv2-IaRRAl7mwGWuMEKRO-b5zM_ROjsn-OCNVdHKCp8JLEH7t0jzORxkDFDMdemHnHWxfwkFcML8CBamX46WeY0akGHT9xS9B8OrZuw1lYGAg_fGmWjcrWtJB4"),
		NULLER("T89aOfkFoll8GjldXCV0X1iMdP0MRSJRE6INDB-ufetwMQmxYQ24XVl_YDsZ5Jaj7xjUrPSqo_LF9IWulbUP6uC05tv0bqn8h9sCY9gQ4dmJM0ubFU2gksr-1lzV0d2qM_HMY1Ml2r8LXTvhyxXLoYbv9MqBIWJTL82eaWU_zao")
	}, {
		NULLER("VDLnzI9DMHUEvsCTaEO4fBqnfDMhI_95e7ecS1pUsdoOAzLQ-sKCF_fyV4HIgE9OdqWBPztzzY-45lo7ttLqSw"),
		NULLER("Qqfq1lUO5SNgIioOh4NkUhbDu0relatTG-E80wYNzdCF-1NOQk4Ge8Tt_ieocnfaqtZCj-5zezrcPFVgj7I-pQ")
	}, {
		NULLER("PfiXxnTrWAqeTs4VbZYTv6HMetm8FTkNbokW0jypIAOPhIFbjfLvUDb57mJwQGeOaGd9-l2CaBrjx2EO8RLRUA"),
		NULLER("LlgRSdjxOS_fBEGnSY-wSVpzcSuJotbRq-fhb2wxnJNpnh9gzlGA8LrSFyQuuTkkd3Hzf9dn4pip9xddWiIwzQ"),
		NULLER("wzRp634HPvvooyY-0YWy9FPAGvbBAIrnZYTK3lUZhN-t8jGazJzqqIja4p17aMpf0cQPHiDSJLuBfPY5t_tQ1Q")
	}
};

#endif

