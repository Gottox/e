#define _GNU_SOURCE

#include <assert.h>
#include <json.h>
#include <rope.h>
#include <stdio.h>
#include <string.h>
#include <testlib.h>

static int
naive_patch(char **naive_content, size_t pos, size_t del, const char *ins) {
	size_t naive_content_len = strlen(*naive_content);
	size_t ins_len = strlen(ins);
	size_t new_len = naive_content_len - del + ins_len;
	char *new_str = malloc(new_len + 1);
	if (!new_str) {
		return -1;
	}
	memcpy(new_str, *naive_content, pos);
	memcpy(new_str + pos, ins, ins_len);
	memcpy(new_str + pos + ins_len, *naive_content + pos + del,
		   naive_content_len - pos - del);
	new_str[new_len] = '\0';
	free(*naive_content);
	*naive_content = new_str;

	return 0;
}
static int
run_patch(
		struct RopeCursor *cursor, char **naive_content, json_object *txn_obj) {
	int rv = 0;
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
	const char *str = json_object_get_string(str_obj);

	rv = rope_cursor_move_to_index(cursor, pos, 0);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_delete(cursor, del);
	if (rv < 0) {
		goto out;
	}
	rv = rope_cursor_insert_str(cursor, str, 0);
	if (rv < 0) {
		goto out;
	}
	rv = naive_patch(naive_content, pos, del, str);
	if (rv < 0) {
		goto out;
	}
	rope_content = rope_to_str(cursor->rope, 0);
	ASSERT_STREQ(*naive_content, rope_content);
out:
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
		rv = run_patch(
				cursor, naive_content,
				json_object_array_get_idx(patches_obj, i));
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
	struct Rope rope = {0};
	struct RopeCursor cursor = {0};
	int rv = 0;

	rv = rope_init(&rope);
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

	size_t len = json_object_array_length(txns_obj);
	for (size_t i = 0; i < len; i++) {
		if (i % 10000 == 0) {
			printf("Applying transaction %zu / %zu\n", i, len);
		}
		rv = run_transaction(
				&cursor, &naive_content,
				json_object_array_get_idx(txns_obj, i));
		if (rv < 0) {
			goto out;
		}
	}
	const char *end_content = json_object_get_string(end_content_obj);
	actual_content = rope_to_str(&rope, 0);
	ASSERT_STREQ(end_content, actual_content);

out:
	free(naive_content);
	free(actual_content);
	// json_object_put(start_content_obj);
	// json_object_put(end_content_obj);
	// json_object_put(txns_obj);
	rope_cleanup(&rope);
	return 0;
}

int
main(int argc, char *argv[]) {
	int rv = 0;
	json_object *trace = NULL;

	assert(argc == 2);

	trace = json_object_from_file(argv[1]);
	if (!trace) {
		rv = -1;
		goto out;
	}

	rv = run_trace(trace);

out:
	json_object_put(trace);
	return rv == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
