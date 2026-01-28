#define _GNU_SOURCE

#include "../common.h"
#include <assert.h>
#include <getopt.h>
#include <json.h>
#include <rope.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static const char *opts = "icv";
static bool intermediate = false;
static bool integrity_check = false;
static int verbose = 0;

static void
write_file(const char *content, int fd) {
	FILE *f = fdopen(fd, "w");
	fwrite(content, 1, strlen(content), f);
	fclose(f);
}

static int
spawn(char *prog, char *args[]) {
	if (fork() == 0) {
		execvp(prog, args);
		perror("execvp failed");
		_exit(1);
	} else {
		int status;
		wait(&status);
		if (WIFEXITED(status)) {
			return WEXITSTATUS(status);
		} else {
			return -1;
		}
	}
	return 0; // Placeholder return value
}

static void
compare(const char *naive_content, const char *rope_content,
		const char *last_good, json_object *patch) {
	if (strcmp(naive_content, rope_content) == 0) {
		return;
	}
	if (last_good) {
		const char *patch_str = json_object_to_json_string(patch);
		puts(last_good);
		puts(patch_str);
		puts(naive_content);
	}
	char naive_path[256] = "/tmp/naive-XXXXXX",
		 rope_path[256] = "/tmp/rope-XXXXXX";
	int naive_fd = mkstemp(naive_path);
	int rope_fd = mkstemp(rope_path);
	write_file(naive_content, naive_fd);
	write_file(rope_content, rope_fd);
	char *script = "if ! DIFF=$(type -pf difft); then DIFF=\"$(type -pf "
				   "diff) -u\"; fi; "
				   "$DIFF $1 $2; "
				   "rm $1 $2; "
				   "exit 1;";
	char *args[] = {
			"/bin/sh", "-c", script, "", naive_path, rope_path, NULL,
	};
	spawn("/bin/sh", args);
	abort();
}

static void
naive_patch(char **naive_content, size_t pos, size_t del, const char *ins) {
	size_t naive_content_len = strlen(*naive_content);
	size_t ins_len = strlen(ins);
	size_t new_len = naive_content_len - del + ins_len;
	char *new_str = malloc(new_len + 1);
	assert(new_str);
	memcpy(new_str, *naive_content, pos);
	memcpy(new_str + pos, ins, ins_len);
	memcpy(new_str + pos + ins_len, *naive_content + pos + del,
		   naive_content_len - pos - del);
	new_str[new_len] = '\0';
	free(*naive_content);
	*naive_content = new_str;
}

static int
run_patch(
		struct RopeCursor *cursor, char **naive_content, json_object *txn_obj) {
	int rv = 0;
	char *last_good = NULL;
	char *rope_content = NULL;
	json_object *pos_obj = json_object_array_get_idx(txn_obj, 0);
	json_object *del_obj = json_object_array_get_idx(txn_obj, 1);
	json_object *str_obj = json_object_array_get_idx(txn_obj, 2);
	if (!pos_obj || !del_obj || !str_obj) {
		rv = -1;
		goto out;
	}
	size_t pos = json_object_get_int(pos_obj);
	size_t del = json_object_get_int(del_obj);
	const char *data = json_object_get_string(str_obj);
	size_t data_len = json_object_get_string_len(str_obj);
	if (intermediate) {
		last_good = to_str(cursor->rope->root);
	}

	rv = rope_cursor_move_to(cursor, ROPE_CHAR, pos, 0);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_delete(cursor, ROPE_CHAR, del);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_insert_data(cursor, (const uint8_t *)data, data_len, 0);
	if (rv < 0) {
		goto out;
	}
	if (intermediate) {
		naive_patch(naive_content, pos, del, data);

		rope_content = rope_to_str(cursor->rope, 0);
		compare(*naive_content, rope_content, last_good, txn_obj);
	}
	if (verbose) {
		printf("Applied patch at pos %zu: delete %zu, insert \"%s\"\n", pos,
			   del, data);
	}
	if (verbose >= 2) {
		char *str = to_str2(cursor->rope->root);
		fputs(str, stderr);
		fputc('\n', stderr);
		free(str);
	}
	if (integrity_check) {
		check_integrity(cursor->rope->root);
	}
out:
	free(last_good);
	free(rope_content);
	// json_object_put(pos_obj);
	// json_object_put(del_obj);
	// json_object_put(str_obj);
	return rv;
}

static int
run_transaction(
		struct RopeCursor *cursor, char **naive_content, json_object *txn_obj) {
	int rv = 0;
	json_object *patches_obj = json_object_object_get(txn_obj, "patches");
	if (!patches_obj) {
		return -1;
	}

	size_t len = json_object_array_length(patches_obj);
	for (size_t i = 0; i < len; i++) {
		json_object *patch_obj = json_object_array_get_idx(patches_obj, i);
		rv = run_patch(cursor, naive_content, patch_obj);
		if (rv < 0) {
			goto out;
		}
	}
out:
	// json_object_put(patches_obj);
	return rv;
}

static int
run_trace(json_object *trace) {
	char *actual_content = NULL;
	json_object *start_content_obj = NULL;
	json_object *end_content_obj = NULL;
	json_object *txns_obj = NULL;
	char *naive_content = NULL;
	struct RopePool pool = {0};
	struct Rope rope = {0};
	struct RopeCursor cursor = {0};
	int rv = 0;

	rv = rope_pool_init(&pool);

	rv = rope_init(&rope, &pool);
	if (rv < 0) {
		goto out;
	}

	rv = rope_cursor_init(&cursor, &rope);
	if (rv < 0) {
		goto out;
	}

	start_content_obj = json_object_object_get(trace, "startContent");
	end_content_obj = json_object_object_get(trace, "endContent");
	txns_obj = json_object_object_get(trace, "txns");

	const char *start_content = json_object_get_string(start_content_obj);
	rv = rope_cursor_insert_str(&cursor, start_content, 0);
	naive_content = strdup(start_content);

	clock_t time = clock();

	size_t len = json_object_array_length(txns_obj);
	for (size_t i = 0; i < len; i++) {
		if (i % 50000 == 0) {
			printf("Applying transaction %zu / %zu\n", i, len);
		}
		json_object *txn_obj = json_object_array_get_idx(txns_obj, i);
		rv = run_transaction(&cursor, &naive_content, txn_obj);
		if (rv < 0) {
			goto out;
		}
	}

	fprintf(stderr, "finished in %.3lfms\n",
			(double)(clock() - time) * 1000.0 / (double)CLOCKS_PER_SEC);

	const char *end_content = json_object_get_string(end_content_obj);
	actual_content = rope_to_str(&rope, 0);
	compare(end_content, actual_content, NULL, NULL);

out:
	free(naive_content);
	free(actual_content);
	rope_cleanup(&rope);
	rope_pool_cleanup(&pool);
	return 0;
}

int
main(int argc, char *argv[]) {
	int rv = 0;
	json_object *trace = NULL;

	int opt;

	while ((opt = getopt(argc, argv, opts)) != -1) {
		switch (opt) {
		case 'c':
			integrity_check = true;
			break;
		case 'i':
			intermediate = true;
			break;
		case 'v':
			verbose++;
			break;
		default:
			fprintf(stderr, "Unknown option: %c\n", opt);
			return -1;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1) {
		fprintf(stderr, "Usage: %s [options] <trace-file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	trace = json_object_from_file(argv[0]);
	if (!trace) {
		rv = -1;
		goto out;
	}

	rv = run_trace(trace);

out:
	json_object_put(trace);
	return rv == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
