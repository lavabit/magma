
#include "framework.h"

extern pthread_mutex_t log_mutex;
stringer_t *xml_version = NULL;

// Functions.
extern void *lavalib;
void (*xmlInitParser_d)(void) = NULL;
void (*xmlCleanupParser_d)(void) = NULL;
void (*xmlFreeDoc_d)(xmlDocPtr doc) = NULL;
const char ** (*__xmlParserVersion_d)(void) = NULL;
xmlParserCtxtPtr (*xmlNewParserCtxt_d)(void) = NULL;
void (*xmlXPathFreeContext_d)(xmlXPathContextPtr ctx) = NULL;
void (*xmlXPathFreeObject_d)(xmlXPathObjectPtr obj) = NULL;
void (*xmlFreeParserCtxt_d)(xmlParserCtxtPtr ctx) = NULL;
xmlXPathContextPtr (*xmlXPathNewContext_d)(xmlDocPtr doc) = NULL;
xmlXPathObjectPtr (*xmlXPathEvalExpression_d)(const xmlChar *xpath, xmlXPathContextPtr ctx) = NULL;
xmlDocPtr (*xmlCtxtReadMemory_d)(xmlParserCtxtPtr ctx, const char *buffer, int size, const char *url, const char *encoding, int options) = NULL;

unsigned long long xml_get_xpath_ull(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	unsigned long long result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (xpath_ctx == NULL || xpath_query == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL value was passed in.");
		#endif
		return 0;
	}

	// Evalutes the expression.
	xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx);
	if (xpath_obj == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not evaluate the XPATH query.");
		#endif
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || xpath_obj->nodesetval == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not evaluate into a string.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not return a single node.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children != NULL && xpath_obj->nodesetval->nodeTab[0]->children->content != NULL) {

		// Create a unsigned long long.
		if (extract_ull_ns(xpath_obj->nodesetval->nodeTab[0]->children->content, size_ns(xpath_obj->nodesetval->nodeTab[0]->children->content), &result) != 1) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Could not convert the node value to an unsigned long long.");
			#endif
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}
	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

unsigned long xml_get_xpath_ul(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	unsigned long result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (xpath_ctx == NULL || xpath_query == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL value was passed in.");
		#endif
		return 0;
	}

	// Evalutes the expression.
	xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx);
	if (xpath_obj == NULL) {
	#ifdef DEBUG_FRAMEWORK
		lavalog("Could not evaluate the XPATH query.");
	#endif
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || xpath_obj->nodesetval == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not evaluate into a string.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not return a single node.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children != NULL && xpath_obj->nodesetval->nodeTab[0]->children->content != NULL) {

		// Create a unsigned long long.
		if (extract_ul_ns(xpath_obj->nodesetval->nodeTab[0]->children->content, size_ns(xpath_obj->nodesetval->nodeTab[0]->children->content), &result) != 1) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Could not convert the node value to an unsigned long.");
			#endif
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}
	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

unsigned int xml_get_xpath_ui(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	unsigned int result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (xpath_ctx == NULL || xpath_query == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL value was passed in.");
		#endif
		return 0;
	}

	// Evalutes the expression.
	xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx);
	if (xpath_obj == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not evaluate the XPATH query.");
		#endif
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || xpath_obj->nodesetval == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not evaluate into a string.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not return a single node.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children != NULL && xpath_obj->nodesetval->nodeTab[0]->children->content != NULL) {

		// Create a unsigned long long.
		if (extract_ui_ns(xpath_obj->nodesetval->nodeTab[0]->children->content, size_ns(xpath_obj->nodesetval->nodeTab[0]->children->content), &result) != 1) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Could not convert the node value to an unsigned integer.");
			#endif
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}
	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

unsigned short int xml_get_xpath_us(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	unsigned short int result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (xpath_ctx == NULL || xpath_query == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL value was passed in.");
		#endif
		return 0;
	}

	// Evalutes the expression.
	xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx);
	if (xpath_obj == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not evaluate the XPATH query.");
		#endif
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || xpath_obj->nodesetval == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not evaluate into a string.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not return a single node.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children != NULL && xpath_obj->nodesetval->nodeTab[0]->children->content != NULL) {

		// Create a unsigned long long.
		if (extract_us_ns(xpath_obj->nodesetval->nodeTab[0]->children->content, size_ns(xpath_obj->nodesetval->nodeTab[0]->children->content), &result) != 1) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Could not convert the node value to an unsigned short.");
			#endif
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}
	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

long long xml_get_xpath_ll(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	long long result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (xpath_ctx == NULL || xpath_query == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL value was passed in.");
		#endif
		return 0;
	}

	// Evalutes the expression.
	xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx);
	if (xpath_obj == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not evaluate the XPATH query.");
		#endif
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || xpath_obj->nodesetval == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not evaluate into a string.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not return a single node.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children != NULL && xpath_obj->nodesetval->nodeTab[0]->children->content != NULL) {

		// Create a unsigned long long.
		if (extract_ll_ns(xpath_obj->nodesetval->nodeTab[0]->children->content, size_ns(xpath_obj->nodesetval->nodeTab[0]->children->content), &result) != 1) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Could not convert the node value to an long long.");
			#endif
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}
	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

long xml_get_xpath_l(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	long result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (xpath_ctx == NULL || xpath_query == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL value was passed in.");
		#endif
		return 0;
	}

	// Evalutes the expression.
	xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx);
	if (xpath_obj == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not evaluate the XPATH query.");
		#endif
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || xpath_obj->nodesetval == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not evaluate into a string.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not return a single node.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children != NULL && xpath_obj->nodesetval->nodeTab[0]->children->content != NULL) {

		// Create a unsigned long long.
		if (extract_l_ns(xpath_obj->nodesetval->nodeTab[0]->children->content, size_ns(xpath_obj->nodesetval->nodeTab[0]->children->content), &result) != 1) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Could not convert the node value to an long.");
			#endif
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}
	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

int xml_get_xpath_i(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	int result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (xpath_ctx == NULL || xpath_query == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL value was passed in.");
		#endif
		return 0;
	}

	// Evalutes the expression.
	xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx);
	if (xpath_obj == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not evaluate the XPATH query.");
		#endif
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || xpath_obj->nodesetval == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not evaluate into a string.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not return a single node.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children != NULL && xpath_obj->nodesetval->nodeTab[0]->children->content != NULL) {

		// Create a unsigned long long.
		if (extract_i_ns(xpath_obj->nodesetval->nodeTab[0]->children->content, size_ns(xpath_obj->nodesetval->nodeTab[0]->children->content), &result) != 1) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Could not convert the node value to an integer.");
			#endif
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}
	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

short int xml_get_xpath_s(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	short int result = 0;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (xpath_ctx == NULL || xpath_query == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL value was passed in.");
		#endif
		return 0;
	}

	// Evalutes the expression.
	xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx);
	if (xpath_obj == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not evaluate the XPATH query.");
		#endif
		return 0;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || xpath_obj->nodesetval == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not evaluate into a string.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not return a single node.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children != NULL && xpath_obj->nodesetval->nodeTab[0]->children->content != NULL) {

		// Create a unsigned long long.
		if (extract_s_ns(xpath_obj->nodesetval->nodeTab[0]->children->content, size_ns(xpath_obj->nodesetval->nodeTab[0]->children->content), &result) != 1) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Could not convert the node value to a short.");
			#endif
			xmlXPathFreeObject_d(xpath_obj);
			return 0;
		}
	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return result;
}

stringer_t *xml_get_xpath_st(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	stringer_t *output = NULL;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (xpath_ctx == NULL || xpath_query == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL value was passed in.");
		#endif
		return NULL;
	}

	// Evalutes the expression.
	xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx);
	if (xpath_obj == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not evaluate the XPATH query.");
		#endif
		return NULL;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || xpath_obj->nodesetval == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not evaluate into a string.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return NULL;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not return a single node.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return NULL;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children != NULL && xpath_obj->nodesetval->nodeTab[0]->children->content != NULL) {

		// Create a stringer.
		output = import_ns(xpath_obj->nodesetval->nodeTab[0]->children->content);
		#ifdef DEBUG_FRAMEWORK
		if (output == NULL) {
			lavalog("Unable to import the node into the stringer.");
		}
		#endif
	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return output;
}

char *xml_get_xpath_ns(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	char *output = NULL;
	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (xpath_ctx == NULL || xpath_query == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL value was passed in.");
		#endif
		return NULL;
	}

	// Evalutes the expression.
	xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx);
	if (xpath_obj == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not evaluate the XPATH query.");
		#endif
		return NULL;
	}

	// Lets make sure we got back a string.
	if (xpath_obj->type != XPATH_NODESET || xpath_obj->nodesetval == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not evaluate into a string.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return NULL;
	}

	// Our XPATH should only return one value.
	if (xpath_obj->nodesetval->nodeNr != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not return a single node.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return NULL;
	}

	// If we selected an attribute or an element.
	if ((xpath_obj->nodesetval->nodeTab[0]->type == XML_ATTRIBUTE_NODE || xpath_obj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE)
	  && xpath_obj->nodesetval->nodeTab[0]->children != NULL && xpath_obj->nodesetval->nodeTab[0]->children->content != NULL) {

		// Create a stringer.
		output = duplicate_ns(xpath_obj->nodesetval->nodeTab[0]->children->content);
		#ifdef DEBUG_FRAMEWORK
		if (output == NULL) {
			lavalog("Unable to duplicate the node.");
		}
		#endif
	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return output;
}

sizer_t xml_get_xpath_node_count(xmlXPathContextPtr xpath_ctx, xmlChar * xpath_query) {

	xmlXPathObjectPtr xpath_obj;

	// Sanity check.
	if (xpath_ctx == NULL || xpath_query == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL value was passed in.");
		#endif
		return 0;
	}

	// Evalutes the expression.
	xpath_obj = xmlXPathEvalExpression_d(xpath_query, xpath_ctx);
	if (xpath_obj == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not evaluate the XPATH query.");
		#endif
		return 0;
	}

	// Lets make sure we got back a number.
	if (xpath_obj->type != XPATH_NUMBER) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The XPATH query did not evaluate into a string.");
		#endif
		xmlXPathFreeObject_d(xpath_obj);
		return 0;
	}

	// Free the query object.
	xmlXPathFreeObject_d(xpath_obj);

	return (sizer_t) xpath_obj->floatval;

}

void xml_error(void *ctx, const char *format, ...) {

	va_list args;

	// Simple test on ctx to avoid compiler warning about an unused variable.
	if (ctx == NULL) { }

	va_start(args, format);
	pthread_mutex_lock(&log_mutex);
	
	printf("(%s - %s - %i) = XML Library Error: ", __FILE__, __FUNCTION__, __LINE__);
	vprintf(format, args);
	fflush(stdout);
	
	pthread_mutex_unlock(&log_mutex);
	
	va_end(args);

	return;
}

void xml_free_xpath_ctx(xmlXPathContextPtr ctx) {
	
	if (ctx != NULL) {
		xmlXPathFreeContext_d(ctx);
	}
	#ifdef DEBUG_FRAMEWORK
	else {
		lavalog("Asked to free a NULL context.");
	}
	#endif

	return;
}

void xml_free_xpath_obj(xmlXPathObjectPtr obj) {
	
	if (obj != NULL) {
		xmlXPathFreeObject_d(obj);
	}
	#ifdef DEBUG_FRAMEWORK
	else {
		lavalog("Asked to free a NULL object.");
	}
	#endif

	return;
}

xmlXPathContextPtr xml_create_xpath_ctx(xmlDocPtr doc) {
	
	xmlXPathContextPtr result;

	if (doc == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return NULL;
	}

	result = xmlXPathNewContext_d(doc);
	
	#ifdef DEBUG_FRAMEWORK
	if (result == NULL) {
		lavalog("Could not create a NULL pointer.");
	}
	#endif

	return result;
}

xmlXPathObjectPtr xml_xpath_eval(const char *xpath, xmlXPathContextPtr ctx) {
	
	xmlXPathObjectPtr result;
	
	if (xpath == NULL || ctx == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif 
		return NULL;
	}
	
	result = xmlXPathEvalExpression_d(xpath, ctx);
	
	#ifdef DEBUG_FRAMEWORK
	if (result == NULL) {
		lavalog("Recieved a NULL XPATH object back.");
	}
	#endif
	
	return result;
}
		
xmlParserCtxtPtr xml_create_parser_ctx(void) {

	xmlParserCtxtPtr result;

	result = xmlNewParserCtxt_d();

	#ifdef DEBUG_FRAMEWORK
	if (result == NULL) {
		lavalog("Unable to create an XML parser context.");
	}
	#endif

	// Setup our own error handlers. We can also use the xmlSetGenericErrorFunc function, but I don't like the output.
	if (result != NULL) {
		result->sax->error = &xml_error;
		result->sax->warning = &xml_error;
		result->vctxt.error = &xml_error;
		result->vctxt.warning = &xml_error;
	}

	return result;
}

void xml_free_parser_ctx(xmlParserCtxtPtr ctx) {

	if (ctx != NULL) {
		xmlFreeParserCtxt_d(ctx);
	}
	#ifdef DEBUG_FRAMEWORK
	else {
		lavalog("Asked to free a NULL context.");
	}
	#endif

	return;
}

void xml_free_doc(xmlDocPtr doc) {

	if (doc != NULL) {
		xmlFreeDoc_d(doc);
	}
	#ifdef DEBUG_FRAMEWORK
	else {
		lavalog("Asked to free a NULL document object.");
	}
	#endif

	return;
}

xmlDocPtr xml_create_doc(xmlParserCtxtPtr ctx, const char *buffer, int size, const char *url, const char *encoding, int options) {

	xmlDocPtr result;

	result = xmlCtxtReadMemory_d(ctx, buffer, size, url, encoding, options);
	
	#ifdef DEBUG_FRAMEWORK
	if (result == NULL) {
		lavalog("Unable to create the XML document object.");
	}
	#endif

	return result;
}

void free_xml(void) {

	xmlCleanupParser_d();
	
	if (xml_version != NULL) {
		free_st(xml_version);
		xml_version = NULL;
	}

	#ifdef DEBUG_FRAMEWORK
	lavalog("XML shutdown complete.");
	#endif

	return;
}

int initialize_xml(void) {

	xmlInitParser_d();

	#ifdef DEBUG_FRAMEWORK
	lavalog("The XML library has been initialized.");
	#endif

	return 1;
}

stringer_t * version_xml(void) {
	
	int version;
	int release;
	int major;
	stringer_t *result;
	unsigned long long holder;
	
	if (xml_version != NULL) {
		return xml_version;
	}
	
	holder = extract_unsigned_number((char *)*__xmlParserVersion_d());
	if (holder == 0) {
		return NULL;
	}
	
	result = allocate_st(16);
	if (result == NULL) {
		return NULL;
	}
	
	version = holder % 100;
	holder /= 100;
	release = holder % 100;
	holder /= 100;
	major = holder % 100;
	
	sprintf_st(result, "%i.%i.%i", major, release, version);
	xml_version = result;
	return result;
}

int load_symbols_xml(void) {

	if (lavalib == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The lava library pointer was NULL.");
		#endif
		return 0;
	}

	xmlXPathEvalExpression_d = dlsym(lavalib, "xmlXPathEvalExpression");
	if (xmlXPathEvalExpression_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function xmlXPathEvalExpression.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	xmlXPathFreeObject_d = dlsym(lavalib, "xmlXPathFreeObject");
	if (xmlXPathFreeObject_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function xmlXPathFreeObject.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	xmlNewParserCtxt_d = dlsym(lavalib, "xmlNewParserCtxt");
	if (xmlNewParserCtxt_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function xmlNewParserCtxt.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	xmlCtxtReadMemory_d = dlsym(lavalib, "xmlCtxtReadMemory");
	if (xmlCtxtReadMemory_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function xmlCtxtReadMemory.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	xmlInitParser_d = dlsym(lavalib, "xmlInitParser");
	if (xmlInitParser_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function xmlInitParser.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	xmlCleanupParser_d = dlsym(lavalib, "xmlCleanupParser");
	if (xmlCleanupParser_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function xmlCleanupParser.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	xmlCtxtReadMemory_d = dlsym(lavalib, "xmlCtxtReadMemory");
	if (xmlCtxtReadMemory_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function xmlCtxtReadMemory.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	xmlFreeParserCtxt_d = dlsym(lavalib, "xmlFreeParserCtxt");
	if (xmlFreeParserCtxt_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function xmlFreeParserCtxt.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	xmlFreeDoc_d = dlsym(lavalib, "xmlFreeDoc");
	if (xmlFreeDoc_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function xmlFreeDoc.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	xmlXPathNewContext_d = dlsym(lavalib, "xmlXPathNewContext");
	if (xmlXPathNewContext_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function xmlXPathNewContext.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	xmlXPathFreeContext_d = dlsym(lavalib, "xmlXPathFreeContext");
	if (xmlXPathFreeContext_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function xmlXPathFreeContext.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	__xmlParserVersion_d = dlsym(lavalib, "__xmlParserVersion");
	if (__xmlParserVersion_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function __xmlParserVersion.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}

	#ifdef DEBUG_FRAMEWORK
	lavalog("The XML library symbols have been loaded.");
	#endif

	return 1;
}
