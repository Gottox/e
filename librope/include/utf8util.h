#include <stdint.h>
#include <stdlib.h>

ssize_t utf8_clen(const uint8_t *str, size_t length);

ssize_t utf8_16len(const uint8_t *str, size_t length);

ssize_t utf8_bidx(const uint8_t *str, size_t length, size_t char_index);
