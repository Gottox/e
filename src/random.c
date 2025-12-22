#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <time.h>
#include <unistd.h>

struct ERandGen {
	uint32_t counter;
	uint32_t key;
};

static uint16_t
round_function(uint16_t val, uint32_t key, int round) {
	uint32_t subkey;
	if (round % 2) {
		subkey = (uint16_t)(key >> 16);
	} else {
		subkey = (uint16_t)(key & 0xFFFF);
	}

	uint32_t x = ((uint32_t)val ^ subkey) * 0x9E3779B9;

	return (uint16_t)((x >> 16) ^ (x & 0xFFFF));
}

int
e_rand_init(struct ERandGen *randgen) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	uint32_t seed1 = (uint32_t)ts.tv_nsec;
	uint32_t seed2 = (uint32_t)ts.tv_sec ^ (uint32_t)getpid();

	randgen->counter = seed1;
	randgen->key = seed2 ^ 0x9E3779B9;

	return 0;
}

uint32_t
e_rand_next(struct ERandGen *randgen) {
	randgen->counter++;
	uint32_t val = randgen->counter;

	uint16_t left = (uint16_t)(val >> 16);
	uint16_t right = (uint16_t)(val & 0xFFFF);
	uint16_t temp;

	for (int i = 0; i < 8; i++) {
		temp = left;
		left = right;
		right = temp ^ round_function(right, randgen->key, i);
	}

	return ((uint32_t)left << 16) | right;
}
