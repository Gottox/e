#include <lsp_initialization.h>
#include <stddef.h>
#include <string.h>

/**
 * Mapping from callback field to capability.
 */
struct LspCapabilityMapping {
	size_t offset;
	const char *field; /* JSON capability name like "hoverProvider" */
};

/**
 * Check if a handler is registered at the given offset in a struct.
 */
static bool
is_handler_registered(const void *callbacks, size_t offset) {
	if (!callbacks) {
		return false;
	}
	const void *const *ptr =
			(const void *const *)((const char *)callbacks + offset);
	return *ptr != NULL;
}

#define REQ(cb_field, json_field) \
	{offsetof(struct LspServerRequestCallbacks, cb_field), (json_field)}
#define NOTIF(cb_field, json_field) \
	{offsetof(struct LspServerNotificationCallbacks, cb_field), (json_field)}

/**
 * Request handler to capability mappings.
 */
static const struct LspCapabilityMapping request_capabilities[] = {
	REQ(text_document__hover, "hoverProvider"),
	REQ(text_document__completion, "completionProvider"),
	REQ(text_document__signature_help, "signatureHelpProvider"),
	REQ(text_document__declaration, "declarationProvider"),
	REQ(text_document__definition, "definitionProvider"),
	REQ(text_document__type_definition, "typeDefinitionProvider"),
	REQ(text_document__implementation, "implementationProvider"),
	REQ(text_document__references, "referencesProvider"),
	REQ(text_document__document_highlight, "documentHighlightProvider"),
	REQ(text_document__document_symbol, "documentSymbolProvider"),
	REQ(text_document__code_action, "codeActionProvider"),
	REQ(text_document__code_lens, "codeLensProvider"),
	REQ(text_document__document_link, "documentLinkProvider"),
	REQ(text_document__document_color, "colorProvider"),
	REQ(text_document__formatting, "documentFormattingProvider"),
	REQ(text_document__range_formatting, "documentRangeFormattingProvider"),
	REQ(text_document__on_type_formatting, "documentOnTypeFormattingProvider"),
	REQ(text_document__rename, "renameProvider"),
	REQ(text_document__folding_range, "foldingRangeProvider"),
	REQ(text_document__selection_range, "selectionRangeProvider"),
	REQ(text_document__prepare_call_hierarchy, "callHierarchyProvider"),
	REQ(text_document__linked_editing_range, "linkedEditingRangeProvider"),
	REQ(text_document__semantic_tokens__full, "semanticTokensProvider"),
	REQ(text_document__moniker, "monikerProvider"),
	REQ(text_document__prepare_type_hierarchy, "typeHierarchyProvider"),
	REQ(text_document__inline_value, "inlineValueProvider"),
	REQ(text_document__inlay_hint, "inlayHintProvider"),
	REQ(text_document__diagnostic, "diagnosticProvider"),
	REQ(text_document__inline_completion, "inlineCompletionProvider"),
	REQ(workspace__symbol, "workspaceSymbolProvider"),
	REQ(workspace__execute_command, "executeCommandProvider"),
};

static const size_t request_capabilities_count =
		sizeof(request_capabilities) / sizeof(request_capabilities[0]);

/**
 * Notification handler to textDocumentSync mappings.
 */
static const struct LspCapabilityMapping notification_capabilities[] = {
	NOTIF(text_document__did_open, "openClose"),
	NOTIF(text_document__did_close, "openClose"),
	NOTIF(text_document__did_change, "change"),
	NOTIF(text_document__did_save, "save"),
	NOTIF(text_document__will_save, "willSave"),
};

static const size_t notification_capabilities_count =
		sizeof(notification_capabilities) /
		sizeof(notification_capabilities[0]);

/**
 * Build textDocumentSync capability from notification handlers.
 */
static json_object *
build_text_document_sync(
		const struct LspServerNotificationCallbacks *notif_cbs) {
	json_object *sync = json_object_new_object();
	bool has_any = false;

	for (size_t i = 0; i < notification_capabilities_count; i++) {
		const struct LspCapabilityMapping *m = &notification_capabilities[i];
		if (!is_handler_registered(notif_cbs, m->offset)) {
			continue;
		}
		has_any = true;
		json_object_object_add(sync, m->field,
				json_object_new_boolean(1));
	}

	if (!has_any) {
		json_object_put(sync);
		return NULL;
	}

	return sync;
}

json_object *
lsp_build_server_capabilities(
		const struct LspServerRequestCallbacks *req_cbs,
		const struct LspServerNotificationCallbacks *notif_cbs) {
	json_object *capabilities = json_object_new_object();

	/* Add capabilities for registered request handlers */
	for (size_t i = 0; i < request_capabilities_count; i++) {
		const struct LspCapabilityMapping *m = &request_capabilities[i];
		if (is_handler_registered(req_cbs, m->offset)) {
			json_object_object_add(capabilities, m->field,
					json_object_new_boolean(1));
		}
	}

	/* Build textDocumentSync from notification handlers */
	json_object *sync = build_text_document_sync(notif_cbs);
	if (sync) {
		json_object_object_add(capabilities, "textDocumentSync", sync);
	}

	return capabilities;
}

json_object *
lsp_build_initialize_result(
		const char *server_name, const char *server_version,
		const struct LspServerRequestCallbacks *req_cbs,
		const struct LspServerNotificationCallbacks *notif_cbs) {
	json_object *result = json_object_new_object();

	/* Build and add capabilities */
	json_object *capabilities =
			lsp_build_server_capabilities(req_cbs, notif_cbs);
	json_object_object_add(result, "capabilities", capabilities);

	/* Add server info if provided */
	if (server_name || server_version) {
		json_object *server_info = json_object_new_object();
		if (server_name) {
			json_object_object_add(server_info, "name",
					json_object_new_string(server_name));
		}
		if (server_version) {
			json_object_object_add(server_info, "version",
					json_object_new_string(server_version));
		}
		json_object_object_add(result, "serverInfo", server_info);
	}

	return result;
}
