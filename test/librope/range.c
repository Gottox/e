#include <assert.h>
#include <rope.h>
#include <string.h>
#include <testlib.h>

static void
test_range(void) {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	assert(rv == 0);

	rv = rope_append_str(
			&r,
			"Hello World\n"
			"This is a multiline string\n");
	assert(rv == 0);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r, NULL, NULL, NULL);
	assert(rv == 0);

	rv = rope_range_cleanup(&range);
	assert(rv == 0);
	rv = rope_cleanup(&r);
	assert(rv == 0);
}

DECLARE_TESTS
TEST(test_range)
END_TESTS
