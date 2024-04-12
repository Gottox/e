#include <e.h>
#include <libgen.h>
#include <quickjs/quickjs.h>
#include <rope.h>
#include <string.h>
#include <sys/prctl.h>
#include <tree.h>
#include <ttyui.h>

int
main(int argc, char *argv[]) {
	// Allow being traced.
	prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);
	enum { E_CLIENT, E_DAEMON } mode = E_CLIENT;

	if (strcmp(basename(argv[0]), "e-daemon") == 0) {
		mode = E_DAEMON;
	} else if (argc == 1) {
		mode = E_CLIENT;
	} else if (strcmp(argv[1], "daemon") == 0) {
		mode = E_DAEMON;
		argc--;
		argv++;
	} else if (strcmp(argv[1], "client") == 0) {
		mode = E_CLIENT;
		argc--;
		argv++;
	}

	switch (mode) {
	case E_CLIENT:
		return e_client(argc, argv);
	case E_DAEMON:
		return e_daemon(argc, argv);
	}
}
