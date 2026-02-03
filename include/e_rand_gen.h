#ifndef E_RAND_GEN_H
#define E_RAND_GEN_H

#include "e_common.h"

struct ERandGen {
	uint64_t counter;
	uint64_t key;
};

int
e_rand_gen_init(struct ERandGen *randgen);

uint64_t
e_rand_gen_next(struct ERandGen *randgen);


#endif /* E_RAND_GEN_H */
