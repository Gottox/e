#include <testlib.h>
#include <e_konstrukt.h>
#include <e_struktur.h>

static void
dummy_cleanup(void *data) {
	(void)data;
}

static void test_list_add(void) {
	int rv = 0;
	struct EList list = {0};

	union EStruktur e = {0};
	struct EBase base1 = {0};
	base1.id = 1;
	e.base = &base1;

	rv = e_list_add(&list, &e);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(1, list.cap);
	ASSERT_EQ(1, (int)list.ids[0]);

	struct EBase base2 = {0};
	base2.id = 2;
	e.base = &base2;
	rv = e_list_add(&list, &e);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(2, list.cap);
	ASSERT_EQ(2, (int)list.ids[1]);

	struct EBase base3 = {0};
	base3.id = 3;
	e.base = &base3;
	rv = e_list_add(&list, &e);
	ASSERT_EQ(0, rv);
	ASSERT_EQ(4, list.cap);
	ASSERT_EQ(3, (int)list.ids[2]);

	e_list_cleanup(&list);
}

static void test_list_it(void) {
	int rv = 0;
	struct EKonstrukt k = {0};
	rv = cx_rc_hash_map_init(
			&k.struktur, 16, sizeof(union EStrukturStorage), dummy_cleanup);
	ASSERT_EQ(0, rv);

	union EStrukturStorage s1 = {0};
	s1.base.id = 10;
	void *ptr1 = cx_rc_hash_map_put(&k.struktur, 10, &s1);
	ASSERT_NOT_NULL(ptr1);

	union EStrukturStorage s2 = {0};
	s2.base.id = 20;
	void *ptr2 = cx_rc_hash_map_put(&k.struktur, 20, &s2);
	ASSERT_NOT_NULL(ptr2);

	struct EList list = {0};
	union EStruktur e_temp = {0};
	struct EBase b1 = {0};
	b1.id = 10;
	e_temp.base = &b1;
	rv = e_list_add(&list, &e_temp);
	ASSERT_EQ(0, rv);

	struct EBase b2 = {0};
	b2.id = 20;
	e_temp.base = &b2;
	rv = e_list_add(&list, &e_temp);
	ASSERT_EQ(0, rv);

	union EStruktur e = {0};
	uint64_t it = 0;
	int found = 0;
	while (e_list_it(&e, &k, &list, &it)) {
		ASSERT_NOT_NULL(e.any);
		if (found == 0) {
			ASSERT_EQ(10, (int)e.base->id);
		} else if (found == 1) {
			ASSERT_EQ(20, (int)e.base->id);
		}
		found++;
	}
	ASSERT_EQ(2, found);

	cx_rc_hash_map_cleanup(&k.struktur);
	e_list_cleanup(&list);
}

DECLARE_TESTS
TEST(test_list_add)
TEST(test_list_it)
END_TESTS
