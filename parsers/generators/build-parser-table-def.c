#include <ctype.h>
#include <stdio.h>

static void
uppercase(char *target, const char *source) {
	while (*source) {
		*target++ = toupper(*source);
		source++;
	}
	*target = '\0';
}
int
main(int argc, char *argv[]) {
	char upper[256];
	for (int i = 1; i < argc; i++) {
		uppercase(upper, argv[i]);
		printf("DEF(%s, %s)\n", argv[i], upper);
	}
}
