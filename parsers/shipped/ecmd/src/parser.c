#include "tree_sitter/parser.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#define LANGUAGE_VERSION 14
#define STATE_COUNT 62
#define LARGE_STATE_COUNT 4
#define SYMBOL_COUNT 35
#define ALIAS_COUNT 0
#define TOKEN_COUNT 19
#define EXTERNAL_TOKEN_COUNT 0
#define FIELD_COUNT 2
#define MAX_ALIAS_SEQUENCE_LENGTH 4
#define PRODUCTION_ID_COUNT 2

enum ts_symbol_identifiers {
  anon_sym_LBRACE = 1,
  anon_sym_COMMA = 2,
  anon_sym_RBRACE = 3,
  anon_sym_COLON = 4,
  anon_sym_LBRACK = 5,
  anon_sym_RBRACK = 6,
  anon_sym_DQUOTE = 7,
  aux_sym_string_content_token1 = 8,
  sym_escape_sequence = 9,
  sym_number = 10,
  sym_true = 11,
  sym_false = 12,
  sym_null = 13,
  sym_e_simple_value = 14,
  aux_sym_e_command_token1 = 15,
  sym_e_command_name = 16,
  aux_sym_e_comment_token1 = 17,
  sym_e_comment_value = 18,
  sym_document = 19,
  sym__value = 20,
  sym_object = 21,
  sym_pair = 22,
  sym_array = 23,
  sym_string = 24,
  sym_string_content = 25,
  sym_e_parameter = 26,
  sym_e_json_value = 27,
  sym_e_command = 28,
  sym_e_comment = 29,
  aux_sym_document_repeat1 = 30,
  aux_sym_object_repeat1 = 31,
  aux_sym_array_repeat1 = 32,
  aux_sym_string_content_repeat1 = 33,
  aux_sym_e_command_repeat1 = 34,
};

static const char * const ts_symbol_names[] = {
  [ts_builtin_sym_end] = "end",
  [anon_sym_LBRACE] = "{",
  [anon_sym_COMMA] = ",",
  [anon_sym_RBRACE] = "}",
  [anon_sym_COLON] = ":",
  [anon_sym_LBRACK] = "[",
  [anon_sym_RBRACK] = "]",
  [anon_sym_DQUOTE] = "\"",
  [aux_sym_string_content_token1] = "string_content_token1",
  [sym_escape_sequence] = "escape_sequence",
  [sym_number] = "number",
  [sym_true] = "true",
  [sym_false] = "false",
  [sym_null] = "null",
  [sym_e_simple_value] = "e_simple_value",
  [aux_sym_e_command_token1] = "e_command_token1",
  [sym_e_command_name] = "e_command_name",
  [aux_sym_e_comment_token1] = "e_comment_token1",
  [sym_e_comment_value] = "e_comment_value",
  [sym_document] = "document",
  [sym__value] = "_value",
  [sym_object] = "object",
  [sym_pair] = "pair",
  [sym_array] = "array",
  [sym_string] = "string",
  [sym_string_content] = "string_content",
  [sym_e_parameter] = "e_parameter",
  [sym_e_json_value] = "e_json_value",
  [sym_e_command] = "e_command",
  [sym_e_comment] = "e_comment",
  [aux_sym_document_repeat1] = "document_repeat1",
  [aux_sym_object_repeat1] = "object_repeat1",
  [aux_sym_array_repeat1] = "array_repeat1",
  [aux_sym_string_content_repeat1] = "string_content_repeat1",
  [aux_sym_e_command_repeat1] = "e_command_repeat1",
};

static const TSSymbol ts_symbol_map[] = {
  [ts_builtin_sym_end] = ts_builtin_sym_end,
  [anon_sym_LBRACE] = anon_sym_LBRACE,
  [anon_sym_COMMA] = anon_sym_COMMA,
  [anon_sym_RBRACE] = anon_sym_RBRACE,
  [anon_sym_COLON] = anon_sym_COLON,
  [anon_sym_LBRACK] = anon_sym_LBRACK,
  [anon_sym_RBRACK] = anon_sym_RBRACK,
  [anon_sym_DQUOTE] = anon_sym_DQUOTE,
  [aux_sym_string_content_token1] = aux_sym_string_content_token1,
  [sym_escape_sequence] = sym_escape_sequence,
  [sym_number] = sym_number,
  [sym_true] = sym_true,
  [sym_false] = sym_false,
  [sym_null] = sym_null,
  [sym_e_simple_value] = sym_e_simple_value,
  [aux_sym_e_command_token1] = aux_sym_e_command_token1,
  [sym_e_command_name] = sym_e_command_name,
  [aux_sym_e_comment_token1] = aux_sym_e_comment_token1,
  [sym_e_comment_value] = sym_e_comment_value,
  [sym_document] = sym_document,
  [sym__value] = sym__value,
  [sym_object] = sym_object,
  [sym_pair] = sym_pair,
  [sym_array] = sym_array,
  [sym_string] = sym_string,
  [sym_string_content] = sym_string_content,
  [sym_e_parameter] = sym_e_parameter,
  [sym_e_json_value] = sym_e_json_value,
  [sym_e_command] = sym_e_command,
  [sym_e_comment] = sym_e_comment,
  [aux_sym_document_repeat1] = aux_sym_document_repeat1,
  [aux_sym_object_repeat1] = aux_sym_object_repeat1,
  [aux_sym_array_repeat1] = aux_sym_array_repeat1,
  [aux_sym_string_content_repeat1] = aux_sym_string_content_repeat1,
  [aux_sym_e_command_repeat1] = aux_sym_e_command_repeat1,
};

static const TSSymbolMetadata ts_symbol_metadata[] = {
  [ts_builtin_sym_end] = {
    .visible = false,
    .named = true,
  },
  [anon_sym_LBRACE] = {
    .visible = true,
    .named = false,
  },
  [anon_sym_COMMA] = {
    .visible = true,
    .named = false,
  },
  [anon_sym_RBRACE] = {
    .visible = true,
    .named = false,
  },
  [anon_sym_COLON] = {
    .visible = true,
    .named = false,
  },
  [anon_sym_LBRACK] = {
    .visible = true,
    .named = false,
  },
  [anon_sym_RBRACK] = {
    .visible = true,
    .named = false,
  },
  [anon_sym_DQUOTE] = {
    .visible = true,
    .named = false,
  },
  [aux_sym_string_content_token1] = {
    .visible = false,
    .named = false,
  },
  [sym_escape_sequence] = {
    .visible = true,
    .named = true,
  },
  [sym_number] = {
    .visible = true,
    .named = true,
  },
  [sym_true] = {
    .visible = true,
    .named = true,
  },
  [sym_false] = {
    .visible = true,
    .named = true,
  },
  [sym_null] = {
    .visible = true,
    .named = true,
  },
  [sym_e_simple_value] = {
    .visible = true,
    .named = true,
  },
  [aux_sym_e_command_token1] = {
    .visible = false,
    .named = false,
  },
  [sym_e_command_name] = {
    .visible = true,
    .named = true,
  },
  [aux_sym_e_comment_token1] = {
    .visible = false,
    .named = false,
  },
  [sym_e_comment_value] = {
    .visible = true,
    .named = true,
  },
  [sym_document] = {
    .visible = true,
    .named = true,
  },
  [sym__value] = {
    .visible = false,
    .named = true,
    .supertype = true,
  },
  [sym_object] = {
    .visible = true,
    .named = true,
  },
  [sym_pair] = {
    .visible = true,
    .named = true,
  },
  [sym_array] = {
    .visible = true,
    .named = true,
  },
  [sym_string] = {
    .visible = true,
    .named = true,
  },
  [sym_string_content] = {
    .visible = true,
    .named = true,
  },
  [sym_e_parameter] = {
    .visible = true,
    .named = true,
  },
  [sym_e_json_value] = {
    .visible = true,
    .named = true,
  },
  [sym_e_command] = {
    .visible = true,
    .named = true,
  },
  [sym_e_comment] = {
    .visible = true,
    .named = true,
  },
  [aux_sym_document_repeat1] = {
    .visible = false,
    .named = false,
  },
  [aux_sym_object_repeat1] = {
    .visible = false,
    .named = false,
  },
  [aux_sym_array_repeat1] = {
    .visible = false,
    .named = false,
  },
  [aux_sym_string_content_repeat1] = {
    .visible = false,
    .named = false,
  },
  [aux_sym_e_command_repeat1] = {
    .visible = false,
    .named = false,
  },
};

enum ts_field_identifiers {
  field_key = 1,
  field_value = 2,
};

static const char * const ts_field_names[] = {
  [0] = NULL,
  [field_key] = "key",
  [field_value] = "value",
};

static const TSFieldMapSlice ts_field_map_slices[PRODUCTION_ID_COUNT] = {
  [1] = {.index = 0, .length = 2},
};

static const TSFieldMapEntry ts_field_map_entries[] = {
  [0] =
    {field_key, 0},
    {field_value, 2},
};

static const TSSymbol ts_alias_sequences[PRODUCTION_ID_COUNT][MAX_ALIAS_SEQUENCE_LENGTH] = {
  [0] = {0},
};

static const uint16_t ts_non_terminal_alias_map[] = {
  0,
};

static const TSStateId ts_primary_state_ids[STATE_COUNT] = {
  [0] = 0,
  [1] = 1,
  [2] = 2,
  [3] = 3,
  [4] = 4,
  [5] = 5,
  [6] = 5,
  [7] = 7,
  [8] = 8,
  [9] = 9,
  [10] = 10,
  [11] = 11,
  [12] = 12,
  [13] = 13,
  [14] = 14,
  [15] = 15,
  [16] = 16,
  [17] = 17,
  [18] = 18,
  [19] = 19,
  [20] = 20,
  [21] = 21,
  [22] = 21,
  [23] = 20,
  [24] = 24,
  [25] = 25,
  [26] = 26,
  [27] = 27,
  [28] = 14,
  [29] = 19,
  [30] = 30,
  [31] = 13,
  [32] = 11,
  [33] = 33,
  [34] = 34,
  [35] = 35,
  [36] = 36,
  [37] = 36,
  [38] = 33,
  [39] = 34,
  [40] = 35,
  [41] = 41,
  [42] = 16,
  [43] = 18,
  [44] = 44,
  [45] = 9,
  [46] = 15,
  [47] = 17,
  [48] = 48,
  [49] = 49,
  [50] = 50,
  [51] = 51,
  [52] = 52,
  [53] = 53,
  [54] = 54,
  [55] = 55,
  [56] = 56,
  [57] = 57,
  [58] = 58,
  [59] = 59,
  [60] = 57,
  [61] = 61,
};

static bool ts_lex(TSLexer *lexer, TSStateId state) {
  START_LEXER();
  eof = lexer->eof(lexer);
  switch (state) {
    case 0:
      if (eof) ADVANCE(27);
      ADVANCE_MAP(
        '"', 34,
        '#', 65,
        ',', 29,
        '-', 7,
        '/', 5,
        '0', 38,
        ':', 31,
        '[', 32,
        '\\', 22,
        ']', 33,
        'f', 9,
        'n', 20,
        't', 17,
        '{', 28,
        '}', 30,
      );
      if (('\t' <= lookahead && lookahead <= '\r') ||
          lookahead == ' ') SKIP(25);
      if (('1' <= lookahead && lookahead <= '9')) ADVANCE(39);
      END_STATE();
    case 1:
      ADVANCE_MAP(
        '\n', 63,
        '"', 34,
        '#', 66,
        '-', 8,
        '/', 6,
        '0', 41,
        '[', 32,
        'f', 10,
        'n', 21,
        't', 16,
        '{', 28,
      );
      if (('\t' <= lookahead && lookahead <= '\r') ||
          lookahead == ' ') SKIP(1);
      if (('1' <= lookahead && lookahead <= '9')) ADVANCE(40);
      if (lookahead != 0) ADVANCE(24);
      END_STATE();
    case 2:
      if (lookahead == '\n') SKIP(3);
      if (lookahead == '"') ADVANCE(34);
      if (lookahead == '\\') ADVANCE(22);
      if (('\t' <= lookahead && lookahead <= '\r') ||
          lookahead == ' ') ADVANCE(35);
      if (lookahead != 0) ADVANCE(36);
      END_STATE();
    case 3:
      if (lookahead == '"') ADVANCE(34);
      if (('\t' <= lookahead && lookahead <= '\r') ||
          lookahead == ' ') SKIP(3);
      END_STATE();
    case 4:
      if (lookahead == '-') ADVANCE(23);
      if (('0' <= lookahead && lookahead <= '9')) ADVANCE(44);
      END_STATE();
    case 5:
      if (lookahead == '/') ADVANCE(65);
      END_STATE();
    case 6:
      if (lookahead == '/') ADVANCE(52);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 7:
      if (lookahead == '0') ADVANCE(38);
      if (('1' <= lookahead && lookahead <= '9')) ADVANCE(39);
      END_STATE();
    case 8:
      if (lookahead == '0') ADVANCE(41);
      if (('1' <= lookahead && lookahead <= '9')) ADVANCE(40);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 9:
      if (lookahead == 'a') ADVANCE(13);
      END_STATE();
    case 10:
      if (lookahead == 'a') ADVANCE(56);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 11:
      if (lookahead == 'e') ADVANCE(46);
      END_STATE();
    case 12:
      if (lookahead == 'e') ADVANCE(48);
      END_STATE();
    case 13:
      if (lookahead == 'l') ADVANCE(18);
      END_STATE();
    case 14:
      if (lookahead == 'l') ADVANCE(50);
      END_STATE();
    case 15:
      if (lookahead == 'l') ADVANCE(14);
      END_STATE();
    case 16:
      if (lookahead == 'r') ADVANCE(60);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 17:
      if (lookahead == 'r') ADVANCE(19);
      END_STATE();
    case 18:
      if (lookahead == 's') ADVANCE(12);
      END_STATE();
    case 19:
      if (lookahead == 'u') ADVANCE(11);
      END_STATE();
    case 20:
      if (lookahead == 'u') ADVANCE(15);
      END_STATE();
    case 21:
      if (lookahead == 'u') ADVANCE(58);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 22:
      ADVANCE_MAP(
        '"', 37,
        '/', 37,
        '\\', 37,
        'b', 37,
        'f', 37,
        'n', 37,
        'r', 37,
        't', 37,
        'u', 37,
      );
      END_STATE();
    case 23:
      if (('0' <= lookahead && lookahead <= '9')) ADVANCE(44);
      END_STATE();
    case 24:
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 25:
      if (eof) ADVANCE(27);
      ADVANCE_MAP(
        '"', 34,
        '#', 65,
        ',', 29,
        '-', 7,
        '/', 5,
        '0', 38,
        ':', 31,
        '[', 32,
        ']', 33,
        'f', 9,
        'n', 20,
        't', 17,
        '{', 28,
        '}', 30,
      );
      if (('\t' <= lookahead && lookahead <= '\r') ||
          lookahead == ' ') SKIP(25);
      if (('1' <= lookahead && lookahead <= '9')) ADVANCE(39);
      END_STATE();
    case 26:
      if (eof) ADVANCE(27);
      if (('\t' <= lookahead && lookahead <= '\r') ||
          lookahead == ' ') SKIP(26);
      if (lookahead == '-' ||
          ('0' <= lookahead && lookahead <= '9') ||
          ('A' <= lookahead && lookahead <= 'Z') ||
          lookahead == '_' ||
          ('a' <= lookahead && lookahead <= 'z')) ADVANCE(64);
      END_STATE();
    case 27:
      ACCEPT_TOKEN(ts_builtin_sym_end);
      END_STATE();
    case 28:
      ACCEPT_TOKEN(anon_sym_LBRACE);
      END_STATE();
    case 29:
      ACCEPT_TOKEN(anon_sym_COMMA);
      END_STATE();
    case 30:
      ACCEPT_TOKEN(anon_sym_RBRACE);
      END_STATE();
    case 31:
      ACCEPT_TOKEN(anon_sym_COLON);
      END_STATE();
    case 32:
      ACCEPT_TOKEN(anon_sym_LBRACK);
      END_STATE();
    case 33:
      ACCEPT_TOKEN(anon_sym_RBRACK);
      END_STATE();
    case 34:
      ACCEPT_TOKEN(anon_sym_DQUOTE);
      END_STATE();
    case 35:
      ACCEPT_TOKEN(aux_sym_string_content_token1);
      if (lookahead == '\t' ||
          (0x0b <= lookahead && lookahead <= '\r') ||
          lookahead == ' ') ADVANCE(35);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != '"' &&
          lookahead != '\\') ADVANCE(36);
      END_STATE();
    case 36:
      ACCEPT_TOKEN(aux_sym_string_content_token1);
      if (lookahead != 0 &&
          lookahead != '\n' &&
          lookahead != '"' &&
          lookahead != '\\') ADVANCE(36);
      END_STATE();
    case 37:
      ACCEPT_TOKEN(sym_escape_sequence);
      END_STATE();
    case 38:
      ACCEPT_TOKEN(sym_number);
      if (lookahead == '.') ADVANCE(42);
      if (lookahead == 'E' ||
          lookahead == 'e') ADVANCE(4);
      END_STATE();
    case 39:
      ACCEPT_TOKEN(sym_number);
      if (lookahead == '.') ADVANCE(42);
      if (lookahead == 'E' ||
          lookahead == 'e') ADVANCE(4);
      if (('0' <= lookahead && lookahead <= '9')) ADVANCE(39);
      END_STATE();
    case 40:
      ACCEPT_TOKEN(sym_number);
      if (lookahead == '.') ADVANCE(43);
      if (lookahead == 'E' ||
          lookahead == 'e') ADVANCE(53);
      if (('0' <= lookahead && lookahead <= '9')) ADVANCE(40);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 41:
      ACCEPT_TOKEN(sym_number);
      if (lookahead == '.') ADVANCE(43);
      if (lookahead == 'E' ||
          lookahead == 'e') ADVANCE(53);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 42:
      ACCEPT_TOKEN(sym_number);
      if (lookahead == 'E' ||
          lookahead == 'e') ADVANCE(4);
      if (('0' <= lookahead && lookahead <= '9')) ADVANCE(42);
      END_STATE();
    case 43:
      ACCEPT_TOKEN(sym_number);
      if (lookahead == 'E' ||
          lookahead == 'e') ADVANCE(53);
      if (('0' <= lookahead && lookahead <= '9')) ADVANCE(43);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 44:
      ACCEPT_TOKEN(sym_number);
      if (('0' <= lookahead && lookahead <= '9')) ADVANCE(44);
      END_STATE();
    case 45:
      ACCEPT_TOKEN(sym_number);
      if (('0' <= lookahead && lookahead <= '9')) ADVANCE(45);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 46:
      ACCEPT_TOKEN(sym_true);
      END_STATE();
    case 47:
      ACCEPT_TOKEN(sym_true);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 48:
      ACCEPT_TOKEN(sym_false);
      END_STATE();
    case 49:
      ACCEPT_TOKEN(sym_false);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 50:
      ACCEPT_TOKEN(sym_null);
      END_STATE();
    case 51:
      ACCEPT_TOKEN(sym_null);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 52:
      ACCEPT_TOKEN(sym_e_simple_value);
      if (lookahead == ' ') ADVANCE(65);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead)) ADVANCE(62);
      END_STATE();
    case 53:
      ACCEPT_TOKEN(sym_e_simple_value);
      if (lookahead == '-') ADVANCE(61);
      if (('0' <= lookahead && lookahead <= '9')) ADVANCE(45);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 54:
      ACCEPT_TOKEN(sym_e_simple_value);
      if (lookahead == 'e') ADVANCE(47);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 55:
      ACCEPT_TOKEN(sym_e_simple_value);
      if (lookahead == 'e') ADVANCE(49);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 56:
      ACCEPT_TOKEN(sym_e_simple_value);
      if (lookahead == 'l') ADVANCE(59);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 57:
      ACCEPT_TOKEN(sym_e_simple_value);
      if (lookahead == 'l') ADVANCE(51);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 58:
      ACCEPT_TOKEN(sym_e_simple_value);
      if (lookahead == 'l') ADVANCE(57);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 59:
      ACCEPT_TOKEN(sym_e_simple_value);
      if (lookahead == 's') ADVANCE(55);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 60:
      ACCEPT_TOKEN(sym_e_simple_value);
      if (lookahead == 'u') ADVANCE(54);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 61:
      ACCEPT_TOKEN(sym_e_simple_value);
      if (('0' <= lookahead && lookahead <= '9')) ADVANCE(45);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 62:
      ACCEPT_TOKEN(sym_e_simple_value);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead) &&
          lookahead != ' ') ADVANCE(62);
      END_STATE();
    case 63:
      ACCEPT_TOKEN(aux_sym_e_command_token1);
      if (lookahead == '\n') ADVANCE(63);
      END_STATE();
    case 64:
      ACCEPT_TOKEN(sym_e_command_name);
      if (lookahead == '-' ||
          ('0' <= lookahead && lookahead <= '9') ||
          ('A' <= lookahead && lookahead <= 'Z') ||
          lookahead == '_' ||
          ('a' <= lookahead && lookahead <= 'z')) ADVANCE(64);
      END_STATE();
    case 65:
      ACCEPT_TOKEN(aux_sym_e_comment_token1);
      if (lookahead == ' ') ADVANCE(65);
      END_STATE();
    case 66:
      ACCEPT_TOKEN(aux_sym_e_comment_token1);
      if (lookahead == ' ') ADVANCE(65);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead)) ADVANCE(62);
      END_STATE();
    case 67:
      ACCEPT_TOKEN(sym_e_comment_value);
      if (lookahead == '\t' ||
          (0x0b <= lookahead && lookahead <= '\r') ||
          lookahead == ' ') ADVANCE(67);
      if (lookahead != 0 &&
          (lookahead < '\t' || '\r' < lookahead)) ADVANCE(68);
      END_STATE();
    case 68:
      ACCEPT_TOKEN(sym_e_comment_value);
      if (lookahead != 0 &&
          lookahead != '\n') ADVANCE(68);
      END_STATE();
    default:
      return false;
  }
}

static const TSLexMode ts_lex_modes[STATE_COUNT] = {
  [0] = {.lex_state = 0},
  [1] = {.lex_state = 26},
  [2] = {.lex_state = 1},
  [3] = {.lex_state = 1},
  [4] = {.lex_state = 1},
  [5] = {.lex_state = 0},
  [6] = {.lex_state = 0},
  [7] = {.lex_state = 0},
  [8] = {.lex_state = 0},
  [9] = {.lex_state = 1},
  [10] = {.lex_state = 1},
  [11] = {.lex_state = 1},
  [12] = {.lex_state = 1},
  [13] = {.lex_state = 1},
  [14] = {.lex_state = 1},
  [15] = {.lex_state = 1},
  [16] = {.lex_state = 1},
  [17] = {.lex_state = 1},
  [18] = {.lex_state = 1},
  [19] = {.lex_state = 1},
  [20] = {.lex_state = 0},
  [21] = {.lex_state = 2},
  [22] = {.lex_state = 2},
  [23] = {.lex_state = 0},
  [24] = {.lex_state = 0},
  [25] = {.lex_state = 26},
  [26] = {.lex_state = 2},
  [27] = {.lex_state = 26},
  [28] = {.lex_state = 0},
  [29] = {.lex_state = 0},
  [30] = {.lex_state = 2},
  [31] = {.lex_state = 0},
  [32] = {.lex_state = 0},
  [33] = {.lex_state = 0},
  [34] = {.lex_state = 0},
  [35] = {.lex_state = 0},
  [36] = {.lex_state = 0},
  [37] = {.lex_state = 0},
  [38] = {.lex_state = 0},
  [39] = {.lex_state = 0},
  [40] = {.lex_state = 0},
  [41] = {.lex_state = 0},
  [42] = {.lex_state = 0},
  [43] = {.lex_state = 0},
  [44] = {.lex_state = 0},
  [45] = {.lex_state = 0},
  [46] = {.lex_state = 0},
  [47] = {.lex_state = 0},
  [48] = {.lex_state = 0},
  [49] = {.lex_state = 26},
  [50] = {.lex_state = 26},
  [51] = {.lex_state = 0},
  [52] = {.lex_state = 0},
  [53] = {.lex_state = 26},
  [54] = {.lex_state = 1},
  [55] = {.lex_state = 1},
  [56] = {.lex_state = 1},
  [57] = {.lex_state = 0},
  [58] = {.lex_state = 0},
  [59] = {.lex_state = 67},
  [60] = {.lex_state = 0},
  [61] = {.lex_state = 0},
};

static const uint16_t ts_parse_table[LARGE_STATE_COUNT][SYMBOL_COUNT] = {
  [0] = {
    [ts_builtin_sym_end] = ACTIONS(1),
    [anon_sym_LBRACE] = ACTIONS(1),
    [anon_sym_COMMA] = ACTIONS(1),
    [anon_sym_RBRACE] = ACTIONS(1),
    [anon_sym_COLON] = ACTIONS(1),
    [anon_sym_LBRACK] = ACTIONS(1),
    [anon_sym_RBRACK] = ACTIONS(1),
    [anon_sym_DQUOTE] = ACTIONS(1),
    [sym_escape_sequence] = ACTIONS(1),
    [sym_number] = ACTIONS(1),
    [sym_true] = ACTIONS(1),
    [sym_false] = ACTIONS(1),
    [sym_null] = ACTIONS(1),
    [aux_sym_e_comment_token1] = ACTIONS(1),
  },
  [1] = {
    [sym_document] = STATE(58),
    [sym_e_command] = STATE(27),
    [aux_sym_document_repeat1] = STATE(27),
    [ts_builtin_sym_end] = ACTIONS(3),
    [sym_e_command_name] = ACTIONS(5),
  },
  [2] = {
    [sym__value] = STATE(12),
    [sym_object] = STATE(9),
    [sym_array] = STATE(9),
    [sym_string] = STATE(9),
    [sym_e_parameter] = STATE(4),
    [sym_e_json_value] = STATE(10),
    [sym_e_comment] = STATE(56),
    [aux_sym_e_command_repeat1] = STATE(4),
    [anon_sym_LBRACE] = ACTIONS(7),
    [anon_sym_LBRACK] = ACTIONS(9),
    [anon_sym_DQUOTE] = ACTIONS(11),
    [sym_number] = ACTIONS(13),
    [sym_true] = ACTIONS(13),
    [sym_false] = ACTIONS(13),
    [sym_null] = ACTIONS(13),
    [sym_e_simple_value] = ACTIONS(15),
    [aux_sym_e_command_token1] = ACTIONS(17),
    [aux_sym_e_comment_token1] = ACTIONS(19),
  },
  [3] = {
    [sym__value] = STATE(12),
    [sym_object] = STATE(9),
    [sym_array] = STATE(9),
    [sym_string] = STATE(9),
    [sym_e_parameter] = STATE(2),
    [sym_e_json_value] = STATE(10),
    [sym_e_comment] = STATE(55),
    [aux_sym_e_command_repeat1] = STATE(2),
    [anon_sym_LBRACE] = ACTIONS(7),
    [anon_sym_LBRACK] = ACTIONS(9),
    [anon_sym_DQUOTE] = ACTIONS(11),
    [sym_number] = ACTIONS(13),
    [sym_true] = ACTIONS(13),
    [sym_false] = ACTIONS(13),
    [sym_null] = ACTIONS(13),
    [sym_e_simple_value] = ACTIONS(15),
    [aux_sym_e_command_token1] = ACTIONS(21),
    [aux_sym_e_comment_token1] = ACTIONS(19),
  },
};

static const uint16_t ts_small_parse_table[] = {
  [0] = 11,
    ACTIONS(23), 1,
      anon_sym_LBRACE,
    ACTIONS(26), 1,
      anon_sym_LBRACK,
    ACTIONS(29), 1,
      anon_sym_DQUOTE,
    ACTIONS(35), 1,
      sym_e_simple_value,
    ACTIONS(38), 1,
      aux_sym_e_command_token1,
    ACTIONS(40), 1,
      aux_sym_e_comment_token1,
    STATE(10), 1,
      sym_e_json_value,
    STATE(12), 1,
      sym__value,
    STATE(4), 2,
      sym_e_parameter,
      aux_sym_e_command_repeat1,
    STATE(9), 3,
      sym_object,
      sym_array,
      sym_string,
    ACTIONS(32), 4,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
  [40] = 7,
    ACTIONS(42), 1,
      anon_sym_LBRACE,
    ACTIONS(44), 1,
      anon_sym_LBRACK,
    ACTIONS(46), 1,
      anon_sym_RBRACK,
    ACTIONS(48), 1,
      anon_sym_DQUOTE,
    STATE(39), 1,
      sym__value,
    STATE(45), 3,
      sym_object,
      sym_array,
      sym_string,
    ACTIONS(50), 4,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
  [67] = 7,
    ACTIONS(42), 1,
      anon_sym_LBRACE,
    ACTIONS(44), 1,
      anon_sym_LBRACK,
    ACTIONS(48), 1,
      anon_sym_DQUOTE,
    ACTIONS(52), 1,
      anon_sym_RBRACK,
    STATE(34), 1,
      sym__value,
    STATE(45), 3,
      sym_object,
      sym_array,
      sym_string,
    ACTIONS(50), 4,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
  [94] = 6,
    ACTIONS(42), 1,
      anon_sym_LBRACE,
    ACTIONS(44), 1,
      anon_sym_LBRACK,
    ACTIONS(48), 1,
      anon_sym_DQUOTE,
    STATE(48), 1,
      sym__value,
    STATE(45), 3,
      sym_object,
      sym_array,
      sym_string,
    ACTIONS(50), 4,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
  [118] = 6,
    ACTIONS(42), 1,
      anon_sym_LBRACE,
    ACTIONS(44), 1,
      anon_sym_LBRACK,
    ACTIONS(48), 1,
      anon_sym_DQUOTE,
    STATE(52), 1,
      sym__value,
    STATE(45), 3,
      sym_object,
      sym_array,
      sym_string,
    ACTIONS(50), 4,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
  [142] = 2,
    ACTIONS(56), 1,
      aux_sym_e_command_token1,
    ACTIONS(54), 9,
      anon_sym_LBRACE,
      anon_sym_LBRACK,
      anon_sym_DQUOTE,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
      sym_e_simple_value,
      aux_sym_e_comment_token1,
  [157] = 2,
    ACTIONS(60), 1,
      aux_sym_e_command_token1,
    ACTIONS(58), 9,
      anon_sym_LBRACE,
      anon_sym_LBRACK,
      anon_sym_DQUOTE,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
      sym_e_simple_value,
      aux_sym_e_comment_token1,
  [172] = 2,
    ACTIONS(64), 1,
      aux_sym_e_command_token1,
    ACTIONS(62), 9,
      anon_sym_LBRACE,
      anon_sym_LBRACK,
      anon_sym_DQUOTE,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
      sym_e_simple_value,
      aux_sym_e_comment_token1,
  [187] = 2,
    ACTIONS(68), 1,
      aux_sym_e_command_token1,
    ACTIONS(66), 9,
      anon_sym_LBRACE,
      anon_sym_LBRACK,
      anon_sym_DQUOTE,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
      sym_e_simple_value,
      aux_sym_e_comment_token1,
  [202] = 2,
    ACTIONS(72), 1,
      aux_sym_e_command_token1,
    ACTIONS(70), 9,
      anon_sym_LBRACE,
      anon_sym_LBRACK,
      anon_sym_DQUOTE,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
      sym_e_simple_value,
      aux_sym_e_comment_token1,
  [217] = 2,
    ACTIONS(76), 1,
      aux_sym_e_command_token1,
    ACTIONS(74), 9,
      anon_sym_LBRACE,
      anon_sym_LBRACK,
      anon_sym_DQUOTE,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
      sym_e_simple_value,
      aux_sym_e_comment_token1,
  [232] = 2,
    ACTIONS(80), 1,
      aux_sym_e_command_token1,
    ACTIONS(78), 9,
      anon_sym_LBRACE,
      anon_sym_LBRACK,
      anon_sym_DQUOTE,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
      sym_e_simple_value,
      aux_sym_e_comment_token1,
  [247] = 2,
    ACTIONS(84), 1,
      aux_sym_e_command_token1,
    ACTIONS(82), 9,
      anon_sym_LBRACE,
      anon_sym_LBRACK,
      anon_sym_DQUOTE,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
      sym_e_simple_value,
      aux_sym_e_comment_token1,
  [262] = 2,
    ACTIONS(88), 1,
      aux_sym_e_command_token1,
    ACTIONS(86), 9,
      anon_sym_LBRACE,
      anon_sym_LBRACK,
      anon_sym_DQUOTE,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
      sym_e_simple_value,
      aux_sym_e_comment_token1,
  [277] = 2,
    ACTIONS(92), 1,
      aux_sym_e_command_token1,
    ACTIONS(90), 9,
      anon_sym_LBRACE,
      anon_sym_LBRACK,
      anon_sym_DQUOTE,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
      sym_e_simple_value,
      aux_sym_e_comment_token1,
  [292] = 2,
    ACTIONS(96), 1,
      aux_sym_e_command_token1,
    ACTIONS(94), 9,
      anon_sym_LBRACE,
      anon_sym_LBRACK,
      anon_sym_DQUOTE,
      sym_number,
      sym_true,
      sym_false,
      sym_null,
      sym_e_simple_value,
      aux_sym_e_comment_token1,
  [307] = 5,
    ACTIONS(48), 1,
      anon_sym_DQUOTE,
    ACTIONS(98), 1,
      anon_sym_RBRACE,
    ACTIONS(100), 1,
      sym_number,
    STATE(40), 1,
      sym_pair,
    STATE(61), 1,
      sym_string,
  [323] = 4,
    ACTIONS(102), 1,
      anon_sym_DQUOTE,
    STATE(26), 1,
      aux_sym_string_content_repeat1,
    STATE(60), 1,
      sym_string_content,
    ACTIONS(104), 2,
      aux_sym_string_content_token1,
      sym_escape_sequence,
  [337] = 4,
    ACTIONS(106), 1,
      anon_sym_DQUOTE,
    STATE(26), 1,
      aux_sym_string_content_repeat1,
    STATE(57), 1,
      sym_string_content,
    ACTIONS(104), 2,
      aux_sym_string_content_token1,
      sym_escape_sequence,
  [351] = 5,
    ACTIONS(48), 1,
      anon_sym_DQUOTE,
    ACTIONS(100), 1,
      sym_number,
    ACTIONS(108), 1,
      anon_sym_RBRACE,
    STATE(35), 1,
      sym_pair,
    STATE(61), 1,
      sym_string,
  [367] = 4,
    ACTIONS(48), 1,
      anon_sym_DQUOTE,
    ACTIONS(100), 1,
      sym_number,
    STATE(51), 1,
      sym_pair,
    STATE(61), 1,
      sym_string,
  [380] = 3,
    ACTIONS(110), 1,
      ts_builtin_sym_end,
    ACTIONS(112), 1,
      sym_e_command_name,
    STATE(25), 2,
      sym_e_command,
      aux_sym_document_repeat1,
  [391] = 3,
    ACTIONS(115), 1,
      anon_sym_DQUOTE,
    STATE(30), 1,
      aux_sym_string_content_repeat1,
    ACTIONS(117), 2,
      aux_sym_string_content_token1,
      sym_escape_sequence,
  [402] = 3,
    ACTIONS(5), 1,
      sym_e_command_name,
    ACTIONS(119), 1,
      ts_builtin_sym_end,
    STATE(25), 2,
      sym_e_command,
      aux_sym_document_repeat1,
  [413] = 1,
    ACTIONS(76), 4,
      anon_sym_COMMA,
      anon_sym_RBRACE,
      anon_sym_COLON,
      anon_sym_RBRACK,
  [420] = 1,
    ACTIONS(96), 4,
      anon_sym_COMMA,
      anon_sym_RBRACE,
      anon_sym_COLON,
      anon_sym_RBRACK,
  [427] = 3,
    ACTIONS(121), 1,
      anon_sym_DQUOTE,
    STATE(30), 1,
      aux_sym_string_content_repeat1,
    ACTIONS(123), 2,
      aux_sym_string_content_token1,
      sym_escape_sequence,
  [438] = 1,
    ACTIONS(72), 3,
      anon_sym_COMMA,
      anon_sym_RBRACE,
      anon_sym_RBRACK,
  [444] = 1,
    ACTIONS(64), 3,
      anon_sym_COMMA,
      anon_sym_RBRACE,
      anon_sym_RBRACK,
  [450] = 3,
    ACTIONS(126), 1,
      anon_sym_COMMA,
    ACTIONS(128), 1,
      anon_sym_RBRACE,
    STATE(41), 1,
      aux_sym_object_repeat1,
  [460] = 3,
    ACTIONS(130), 1,
      anon_sym_COMMA,
    ACTIONS(132), 1,
      anon_sym_RBRACK,
    STATE(36), 1,
      aux_sym_array_repeat1,
  [470] = 3,
    ACTIONS(126), 1,
      anon_sym_COMMA,
    ACTIONS(134), 1,
      anon_sym_RBRACE,
    STATE(33), 1,
      aux_sym_object_repeat1,
  [480] = 3,
    ACTIONS(130), 1,
      anon_sym_COMMA,
    ACTIONS(136), 1,
      anon_sym_RBRACK,
    STATE(44), 1,
      aux_sym_array_repeat1,
  [490] = 3,
    ACTIONS(130), 1,
      anon_sym_COMMA,
    ACTIONS(138), 1,
      anon_sym_RBRACK,
    STATE(44), 1,
      aux_sym_array_repeat1,
  [500] = 3,
    ACTIONS(126), 1,
      anon_sym_COMMA,
    ACTIONS(140), 1,
      anon_sym_RBRACE,
    STATE(41), 1,
      aux_sym_object_repeat1,
  [510] = 3,
    ACTIONS(130), 1,
      anon_sym_COMMA,
    ACTIONS(142), 1,
      anon_sym_RBRACK,
    STATE(37), 1,
      aux_sym_array_repeat1,
  [520] = 3,
    ACTIONS(126), 1,
      anon_sym_COMMA,
    ACTIONS(144), 1,
      anon_sym_RBRACE,
    STATE(38), 1,
      aux_sym_object_repeat1,
  [530] = 3,
    ACTIONS(146), 1,
      anon_sym_COMMA,
    ACTIONS(149), 1,
      anon_sym_RBRACE,
    STATE(41), 1,
      aux_sym_object_repeat1,
  [540] = 1,
    ACTIONS(84), 3,
      anon_sym_COMMA,
      anon_sym_RBRACE,
      anon_sym_RBRACK,
  [546] = 1,
    ACTIONS(92), 3,
      anon_sym_COMMA,
      anon_sym_RBRACE,
      anon_sym_RBRACK,
  [552] = 3,
    ACTIONS(151), 1,
      anon_sym_COMMA,
    ACTIONS(154), 1,
      anon_sym_RBRACK,
    STATE(44), 1,
      aux_sym_array_repeat1,
  [562] = 1,
    ACTIONS(56), 3,
      anon_sym_COMMA,
      anon_sym_RBRACE,
      anon_sym_RBRACK,
  [568] = 1,
    ACTIONS(80), 3,
      anon_sym_COMMA,
      anon_sym_RBRACE,
      anon_sym_RBRACK,
  [574] = 1,
    ACTIONS(88), 3,
      anon_sym_COMMA,
      anon_sym_RBRACE,
      anon_sym_RBRACK,
  [580] = 1,
    ACTIONS(154), 2,
      anon_sym_COMMA,
      anon_sym_RBRACK,
  [585] = 1,
    ACTIONS(156), 2,
      ts_builtin_sym_end,
      sym_e_command_name,
  [590] = 1,
    ACTIONS(158), 2,
      ts_builtin_sym_end,
      sym_e_command_name,
  [595] = 1,
    ACTIONS(149), 2,
      anon_sym_COMMA,
      anon_sym_RBRACE,
  [600] = 1,
    ACTIONS(160), 2,
      anon_sym_COMMA,
      anon_sym_RBRACE,
  [605] = 1,
    ACTIONS(162), 2,
      ts_builtin_sym_end,
      sym_e_command_name,
  [610] = 1,
    ACTIONS(164), 1,
      aux_sym_e_command_token1,
  [614] = 1,
    ACTIONS(17), 1,
      aux_sym_e_command_token1,
  [618] = 1,
    ACTIONS(166), 1,
      aux_sym_e_command_token1,
  [622] = 1,
    ACTIONS(168), 1,
      anon_sym_DQUOTE,
  [626] = 1,
    ACTIONS(170), 1,
      ts_builtin_sym_end,
  [630] = 1,
    ACTIONS(172), 1,
      sym_e_comment_value,
  [634] = 1,
    ACTIONS(174), 1,
      anon_sym_DQUOTE,
  [638] = 1,
    ACTIONS(176), 1,
      anon_sym_COLON,
};

static const uint32_t ts_small_parse_table_map[] = {
  [SMALL_STATE(4)] = 0,
  [SMALL_STATE(5)] = 40,
  [SMALL_STATE(6)] = 67,
  [SMALL_STATE(7)] = 94,
  [SMALL_STATE(8)] = 118,
  [SMALL_STATE(9)] = 142,
  [SMALL_STATE(10)] = 157,
  [SMALL_STATE(11)] = 172,
  [SMALL_STATE(12)] = 187,
  [SMALL_STATE(13)] = 202,
  [SMALL_STATE(14)] = 217,
  [SMALL_STATE(15)] = 232,
  [SMALL_STATE(16)] = 247,
  [SMALL_STATE(17)] = 262,
  [SMALL_STATE(18)] = 277,
  [SMALL_STATE(19)] = 292,
  [SMALL_STATE(20)] = 307,
  [SMALL_STATE(21)] = 323,
  [SMALL_STATE(22)] = 337,
  [SMALL_STATE(23)] = 351,
  [SMALL_STATE(24)] = 367,
  [SMALL_STATE(25)] = 380,
  [SMALL_STATE(26)] = 391,
  [SMALL_STATE(27)] = 402,
  [SMALL_STATE(28)] = 413,
  [SMALL_STATE(29)] = 420,
  [SMALL_STATE(30)] = 427,
  [SMALL_STATE(31)] = 438,
  [SMALL_STATE(32)] = 444,
  [SMALL_STATE(33)] = 450,
  [SMALL_STATE(34)] = 460,
  [SMALL_STATE(35)] = 470,
  [SMALL_STATE(36)] = 480,
  [SMALL_STATE(37)] = 490,
  [SMALL_STATE(38)] = 500,
  [SMALL_STATE(39)] = 510,
  [SMALL_STATE(40)] = 520,
  [SMALL_STATE(41)] = 530,
  [SMALL_STATE(42)] = 540,
  [SMALL_STATE(43)] = 546,
  [SMALL_STATE(44)] = 552,
  [SMALL_STATE(45)] = 562,
  [SMALL_STATE(46)] = 568,
  [SMALL_STATE(47)] = 574,
  [SMALL_STATE(48)] = 580,
  [SMALL_STATE(49)] = 585,
  [SMALL_STATE(50)] = 590,
  [SMALL_STATE(51)] = 595,
  [SMALL_STATE(52)] = 600,
  [SMALL_STATE(53)] = 605,
  [SMALL_STATE(54)] = 610,
  [SMALL_STATE(55)] = 614,
  [SMALL_STATE(56)] = 618,
  [SMALL_STATE(57)] = 622,
  [SMALL_STATE(58)] = 626,
  [SMALL_STATE(59)] = 630,
  [SMALL_STATE(60)] = 634,
  [SMALL_STATE(61)] = 638,
};

static const TSParseActionEntry ts_parse_actions[] = {
  [0] = {.entry = {.count = 0, .reusable = false}},
  [1] = {.entry = {.count = 1, .reusable = false}}, RECOVER(),
  [3] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_document, 0, 0, 0),
  [5] = {.entry = {.count = 1, .reusable = true}}, SHIFT(3),
  [7] = {.entry = {.count = 1, .reusable = false}}, SHIFT(23),
  [9] = {.entry = {.count = 1, .reusable = false}}, SHIFT(6),
  [11] = {.entry = {.count = 1, .reusable = false}}, SHIFT(22),
  [13] = {.entry = {.count = 1, .reusable = false}}, SHIFT(9),
  [15] = {.entry = {.count = 1, .reusable = false}}, SHIFT(10),
  [17] = {.entry = {.count = 1, .reusable = true}}, SHIFT(50),
  [19] = {.entry = {.count = 1, .reusable = false}}, SHIFT(59),
  [21] = {.entry = {.count = 1, .reusable = true}}, SHIFT(49),
  [23] = {.entry = {.count = 2, .reusable = false}}, REDUCE(aux_sym_e_command_repeat1, 2, 0, 0), SHIFT_REPEAT(23),
  [26] = {.entry = {.count = 2, .reusable = false}}, REDUCE(aux_sym_e_command_repeat1, 2, 0, 0), SHIFT_REPEAT(6),
  [29] = {.entry = {.count = 2, .reusable = false}}, REDUCE(aux_sym_e_command_repeat1, 2, 0, 0), SHIFT_REPEAT(22),
  [32] = {.entry = {.count = 2, .reusable = false}}, REDUCE(aux_sym_e_command_repeat1, 2, 0, 0), SHIFT_REPEAT(9),
  [35] = {.entry = {.count = 2, .reusable = false}}, REDUCE(aux_sym_e_command_repeat1, 2, 0, 0), SHIFT_REPEAT(10),
  [38] = {.entry = {.count = 1, .reusable = true}}, REDUCE(aux_sym_e_command_repeat1, 2, 0, 0),
  [40] = {.entry = {.count = 1, .reusable = false}}, REDUCE(aux_sym_e_command_repeat1, 2, 0, 0),
  [42] = {.entry = {.count = 1, .reusable = true}}, SHIFT(20),
  [44] = {.entry = {.count = 1, .reusable = true}}, SHIFT(5),
  [46] = {.entry = {.count = 1, .reusable = true}}, SHIFT(47),
  [48] = {.entry = {.count = 1, .reusable = true}}, SHIFT(21),
  [50] = {.entry = {.count = 1, .reusable = true}}, SHIFT(45),
  [52] = {.entry = {.count = 1, .reusable = true}}, SHIFT(17),
  [54] = {.entry = {.count = 1, .reusable = false}}, REDUCE(sym__value, 1, 0, 0),
  [56] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym__value, 1, 0, 0),
  [58] = {.entry = {.count = 1, .reusable = false}}, REDUCE(sym_e_parameter, 1, 0, 0),
  [60] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_e_parameter, 1, 0, 0),
  [62] = {.entry = {.count = 1, .reusable = false}}, REDUCE(sym_array, 4, 0, 0),
  [64] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_array, 4, 0, 0),
  [66] = {.entry = {.count = 1, .reusable = false}}, REDUCE(sym_e_json_value, 1, 0, 0),
  [68] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_e_json_value, 1, 0, 0),
  [70] = {.entry = {.count = 1, .reusable = false}}, REDUCE(sym_object, 4, 0, 0),
  [72] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_object, 4, 0, 0),
  [74] = {.entry = {.count = 1, .reusable = false}}, REDUCE(sym_string, 3, 0, 0),
  [76] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_string, 3, 0, 0),
  [78] = {.entry = {.count = 1, .reusable = false}}, REDUCE(sym_object, 2, 0, 0),
  [80] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_object, 2, 0, 0),
  [82] = {.entry = {.count = 1, .reusable = false}}, REDUCE(sym_array, 3, 0, 0),
  [84] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_array, 3, 0, 0),
  [86] = {.entry = {.count = 1, .reusable = false}}, REDUCE(sym_array, 2, 0, 0),
  [88] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_array, 2, 0, 0),
  [90] = {.entry = {.count = 1, .reusable = false}}, REDUCE(sym_object, 3, 0, 0),
  [92] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_object, 3, 0, 0),
  [94] = {.entry = {.count = 1, .reusable = false}}, REDUCE(sym_string, 2, 0, 0),
  [96] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_string, 2, 0, 0),
  [98] = {.entry = {.count = 1, .reusable = true}}, SHIFT(46),
  [100] = {.entry = {.count = 1, .reusable = true}}, SHIFT(61),
  [102] = {.entry = {.count = 1, .reusable = false}}, SHIFT(29),
  [104] = {.entry = {.count = 1, .reusable = true}}, SHIFT(26),
  [106] = {.entry = {.count = 1, .reusable = false}}, SHIFT(19),
  [108] = {.entry = {.count = 1, .reusable = true}}, SHIFT(15),
  [110] = {.entry = {.count = 1, .reusable = true}}, REDUCE(aux_sym_document_repeat1, 2, 0, 0),
  [112] = {.entry = {.count = 2, .reusable = true}}, REDUCE(aux_sym_document_repeat1, 2, 0, 0), SHIFT_REPEAT(3),
  [115] = {.entry = {.count = 1, .reusable = false}}, REDUCE(sym_string_content, 1, 0, 0),
  [117] = {.entry = {.count = 1, .reusable = true}}, SHIFT(30),
  [119] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_document, 1, 0, 0),
  [121] = {.entry = {.count = 1, .reusable = false}}, REDUCE(aux_sym_string_content_repeat1, 2, 0, 0),
  [123] = {.entry = {.count = 2, .reusable = true}}, REDUCE(aux_sym_string_content_repeat1, 2, 0, 0), SHIFT_REPEAT(30),
  [126] = {.entry = {.count = 1, .reusable = true}}, SHIFT(24),
  [128] = {.entry = {.count = 1, .reusable = true}}, SHIFT(13),
  [130] = {.entry = {.count = 1, .reusable = true}}, SHIFT(7),
  [132] = {.entry = {.count = 1, .reusable = true}}, SHIFT(16),
  [134] = {.entry = {.count = 1, .reusable = true}}, SHIFT(18),
  [136] = {.entry = {.count = 1, .reusable = true}}, SHIFT(11),
  [138] = {.entry = {.count = 1, .reusable = true}}, SHIFT(32),
  [140] = {.entry = {.count = 1, .reusable = true}}, SHIFT(31),
  [142] = {.entry = {.count = 1, .reusable = true}}, SHIFT(42),
  [144] = {.entry = {.count = 1, .reusable = true}}, SHIFT(43),
  [146] = {.entry = {.count = 2, .reusable = true}}, REDUCE(aux_sym_object_repeat1, 2, 0, 0), SHIFT_REPEAT(24),
  [149] = {.entry = {.count = 1, .reusable = true}}, REDUCE(aux_sym_object_repeat1, 2, 0, 0),
  [151] = {.entry = {.count = 2, .reusable = true}}, REDUCE(aux_sym_array_repeat1, 2, 0, 0), SHIFT_REPEAT(7),
  [154] = {.entry = {.count = 1, .reusable = true}}, REDUCE(aux_sym_array_repeat1, 2, 0, 0),
  [156] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_e_command, 2, 0, 0),
  [158] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_e_command, 3, 0, 0),
  [160] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_pair, 3, 0, 1),
  [162] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_e_command, 4, 0, 0),
  [164] = {.entry = {.count = 1, .reusable = true}}, REDUCE(sym_e_comment, 2, 0, 0),
  [166] = {.entry = {.count = 1, .reusable = true}}, SHIFT(53),
  [168] = {.entry = {.count = 1, .reusable = true}}, SHIFT(14),
  [170] = {.entry = {.count = 1, .reusable = true}},  ACCEPT_INPUT(),
  [172] = {.entry = {.count = 1, .reusable = true}}, SHIFT(54),
  [174] = {.entry = {.count = 1, .reusable = true}}, SHIFT(28),
  [176] = {.entry = {.count = 1, .reusable = true}}, SHIFT(8),
};

#ifdef __cplusplus
extern "C" {
#endif
#ifdef TREE_SITTER_HIDE_SYMBOLS
#define TS_PUBLIC
#elif defined(_WIN32)
#define TS_PUBLIC __declspec(dllexport)
#else
#define TS_PUBLIC __attribute__((visibility("default")))
#endif

TS_PUBLIC const TSLanguage *tree_sitter_ecmd(void) {
  static const TSLanguage language = {
    .version = LANGUAGE_VERSION,
    .symbol_count = SYMBOL_COUNT,
    .alias_count = ALIAS_COUNT,
    .token_count = TOKEN_COUNT,
    .external_token_count = EXTERNAL_TOKEN_COUNT,
    .state_count = STATE_COUNT,
    .large_state_count = LARGE_STATE_COUNT,
    .production_id_count = PRODUCTION_ID_COUNT,
    .field_count = FIELD_COUNT,
    .max_alias_sequence_length = MAX_ALIAS_SEQUENCE_LENGTH,
    .parse_table = &ts_parse_table[0][0],
    .small_parse_table = ts_small_parse_table,
    .small_parse_table_map = ts_small_parse_table_map,
    .parse_actions = ts_parse_actions,
    .symbol_names = ts_symbol_names,
    .field_names = ts_field_names,
    .field_map_slices = ts_field_map_slices,
    .field_map_entries = ts_field_map_entries,
    .symbol_metadata = ts_symbol_metadata,
    .public_symbol_map = ts_symbol_map,
    .alias_map = ts_non_terminal_alias_map,
    .alias_sequences = &ts_alias_sequences[0][0],
    .lex_modes = ts_lex_modes,
    .lex_fn = ts_lex,
    .primary_state_ids = ts_primary_state_ids,
  };
  return &language;
}
#ifdef __cplusplus
}
#endif
