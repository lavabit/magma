
/**
 * @file /magma/network/http.h
 *
 * @brief	The HTTP server control structures.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_NETWORK_HTTP_H
#define MAGMA_NETWORK_HTTP_H

typedef enum {
	HTTP_METHOD_NONE,
	HTTP_METHOD_GET,
	HTTP_METHOD_POST,
	HTTP_METHOD_PUT,
	HTTP_METHOD_DELETE,
	HTTP_METHOD_HEAD,
	HTTP_METHOD_TRACE,
	HTTP_METHOD_OPTIONS,
	HTTP_METHOD_CONNECT,
	HTTP_METHOD_UNSUPPORTED
} http_method_t;

typedef enum {
	HTTP_DATA_ANY,
	HTTP_DATA_HEADER,
	HTTP_DATA_GET,
	HTTP_DATA_POST
} HTTP_DATA;

enum {
	HTTP_MERGED,
	HTTP_PORTAL
};

typedef struct {
	HTTP_DATA source;
	stringer_t *name, *value;
} http_data_t;

typedef struct {
	stringer_t *location, *resource, *type;
	struct http_content_t *next;
} http_content_t;

typedef struct {
	xmlDocPtr doc_obj;
	http_content_t *content;
	xmlParserCtxtPtr doc_ctx;
	xmlXPathContextPtr xpath_ctx;
} http_page_t;

typedef struct {

	struct {
		int_t cookie;
		int_t connection;
	} response;

	session_t *session;
	http_method_t method;
	inx_t *pairs, *headers;
	int_t mode, merged, port;
	stringer_t *host, *location, *cookie, *agent, *body;

	union {
		struct {
			uint64_t id;
			json_t *request, *params;
		} portal;
	};

} __attribute__ ((__packed__)) http_session_t;

#endif

