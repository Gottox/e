#ifndef LSP_INITIALIZATION_H
#define LSP_INITIALIZATION_H

#include <lsp_util.h>
#include <messages.h>
#include <model.h>

/**
 * Build server capabilities JSON based on registered handlers.
 *
 * Examines the callback structs and builds a capabilities object
 * advertising support for each method that has a non-NULL handler.
 * This is best-effort: capabilities that require additional configuration
 * are left for the user to set manually.
 *
 * @param req_cbs      Request callbacks (may be NULL)
 * @param notif_cbs    Notification callbacks (may be NULL)
 * @return             New JSON object with capabilities (caller must free)
 */
json_object *lsp_build_server_capabilities(
		const struct LspServerRequestCallbacks *req_cbs,
		const struct LspServerNotificationCallbacks *notif_cbs);

/**
 * Build a complete InitializeResult JSON object.
 *
 * Creates the result including auto-detected capabilities from handlers
 * and optional server info. Used internally by lsp_server_process() for
 * auto-handling initialize, but exposed for users who want custom logic.
 *
 * @param server_name    Server name (may be NULL)
 * @param server_version Server version (may be NULL)
 * @param req_cbs        Request callbacks for capability detection (may be NULL)
 * @param notif_cbs      Notification callbacks for capability detection (may be NULL)
 * @return               New JSON object with InitializeResult (caller must free)
 */
json_object *lsp_build_initialize_result(
		const char *server_name, const char *server_version,
		const struct LspServerRequestCallbacks *req_cbs,
		const struct LspServerNotificationCallbacks *notif_cbs);

#endif /* LSP_INITIALIZATION_H */
