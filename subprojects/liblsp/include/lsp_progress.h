#ifndef LSP_PROGRESS_H
#define LSP_PROGRESS_H

#include <lsp.h>
#include <model.h>

/**
 * Progress value kinds for $/progress notifications.
 */
enum LspProgressValueKind {
	LSP_PROGRESS_VALUE_UNKNOWN = 0,
	LSP_PROGRESS_VALUE_BEGIN,
	LSP_PROGRESS_VALUE_REPORT,
	LSP_PROGRESS_VALUE_END
};

/**
 * Determine the kind of progress value from ProgressParams.
 *
 * Examines the "kind" field in the value to determine whether this is
 * a begin, report, or end progress notification.
 *
 * @param params  The ProgressParams from a $/progress notification
 * @return        The progress value kind
 */
enum LspProgressValueKind lsp_progress_value_kind(
		const struct LspProgressParams *params);

/**
 * Extract WorkDoneProgressBegin from ProgressParams.
 *
 * @param params  The ProgressParams from a $/progress notification
 * @param begin   Output: the begin progress data
 * @return        LSP_OK on success, error code on failure
 */
LSP_NO_UNUSED int lsp_progress_value_as_begin(
		const struct LspProgressParams *params,
		struct LspWorkDoneProgressBegin *begin);

/**
 * Extract WorkDoneProgressReport from ProgressParams.
 *
 * @param params  The ProgressParams from a $/progress notification
 * @param report  Output: the report progress data
 * @return        LSP_OK on success, error code on failure
 */
LSP_NO_UNUSED int lsp_progress_value_as_report(
		const struct LspProgressParams *params,
		struct LspWorkDoneProgressReport *report);

/**
 * Extract WorkDoneProgressEnd from ProgressParams.
 *
 * @param params  The ProgressParams from a $/progress notification
 * @param end     Output: the end progress data
 * @return        LSP_OK on success, error code on failure
 */
LSP_NO_UNUSED int lsp_progress_value_as_end(
		const struct LspProgressParams *params,
		struct LspWorkDoneProgressEnd *end);

/**
 * Send a WorkDoneProgressBegin notification.
 *
 * @param lsp         The LSP connection
 * @param token       The progress token (string or integer)
 * @param title       The title of the operation (required)
 * @param message     Optional message (may be NULL)
 * @param percentage  Optional percentage (0-100, or -1 to omit)
 * @param cancellable Whether the operation is cancellable
 * @return            LSP_OK on success, error code on failure
 */
LSP_NO_UNUSED int lsp_progress_begin_send(
		struct Lsp *lsp,
		const struct LspProgressToken *token,
		const char *title,
		const char *message,
		int percentage,
		bool cancellable);

/**
 * Send a WorkDoneProgressReport notification.
 *
 * @param lsp         The LSP connection
 * @param token       The progress token
 * @param message     Optional message (may be NULL)
 * @param percentage  Optional percentage (0-100, or -1 to omit)
 * @param cancellable Whether the operation is cancellable
 * @return            LSP_OK on success, error code on failure
 */
LSP_NO_UNUSED int lsp_progress_report_send(
		struct Lsp *lsp,
		const struct LspProgressToken *token,
		const char *message,
		int percentage,
		bool cancellable);

/**
 * Send a WorkDoneProgressEnd notification.
 *
 * @param lsp      The LSP connection
 * @param token    The progress token
 * @param message  Optional final message (may be NULL)
 * @return         LSP_OK on success, error code on failure
 */
LSP_NO_UNUSED int lsp_progress_end_send(
		struct Lsp *lsp,
		const struct LspProgressToken *token,
		const char *message);

/**
 * Create a progress token from a string.
 *
 * @param token  Output: the progress token
 * @param str    The string value
 * @return       LSP_OK on success, error code on failure
 */
LSP_NO_UNUSED int lsp_progress_token_from_string(
		struct LspProgressToken *token,
		const char *str);

/**
 * Create a progress token from an integer.
 *
 * @param token  Output: the progress token
 * @param value  The integer value
 * @return       LSP_OK on success, error code on failure
 */
LSP_NO_UNUSED int lsp_progress_token_from_integer(
		struct LspProgressToken *token,
		int64_t value);

#endif /* LSP_PROGRESS_H */
