#include "rope_deser.h"
#include <rope.h>
#include <stdint.h>

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	int rv;
	struct Rope rope;
	rv = rope_deserialize(&rope, data, size, false);

	(void)rv; // Suppress unused variable warning
	return 0;
}
