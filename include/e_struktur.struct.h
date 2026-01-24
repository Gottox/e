STRUCT(EBase, base, {
	const struct EStrukturType *type;
	uint64_t id;
	struct EKonstrukt *konstrukt;
});

STRUCT(EDokument, dokument, {
	uint64_t mode;

	struct EBase base;

	struct EList klients;

	struct Rope content;
});

STRUCT(ECursor, cursor, {
	struct EBase base;

	struct EDokument *dokument;
	struct EKlient *klient;

	struct RopeRange range;
});

STRUCT(EKlient, klient, {
	struct EBase base;

	struct EList cursors;
	size_t cursor_count;

	struct EList endpunkts;
	size_t endpunkt_count;

	struct Rope input_buffer;
	struct Rope output_buffer;

	int writer_fd;
	int reader_fd;
});

STRUCT(EEndpunkt, endpunkt, {
	struct EBase base;

	uint64_t klient_id;

	char name[128];

	struct ECursor *cursors;
	size_t cursor_count;

	enum ETerminationMode {
		E_TERMINATION_INLINE_STRING,
		E_TERMINATION_SIZED,
		E_TERMINATION_SEQUENCE,
	} termination_mode;
});

STRUCT(ERequest, request, {
	struct EBase base;

	struct timespec timeout;
	uint64_t sender_klient_id;
});
