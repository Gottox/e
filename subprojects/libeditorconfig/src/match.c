#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum MatchResult { MATCH_ERROR = -1, MATCH_NONE = 0, MATCH_OK = 1 };

static int match_brackets(
		const char **pat, size_t *pat_len, const char **str, size_t *str_len);
static int match_number_range(
		const char **pat, size_t *pat_len, const char **str, size_t *str_len);
static int match_group(
		const char **pat, size_t *pat_len, const char **str, size_t *str_len);
static int
match(const char **pat, size_t *pat_len, const char **str, size_t *str_len);

static int
match_literal(
		const char **pat, size_t *pat_len, const char **str, size_t *str_len) {
	if (*pat_len == 0 || *str_len == 0 || **pat != **str) {
		return MATCH_NONE;
	}
	(*pat)++;
	(*pat_len)--;
	(*str)++;
	(*str_len)--;
	return MATCH_OK;
}

static int
match_question(
		const char **pat, size_t *pat_len, const char **str, size_t *str_len) {
	if (*str_len == 0) {
		return MATCH_NONE;
	}
	(*pat)++;
	(*pat_len)--;
	(*str)++;
	(*str_len)--;
	return MATCH_OK;
}

static int
match_star(
		const char **pat, size_t *pat_len, const char **str, size_t *str_len) {
	int is_double = (*pat_len >= 2 && (*pat)[1] == '*');

	if (is_double) {
		*pat += 2;
		*pat_len -= 2;
	} else {
		(*pat)++;
		(*pat_len)--;
	}

	while (1) {
		const char *tmp_pat = *pat;
		size_t tmp_pat_len = *pat_len;
		const char *tmp_str = *str;
		size_t tmp_str_len = *str_len;

		int rv = match(&tmp_pat, &tmp_pat_len, &tmp_str, &tmp_str_len);
		if (rv == MATCH_OK) {
			*pat = tmp_pat;
			*pat_len = tmp_pat_len;
			*str = tmp_str;
			*str_len = tmp_str_len;
			return MATCH_OK;
		} else if (rv == MATCH_ERROR) {
			return MATCH_ERROR;
		}

		if (*str_len == 0 || (!is_double && **str == '/')) {
			break;
		}

		(*str)++;
		(*str_len)--;
	}

	return MATCH_NONE;
}

static int
match_brackets(
		const char **pat, size_t *pat_len, const char **str, size_t *str_len) {
	if (*str_len == 0 || *pat_len == 0) {
		return MATCH_NONE;
	}

	const char *p = *pat;
	size_t len = *pat_len;
	int negate = 0;
	int matched = 0;

	if (*p == '!') {
		negate = 1;
		p++;
		len--;
	}

	while (len > 0 && *p != ']') {
		if (len >= 3 && p[1] == '-' && p[2] != ']') {
			char start = p[0];
			char end = p[2];
			if (**str >= start && **str <= end) {
				matched = 1;
			}
			p += 3;
			len -= 3;
		} else {
			if (**str == *p) {
				matched = 1;
			}
			p++;
			len--;
		}
	}

	if (len == 0 || *p != ']') {
		return MATCH_ERROR;
	}

	p++;
	len--;

	*pat = p;
	*pat_len = len;
	(*str)++;
	(*str_len)--;

	return (negate ? !matched : matched) ? MATCH_OK : MATCH_NONE;
}

static int
match_number_range(
		const char **pat, size_t *pat_len, const char **str, size_t *str_len) {
	int begin, end, consumed;
	if (*pat_len < 5 || **pat != '{') {
		return MATCH_ERROR;
	}

	if (sscanf(*pat, "{%d..%d}%n", &begin, &end, &consumed) != 2) {
		return MATCH_ERROR;
	}

	if ((size_t)consumed > *pat_len) {
		return MATCH_ERROR;
	}

	const char *s = *str;
	size_t s_len = *str_len;
	char num_buf[32];

	for (int i = begin; i <= end; i++) {
		int len = snprintf(num_buf, sizeof(num_buf), "%d", i);
		if ((size_t)len <= s_len && strncmp(num_buf, s, (size_t)len) == 0) {
			*str += len;
			*str_len -= len;
			*pat += consumed;
			*pat_len -= consumed;
			return MATCH_OK;
		}
	}

	return MATCH_NONE;
}

static int
match_group(
		const char **pat, size_t *pat_len, const char **str, size_t *str_len) {
	if (*pat_len == 0 || **pat != '{') {
		return MATCH_ERROR;
	}

	const char *start = *pat + 1;
	size_t len = *pat_len - 1;
	int depth = 1;
	size_t group_len = 0;

	while (group_len < len && depth > 0) {
		if (start[group_len] == '{') {
			depth++;
		} else if (start[group_len] == '}') {
			depth--;
		}
		group_len++;
	}

	if (depth != 0) {
		return MATCH_ERROR;
	}

	const char *group_end = start + group_len - 1;
	const char *alt_start = start;
	const char *alt = start;

	while (alt <= group_end) {
		int sub_depth = 1;
		const char *alt_end = alt;

		while (alt_end <= group_end && *alt_end != ',') {
			if (*alt_end == '{') {
				sub_depth++;
			} else if (*alt_end == '}') {
				sub_depth--;
				if (sub_depth == 0) {
					break; // end of the current group
				}
			}
			alt_end++;
		}

		const char *sub_pat = alt_start;
		size_t sub_pat_len = (size_t)(alt_end - alt_start);
		const char *sub_str = *str;
		size_t sub_str_len = *str_len;

		int res = match(&sub_pat, &sub_pat_len, &sub_str, &sub_str_len);
		if (res == MATCH_ERROR) {
			return MATCH_ERROR;
		} else if (sub_pat_len == 0) {
			// Either MATCH_OK or MATCH_NONE can lead to a successful match, so
			// we check insted if the sub pattern was consumed.
			*pat_len -= (group_end - *pat + 1);
			*pat = group_end + 1;
			*str = sub_str;
			*str_len = sub_str_len;
			return MATCH_OK;
		}

		alt_start = alt_end + 1;
		alt = alt_start;
	}

	return MATCH_NONE;
}

int
match(const char **pat, size_t *pat_len, const char **str, size_t *str_len) {
	int rv;
	while (*pat_len > 0) {
		rv = MATCH_ERROR;

		switch (**pat) {
		case '*':
			rv = match_star(pat, pat_len, str, str_len);
			break;
		case '?':
			rv = match_question(pat, pat_len, str, str_len);
			break;
		case '[':
			(*pat)++;
			(*pat_len)--;
			rv = match_brackets(pat, pat_len, str, str_len);
			break;
		case '{':
			rv = match_number_range(pat, pat_len, str, str_len);
			if (rv == MATCH_ERROR) {
				rv = match_group(pat, pat_len, str, str_len);
			}
			break;
		default:
			rv = match_literal(pat, pat_len, str, str_len);
			break;
		}

		if (rv == MATCH_ERROR) {
			return MATCH_ERROR;
		} else if (rv == MATCH_NONE) {
			return MATCH_NONE;
		}

		// continue matching next segment
	}

	// trailing * may still be valid
	while (*pat_len > 0 && **pat == '*') {
		(*pat)++;
		(*pat_len)--;
	}

	return (*pat_len == 0 && *str_len == 0) ? MATCH_OK : MATCH_NONE;
}

int
editorconfig_match(const char *pat, const char *str, size_t str_len) {
	size_t pat_len = strlen(pat);
	enum MatchResult rv = match(&pat, &pat_len, &str, &str_len);
	// if (rv == MATCH_OK) {
	//	assert(pat_len == 0);
	//	assert(str_len == 0);
	// }
	return rv;
}
