#include "lsp_util.h"
#include <stdio.h>
#include <string.h>

/**
 * Create a new JSON-RPC 2.0 message object with "jsonrpc":"2.0" already set.
 */
static json_object *
jsonrpc_new(void) {
	json_object *obj = json_object_new_object();
	if (obj == NULL) {
		return NULL;
	}
	if (json_object_object_add(obj, "jsonrpc", json_object_new_string("2.0")) < 0) {
		json_object_put(obj);
		return NULL;
	}
	return obj;
}

enum LspMessageKind
lsp_message_classify(json_object *msg) {
	if (!msg) return LSP_MESSAGE_INVALID;

	json_object *method = json_object_object_get(msg, "method");
	json_object *id = json_object_object_get(msg, "id");

	if (method) {
		return id ? LSP_MESSAGE_REQUEST : LSP_MESSAGE_NOTIFICATION;
	}
	if (id) {
		// Response has id but no method, plus result or error
		// Use _get_ex because json_object_object_get returns NULL for null values
		bool has_result = json_object_object_get_ex(msg, "result", NULL);
		bool has_error = json_object_object_get_ex(msg, "error", NULL);
		if (has_result || has_error) return LSP_MESSAGE_RESPONSE;
	}
	return LSP_MESSAGE_INVALID;
}

json_object *
lsp_response_new(json_object *id, json_object *result) {
	json_object *resp = jsonrpc_new();
	if (resp == NULL) {
		return NULL;
	}

	int rv = json_object_object_add(resp, "id", json_object_get(id));
	if (rv < 0) {
		goto err;
	}
	rv = json_object_object_add(resp, "result", result);
	if (rv < 0) {
		goto err;
	}
	return resp;
err:
	json_object_put(resp);
	return NULL;
}

json_object *
lsp_response_error_new(json_object *id, int code, const char *message) {
	json_object *resp = jsonrpc_new();
	if (resp == NULL) {
		return NULL;
	}

	int rv = json_object_object_add(resp, "id", json_object_get(id));
	if (rv < 0) {
		goto err;
	}

	json_object *err_obj = json_object_new_object();
	if (err_obj == NULL) {
		goto err;
	}
	rv = json_object_object_add(err_obj, "code", json_object_new_int(code));
	if (rv < 0) {
		json_object_put(err_obj);
		goto err;
	}
	rv = json_object_object_add(err_obj, "message", json_object_new_string(message));
	if (rv < 0) {
		json_object_put(err_obj);
		goto err;
	}
	rv = json_object_object_add(resp, "error", err_obj);
	if (rv < 0) {
		json_object_put(err_obj);
		goto err;
	}
	return resp;
err:
	json_object_put(resp);
	return NULL;
}

void
lsp_response_set_invalid_params(json_object *response) {
	json_object *err = json_object_new_object();
	json_object_object_add(err, "code", json_object_new_int(-32602));
	json_object_object_add(err, "message",
			json_object_new_string("Invalid params"));
	json_object_object_add(response, "error", err);
}

int
lsp_response_set_handler_error(json_object *response,
		struct LspResponseError *error, int err_code) {
	int rv = 0;
	if (json_object_object_length(error->json) > 0) {
		rv = json_object_object_add(response, "error", error->json);
		if (rv < 0) {
			return LSP_ERR_OOM;
		}
	} else {
		rv = lsp_response_error__set_code(error, err_code);
		if (rv < 0) {
			return rv;
		}
		rv = lsp_response_error__set_message(error, "Request failed");
		if (rv < 0) {
			return rv;
		}
		rv = json_object_object_add(response, "error", error->json);
		if (rv < 0) {
			return LSP_ERR_OOM;
		}
	}
	return LSP_OK;
}

void
lsp_response_set_result(json_object *response,
		struct LspResponseError *error, json_object *result) {
	lsp_response_error__cleanup(error);
	json_object_object_add(response, "result", result);
}

int
lsp_response_error__init(struct LspResponseError *err) {
	err->json = json_object_new_object();
	if (err->json == NULL) {
		return LSP_ERR_OOM;
	}
	return LSP_OK;
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

int
lsp_response_error__set_code(struct LspResponseError *err, int64_t code) {
	json_object *val = json_object_new_int64(code);
	if (val == NULL) {
		return LSP_ERR_OOM;
	}
	if (json_object_object_add(err->json, "code", val) < 0) {
		return LSP_ERR_OOM;
	}
	return LSP_OK;
}

int
lsp_response_error__set_message(struct LspResponseError *err, const char *message) {
	json_object *val = json_object_new_string(message);
	if (val == NULL) {
		return LSP_ERR_OOM;
	}
	if (json_object_object_add(err->json, "message", val) < 0) {
		return LSP_ERR_OOM;
	}
	return LSP_OK;
}

int
lsp_response_error__set_data(struct LspResponseError *err, json_object *data) {
	if (json_object_object_add(err->json, "data", json_object_get(data)) < 0) {
		return LSP_ERR_OOM;
	}
	return LSP_OK;
}

json_object *
lsp_request_new(const char *method, json_object *params, json_object *id) {
	json_object *req = jsonrpc_new();
	if (req == NULL) {
		return NULL;
	}

	int rv = json_object_object_add(req, "id", json_object_get(id));
	if (rv < 0) {
		goto err;
	}
	rv = json_object_object_add(req, "method", json_object_new_string(method));
	if (rv < 0) {
		goto err;
	}
	if (params != NULL) {
		rv = json_object_object_add(req, "params", params);
		if (rv < 0) {
			goto err;
		}
	}
	return req;
err:
	json_object_put(req);
	return NULL;
}

json_object *
lsp_notification_new(const char *method, json_object *params) {
	json_object *notif = jsonrpc_new();
	if (notif == NULL) {
		return NULL;
	}

	int rv = json_object_object_add(notif, "method", json_object_new_string(method));
	if (rv < 0) {
		goto err;
	}
	if (params != NULL) {
		rv = json_object_object_add(notif, "params", params);
		if (rv < 0) {
			goto err;
		}
	}
	return notif;
err:
	json_object_put(notif);
	return NULL;
}
