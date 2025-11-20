#ifndef ROPE_H
#define ROPE_H

/**
 * @file rope.h
 * @brief Public API for the UTF-8 rope data structure used by the editor core.
 *
 * The rope stores text in a balanced binary tree of leaf nodes and provides
 * cursor, range, and iterator helpers that keep offsets up to date as the tree
 * mutates.
 */

#include <cextras/memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/** Maximum number of bytes stored directly in an inline leaf node. */
#define ROPE_INLINE_LEAF_SIZE (sizeof(void *[2]))
/** Number of @ref RopeNode objects allocated together when using the pool. */
#define ROPE_POOL_CHUNK_SIZE 1024
/** Convenience wrapper to detect addition overflow for size calculations. */
#define ROPE_OVERFLOW_ADD(a, b, res) __builtin_add_overflow(a, b, res)
/** Convenience wrapper to detect subtraction overflow for size calculations. */
#define ROPE_OVERFLOW_SUB(a, b, res) __builtin_sub_overflow(a, b, res)

struct Rope;

/** Index measured in Unicode codepoints. */
typedef size_t rope_char_index_t;
/** Index measured in bytes. */
typedef size_t rope_byte_index_t;
/** Generic rope index type. */
typedef size_t rope_index_t;

/** Newline byte used while counting lines in nodes. */
#define ROPE_NEWLINE '\n'

/**********************************
 * rc_string.c
 */

/**
 * Reference counted container for raw UTF-8 data used by non-inline leaves.
 */
struct RopeRcString {
	struct CxRc rc;
	size_t size;
	uint8_t data[];
};

/**
 * Create a new reference counted string from a single buffer.
 *
 * @param data UTF-8 data to copy into the allocation.
 * @param size Number of bytes to copy.
 *
 * @return Newly allocated reference counted string or NULL on failure.
 */
struct RopeRcString *rope_rc_string_new(const uint8_t *data, size_t size);

/**
 * Create a new reference counted string from two buffers.
 *
 * @param data1 First buffer (may be NULL).
 * @param size1 Size of the first buffer in bytes.
 * @param data2 Second buffer (may be NULL).
 * @param size2 Size of the second buffer in bytes.
 *
 * @return Newly allocated reference counted string or NULL on failure.
 */
struct RopeRcString *rope_rc_string_new2(
		const uint8_t *data1, size_t size1, const uint8_t *data2, size_t size2);

/**
 * Increment the reference count of the given rope string.
 *
 * @param str String to retain (may be NULL).
 *
 * @return The same pointer passed in for chaining.
 */
struct RopeRcString *rope_rc_string_retain(struct RopeRcString *str);

/**
 * Retrieve the raw bytes stored in a rope string.
 *
 * @param rc_str Reference counted string.
 * @param size Out parameter receiving the byte length of the string.
 *
 * @return Pointer to the stored bytes.
 */
const uint8_t *rope_rc_string(struct RopeRcString *rc_str, size_t *size);

/**
 * Decrement the reference count of a rope string and free when it reaches 0.
 *
 * @param str String to release (may be NULL).
 */
void rope_rc_string_release(struct RopeRcString *str);

/**********************************
 * string_nav.c
 */

/**
 * Find the next `\n` within a UTF-8 buffer.
 *
 * @param str Data to scan.
 * @param len Number of bytes available in @p str.
 *
 * @return Index of the next newline or @p len if none is present.
 */
size_t rope_next_line_break(const char *str, size_t len);

/**********************************
 * pool.c
 */

/**
 * Fixed-size allocation pool used to recycle @ref RopeNode instances.
 */
struct RopePool {
	struct CxPreallocPool pool;
};

/**
 * Initialise a rope node pool.
 *
 * @param pool Pool to initialise.
 *
 * @return 0 on success, negative on failure.
 */
int rope_pool_init(struct RopePool *pool);

/**
 * Acquire a @ref RopeNode from the pool.
 *
 * @param pool Pool to allocate from.
 *
 * @return Pointer to an available node or NULL if allocation failed.
 */
struct RopeNode *rope_pool_get(struct RopePool *pool);

/**
 * Recycle a node back into the pool for reuse.
 *
 * @param pool Pool that owns the node.
 * @param node Node to recycle.
 *
 * @return 0 on success.
 */
int rope_pool_recycle(struct RopePool *pool, struct RopeNode *node);

/**
 * Release resources held by a pool. Does not free nodes still in use.
 *
 * @param pool Pool to clean up.
 *
 * @return 0 on success.
 */
int rope_pool_cleanup(struct RopePool *pool);

/**********************************
 * node.c
 */

/** Classification of the different node shapes stored in the rope. */
enum RopeNodeType {
	ROPE_NODE_INLINE_LEAF,
	ROPE_NODE_LEAF,
	ROPE_NODE_BRANCH,
};

/** Direction used when traversing or merging nodes. */
enum RopeNodeDirection {
	ROPE_NODE_LEFT,
	ROPE_NODE_RIGHT,
};

/**
 * Node in the rope tree. Leaf nodes carry text bytes while branch nodes store
 * aggregated sizing and child pointers.
 */
struct RopeNode {
	enum RopeNodeType type;

	union {
		struct {
			uint64_t tags;
			uint8_t data[ROPE_INLINE_LEAF_SIZE];
		} inline_leaf;
		struct {
			uint64_t tags;
			struct RopeRcString *owned;
			const uint8_t *data;
		} leaf;
		struct {
			struct RopeNode *children[2];
			size_t leafs;
		} branch;
	} data;

	struct RopeNode *parent;
	size_t byte_size;
	size_t char_size;
	size_t utf16_size;
	size_t new_lines;
};

/**
 * Allocate a new node from the provided pool and clear its contents.
 *
 * @param pool Pool that supplies node storage.
 *
 *
 * @return Newly allocated node or NULL on failure.
 */
struct RopeNode *rope_node_new(struct RopePool *pool);

/**
 * Check whether a node's tag set includes the provided mask.
 *
 * @param node Node to test (must not be a branch).
 * @param tags Bitmask of tags to match.
 *
 * @return true when the node contains all requested tags.
 */
bool rope_node_match_tags(struct RopeNode *node, uint64_t tags);

/**
 * Replace the contents of a node with a copy of the given buffer.
 *
 * @param node Target node (must be a leaf).
 * @param data Bytes to copy.
 * @param byte_size Size of @p data in bytes.
 *
 * @return 0 on success, negative on allocation/overflow failure.
 */
int rope_node_set_value(
		struct RopeNode *node, const uint8_t *data, size_t byte_size);

/**
 * Point a node at an existing reference-counted string.
 *
 * @param node Target node (must be a leaf).
 * @param str Shared string backing the node.
 * @param byte_index Offset into @p str where the node's data begins.
 *
 * @return 0 on success, negative on overflow.
 */
int rope_node_set_rc_string(
		struct RopeNode *node, struct RopeRcString *str, size_t byte_index);

/**
 * Insert @p new_node immediately to the left of @p target.
 *
 * @param target Node whose left neighbour is being inserted.
 * @param new_node New leaf node to insert.
 * @param pool Pool providing temporary nodes.
 *
 * @return 0 on success, negative on allocation failure.
 */
int rope_node_insert_left(
		struct RopeNode *target, struct RopeNode *new_node,
		struct RopePool *pool);

/**
 * Insert @p new_node immediately to the right of @p target.
 *
 * @param target Node whose right neighbour is being inserted.
 * @param new_node New leaf node to insert.
 * @param pool Pool providing temporary nodes.
 *
 * @return 0 on success, negative on allocation failure.
 */
int rope_node_insert_right(
		struct RopeNode *target, struct RopeNode *new_node,
		struct RopePool *pool);

/**
 * Detach a node from the tree and recycle its storage.
 *
 * @param node Node to remove.
 * @param pool Pool used to recycle nodes.
 *
 * @return 0 on success.
 */
int rope_node_delete(struct RopeNode *node, struct RopePool *pool);

/**
 * Split a leaf node into two nodes at the provided byte offset.
 *
 * @param node Leaf node to split.
 * @param pool Pool for allocating the new nodes.
 * @param byte_index Byte offset where the split should occur.
 * @param left_ptr Optional out pointer receiving the left node.
 * @param right_ptr Optional out pointer receiving the right node.
 *
 * @return 0 on success, negative on allocation failure.
 */
int rope_node_split(
		struct RopeNode *node, struct RopePool *pool, rope_index_t byte_index,
		struct RopeNode **left_ptr, struct RopeNode **right_ptr);

/**
 * Merge a node with its left neighbour when tags and size constraints allow.
 *
 * @param node Starting node for the merge.
 * @param which Direction of the neighbour to merge with.
 * @param pool Pool used for recycling the removed node.
 *
 * @return 0 on success, negative when merge is not possible.
 */
int rope_node_merge_left(struct RopeNode *node, struct RopePool *pool);

/**
 * Merge a node with its right neighbour when tags and size constraints allow.
 *
 * @param node Starting node for the merge.
 * @param which Direction of the neighbour to merge with.
 * @param pool Pool used for recycling the removed node.
 *
 * @return 0 on success, negative when merge is not possible.
 */
int rope_node_merge_right(struct RopeNode *node, struct RopePool *pool);

/**
 * Locate the node that contains the character at the given line and column.
 *
 * @param node Root node to search under.
 * @param line Zero-based line number.
 * @param column Column measured in codepoints.
 * @param tags Tag mask that must be present on matching leaves.
 * @param byte_index Out parameter receiving the byte offset to the character.
 *
 * @return Pointer to the matching leaf node or NULL on failure.
 */
struct RopeNode *rope_node_find(
		struct RopeNode *node, rope_index_t line, rope_index_t column,
		uint64_t tags, rope_byte_index_t *byte_index);

/**
 * Locate the node that owns the provided global character index.
 *
 * @param node Root node to search under.
 * @param char_index Character index within the rope.
 * @param tags Tag mask that must be present on matching leaves.
 * @param byte_index Out parameter receiving the byte offset to the character.
 *
 * @return Pointer to the matching leaf node or NULL on failure.
 */
struct RopeNode *rope_node_find_char(
		struct RopeNode *node, rope_char_index_t char_index, uint64_t tags,
		rope_byte_index_t *byte_index);

/** Return the type of the given node. */
enum RopeNodeType rope_node_type(struct RopeNode *node);

/** Determine whether a node is the left or right child of its parent. */
enum RopeNodeDirection rope_node_which(struct RopeNode *node);

/** Return the sibling of the given node. */
struct RopeNode *rope_node_sibling(struct RopeNode *node);

/** Return the leftmost node reachable from this node. */
struct RopeNode *rope_node_first(struct RopeNode *node);

/** Return the rightmost node reachable from this node. */
struct RopeNode *rope_node_last(struct RopeNode *node);

/** Convenience accessor for the left child of a branch. */
struct RopeNode *rope_node_left(struct RopeNode *node);

/** Convenience accessor for the right child of a branch. */
struct RopeNode *rope_node_right(struct RopeNode *node);

/** Return the parent node or NULL if this is the root. */
struct RopeNode *rope_node_parent(struct RopeNode *node);

/**
 * Move the node pointer to the parent.
 *
 * @param node In-out pointer updated to the parent.
 *
 * @return true when a parent exists.
 */
bool rope_node_up(struct RopeNode **node);

/**
 * Advance to the next node in in-order traversal.
 *
 * @param node In-out pointer updated to the next node.
 *
 * @return true when a sibling exists.
 */
bool rope_node_next(struct RopeNode **node);

/**
 * Move to the previous node in in-order traversal.
 *
 * @param node In-out pointer updated to the previous node.
 *
 * @return true when a sibling exists.
 */
bool rope_node_prev(struct RopeNode **node);

/**
 * Retrieve the raw bytes stored in a leaf and their size.
 *
 * @param node Leaf node to inspect.
 * @param size Out parameter receiving the byte size.
 *
 * @return Pointer to the data or NULL for branch nodes.
 */
const uint8_t *rope_node_value(const struct RopeNode *node, size_t *size);

/**
 * Return the tag bitmask stored on a leaf node.
 *
 * @param node Node whose tags are requested.
 *
 * @return Tag bitmask.
 */
uint64_t rope_node_tags(struct RopeNode *node);

/**
 * Delete all leaves underneath @p node that match @p tags.
 *
 * @param node Branch node to search.
 * @param pool Pool used to recycle removed nodes.
 * @param tags Mask that must be present on leaves to delete.
 *
 * @return 0 on success, negative on errors.
 */
int rope_node_delete_by_tags(
		struct RopeNode *node, struct RopePool *pool, uint64_t tags);

/**
 * Add the provided tags to every leaf under the given node.
 *
 * @param node Root of the subtree to update.
 * @param tags Tags to OR into existing tag sets.
 */
void rope_node_add_tags(struct RopeNode *node, uint64_t tags);

/**
 * Remove the provided tags from every leaf under the given node.
 *
 * @param node Root of the subtree to update.
 * @param tags Tags to clear.
 */
void rope_node_remove_tags(struct RopeNode *node, uint64_t tags);

/**
 * Replace a node's tags with the provided value.
 *
 * @param node Node or subtree to update.
 * @param tags New tag bitmask.
 */
void rope_node_set_tags(struct RopeNode *node, uint64_t tags);

/**
 * Free a node, releasing owned strings and recycling storage.
 *
 * @param node Node to free (may be NULL).
 * @param pool Pool used for recycling.
 *
 * @return 0 on success.
 */
int rope_node_free(struct RopeNode *node, struct RopePool *pool);

/**********************************
 * rope.c
 */

/** Bias applied when balancing the tree after mutations. */
enum RopeBias {
	ROPE_BIAS_LEFT,
	ROPE_BIAS_RIGHT,
};

/**
 * Top-level rope structure holding the root node, cursor list, and allocator.
 */
struct Rope {
	struct RopeNode *root;
	struct RopeCursor *cursors;
	struct RopePool pool;
	enum RopeBias bias;
};

/**
 * Initialise a rope with an empty root node and internal pool.
 *
 * @param rope Rope instance to initialise.
 *
 * @return 0 on success, negative on allocation failure.
 */
int rope_init(struct Rope *rope);

/**
 * Append a byte buffer to the end of the rope.
 *
 * @param rope Rope to mutate.
 * @param data UTF-8 data to append.
 * @param byte_size Number of bytes to append.
 *
 * @return 0 on success, negative on error.
 */
int rope_append(struct Rope *rope, const uint8_t *data, size_t byte_size);

/**
 * Append a null-terminated C string to the rope.
 *
 * @param rope Rope to mutate.
 * @param str String to append.
 *
 * @return 0 on success, negative on error.
 */
int rope_append_str(struct Rope *rope, const char *str);

/**
 * Allocate a new node that is backed by the rope's internal pool.
 *
 * @param rope Rope supplying the pool.
 *
 * @return New node or NULL on allocation failure.
 */
struct RopeNode *rope_new_node(struct Rope *rope);

/**
 * Find the node containing the given character index.
 *
 * @param rope Rope to search.
 * @param char_index Character index within the rope.
 * @param byte_index Out parameter receiving the byte offset into the node.
 *
 * @return Pointer to the matching leaf or NULL on error.
 */
struct RopeNode *rope_find(
		struct Rope *rope, rope_char_index_t char_index,
		rope_byte_index_t *byte_index);

/**
 * Insert bytes at the provided character index.
 *
 * @param rope Rope to mutate.
 * @param index Character index where data should be inserted.
 * @param data Bytes to insert.
 * @param byte_size Size of @p data in bytes.
 *
 * @return 0 on success, negative on failure.
 */
int rope_insert(
		struct Rope *rope, size_t index, const uint8_t *data, size_t byte_size);

/**
 * Delete @p char_count codepoints starting at @p index.
 *
 * @param rope Rope to mutate.
 * @param index Starting character index.
 * @param char_count Number of codepoints to remove.
 *
 * @return 0 on success, negative on error.
 */
int rope_delete(struct Rope *rope, size_t index, size_t char_count);

/** Return the first leaf node in the rope. */
struct RopeNode *rope_first(struct Rope *rope);

/** Release resources and recycle nodes owned by the rope. */
int rope_cleanup(struct Rope *rope);

/** Total number of codepoints stored in the rope. */
int rope_char_size(struct Rope *rope);

/** Total number of bytes stored in the rope. */
int rope_byte_size(struct Rope *rope);

/**
 * Dump the rope structure as a Graphviz DOT file for debugging.
 *
 * @param root Node to print (typically the rope root).
 * @param file Path to the file that will be written.
 */
void rope_node_print(struct RopeNode *root, const char *file);

/**********************************
 * cursor.c
 */

/**
 * Callback invoked when a cursor moves due to edits or explicit positioning.
 */
typedef void (*rope_cursor_callback_t)(
		struct Rope *, struct RopeCursor *, void *);

/**
 * Tracks a position within a rope using byte and character offsets.
 */
struct RopeCursor {
	rope_char_index_t index;
	rope_index_t line;
	rope_char_index_t column;
	struct Rope *rope;
	struct RopeCursor *next;
	rope_cursor_callback_t callback;
	void *userdata;
};

/**
 * Initialise a cursor at the beginning of the rope.
 *
 * @param cursor Cursor to initialise.
 * @param rope Rope the cursor operates on.
 *
 * @return 0 on success.
 */
int rope_cursor_init(struct RopeCursor *cursor, struct Rope *rope);

/**
 * Register a callback invoked when the cursor's offset changes.
 *
 * @param cursor Cursor to configure.
 * @param callback Function invoked after mutations.
 * @param userdata Opaque pointer passed to the callback.
 *
 * @return 0 on success.
 */
int rope_cursor_set_callback(
		struct RopeCursor *cursor, rope_cursor_callback_t callback,
		void *userdata);

/**
 * Compare two cursors to see if @p first appears before or at @p second.
 *
 * @param first Cursor expected to precede @p second.
 * @param second Cursor expected to follow @p first.
 *
 * @return true when the ordering is correct.
 */
bool rope_cursor_is_order(struct RopeCursor *first, struct RopeCursor *second);

/**
 * Move a cursor to the provided global character index.
 *
 * @param cursor Cursor to move.
 * @param char_index Destination index.
 * @param tags Optional tag filter applied while searching.
 *
 * @return 0 on success, negative when the index is out of range.
 */
int rope_cursor_move_to_index(
		struct RopeCursor *cursor, rope_char_index_t char_index, uint64_t tags);

/**
 * Move a cursor to the specified line/column combination.
 *
 * @param cursor Cursor to move.
 * @param line Zero-based line number.
 * @param column Column measured in codepoints.
 *
 * @return 0 on success, negative if the location is invalid.
 */
int rope_cursor_move_to(
		struct RopeCursor *cursor, rope_index_t line, rope_char_index_t column);

/**
 * Insert bytes at the cursor's current position.
 *
 * @param cursor Cursor indicating the insertion point.
 * @param data Bytes to insert.
 * @param byte_size Number of bytes to insert.
 * @param tags Tags to associate with the inserted leaves.
 *
 * @return 0 on success, negative on failure.
 */
int rope_cursor_insert(
		struct RopeCursor *cursor, const uint8_t *data, size_t byte_size,
		uint64_t tags);

/**
 * Move the cursor by a signed offset.
 *
 * @param cursor Cursor to move.
 * @param offset Number of codepoints to move (can be negative).
 *
 * @return 0 on success.
 */
int rope_cursor_move(struct RopeCursor *cursor, off_t offset);

/**
 * Convenience wrapper for inserting a null-terminated string.
 *
 * @param cursor Cursor indicating the insertion point.
 * @param str String to insert.
 * @param tags Tags to associate with inserted leaves.
 *
 * @return 0 on success, negative on failure.
 */
int rope_cursor_insert_str(
		struct RopeCursor *cursor, const char *str, uint64_t tags);

/**
 * Delete @p char_count characters starting at the cursor.
 *
 * @param cursor Cursor indicating the start of deletion.
 * @param char_count Number of codepoints to remove.
 *
 * @return 0 on success.
 */
int rope_cursor_delete(struct RopeCursor *cursor, size_t char_count);

/**
 * Resolve the underlying leaf node and byte offset for the cursor.
 *
 * @param cursor Cursor to inspect.
 * @param byte_index Out parameter receiving the byte offset.
 *
 * @return Leaf node containing the cursor or NULL on failure.
 */
struct RopeNode *
rope_cursor_node(struct RopeCursor *cursor, rope_char_index_t *byte_index);

/**
 * Return the Unicode codepoint at the cursor position.
 *
 * @param cursor Cursor to inspect.
 *
 * @return Codepoint value or -1 on error.
 */
int32_t rope_cursor_codepoint(struct RopeCursor *cursor);

/**
 * Replace the codepoint at the cursor position.
 *
 * @param cursor Cursor to mutate.
 * @param codepoint New codepoint to write.
 *
 * @return 0 on success, negative on failure.
 */
int rope_cursor_set_codepoint(struct RopeCursor *cursor, int32_t codepoint);

/**
 * Detach a cursor from the rope and release resources.
 *
 * @param cursor Cursor to clean up.
 *
 * @return 0 on success.
 */
int rope_cursor_cleanup(struct RopeCursor *cursor);

/**********************************
 * range.c
 */

struct RopeRange;

/**
 * Callback signature used by ranges to signal offset changes or damage.
 */
typedef void (*rope_range_callback_t)(
		struct Rope *rope, struct RopeRange *cursor, void *userdata);

/**
 * Represents a half-open selection spanning two cursors.
 */
struct RopeRange {
	struct Rope *rope;
	struct RopeCursor cursors[2];
	bool is_collapsed;
	rope_range_callback_t offset_change_callback;
	rope_range_callback_t damage_callback;
	void *userdata;
};

/**
 * Initialise a range over the given rope.
 *
 * @param range Range to initialise.
 * @param rope Rope backing the selection.
 * @param offset_change_callback Callback invoked when start offset changes.
 * @param damage_callback Callback invoked when non-collapsed ranges are edited.
 * @param userdata Opaque pointer forwarded to callbacks.
 *
 * @return 0 on success, negative on error.
 */
int rope_range_init(
		struct RopeRange *range, struct Rope *rope,
		rope_range_callback_t offset_change_callback,
		rope_range_callback_t damage_callback, void *userdata);

/** Return the earlier cursor in the range. */
struct RopeCursor *rope_range_start(struct RopeRange *range);

/** Return the later cursor in the range. */
struct RopeCursor *rope_range_end(struct RopeRange *range);

/**
 * Update the tag mask stored on the range for later operations.
 *
 * @param range Range whose tag mask should change.
 * @param tags Tags to associate with the range.
 */
void rope_range_set_tags(struct RopeRange *range, uint64_t tags);

/**
 * Read the tag mask stored on the range.
 *
 * @param range Range to inspect.
 *
 * @return Current tag mask.
 */
uint64_t rope_range_get_tags(struct RopeRange *range);

/**
 * Insert a null-terminated string, replacing the current selection if any.
 *
 * @param range Range to mutate.
 * @param str String to insert.
 * @param tags Tags to associate with inserted data.
 *
 * @return 0 on success, negative on failure.
 */
int
rope_range_insert_str(struct RopeRange *range, const char *str, uint64_t tags);

/**
 * Delete the contents of the range.
 *
 * @param range Range to mutate.
 *
 * @return 0 on success.
 */
int rope_range_delete(struct RopeRange *range);

/**
 * Materialise the text represented by the range into a newly allocated string.
 *
 * @param range Range to read from.
 * @param tags Filter to include only leaves with these tags (0 for all).
 *
 * @return Null-terminated string or NULL on failure. Caller owns the result.
 */
char *rope_range_to_str(struct RopeRange *range, uint64_t tags);

/**
 * Release resources held by the range's cursors.
 *
 * @param range Range to clean up.
 *
 * @return 0 on success.
 */
int rope_range_cleanup(struct RopeRange *range);

/**********************************
 * iterator.c
 */

/**
 * Iterator used to walk the leaves that make up a range.
 */
struct RopeIterator {
	struct RopeRange *range;
	struct RopeNode *node;
	struct RopeNode *end;
	rope_byte_index_t start_byte;
	rope_byte_index_t end_byte;
	bool started;
	uint64_t tags;
};

/**
 * Prepare an iterator that will walk the provided range.
 *
 * @param iter Iterator to initialise.
 * @param range Range to iterate over.
 * @param tags Optional tag mask filter (0 to include all).
 *
 * @return 0 on success.
 */
int rope_iterator_init(
		struct RopeIterator *iter, struct RopeRange *range, uint64_t tags);

/**
 * Advance the iterator to the next contiguous chunk of bytes.
 *
 * @param iter Iterator to advance.
 * @param value Out parameter receiving pointer to the bytes.
 * @param size Out parameter receiving the number of bytes.
 *
 * @return true while more data is available.
 */
bool rope_iterator_next(
		struct RopeIterator *iter, const uint8_t **value, size_t *size);

/**
 * Clean up any iterator state.
 *
 * @param iter Iterator to clean up.
 *
 * @return 0 on success.
 */
int rope_iterator_cleanup(struct RopeIterator *iter);

#endif
