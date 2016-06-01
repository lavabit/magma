
/**
 * @file /magma/providers/parsers/xml.c
 *
 * @brief	The interface to the xml parser.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

chr_t xml_version_string[9];
extern pthread_mutex_t log_mutex;

/**
 * @brief	Return the version string of libxml.
 * @return	a pointer to a character string containing the libxml version information.
 */
const chr_t * lib_version_xml(void) {

	return xml_version_string;
}

/**
 * @brief	Initialize libxml and bind dynamically to the exported functions that are required.
 * @result	true on success or false on failure.
 */
bool_t lib_load_xml(void) {

	uint64_t xml_ver_num;
	symbol_t xml[] = {
		M_BIND(xmlAddSibling), M_BIND(xmlBufferContent), M_BIND(xmlBufferCreate), M_BIND(xmlBufferFree), M_BIND(xmlBufferLength),
		M_BIND(xmlCleanupGlobals), M_BIND(xmlCleanupParser), M_BIND(xmlCtxtReadMemory),	M_BIND(xmlDocDumpFormatMemory),
		M_BIND(xmlEncodeEntitiesReentrant),	M_BIND(xmlFreeDoc), M_BIND(xmlFreeNode), M_BIND(xmlFreeParserCtxt),	M_BIND(xmlInitParser),
		M_BIND(xmlMemoryDump), M_BIND(xmlNewNode), M_BIND(xmlNewParserCtxt), M_BIND(xmlNodeBufGetContent), M_BIND(xmlNodeSetContent),
		M_BIND(xmlParserVersion), M_BIND(xmlSetProp), M_BIND(xmlXPathEvalExpression), M_BIND(xmlXPathFreeContext), M_BIND(xmlXPathFreeObject),
		M_BIND(xmlXPathNewContext),	M_BIND(xmlXPathRegisterNs)

	};

	if (!lib_symbols(sizeof(xml) / sizeof(symbol_t), xml)) {
		return false;
	}

	if (uint64_conv_ns(*xmlParserVersion_d, &xml_ver_num)) {
		snprintf(xml_version_string, sizeof(xml_version_string), "%hhu.%hhu.%hhu", (char)((xml_ver_num / 100 / 100) % 100), (char)((xml_ver_num / 100) % 100), (char)(xml_ver_num % 100));
	} else {
		snprintf(xml_version_string, sizeof(xml_version_string), "unknown");
	}

	return true;
}

/**
 * @brief	Free an xml node.
 * @param	node	a pointer to the xml node to be freed.
 * @return	This function returns no value.
 */
void xml_node_free(xmlNodePtr node) {

	xmlFreeNode_d(node);
	return;
}

/**
 * @brief	Encode an xml string, performing proper replacement of defined entities and non-ASCII values.
 * @param	doc		a pointer to the xml document containing the DTD to be used for the encoding process.
 * @param	string	a managed string containing the xml data to be encoded.
 * @param	NULL on failure, or a newly allocated null-terminated string containing the encoded xml data on success.
 */
xmlChar * xml_encode(xmlDocPtr doc, stringer_t *string) {

	return xmlEncodeEntitiesReentrant_d(doc, st_data_get(string));
}

/**
 * @brief	Set the namespace for an xpath context.
 * @see		xmlXPathRegisterNs()
 * @param	ctxt	a pointer to the specified xpath context.
 * @param	prefix	a pointer to the prefix of the namespace as a string.
 * @param	ns_uri	a pointer to the uri of the namespace as a string.
 * @return	-1 on failure or 0 on success.
 */
int_t xml_xpath_set_namespace(xmlXPathContextPtr ctxt, xmlChar *prefix, xmlChar *ns_uri) {

	return xmlXPathRegisterNs_d(ctxt, (const xmlChar *)prefix, (const xmlChar *)ns_uri);
}

/**
 * @brief	Add an xml node to the list of siblings of another xml node.
 * @note	If the sibling node was already inserted into a document it will first be unlinked.
 * @param	current		a pointer to the xml node to which the sibling node will be added.
 * @param	element		a pointer to the sibling node to be added to the target node.
 * @return	NULL on failure, or a pointer to the sibling node on success.
 */
xmlNodePtr xml_node_add_sibling(xmlNodePtr current, xmlNodePtr element) {

	return xmlAddSibling_d(current, element);
}

/**
 * @brief	Create an xml node.
 * @param	name	the name of the new xml node.
 * @return	NULL on failure, or a pointer to the newly allocated and initialized xml node on success.
 */
xmlNodePtr xml_node_new(uchr_t *name) {

	return xmlNewNode_d(NULL, name);
}

/**
 * @brief	Get the content of an xml node as a managed string.
 * @param	node	a pointer to the xml node to be queried.
 * @return	NULL on failure, or a pointer to a managed string
 */
stringer_t * xml_node_get_content_st(xmlNodePtr node) {

	xmlBuffer *buf;
	stringer_t *result = NULL;

	if (!node || !(node->type == XML_CDATA_SECTION_NODE || node->type == XML_TEXT_NODE)) {
		log_pedantic("Invalid content node.");
		return NULL;
	}

	if (!(buf = xmlBufferCreate_d())) {
		log_pedantic("Unable to create a buffer for the XML content.");
		return NULL;
	}

	if (xmlNodeBufGetContent_d(buf, node)) {
		log_pedantic("Content extraction failed.");
		xmlBufferFree_d(buf);
		return NULL;
	}

	if (!(result = st_import((xmlChar *)xmlBufferContent_d(buf), xmlBufferLength_d(buf)))) {
		log_pedantic("Unable to import the XML content into a managed stringer.");
	}

	xmlBufferFree_d(buf);

	return result;
}

/**
 * @brief	Set the content of an xml node.
 * @param	node	a pointer to the xml node to be set.
 * @param	content	a null-terminated string containing the new content of the xml node.
 * @return	This function returns no value.
 */
void xml_node_set_content(xmlNodePtr node, uchr_t *content) {

	xmlNodeSetContent_d(node, content);
	return;
}

/**
 * @brief	Set an attribute of an xml node.
 * @param	node	a pointer to the xml node to be set.
 * @param	name	a null-terminated string containing the name of the node attribute to be set.
 * @param	value	a null-terminated string containing the new value of the specified xml node's attribute.
 * @return	NULL on failure, or a pointer to node's attribute on success.
 */
xmlAttrPtr xml_node_set_property(xmlNodePtr node, uchr_t *name, uchr_t *value) {

	return xmlSetProp_d(node, name, value);
}

/**
 * @brief	Get the value of a node element or attribute selected by an xpath expression as an unsigned 64-bit integer.
 * @param	xpath_ctx		a pointer to the specified xpath context.
 * @param	xpath_query		a null-terminated string containing the xpath expression to select the desired node.
 * @return	0 on failure, or the value of the selected node as an unsigned 64-bit integer.
 */
uint64_t xml_get_xpath_uint64(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	uint64_t result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (!xpath_ctx || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return 0;
	}

	// Evaluates the expression.
	if (!(xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx))) {
		log_pedantic("Could not evaluate the XPATH query.");
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || !xpath_obj->nodesetval) {
		log_pedantic("The XPATH query did not evaluate into a string.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		log_pedantic("The XPATH query did not return a single node.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children && xpath_obj->nodesetval->nodeTab[0]->children->content) {

		if (!uint64_conv_st(PLACER(xpath_obj->nodesetval->nodeTab[0]->children->content, ns_length_get((chr_t *)(xpath_obj->nodesetval->nodeTab[0]->children->content))), &result)) {
			log_pedantic("Could not convert the node value to an unsigned long.");
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}

	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

/**
 * @brief	Get the value of a node element or attribute selected by an xpath expression as an unsigned 32-bit integer.
 * @param	xpath_ctx		a pointer to the specified xpath context.
 * @param	xpath_query		a null-terminated string containing the xpath expression to select the desired node.
 * @return	0 on failure, or the value of the selected node as an unsigned 32-bit integer.
 */
uint32_t xml_get_xpath_uint32(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	uint32_t result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (!xpath_ctx || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return 0;
	}

	// Evaluates the expression.
	if (!(xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx))) {
		log_pedantic("Could not evaluate the XPATH query.");
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || !xpath_obj->nodesetval) {
		log_pedantic("The XPATH query did not evaluate into a string.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		log_pedantic("The XPATH query did not return a single node.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children && xpath_obj->nodesetval->nodeTab[0]->children->content) {

		if (!uint32_conv_st(PLACER(xpath_obj->nodesetval->nodeTab[0]->children->content, ns_length_get((chr_t *)xpath_obj->nodesetval->nodeTab[0]->children->content)), &result)) {
			log_pedantic("Could not convert the node value to an unsigned integer.");
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}
	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

/**
 * @brief	Get the value of a node element or attribute selected by an xpath expression as an unsigned 16-bit integer.
 * @param	xpath_ctx		a pointer to the specified xpath context.
 * @param	xpath_query		a null-terminated string containing the xpath expression to select the desired node.
 * @return	0 on failure, or the value of the selected node as an unsigned 16-bit integer.
 */
uint16_t xml_get_xpath_uint16(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	uint16_t result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (!xpath_ctx || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return 0;
	}

	// Evaluates the expression.
	if (!(xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx))) {
		log_pedantic("Could not evaluate the XPATH query.");
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || !xpath_obj->nodesetval) {
		log_pedantic("The XPATH query did not evaluate into a string.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		log_pedantic("The XPATH query did not return a single node.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children && xpath_obj->nodesetval->nodeTab[0]->children->content) {

		if (!uint16_conv_st(PLACER(xpath_obj->nodesetval->nodeTab[0]->children->content, ns_length_get((chr_t *)xpath_obj->nodesetval->nodeTab[0]->children->content)), &result)) {
			log_pedantic("Could not convert the node value to an unsigned short.");
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}

	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

/**
 * @brief	Get the value of a node element or attribute selected by an xpath expression as an unsigned 8-bit integer.
 * @param	xpath_ctx		a pointer to the specified xpath context.
 * @param	xpath_query		a null-terminated string containing the xpath expression to select the desired node.
 * @return	0 on failure, or the value of the selected node as an unsigned 8-bit integer.
 */
uint8_t xml_get_xpath_uint8(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	uint8_t result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (!xpath_ctx || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return 0;
	}

	// Evaluates the expression.
	if (!(xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx))) {
		log_pedantic("Could not evaluate the XPATH query.");
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || !xpath_obj->nodesetval) {
		log_pedantic("The XPATH query did not evaluate into a string.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		log_pedantic("The XPATH query did not return a single node.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children && xpath_obj->nodesetval->nodeTab[0]->children->content) {

		if (!uint8_conv_st(PLACER(xpath_obj->nodesetval->nodeTab[0]->children->content, ns_length_get((chr_t *)xpath_obj->nodesetval->nodeTab[0]->children->content)), &result)) {
			log_pedantic("Could not convert the node value to an unsigned short.");
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}

	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

/**
 * @brief	Get the value of a node element or attribute selected by an xpath expression as a signed 64-bit integer.
 * @param	xpath_ctx		a pointer to the specified xpath context.
 * @param	xpath_query		a null-terminated string containing the xpath expression to select the desired node.
 * @return	0 on failure, or the value of the selected node as a signed 64-bit integer.
 */
int64_t xml_get_xpath_int64(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	int64_t result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (!xpath_ctx || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return 0;
	}

	// Evaluates the expression.
	if (!(xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx))) {
		log_pedantic("Could not evaluate the XPATH query.");
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || !xpath_obj->nodesetval) {
		log_pedantic("The XPATH query did not evaluate into a string.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		log_pedantic("The XPATH query did not return a single node.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children && xpath_obj->nodesetval->nodeTab[0]->children->content) {

		if (!int64_conv_st(PLACER(xpath_obj->nodesetval->nodeTab[0]->children->content, ns_length_get((chr_t *)xpath_obj->nodesetval->nodeTab[0]->children->content)), &result)) {
			log_pedantic("Could not convert the node value to an long.");
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}

	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

/**
 * @brief	Get the value of a node element or attribute selected by an xpath expression as a signed 32-bit integer.
 * @param	xpath_ctx		a pointer to the specified xpath context.
 * @param	xpath_query		a null-terminated string containing the xpath expression to select the desired node.
 * @return	0 on failure, or the value of the selected node as a signed 32-bit integer.
 */
int32_t xml_get_xpath_int32(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	int32_t result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (!xpath_ctx || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return 0;
	}

	// Evaluates the expression.
	if (!(xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx))) {
		log_pedantic("Could not evaluate the XPATH query.");
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || !xpath_obj->nodesetval) {
		log_pedantic("The XPATH query did not evaluate into a string.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		log_pedantic("The XPATH query did not return a single node.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children && xpath_obj->nodesetval->nodeTab[0]->children->content) {

		if (!int32_conv_st(PLACER(xpath_obj->nodesetval->nodeTab[0]->children->content, ns_length_get((chr_t *)xpath_obj->nodesetval->nodeTab[0]->children->content)), &result)) {
			log_pedantic("Could not convert the node value to an integer.");
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}

	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

/**
 * @brief	Get the value of a node element or attribute selected by an xpath expression as a signed 16-bit integer.
 * @param	xpath_ctx		a pointer to the specified xpath context.
 * @param	xpath_query		a null-terminated string containing the xpath expression to select the desired node.
 * @return	0 on failure, or the value of the selected node as a signed 16-bit integer.
 */
int16_t xml_get_xpath_int16(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	int16_t result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (!xpath_ctx || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return 0;
	}

	// Evaluates the expression.
	if (!(xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx))) {
		log_pedantic("Could not evaluate the XPATH query.");
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || !xpath_obj->nodesetval) {
		log_pedantic("The XPATH query did not evaluate into a string.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		log_pedantic("The XPATH query did not return a single node.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children && xpath_obj->nodesetval->nodeTab[0]->children->content) {

		if (!int16_conv_st(PLACER(xpath_obj->nodesetval->nodeTab[0]->children->content, ns_length_get((chr_t *)xpath_obj->nodesetval->nodeTab[0]->children->content)), &result)) {
			log_pedantic("Could not convert the node value to a short.");
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}

	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

/**
 * @brief	Get the value of a node element or attribute selected by an xpath expression as a signed 8-bit integer.
 * @param	xpath_ctx		a pointer to the specified xpath context.
 * @param	xpath_query		a null-terminated string containing the xpath expression to select the desired node.
 * @return	0 on failure, or the value of the selected node as a signed 8-bit integer.
 */
int8_t xml_get_xpath_int8(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	int8_t result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (!xpath_ctx  || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return 0;
	}

	// Evaluates the expression.
	if (!(xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx))) {
		log_pedantic("Could not evaluate the XPATH query.");
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || !xpath_obj->nodesetval) {
		log_pedantic("The XPATH query did not evaluate into a string.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		log_pedantic("The XPATH query did not return a single node.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children && xpath_obj->nodesetval->nodeTab[0]->children->content) {

		if (!int8_conv_st(PLACER(xpath_obj->nodesetval->nodeTab[0]->children->content, ns_length_get((chr_t *)xpath_obj->nodesetval->nodeTab[0]->children->content)), &result)) {
			log_pedantic("Could not convert the node value to a short.");
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}

	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

/**
 * @brief	Get the value of a node element or attribute selected by an xpath expression as a managed string.
 * @param	xpath_ctx		a pointer to the specified xpath context.
 * @param	xpath_query		a null-terminated string containing the xpath expression to select the desired node.
 * @return	0 on failure, or the value of the selected node as a managed string.
 */
stringer_t * xml_get_xpath_st(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	stringer_t *output = NULL;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (!xpath_ctx || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return NULL;
	}

	// Evaluates the expression.
	if (!(xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx))) {
		log_pedantic("Could not evaluate the XPATH query.");
		return NULL;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || !xpath_obj->nodesetval) {
		log_pedantic("The XPATH query did not evaluate into a string.");
		xmlXPathFreeObject_d(xpath_obj);
		return NULL;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		log_pedantic("The XPATH query did not return a single node.");
		xmlXPathFreeObject_d(xpath_obj);
		return NULL;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children && xpath_obj->nodesetval->nodeTab[0]->children->content) {

		// Create a stringer.
		if (!(output = st_import(xpath_obj->nodesetval->nodeTab[0]->children->content, ns_length_get((chr_t *)(xpath_obj->nodesetval->nodeTab[0]->children->content))))) {
			log_pedantic("Unable to import the node into the stringer.");
		}

	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return output;
}

/**
 * @brief	Set the value of a node element selected by an xpath expression, as a 64-bit unsigned integer.
 * @param	xpath_ctx		a pointer to the specified xpath context.
 * @param	xpath_query		a null-terminated string containing the xpath expression to select the desired node.
 * @param	val				the new value of the selected node, as a 64-bit unsigned integer.
 * @return	true if the value was set successfully, or false on failure.
 */
bool_t xml_set_xpath_uint64(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query, uint64_t val) {

	xmlXPathObjectPtr xpath_obj = NULL;
	xmlNodePtr node;
	chr_t strbuf[32];
	bool_t result = false;

	if (!xpath_ctx || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return false;
	}

	if ((xpath_obj = xml_xpath_eval(xpath_query, xpath_ctx)) && xpath_obj->nodesetval &&
		xpath_obj->nodesetval->nodeNr && xpath_obj->nodesetval->nodeTab[0]) {
		node = (xmlNodePtr)xpath_obj->nodesetval->nodeTab[0];
		snprintf(strbuf,sizeof(strbuf),"%zu",val);
		xml_node_set_content(node, (uchr_t *)strbuf);
		result = true;
	}

	if (xpath_obj) {
		xml_free_xpath_obj(xpath_obj);
	}

	return result;
}

/**
 * @brief	Set the value of a node element selected by an xpath expression, as a null-terminated string.
 * @param	xpath_ctx		a pointer to the specified xpath context.
 * @param	xpath_query		a null-terminated string containing the xpath expression to select the desired node.
 * @param	val				the new value of the selected node, as a null-terminated string.
 * @return	true if the value was set successfully, or false on failure.
 */
bool_t xml_set_xpath_ns(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query, uchr_t *val) {

	xmlXPathObjectPtr xpath_obj = NULL;
	xmlNodePtr node;
	bool_t result = false;

	if (!xpath_ctx || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return false;
	}

	if ((xpath_obj = xml_xpath_eval(xpath_query, xpath_ctx)) && xpath_obj->nodesetval &&
		xpath_obj->nodesetval->nodeNr && xpath_obj->nodesetval->nodeTab[0]) {
		node = (xmlNodePtr)xpath_obj->nodesetval->nodeTab[0];
		xml_node_set_content(node, val);
		result = true;
	}

	if (xpath_obj) {
		xml_free_xpath_obj(xpath_obj);
	}

	return result;
}

/**
 * @brief	Set the value of a specified property of a node element selected by an xpath expression.
 * @param	xpath_ctx		a pointer to the specified xpath context.
 * @param	xpath_query		a null-terminated string containing the xpath expression to select the desired node.
 * @param	name			a null-terminated string containing the name of the node's property to be set.
 * @param	val				the new value of the selected node's property, as a null-terminated string.
 * @return	true if the selected node's value was set successfully, or false on failure.
 */
bool_t xml_set_xpath_property(xmlXPathContextPtr xpath_ctx, xmlChar *xpath_query, uchr_t *name, uchr_t *val) {

	xmlXPathObjectPtr xpath_obj = NULL;
	xmlNodePtr node;
	bool_t result = false;

	if (!xpath_ctx || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return false;
	}

	if ((xpath_obj = xml_xpath_eval(xpath_query, xpath_ctx)) && xpath_obj->nodesetval &&
		xpath_obj->nodesetval->nodeNr && xpath_obj->nodesetval->nodeTab[0]) {
		node = (xmlNodePtr)xpath_obj->nodesetval->nodeTab[0];
		xml_node_set_property(node, name, val);
		result = true;
	}

	if (xpath_obj) {
		xml_free_xpath_obj(xpath_obj);
	}

	return result;
}

/**
 * @brief	Get the content of an evaluated xpath expression as a null-terminated string.
 * @param	ctx				a pointer to the xpath context to be used for the evaluation.
 * @param	xpath_query		a null-terminated string containing the xpath expression to be evaluated.
 * @return	NULL on failure, or a null-terminated string containing the value of the xpath expression's specified node on success.
 */
chr_t * xml_get_xpath_ns(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	chr_t *output = NULL;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (!xpath_ctx || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return NULL;
	}

	// Evaluates the expression.
	if (!(xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx))) {
		log_pedantic("Could not evaluate the XPATH query.");
		return NULL;
	}

	// Lets make sure we got back a string.
	if (!xpath_obj->nodesetval || xpath_obj->type != XPATH_NODESET) {
		//log_pedantic("The XPATH query did not evaluate into a string.");
		xmlXPathFreeObject_d(xpath_obj);
		return NULL;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		log_pedantic("The XPATH query did not return a single node.");
		xmlXPathFreeObject_d(xpath_obj);
		return NULL;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children && xpath_obj->nodesetval->nodeTab[0]->children->content) {

		// Create a stringer.
		if (!(output = ns_dupe((chr_t *)(xpath_obj->nodesetval->nodeTab[0]->children->content)))) {
			log_pedantic("Unable to duplicate the node.");
		}

	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return output;
}

/**
 * @brief	This function is not currently being called by any other part of the code.
 */
size_t xml_get_xpath_node_count(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (!xpath_ctx || !xpath_query) {
		log_pedantic("A NULL value was passed in.");
		return 0;
	}

	// Evaluates the expression.
	if (!(xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx))) {
		log_pedantic("Could not evaluate the XPATH query.");
		return 0;
	}

	// Lets make sure we got back a number.
	if (xpath_obj->type != XPATH_NUMBER) {
		log_pedantic("The XPATH query did not evaluate into a string.");
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return (size_t) xpath_obj->floatval;

}

/**
 * @brief	A function handler for displaying xml parser error messages to the user.
 * @param	ctx		a placeholder; ignored.
 * @param	format	a null-terminated string containing a format string for the error message to be displayed.
 * @param	...		a variable argument list containing the parameters to the format string.
 * @return	This function returns no value.
 */
void xml_error(void *ctx, const chr_t *format, ...) {

	va_list args;

	// Simple test on ctx to avoid compiler warning about an unused variable.
	if (ctx == NULL) { }

	va_start(args, format);
	mutex_get_lock(&log_mutex);

	printf("(%s - %s - %i) = XML Library Error: ", __FILE__, __FUNCTION__, __LINE__);
	vprintf(format, args);
	fflush(stdout);

	mutex_unlock(&log_mutex);

	va_end(args);

	return;
}

/**
 * @brief	Free an xml xpath context.
 * @param	ctx		a pointer to the xml xpath context to be freed.
 * @return	This function returns no value.
 */
void xml_free_xpath_ctx(xmlXPathContextPtr ctx) {

	if (ctx) {
		xmlXPathFreeContext_d(ctx);
	}
	else {
		log_pedantic("Asked to free a NULL context.");
	}

	return;
}

/**
 * @brief	Free an xml xpath object.
 * @param	obj		a pointer to the xml xpath object to be freed.
 * @return	This function returns no value.
 */
void xml_free_xpath_obj(xmlXPathObjectPtr obj) {

	if (obj) {
		xmlXPathFreeObject_d(obj);
	}
	else {
		log_pedantic("Asked to free a NULL object.");
	}

	return;
}

/**
 * @brief	Create an xpath context for an xml document.
 * @param	doc		a pointer to the input xml document object.
 * @return	NULL on failure, or a pointer to the new xpath context on success.
 */
xmlXPathContextPtr xml_create_xpath_ctx(xmlDocPtr doc) {

	xmlXPathContextPtr result;

	if (!doc) {
		log_pedantic("Passed a NULL pointer.");
		return NULL;
	}

	result = xmlXPathNewContext_d(doc);

	if (!result) {
		log_pedantic("Could not create a NULL pointer.");
	}

	return result;
}

/**
 * @brief	Evaluate an xml xpath expression.
 * @param	a null-terminated string containing the xpath expression to be evaluated.
 * @param	a pointer to the xpath context in which to perform the evaluation.
 * @brief	NULL on failure, or a pointer to the xml path object of the evaluation on success.
 */
xmlXPathObjectPtr xml_xpath_eval(const uchr_t *xpath, xmlXPathContextPtr ctx) {

	xmlXPathObjectPtr result;

	if (!xpath || !ctx) {
		log_pedantic("Passed in a NULL pointer.");
		return NULL;
	}

	if (!(result = xmlXPathEvalExpression_d(xpath, ctx))) {
		log_pedantic("Received a NULL XPATH object back.");
	}

	return result;
}

/**
 * @brief	Create and initialize a new xml parser context.
 * @return	NULL on failure, or a newly initialized xml parser context on success.
 */
xmlParserCtxtPtr xml_create_parser_ctx(void) {

	xmlParserCtxtPtr result;

	if (!(result = xmlNewParserCtxt_d())) {
		log_pedantic("Unable to create an XML parser context.");
		return NULL;
	}

	// Setup our own error handlers. We can also use the xmlSetGenericErrorFunc function, but I don't like the output.
	result->sax->error = &xml_error;
	result->sax->warning = &xml_error;
	result->vctxt.error = &xml_error;
	result->vctxt.warning = &xml_error;

	return result;
}

/**
 * @brief	Destroy an xml parser context.
 * @param	ctx		a pointer to the xml parser context to be freed.
 * @return	This function returns no value.
 */
void xml_free_parser_ctx(xmlParserCtxtPtr ctx) {

	if (ctx) {
		xmlFreeParserCtxt_d(ctx);
	}
	else {
		log_pedantic("Asked to free a NULL context.");
	}

	return;
}

/**
 * @brief	Free an xml document.
 * @param	doc		a pointer to the xml document to be freed.
 * @return	This function returns no value.
 */
void xml_free_doc(xmlDocPtr doc) {

	if (doc) {
		xmlFreeDoc_d(doc);
	}
	else {
		log_pedantic("Asked to free a NULL document object.");
	}

	return;
}

/**
 * @brief	Get an xml document object as a string.
 * @param	doc		a pointer to the xml document object to be serialized.
 * @return	NULL on failure or a pointer to a managed string containing the specified xml document's serialized data on success.
 */
stringer_t * xml_dump_doc(xmlDocPtr doc) {

	int_t size;
	stringer_t *result;
	xmlChar *buffer = NULL;

	if (!doc) {
		return NULL;
	}

	xmlDocDumpFormatMemory_d(doc, &buffer, &size, 1);

	if (!buffer) {
		return NULL;
	}

	result = st_import(buffer, size);
	mm_free(buffer);

	return result;
}

/**
 * @brief	Create a new xml document object from specified user data.
 * @see		xmlCtxtReadMemory()
 * @param	ctx			a pointer to the xml context to be used for the parsing operation.
 * @param	buffer		a null-terminated string containing the xml data to be parsed.
 * @param	size		the length, in bytes, of the xml data buffer to be parsed.
 * @param	url			a null-terminated string containing the base url to be used for the document.
 * @param	encoding	a null-terminated string specifying the document encoding type, or NULL for default.
 * @param	options		a value containing a mask of xml parser options of type xmlParserOption.
 * @return	NULL on failure, or a pointer to the xml document tree of the parsed xml text on success.
 *
 */
xmlDocPtr xml_create_doc(xmlParserCtxtPtr ctx, const chr_t *buffer, int_t size, const chr_t *url, const chr_t *encoding, int_t options) {

	xmlDocPtr result;

	if (!(result = xmlCtxtReadMemory_d(ctx, buffer, size, url, encoding, options))) {
		log_pedantic("Unable to create the XML document object.");
	}

	return result;
}

/**
 * @brief	Cleanup the state and allocated memory of the xml parser in preparation to be shutdown.
 * @return	This function returns no value.
 */
void xml_stop(void) {

	xmlCleanupGlobals_d();
	xmlCleanupParser_d();
	xmlMemoryDump_d();

	return;
}

/**
 * @brief	Initialize the xml parser library.
 * @return	This function returns no value.
 */
bool_t xml_start(void) {

	xmlInitParser_d();
	return true;
}

