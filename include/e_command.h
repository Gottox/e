#ifndef E_COMMAND_H
#define E_COMMAND_H

#include "e.h"

typedef int (*e_command_function_t)(struct EKonstrukt *konstrukt);

struct ECommand {
	char name[16];
	e_command_function_t function;
};

#define DEF(cmd) int e_command_##cmd (struct EKonstrukt *konstrukt);
#include "e_command.def.h"
#undef DEF

#endif /* E_COMMAND_H */
