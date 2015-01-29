
#include "framework.h"

stringer_t * append_st_st(stringer_t *left, stringer_t *right) {

	if (left == NULL && right == NULL) {
		return NULL;
	}
	else if (left != NULL && right == NULL) {
		return duplicate_st(left);
	}
	else if (left == NULL && right != NULL) {
		return duplicate_st(right);
	}
	
	return merge_strings("ss", left, right);
}

stringer_t * append_st_ns(stringer_t *left, char *right) {

	if (left == NULL && right == NULL) {
		return NULL;
	}
	else if (left != NULL && right == NULL) {
		return duplicate_st(left);
	}
	else if (left == NULL && right != NULL) {
		return import_ns(right);
	}
	
	return merge_strings("sn", left, right);
}
