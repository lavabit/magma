
/**
 * @file /magma/servers/http/http.h
 *
 * @brief	The entry point for the HTTP server module.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_SERVERS_HTTP_H
#define MAGMA_SERVERS_HTTP_H

enum {
	HTTP_CONNECTION_CLOSE = -1,
	HTTP_CONNECTION_NEUTRAL = 0,
	HTTP_CONNECTION_KEEPALIVE = 1
};

enum {
	HTTP_COOKIE_DELETE = -1,
	HTTP_COOKIE_NEUTRAL = 0,
	HTTP_COOKIE_SET = 1
};

enum {
	HTTP_READY = 0,

	HTTP_PARSE_HEADER = 1,
	HTTP_PARSE_PAIRS = 2,
	HTTP_PARSE_COOKIE = 3,

	HTTP_READ_BODY = 8,

	HTTP_RESPOND = 10,
	HTTP_COMPLETE = 12,

	HTTP_OK = 200,

	HTTP_ERROR_400 = 400,
	HTTP_ERROR_401 = 401,
	HTTP_ERROR_403 = 403,
	HTTP_ERROR_404 = 404,
	HTTP_ERROR_405 = 405,
	HTTP_ERROR_422 = 422,
	HTTP_ERROR_500 = 500,
	HTTP_ERROR_501 = 501,

	HTTP_CLOSE = 1000
};

/// content.c
bool_t            http_content_load_directory(int_t template, chr_t *directory);
bool_t            http_content_load_fonts(void);
bool_t            http_content_refresh(void);
bool_t            http_content_start(void);
void              http_content_stop(void);
void              http_free_content(http_content_t *page);
http_content_t *  http_get_static(stringer_t *location);
http_content_t *  http_get_template(chr_t *location);
bool_t             http_load_file(int_t template, chr_t *filename);
void              http_page_free(http_page_t *page);
http_page_t *     http_page_get(chr_t *location);

/// data.c
void           http_data_free(http_data_t *data);
http_data_t *  http_data_get(connection_t *con, HTTP_DATA source, chr_t *name);
http_data_t *  http_data_header_parse_line(chr_t *buf, size_t len);
http_data_t *  http_data_header_parse(connection_t *con);
void           http_data_value_decode(stringer_t *string);
int_t          http_data_value_parse(connection_t *con, HTTP_DATA source, placer_t pair);

/// errors.c
void   http_print_301(connection_t *con, chr_t *location, int_t ssl);
void   http_print_400(connection_t *con);
void   http_print_403(connection_t *con);
void   http_print_404(connection_t *con);
void   http_print_405(connection_t *con);
void   http_print_500(connection_t *con);
void   http_print_500_log(connection_t *con, chr_t *logmsg);
void   http_print_501(connection_t *con);

/// http.c
void   http_body(connection_t *con);
void   http_close(connection_t *con);
void   http_init(connection_t *con);
void   http_process(connection_t *con);
void   http_requeue(connection_t *con);

//// TODO: The header and body parsers need limits on how many headers/bytes they will accept.
/// parse.c
void    http_parse_context(connection_t *con, stringer_t *application, stringer_t *path);
void    http_parse_header(connection_t *con);
void    http_parse_method(connection_t *con);
int_t   http_parse_origin(stringer_t *s, placer_t *output);
void    http_parse_pairs(connection_t *con);
placer_t get_header_value_noopt(stringer_t *vstring);
placer_t get_header_opt(stringer_t *vstring, stringer_t *optname);
bool_t	 multipart_get_boundary(connection_t *con, placer_t *output);

/// response.c
void          http_response(connection_t *con);
stringer_t *  http_response_allow_cross(connection_t *con);
stringer_t *  http_response_connection(connection_t *con, int_t force);
stringer_t *  http_response_cookie(connection_t *con);
void          http_response_header(connection_t *con, int_t status, stringer_t *type, size_t len);
void          http_response_options(connection_t *con);
chr_t *       http_response_status(int_t status);

/// sessions.c
void   http_session_destroy(connection_t *con);
void   http_session_reset(connection_t *con);

#endif
