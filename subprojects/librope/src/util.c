#include <assert.h>
#include <cextras/macro.h>
#include <grapheme.h>
#include <rope_util.h>
#include <string.h>

size_t
rope_utf8_char_break(
		struct RopeUtf8Counter *counter, const uint8_t *data, size_t size) {
	uint_least32_t last_cp = GRAPHEME_INVALID_CODEPOINT, cp;
	const uint8_t *p = data;

	if (counter->buffer_size != 0) {
		memcpy(&counter->buffer[counter->buffer_size], p, CX_MIN(4, size));
		size_t cp_size = grapheme_decode_utf8(counter->buffer, 4, &last_cp);
		cp_size -= counter->buffer_size;
		p += cp_size;
		size -= cp_size;

		memset(counter->buffer, 0, sizeof(counter->buffer));
		counter->buffer_size = 0;
	}

	for (;;) {
		size_t cp_size = grapheme_decode_utf8((const char *)p, size, &cp);
		if (cp_size > size) {
			memcpy(counter->buffer, p, size);
			counter->buffer_size = size;
			p += size;
			break;
		} else if (grapheme_is_character_break(last_cp, cp, &counter->state)) {
			memset(counter, 0, sizeof(struct RopeUtf8Counter));
			break;
		}
		last_cp = cp;
		p += cp_size;
		size -= cp_size;
	}

	return p - data;
}
