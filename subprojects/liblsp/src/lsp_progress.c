#include <lsp_progress.h>
#include <string.h>

enum LspProgressValueKind
lsp_progress_value_kind(const struct LspProgressParams *params) {
	if (!params || !params->json) {
		return LSP_PROGRESS_VALUE_UNKNOWN;
	}

	json_object *value = json_object_object_get(params->json, "value");
	if (!value || !json_object_is_type(value, json_type_object)) {
		return LSP_PROGRESS_VALUE_UNKNOWN;
	}

	json_object *kind = json_object_object_get(value, "kind");
	if (!kind || !json_object_is_type(kind, json_type_string)) {
		return LSP_PROGRESS_VALUE_UNKNOWN;
	}

	const char *kind_str = json_object_get_string(kind);
	if (strcmp(kind_str, "begin") == 0) {
		return LSP_PROGRESS_VALUE_BEGIN;
	} else if (strcmp(kind_str, "report") == 0) {
		return LSP_PROGRESS_VALUE_REPORT;
	} else if (strcmp(kind_str, "end") == 0) {
		return LSP_PROGRESS_VALUE_END;
	}

	return LSP_PROGRESS_VALUE_UNKNOWN;
}

/**
 * Validate params and extract the "value" field from ProgressParams JSON.
 */
static json_object *
progress_get_value(const struct LspProgressParams *params, const void *output) {
	if (!params || !params->json || !output) {
		return NULL;
	}
	return json_object_object_get(params->json, "value");
}

int
lsp_progress_value_as_begin(
		const struct LspProgressParams *params,
		struct LspWorkDoneProgressBegin *begin) {
	json_object *value = progress_get_value(params, begin);
	if (!value) {
		return LSP_ERR_MISSING_FIELD;
	}
	return lsp_work_done_progress_begin__from_json(begin, value);
}

int
lsp_progress_value_as_report(
		const struct LspProgressParams *params,
		struct LspWorkDoneProgressReport *report) {
	json_object *value = progress_get_value(params, report);
	if (!value) {
		return LSP_ERR_MISSING_FIELD;
	}
	return lsp_work_done_progress_report__from_json(report, value);
}

int
lsp_progress_value_as_end(
		const struct LspProgressParams *params,
		struct LspWorkDoneProgressEnd *end) {
	json_object *value = progress_get_value(params, end);
	if (!value) {
		return LSP_ERR_MISSING_FIELD;
	}
	return lsp_work_done_progress_end__from_json(end, value);
}

/**
 * Build ProgressParams with the given token and value.
 */
static int
build_progress_params(
		struct LspProgressParams *params,
		const struct LspProgressToken *token,
		json_object *value) {
	int rv = lsp_progress_params__init(params);
	if (rv != LSP_OK) {
		return rv;
	}

	/* Set the token */
	rv = lsp_progress_params__set_token(params,
			(struct LspProgressToken *)token);
	if (rv != LSP_OK) {
		lsp_progress_params__cleanup(params);
		return rv;
	}

	/* Set the value directly on the JSON */
	json_object_object_add(params->json, "value", value);

	return LSP_OK;
}

/**
 * Build ProgressParams from token and value, send the notification, and clean up.
 * Takes ownership of value on success; frees it on failure.
 */
static int
send_progress_value(
		struct Lsp *lsp, const struct LspProgressToken *token,
		json_object *value) {
	struct LspProgressParams params = {0};
	int rv = build_progress_params(&params, token, value);
	if (rv != LSP_OK) {
		json_object_put(value);
		return rv;
	}
	rv = lsp_progress__send(&params, lsp);
	lsp_progress_params__cleanup(&params);
	return rv;
}

int
lsp_progress_begin_send(
		struct Lsp *lsp,
		const struct LspProgressToken *token,
		const char *title,
		const char *message,
		int percentage,
		bool cancellable) {
	if (!lsp || !token || !title) {
		return LSP_ERR_MISSING_FIELD;
	}

	json_object *value = json_object_new_object();
	json_object_object_add(value, "kind", json_object_new_string("begin"));
	json_object_object_add(value, "title", json_object_new_string(title));

	if (message) {
		json_object_object_add(value, "message",
				json_object_new_string(message));
	}
	if (percentage >= 0 && percentage <= 100) {
		json_object_object_add(value, "percentage",
				json_object_new_int(percentage));
	}
	if (cancellable) {
		json_object_object_add(value, "cancellable",
				json_object_new_boolean(1));
	}

	return send_progress_value(lsp, token, value);
}

int
lsp_progress_report_send(
		struct Lsp *lsp,
		const struct LspProgressToken *token,
		const char *message,
		int percentage,
		bool cancellable) {
	if (!lsp || !token) {
		return LSP_ERR_MISSING_FIELD;
	}

	json_object *value = json_object_new_object();
	json_object_object_add(value, "kind", json_object_new_string("report"));

	if (message) {
		json_object_object_add(value, "message",
				json_object_new_string(message));
	}
	if (percentage >= 0 && percentage <= 100) {
		json_object_object_add(value, "percentage",
				json_object_new_int(percentage));
	}
	if (cancellable) {
		json_object_object_add(value, "cancellable",
				json_object_new_boolean(1));
	}

	return send_progress_value(lsp, token, value);
}

int
lsp_progress_end_send(
		struct Lsp *lsp,
		const struct LspProgressToken *token,
		const char *message) {
	if (!lsp || !token) {
		return LSP_ERR_MISSING_FIELD;
	}

	json_object *value = json_object_new_object();
	json_object_object_add(value, "kind", json_object_new_string("end"));

	if (message) {
		json_object_object_add(value, "message",
				json_object_new_string(message));
	}

	return send_progress_value(lsp, token, value);
}

int
lsp_progress_token_from_string(
		struct LspProgressToken *token,
		const char *str) {
	if (!token || !str) {
		return LSP_ERR_MISSING_FIELD;
	}

	token->json = json_object_new_string(str);
	if (!token->json) {
		return LSP_ERR_OOM;
	}

	return LSP_OK;
}

int
lsp_progress_token_from_integer(
		struct LspProgressToken *token,
		int64_t value) {
	if (!token) {
		return LSP_ERR_MISSING_FIELD;
	}

	token->json = json_object_new_int64(value);
	if (!token->json) {
		return LSP_ERR_OOM;
	}

	return LSP_OK;
}
