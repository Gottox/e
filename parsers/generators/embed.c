#include <assert.h>
#include <stdio.h>

int
main(int argc, char **argv) {
	assert(argc == 4);
	const char *input = argv[1];
	const char *output = argv[2];
	const char *var_name = argv[3];

	FILE *in = fopen(input, "r");
	assert(in);

	FILE *out = fopen(output, "w");
	assert(out);

	fprintf(out, "const char %s[] = {\n", var_name);
	for (int c; (c = fgetc(in)) != EOF;) {
		fprintf(out, "%d,", c);
	}
	fprintf(out,
			"};\n"
			"const size_t %s_len = sizeof(%s);\n",
			var_name, var_name);
}
