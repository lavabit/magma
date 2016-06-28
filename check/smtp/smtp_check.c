
/**
 * @file /check/smtp/smtp_check.c
 *
 * @brief SMTP interface test functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

START_TEST (check_smtp_inbound_creation_s) {

	//auth_t *cred;
	char *errmsg = NULL;

	log_unit("%-64.64s%10.10s\n", "SMTP / INBOUND / CREATION / SINGLE THREADED:", "SKIPPED");
	return;

	if (!status()) {
		log_test("SMTP / INBOUND / CREATION / SINGLE THREADED:", errmsg);
		return;
	}

//	if ((cred = credential_alloc_mail(CONSTANT("ladar")))) {
//		errmsg = "Credential creation should have failed but didn't.";
//	}
//
//	if (cred) {
//		credential_free(cred);
//		cred = NULL;
//	}
//
//	if ((cred = credential_alloc_mail(CONSTANT("ladar@")))) {
//		errmsg = "Credential creation should have failed but didn't.";
//	}
//
//	if (cred) {
//		credential_free(cred);
//		cred = NULL;
//	}
//
//	if ((cred = credential_alloc_mail(CONSTANT("@lavabit.com")))) {
//		errmsg = "Credential creation should have failed but didn't.";
//	}
//
//	if (cred) {
//		credential_free(cred);
//		cred = NULL;
//	}
//
//	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("ladar+tag@lavabit.com")))) {
//		errmsg = "Credential creation failed.";
//	}
//	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("ladar@lavabit.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("lavabit.com")))) {
//		errmsg = "The credential password hash doesn't match the expected value.";
//	}
//
//	if (cred) {
//		credential_free(cred);
//	}
//
//	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("LADAR+TAG@LAVABIT.COM")))) {
//		errmsg = "Credential creation failed.";
//	}
//	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("ladar@lavabit.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("lavabit.com")))) {
//		errmsg = "The credential password hash doesn't match the expected value.";
//	}
//
//	if (cred) {
//		credential_free(cred);
//	}
//
//	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("ladar+tag@nerdshack.com")))) {
//		errmsg = "Credential creation failed.";
//	}
//	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("ladar@nerdshack.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("nerdshack.com")))) {
//		errmsg = "The credential password hash doesn't match the expected value.";
//	}
//
//	if (cred) {
//		credential_free(cred);
//	}
//
//	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("LADAR+TAG@NERDSHACK.COM")))) {
//		errmsg = "Credential creation failed.";
//	}
//	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("ladar@nerdshack.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("nerdshack.com")))) {
//		errmsg = "The credential password hash doesn't match the expected value.";
//	}
//
//	if (cred) {
//		credential_free(cred);
//	}
//
//	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("ladar+tag@mailshack.com")))) {
//		errmsg = "Credential creation failed.";
//	}
//	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("ladar@mailshack.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("mailshack.com")))) {
//		errmsg = "The credential password hash doesn't match the expected value.";
//	}
//
//	if (cred) {
//		credential_free(cred);
//	}
//
//	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("LADAR+TAG@MAILSHACK.COM")))) {
//		errmsg = "Credential creation failed.";
//	}
//	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("ladar@mailshack.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("mailshack.com")))) {
//		errmsg = "The credential password hash doesn't match the expected value.";
//	}
//
//	if (cred) {
//		credential_free(cred);
//	}
//
//	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("user+tag@domain.com")))) {
//		errmsg = "Credential creation failed.";
//	}
//	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("user@domain.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("domain.com")))) {
//		errmsg = "The credential password hash doesn't match the expected value.";
//	}
//
//	if (cred) {
//		credential_free(cred);
//	}
//
//	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("USER+TAG@DOMAIN.COM")))) {
//		errmsg = "Credential creation failed.";
//	}
//	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("user@domain.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("domain.com")))) {
//		errmsg = "The credential password hash doesn't match the expected value.";
//	}
//
//	if (cred) {
//		credential_free(cred);
//	}

	log_test("SMTP / INBOUND / CREATION / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, errmsg);
} END_TEST

START_TEST (check_smtp_authentication_s) {

	//auth_t *cred;
	char *errmsg = NULL;

	log_unit("%-64.64s%10.10s\n", "SMTP / AUTHENTICATION / SINGLE THREADED:", "SKIPPED");
	return;

	if (!status()) {
		log_test("USERS / SMTP / AUTHENTICATION / SINGLE THREADED:", errmsg);
		return;
	}

//	if (!(cred = credential_alloc_auth(CONSTANT("ladar")))) {
//		errmsg = "Credential allocation failed.";
//	}
//	else if(!credential_calc_auth(cred, CONSTANT("test"), NULL)) {
//		errmsg = "Credential calculation failed.";
//	}
//	else if (st_cmp_cs_eq(cred->auth.password,
//	CONSTANT("46c3c0f5c777aacbdb0c25b14d6889b98efa62fa0ae551ec067d7aa126392805e3e3a2ce07d36df7e715e24f35c88105fff5a9eebff0532f990644cf07a4751f"))) {
//		errmsg = "The credential password hash doesn't match the expected value.";
//	}
//
//	if (cred) {
//		credential_free(cred);
//		cred = NULL;
//	}
//
//	if (!errmsg && !(cred = credential_alloc_auth(CONSTANT("ladar@lavabit.com")))) {
//		errmsg = "Credential allocation failed.";
//	}
//	else if(!errmsg && !credential_calc_auth(cred, CONSTANT("test"), NULL)) {
//		errmsg = "Credential calculation failed.";
//	}
//	else if (!errmsg && st_cmp_cs_eq(cred->auth.password,
//	CONSTANT("46c3c0f5c777aacbdb0c25b14d6889b98efa62fa0ae551ec067d7aa126392805e3e3a2ce07d36df7e715e24f35c88105fff5a9eebff0532f990644cf07a4751f"))) {
//		errmsg = "The credential password hash doesn't match the expected value.";
//	}
//
//	if (cred) {
//		credential_free(cred);
//	}

	log_test("USERS / SMTP / AUTHENTICATION / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, errmsg);
} END_TEST

Suite * suite_check_smtp(void) {

	TCase *tc;
	Suite *s = suite_create("\tSMTP");

	testcase(s, tc, "SMTP Inbound Processing/S", check_smtp_inbound_creation_s);
	testcase(s, tc, "SMTP Authentication/S", check_smtp_authentication_s);

	return s;
}


