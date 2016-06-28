#ifndef DMTP_H
#define DMTP_H

#include "dime/signet/signet.h"
#include "dime/signet-resolver/mrec.h"
#include "dime/common/error.h"
#include "dime/signet-resolver/signet-ssl.h"


#define DMTP_PORT      26
#define DMTP_PORT_DUAL 25

#define DMTP_V1_CIPHER_LIST "ECDHE-RSA-AES256-GCM-SHA384"

#define DMTP_MAX_MX_RETRIES 3

#define DMTP_LINE_BUF_SIZE 4096


typedef enum {
    dmtp_mode_unknown = 0,
    dmtp_mode_dual = 1,
    dmtp_mode_dmtp = 2,
    dmtp_mode_smtp = 3,
    dmtp_mode_esmtp = 4
} dmtp_mode_t;


typedef struct {
    char *domain;           ///< The name of the dark domain underlying the DMTP connection.
    char *dx;               ///< The canonical name of the DX that we're connected to.
    SSL *con;               ///< The handle to this DMTP session's underlying SSL connection.
    dime_record_t *drec;    ///< The DIME management record associated with this dark domain.
    dmtp_mode_t mode;       ///< The current mode of this connection (if made through dual mode).
    unsigned int active;    ///< Boolean flag: whether or not this session is active.

    int _fd;
    unsigned char _inbuf[DMTP_LINE_BUF_SIZE + 1];
    size_t _inpos;
} dmtp_session_t;


typedef enum {
    return_type_default = 0,
    return_type_full = 1,
    return_type_display = 2,
    return_type_header = 3
} dmtp_mail_rettype_t;

typedef enum {
    data_type_default = 0,
    data_type_7bit = 1,
    data_type_8bit = 2
} dmtp_mail_datatype_t;


// High-level interfaces built on DMTP.
PUBLIC_FUNC_DECL(signet_t *,       get_signet,            const char *name, const char *fingerprint, int use_cache);

// General session control routines.
PUBLIC_FUNC_DECL(dmtp_session_t *, sgnt_resolv_dmtp_connect,          const char *domain, int force_family);
PUBLIC_FUNC_DECL(void,             sgnt_resolv_destroy_dmtp_session,  dmtp_session_t *session);
PUBLIC_FUNC_DECL(dmtp_session_t *, dx_connect_standard,   const char *host, const char *domain, int force_family, dime_record_t *dimerec);
PUBLIC_FUNC_DECL(dmtp_session_t *, dx_connect_dual,       const char *host, const char *domain, int force_family, dime_record_t *dimerec, int failover);
PUBLIC_FUNC_DECL(int,              verify_dx_certificate, dmtp_session_t *session);

// Message flow.
PUBLIC_FUNC_DECL(int,              sgnt_resolv_dmtp_ehlo,             dmtp_session_t *session, const char *domain);
PUBLIC_FUNC_DECL(int,              sgnt_resolv_dmtp_mail_from,        dmtp_session_t *session, const char *origin, size_t msgsize, dmtp_mail_rettype_t rettype, dmtp_mail_datatype_t dtype);
PUBLIC_FUNC_DECL(int,              sgnt_resolv_dmtp_rcpt_to,          dmtp_session_t *session, const char *domain);
PUBLIC_FUNC_DECL(char *,           sgnt_resolv_dmtp_data,             dmtp_session_t *session, void *msg, size_t msglen);

// DMTP-protocol specific client commands.
PUBLIC_FUNC_DECL(char *,           sgnt_resolv_dmtp_get_signet,       dmtp_session_t *session, const char *signame, const char *fingerprint);
PUBLIC_FUNC_DECL(int,              sgnt_resolv_dmtp_verify_signet,    dmtp_session_t *session, const char *signame, const char *fingerprint, char **newprint);
PUBLIC_FUNC_DECL(char *,           sgnt_resolv_dmtp_history,          dmtp_session_t *session, const char *signame, const char *startfp, const char *endfp);
PUBLIC_FUNC_DECL(char *,           sgnt_resolv_dmtp_stats,            dmtp_session_t *session, const unsigned char *secret);

// Dual mode/SMTP helper commands.
PUBLIC_FUNC_DECL(dmtp_mode_t,      sgnt_resolv_dmtp_str_to_mode,      const char *modestr);
PUBLIC_FUNC_DECL(dmtp_mode_t,      sgnt_resolv_dmtp_get_mode,         dmtp_session_t *session);
PUBLIC_FUNC_DECL(int,              sgnt_resolv_dmtp_noop,             dmtp_session_t *session);
PUBLIC_FUNC_DECL(int,              sgnt_resolv_dmtp_reset,            dmtp_session_t *session);
PUBLIC_FUNC_DECL(char *,           sgnt_resolv_dmtp_help,             dmtp_session_t *session);
PUBLIC_FUNC_DECL(int,              sgnt_resolv_dmtp_quit,             dmtp_session_t *session, int do_close);


// Internal network and parsing functions.
char *      _sgnt_resolv_read_dmtp_line(dmtp_session_t *session, int *overflow, unsigned short *rcode, int *multiline);
char *      _sgnt_resolv_read_dmtp_multiline(dmtp_session_t *session, int *overflow, unsigned short *rcode);
char *      _sgnt_resolv_parse_line_code(const char *line, unsigned short *rcode, int *multiline);
dmtp_mode_t _sgnt_resolv_dmtp_str_to_mode(const char *modestr);
dmtp_mode_t _sgnt_resolv_dmtp_initiate_starttls(dmtp_session_t *session, const char *dxname);
int         _sgnt_resolv_dmtp_expect_banner(dmtp_session_t *session);
int         _sgnt_resolv_dmtp_issue_command(dmtp_session_t *session, const char *cmd);
char *      _sgnt_resolv_dmtp_send_and_read(dmtp_session_t *session, const char *cmd, unsigned short *rcode);
int         _sgnt_resolv_dmtp_write_data(dmtp_session_t *session, const void *buf, size_t buflen);

#endif
