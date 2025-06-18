#include <rope.h>
#include <string.h>
#include <testlib.h>

static void
test_iterator_simple() {
	int rv = 0;
	struct Rope r = {0};
	rv = rope_init(&r);
	ASSERT_EQ(0, rv);

	rv = rope_append(&r, (uint8_t *)"Hello", 5);
	ASSERT_EQ(0, rv);
	rv = rope_append(&r, (uint8_t *)"World", 5);
	ASSERT_EQ(0, rv);

	struct RopeRange range = {0};
	rv = rope_range_init(&range, &r, NULL, NULL, NULL);
	ASSERT_EQ(0, rv);

	rv = rope_range_cleanup(&range);
}

DECLARE_TESTS
TEST(test_iterator_simple)
END_TESTS
