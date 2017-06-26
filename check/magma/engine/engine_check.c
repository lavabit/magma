
/**
 * @file /check/magma/smtp/engine_check.c
 *
 * @brief Magma engine unit tests.
 */

#include "magma_check.h"

START_TEST (check_engine_context_system_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) {

		magma.config.output_resource_limits = true;
		if (result && !system_init_resource_limits()) {
			st_sprint(errmsg, "System limit resource manipulation failed.");
			result = false;
		}

	}

	log_test("ENGINE / CONTEXT / SYSTEM / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

Suite * suite_check_engine(void) {

	Suite *s = suite_create("\tEngine");

	suite_check_testcase(s, "ENGINE", "Engine System Interfaces/S", check_engine_context_system_s);

	return s;
}


