#ifndef DIME_DMTP_COMMANDS
#define DIME_DMTP_COMMANDS

#include <stdlib.h>
#include "dime/sds/sds.h" 

#define DMTP_MAX_ARGUMENT_NUM  3
#define DMTP_COMMANDS_NUM     14

typedef enum {
    DMTP_STARTTLS = 0,
    DMTP_HELO,
    DMTP_EHLO,
    DMTP_MODE,
    DMTP_RSET,
    DMTP_NOOP,
    DMTP_HELP,
    DMTP_QUIT,
    DMTP_MAIL,
    DMTP_RCPT,
    DMTP_DATA,
    DMTP_SGNT,
    DMTP_HIST,
    DMTP_VRFY,
    DMTP_COMMAND_INVALID
} dmtp_command_type_t;

typedef enum {
    DMTP_ARG_NONE = 0,
    DMTP_ARG_PLAIN,
    DMTP_ARG_ANGULAR_BRCKT,
    DMTP_ARG_SQUARE_BRCKT
} dmtp_argument_type_t;

typedef enum {
    DMTP_MODE_DMTP,
    DMTP_MODE_SMTP,
    DMTP_MODE_NONE
} dmtp_mode_t;

typedef enum {
    DMTP_PARSE_SUCCESS,
    DMTP_PARSE_INVALID_CALL,
    DMTP_PARSE_INTERNAL_ERROR,
    DMTP_PARSE_COMMAND_ERROR,
    DMTP_PARSE_ARGUMENT_ERROR
} dmtp_parse_state_t;


typedef struct {
    char const *arg_name;
    size_t arg_name_len;
    dmtp_argument_type_t type;
    size_t size;          //optional for when argument size is required to be constant
} dmtp_argument_t;

typedef struct {
    dmtp_command_type_t type;
    sds args[DMTP_MAX_ARGUMENT_NUM];
} dmtp_command_t;

typedef struct {
    char const *com_name;
    size_t com_name_len;
    dmtp_argument_t args[DMTP_MAX_ARGUMENT_NUM];
} dmtp_command_key_t;

extern dmtp_command_key_t dmtp_command_list[DMTP_COMMANDS_NUM];

//generic functions
dmtp_command_t *     dime_dmtp_command_create(dmtp_command_type_t type);
void                 dime_dmtp_command_destroy(dmtp_command_t *command);
sds                  dime_dmtp_command_format(dmtp_command_t *command);
dmtp_parse_state_t   dime_dmtp_command_parse(sds comm_line, dmtp_command_t **comm_struct);

//command specific formatting functions
sds dime_dmtp_command_starttls(sds host, dmtp_mode_t mode);
sds dime_dmtp_command_helo(sds host);
sds dime_dmtp_command_ehlo(sds host);
sds dime_dmtp_command_mode(void);
sds dime_dmtp_command_rset(void);
sds dime_dmtp_command_noop(sds optarg1, sds optarg2, sds optarg3);
sds dime_dmtp_command_help(void);
sds dime_dmtp_command_quit(void);
sds dime_dmtp_command_mail(sds from, sds fingerprint);
sds dime_dmtp_command_rcpt(sds to, sds fingerprint);
sds dime_dmtp_command_data(void);
sds dime_dmtp_command_sgnt_user(sds address, sds fingerprint);
sds dime_dmtp_command_sgnt_domain(sds domain, sds fingerprint);
sds dime_dmtp_command_hist(sds address, sds start, sds stop);
sds dime_dmtp_command_vrfy_user(sds address, sds fingerprint);
sds dime_dmtp_command_vrfy_domain(sds domain, sds fingerprint);



#endif
