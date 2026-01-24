#include <grapheme.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <rope.h>

// TODO: Implement proper length calculation for unescaped strings in 
// the rope_range context.
static size_t
supid_range_length(struct RopeRange *range, uint64_t tags) {
	char *data = rope_range_to_str(range, tags);
	size_t len = strlen(data);
	free(data);
	return len;
}

char *
unescape_range(struct RopeRange *range, uint64_t tags) {
	size_t len = supid_range_length(range, tags);

	struct RopeIterator iterator = {0};
	rope_iterator_init(&iterator, range, tags);
	char *result = malloc(len + 1);
	
	return 
}
