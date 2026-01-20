#ifndef CURSOR_INTERNAL_H
#define CURSOR_INTERNAL_H

#include <rope.h>

/* list.c - linked list management */
struct RopeCursor **cursor_detach(struct RopeCursor *cursor);
void cursor_attach_at(struct RopeCursor *cursor, struct RopeCursor **ptr);
void cursor_attach(struct RopeCursor *cursor);
void cursor_bubble_up(struct RopeCursor *cursor);
int cursor_update(struct RopeCursor *cursor);
void cursor_damaged(
		struct RopeCursor *cursor, rope_byte_index_t lower_bound,
		off_t byte_offset);

/* query.c - position queries */
size_t byte_index_to_index(
		struct Rope *rope, rope_byte_index_t byte_idx, enum RopeUnit unit);

/* query.c - internal node query functions */
struct RopeNode *rope_cursor_find_node(
		struct RopeCursor *cursor, struct RopeNode *node, enum RopeUnit unit,
		size_t index, uint64_t tags, rope_byte_index_t *node_byte_index, rope_byte_index_t *local_byte_index);

size_t rope_node_byte_to_index(
		struct RopeNode *node, rope_byte_index_t byte_idx, enum RopeUnit unit);

#endif /* CURSOR_INTERNAL_H */
