#include "cursor_internal.h"
#include <rope.h>
#include <string.h>

bool
rope_cursor_starts_with_data(
		struct RopeCursor *cursor, const uint8_t *prefix, size_t prefix_size) {
	int rv = 0;

	size_t local_byte_index;

	size_t byte_size;
	struct RopeNode *node = rope_cursor_find_node(
			cursor, cursor->rope->root, ROPE_BYTE, cursor->byte_index, 0, NULL,
			&local_byte_index);
	while (node && byte_size < prefix_size) {
		const uint8_t *data = rope_node_value(node, &byte_size);

		data += local_byte_index;
		byte_size -= local_byte_index;

		rv = memcmp(data, prefix, byte_size);
		if (rv != 0) {
			return false;
		}
		prefix += byte_size;
		prefix_size -= CX_MIN(byte_size, prefix_size);

		node = rope_node_next(node);
		local_byte_index = 0;
	}

	return prefix_size == 0;
}
