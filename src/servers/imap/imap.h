
/**
 * @file /magma/servers/imap/imap.h
 *
 * @brief The entry point for the IMAP server module.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_SERVERS_IMAP_H
#define MAGMA_SERVERS_IMAP_H

#define IMAP_ARRAY_RECURSION_LIMIT 16
#define IMAP_SEARCH_RECURSION_LIMIT 16
#define IMAP_FOLDER_RECURSION_LMIIT 16

// IMAP Argument types.
#define IMAP_ARGUMENT_TYPE_EMPTY 0
#define IMAP_ARGUMENT_TYPE_ARRAY 1
#define IMAP_ARGUMENT_TYPE_ASTRING 2
#define IMAP_ARGUMENT_TYPE_QSTRING 3
#define IMAP_ARGUMENT_TYPE_NSTRING 4
#define IMAP_ARGUMENT_TYPE_LITERAL 5

// IMAP Body parts.
#define IMAP_FETCH_BODY_HEADER_FIELDS_NOT 1
#define IMAP_FETCH_BODY_HEADER_FIELDS 2
#define IMAP_FETCH_BODY_HEADER 3
#define IMAP_FETCH_BODY_TEXT 4
#define IMAP_FETCH_BODY_MIME 5
#define IMAP_FETCH_BODY_PART 6

// IMAP Flags actions.
#define IMAP_FLAG_SILENT 1
#define IMAP_FLAG_ADD 2
#define IMAP_FLAG_REMOVE 4
#define IMAP_FLAG_REPLACE 8

/// commands.c
int_t   imap_compare(const void *compare, const void *command);
void    imap_process(connection_t *con);
void    imap_requeue(connection_t *con);
void    imap_sort(void);

/// fetch_response.c
imap_fetch_response_t *  imap_fetch_response_add(imap_fetch_response_t *response, stringer_t *key, stringer_t *value);
void                     imap_fetch_response_free(imap_fetch_response_t *response);

/// fetch.c
inx_t *                   imap_duplicate_messages(inx_t *messages);
imap_fetch_response_t *   imap_fetch_body(array_t *outer, array_t *partial, connection_t *con, meta_message_t *meta,mail_message_t **message, stringer_t **header, imap_fetch_response_t *output);
stringer_t *              imap_fetch_body_header(placer_t header, imap_arguments_t *array, int_t not);
stringer_t *              imap_fetch_body_mime(placer_t header);
mail_mime_t *             imap_fetch_body_part(mail_message_t *message, placer_t portion);
placer_t                  imap_fetch_body_portion(stringer_t *part);
stringer_t *              imap_fetch_body_tag(stringer_t *tag, array_t *items);
stringer_t *              imap_fetch_bodystructure(mail_mime_t *mime);
stringer_t *              imap_fetch_envelope(stringer_t *header);
void                      imap_fetch_free_items(imap_fetch_dataitems_t *items);
imap_fetch_response_t *   imap_fetch_message(connection_t *con, meta_message_t *meta, imap_fetch_dataitems_t *items);
int_t                     imap_fetch_parse_partial(stringer_t *partial, size_t *start, size_t *length);
stringer_t *              imap_fetch_return_header(connection_t *con, meta_message_t *meta, mail_message_t **message, stringer_t **header, imap_fetch_response_t *output);
mail_message_t *          imap_fetch_return_message(connection_t *con, meta_message_t *meta, mail_message_t **message, stringer_t **header, imap_fetch_response_t *output);
mail_mime_t *             imap_fetch_return_mime(connection_t *con, meta_message_t *meta, mail_message_t **message, stringer_t **header, imap_fetch_response_t *output);
stringer_t *              imap_fetch_return_text(connection_t *con, meta_message_t *meta, mail_message_t **message, stringer_t **header, imap_fetch_response_t *output);
inx_t *                   imap_narrow_messages(inx_t *messages, uint64_t selected, stringer_t *range, int_t uid);
imap_fetch_dataitems_t *  imap_parse_dataitems(imap_arguments_t *arguments);
int_t                     imap_valid_sequence(stringer_t *range);

/// flags.c
int_t      imap_flag_action(stringer_t *string);
uint32_t   imap_flag_parse(void *ptr, int_t type);
uint32_t   imap_get_flag(stringer_t *string);
void       imap_update_flags(new_meta_user_t *user, inx_t *messages, uint64_t foldernum, int_t action, uint32_t flags);

/// folders.c
int_t         imap_count_folder_levels(stringer_t *name);
int_t         imap_folder_compare(stringer_t *name, stringer_t *compare);
int_t         imap_folder_create(uint64_t usernum, inx_t *folders, stringer_t *name);
stringer_t *  imap_folder_name_escaped(inx_t *folders, meta_folder_t *active);
int_t         imap_folder_remove(uint64_t usernum, inx_t *folders, inx_t *messages, stringer_t *name);
int_t         imap_folder_rename(uint64_t usernum, inx_t *folders, stringer_t *original, stringer_t *rename);
int_t         imap_folder_status(inx_t *folders, inx_t *messages, stringer_t *name, imap_folder_status_t *status);
inx_t *       imap_narrow_folders(inx_t *folders, stringer_t *reference, stringer_t *mailbox);
uint64_t      imap_next_folder_order(inx_t *folders, uint64_t parent);
bool_t        imap_valid_folder_name(stringer_t *name);

/// imap.c
void   imap_append(connection_t *con);
void   imap_capability(connection_t *con);
void   imap_check(connection_t *con);
void   imap_close(connection_t *con);
void   imap_copy(connection_t *con);
void   imap_create(connection_t *con);
void   imap_delete(connection_t *con);
void   imap_examine(connection_t *con);
void   imap_expunge(connection_t *con);
void   imap_fetch(connection_t *con);
void   imap_id(connection_t *con);
void   imap_idle(connection_t *con);
void   imap_init(connection_t *con);
void   imap_invalid(connection_t *con);
void   imap_list(connection_t *con);
void   imap_login(connection_t *con);
void   imap_logout(connection_t *con);
void   imap_lsub(connection_t *con);
void   imap_noop(connection_t *con);
void   imap_rename(connection_t *con);
void   imap_search(connection_t *con);
void   imap_select(connection_t *con);
void   imap_starttls(connection_t *con);
void   imap_status(connection_t *con);
void   imap_store(connection_t *con);
void   imap_subscribe(connection_t *con);
void   imap_unsubscribe(connection_t *con);

/// messages.c
int_t   imap_append_message(connection_t *con, meta_folder_t *folder, uint32_t flags, stringer_t *message, uint64_t *outnum);
int_t   imap_message_copier(connection_t *con, meta_message_t *message, uint64_t target, uint64_t *outnum);
int_t   imap_message_expunge(connection_t *con, meta_message_t *message);

/// output.c
stringer_t *  imap_build_array(chr_t *format, ...);
stringer_t *  imap_build_array_isliteral(placer_t data);

/// parse_address.c
stringer_t *  imap_parse_address(stringer_t *address);
placer_t      imap_parse_address_breaker(stringer_t *address, uint32_t part);
stringer_t *  imap_parse_address_part(placer_t input);
void          imap_parse_address_put(stringer_t *buffer, chr_t c);

/// parse.c
int_t               imap_command_parser(connection_t *con);
imap_arguments_t *  imap_get_ar_ar(imap_arguments_t *array, size_t element);
void *              imap_get_ptr(imap_arguments_t *array, size_t element);
stringer_t *        imap_get_st_ar(imap_arguments_t *array, size_t element);
int_t               imap_get_type_ar(imap_arguments_t *array, size_t element);
int_t               imap_parse_arguments(connection_t *con, chr_t **start, size_t *length);
int_t               imap_parse_array(int_t recursion, connection_t *con, imap_arguments_t **array, chr_t **start, size_t *length);
int_t               imap_parse_astring(stringer_t **output, chr_t **start, size_t *length);
int_t               imap_parse_literal(connection_t *con, stringer_t **output, chr_t **start, size_t *length);
int_t               imap_parse_nstring(stringer_t **output, chr_t **start, size_t *length, chr_t type);
int_t               imap_parse_qstring(stringer_t **output, chr_t **start, size_t *length);

/// range.c
stringer_t *  imap_range_build(size_t length, uint64_t *numbers);

/// search.c
int_t    imap_search_flag(uint32_t status, uint32_t flag, int_t has);
inx_t *  imap_search_messages(connection_t *con);
int_t    imap_search_messages_body(new_meta_user_t *user, mail_message_t **data, meta_message_t *active, stringer_t *value);
int_t    imap_search_messages_date(new_meta_user_t *user, mail_message_t **data, stringer_t **header, meta_message_t *active, stringer_t *date, int_t internal, int_t expected);
int_t    imap_search_messages_date_compare(stringer_t *one, stringer_t *two);
int_t    imap_search_messages_header(new_meta_user_t *user, mail_message_t **data, stringer_t **header, meta_message_t *active, stringer_t *field, stringer_t *value);
int_t    imap_search_messages_inner(new_meta_user_t *user, mail_message_t **message, stringer_t **header, meta_message_t *current, imap_arguments_t *array, unsigned recursion);
int_t    imap_search_messages_range(meta_message_t *active, stringer_t *range, int_t uid);
int_t    imap_search_messages_size(meta_message_t *active, stringer_t *value, int_t expected);
int_t    imap_search_messages_text(new_meta_user_t *user, mail_message_t **data, meta_message_t *active, stringer_t *value);

/// sessions.c
void    imap_session_destroy(connection_t *con);
int_t   imap_session_update(connection_t *con);

#endif
