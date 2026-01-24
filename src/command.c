#include <e.h>

#define DEF(cmd) \
	int e_command_##cmd(struct EKonstrukt *konstrukt) { \
		(void)konstrukt; \
		return 0; \
	}
#include "e_command.def.h"
#undef DEF
