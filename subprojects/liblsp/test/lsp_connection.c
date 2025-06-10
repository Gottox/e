#include <lsp_private.h>
#include <stdlib.h>
#include <string.h>
#include <testlib.h>

static void
test_receive(void) {
	int rv = 0;
	const char *lsp_command[] = {
			"/bin/printf", "%s\r\n", "Content-Length: 5", "", "Hello", NULL,
	};

	struct LSPConnection connection = {0};
	char *response = NULL;
	size_t response_length = 0;

	rv = lsp_connection_client_init(&connection, lsp_command);
	ASSERT_EQ(rv, 0);

	rv = lsp_connection_receive(&connection, &response, &response_length);
	ASSERT_EQ(rv, 0);
	ASSERT_EQ(response_length, 5);
	ASSERT_STREQ("Hello", response);

	free(response);
	lsp_connection_cleanup(&connection);
}

static void
test_receive_line_truncate(void) {
	int rv = 0;
	const char *lsp_command[] = {
			"/bin/printf",
			"%s\r\n",
			"X-OVERSIZED-HEADER-LONGER-THAN: 32 characters",
			"Content-Length: 5",
			"X-OVERSIZED-HEADER-LONGER-THAN: 32 characters",
			"",
			"Hello",
			NULL,
	};
	struct LSPConnection connection = {0};
	char *response = NULL;
	size_t response_length = 0;

	rv = lsp_connection_client_init(&connection, lsp_command);
	ASSERT_EQ(rv, 0);

	rv = lsp_connection_receive(&connection, &response, &response_length);
	ASSERT_EQ(rv, 0);
	ASSERT_EQ(response_length, 5);
	ASSERT_STREQ("Hello", response);

	free(response);
	lsp_connection_cleanup(&connection);
}

static void
test_roundtrip(void) {
	int rv = 0;
	const char *lsp_command[] = {
			"/bin/cat",
			NULL,
	};
	struct LSPConnection connection = {0};
	char *response = NULL;
	size_t response_length = 0;

	rv = lsp_connection_client_init(&connection, lsp_command);
	ASSERT_EQ(rv, 0);

	rv = lsp_connection_send(&connection, "Hello", 5);
	rv = lsp_connection_receive(&connection, &response, &response_length);
	ASSERT_EQ(rv, 0);
	ASSERT_EQ(response_length, 5);
	ASSERT_STREQ("Hello", response);

	free(response);
	lsp_connection_cleanup(&connection);
}

DECLARE_TESTS
TEST(test_receive)
TEST(test_receive_line_truncate)
TEST(test_roundtrip)
END_TESTS
