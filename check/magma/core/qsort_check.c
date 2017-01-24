
/**
 * @file /check/magma/core/qsort_check.c
 *
 * @brief The heart of the suite of unit tests for the Magma core module.
 */

#include "magma_check.h"

struct check_mi_t months[] = {
	{ 1, "jan"},
	{ 2, "feb"},
	{ 3, "mar"},
	{ 4, "apr"},
	{ 5, "may"},
	{ 6, "jun"},
	{ 7, "jul"},
	{ 8, "aug"},
	{ 9, "sep"},
	{ 10, "oct"},
	{ 11, "nov"},
	{ 12, "dec"}
};

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
