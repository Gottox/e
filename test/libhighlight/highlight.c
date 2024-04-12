#include <highlight.h>
#include <utest.h>

UTEST(highlight, init) {
	struct HighlightConfig cfg = {0};
	struct Highlighter hl;
	int rv = highlighter_init(&hl, &cfg);
	ASSERT_EQ(rv, 0);
	highlighter_cleanup(&hl);
}

UTEST_MAIN()
