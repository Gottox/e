////////////////////////////
////////////////////////////
// Structures

////////////////////////////
// TextDocumentPositionParams 156

struct LspTextDocumentPositionParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspTextDocumentIdentifier text_document;
	struct LspPosition position;
};

////////////////////////////
// WorkDoneProgressParams 157

struct LspWorkDoneProgressParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspProgressToken work_done_token;
};

////////////////////////////
// PartialResultParams 158

struct LspPartialResultParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspProgressToken partial_result_token;
};

////////////////////////////
// ImplementationParams 0

struct LspImplementationParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
};

////////////////////////////
// Location 1

struct LspLocation {
	char info;
	// Extends
	// Mixins
	// Properties
	char *uri;
	struct LspRange range;
};

////////////////////////////
// TextDocumentRegistrationOptions 14

struct LspTextDocumentRegistrationOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	document_selector;
};

////////////////////////////
// WorkDoneProgressOptions 13

struct LspWorkDoneProgressOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	bool work_done_progress;
};

////////////////////////////
// ImplementationOptions 161

struct LspImplementationOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// StaticRegistrationOptions 162

struct LspStaticRegistrationOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	char *id;
};

////////////////////////////
// ImplementationRegistrationOptions 2

struct LspImplementationRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspImplementationOptions implementation_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// TypeDefinitionParams 3

struct LspTypeDefinitionParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
};

////////////////////////////
// TypeDefinitionOptions 163

struct LspTypeDefinitionOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// TypeDefinitionRegistrationOptions 4

struct LspTypeDefinitionRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspTypeDefinitionOptions type_definition_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// WorkspaceFolder 5

struct LspWorkspaceFolder {
	char info;
	// Extends
	// Mixins
	// Properties
	uri;
	char *name;
};

////////////////////////////
// DidChangeWorkspaceFoldersParams 6

struct LspDidChangeWorkspaceFoldersParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspWorkspaceFoldersChangeEvent event;
};

////////////////////////////
// ConfigurationParams 7

struct LspConfigurationParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspConfigurationItem *items;
	} items;
};

////////////////////////////
// DocumentColorParams 8

struct LspDocumentColorParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
};

////////////////////////////
// ColorInformation 9

struct LspColorInformation {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspRange range;
	struct LspColor color;
};

////////////////////////////
// DocumentColorOptions 168

struct LspDocumentColorOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// DocumentColorRegistrationOptions 10

struct LspDocumentColorRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspDocumentColorOptions document_color_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// ColorPresentationParams 11

struct LspColorPresentationParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
	struct LspColor color;
	struct LspRange range;
};

////////////////////////////
// ColorPresentation 12

struct LspColorPresentation {
	char info;
	// Extends
	// Mixins
	// Properties
	char *label;
	struct LspTextEdit text_edit;
	struct {
		size_t count, struct LspTextEdit *items;
	} additional_text_edits;
};

////////////////////////////
// FoldingRangeParams 15

struct LspFoldingRangeParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
};

////////////////////////////
// FoldingRange 16

struct LspFoldingRange {
	char info;
	// Extends
	// Mixins
	// Properties
	start_line;
	start_character;
	end_line;
	end_character;
	struct LspFoldingRangeKind kind;
	char *collapsed_text;
};

////////////////////////////
// FoldingRangeOptions 169

struct LspFoldingRangeOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// FoldingRangeRegistrationOptions 17

struct LspFoldingRangeRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspFoldingRangeOptions folding_range_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// DeclarationParams 18

struct LspDeclarationParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
};

////////////////////////////
// DeclarationOptions 170

struct LspDeclarationOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// DeclarationRegistrationOptions 19

struct LspDeclarationRegistrationOptions {
	char info;
	// Extends
	struct LspDeclarationOptions declaration_options;
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// SelectionRangeParams 20

struct LspSelectionRangeParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
	struct {
		size_t count, struct LspPosition *items;
	} positions;
};

////////////////////////////
// SelectionRange 21

struct LspSelectionRange {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspRange range;
	struct LspSelectionRange parent;
};

////////////////////////////
// SelectionRangeOptions 172

struct LspSelectionRangeOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// SelectionRangeRegistrationOptions 22

struct LspSelectionRangeRegistrationOptions {
	char info;
	// Extends
	struct LspSelectionRangeOptions selection_range_options;
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// WorkDoneProgressCreateParams 23

struct LspWorkDoneProgressCreateParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspProgressToken token;
};

////////////////////////////
// WorkDoneProgressCancelParams 24

struct LspWorkDoneProgressCancelParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspProgressToken token;
};

////////////////////////////
// CallHierarchyPrepareParams 25

struct LspCallHierarchyPrepareParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
};

////////////////////////////
// CallHierarchyItem 26

struct LspCallHierarchyItem {
	char info;
	// Extends
	// Mixins
	// Properties
	char *name;
	struct LspSymbolKind kind;
	struct {
		size_t count, struct LspSymbolTag *items;
	} tags;
	char *detail;
	char *uri;
	struct LspRange range;
	struct LspRange selection_range;
	struct LspLSPAny data;
};

////////////////////////////
// CallHierarchyOptions 173

struct LspCallHierarchyOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// CallHierarchyRegistrationOptions 27

struct LspCallHierarchyRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspCallHierarchyOptions call_hierarchy_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// CallHierarchyIncomingCallsParams 28

struct LspCallHierarchyIncomingCallsParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspCallHierarchyItem item;
};

////////////////////////////
// CallHierarchyIncomingCall 29

struct LspCallHierarchyIncomingCall {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspCallHierarchyItem from;
	struct {
		size_t count, struct LspRange *items;
	} from_ranges;
};

////////////////////////////
// CallHierarchyOutgoingCallsParams 30

struct LspCallHierarchyOutgoingCallsParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspCallHierarchyItem item;
};

////////////////////////////
// CallHierarchyOutgoingCall 31

struct LspCallHierarchyOutgoingCall {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspCallHierarchyItem to;
	struct {
		size_t count, struct LspRange *items;
	} from_ranges;
};

////////////////////////////
// SemanticTokensParams 32

struct LspSemanticTokensParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
};

////////////////////////////
// SemanticTokens 33

struct LspSemanticTokens {
	char info;
	// Extends
	// Mixins
	// Properties
	char *result_id;
	struct {
		size_t count, *items;
	} data;
};

////////////////////////////
// SemanticTokensPartialResult 34

struct LspSemanticTokensPartialResult {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, *items;
	} data;
};

////////////////////////////
// SemanticTokensOptions 174

struct LspSemanticTokensOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
	struct LspSemanticTokensLegend legend;
	range;
	full;
};

////////////////////////////
// SemanticTokensRegistrationOptions 35

struct LspSemanticTokensRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspSemanticTokensOptions semantic_tokens_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// SemanticTokensDeltaParams 36

struct LspSemanticTokensDeltaParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
	char *previous_result_id;
};

////////////////////////////
// SemanticTokensDelta 37

struct LspSemanticTokensDelta {
	char info;
	// Extends
	// Mixins
	// Properties
	char *result_id;
	struct {
		size_t count, struct LspSemanticTokensEdit *items;
	} edits;
};

////////////////////////////
// SemanticTokensDeltaPartialResult 38

struct LspSemanticTokensDeltaPartialResult {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspSemanticTokensEdit *items;
	} edits;
};

////////////////////////////
// SemanticTokensRangeParams 39

struct LspSemanticTokensRangeParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
	struct LspRange range;
};

////////////////////////////
// ShowDocumentParams 40

struct LspShowDocumentParams {
	char info;
	// Extends
	// Mixins
	// Properties
	uri;
	bool external;
	bool take_focus;
	struct LspRange selection;
};

////////////////////////////
// ShowDocumentResult 41

struct LspShowDocumentResult {
	char info;
	// Extends
	// Mixins
	// Properties
	bool success;
};

////////////////////////////
// LinkedEditingRangeParams 42

struct LspLinkedEditingRangeParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
};

////////////////////////////
// LinkedEditingRanges 43

struct LspLinkedEditingRanges {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspRange *items;
	} ranges;
	char *word_pattern;
};

////////////////////////////
// LinkedEditingRangeOptions 176

struct LspLinkedEditingRangeOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// LinkedEditingRangeRegistrationOptions 44

struct LspLinkedEditingRangeRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspLinkedEditingRangeOptions linked_editing_range_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// CreateFilesParams 45

struct LspCreateFilesParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspFileCreate *items;
	} files;
};

////////////////////////////
// WorkspaceEdit 46

struct LspWorkspaceEdit {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, char **keys;
		struct {
			size_t count, struct LspTextEdit *items;
		} *items;
	} changes;
	struct {
		size_t count, *items;
	} document_changes;
	struct {
		size_t count, struct LspChangeAnnotationIdentifier *keys;
		struct LspChangeAnnotation *items;
	} change_annotations;
};

////////////////////////////
// FileOperationRegistrationOptions 47

struct LspFileOperationRegistrationOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspFileOperationFilter *items;
	} filters;
};

////////////////////////////
// RenameFilesParams 48

struct LspRenameFilesParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspFileRename *items;
	} files;
};

////////////////////////////
// DeleteFilesParams 49

struct LspDeleteFilesParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspFileDelete *items;
	} files;
};

////////////////////////////
// MonikerParams 50

struct LspMonikerParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
};

////////////////////////////
// Moniker 51

struct LspMoniker {
	char info;
	// Extends
	// Mixins
	// Properties
	char *scheme;
	char *identifier;
	struct LspUniquenessLevel unique;
	struct LspMonikerKind kind;
};

////////////////////////////
// MonikerOptions 186

struct LspMonikerOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// MonikerRegistrationOptions 52

struct LspMonikerRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspMonikerOptions moniker_options;
	// Mixins
	// Properties
};

////////////////////////////
// TypeHierarchyPrepareParams 53

struct LspTypeHierarchyPrepareParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
};

////////////////////////////
// TypeHierarchyItem 54

struct LspTypeHierarchyItem {
	char info;
	// Extends
	// Mixins
	// Properties
	char *name;
	struct LspSymbolKind kind;
	struct {
		size_t count, struct LspSymbolTag *items;
	} tags;
	char *detail;
	char *uri;
	struct LspRange range;
	struct LspRange selection_range;
	struct LspLSPAny data;
};

////////////////////////////
// TypeHierarchyOptions 187

struct LspTypeHierarchyOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// TypeHierarchyRegistrationOptions 55

struct LspTypeHierarchyRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspTypeHierarchyOptions type_hierarchy_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// TypeHierarchySupertypesParams 56

struct LspTypeHierarchySupertypesParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTypeHierarchyItem item;
};

////////////////////////////
// TypeHierarchySubtypesParams 57

struct LspTypeHierarchySubtypesParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTypeHierarchyItem item;
};

////////////////////////////
// InlineValueParams 58

struct LspInlineValueParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
	struct LspRange range;
	struct LspInlineValueContext context;
};

////////////////////////////
// InlineValueOptions 192

struct LspInlineValueOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// InlineValueRegistrationOptions 59

struct LspInlineValueRegistrationOptions {
	char info;
	// Extends
	struct LspInlineValueOptions inline_value_options;
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// InlayHintParams 60

struct LspInlayHintParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
	struct LspRange range;
};

////////////////////////////
// InlayHint 61

struct LspInlayHint {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspPosition position;
	label;
	struct LspInlayHintKind kind;
	struct {
		size_t count, struct LspTextEdit *items;
	} text_edits;
	tooltip;
	bool padding_left;
	bool padding_right;
	struct LspLSPAny data;
};

////////////////////////////
// InlayHintOptions 195

struct LspInlayHintOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
	bool resolve_provider;
};

////////////////////////////
// InlayHintRegistrationOptions 62

struct LspInlayHintRegistrationOptions {
	char info;
	// Extends
	struct LspInlayHintOptions inlay_hint_options;
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// DocumentDiagnosticParams 63

struct LspDocumentDiagnosticParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
	char *identifier;
	char *previous_result_id;
};

////////////////////////////
// DocumentDiagnosticReportPartialResult 64

struct LspDocumentDiagnosticReportPartialResult {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, char **keys;
		*items;
	} related_documents;
};

////////////////////////////
// DiagnosticServerCancellationData 65

struct LspDiagnosticServerCancellationData {
	char info;
	// Extends
	// Mixins
	// Properties
	bool retrigger_request;
};

////////////////////////////
// DiagnosticOptions 200

struct LspDiagnosticOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
	char *identifier;
	bool inter_file_dependencies;
	bool workspace_diagnostics;
};

////////////////////////////
// DiagnosticRegistrationOptions 66

struct LspDiagnosticRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspDiagnosticOptions diagnostic_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// WorkspaceDiagnosticParams 67

struct LspWorkspaceDiagnosticParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	char *identifier;
	struct {
		size_t count, struct LspPreviousResultId *items;
	} previous_result_ids;
};

////////////////////////////
// WorkspaceDiagnosticReport 68

struct LspWorkspaceDiagnosticReport {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspWorkspaceDocumentDiagnosticReport *items;
	} items;
};

////////////////////////////
// WorkspaceDiagnosticReportPartialResult 69

struct LspWorkspaceDiagnosticReportPartialResult {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspWorkspaceDocumentDiagnosticReport *items;
	} items;
};

////////////////////////////
// DidOpenNotebookDocumentParams 70

struct LspDidOpenNotebookDocumentParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspNotebookDocument notebook_document;
	struct {
		size_t count, struct LspTextDocumentItem *items;
	} cell_text_documents;
};

////////////////////////////
// DidChangeNotebookDocumentParams 71

struct LspDidChangeNotebookDocumentParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspVersionedNotebookDocumentIdentifier notebook_document;
	struct LspNotebookDocumentChangeEvent change;
};

////////////////////////////
// DidSaveNotebookDocumentParams 72

struct LspDidSaveNotebookDocumentParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspNotebookDocumentIdentifier notebook_document;
};

////////////////////////////
// DidCloseNotebookDocumentParams 73

struct LspDidCloseNotebookDocumentParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspNotebookDocumentIdentifier notebook_document;
	struct {
		size_t count, struct LspTextDocumentIdentifier *items;
	} cell_text_documents;
};

////////////////////////////
// InlineCompletionParams 74

struct LspInlineCompletionParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
	struct LspInlineCompletionContext context;
};

////////////////////////////
// InlineCompletionList 75

struct LspInlineCompletionList {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspInlineCompletionItem *items;
	} items;
};

////////////////////////////
// InlineCompletionItem 76

struct LspInlineCompletionItem {
	char info;
	// Extends
	// Mixins
	// Properties
	insert_text;
	char *filter_text;
	struct LspRange range;
	struct LspCommand command;
};

////////////////////////////
// InlineCompletionOptions 209

struct LspInlineCompletionOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// InlineCompletionRegistrationOptions 77

struct LspInlineCompletionRegistrationOptions {
	char info;
	// Extends
	struct LspInlineCompletionOptions inline_completion_options;
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// RegistrationParams 78

struct LspRegistrationParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspRegistration *items;
	} registrations;
};

////////////////////////////
// UnregistrationParams 79

struct LspUnregistrationParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspUnregistration *items;
	} unregisterations;
};

////////////////////////////
// _InitializeParams 212

struct Lsp_InitializeParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
	process_id;
	client_info;
	char *locale;
	root_path;
	root_uri;
	struct LspClientCapabilities capabilities;
	struct LspLSPAny initialization_options;
	struct LspTraceValues trace;
};

////////////////////////////
// WorkspaceFoldersInitializeParams 213

struct LspWorkspaceFoldersInitializeParams {
	char info;
	// Extends
	// Mixins
	// Properties
	workspace_folders;
};

////////////////////////////
// InitializeParams 80

struct LspInitializeParams {
	char info;
	// Extends
	struct Lsp_InitializeParams __initialize_params;
	struct LspWorkspaceFoldersInitializeParams
			workspace_folders_initialize_params;
	// Mixins
	// Properties
};

////////////////////////////
// InitializeResult 81

struct LspInitializeResult {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspServerCapabilities capabilities;
	server_info;
};

////////////////////////////
// InitializeError 82

struct LspInitializeError {
	char info;
	// Extends
	// Mixins
	// Properties
	bool retry;
};

////////////////////////////
// InitializedParams 83

struct LspInitializedParams {
	char info;
	// Extends
	// Mixins
	// Properties
};

////////////////////////////
// DidChangeConfigurationParams 84

struct LspDidChangeConfigurationParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspLSPAny settings;
};

////////////////////////////
// DidChangeConfigurationRegistrationOptions 85

struct LspDidChangeConfigurationRegistrationOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	section;
};

////////////////////////////
// ShowMessageParams 86

struct LspShowMessageParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspMessageType type;
	char *message;
};

////////////////////////////
// ShowMessageRequestParams 87

struct LspShowMessageRequestParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspMessageType type;
	char *message;
	struct {
		size_t count, struct LspMessageActionItem *items;
	} actions;
};

////////////////////////////
// MessageActionItem 88

struct LspMessageActionItem {
	char info;
	// Extends
	// Mixins
	// Properties
	char *title;
};

////////////////////////////
// LogMessageParams 89

struct LspLogMessageParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspMessageType type;
	char *message;
};

////////////////////////////
// DidOpenTextDocumentParams 90

struct LspDidOpenTextDocumentParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspTextDocumentItem text_document;
};

////////////////////////////
// DidChangeTextDocumentParams 91

struct LspDidChangeTextDocumentParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspVersionedTextDocumentIdentifier text_document;
	struct {
		size_t count, struct LspTextDocumentContentChangeEvent *items;
	} content_changes;
};

////////////////////////////
// TextDocumentChangeRegistrationOptions 92

struct LspTextDocumentChangeRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	// Mixins
	// Properties
	struct LspTextDocumentSyncKind sync_kind;
};

////////////////////////////
// DidCloseTextDocumentParams 93

struct LspDidCloseTextDocumentParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspTextDocumentIdentifier text_document;
};

////////////////////////////
// DidSaveTextDocumentParams 94

struct LspDidSaveTextDocumentParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspTextDocumentIdentifier text_document;
	char *text;
};

////////////////////////////
// SaveOptions 216

struct LspSaveOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	bool include_text;
};

////////////////////////////
// TextDocumentSaveRegistrationOptions 95

struct LspTextDocumentSaveRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspSaveOptions save_options;
	// Mixins
	// Properties
};

////////////////////////////
// WillSaveTextDocumentParams 96

struct LspWillSaveTextDocumentParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspTextDocumentIdentifier text_document;
	struct LspTextDocumentSaveReason reason;
};

////////////////////////////
// TextEdit 97

struct LspTextEdit {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspRange range;
	char *new_text;
};

////////////////////////////
// DidChangeWatchedFilesParams 98

struct LspDidChangeWatchedFilesParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspFileEvent *items;
	} changes;
};

////////////////////////////
// DidChangeWatchedFilesRegistrationOptions 99

struct LspDidChangeWatchedFilesRegistrationOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspFileSystemWatcher *items;
	} watchers;
};

////////////////////////////
// PublishDiagnosticsParams 100

struct LspPublishDiagnosticsParams {
	char info;
	// Extends
	// Mixins
	// Properties
	char *uri;
	int version;
	struct {
		size_t count, struct LspDiagnostic *items;
	} diagnostics;
};

////////////////////////////
// CompletionParams 101

struct LspCompletionParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspCompletionContext context;
};

////////////////////////////
// CompletionItem 102

struct LspCompletionItem {
	char info;
	// Extends
	// Mixins
	// Properties
	char *label;
	struct LspCompletionItemLabelDetails label_details;
	struct LspCompletionItemKind kind;
	struct {
		size_t count, struct LspCompletionItemTag *items;
	} tags;
	char *detail;
	documentation;
	bool deprecated;
	bool preselect;
	char *sort_text;
	char *filter_text;
	char *insert_text;
	struct LspInsertTextFormat insert_text_format;
	struct LspInsertTextMode insert_text_mode;
	text_edit;
	char *text_edit_text;
	struct {
		size_t count, struct LspTextEdit *items;
	} additional_text_edits;
	struct {
		size_t count, char **items;
	} commit_characters;
	struct LspCommand command;
	struct LspLSPAny data;
};

////////////////////////////
// CompletionList 103

struct LspCompletionList {
	char info;
	// Extends
	// Mixins
	// Properties
	bool is_incomplete;
	item_defaults;
	struct {
		size_t count, struct LspCompletionItem *items;
	} items;
};

////////////////////////////
// CompletionOptions 223

struct LspCompletionOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
	struct {
		size_t count, char **items;
	} trigger_characters;
	struct {
		size_t count, char **items;
	} all_commit_characters;
	bool resolve_provider;
	completion_item;
};

////////////////////////////
// CompletionRegistrationOptions 104

struct LspCompletionRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspCompletionOptions completion_options;
	// Mixins
	// Properties
};

////////////////////////////
// HoverParams 105

struct LspHoverParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
};

////////////////////////////
// Hover 106

struct LspHover {
	char info;
	// Extends
	// Mixins
	// Properties
	contents;
	struct LspRange range;
};

////////////////////////////
// HoverOptions 224

struct LspHoverOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// HoverRegistrationOptions 107

struct LspHoverRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspHoverOptions hover_options;
	// Mixins
	// Properties
};

////////////////////////////
// SignatureHelpParams 108

struct LspSignatureHelpParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
	struct LspSignatureHelpContext context;
};

////////////////////////////
// SignatureHelp 109

struct LspSignatureHelp {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspSignatureInformation *items;
	} signatures;
	active_signature;
	active_parameter;
};

////////////////////////////
// SignatureHelpOptions 227

struct LspSignatureHelpOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
	struct {
		size_t count, char **items;
	} trigger_characters;
	struct {
		size_t count, char **items;
	} retrigger_characters;
};

////////////////////////////
// SignatureHelpRegistrationOptions 110

struct LspSignatureHelpRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspSignatureHelpOptions signature_help_options;
	// Mixins
	// Properties
};

////////////////////////////
// DefinitionParams 111

struct LspDefinitionParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
};

////////////////////////////
// DefinitionOptions 228

struct LspDefinitionOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// DefinitionRegistrationOptions 112

struct LspDefinitionRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspDefinitionOptions definition_options;
	// Mixins
	// Properties
};

////////////////////////////
// ReferenceParams 113

struct LspReferenceParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspReferenceContext context;
};

////////////////////////////
// ReferenceOptions 230

struct LspReferenceOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// ReferenceRegistrationOptions 114

struct LspReferenceRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspReferenceOptions reference_options;
	// Mixins
	// Properties
};

////////////////////////////
// DocumentHighlightParams 115

struct LspDocumentHighlightParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
};

////////////////////////////
// DocumentHighlight 116

struct LspDocumentHighlight {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspRange range;
	struct LspDocumentHighlightKind kind;
};

////////////////////////////
// DocumentHighlightOptions 231

struct LspDocumentHighlightOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// DocumentHighlightRegistrationOptions 117

struct LspDocumentHighlightRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspDocumentHighlightOptions document_highlight_options;
	// Mixins
	// Properties
};

////////////////////////////
// DocumentSymbolParams 118

struct LspDocumentSymbolParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
};

////////////////////////////
// BaseSymbolInformation 232

struct LspBaseSymbolInformation {
	char info;
	// Extends
	// Mixins
	// Properties
	char *name;
	struct LspSymbolKind kind;
	struct {
		size_t count, struct LspSymbolTag *items;
	} tags;
	char *container_name;
};

////////////////////////////
// SymbolInformation 119

struct LspSymbolInformation {
	char info;
	// Extends
	struct LspBaseSymbolInformation base_symbol_information;
	// Mixins
	// Properties
	bool deprecated;
	struct LspLocation location;
};

////////////////////////////
// DocumentSymbol 120

struct LspDocumentSymbol {
	char info;
	// Extends
	// Mixins
	// Properties
	char *name;
	char *detail;
	struct LspSymbolKind kind;
	struct {
		size_t count, struct LspSymbolTag *items;
	} tags;
	bool deprecated;
	struct LspRange range;
	struct LspRange selection_range;
	struct {
		size_t count, struct LspDocumentSymbol *items;
	} children;
};

////////////////////////////
// DocumentSymbolOptions 233

struct LspDocumentSymbolOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
	char *label;
};

////////////////////////////
// DocumentSymbolRegistrationOptions 121

struct LspDocumentSymbolRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspDocumentSymbolOptions document_symbol_options;
	// Mixins
	// Properties
};

////////////////////////////
// CodeActionParams 122

struct LspCodeActionParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
	struct LspRange range;
	struct LspCodeActionContext context;
};

////////////////////////////
// Command 123

struct LspCommand {
	char info;
	// Extends
	// Mixins
	// Properties
	char *title;
	char *command;
	struct {
		size_t count, struct LspLSPAny *items;
	} arguments;
};

////////////////////////////
// CodeAction 124

struct LspCodeAction {
	char info;
	// Extends
	// Mixins
	// Properties
	char *title;
	struct LspCodeActionKind kind;
	struct {
		size_t count, struct LspDiagnostic *items;
	} diagnostics;
	bool is_preferred;
	disabled;
	struct LspWorkspaceEdit edit;
	struct LspCommand command;
	struct LspLSPAny data;
};

////////////////////////////
// CodeActionOptions 235

struct LspCodeActionOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
	struct {
		size_t count, struct LspCodeActionKind *items;
	} code_action_kinds;
	bool resolve_provider;
};

////////////////////////////
// CodeActionRegistrationOptions 125

struct LspCodeActionRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspCodeActionOptions code_action_options;
	// Mixins
	// Properties
};

////////////////////////////
// WorkspaceSymbolParams 126

struct LspWorkspaceSymbolParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	char *query;
};

////////////////////////////
// WorkspaceSymbol 127

struct LspWorkspaceSymbol {
	char info;
	// Extends
	struct LspBaseSymbolInformation base_symbol_information;
	// Mixins
	// Properties
	location;
	struct LspLSPAny data;
};

////////////////////////////
// WorkspaceSymbolOptions 236

struct LspWorkspaceSymbolOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
	bool resolve_provider;
};

////////////////////////////
// WorkspaceSymbolRegistrationOptions 128

struct LspWorkspaceSymbolRegistrationOptions {
	char info;
	// Extends
	struct LspWorkspaceSymbolOptions workspace_symbol_options;
	// Mixins
	// Properties
};

////////////////////////////
// CodeLensParams 129

struct LspCodeLensParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
};

////////////////////////////
// CodeLens 130

struct LspCodeLens {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspRange range;
	struct LspCommand command;
	struct LspLSPAny data;
};

////////////////////////////
// CodeLensOptions 237

struct LspCodeLensOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
	bool resolve_provider;
};

////////////////////////////
// CodeLensRegistrationOptions 131

struct LspCodeLensRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspCodeLensOptions code_lens_options;
	// Mixins
	// Properties
};

////////////////////////////
// DocumentLinkParams 132

struct LspDocumentLinkParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	struct LspPartialResultParams partial_result_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
};

////////////////////////////
// DocumentLink 133

struct LspDocumentLink {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspRange range;
	target;
	char *tooltip;
	struct LspLSPAny data;
};

////////////////////////////
// DocumentLinkOptions 238

struct LspDocumentLinkOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
	bool resolve_provider;
};

////////////////////////////
// DocumentLinkRegistrationOptions 134

struct LspDocumentLinkRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspDocumentLinkOptions document_link_options;
	// Mixins
	// Properties
};

////////////////////////////
// DocumentFormattingParams 135

struct LspDocumentFormattingParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
	struct LspFormattingOptions options;
};

////////////////////////////
// DocumentFormattingOptions 240

struct LspDocumentFormattingOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
};

////////////////////////////
// DocumentFormattingRegistrationOptions 136

struct LspDocumentFormattingRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspDocumentFormattingOptions document_formatting_options;
	// Mixins
	// Properties
};

////////////////////////////
// DocumentRangeFormattingParams 137

struct LspDocumentRangeFormattingParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
	struct LspRange range;
	struct LspFormattingOptions options;
};

////////////////////////////
// DocumentRangeFormattingOptions 241

struct LspDocumentRangeFormattingOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
	bool ranges_support;
};

////////////////////////////
// DocumentRangeFormattingRegistrationOptions 138

struct LspDocumentRangeFormattingRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspDocumentRangeFormattingOptions document_range_formatting_options;
	// Mixins
	// Properties
};

////////////////////////////
// DocumentRangesFormattingParams 139

struct LspDocumentRangesFormattingParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
	struct {
		size_t count, struct LspRange *items;
	} ranges;
	struct LspFormattingOptions options;
};

////////////////////////////
// DocumentOnTypeFormattingParams 140

struct LspDocumentOnTypeFormattingParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspTextDocumentIdentifier text_document;
	struct LspPosition position;
	char *ch;
	struct LspFormattingOptions options;
};

////////////////////////////
// DocumentOnTypeFormattingOptions 242

struct LspDocumentOnTypeFormattingOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	char *first_trigger_character;
	struct {
		size_t count, char **items;
	} more_trigger_character;
};

////////////////////////////
// DocumentOnTypeFormattingRegistrationOptions 141

struct LspDocumentOnTypeFormattingRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspDocumentOnTypeFormattingOptions
			document_on_type_formatting_options;
	// Mixins
	// Properties
};

////////////////////////////
// RenameParams 142

struct LspRenameParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
	struct LspTextDocumentIdentifier text_document;
	struct LspPosition position;
	char *new_name;
};

////////////////////////////
// RenameOptions 243

struct LspRenameOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
	bool prepare_provider;
};

////////////////////////////
// RenameRegistrationOptions 143

struct LspRenameRegistrationOptions {
	char info;
	// Extends
	struct LspTextDocumentRegistrationOptions
			text_document_registration_options;
	struct LspRenameOptions rename_options;
	// Mixins
	// Properties
};

////////////////////////////
// PrepareRenameParams 144

struct LspPrepareRenameParams {
	char info;
	// Extends
	struct LspTextDocumentPositionParams text_document_position_params;
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
};

////////////////////////////
// ExecuteCommandParams 145

struct LspExecuteCommandParams {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressParams work_done_progress_params;
	// Properties
	char *command;
	struct {
		size_t count, struct LspLSPAny *items;
	} arguments;
};

////////////////////////////
// ExecuteCommandOptions 244

struct LspExecuteCommandOptions {
	char info;
	// Extends
	// Mixins
	struct LspWorkDoneProgressOptions work_done_progress_options;
	// Properties
	struct {
		size_t count, char **items;
	} commands;
};

////////////////////////////
// ExecuteCommandRegistrationOptions 146

struct LspExecuteCommandRegistrationOptions {
	char info;
	// Extends
	struct LspExecuteCommandOptions execute_command_options;
	// Mixins
	// Properties
};

////////////////////////////
// ApplyWorkspaceEditParams 147

struct LspApplyWorkspaceEditParams {
	char info;
	// Extends
	// Mixins
	// Properties
	char *label;
	struct LspWorkspaceEdit edit;
};

////////////////////////////
// ApplyWorkspaceEditResult 148

struct LspApplyWorkspaceEditResult {
	char info;
	// Extends
	// Mixins
	// Properties
	bool applied;
	char *failure_reason;
	failed_change;
};

////////////////////////////
// WorkDoneProgressBegin 149

struct LspWorkDoneProgressBegin {
	char info;
	// Extends
	// Mixins
	// Properties
	kind;
	char *title;
	bool cancellable;
	char *message;
	percentage;
};

////////////////////////////
// WorkDoneProgressReport 150

struct LspWorkDoneProgressReport {
	char info;
	// Extends
	// Mixins
	// Properties
	kind;
	bool cancellable;
	char *message;
	percentage;
};

////////////////////////////
// WorkDoneProgressEnd 151

struct LspWorkDoneProgressEnd {
	char info;
	// Extends
	// Mixins
	// Properties
	kind;
	char *message;
};

////////////////////////////
// SetTraceParams 152

struct LspSetTraceParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspTraceValues value;
};

////////////////////////////
// LogTraceParams 153

struct LspLogTraceParams {
	char info;
	// Extends
	// Mixins
	// Properties
	char *message;
	char *verbose;
};

////////////////////////////
// CancelParams 154

struct LspCancelParams {
	char info;
	// Extends
	// Mixins
	// Properties
	id;
};

////////////////////////////
// ProgressParams 155

struct LspProgressParams {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspProgressToken token;
	struct LspLSPAny value;
};

////////////////////////////
// LocationLink 159

struct LspLocationLink {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspRange origin_selection_range;
	char *target_uri;
	struct LspRange target_range;
	struct LspRange target_selection_range;
};

////////////////////////////
// Range 160

struct LspRange {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspPosition start;
	struct LspPosition end;
};

////////////////////////////
// WorkspaceFoldersChangeEvent 164

struct LspWorkspaceFoldersChangeEvent {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspWorkspaceFolder *items;
	} added;
	struct {
		size_t count, struct LspWorkspaceFolder *items;
	} removed;
};

////////////////////////////
// ConfigurationItem 165

struct LspConfigurationItem {
	char info;
	// Extends
	// Mixins
	// Properties
	scope_uri;
	char *section;
};

////////////////////////////
// TextDocumentIdentifier 166

struct LspTextDocumentIdentifier {
	char info;
	// Extends
	// Mixins
	// Properties
	char *uri;
};

////////////////////////////
// Color 167

struct LspColor {
	char info;
	// Extends
	// Mixins
	// Properties
	red;
	green;
	blue;
	alpha;
};

////////////////////////////
// Position 171

struct LspPosition {
	char info;
	// Extends
	// Mixins
	// Properties
	line;
	character;
};

////////////////////////////
// SemanticTokensEdit 175

struct LspSemanticTokensEdit {
	char info;
	// Extends
	// Mixins
	// Properties
	start;
	delete_count;
	struct {
		size_t count, *items;
	} data;
};

////////////////////////////
// FileCreate 177

struct LspFileCreate {
	char info;
	// Extends
	// Mixins
	// Properties
	char *uri;
};

////////////////////////////
// TextDocumentEdit 178

struct LspTextDocumentEdit {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspOptionalVersionedTextDocumentIdentifier text_document;
	struct {
		size_t count, *items;
	} edits;
};

////////////////////////////
// ResourceOperation 248

struct LspResourceOperation {
	char info;
	// Extends
	// Mixins
	// Properties
	char *kind;
	struct LspChangeAnnotationIdentifier annotation_id;
};

////////////////////////////
// CreateFile 179

struct LspCreateFile {
	char info;
	// Extends
	struct LspResourceOperation resource_operation;
	// Mixins
	// Properties
	kind;
	char *uri;
	struct LspCreateFileOptions options;
};

////////////////////////////
// RenameFile 180

struct LspRenameFile {
	char info;
	// Extends
	struct LspResourceOperation resource_operation;
	// Mixins
	// Properties
	kind;
	char *old_uri;
	char *new_uri;
	struct LspRenameFileOptions options;
};

////////////////////////////
// DeleteFile 181

struct LspDeleteFile {
	char info;
	// Extends
	struct LspResourceOperation resource_operation;
	// Mixins
	// Properties
	kind;
	char *uri;
	struct LspDeleteFileOptions options;
};

////////////////////////////
// ChangeAnnotation 182

struct LspChangeAnnotation {
	char info;
	// Extends
	// Mixins
	// Properties
	char *label;
	bool needs_confirmation;
	char *description;
};

////////////////////////////
// FileOperationFilter 183

struct LspFileOperationFilter {
	char info;
	// Extends
	// Mixins
	// Properties
	char *scheme;
	struct LspFileOperationPattern pattern;
};

////////////////////////////
// FileRename 184

struct LspFileRename {
	char info;
	// Extends
	// Mixins
	// Properties
	char *old_uri;
	char *new_uri;
};

////////////////////////////
// FileDelete 185

struct LspFileDelete {
	char info;
	// Extends
	// Mixins
	// Properties
	char *uri;
};

////////////////////////////
// InlineValueContext 188

struct LspInlineValueContext {
	char info;
	// Extends
	// Mixins
	// Properties
	int frame_id;
	struct LspRange stopped_location;
};

////////////////////////////
// InlineValueText 189

struct LspInlineValueText {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspRange range;
	char *text;
};

////////////////////////////
// InlineValueVariableLookup 190

struct LspInlineValueVariableLookup {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspRange range;
	char *variable_name;
	bool case_sensitive_lookup;
};

////////////////////////////
// InlineValueEvaluatableExpression 191

struct LspInlineValueEvaluatableExpression {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspRange range;
	char *expression;
};

////////////////////////////
// InlayHintLabelPart 193

struct LspInlayHintLabelPart {
	char info;
	// Extends
	// Mixins
	// Properties
	char *value;
	tooltip;
	struct LspLocation location;
	struct LspCommand command;
};

////////////////////////////
// MarkupContent 194

struct LspMarkupContent {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspMarkupKind kind;
	char *value;
};

////////////////////////////
// FullDocumentDiagnosticReport 198

struct LspFullDocumentDiagnosticReport {
	char info;
	// Extends
	// Mixins
	// Properties
	kind;
	char *result_id;
	struct {
		size_t count, struct LspDiagnostic *items;
	} items;
};

////////////////////////////
// RelatedFullDocumentDiagnosticReport 196

struct LspRelatedFullDocumentDiagnosticReport {
	char info;
	// Extends
	struct LspFullDocumentDiagnosticReport full_document_diagnostic_report;
	// Mixins
	// Properties
	struct {
		size_t count, char **keys;
		*items;
	} related_documents;
};

////////////////////////////
// UnchangedDocumentDiagnosticReport 199

struct LspUnchangedDocumentDiagnosticReport {
	char info;
	// Extends
	// Mixins
	// Properties
	kind;
	char *result_id;
};

////////////////////////////
// RelatedUnchangedDocumentDiagnosticReport 197

struct LspRelatedUnchangedDocumentDiagnosticReport {
	char info;
	// Extends
	struct LspUnchangedDocumentDiagnosticReport
			unchanged_document_diagnostic_report;
	// Mixins
	// Properties
	struct {
		size_t count, char **keys;
		*items;
	} related_documents;
};

////////////////////////////
// PreviousResultId 201

struct LspPreviousResultId {
	char info;
	// Extends
	// Mixins
	// Properties
	char *uri;
	char *value;
};

////////////////////////////
// NotebookDocument 202

struct LspNotebookDocument {
	char info;
	// Extends
	// Mixins
	// Properties
	uri;
	char *notebook_type;
	int version;
	struct LspLSPObject metadata;
	struct {
		size_t count, struct LspNotebookCell *items;
	} cells;
};

////////////////////////////
// TextDocumentItem 203

struct LspTextDocumentItem {
	char info;
	// Extends
	// Mixins
	// Properties
	char *uri;
	char *language_id;
	int version;
	char *text;
};

////////////////////////////
// VersionedNotebookDocumentIdentifier 204

struct LspVersionedNotebookDocumentIdentifier {
	char info;
	// Extends
	// Mixins
	// Properties
	int version;
	uri;
};

////////////////////////////
// NotebookDocumentChangeEvent 205

struct LspNotebookDocumentChangeEvent {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspLSPObject metadata;
	cells;
};

////////////////////////////
// NotebookDocumentIdentifier 206

struct LspNotebookDocumentIdentifier {
	char info;
	// Extends
	// Mixins
	// Properties
	uri;
};

////////////////////////////
// InlineCompletionContext 207

struct LspInlineCompletionContext {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspInlineCompletionTriggerKind trigger_kind;
	struct LspSelectedCompletionInfo selected_completion_info;
};

////////////////////////////
// StringValue 208

struct LspStringValue {
	char info;
	// Extends
	// Mixins
	// Properties
	kind;
	char *value;
};

////////////////////////////
// Registration 210

struct LspRegistration {
	char info;
	// Extends
	// Mixins
	// Properties
	char *id;
	char *method;
	struct LspLSPAny register_options;
};

////////////////////////////
// Unregistration 211

struct LspUnregistration {
	char info;
	// Extends
	// Mixins
	// Properties
	char *id;
	char *method;
};

////////////////////////////
// ServerCapabilities 214

struct LspServerCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspPositionEncodingKind position_encoding;
	text_document_sync;
	notebook_document_sync;
	struct LspCompletionOptions completion_provider;
	hover_provider;
	struct LspSignatureHelpOptions signature_help_provider;
	declaration_provider;
	definition_provider;
	type_definition_provider;
	implementation_provider;
	references_provider;
	document_highlight_provider;
	document_symbol_provider;
	code_action_provider;
	struct LspCodeLensOptions code_lens_provider;
	struct LspDocumentLinkOptions document_link_provider;
	color_provider;
	workspace_symbol_provider;
	document_formatting_provider;
	document_range_formatting_provider;
	struct LspDocumentOnTypeFormattingOptions
			document_on_type_formatting_provider;
	rename_provider;
	folding_range_provider;
	selection_range_provider;
	struct LspExecuteCommandOptions execute_command_provider;
	call_hierarchy_provider;
	linked_editing_range_provider;
	semantic_tokens_provider;
	moniker_provider;
	type_hierarchy_provider;
	inline_value_provider;
	inlay_hint_provider;
	diagnostic_provider;
	inline_completion_provider;
	text_document;
	workspace;
	struct LspLSPAny experimental;
};

////////////////////////////
// VersionedTextDocumentIdentifier 215

struct LspVersionedTextDocumentIdentifier {
	char info;
	// Extends
	struct LspTextDocumentIdentifier text_document_identifier;
	// Mixins
	// Properties
	int version;
};

////////////////////////////
// FileEvent 217

struct LspFileEvent {
	char info;
	// Extends
	// Mixins
	// Properties
	char *uri;
	struct LspFileChangeType type;
};

////////////////////////////
// FileSystemWatcher 218

struct LspFileSystemWatcher {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspGlobPattern glob_pattern;
	struct LspWatchKind kind;
};

////////////////////////////
// Diagnostic 219

struct LspDiagnostic {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspRange range;
	struct LspDiagnosticSeverity severity;
	code;
	struct LspCodeDescription code_description;
	char *source;
	message;
	struct {
		size_t count, struct LspDiagnosticTag *items;
	} tags;
	struct {
		size_t count, struct LspDiagnosticRelatedInformation *items;
	} related_information;
	struct LspLSPAny data;
};

////////////////////////////
// CompletionContext 220

struct LspCompletionContext {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspCompletionTriggerKind trigger_kind;
	char *trigger_character;
};

////////////////////////////
// CompletionItemLabelDetails 221

struct LspCompletionItemLabelDetails {
	char info;
	// Extends
	// Mixins
	// Properties
	char *detail;
	char *description;
};

////////////////////////////
// InsertReplaceEdit 222

struct LspInsertReplaceEdit {
	char info;
	// Extends
	// Mixins
	// Properties
	char *new_text;
	struct LspRange insert;
	struct LspRange replace;
};

////////////////////////////
// SignatureHelpContext 225

struct LspSignatureHelpContext {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspSignatureHelpTriggerKind trigger_kind;
	char *trigger_character;
	bool is_retrigger;
	struct LspSignatureHelp active_signature_help;
};

////////////////////////////
// SignatureInformation 226

struct LspSignatureInformation {
	char info;
	// Extends
	// Mixins
	// Properties
	char *label;
	documentation;
	struct {
		size_t count, struct LspParameterInformation *items;
	} parameters;
	active_parameter;
};

////////////////////////////
// ReferenceContext 229

struct LspReferenceContext {
	char info;
	// Extends
	// Mixins
	// Properties
	bool include_declaration;
};

////////////////////////////
// CodeActionContext 234

struct LspCodeActionContext {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, struct LspDiagnostic *items;
	} diagnostics;
	struct {
		size_t count, struct LspCodeActionKind *items;
	} only;
	struct LspCodeActionTriggerKind trigger_kind;
};

////////////////////////////
// FormattingOptions 239

struct LspFormattingOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	tab_size;
	bool insert_spaces;
	bool trim_trailing_whitespace;
	bool insert_final_newline;
	bool trim_final_newlines;
};

////////////////////////////
// SemanticTokensLegend 245

struct LspSemanticTokensLegend {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, char **items;
	} token_types;
	struct {
		size_t count, char **items;
	} token_modifiers;
};

////////////////////////////
// OptionalVersionedTextDocumentIdentifier 246

struct LspOptionalVersionedTextDocumentIdentifier {
	char info;
	// Extends
	struct LspTextDocumentIdentifier text_document_identifier;
	// Mixins
	// Properties
	version;
};

////////////////////////////
// AnnotatedTextEdit 247

struct LspAnnotatedTextEdit {
	char info;
	// Extends
	struct LspTextEdit text_edit;
	// Mixins
	// Properties
	struct LspChangeAnnotationIdentifier annotation_id;
};

////////////////////////////
// CreateFileOptions 249

struct LspCreateFileOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	bool overwrite;
	bool ignore_if_exists;
};

////////////////////////////
// RenameFileOptions 250

struct LspRenameFileOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	bool overwrite;
	bool ignore_if_exists;
};

////////////////////////////
// DeleteFileOptions 251

struct LspDeleteFileOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	bool recursive;
	bool ignore_if_not_exists;
};

////////////////////////////
// FileOperationPattern 252

struct LspFileOperationPattern {
	char info;
	// Extends
	// Mixins
	// Properties
	char *glob;
	struct LspFileOperationPatternKind matches;
	struct LspFileOperationPatternOptions options;
};

////////////////////////////
// WorkspaceFullDocumentDiagnosticReport 253

struct LspWorkspaceFullDocumentDiagnosticReport {
	char info;
	// Extends
	struct LspFullDocumentDiagnosticReport full_document_diagnostic_report;
	// Mixins
	// Properties
	char *uri;
	version;
};

////////////////////////////
// WorkspaceUnchangedDocumentDiagnosticReport 254

struct LspWorkspaceUnchangedDocumentDiagnosticReport {
	char info;
	// Extends
	struct LspUnchangedDocumentDiagnosticReport
			unchanged_document_diagnostic_report;
	// Mixins
	// Properties
	char *uri;
	version;
};

////////////////////////////
// NotebookCell 255

struct LspNotebookCell {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspNotebookCellKind kind;
	char *document;
	struct LspLSPObject metadata;
	struct LspExecutionSummary execution_summary;
};

////////////////////////////
// NotebookCellArrayChange 256

struct LspNotebookCellArrayChange {
	char info;
	// Extends
	// Mixins
	// Properties
	start;
	delete_count;
	struct {
		size_t count, struct LspNotebookCell *items;
	} cells;
};

////////////////////////////
// SelectedCompletionInfo 257

struct LspSelectedCompletionInfo {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspRange range;
	char *text;
};

////////////////////////////
// ClientCapabilities 258

struct LspClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspWorkspaceClientCapabilities workspace;
	struct LspTextDocumentClientCapabilities text_document;
	struct LspNotebookDocumentClientCapabilities notebook_document;
	struct LspWindowClientCapabilities window;
	struct LspGeneralClientCapabilities general;
	struct LspLSPAny experimental;
};

////////////////////////////
// TextDocumentSyncOptions 259

struct LspTextDocumentSyncOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	bool open_close;
	struct LspTextDocumentSyncKind change;
	bool will_save;
	bool will_save_wait_until;
	save;
};

////////////////////////////
// NotebookDocumentSyncOptions 260

struct LspNotebookDocumentSyncOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	struct {
		size_t count, *items;
	} notebook_selector;
	bool save;
};

////////////////////////////
// NotebookDocumentSyncRegistrationOptions 261

struct LspNotebookDocumentSyncRegistrationOptions {
	char info;
	// Extends
	struct LspNotebookDocumentSyncOptions notebook_document_sync_options;
	// Mixins
	struct LspStaticRegistrationOptions static_registration_options;
	// Properties
};

////////////////////////////
// WorkspaceFoldersServerCapabilities 262

struct LspWorkspaceFoldersServerCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool supported;
	change_notifications;
};

////////////////////////////
// FileOperationOptions 263

struct LspFileOperationOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspFileOperationRegistrationOptions did_create;
	struct LspFileOperationRegistrationOptions will_create;
	struct LspFileOperationRegistrationOptions did_rename;
	struct LspFileOperationRegistrationOptions will_rename;
	struct LspFileOperationRegistrationOptions did_delete;
	struct LspFileOperationRegistrationOptions will_delete;
};

////////////////////////////
// CodeDescription 264

struct LspCodeDescription {
	char info;
	// Extends
	// Mixins
	// Properties
	href;
};

////////////////////////////
// DiagnosticRelatedInformation 265

struct LspDiagnosticRelatedInformation {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspLocation location;
	char *message;
};

////////////////////////////
// ParameterInformation 266

struct LspParameterInformation {
	char info;
	// Extends
	// Mixins
	// Properties
	label;
	documentation;
};

////////////////////////////
// NotebookCellTextDocumentFilter 267

struct LspNotebookCellTextDocumentFilter {
	char info;
	// Extends
	// Mixins
	// Properties
	notebook;
	char *language;
};

////////////////////////////
// FileOperationPatternOptions 268

struct LspFileOperationPatternOptions {
	char info;
	// Extends
	// Mixins
	// Properties
	bool ignore_case;
};

////////////////////////////
// ExecutionSummary 269

struct LspExecutionSummary {
	char info;
	// Extends
	// Mixins
	// Properties
	execution_order;
	bool success;
};

////////////////////////////
// WorkspaceClientCapabilities 270

struct LspWorkspaceClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool apply_edit;
	struct LspWorkspaceEditClientCapabilities workspace_edit;
	struct LspDidChangeConfigurationClientCapabilities did_change_configuration;
	struct LspDidChangeWatchedFilesClientCapabilities did_change_watched_files;
	struct LspWorkspaceSymbolClientCapabilities symbol;
	struct LspExecuteCommandClientCapabilities execute_command;
	bool workspace_folders;
	bool configuration;
	struct LspSemanticTokensWorkspaceClientCapabilities semantic_tokens;
	struct LspCodeLensWorkspaceClientCapabilities code_lens;
	struct LspFileOperationClientCapabilities file_operations;
	struct LspInlineValueWorkspaceClientCapabilities inline_value;
	struct LspInlayHintWorkspaceClientCapabilities inlay_hint;
	struct LspDiagnosticWorkspaceClientCapabilities diagnostics;
	struct LspFoldingRangeWorkspaceClientCapabilities folding_range;
};

////////////////////////////
// TextDocumentClientCapabilities 271

struct LspTextDocumentClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspTextDocumentSyncClientCapabilities synchronization;
	struct LspCompletionClientCapabilities completion;
	struct LspHoverClientCapabilities hover;
	struct LspSignatureHelpClientCapabilities signature_help;
	struct LspDeclarationClientCapabilities declaration;
	struct LspDefinitionClientCapabilities definition;
	struct LspTypeDefinitionClientCapabilities type_definition;
	struct LspImplementationClientCapabilities implementation;
	struct LspReferenceClientCapabilities references;
	struct LspDocumentHighlightClientCapabilities document_highlight;
	struct LspDocumentSymbolClientCapabilities document_symbol;
	struct LspCodeActionClientCapabilities code_action;
	struct LspCodeLensClientCapabilities code_lens;
	struct LspDocumentLinkClientCapabilities document_link;
	struct LspDocumentColorClientCapabilities color_provider;
	struct LspDocumentFormattingClientCapabilities formatting;
	struct LspDocumentRangeFormattingClientCapabilities range_formatting;
	struct LspDocumentOnTypeFormattingClientCapabilities on_type_formatting;
	struct LspRenameClientCapabilities rename;
	struct LspFoldingRangeClientCapabilities folding_range;
	struct LspSelectionRangeClientCapabilities selection_range;
	struct LspPublishDiagnosticsClientCapabilities publish_diagnostics;
	struct LspCallHierarchyClientCapabilities call_hierarchy;
	struct LspSemanticTokensClientCapabilities semantic_tokens;
	struct LspLinkedEditingRangeClientCapabilities linked_editing_range;
	struct LspMonikerClientCapabilities moniker;
	struct LspTypeHierarchyClientCapabilities type_hierarchy;
	struct LspInlineValueClientCapabilities inline_value;
	struct LspInlayHintClientCapabilities inlay_hint;
	struct LspDiagnosticClientCapabilities diagnostic;
	struct LspInlineCompletionClientCapabilities inline_completion;
};

////////////////////////////
// NotebookDocumentClientCapabilities 272

struct LspNotebookDocumentClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	struct LspNotebookDocumentSyncClientCapabilities synchronization;
};

////////////////////////////
// WindowClientCapabilities 273

struct LspWindowClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool work_done_progress;
	struct LspShowMessageRequestClientCapabilities show_message;
	struct LspShowDocumentClientCapabilities show_document;
};

////////////////////////////
// GeneralClientCapabilities 274

struct LspGeneralClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	stale_request_support;
	struct LspRegularExpressionsClientCapabilities regular_expressions;
	struct LspMarkdownClientCapabilities markdown;
	struct {
		size_t count, struct LspPositionEncodingKind *items;
	} position_encodings;
};

////////////////////////////
// RelativePattern 275

struct LspRelativePattern {
	char info;
	// Extends
	// Mixins
	// Properties
	base_uri;
	struct LspPattern pattern;
};

////////////////////////////
// WorkspaceEditClientCapabilities 276

struct LspWorkspaceEditClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool document_changes;
	struct {
		size_t count, struct LspResourceOperationKind *items;
	} resource_operations;
	struct LspFailureHandlingKind failure_handling;
	bool normalizes_line_endings;
	change_annotation_support;
};

////////////////////////////
// DidChangeConfigurationClientCapabilities 277

struct LspDidChangeConfigurationClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// DidChangeWatchedFilesClientCapabilities 278

struct LspDidChangeWatchedFilesClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	bool relative_pattern_support;
};

////////////////////////////
// WorkspaceSymbolClientCapabilities 279

struct LspWorkspaceSymbolClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	symbol_kind;
	tag_support;
	resolve_support;
};

////////////////////////////
// ExecuteCommandClientCapabilities 280

struct LspExecuteCommandClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// SemanticTokensWorkspaceClientCapabilities 281

struct LspSemanticTokensWorkspaceClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool refresh_support;
};

////////////////////////////
// CodeLensWorkspaceClientCapabilities 282

struct LspCodeLensWorkspaceClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool refresh_support;
};

////////////////////////////
// FileOperationClientCapabilities 283

struct LspFileOperationClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	bool did_create;
	bool will_create;
	bool did_rename;
	bool will_rename;
	bool did_delete;
	bool will_delete;
};

////////////////////////////
// InlineValueWorkspaceClientCapabilities 284

struct LspInlineValueWorkspaceClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool refresh_support;
};

////////////////////////////
// InlayHintWorkspaceClientCapabilities 285

struct LspInlayHintWorkspaceClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool refresh_support;
};

////////////////////////////
// DiagnosticWorkspaceClientCapabilities 286

struct LspDiagnosticWorkspaceClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool refresh_support;
};

////////////////////////////
// FoldingRangeWorkspaceClientCapabilities 287

struct LspFoldingRangeWorkspaceClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool refresh_support;
};

////////////////////////////
// TextDocumentSyncClientCapabilities 288

struct LspTextDocumentSyncClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	bool will_save;
	bool will_save_wait_until;
	bool did_save;
};

////////////////////////////
// CompletionClientCapabilities 289

struct LspCompletionClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	completion_item;
	completion_item_kind;
	struct LspInsertTextMode insert_text_mode;
	bool context_support;
	completion_list;
};

////////////////////////////
// HoverClientCapabilities 290

struct LspHoverClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	struct {
		size_t count, struct LspMarkupKind *items;
	} content_format;
};

////////////////////////////
// SignatureHelpClientCapabilities 291

struct LspSignatureHelpClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	signature_information;
	bool context_support;
};

////////////////////////////
// DeclarationClientCapabilities 292

struct LspDeclarationClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	bool link_support;
};

////////////////////////////
// DefinitionClientCapabilities 293

struct LspDefinitionClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	bool link_support;
};

////////////////////////////
// TypeDefinitionClientCapabilities 294

struct LspTypeDefinitionClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	bool link_support;
};

////////////////////////////
// ImplementationClientCapabilities 295

struct LspImplementationClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	bool link_support;
};

////////////////////////////
// ReferenceClientCapabilities 296

struct LspReferenceClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// DocumentHighlightClientCapabilities 297

struct LspDocumentHighlightClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// DocumentSymbolClientCapabilities 298

struct LspDocumentSymbolClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	symbol_kind;
	bool hierarchical_document_symbol_support;
	tag_support;
	bool label_support;
};

////////////////////////////
// CodeActionClientCapabilities 299

struct LspCodeActionClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	code_action_literal_support;
	bool is_preferred_support;
	bool disabled_support;
	bool data_support;
	resolve_support;
	bool honors_change_annotations;
};

////////////////////////////
// CodeLensClientCapabilities 300

struct LspCodeLensClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// DocumentLinkClientCapabilities 301

struct LspDocumentLinkClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	bool tooltip_support;
};

////////////////////////////
// DocumentColorClientCapabilities 302

struct LspDocumentColorClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// DocumentFormattingClientCapabilities 303

struct LspDocumentFormattingClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// DocumentRangeFormattingClientCapabilities 304

struct LspDocumentRangeFormattingClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	bool ranges_support;
};

////////////////////////////
// DocumentOnTypeFormattingClientCapabilities 305

struct LspDocumentOnTypeFormattingClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// RenameClientCapabilities 306

struct LspRenameClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	bool prepare_support;
	struct LspPrepareSupportDefaultBehavior prepare_support_default_behavior;
	bool honors_change_annotations;
};

////////////////////////////
// FoldingRangeClientCapabilities 307

struct LspFoldingRangeClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	range_limit;
	bool line_folding_only;
	folding_range_kind;
	folding_range;
};

////////////////////////////
// SelectionRangeClientCapabilities 308

struct LspSelectionRangeClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// PublishDiagnosticsClientCapabilities 309

struct LspPublishDiagnosticsClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool related_information;
	tag_support;
	bool version_support;
	bool code_description_support;
	bool data_support;
};

////////////////////////////
// CallHierarchyClientCapabilities 310

struct LspCallHierarchyClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// SemanticTokensClientCapabilities 311

struct LspSemanticTokensClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	requests;
	struct {
		size_t count, char **items;
	} token_types;
	struct {
		size_t count, char **items;
	} token_modifiers;
	struct {
		size_t count, struct LspTokenFormat *items;
	} formats;
	bool overlapping_token_support;
	bool multiline_token_support;
	bool server_cancel_support;
	bool augments_syntax_tokens;
};

////////////////////////////
// LinkedEditingRangeClientCapabilities 312

struct LspLinkedEditingRangeClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// MonikerClientCapabilities 313

struct LspMonikerClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// TypeHierarchyClientCapabilities 314

struct LspTypeHierarchyClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// InlineValueClientCapabilities 315

struct LspInlineValueClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// InlayHintClientCapabilities 316

struct LspInlayHintClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	resolve_support;
};

////////////////////////////
// DiagnosticClientCapabilities 317

struct LspDiagnosticClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	bool related_document_support;
	bool markup_message_support;
};

////////////////////////////
// InlineCompletionClientCapabilities 318

struct LspInlineCompletionClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
};

////////////////////////////
// NotebookDocumentSyncClientCapabilities 319

struct LspNotebookDocumentSyncClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool dynamic_registration;
	bool execution_summary_support;
};

////////////////////////////
// ShowMessageRequestClientCapabilities 320

struct LspShowMessageRequestClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	message_action_item;
};

////////////////////////////
// ShowDocumentClientCapabilities 321

struct LspShowDocumentClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	bool support;
};

////////////////////////////
// RegularExpressionsClientCapabilities 322

struct LspRegularExpressionsClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	char *engine;
	char *version;
};

////////////////////////////
// MarkdownClientCapabilities 323

struct LspMarkdownClientCapabilities {
	char info;
	// Extends
	// Mixins
	// Properties
	char *parser;
	char *version;
	struct {
		size_t count, char **items;
	} allowed_tags;
};
