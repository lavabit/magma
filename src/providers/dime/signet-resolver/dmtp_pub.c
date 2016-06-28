#include "dime/signet-resolver/dmtp.h"


signet_t *get_signet(const char *name, const char *fingerprint, int use_cache) {
    PUBLIC_FUNC_IMPL(get_signet, name, fingerprint, use_cache);
}

dmtp_session_t *sgnt_resolv_dmtp_connect(const char *domain, int force_family) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_connect, domain, force_family);
}

void sgnt_resolv_destroy_dmtp_session(dmtp_session_t *session) {
    PUBLIC_FUNC_IMPL_VOID(sgnt_resolv_destroy_dmtp_session, session);
}

dmtp_session_t *dx_connect_standard(const char *host, const char *domain, int force_family, dime_record_t *dimerec) {
    PUBLIC_FUNC_IMPL(dx_connect_standard, host, domain, force_family, dimerec);
}

dmtp_session_t *dx_connect_dual(const char *host, const char *domain, int force_family, dime_record_t *dimerec, int failover) {
    PUBLIC_FUNC_IMPL(dx_connect_dual, host, domain, force_family, dimerec, failover);
}

int verify_dx_certificate(dmtp_session_t *session) {
    PUBLIC_FUNC_IMPL(verify_dx_certificate, session);
}

char * sgnt_resolv_dmtp_get_signet(dmtp_session_t *session, const char *signame, const char *fingerprint) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_get_signet, session, signame, fingerprint);
}

int sgnt_resolv_dmtp_ehlo(dmtp_session_t *session, const char *domain) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_ehlo, session, domain);
}

int sgnt_resolv_dmtp_mail_from(dmtp_session_t *session, const char *origin, size_t msgsize, dmtp_mail_rettype_t rettype, dmtp_mail_datatype_t dtype) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_mail_from, session, origin, msgsize, rettype, dtype);
}

int sgnt_resolv_dmtp_rcpt_to(dmtp_session_t *session, const char *domain) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_rcpt_to, session, domain);
}

char * sgnt_resolv_dmtp_data(dmtp_session_t *session, void *msg, size_t msglen) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_data, session, msg, msglen);
}

int sgnt_resolv_dmtp_verify_signet(dmtp_session_t *session, const char *signame, const char *fingerprint, char **newprint) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_verify_signet, session, signame, fingerprint, newprint);
}

char * sgnt_resolv_dmtp_history(dmtp_session_t *session, const char *signame, const char *startfp, const char *endfp) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_history, session, signame, startfp, endfp);
}

char * sgnt_resolv_dmtp_stats(dmtp_session_t *session, const unsigned char *secret) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_stats, session, secret);
}

dmtp_mode_t sgnt_resolv_dmtp_str_to_mode(const char *modestr) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_str_to_mode, modestr);
}

dmtp_mode_t sgnt_resolv_dmtp_get_mode(dmtp_session_t *session) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_get_mode, session);
}

int sgnt_resolv_dmtp_noop(dmtp_session_t *session) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_noop, session);
}

int sgnt_resolv_dmtp_reset(dmtp_session_t *session) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_reset, session);
}

char * sgnt_resolv_dmtp_help(dmtp_session_t *session) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_help, session);
}

int sgnt_resolv_dmtp_quit(dmtp_session_t *session, int do_close) {
    PUBLIC_FUNC_IMPL(sgnt_resolv_dmtp_quit, session, do_close);
}
