
/**
 * @file /magma/core/parsers/formats/formats.h
 *
 * @brief	Function declarations and types used by the structured format parsers.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_CORE_PARSERS_FORMATS_H
#define MAGMA_CORE_PARSERS_FORMATS_H

/************ NVP ************/

typedef struct {
	MAGMA_INDEX options;
	struct {
		char comment, value, line;
	} tokens;
	inx_t *pairs;
} nvp_t;

nvp_t * nvp_alloc();
void nvp_free(nvp_t *nvp);
void nvp_init(nvp_t *nvp);
int nvp_parse(nvp_t *nvp, stringer_t *data);
/************ NVP ************/

#endif /* FORMATS_H_ */
