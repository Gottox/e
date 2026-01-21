#include "rope_node.h"
#include <assert.h>
#include <grapheme.h>
#include <rope.h>
#include <rope_util.h>
#include <string.h>

// Merge bytes from subsequent nodes that form part of the same grapheme
// cluster as the last character of `node`. Returns 0 on success, negative on
// error.
static int
merge_next_grapheme_bytes(
		struct RopeNode *node, uint64_t tags, struct RopePool *pool) {
	int rv = 0;

	for (;;) {
		struct RopeNode *next = rope_node_next(node);
		if (next == NULL || rope_node_tags(next) != tags) {
			break;
		}

		// Get the last character of the current node
		size_t node_size;
		const uint8_t *node_data = rope_node_value(node, &node_size);
		size_t last_char_index = rope_str_last_char_index(&node->data.leaf);

		// Initialize counter from the last char of current node
		struct RopeUtf8Counter counter = {0};
		rope_utf8_char_break(
				&counter, node_data + last_char_index,
				node_size - last_char_index);

		// Check where the first grapheme break is in the next node
		size_t next_size;
		const uint8_t *next_data = rope_node_value(next, &next_size);
		size_t first_break =
				rope_utf8_char_break(&counter, next_data, next_size);

		if (first_break == 0) {
			// No bytes from next need to be merged
			break;
		}

		// We need to merge first_break bytes from next into node
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
			// All of next was merged, delete it.
			// Check if node and next are siblings - if so, node's content
			// will be moved to parent when next is deleted.
			struct RopeNode *parent = rope_node_parent(node);
			bool siblings = parent && rope_node_right(parent) == next;

			rope_node_delete_and_next(next, pool);

			if (siblings) {
				// node was collapsed into parent
				node = parent;
			}
			// Continue loop to check for more nodes to merge
		} else {
			// Skip the merged bytes from next
			rv = rope_node_skip(next, first_break);
			if (rv < 0) {
				return rv;
			}
			rope_node_propagate_dim(node);
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
		if (rope_node_dim(node, ROPE_BYTE) == 0) {
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

		// Compute first_break EARLY to determine merge behavior
		size_t first_break = rope_utf8_char_break(&counter, data, byte_size);

		// Extend reuse_prev when prev == node and entire content is one
		// grapheme AND we're actually going to merge (first_break > 0)
		if (!reuse_prev && prev == node && prev_char_size > 0 &&
			last_char_index == 0 && first_break > 0) {
			rv = rope_str_clone(&prev_char_tmp, &prev->data.leaf);
			if (rv < 0) {
				goto out;
			}
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
			// Only modify prev if we didn't copy to tmp (i.e., prev is still
			// valid)
			if (prev_char_tmp.dim == 0) {
				if (last_char_index > 0) {
					rope_node_truncate(prev, last_char_index);
				} else {
					// Entire prev node is a single grapheme, delete it
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

		// Clean up tmp at end of each iteration
		rope_str_cleanup(&prev_char_tmp);
	}

	rope_node_propagate_dim(node);

	rv = merge_next_grapheme_bytes(node, tags, pool);
	if (rv < 0) {
		goto out;
	}
	new_node = NULL;

out:
	rope_str_cleanup(&prev_char_tmp);
	rope_node_free(new_node, pool);
	return rv;
}

int
rope_node_insert_right(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool) {
	if (byte_size == 0) {
		return 0;
	}

	int rv = -1;
	bool inserted = false;

	if (rope_node_tags(node) == tags) {
		rv = rope_str_inline_append(&node->data.leaf, data, byte_size);
	}
	inserted = rv == 0;

	rv = 0;
	if (!inserted) {
		rv = node_insert_data(
				node, data, byte_size, tags, pool, ROPE_RIGHT);
	} else {
		rope_node_propagate_dim(node);
		rv = merge_next_grapheme_bytes(node, tags, pool);
	}

	return rv;
}

int
rope_node_insert_left(
		struct RopeNode *node, const uint8_t *data, size_t byte_size,
		uint64_t tags, struct RopePool *pool) {
	if (byte_size == 0) {
		return 0;
	}

	struct RopeNode *neighbour = rope_node_prev(node);
	if (neighbour) {
		return rope_node_insert_right(neighbour, data, byte_size, tags, pool);
	} else {
		return node_insert_data(
				node, data, byte_size, tags, pool, ROPE_LEFT);
	}
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
	if (rope_node_dim(node, ROPE_BYTE) == 0) {
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
