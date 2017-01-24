
/**
 * @file /check/magma/core/clamp_check.c
 *
 * @brief The clamp function test cases.
 */

#include "magma_check.h"

/**
 * @brief	Check whether the clamp functions properly handle setting numbers to the min legal value in the supplied range.
 * @return	If the test passes, then NULL is returned, otherwise an error message is returned inside an allocated string.
 */
 chr_t * check_clamp_min(void) {

	void *numbers = MEMORYBUF(16);
	int8_t *i8 = numbers;
	int16_t *i16 = numbers;
	int32_t *i32 = numbers;
	int64_t *i64 = numbers;
	uint8_t *ui8 = numbers;
	uint16_t *ui16 = numbers;
	uint32_t *ui32 = numbers;
	uint64_t *ui64 = numbers;

	// Randomly pick two numbers, and call clamp with the max value for the type and see if the right number is returned.
	for (uint64_t i = 0; status() && i < 8192; i++) {

		if (rand_write(PLACER(numbers, 16)) != 16) {
			return ns_dupe("We were unable to allocate the random number of bytes required for the test.");
		}

		// Randomly pick two numbers as the min and max, then call the clamp function with the lowest possible value for the type and ensure the number
		// is reset to the min value.
		if (int8_clamp(i8[0] < i8[1] ? i8[0] : i8[1], i8[0] > i8[1] ? i8[0] : i8[1], INT8_MIN) != (i8[0] < i8[1] ? i8[0] : i8[1]) ||
			int16_clamp(i16[0] < i16[1] ? i16[0] : i16[1], i16[0] > i16[1] ? i16[0] : i16[1], INT16_MIN) != (i16[0] < i16[1] ? i16[0] : i16[1]) ||
			int32_clamp(i32[0] < i32[1] ? i32[0] : i32[1], i32[0] > i32[1] ? i32[0] : i32[1], INT32_MIN) != (i32[0] < i32[1] ? i32[0] : i32[1]) ||
			int64_clamp(i64[0] < i64[1] ? i64[0] : i64[1], i64[0] > i64[1] ? i64[0] : i64[1], INT64_MIN) != (i64[0] < i64[1] ? i64[0] : i64[1]) ||
			uint8_clamp(ui8[0] < ui8[1] ? ui8[0] : ui8[1], ui8[0] > ui8[1] ? ui8[0] : ui8[1], 0) != (ui8[0] < ui8[1] ? ui8[0] : ui8[1]) ||
			uint16_clamp(ui16[0] < ui16[1] ? ui16[0] : ui16[1], ui16[0] > ui16[1] ? ui16[0] : ui16[1], 0) != (ui16[0] < ui16[1] ? ui16[0] : ui16[1]) ||
			uint32_clamp(ui32[0] < ui32[1] ? ui32[0] : ui32[1], ui32[0] > ui32[1] ? ui32[0] : ui32[1], 0) != (ui32[0] < ui32[1] ? ui32[0] : ui32[1]) ||
			uint64_clamp(ui64[0] < ui64[1] ? ui64[0] : ui64[1], ui64[0] > ui64[1] ? ui64[0] : ui64[1], 0) != (ui64[0] < ui64[1] ? ui64[0] : ui64[1])) {
			return ns_dupe("Our attempt to clamp the values to a randomly generated min number was thwarted.");
		}
	}

	return NULL;
}

/**
 * @brief	Check whether the clamp functions properly handle setting numbers to the max legal value in the supplied range.
 * @return	If the test passes, then NULL is returned, otherwise an error message is returned inside an allocated string.
 */
chr_t * check_clamp_max(void) {

	void *numbers = MEMORYBUF(16);
	int8_t *i8 = numbers;
	int16_t *i16 = numbers;
	int32_t *i32 = numbers;
	int64_t *i64 = numbers;
	uint8_t *ui8 = numbers;
	uint16_t *ui16 = numbers;
	uint32_t *ui32 = numbers;
	uint64_t *ui64 = numbers;

	// Randomly pick two numbers, and call clamp with the max value for the type and see if the right number is returned.
	for (uint64_t i = 0; status() && i < 8192; i++) {

		if (rand_write(PLACER(numbers, 16)) != 16) {
			return ns_dupe("We were unable to allocate the random number of bytes required for the test.");
		}

		if (int8_clamp(i8[0] < i8[1] ? i8[0] : i8[1], i8[0] > i8[1] ? i8[0] : i8[1], INT8_MAX) != (i8[0] > i8[1] ? i8[0] : i8[1])
			|| int16_clamp(i16[0] < i16[1] ? i16[0] : i16[1], i16[0] > i16[1] ? i16[0] : i16[1], INT16_MAX) != (i16[0] > i16[1] ? i16[0] : i16[1])
			|| int32_clamp(i32[0] < i32[1] ? i32[0] : i32[1], i32[0] > i32[1] ? i32[0] : i32[1], INT32_MAX) != (i32[0] > i32[1] ? i32[0] : i32[1])
			|| int64_clamp(i64[0] < i64[1] ? i64[0] : i64[1], i64[0] > i64[1] ? i64[0] : i64[1], INT64_MAX) != (i64[0] > i64[1] ? i64[0] : i64[1])
			|| uint8_clamp(ui8[0] < ui8[1] ? ui8[0] : ui8[1], ui8[0] > ui8[1] ? ui8[0] : ui8[1], UINT8_MAX) != (ui8[0] > ui8[1] ? ui8[0] : ui8[1])
			|| uint16_clamp(ui16[0] < ui16[1] ? ui16[0] : ui16[1], ui16[0] > ui16[1] ? ui16[0] : ui16[1], UINT16_MAX) != (ui16[0] > ui16[1] ? ui16[0] : ui16[1])
			|| uint32_clamp(ui32[0] < ui32[1] ? ui32[0] : ui32[1], ui32[0] > ui32[1] ? ui32[0] : ui32[1], UINT32_MAX) != (ui32[0] > ui32[1] ? ui32[0] : ui32[1])
			|| uint64_clamp(ui64[0] < ui64[1] ? ui64[0] : ui64[1], ui64[0] > ui64[1] ? ui64[0] : ui64[1], UINT64_MAX) != (ui64[0] > ui64[1] ? ui64[0] : ui64[1])) {
			return ns_dupe("Our attempt to clamp the values to a randomly generated max number was thwarted.");
		}
	}

	return NULL;
}


 /**
  * @brief	Check whether the clamp functions properly handle setting numbers when the min and max values are identical.
  * @return	If the test passes, then NULL is returned, otherwise an error message is returned inside an allocated string.
  */
 chr_t * check_clamp_min_max_equal(void) {

 	void *numbers = MEMORYBUF(16);
 	int8_t *i8 = numbers;
 	int16_t *i16 = numbers;
 	int32_t *i32 = numbers;
 	int64_t *i64 = numbers;
 	uint8_t *ui8 = numbers;
 	uint16_t *ui16 = numbers;
 	uint32_t *ui32 = numbers;
 	uint64_t *ui64 = numbers;

	// Randomly pick two numbers, and call clamp using the first number as the min and max, and then the second random value
	// as the number. Then see if the result is equal to the first number.
	for (uint64_t i = 0; status() && i < 8192; i++) {

		if (rand_write(PLACER(numbers, 16)) != 16) {
			return ns_dupe("We were unable to allocate the random number of bytes required for the test.");
		}

		if (int8_clamp(i8[0], i8[0], i8[1]) != i8[0] ||
			int16_clamp(i16[0], i16[0], i16[1]) != i16[0] ||
			int32_clamp(i32[0], i32[0], i32[1]) != i32[0] ||
			int64_clamp(i64[0], i64[0], i64[1]) != i64[0] ||
			uint8_clamp(ui8[0], ui8[0], ui8[1]) != ui8[0] ||
			uint16_clamp(ui16[0], ui16[0], ui16[1]) != ui16[0] ||
			uint32_clamp(ui32[0], ui32[0], ui32[1]) != ui32[0] ||
			uint64_clamp(ui64[0], ui64[0], ui64[1]) != ui64[0]) {
			return ns_dupe("Our attempt to clamp the values to a randomly generated number which was passed in as the min and the max didn't work.");
		}

	}

	return NULL;
}

 /**
  * @brief	Check whether the clamp functions properly handle a call where min is larger than max.
  * @return	If the test passes, then NULL is returned, otherwise an error message is returned inside an allocated string.
  */
chr_t * check_clamp_min_max_invalid(void) {

	void *numbers = MEMORYBUF(32);
	int8_t *i8 = numbers;
	int16_t *i16 = numbers;
	int32_t *i32 = numbers;
	int64_t *i64 = numbers;
	uint8_t *ui8 = numbers;
	uint16_t *ui16 = numbers;
	uint32_t *ui32 = numbers;
	uint64_t *ui64 = numbers;

	// Randomly pick two numbers, and call clamp using the first number as the min and max, and then the second random value
	// as the number. Then see if the result is equal to the first number.
	for (uint64_t i = 0; status() && i < 8192; i++) {

		if (rand_write(PLACER(numbers, 32)) != 32) {
			return ns_dupe("We were unable to allocate the random number of bytes required for the test.");
		}

		if ((i8[0] != i8[1] && int8_clamp(i8[0] > i8[1] ? i8[0] : i8[1], i8[0] < i8[1] ? i8[0] : i8[1], i8[2]) != i8[2]) ||
			(i16[0] != i16[1] && int16_clamp(i16[0] > i16[1] ? i16[0] : i16[1], i16[0] < i16[1] ? i16[0] : i16[1], i16[2]) != i16[2]) ||
			(i32[0] != i32[1] && int32_clamp(i32[0] > i32[1] ? i32[0] : i32[1], i32[0] < i32[1] ? i32[0] : i32[1], i32[2]) != i32[2]) ||
			(i64[0] != i64[1] && int64_clamp(i64[0] > i64[1] ? i64[0] : i64[1], i64[0] < i64[1] ? i64[0] : i64[1], i64[2]) != i64[2]) ||
			(ui8[0] != ui8[1] && uint8_clamp(ui8[0] > ui8[1] ? ui8[0] : ui8[1], ui8[0] < ui8[1] ? ui8[0] : ui8[1], ui8[2]) != ui8[2]) ||
			(ui16[0] != ui16[1] && uint16_clamp(ui16[0] > ui16[1] ? ui16[0] : ui16[1], ui16[0] < ui16[1] ? ui16[0] : ui16[1], ui16[2]) != ui16[2]) ||
			(ui32[0] != ui32[1] && uint32_clamp(ui32[0] > ui32[1] ? ui32[0] : ui32[1], ui32[0] < ui32[1] ? ui32[0] : ui32[1], ui32[2]) != ui32[2]) ||
			(ui64[0] != ui64[1] && uint64_clamp(ui64[0] > ui64[1] ? ui64[0] : ui64[1], ui64[0] < ui64[1] ? ui64[0] : ui64[1], ui64[2]) != ui64[2])) {
			return ns_dupe("Our attempt to intentionally use invalid min and max values didn't give us the proper result.");
		}

	}

	return NULL;
}

/**
 * @brief	Check whether the clamp functions work properly. This logic uses random inputs and tries to verify the output.
 * @return	If the test passes, then NULL is returned, otherwise an error message is returned inside an allocated string.
 */
chr_t * check_clamp_randomizer(void) {

	void *numbers = MEMORYBUF(32);
	int8_t *i8 = numbers, i8_result;
	int16_t *i16 = numbers, i16_result;
	int32_t *i32 = numbers, i32_result;
	int64_t *i64 = numbers, i64_result;
	uint8_t *ui8 = numbers, ui8_result;
	uint16_t *ui16 = numbers, ui16_result;
	uint32_t *ui32 = numbers, ui32_result;
	uint64_t *ui64 = numbers, ui64_result;

	// Randomly pick two numbers, and call clamp using the first number as the min and max, and then the second random value
	// as the number. Then see if the result is equal to the first number.
	for (uint64_t i = 0; status() && i < 8192; i++) {

		if (rand_write(PLACER(numbers, 32)) != 32) {
			return ns_dupe("We were unable to allocate the random number of bytes required for a true fuzz test.");
		}

		// Compute the expected integer results.
		i8_result = i8[2] > (i8[0] > i8[1] ? i8[0] : i8[1]) ? (i8[0] > i8[1] ? i8[0] : i8[1]) : (i8[2] < (i8[0] < i8[1] ? i8[0] : i8[1]) ? (i8[0] < i8[1] ? i8[0] : i8[1]) : i8[2]);
		i16_result = i16[2] > (i16[0] > i16[1] ? i16[0] : i16[1]) ? (i16[0] > i16[1] ? i16[0] : i16[1]) : (i16[2] < (i16[0] < i16[1] ? i16[0] : i16[1]) ? (i16[0] < i16[1] ? i16[0] : i16[1]) : i16[2]);
		i32_result = i32[2] > (i32[0] > i32[1] ? i32[0] : i32[1]) ? (i32[0] > i32[1] ? i32[0] : i32[1]) : (i32[2] < (i32[0] < i32[1] ? i32[0] : i32[1]) ? (i32[0] < i32[1] ? i32[0] : i32[1]) : i32[2]);
		i64_result = i64[2] > (i64[0] > i64[1] ? i64[0] : i64[1]) ? (i64[0] > i64[1] ? i64[0] : i64[1]) : (i64[2] < (i64[0] < i64[1] ? i64[0] : i64[1]) ? (i64[0] < i64[1] ? i64[0] : i64[1]) : i64[2]);

		// Compute the expected unsigned integer results.
		ui8_result = ui8[2] > (ui8[0] > ui8[1] ? ui8[0] : ui8[1]) ? (ui8[0] > ui8[1] ? ui8[0] : ui8[1]) : (ui8[2] < (ui8[0] < ui8[1] ? ui8[0] : ui8[1]) ? (ui8[0] < ui8[1] ? ui8[0] : ui8[1]) : ui8[2]);
		ui16_result = ui16[2] > (ui16[0] > ui16[1] ? ui16[0] : ui16[1]) ? (ui16[0] > ui16[1] ? ui16[0] : ui16[1]) : (ui16[2] < (ui16[0] < ui16[1] ? ui16[0] : ui16[1]) ? (ui16[0] < ui16[1] ? ui16[0] : ui16[1]) : ui16[2]);
		ui32_result = ui32[2] > (ui32[0] > ui32[1] ? ui32[0] : ui32[1]) ? (ui32[0] > ui32[1] ? ui32[0] : ui32[1]) : (ui32[2] < (ui32[0] < ui32[1] ? ui32[0] : ui32[1]) ? (ui32[0] < ui32[1] ? ui32[0] : ui32[1]) : ui32[2]);
		ui64_result = ui64[2] > (ui64[0] > ui64[1] ? ui64[0] : ui64[1]) ? (ui64[0] > ui64[1] ? ui64[0] : ui64[1]) : (ui64[2] < (ui64[0] < ui64[1] ? ui64[0] : ui64[1]) ? (ui64[0] < ui64[1] ? ui64[0] : ui64[1]) : ui64[2]);

		if (int8_clamp(i8[0] < i8[1] ? i8[0] : i8[1], i8[0] > i8[1] ? i8[0] : i8[1], i8[2]) != i8_result ||
			int16_clamp(i16[0] < i16[1] ? i16[0] : i16[1], i16[0] > i16[1] ? i16[0] : i16[1], i16[2]) != i16_result ||
			int32_clamp(i32[0] < i32[1] ? i32[0] : i32[1], i32[0] > i32[1] ? i32[0] : i32[1], i32[2]) != i32_result ||
			int64_clamp(i64[0] < i64[1] ? i64[0] : i64[1], i64[0] > i64[1] ? i64[0] : i64[1], i64[2]) != i64_result ||
			uint8_clamp(ui8[0] < ui8[1] ? ui8[0] : ui8[1], ui8[0] > ui8[1] ? ui8[0] : ui8[1], ui8[2]) != ui8_result ||
			uint16_clamp(ui16[0] < ui16[1] ? ui16[0] : ui16[1], ui16[0] > ui16[1] ? ui16[0] : ui16[1], ui16[2]) != ui16_result ||
			uint32_clamp(ui32[0] < ui32[1] ? ui32[0] : ui32[1], ui32[0] > ui32[1] ? ui32[0] : ui32[1], ui32[2]) != ui32_result ||
			uint64_clamp(ui64[0] < ui64[1] ? ui64[0] : ui64[1], ui64[0] > ui64[1] ? ui64[0] : ui64[1], ui64[2]) != ui64_result) {
			return ns_dupe("Our attempt to validate the clamp function results using random inputs didn't give us the expected result.");
		}

	}

	return NULL;
}
