#ifndef LSP_UTIL_H
#define LSP_UTIL_H

#include "model.h"
#include "messages.h"

#ifndef LSP_NO_UNUSED
#define LSP_NO_UNUSED __attribute__((warn_unused_result))
#endif

// Message classification
enum LspMessageKind {
	LSP_MESSAGE_REQUEST,      // Has "method" and "id"
	LSP_MESSAGE_NOTIFICATION, // Has "method", no "id"
	LSP_MESSAGE_RESPONSE,     // Has "id", no "method" (has "result" or "error")
	LSP_MESSAGE_INVALID
};

// Classify an incoming JSON-RPC message
enum LspMessageKind lsp_message_classify(json_object *msg);

// Create a success response
// id is taken by reference (caller retains ownership)
json_object *lsp_response_new(json_object *id, json_object *result);

// Create an error response
// id is taken by reference (caller retains ownership)
json_object *lsp_response_error_new(json_object *id, int code, const char *message);

// Set JSON-RPC "Invalid params" error (-32602) on response
void lsp_response_set_invalid_params(json_object *response);

// Set error response from handler result
// If error was populated by handler, uses it directly; otherwise creates generic error
// Takes ownership of error->json
LSP_NO_UNUSED int lsp_response_set_handler_error(json_object *response,
		struct LspResponseError *error, int err_code);

// Set successful result on response and cleanup error
// Combines error cleanup (not needed on success) with result assignment
void lsp_response_set_result(json_object *response,
		struct LspResponseError *error, json_object *result);

// Assign JSON result to an or-type struct
#define LSP_OR_RESULT_FROM_RESP(result, resp) \
	((result)->json = json_object_object_get((resp), "result"), \
	 (result)->json != NULL ? LSP_OK : LSP_ERR_MISSING_FIELD)

// JSON-RPC ResponseError type
struct LspResponseError {
	json_object *json;
};

// Initialize empty error
LSP_NO_UNUSED int lsp_response_error__init(struct LspResponseError *err);

// Cleanup (releases json reference)
void lsp_response_error__cleanup(struct LspResponseError *err);

// Getters
int64_t lsp_response_error__code(const struct LspResponseError *err);
const char *lsp_response_error__message(const struct LspResponseError *err);
json_object *lsp_response_error__data(const struct LspResponseError *err);

// Setters
LSP_NO_UNUSED int lsp_response_error__set_code(struct LspResponseError *err, int64_t code);
LSP_NO_UNUSED int lsp_response_error__set_message(struct LspResponseError *err, const char *message);
LSP_NO_UNUSED int lsp_response_error__set_data(struct LspResponseError *err, json_object *data);

// Create a new JSON-RPC request object
// params can be NULL for requests without parameters
// id is taken by reference (caller retains ownership)
json_object *lsp_request_new(const char *method, json_object *params, json_object *id);

// Create a new JSON-RPC notification object
// params can be NULL for notifications without parameters
json_object *lsp_notification_new(const char *method, json_object *params);

#endif // LSP_UTIL_H
