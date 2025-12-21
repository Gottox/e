/*  Written in 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#include <stdint.h>
#include <sys/random.h>

/* This is xoshiro256++ 1.0, one of our all-purpose, rock-solid generators.
   It has excellent (sub-ns) speed, a state (256 bits) that is large
   enough for any parallel application, and it passes all tests we are
   aware of.

   For generating just floating-point numbers, xoshiro256+ is even faster.

   The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to seed a splitmix64 generator and use its
   output to fill s. */

static inline uint64_t
rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

struct ERand {
	uint64_t state[4];
};

int
e_rand_init(struct ERand *state) {
	return getrandom(state->state, sizeof(state->state), 0);
}

uint64_t
e_rand_next(struct ERand *state) {
	const uint64_t result = rotl(state->state[0] + state->state[3], 23) + state->state[0];

	const uint64_t t = state->state[1] << 17;

	state->state[2] ^= state->state[0];
	state->state[3] ^= state->state[1];
	state->state[1] ^= state->state[2];
	state->state[0] ^= state->state[3];

	state->state[2] ^= t;

	state->state[3] = rotl(state->state[3], 45);

	return result;
}
