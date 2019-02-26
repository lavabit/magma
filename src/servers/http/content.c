
/**
 * @file /magma/servers/http/content.c
 *
 * @brief	Functions for handling the management of web server content.
 */

#include "magma.h"

struct {
	inx_t *fonts, *pages, *templates;
} content = {
	.fonts = NULL, .pages = NULL, .templates = NULL
};

/// LOW: These functions should all be renamed to http_content_XXXX. The file and directory functions should be updated
/// to use the x64/reentrant alternatives. Specifically open64/fstat64/readdir64_r. An update function that can be triggered with
/// a SIGHUP would also be nice.

/// LOW: We should use basename() and dirname() to cleanup path strings.

/**
 * @brief	Free a stored piece of http content and all its underlying data.
 * @param	page	a pointer to the loaded content page to be freed.
 * @return	This function returns no value.
 */
void http_free_content(http_content_t *page) {

	if (page) {
		st_cleanup(page->location);
		st_cleanup(page->resource);
		st_cleanup(page->type);
		mm_free(page);
	}

	return;
}

/**
 * @brief	Free an http page object and all its underlying data.
 * @param	page	a pointer to the http page object to be freed.
 */
void http_page_free(http_page_t *page) {

	if (page) {

		if (page->xpath_ctx) {
			xml_free_xpath_ctx(page->xpath_ctx);
		}
		if (page->doc_obj) {
			xml_free_doc(page->doc_obj);
		}
		if (page->doc_ctx) {
			xml_free_parser_ctx(page->doc_ctx);
		}

		mm_free(page);
	}
	return;
}

/**
 * @brief	Get a cached copy of a static web page.
 * @param	location	a pointer to a managed string containing the location of the requested resource.
 * @return	NULL on failure, or a pointer to an http content object with the contents of the requested resource on success.
 */
http_content_t * http_get_static(stringer_t *location) {

	multi_t key = { .type = M_TYPE_STRINGER, .val.st = location };

	if (!content.pages) {
		return NULL;
	}

	return inx_find(content.pages, key);
}

/**
 * @brief	Return a template by location.
 * @param	location	a string specifying the location of the template.
 * @return	NULL on failure, or a pointer to an http_content_t object containing the template.
 */
http_content_t * http_get_template(chr_t *location) {

	multi_t key = { .type = M_TYPE_STRINGER, .val.st = NULLER(location) };

	if (!content.templates) {
		return NULL;
	}

	return inx_find(content.templates, key);
}

/**
 * @brief	Get a template page and prepare its xml document root for use.
 * @note	Each page is affixed with an xpath with a namespace after passing through the xml parser.
 * @param	location	a pointer to a null-terminated string with the pathname of the template to be returned.
 * @return	NULL on failure, or a pointer to the http page object of the requested template.
 */
http_page_t * http_page_get(chr_t *location) {

	http_page_t *page;

	if (!(page = mm_alloc(sizeof(http_page_t)))) {
		return NULL;
	}
	else if ((page->content = http_get_template(location)) == NULL) {
		log_pedantic("Unable to find the requested resource. {location = %s}", location);
		http_page_free(page);
		return NULL;
	}
	else if ((page->doc_ctx = xml_create_parser_ctx()) == NULL) {
		log_pedantic("Could not create the parser context.");
		http_page_free(page);
		return NULL;
	}
	// Build the document object.
	else if ((page->doc_obj = xml_create_doc(page->doc_ctx, st_char_get(page->content->resource), st_length_get(page->content->resource), NULL, NULL,
		XML_PARSE_RECOVER + XML_PARSE_NOERROR + XML_PARSE_NOWARNING)) == NULL) {
		log_pedantic("Could not load the XML document. {location = %s}", location);
		http_page_free(page);
		return NULL;
	}
	else if ((page->xpath_ctx = xml_create_xpath_ctx(page->doc_obj)) == NULL) {
		log_pedantic("Could not create the XPATH context.");
		http_page_free(page);
		return NULL;
	}
	// We need to set the default namespace or our Xpath queries won't work.
	else if (xml_xpath_set_namespace(page->xpath_ctx, (uchr_t *)"xhtml", (uchr_t *)"http://www.w3.org/1999/xhtml") != 0) {
		log_pedantic("Could not create the XPATH context.");
		http_page_free(page);
		return NULL;
	}

	return page;
}

/**
 * @brief	Load file content into the http server repository.
 * @note	Each file that is loaded will be cached for retrieval by http clients, with its mime type determined automatically.
 * 			Any files passed as a template will have the ".template" extension trimmed from the end of its resource path, and
 * 			any file named 'index.html' will be read as the default directory index path.
 * 			Each file will also be added to its respective parent page or template inx holder.
 * @param	template	a value specifying whether or not the specified file is a template.
 * @param	filename	a null-terminated string specifying the pathname of the file to be loaded.
 * @return	0 on failure or 1 on success.
 */
bool_t http_load_file(int_t template, chr_t *filename) {

	int_t fd;
	stringer_t *data;
	struct stat file_info;
	http_content_t *resource, *index;
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = NULL };

	// First skip over stale vi leftovers.
	if (!st_cmp_cs_ends(NULLER(filename), PLACER(".swp", 4))) {
		log_pedantic("HTTP resource loading phase skipped over .swp file { filename = %s }", filename);
		return true;
	}

	// Open and read the file.
	else if ((fd = open(filename, O_RDONLY)) == -1) {
		log_pedantic("Unable to open a file. { file = \"%s\" / error = %s }", filename, errno_string(errno, MEMORYBUF(1024), 1024));
		return false;
	}
	else if (fstat(fd, &file_info) == -1) {
		log_pedantic("Unable to fstat a file. { file = \"%s\" / error = %s }", filename, errno_string(errno, MEMORYBUF(1024), 1024));
		close(fd);
		return false;
	}
	else if (!st_cmp_cs_eq(NULLER(basename(filename)), PLACER(".empty", 6)) && !file_info.st_size) {
		log_pedantic("HTTP resource loading phase skipped over .empty file { filename = %s }", filename);
		close(fd);
		return true;
	}
	else if ((data = st_alloc(file_info.st_size + 1)) == NULL) {
		log_pedantic("Unable to allocate %li bytes for a file. { file = %s }", file_info.st_size + 1, filename);
		close(fd);
		return false;
	}
	else if (read(fd, st_char_get(data), file_info.st_size) != file_info.st_size) {
		log_pedantic("Unable to read a file. { file = \"%s\" / error = %s }", filename, errno_string(errno, MEMORYBUF(1024), 1024));
		close(fd);
		return false;
	}

	st_length_set(data, file_info.st_size);
	close(fd);

	// Build the resource structure.
	if ((resource = mm_alloc(sizeof(http_content_t))) == NULL) {
		log_pedantic("Unable to allocate memory for a resource structure. { file = %s }", filename);
		st_free(data);
		return false;
	}
	else {
		resource->resource = data;
	}

	// Build the location.
	if (template == 1) {
		resource->location = st_import(filename + ns_length_get(magma.http.templates), ns_length_get(filename + ns_length_get(magma.http.templates)));
	}
	else {
		resource->location = st_import(filename + ns_length_get(magma.http.pages) - 1, ns_length_get(filename + ns_length_get(magma.http.pages) - 1));
	}

	if (!resource->location) {
		log_pedantic("Unable to build the location string. { file = %s }", filename);
		http_free_content(resource);
		return false;
	}

	// TODO: We have a table that stores these mime types now!
	// Set the mime type.
	else if (!st_cmp_ci_ends(NULLER(filename), PLACER(".html", 5)) || !st_cmp_ci_ends(NULLER(filename), PLACER(".template", 9))) {
		resource->type = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, NULLER("text/html"));
	}
	else if (!st_cmp_ci_ends(NULLER(filename), PLACER(".json", 5))) {
		resource->type = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, NULLER("application/json"));
	}
	else if (!st_cmp_ci_ends(NULLER(filename), PLACER(".xml", 4))) {
		resource->type = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, NULLER("text/xml"));
	}
	else if (!st_cmp_ci_ends(NULLER(filename), PLACER(".css", 4))) {
		resource->type = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, NULLER("text/css"));
	}
	else if (!st_cmp_ci_ends(NULLER(filename), PLACER(".js", 3))) {
		resource->type = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, NULLER("application/x-javascript"));
	}
	else if (!st_cmp_ci_ends(NULLER(filename), PLACER(".gif", 4))) {
		resource->type = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, NULLER("image/gif"));
	}
	else if (!st_cmp_ci_ends(NULLER(filename), PLACER(".png", 4))) {
		resource->type = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, NULLER("image/png"));
	}
	else if (!st_cmp_ci_ends(NULLER(filename), PLACER(".jpg", 4)) || !st_cmp_ci_ends(NULLER(filename), PLACER(".jpeg", 5))) {
		resource->type = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, NULLER("image/jpeg"));
	}
	else if (!st_cmp_ci_ends(NULLER(filename), PLACER(".txt", 4))) {
		resource->type = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, NULLER("text/plain"));
	}
	else if (!st_cmp_ci_ends(NULLER(filename), PLACER(".ico", 4))) {
		resource->type = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, NULLER("image/x-icon"));
	}
	// If the file extension isn't recognized use a generic MIME type.
	else {
		log_pedantic("Unknown file type, using a generic octet-stream MIME type. { file = %s }", filename);
		resource->type = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, NULLER("application/octet-stream"));
	}

	if (!resource->type) {
		log_pedantic("Unable to copy the MIME type.");
		http_free_content(resource);
		return false;
	}

	// Trim the extension off the static HTML files and web application templates.
	if (template == 1 && !st_cmp_ci_ends(NULLER(filename), PLACER(".template", 9))) {
		st_length_set(resource->location, st_length_get(resource->location) - 9);
	}

	// Catch index pages.
	if (template == 0 && !st_cmp_ci_ends(NULLER(filename), PLACER("/index.html", 11))) {

		// Duplicate the content structure.
		if ((index = mm_alloc(sizeof(http_content_t))) == NULL || (index->resource = st_dupe(resource->resource)) == NULL ||
			(index->type = st_dupe(resource->type)) == NULL	|| (index->location = st_dupe(resource->location)) == NULL) {
			log_pedantic("Unable to copy the index page.");
			http_free_content(resource);
			return false;
		}

		// Trim the index.html from the location.
		st_length_set(index->location, st_length_get(index->location) - 10);

		// Insert the page into the content cache.
		if (!(key.val.st = index->location) || inx_insert(content.pages, key, index) != 1) {
			log_pedantic("Unable to add the content structure to the content cache. { location = %.*s }", st_length_int(index->location),
				st_char_get(index->location));
			http_free_content(resource);
			http_free_content(index);
			return false;
		}

	}

	// Add the structure to the correct content index.
	if (!(key.val.st = resource->location) || inx_insert(template ? content.templates : content.pages, key, resource) != 1) {
		log_pedantic("Unable to add the content structure to the content cache. { location = %.*s }", st_length_int(resource->location),
			st_char_get(resource->location));
		http_free_content(resource);
		return false;
	}

	// Output all of the files that are loaded.
	if (magma.log.content) {
		log_info("%.*s", st_length_int(resource->location), st_char_get(resource->location));
	}

	return true;
}

/**
 * @brief	Load the content from a directory into the http server repository, recursively.
 * @see		http_load_file()
 * @param	template	a value specifying whether the specified directory contains templates or regular web content.
 * @param	directory	a null-terminated string specifying the pathname of the directory to be scanned recursively.
 * @return	0 on failure or 1 on success.
 */
bool_t http_content_load_directory(int_t template, chr_t *directory) {

	DIR *dir;
	stringer_t *node;
	struct dirent *entry;

	if (!(dir = opendir(directory))) {
		log_info("Unable to access the web content directory. { directory = \"%s\" / error = %s }", directory, errno_string(errno, MEMORYBUF(1024), 1024));
		return false;
	}

	// Loop through.
	while ((entry = readdir(dir))) {

		if (st_cmp_cs_eq(NULLER(entry->d_name), PLACER(".", 1)) && st_cmp_cs_eq(NULLER(entry->d_name), PLACER("..", 2))) {

			// Build the path.
			if (!(node = st_merge("nnn", directory, !st_cmp_cs_ends(NULLER(directory), PLACER("/", 1)) ? "" : "/", entry->d_name))) {
				closedir(dir);
				return false;
			}

			// Load the sub directory.
			else if (entry->d_type == DT_DIR && !http_content_load_directory(template, st_char_get(node))) {
				st_free(node);
				closedir(dir);
				return false;
			}
			// Regular file.
			else if (entry->d_type == DT_REG && !http_load_file(template, st_char_get(node))) {
				st_free(node);
				closedir(dir);
				return false;
			}

			st_free(node);
		}
	}

	closedir(dir);

	return true;
}

/**
 * @brief	Load all fonts into the http server repository.
 * @note	This function will load all fonts of extension ".ttf" from the directory configured in magma.http.fonts.
 * @return	0 on failure or 1 on success.
 */
bool_t http_content_load_fonts(void) {

	DIR *dir;
	stringer_t *font = NULL;
	struct dirent *entry;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 1 };

	if (!(dir = opendir(magma.http.fonts))) {
		log_pedantic("Unable to open the font directory. { directory = \"%s\" / error = %s }", magma.http.fonts, errno_string(errno, MEMORYBUF(1024), 1024));
		return false;
	}

	// Add the fonts.
	while ((entry = readdir(dir))) {

		// Generate the font path string.
		if (!st_cmp_ci_ends(NULLER(entry->d_name), PLACER(".ttf", 4)) &&
			(font = st_merge("nnn", magma.http.fonts, !st_cmp_cs_ends(NULLER(magma.http.fonts), PLACER("/", 1)) ? "" : "/", entry->d_name))) {

			// Insert it.
			if (!inx_insert(content.fonts, key, font)) {
				st_free(font);
			}
			else if (magma.log.content) {
				log_info("%s%s%s", magma.http.fonts, !st_cmp_cs_ends(NULLER(magma.http.fonts), PLACER("/", 1)) ? "" : "/", entry->d_name);
				key.val.u64++;
			}
			else {
				key.val.u64++;
			}

		}
	}

	closedir(dir);

	return true;
}

/**
 * @brief	Prime the the web app templates, static content, and fonts directories for future use.
 * @note	This function makes sure that the web templates, static content, and fonts directory have been properly set, and loads
 * 			and caches their content for future use.
 * @return	true on success or false on failure.
 */
bool_t http_content_start(void) {

	// Missing a resource path.
	if (!magma.http.templates || !magma.http.pages || !magma.http.fonts) {
		log_pedantic("One of the web resource paths is missing.");
		return false;
	}

	// Make sure the root paths end with a forward slash.
	if (st_cmp_cs_ends(NULLER(magma.http.pages), PLACER("/", 1)) ||
		st_cmp_cs_ends(NULLER(magma.http.templates), PLACER("/", 1)) ||
		st_cmp_cs_ends(NULLER(magma.http.fonts), PLACER("/", 1))) {
		log_pedantic("One of the web resource paths does not end with a forward slash.");
		return false;
	}
	else if (!(content.pages = inx_alloc(M_INX_LINKED, &http_free_content)) || !http_content_load_directory(0, magma.http.pages) ||
		!(content.templates = inx_alloc(M_INX_LINKED, &http_free_content)) || !http_content_load_directory(1, magma.http.templates) ||
		!(content.fonts = inx_alloc(M_INX_LINKED, &st_free)) || !http_content_load_fonts()) {
		return false;
	}

	return true;
}

/**
 * @brief	Purge the http contents repository of stored fonts, templates, and static web pages.
 * @return	This function returns no value.
 */
void http_content_stop(void) {

	inx_cleanup(content.fonts);
	inx_cleanup(content.templates);
	inx_cleanup(content.pages);

	return;

}

/**
 * @brief	Refresh the stored contents of the web app templates, static content, and fonts directories.
 * @return	true if all content directories were successfully refreshed, or false on failure.
 */
bool_t http_content_refresh(void) {

	bool_t result = true;

	if (content.fonts) {
		inx_free(content.fonts);
		if (!(content.fonts = inx_alloc(M_INX_LINKED, &st_free)) || !http_content_load_fonts()) result = false;
	}

	if (content.templates) {
		inx_free(content.templates);
		if (!(content.templates = inx_alloc(M_INX_LINKED, &http_free_content)) || !http_content_load_directory(1, magma.http.templates)) result = false;
	}

	if (content.pages) {
		inx_free(content.pages);
		if (!(content.pages = inx_alloc(M_INX_LINKED, &http_free_content)) || !http_content_load_directory(0, magma.http.pages)) result = false;
	}

	return result;
}
