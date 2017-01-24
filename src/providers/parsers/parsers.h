
/**
 * @file /magma/providers/parsers/parsers.h
 *
 * @brief The entry point for modules involved with accessing functionality provided by alien code.
 */

#ifndef MAGMA_PROVIDERS_PARSERS_H
#define MAGMA_PROVIDERS_PARSERS_H

/// json.c
bool_t   lib_load_jansson(void);
chr_t *  lib_version_jansson(void);

/// xml.c
bool_t lib_load_xml(void);
bool_t xml_start(void);
chr_t * xml_get_xpath_ns(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query);
const chr_t * lib_version_xml(void);
int16_t xml_get_xpath_int16(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query);
int32_t xml_get_xpath_int32(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query);
int64_t xml_get_xpath_int64(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query);
int8_t xml_get_xpath_int8(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query);
int_t xml_xpath_set_namespace(xmlXPathContextPtr ctxt, xmlChar *prefix, xmlChar *ns_uri);
size_t xml_get_xpath_node_count(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query);
stringer_t * xml_dump_doc(xmlDocPtr doc);
stringer_t * xml_get_xpath_st(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query);
stringer_t * xml_node_get_content_st(xmlNodePtr node);
uint16_t xml_get_xpath_uint16(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query);
uint32_t xml_get_xpath_uint32(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query);
uint64_t xml_get_xpath_uint64(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query);
uint8_t xml_get_xpath_uint8(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query);
bool_t xml_set_xpath_uint64(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query, uint64_t val);
bool_t xml_set_xpath_ns(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query, uchr_t *val);
bool_t xml_set_xpath_property(xmlXPathContextPtr xpath_ctx, xmlChar *xpath_query, uchr_t *name, uchr_t *val);
void xml_error(void *ctx, const chr_t *format, ...);
void xml_free_doc(xmlDocPtr doc);
void xml_free_parser_ctx(xmlParserCtxtPtr ctx);
void xml_free_xpath_ctx(xmlXPathContextPtr ctx);
void xml_free_xpath_obj(xmlXPathObjectPtr obj);
void xml_node_free(xmlNodePtr node);
void xml_node_set_content(xmlNodePtr node, uchr_t *content);
void xml_stop(void);
xmlAttrPtr xml_node_set_property(xmlNodePtr node, uchr_t *name, uchr_t *value);
xmlChar * xml_encode(xmlDocPtr doc, stringer_t *string);
xmlDocPtr xml_create_doc(xmlParserCtxtPtr ctx, const chr_t *buffer, int_t size, const chr_t *url, const chr_t *encoding, int_t options);
xmlNodePtr xml_node_add_sibling(xmlNodePtr current, xmlNodePtr element);
xmlNodePtr xml_node_new(uchr_t *name);
xmlParserCtxtPtr xml_create_parser_ctx(void);
xmlXPathContextPtr xml_create_xpath_ctx(xmlDocPtr doc);
xmlXPathObjectPtr xml_xpath_eval(const uchr_t *xpath, xmlXPathContextPtr ctx);

/// utf.c
bool_t   lib_load_utf8proc(void);
chr_t *  lib_version_utf8proc(void);
bool_t utf8_valid_st(stringer_t *s);
size_t utf8_length_st(stringer_t *s);
const chr_t * utf8_error_string(ssize_t error_code);

#endif
