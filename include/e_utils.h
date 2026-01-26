#ifndef E_UTILS
#define E_UTILS

#include <e_common.h>

int e_push_array(void **array, int *size, void *value);

int e_parse_unsigned_dec(struct RopeCursor *cursor, uint64_t *number);

int e_skip_whitespace(struct RopeCursor *cursor);

#endif
