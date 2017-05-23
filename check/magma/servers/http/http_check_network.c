
/**
 * @file /magma/check/magma/servers/http/http_check_network.c
 *
 * @brief Functions used to test HTTP connections over a network connection.
 *
 */

#include "magma_check.h"

/**
 * @brief	Reads lines from client until an empty line is reached.
 * @param	client	A client_t* containing response lines to read.
 * @return	True if an empty line was reached, false otherwise.
 */
bool_t check_http_read_to_empty(client_t *client) {

	while (client_read_line(client) >= 0) {
		if (pl_length_get(client->line) == 2) return true; // contains only "\r\n"
	}
	return false;
}

/**
 * @brief 	Reads lines from client and returns the value of Content-Length.
 *
 * This function reads lines from the passed client until it finds the Content-Length
 * line, at which point it parses the value and returns it.
 *
 * @param 	client	A client_t containing the response of an HTTP request.
 * @return 	The value of Content-Length in the HTTP message header.
 */
int32_t check_http_content_length_get(client_t *client) {

	placer_t cl_placer = pl_null();
	size_t location = 0, content_length = 0;

	/// HIGH: This logic is wrong.
	while (st_cmp_ci_starts(&(client->line), NULLER("Content-Length:")) != 0) {
		if (client_read_line(client) <= 2) return 0;
	}

	if (!st_search_chr(&(client->line), ' ', &location)) {
		return -1;
	}
	else if (pl_empty(cl_placer = pl_init(pl_data_get(client->line) + location, pl_length_get(client->line) - location))) {
		return -1;
	}
	else if (!pl_inc(&cl_placer, pl_length_get(client->line) - location) || !(cl_placer.length = pl_length_get(cl_placer)-2)) {
		return -1;
	}
	else if (!size_conv_bl(pl_char_get(cl_placer), pl_length_get(cl_placer), &content_length)) {
		return -1;
	}

	return content_length;
}

/**
 * @brief 	Reads lines from the client until the end of message, checking if it matches content_length.
 *
 * @param 	client			A client_t* that should be connected to an HTTP server and have read to the beginning
 * 							of the body of a response.
 * @param	content_length	A unint32_t containing the expected size of the message body.
 * @return	True if the message body size matches content_length, false if not.
 */
bool_t check_http_content_length_test(client_t *client, uint32_t content_length, stringer_t *errmsg) {

	uint32_t total = 0;

	while (total < content_length) {
		total += client_read_line(client);
	}

	return (total == content_length);
}

/**
 * @brief	Reads lines from the client, checking if each of the options are present.
 *
 * @param	client	A client_t* that should be connected to an HTTP server and has had the OPTIONS request
 * 					submitted already.
 * @param	options	An array of chr_t* containing the options that should be in the response.
 * @return	True if all of the options were present, false otherwise.
 */
bool_t check_http_options(client_t *client, chr_t **options, uint32_t options_count, stringer_t *errmsg) {

	bool_t *opts_present = mm_alloc(options_count * sizeof(bool_t));
	for (size_t i = 0; i < options_count; i++) opts_present[i] = false;

	while (st_cmp_ci_starts(&client->line, NULLER("\r\n")) != 0) {
		for (size_t i = 0; i < options_count; i++) {
			if (st_cmp_cs_starts(&client->line, NULLER(options[i]))) {

				opts_present[i] = true;
			}
		}
		client_read_line(client);
	}

	for (size_t i = 0; i < options_count; i++) {
		if (!opts_present[i]) {

			st_sprint(errmsg, "One of the HTTP options was not present in the response. { option = \"%s\" }", options[i]);
			mm_free(opts_present);
			return false;
		}
	}

	mm_free(opts_present);
	return true;
}

bool_t check_http_network_basic_sthread(stringer_t *errmsg, uint32_t port, bool_t secure) {

	size_t content_length;
	client_t *client = NULL;

	// Test the connection.
	if (!(client = client_connect("localhost", port)) || (secure && (client_secure(client) == -1)) || client_status(client) != 1) {
		st_sprint(errmsg, "Failed to connect with the HTTP server.");
		client_close(client);
		return false;
	}
	// Test submitting a GET request.
	else if (client_write(client, PLACER("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n", 35)) != 35 ||
		client_status(client) != 1 || (content_length = check_http_content_length_get(client)) < 0) {
		if (st_empty(errmsg)) st_sprint(errmsg, "Failed to return a valid GET response.");
		client_close(client);
		return false;
	}
	// Test the response.
	else if (check_http_content_length_test(client, content_length, errmsg)) {
		if (st_empty(errmsg)) st_sprint(errmsg, "The content length and actual body length of the GET response did not match.");
		client_close(client);
		return false;
	}

	client_close(client);
	return true;
}

bool_t check_http_network_options_sthread(stringer_t *errmsg, uint32_t port, bool_t secure) {

	client_t *client = NULL;
	uint32_t options_count = 7;
	chr_t *options[] = {
		"Connection: close",
		"Content-Length: 0",
		"Content-Type: text/plain",
		"Allow: GET, POST, OPTIONS",
		"Access-Control-Max-Age: 86400",
		"Access-Control-Allow-Origin: *",
		"Access-Control-Allow-Credentials: true"
	};

	// Test the connection.
	if (!(client = client_connect("localhost", port)) || (secure && (client_secure(client) == -1)) || client_status(client) != 1) {
		st_sprint(errmsg, "Failed to connect with the HTTP server.");
		client_close(client);
		return false;
	}
	// Test OPTIONS.
	else if (client_write(client, PLACER("OPTIONS /portal/camel HTTP/1.1\r\n\r\n", 34)) != 34 ||
		client_status(client) != 1 || !check_http_options(client, options, options_count, errmsg)) {
		client_close(client);
		return false;
	}

	client_close(client);

	return true;
}

bool_t check_http_mime_types_sthread(stringer_t *errmsg, uint32_t port, bool_t secure) {

	size_t delim_index = 0;
	placer_t header_pl = pl_null();
	media_type_t *media_type = NULL;
	stringer_t *content_type = MANAGEDBUF(128), *group = NULL, *sub = NULL;
	/// LOW: Why do the mime types for .tcl and .pl use a '-' instead of a '/'?
	chr_t *types[] = {
		"application/octet-stream",
		"audio/aiff", "audio/aiff",
		"audio/basic",
		"video/avi",
		"image/bmp",
		"application/x-bzip",
		"application/x-bzip2",
		"text/x-c",
		"application/java",
		"text/plain",
		"text/x-c",
		"application/x-x509-ca-cert",
		"text/css",
		"text/csv",
		"application/msword",
		"application/msword",
		"video/x-flv",
		"image/gif",
		"application/gzip",
		"text/x-h",
		"text/html",
		"text/html",
		"image/x-icon",
		"application/inf",
		"text/x-java-source",
		"image/jpeg",
		"image/jpeg",
		"image/jpeg",
		"image/jpeg",
		"application/javascript",
		"application/json",
		"text/plain",
		"application/x-lzh",
		"application/x-lzh",
		"video/mpeg",
		"video/mpeg",
		"video/mpeg",
		"audio/midi",
		"audio/midi",
		"video/quicktime",
		"audio/mpeg3",
		"video/mpeg",
		"video/mpeg",
		"application/ogg",
		"application/pdf",
		"text/x-perl",
		"image/png",
		"application/mspowerpoint",
		"application/powerpoint",
		"application/postscript",
		"text/x-script.python",
		"video/quicktime",
		"audio/x-realaudio",
		"application/rss+xml",
		"text/rtf",
		"text/x-asm",
		"text/x-sgml",
		"text/x-sgml",
		"application/x-sh",
		"text/html",
		"application/x-shockwave-flash",
		"application/x-tar",
		"text/plain",
		"application/x-compressed",
		"image/tiff",
		"image/tiff",
		"text/plain",
		"text/x-uuencode",
		"text/x-uuencode",
		"audio/wav",
		"video/x-ms-wmv",
		"application/wordperfect",
		"application/wordperfect",
		"image/xbm",
		"application/excel",
		"application/excel",
		"text/xml",
		"application/x-compressed",
		"application/zip"
	}, *extensions[] = {
		"",
		".aif",
		".aiff",
		".au",
		".avi",
		".bmp",
		".bz",
		".bz2",
		".c",
		".class",
		".conf",
		".cpp",
		".crt",
		".css",
		".csv",
		".doc",
		".dot",
		".flv",
		".gif",
		".gz",
		".h",
		".htm",
		".html",
		".ico",
		".inf",
		".java",
		".jfif",
		".jpe",
		".jpeg",
		".jpg",
		".js",
		".json",
		".lst",
		".lzh",
		".lzs",
		".m1v",
		".m2v",
		".m4v",
		".mid",
		".midi",
		".mov",
		".mp3",
		".mpeg",
		".mpg",
		".ogg",
		".pdf",
		".pl",
		".png",
		".pps",
		".ppt",
		".ps",
		".py",
		".qt",
		".ra",
		".rss",
		".rtf",
		".s",
		".sgm",
		".smgl",
		".sh",
		".shtml",
		".swf",
		".tar",
		".text",
		".tgz",
		".tif",
		".tiff",
		".txt",
		".uu",
		".uue",
		".wav",
		".wmv",
		".wp5",
		".wp6",
		".xbm",
		".xl",
		".xls",
		".xml",
		".z",
		".zip"
	};

	for (size_t i = 0; i < sizeof(types)/sizeof(chr_t *); i++) {

		st_wipe(content_type);
		st_sprint(content_type, "Content-Type: %s", types[i]);
		header_pl = pl_init(st_data_get(content_type), st_length_get(content_type));

		if (!st_search_chr(NULLER(types[i]), '/', &delim_index)) {

			st_sprint(errmsg, "Failed to split mime type. { type = %s }", types[i]);
			return false;
		}

		if (!(group = mail_mime_type_group(header_pl)) || !(sub = mail_mime_type_sub(header_pl)) ||
			st_cmp_cs_starts(NULLER(types[i]), group) != 0 || st_cmp_cs_ends(NULLER(types[i]), sub) != 0) {

			st_sprint(errmsg, "Failed to return correct group/sub values from content type header. " \
				"{ header = %.*s , group = %.*s , sub = %.*s }", st_length_int(content_type), st_char_get(content_type),
				st_length_int(group), st_char_get(group), st_length_int(sub), st_char_get(sub));
			st_cleanup(group, sub);
			return false;
		}

		if (!(media_type = mail_mime_get_media_type(extensions[i])) || st_cmp_cs_eq(NULLER(media_type->name), NULLER(types[i])) != 0) {

			st_sprint(errmsg, "Failed to return the correct media type object for an extension. { extension = %s , type = %s }",
				extensions[i], types[i]);
			st_cleanup(group, sub);
			return false;
		}

		st_cleanup(group, sub);
		group = sub = NULL;
	}

	return true;
}
