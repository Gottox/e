#include "lsp_util.h"
#include <stdio.h>
#include <string.h>

void
lsp_response_set_invalid_params(json_object *response) {
	json_object *err = json_object_new_object();
	json_object_object_add(err, "code", json_object_new_int(-32602));
	json_object_object_add(err, "message",
			json_object_new_string("Invalid params"));
	json_object_object_add(response, "error", err);
}

void
lsp_response_set_handler_error(json_object *response,
		struct LspResponseError *error, int err_code) {
	if (json_object_object_length(error->json) > 0) {
		json_object_object_add(response, "error", error->json);
	} else {
		lsp_response_error__set_code(error, err_code);
		lsp_response_error__set_message(error, "Request failed");
		json_object_object_add(response, "error", error->json);
	}
}

void
lsp_response_set_result(json_object *response,
		struct LspResponseError *error, json_object *result) {
	lsp_response_error__cleanup(error);
	json_object_object_add(response, "result", result);
}

void
lsp_response_error__init(struct LspResponseError *err) {
	err->json = json_object_new_object();
}

void
lsp_response_error__cleanup(struct LspResponseError *err) {
	json_object_put(err->json);
	err->json = NULL;
}

int64_t
lsp_response_error__code(const struct LspResponseError *err) {
	return json_object_get_int64(json_object_object_get(err->json, "code"));
}

const char *
lsp_response_error__message(const struct LspResponseError *err) {
	return json_object_get_string(json_object_object_get(err->json, "message"));
}

json_object *
lsp_response_error__data(const struct LspResponseError *err) {
	return json_object_object_get(err->json, "data");
}

void
lsp_response_error__set_code(struct LspResponseError *err, int64_t code) {
	json_object_object_add(err->json, "code", json_object_new_int64(code));
}

void
lsp_response_error__set_message(struct LspResponseError *err, const char *message) {
	json_object_object_add(err->json, "message", json_object_new_string(message));
}

void
lsp_response_error__set_data(struct LspResponseError *err, json_object *data) {
	json_object_object_add(err->json, "data", json_object_get(data));
}

json_object *
lsp_request_new(const char *method, json_object *params, json_object *id) {
	json_object *req = json_object_new_object();
	json_object_object_add(req, "jsonrpc", json_object_new_string("2.0"));
	json_object_object_add(req, "id", json_object_get(id));
	json_object_object_add(req, "method", json_object_new_string(method));
	if (params != NULL) {
		json_object_object_add(req, "params", params);
	}
	return req;
}

json_object *
lsp_notification_new(const char *method, json_object *params) {
	json_object *notif = json_object_new_object();
	json_object_object_add(notif, "jsonrpc", json_object_new_string("2.0"));
	json_object_object_add(notif, "method", json_object_new_string(method));
	if (params != NULL) {
		json_object_object_add(notif, "params", params);
	}
	return notif;
}
