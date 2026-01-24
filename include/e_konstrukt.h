#ifndef E_KONSTRUKT_H
#define E_KONSTRUKT_H

#include "e_list.h"
#include "e_rand_gen.h"

struct EKonstrukt {
	bool running;
	struct CxRcHashMap struktur;

	int timeout_ms;

	struct EList dokuments;

	struct EList klients;

	struct RopePool rope_pool;

	const struct ECommand *commands;

	struct pollfd *poll_list;

	struct ERandGen rand_gen;
};

int e_init(struct EKonstrukt *konstrukt, int argc, char *argv[]);

int e_main(struct EKonstrukt *konstrukt, int argc, char *argv[]);

void e_cleanup(struct EKonstrukt *konstrukt);

#endif /* E_KONSTRUKT_H */
