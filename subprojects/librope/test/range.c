#include <rope.h>
#include <string.h>
#include <testlib.h>

static void
range_basic() {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append_str(&r, "Hello World This is a string");
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r, NULL, NULL, NULL);
	ASSERT_EQ(0, rv);

	rv = rope_range_cleanup(&range);
	ASSERT_EQ(0, rv);
	rv = rope_cleanup(&r);
	ASSERT_EQ(0, rv);
}

DECLARE_TESTS
TEST(range_basic)
END_TESTS
