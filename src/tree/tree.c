#include <cextras/collection.h>
#include <stdbool.h>
#include <stdio.h>
#include <tree.h>

int
e_tree_view_ascii(
		const struct ETreeVisitorImpl *visitor, void *root_node, FILE *out) {
	int rv = 0;
	struct CxBuffer buffer = {0};
	char prefix[256] = {0};
	int depth = 0;
	void *user_data = NULL;

	rv = cx_buffer_init(&buffer);
	if (rv < 0) {
		goto out;
	}
	void *current_node = root_node;

	while (current_node != NULL) {
		cx_buffer_drain(&buffer);
		rv = visitor->label(current_node, &buffer, user_data);
		bool has_sibling =
				visitor->next_sibling(current_node, user_data) != NULL;
		if (rv < 0) {
			goto out;
		}
		size_t size = cx_buffer_size(&buffer);
		const uint8_t *data = cx_buffer_data(&buffer);
		for (int i = 0; prefix[i]; i++) {
			if (prefix[i] == '|') {
				fputs("│ ", out);
			} else {
				fputs("  ", out);
			}
		}
		if (has_sibling) {
			fputs("├ ", out);
		} else {
			fputs("└ ", out);
		}
		if (size > 0) {
			fwrite(data, 1, size, out);
		}
		fputc('\n', out);

		void *next = visitor->down(current_node, user_data);
		if (next != NULL) {
			if (visitor->next_sibling(current_node, user_data) != NULL) {
				prefix[depth] = '|';
			} else {
				prefix[depth] = ' ';
			}
			depth++;
			current_node = next;
			continue;
		}

		next = visitor->next_sibling(current_node, user_data);
		if (next != NULL) {
			current_node = next;
			continue;
		}

		while (next == NULL && depth > 0) {
			depth--;
			prefix[depth] = '\0';
			current_node = visitor->up(current_node, user_data);
			if (current_node == root_node) {
				break;
			}
			next = visitor->next_sibling(current_node, user_data);
		}
		current_node = next;
	}

out:
	cx_buffer_cleanup(&buffer);
	return rv;
}
