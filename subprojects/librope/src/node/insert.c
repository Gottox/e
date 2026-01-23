#include "rope_node.h"
#include "rope_str.h"
#include <assert.h>
#include <grapheme.h>
#include <rope.h>
#include <rope_util.h>
#include <string.h>

static int
merge_next_char_bytes(struct RopeNode *node, struct RopePool *pool) {
	int rv = 0;

	uint64_t tags = rope_node_tags(node);
	for (;;) {
		struct RopeNode *next = rope_node_next(node);
		if (next == NULL || rope_node_tags(next) != tags) {
			break;
		}

		size_t node_size;
		const uint8_t *node_data = rope_node_value(node, &node_size);
		size_t last_char_index = rope_str_last_char_index(&node->data.leaf);

		struct RopeUtf8Counter counter = {0};
		rope_utf8_char_break(
				&counter, node_data + last_char_index,
				node_size - last_char_index);

		size_t next_size;
		const uint8_t *next_data = rope_node_value(next, &next_size);
		size_t first_break =
				rope_utf8_char_break(&counter, next_data, next_size);

		if (first_break == 0) {
			break;
		}

		size_t new_size = node_size + first_break;
		uint8_t *buffer;
		struct RopeStr new_str;
		rv = rope_str_alloc(&new_str, new_size, &buffer);
		if (rv < 0) {
			return rv;
		}

		memcpy(buffer, node_data, node_size);
		memcpy(buffer + node_size, next_data, first_break);

		rope_str_alloc_commit(&new_str, SIZE_MAX);
		rope_str_cleanup(&node->data.leaf);
		rope_str_move(&node->data.leaf, &new_str);

		if (first_break >= next_size) {
			node = rope_node_delete_and_prev(next, pool);
		} else {
			rv = rope_node_skip(next, ROPE_BYTE, first_break);
			if (rv < 0) {
				return rv;
			}
			rope_node_propagate_sizes(node);
			break;
		}
	}

	return 0;
}

static int
node_insert(
		struct RopeNode *target, struct RopeNode *node, struct RopePool *pool,
		enum RopeDirection which) {
	// TODO: allow insertion into branch nodes.
	assert(ROPE_NODE_IS_LEAF(target));
	assert(ROPE_NODE_IS_LEAF(node));

	int rv = 0;
	struct RopeNode *new_node = NULL;

	struct RopeNode *neighbour = rope_node_neighbour(node, which);
	if (neighbour && rope_node_depth(neighbour) < rope_node_depth(node)) {
		node = neighbour;
		which = !which;
	}

	new_node = rope_node_new(pool);
	if (new_node == NULL) {
		goto out;
	}

	rope_node_move(new_node, target);

	rope_node_set_type(target, ROPE_NODE_BRANCH);
	struct RopeNode **children = target->data.branch.children;
	children[which] = node;
	children[!which] = new_node;
	rope_node_update_children(target);
	rope_node_balance_up(new_node);
	new_node = NULL;

out:
	rope_node_free(new_node, pool);
	return rv;
}

static int
node_insert_data(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool, enum RopeDirection which) {
	int rv = 0;
	struct RopeNode *new_node = NULL;
	struct RopeStr prev_char_tmp = {0};
	while (byte_size > 0) {
		struct RopeUtf8Counter counter = {0};

		bool reuse_prev = false;
		struct RopeNode *prev;
		if (rope_node_size(node, ROPE_BYTE) == 0) {
			reuse_prev = true;
			prev = node;
		} else if (which == ROPE_RIGHT) {
			prev = node;
		} else {
			prev = rope_node_prev(node);
		}

		const uint8_t *prev_char = NULL;
		size_t prev_char_size = 0;
		size_t last_char_index = 0;
		if (prev && tags == rope_node_tags(prev)) {
			last_char_index = rope_str_last_char_index(&prev->data.leaf);
			prev_char = rope_node_value(prev, &prev_char_size);
			prev_char += last_char_index;
			prev_char_size -= last_char_index;

			rope_utf8_char_break(&counter, prev_char, prev_char_size);
		}

		size_t first_break = rope_utf8_char_break(&counter, data, byte_size);

		if (!reuse_prev && prev == node && prev_char_size > 0 &&
			last_char_index == 0 && first_break > 0) {
			rope_str_move(&prev_char_tmp, &prev->data.leaf);
			prev_char = rope_str_data(&prev_char_tmp, NULL);
			reuse_prev = true;
		}

		struct RopeNode *new_node;
		if (reuse_prev) {
			rope_node_cleanup(prev);
			new_node = prev;
		} else {
			new_node = rope_node_new(pool);
		}
		if (new_node == NULL) {
			rv = -1;
			goto out;
		}

		size_t chunk_size;
		if (first_break == 0) {
			prev_char_size = 0;
			chunk_size = CX_MIN(byte_size, ROPE_STR_FAST_SIZE);
		} else {
			chunk_size = CX_MIN(byte_size + prev_char_size, ROPE_STR_FAST_SIZE);
			chunk_size = CX_MAX(chunk_size, first_break + prev_char_size);
		}

		uint8_t *buffer = NULL;
		rv = rope_str_alloc(&new_node->data.leaf, chunk_size, &buffer);
		if (rv < 0) {
			goto out;
		}
		if (prev_char_size > 0) {
			memcpy(buffer, prev_char, prev_char_size);
			buffer += prev_char_size;
			chunk_size -= prev_char_size;
			if (prev_char_tmp.dim == 0) {
				if (last_char_index > 0) {
					rv = rope_node_truncate(prev, ROPE_BYTE, last_char_index);
					if (rv < 0) {
						goto out;
					}
				} else {
					rope_node_delete(prev, pool);
				}
			}
		}
		memcpy(buffer, data, chunk_size);
		rope_str_alloc_commit(&new_node->data.leaf, SIZE_MAX);
		rope_node_set_tags(new_node, tags);

		if (!reuse_prev) {
			rv = node_insert(node, new_node, pool, which);
			if (rv < 0) {
				goto out;
			}
		}
		data += chunk_size;
		byte_size -= chunk_size;
		node = new_node;
		which = ROPE_RIGHT;

		rope_str_cleanup(&prev_char_tmp);
	}

	rope_node_propagate_sizes(node);

	struct RopeNode *prev = rope_node_prev(node);
	if (prev) {
		rv = merge_next_char_bytes(prev, pool);
		if (rv < 0) {
			goto out;
		}
	}
	new_node = NULL;

out:
	rope_str_cleanup(&prev_char_tmp);
	rope_node_free(new_node, pool);
	return rv;
}

int
rope_node_insert(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool, enum RopeDirection which) {
	if (byte_size == 0) {
		return 0;
	}

	if (rope_node_tags(node) == tags) {
		size_t position = which == ROPE_LEFT ? 0 : SIZE_MAX;
		int rv = rope_str_inline_insert(&node->data.leaf, ROPE_BYTE, position, data, byte_size);
		if (rv == 0) {
			rope_node_propagate_sizes(node);
			return merge_next_char_bytes(node, pool);
		}
	}

	return node_insert_data(node, data, byte_size, tags, pool, which);
}

int
rope_node_insert_right(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool) {
	return rope_node_insert(node, data, byte_size, tags, pool, ROPE_RIGHT);
}

int
rope_node_insert_left(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool) {
	return rope_node_insert(node, data, byte_size, tags, pool, ROPE_LEFT);
}

static int
node_insert_wrap(
		struct RopeNode *node, uint8_t *data, size_t byte_size, uint64_t tags,
		struct RopePool *pool, enum RopeDirection which) {
	if (byte_size == 0) {
		return 0;
	}

	int rv = 0;
	struct RopeNode *new_node;
	if (rope_node_size(node, ROPE_BYTE) == 0) {
		new_node = node;
	} else {
		new_node = rope_node_new(pool);
		if (new_node == NULL) {
			rv = -1;
			goto out;
		}
	}

	rope_str_wrap(&new_node->data.leaf, data, byte_size);
	rope_node_set_tags(new_node, tags);

	if (new_node != node) {
		rv = node_insert(node, new_node, pool, which);
		if (rv < 0) {
			goto out;
		}
	}

	new_node = NULL;
out:
	rope_node_free(new_node, pool);
	return rv;
}

int
rope_node_insert_heap_left(
		struct RopeNode *node, uint8_t *data, size_t byte_size, uint64_t tags,
		struct RopePool *pool) {
	return node_insert_wrap(node, data, byte_size, tags, pool, ROPE_LEFT);
}

int
rope_node_insert_heap_right(
		struct RopeNode *node, uint8_t *data, size_t byte_size, uint64_t tags,
		struct RopePool *pool) {
	return node_insert_wrap(node, data, byte_size, tags, pool, ROPE_RIGHT);
}
