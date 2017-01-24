
/**
 * @file /magma/web/portal/portal.h
 *
 * @brief	The portal web application.
 */

#ifndef MAGMA_WEB_PORTAL_H
#define MAGMA_WEB_PORTAL_H

#define MAGMA_PORTAL_VERSION	"1.02"

// Definitions for the json-rpc 2.0 protocol specification
enum {
	JSON_RPC_2_ERROR_PARSE_MALFORMED       = -32700, /* Parse error: request was not well formed; invalid JSON was received by the server. */
	JSON_RPC_2_ERROR_PARSE_ENCODING        = -32701, /* Parse error: request contained unsupported encoding. */
	JSON_RPC_2_ERROR_PARSE_CHAR            = -32702, /* Parse error: request contained invalid character for encoding. */
	JSON_RPC_2_ERROR_SERVER_REQUEST        = -32600, /* Server error: invalid xml-rpc request did not conform to spec. */
	JSON_RPC_2_ERROR_SERVER_METHOD_UNAVAIL = -32601, /* Server error: method does not exist or is not available. */
	JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS  = -32602, /* Server error: invalid method parameters. */
	JSON_RPC_2_ERROR_SERVER_INTERNAL       = -32603, /* Server error: internal xml-rpc error. */
	JSON_RPC_2_ERROR_APPLICATION           = -32500, /* Application error */
	JSON_RPC_2_ERROR_SYSTEM                = -32400, /* System error */
	JSON_RPC_2_ERROR_TRANSPORT             = -32300 /* Transport error */
};



enum {
	PORTAL_ENDPOINT_ACTION_INVALID = -1,
	PORTAL_ENDPOINT_ACTION_ADD = 1,
	PORTAL_ENDPOINT_ACTION_REMOVE = 2,
	PORTAL_ENDPOINT_ACTION_REPLACE = 3,
	PORTAL_ENDPOINT_ACTION_LIST = 4
};

enum {
	PORTAL_ENDPOINT_CONTEXT_INVALID = -1,
	PORTAL_ENDPOINT_CONTEXT_MAIL = 1,
	PORTAL_ENDPOINT_CONTEXT_CONTACTS = 2,
	PORTAL_ENDPOINT_CONTEXT_SETTINGS = 3,
	PORTAL_ENDPOINT_CONTEXT_HELP = 4
};

enum {
	PORTAL_ENDPOINT_MESSAGE_META = 1,
	PORTAL_ENDPOINT_MESSAGE_SOURCE = 2,
	PORTAL_ENDPOINT_MESSAGE_SECURITY = 4,
	PORTAL_ENDPOINT_MESSAGE_SERVER = 8,
	PORTAL_ENDPOINT_MESSAGE_HEADER = 16,
	PORTAL_ENDPOINT_MESSAGE_BODY = 32,
	PORTAL_ENDPOINT_MESSAGE_ATTACHMENTS = 64,
	PORTAL_ENDPOINT_MESSAGE_INFO = 128
};

enum {
	PORTAL_ENDPOINT_ERROR_MODE = 10000, // For requesting a method that is unavailable because of the session auth status.
	PORTAL_ENDPOINT_ERROR_REFERENCE = 10100, // For numeric references that cannot be correlated.
	PORTAL_ENDPOINT_ERROR_CONSTRAINT_VIOLATION = 10200, // If the request violates a system constraint.
	PORTAL_ENDPOINT_ERROR_INVALID_KEYWORD = 10300, // The parameter accepts a fixed list of keywords and the supplied value isn't on the list.
	PORTAL_ENDPOINT_ERROR_ILLEGAL_COMBINATION = 10300, // The combination of parameters is invalid.
	PORTAL_ENDPOINT_ERROR_SYSTEM_FLAG = 10400, // An attempt was made to modify a status flag that can only be changed by the system.
	PORTAL_ENDPOINT_ERROR_READ = 10500 // Unable to read/load/parse the requested data.
};


enum {
	PORTAL_ENDPOINT_ERROR_NONE = 0,
	PORTAL_ENDPOINT_ERROR_AUTH,
	PORTAL_ENDPOINT_ERROR_COOKIES,
	PORTAL_ENDPOINT_ERROR_AD,
	PORTAL_ENDPOINT_ERROR_ALERT_LIST,
	PORTAL_ENDPOINT_ERROR_ALERT_ACKNOWLEDGE,
	PORTAL_ENDPOINT_ERROR_ALIASES,
	PORTAL_ENDPOINT_ERROR_SCRAPE_ADD,
	PORTAL_ENDPOINT_ERROR_SCRAPE,
	PORTAL_ENDPOINT_ERROR_ATTACHMENTS_ADD,
	PORTAL_ENDPOINT_ERROR_ATTACHMENTS_PROGRESS,
	PORTAL_ENDPOINT_ERROR_ATTACHMENTS_REMOVE,
	PORTAL_ENDPOINT_ERROR_CONFIG_LOAD,
	PORTAL_ENDPOINT_ERROR_CONFIG_EDIT,
	PORTAL_ENDPOINT_ERROR_CONTACTS_ADD,
	PORTAL_ENDPOINT_ERROR_CONTACTS_EDIT,
	PORTAL_ENDPOINT_ERROR_CONTACTS_LIST,
	PORTAL_ENDPOINT_ERROR_CONTACTS_LOAD,
	PORTAL_ENDPOINT_ERROR_CONTACTS_MOVE,
	PORTAL_ENDPOINT_ERROR_CONTACTS_COPY,
	PORTAL_ENDPOINT_ERROR_CONTACTS_REMOVE,
	PORTAL_ENDPOINT_ERROR_FOLDERS_ADD,
	PORTAL_ENDPOINT_ERROR_FOLDERS_LIST,
	PORTAL_ENDPOINT_ERROR_FOLDERS_TAGS,
	PORTAL_ENDPOINT_ERROR_FOLDERS_REMOVE,
	PORTAL_ENDPOINT_ERROR_FOLDERS_RENAME,
	PORTAL_ENDPOINT_ERROR_MESSAGES_COMPOSE,
	PORTAL_ENDPOINT_ERROR_MESSAGES_COPY,
	PORTAL_ENDPOINT_ERROR_MESSAGES_FLAG,
	PORTAL_ENDPOINT_ERROR_MESSAGES_LIST,
	PORTAL_ENDPOINT_ERROR_MESSAGES_LOAD,
	PORTAL_ENDPOINT_ERROR_MESSAGES_MOVE,
	PORTAL_ENDPOINT_ERROR_MESSAGES_REMOVE,
	PORTAL_ENDPOINT_ERROR_MESSAGES_SEND,
	PORTAL_ENDPOINT_ERROR_MESSAGES_TAG,
	PORTAL_ENDPOINT_ERROR_MESSAGES_TAGS,
	PORTAL_ENDPOINT_ERROR_META,
	PORTAL_ENDPOINT_ERROR_SEARCH,
	PORTAL_ENDPOINT_ERROR_SETTINGS_IDENTITY,
	PORTAL_ENDPOINT_ERROR_SETTINGS_CHANGEPASS,
	PORTAL_ENDPOINT_ERROR_LOGOUT
};

/// config.c
json_t *  portal_config_collection(user_config_t *collection);
json_t *  portal_config_entry(user_config_entry_t *entry);
json_t *  portal_config_entry_flags(user_config_entry_t *entry);

/// contacts.c
json_t *  portal_contact_detail_flags(contact_detail_t *detail);
json_t *  portal_contact_details(contact_t *contact);

/// endpoint.c
void    portal_endpoint(connection_t *con);
void    portal_endpoint_ad(connection_t *con);
void    portal_endpoint_alert_acknowledge(connection_t *con);
void    portal_endpoint_alert_list(connection_t *con);
void    portal_endpoint_aliases(connection_t *con);
void    portal_endpoint_attachments_add(connection_t *con);
void    portal_endpoint_attachments_progress(connection_t *con);
void    portal_endpoint_attachments_remove(connection_t *con);
void    portal_endpoint_auth(connection_t *con);
int_t   portal_endpoint_compare(const void *compare, const void *command);
void    portal_endpoint_config_edit(connection_t *con);
void    portal_endpoint_config_load(connection_t *con);
void    portal_endpoint_contacts_add(connection_t *con);
void    portal_endpoint_contacts_copy(connection_t *con);
void    portal_endpoint_contacts_edit(connection_t *con);
void    portal_endpoint_contacts_list(connection_t *con);
void    portal_endpoint_contacts_load(connection_t *con);
void    portal_endpoint_contacts_move(connection_t *con);
void    portal_endpoint_contacts_remove(connection_t *con);
void    portal_endpoint_cookies(connection_t *con);
void    portal_endpoint_error(connection_t *con, int_t http_code, int_t error_code, chr_t *message);
void    portal_endpoint_folders_add(connection_t *con);
void    portal_endpoint_folders_list(connection_t *con);
void    portal_endpoint_folders_remove(connection_t *con);
void    portal_endpoint_folders_rename(connection_t *con);
void    portal_endpoint_folders_tags(connection_t *con);
void    portal_endpoint_logout(connection_t *con);
void    portal_endpoint_messages_compose(connection_t *con);
void    portal_endpoint_messages_copy(connection_t *con);
void    portal_endpoint_messages_flag(connection_t *con);
void    portal_endpoint_messages_list(connection_t *con);
void    portal_endpoint_messages_load(connection_t *con);
void    portal_endpoint_messages_move(connection_t *con);
void    portal_endpoint_messages_remove(connection_t *con);
void    portal_endpoint_messages_tags(connection_t *con);
void    portal_endpoint_messages_tag(connection_t *con);
void    portal_endpoint_response(connection_t *con, chr_t *format, ...);
void    portal_endpoint_scrape(connection_t *con);
void    portal_endpoint_scrape_add(connection_t *con);
void    portal_endpoint_search(connection_t *con);
void    portal_settings_identity(connection_t *con);
void    portal_meta(connection_t *con);
void    portal_endpoint_sort(void);
void	portal_upload(connection_t *con);
void    portal_endpoint_messages_send(connection_t *con);
void    portal_debug(connection_t *con);

/// mail.c
bool_t       portal_outbound_checks(uint64_t usernum, stringer_t *username, stringer_t *verification, stringer_t *from, size_t num_recipients, stringer_t *body_plain, stringer_t *body_html, chr_t **errmsg);
bool_t       portal_smtp_relay_message(stringer_t *from, inx_t *to, stringer_t *data, size_t send_size, chr_t **errmsg);
stringer_t * portal_smtp_create_data(inx_t *attachments, stringer_t *from, inx_t *to, inx_t *cc, inx_t *bcc, stringer_t *subject, stringer_t *body_plain, stringer_t *body_html);
stringer_t * portal_smtp_merge_headers(inx_t *headers, stringer_t *leading, stringer_t *trailing);

/// flags.c
json_t *  portal_message_flags_array(meta_message_t *meta);
json_t *  portal_message_tags_array(meta_message_t *meta);
int_t     portal_parse_flags(json_t *array, uint32_t *flags);

/// folders.c
void   portal_folder_contacts_add(connection_t *con, stringer_t *name, uint64_t parent);
void   portal_folder_contacts_remove(connection_t *con, uint64_t foldernum);
void   portal_folder_mail_add(connection_t *con, stringer_t *name, uint64_t parent);
void   portal_folder_mail_remove(connection_t *con, uint64_t foldernum);

/// messages.c
json_t *  portal_message_attachments(meta_message_t *meta, mail_message_t *data);
json_t *  portal_message_body(meta_message_t *meta, mail_message_t *data);
json_t *  portal_message_header(meta_message_t *meta, mail_message_t *data);
json_t *  portal_message_meta(meta_message_t *meta);
json_t *  portal_message_security(meta_message_t *meta);
json_t *  portal_message_server(meta_message_t *meta);
json_t *  portal_message_source(meta_message_t *meta);
json_t *  portal_message_info(meta_message_t *meta);

/// parse.c
inx_t * portal_parse_json_str_array (json_t *json, size_t *nout);
int_t   portal_parse_action(stringer_t *action);
int_t   portal_parse_context(stringer_t *context);
int_t   portal_parse_sections(json_t *array, uint32_t *sections);

/// portal.c
void   portal_print_login(connection_t *con, chr_t *message);
void   portal_process(connection_t *con);

#endif

