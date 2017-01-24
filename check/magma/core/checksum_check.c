
/**
 * @file /check/magma/core/checksum_check.c
 *
 * @brief The unit tests for the checksumming functions.
 */

#include "magma_check.h"

void check_checksum_fillbuf(uchr_t *buf, size_t len, uchr_t pos) {
  while (len--) {
    *((uchr_t *)(buf++)) = pos++;
  }
  return;
}

bool_t check_checksum_fuzz_sthread(void) {

	size_t len;
	bool_t result = true;
	byte_t buffer[CHECKSUM_CHECK_SIZE];

	for (uint64_t i = 0; status() && result && i < CHECKSUM_CHECK_ITERATIONS; i++) {

		// Pick a random length.
		len = uint64_clamp(16, CHECKSUM_CHECK_SIZE, (rand() % CHECKSUM_CHECK_SIZE));

		// Fill the buffer with random data and convert the buffer to encrypted.
		if (rand_write(PLACER(buffer, len)) != len) {
			return false;
		}

		crc24_checksum(buffer, len);
		crc32_checksum(buffer, len);
		crc64_checksum(buffer, len);
		crc32_update(buffer, len, 0);
		crc64_update(buffer, len, 0);
		hash_adler32(buffer, len);
		hash_murmur32(buffer, len);
		hash_murmur64(buffer, len);
		hash_fletcher32(buffer, len);

	}

	return result;
}

/**
 * @brief	Looped input checks using the crc24 and crc32 functions. Inspired by similar checks in libgcrypt.
 * @return
 */
bool_t check_checksum_loop_sthread(void) {

	uint32_t crc;
	uchr_t buf[1000];
	size_t left, startlen, piecelen;

	// Loop test for crc32 using 1m "a" chars.
	crc = 0;
	for (int_t i = 0; i < 1000000; i++) crc = crc32_update("a", 1, crc);
	if (crc != 0xdc25bfbc) return false;

	// Loop test for crc24 using 1m "a" chars.
	crc = crc24_init();
	for (int_t i = 0; i < 1000000; i++) crc = crc24_update("a", 1, crc);
	if (crc24_final(crc) != 0xa5cb6b) return false;

	// Loop test for crc32 using 1m chars in an ascending sequence.
	crc = 0;
	startlen = 1;
	left = 1000 * 1000;
	piecelen = startlen;
	for (int_t i = 1; i <= 1000 && left > 0; i++) {
		piecelen = i;
		if (piecelen > sizeof(buf)) piecelen = sizeof(buf);
		if (piecelen > left) piecelen = left;
		check_checksum_fillbuf(buf, piecelen, 1000 * 1000 - left);
		crc = crc32_update(buf, piecelen, crc);
		left -= piecelen;
	}

	while (left > 0) {
		if (piecelen > sizeof(buf)) piecelen = sizeof(buf);
		if (piecelen > left) piecelen = left;
		check_checksum_fillbuf(buf, piecelen, 1000 * 1000 - left);
		crc = crc32_update(buf, piecelen, crc);
		left -= piecelen;
		if (piecelen == sizeof(buf)) piecelen = ++startlen;
		else piecelen = piecelen * 2 - ((piecelen != startlen) ? startlen : 0);
	}

	if (crc != 0x6182291b) return false;

	// Loop test for crc24 using 1m chars in an ascending sequence.
	startlen = 1;
	crc = crc24_init();
	left = 1000 * 1000;
	piecelen = startlen;
	for (int_t i = 1; i <= 1000 && left > 0; i++) {
		piecelen = i;
		if (piecelen > sizeof(buf)) piecelen = sizeof(buf);
		if (piecelen > left) piecelen = left;
		check_checksum_fillbuf(buf, piecelen, 1000 * 1000 - left);
		crc = crc24_update(buf, piecelen, crc);
		left -= piecelen;
	}

	while (left > 0) {
		if (piecelen > sizeof(buf)) piecelen = sizeof(buf);
		if (piecelen > left) piecelen = left;
		check_checksum_fillbuf(buf, piecelen, 1000 * 1000 - left);
		crc = crc24_update(buf, piecelen, crc);
		left -= piecelen;
		if (piecelen == sizeof(buf)) piecelen = ++startlen;
		else piecelen = piecelen * 2 - ((piecelen != startlen) ? startlen : 0);
	}

	if (crc24_final(crc) != 0x7f6703) return false;

	return true;
}

bool_t check_checksum_fixed_sthread(void) {

	bool_t result = true;
	stringer_t *inputs[2] = {
		base64_decode(NULLER("yDgBO22WxBHv7O8X7O/jygAEzol56iUKiXmV+XmpCtmpqQUKiQrFqclFqUDBovzS"
			"vBSFjNSiVHsuAA=="), NULL),
		base64_decode(NULLER("mQENBFfwHDgBCACnDGinN0rpisqfzK1xX1GYDRrBkLMS125J85YUr23JlOfxkubD"
			"amqdb+jQz0RpVViOTrAkpMYMNzlovw6bd0FWG0g9vVvwPcxT6LnbzkRGcyTxmUNC"
			"Bb4U6OTqvqTk7jMWFM1Jo52/B3dpY+UDuKHPLjX3I1OI/DWUW7lrdfIdxrKnPyBI"
			"mOKdbbXwLaH2LLjUjtt/p4aADBFvr3gWK1oPEqOUNGVlgvAkPacT/drAjq+B6317"
			"u2qX8j4DEaKklBjr0ectuzP+sGSq93chyR2qmBlGGWl5c4biofh8pJ7/em8JrWLk"
			"l0+G8xmgpGk4WN1c3fGPtYPUTdCCh8SHfr2JABEBAAG0CVJlYWwgTmFtZYkBOAQT"
			"AQIAIgUCV/AcOAIbAwYLCQgHAwIGFQgCCQoLBBYCAwECHgECF4AACgkQkDXDtcgr"
			"rITdbgf/cEm9cSyrnhUbHEUlXSYLPtT3ZRzEPoXsAibdQuDb2qCeyvuH63vZMP66"
			"ePeniy3deUD1dQDc8YJVyO/IvESpd4hGvE4fxyhc/+u7ornsPWaP5+7DC+56kp8q"
			"05UChLh6rTrmnVfV6Eyaq8tCv3rxOcCJ7T6Nea0rctkJC6NhDWyK/4FnRoHt7PAv"
			"rEXUhpkX+Dg4F9cFtHyS7K56XMrUNmKq6iSjjtWDHUfDbfAZ6dRE1UeE2Pbrdc51"
			"bMFT+wTdnuwQfK5EcBttPljfkTfyMdfHcaC/sBy6EQxf3V0sF5NMxyk5V0aUixQe"
			"T/uh2x2K0Lg1ooQXLEec+Bw9/zPB4Q=="), NULL),
	};

	if (crc24_checksum(st_data_get(inputs[0]), st_length_get(inputs[0])) != 10368269 ||
		crc24_final(crc24_update(st_data_get(inputs[0]), st_length_get(inputs[0]), crc24_init())) != 10368269 ||
		crc32_checksum(st_data_get(inputs[0]), st_length_get(inputs[0])) != 3909602454 ||
		crc64_checksum(st_data_get(inputs[0]), st_length_get(inputs[0])) != 14166404413710800030ULL ||
		crc32_update(st_data_get(inputs[0]), st_length_get(inputs[0]), 0) != 3909602454 ||
		crc64_update(st_data_get(inputs[0]), st_length_get(inputs[0]), 0) != 14166404413710800030ULL ||
		hash_adler32(st_data_get(inputs[0]), st_length_get(inputs[0])) != 4294966238 ||
		hash_murmur32(st_data_get(inputs[0]), st_length_get(inputs[0])) != 1024266973 ||
		hash_murmur64(st_data_get(inputs[0]), st_length_get(inputs[0])) != 2394673536925871334ULL ||
		hash_fletcher32(st_data_get(inputs[0]), st_length_get(inputs[0])) != 2177786754) {
		result = false;
	}
	else if (crc24_checksum(st_data_get(inputs[1]), st_length_get(inputs[1])) != 2790188 ||
		crc24_final(crc24_update(st_data_get(inputs[1]), st_length_get(inputs[1]), crc24_init())) != 2790188 ||
		crc32_checksum(st_data_get(inputs[1]), st_length_get(inputs[1])) != 3782771493 ||
		crc64_checksum(st_data_get(inputs[1]), st_length_get(inputs[1])) != 7822313678304244707ULL ||
		crc32_update(st_data_get(inputs[1]), st_length_get(inputs[1]), 0) != 3782771493 ||
		crc64_update(st_data_get(inputs[1]), st_length_get(inputs[1]), 0) != 7822313678304244707ULL ||
		hash_adler32(st_data_get(inputs[1]), st_length_get(inputs[1])) != 4294965559 ||
		hash_murmur32(st_data_get(inputs[1]), st_length_get(inputs[1])) != 1965527769 ||
		hash_murmur64(st_data_get(inputs[1]), st_length_get(inputs[1])) != 17223540775224389486ULL ||
		hash_fletcher32(st_data_get(inputs[1]), st_length_get(inputs[1])) != 2576491425) {
		result = false;
	}

	st_cleanup(inputs[0]);
	st_cleanup(inputs[1]);

	if (crc32_checksum("", 0) != 0x00000000 ||
		crc32_checksum("foo", 3) != 0x8c736521 ||
		crc32_checksum("123456789", 9) != 0xcbf43926) {
		result = false;
	}

	if (crc24_final(crc24_update("", 0, crc24_init())) != 0xb704ce ||
		crc24_final(crc24_update("foo", 3, crc24_init())) != 0x4fc255 ||
		crc24_final(crc24_update("123456789", 9, crc24_init())) != 0x21cf02 ||
		crc24_checksum("", 0) != 0xb704ce ||
		crc24_checksum("foo", 3) != 0x4fc255 ||
		crc24_checksum("123456789", 9) != 0x21cf02) {
		result = false;
	}

	return result;
}
