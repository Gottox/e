#include <e_rand_gen.h>
#include <sys/random.h>
#include <unistd.h>

static uint32_t
round_function(uint32_t val, uint64_t key, int round) {
	uint32_t subkey;
	if (round % 2) {
		subkey = (uint32_t)(key >> 32);
	} else {
		subkey = (uint32_t)(key & 0xFFFFFFFF);
	}

	uint64_t x = ((uint64_t)val ^ subkey) * 0x9E3779B97F4A7C15ULL;

	return (uint32_t)((x >> 32) ^ (x & 0xFFFFFFFF));
}

int
e_rand_gen_init(struct ERandGen *rand_gen) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	uint64_t seed1 = (uint64_t)ts.tv_nsec | ((uint64_t)ts.tv_sec << 32);
	uint64_t seed2 = (uint64_t)getpid() ^ 0x9E3779B97F4A7C15ULL;

	rand_gen->counter = seed1;
	rand_gen->key = seed2;

	return 0;
}

uint64_t
e_rand_gen_next(struct ERandGen *rand_gen) {
	uint64_t result;
	do {
		rand_gen->counter++;
		uint64_t val = rand_gen->counter;

		uint32_t left = (uint32_t)(val >> 32);
		uint32_t right = (uint32_t)(val & 0xFFFFFFFF);
		uint32_t temp;

		for (int i = 0; i < 8; i++) {
			temp = left;
			left = right;
			right = temp ^ round_function(right, rand_gen->key, i);
		}
		result = ((uint64_t)left << 32) | right;
	} while (result == 0);
	return result;
}
