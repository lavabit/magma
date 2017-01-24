
/**
 * @file /magma/engine/status/performance.c
 *
 * @brief	A collection of functions used to measure and track system performance.
 */

#include "magma.h"

// TODO: Clean up these functions and record the output in our stats table under the useless info heading.
/*
void perf_align8_short (void *data, uint64_t comparison, uint32_t size) {

		uint8_t *data8 = (uint8_t*) data, *data8End = data8 + size, assignment = 0;

		while(data8 != data8End) {
			if (*data8 & comparison) assignment++;
      data8++;
      data8++;
      data8++;
      data8++;
      data8++;
			data8++;
			data8++;
			data8++;
    }

    return;
}

void perf_align8_long (void *data, uint32_t size) {

		uint8_t *data8 = (uint8_t*) data, *data8End = data8 + size, comparison, assignment = 0;

		while(data8 != data8End) {
			comparison = UINT8_MAX;
			while (comparison) {
				if (*data8 & comparison) assignment++;
				comparison /= 2;
			}
      data8++;
    }

    return;
}

void perf_align16_short (void *data, uint64_t comparison, uint32_t size) {

    uint16_t *data16 = (uint16_t*) data, *data16End = data16 + (size / 2), assignment = 0;

    while( data16 != data16End ) {
			if (*data16 & comparison) assignment++;
      data16++;
      data16++;
      data16++;
      data16++;
    }

    return;
}

void perf_align16_long (void *data, uint32_t size) {

    uint16_t *data16 = (uint16_t*) data, *data16End = data16 + (size / 2), comparison, assignment = 0;

    while( data16 != data16End ) {
    	comparison = UINT16_MAX;
			while (comparison) {
				if (*data16 & comparison) assignment++;
				comparison /= 2;
			}
      data16++;
    }

    return;
}

void perf_align32_short (void *data, uint64_t comparison, uint32_t size) {

    uint32_t *data32 = (uint32_t*) data, *data32End = data32 + (size / 4), assignment = 0;

    while (data32 != data32End) {
    	if (*data32 & comparison) assignment++;
      data32++;
      data32++;
    }

    return;
}

void perf_align32_long (void *data, uint32_t size) {

    uint32_t *data32 = (uint32_t*) data, *data32End = data32 + (size / 4), comparison, assignment = 0;

    while (data32 != data32End) {
    	comparison = UINT32_MAX;
			while (comparison) {
				if (*data32 & comparison) assignment++;
				comparison /= 2;
			}
      data32++;
    }

    return;
}

void perf_align64_short (void *data, uint64_t comparison, uint32_t size) {

	uint64_t *data64 = (uint64_t*) data, *data64End = data64 + (size / 8), assignment = 0;

	while (data64 != data64End) {
		if (*data64 & comparison) assignment++;
		data64++;
	}

  return;
}

void perf_align64_long (void *data, uint32_t size) {

	uint64_t *data64 = (uint64_t*) data, *data64End = data64 + (size / 8), comparison, assignment = 0;

	while (data64 != data64End) {
		comparison = UINT64_MAX;
		while (comparison) {
			if (*data64 & comparison) assignment++;
			comparison /= 2;
		}
		data64++;
	}

  return;
}

uint64_t perf_timer_short(void (*func)(void *data, uint64_t comparison, uint32_t size), void *data, uint64_t comparison, uint32_t size) {
	uint64_t start, finish;
	start = perf_rdtsc();
	func(data, comparison, size);
	finish = perf_rdtsc();
	return finish - start;
}

uint64_t perf_timer_long(void (*func)(void *data, uint32_t size), void *data, uint32_t size) {
	uint64_t start, finish;
	start = perf_rdtsc();
	func(data, size);
	finish = perf_rdtsc();
	return finish - start;
}

void perf_align_benchmark(void) {

	uint32_t len = 16 * 1024 * 1024;
	void *data = mm_alloc(len);

	for (uint64_t len = 0; len < 16 * 1024 * 1024; len++) {
		*(((char *)data) + len) = (uint8_t)rand();
	}

	printf("by 8 = %lu\n", perf_timer_short(&perf_align8_short, data, 128, len));
	printf("by 16 = %lu\n", perf_timer_short(&perf_align16_short, data, 128, len));
	printf("by 32 = %lu\n", perf_timer_short(&perf_align32_short, data, 128, len));
	printf("by 64 = %lu\n", perf_timer_short(&perf_align64_short, data, 128, len));

	printf("by 8 = %lu\n", perf_timer_long(&perf_align8_long, data, len));
	printf("by 16 = %lu\n", perf_timer_long(&perf_align16_long, data, len));
	printf("by 32 = %lu\n", perf_timer_long(&perf_align32_long, data, len));
	printf("by 64 = %lu\n", perf_timer_long(&perf_align64_long, data, len));

	return;
}
*/
/**
 * Returns the CPU cycle counter. By calculating variations in the time stamp, we can discover the exact number of
 * elapsed CPU ticks, making it possible to precisely time code execution. Note that in a multithreaded environment
 * its possible that not all of the elapsed ticks were consumed by the code being timed.
 *
 * @return The number of CPU cycles that have elapsed since boot.
 */
uint64_t perf_rdtsc(void) {
	uint32_t hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t)lo)|(((uint64_t)hi) << 32);
}

