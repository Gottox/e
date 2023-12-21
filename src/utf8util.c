#include <utf8util.h>

ssize_t utf8_strlen(const char *s) {
	ssize_t len = 0;
	while (*s) {
		if ((*s & 0xc0) != 0x80) len++;
		s++;
	}
	return len;
}
