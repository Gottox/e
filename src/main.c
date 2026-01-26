#include <e.h>
#include <e_command.h>

static struct EKonstrukt e = {
		.commands =
				(const struct ECommand[]){
#define DEF(cmd) {.name = #cmd, .function = e_command_##cmd},
#include <e_command.def.h>
#undef DEF
						{0},
				},
};

int
main(int argc, char *argv[]) {
	return e_main(&e, argc, argv);
}
