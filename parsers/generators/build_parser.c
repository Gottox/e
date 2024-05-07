#include <assert.h>
#include <linux/limits.h>
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char *argv[]) {
	int rv = 0;
	char name_buf[PATH_MAX] = {0};
	assert(argc == 4);
	const char *grammar_name = argv[1];
	const char *grammar_dir = argv[2];
	const char *output_dir = argv[3];

	rv = chdir(output_dir);
	assert(rv == 0);

	snprintf(name_buf, sizeof(name_buf) - 1, "%s-parser.c", grammar_name);
	FILE *parser_file = fopen(name_buf, "w");
	fprintf(parser_file, "#include \"%s/src/parser.c\"", grammar_dir);
	fclose(parser_file);

	snprintf(
			name_buf, sizeof(name_buf) - 1, "%s/%s-scanner.c", output_dir,
			grammar_name);
	rv = access(name_buf, F_OK);
	FILE *scanner_file = fopen(name_buf, "w");
	snprintf(name_buf, sizeof(name_buf) - 1, "%s/src/scanner.c", grammar_dir);
	rv = access(name_buf, F_OK);
	if (rv == 0) {
		fprintf(scanner_file, "#include \"%s/src/scanner.c\"", grammar_dir);
	} else {
		fprintf(scanner_file, "void E_OPTIMIZE_ME_OUT_%s() {}", grammar_name);
	}
	fclose(scanner_file);
}
