#include <e.h>
#include <e_command.h>

const struct ECommand commands[] = {
#define DEF(cmd) {.name = #cmd, .function = e_command_##cmd},
#include <e_command.def.h>
#undef DEF
		{.name = "", .function = NULL},
};

static struct EKonstrukt e = {.commands = commands};

int
main(int argc, char *argv[]) {
	return e_main(&e, argc, argv);
}
