
/**
 * @file /check/core/qsort_check.c
 *
 * @brief The heart of the suite of unit tests for the Magma core module.
 *
 * $Author: $
 * $Date: $
 * $Revision: $
 *
 */

#include "magma_check.h"
#include "core_data.h"

int check_bsearch_compare(const void *m1, const void *m2) {

	struct check_mi_t *mi1 = (struct check_mi_t *)m1;
	struct check_mi_t *mi2 = (struct check_mi_t *)m2;

	return strcmp(mi1->name, mi2->name);
}

int check_bsearch_months(int num, char *name) {

	struct check_mi_t *res, key;

	key.name = name;

	qsort(months, (sizeof(months) / sizeof(months[0])), sizeof(struct check_mi_t), check_bsearch_compare);
	res = bsearch(&key, months, (sizeof(months) / sizeof(months[0])), sizeof(struct check_mi_t), check_bsearch_compare);
	if (res == NULL || res->nr != num || st_cmp_cs_eq(NULLER(res->name), NULLER(name))) {
		return false;
	}
	return true;
}
