#include <ctype.h>
#include <jw.h>
#include <jw_quickjs.h>
#include <quickjs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
pascal_case(char *str) {
	int j = 0;

	str[0] = toupper(str[0]);
	for (int i = 0; str[i]; i++, j++) {
		if (str[i] == '_' || str[i] == '/') {
			i++;
			if (str[i]) {
				str[j] = toupper(str[i]);
			}
		} else {
			str[j] = str[i];
		}
	}
	str[j] = '\0';
}

__attribute__((warn_unused_result)) char *
snake_case(const char *str) {
	int result_len = strlen(str) + 1;
	for (int i = 1; str[i]; i++) {
		if (isupper(str[i])) {
			result_len++;
		}
	}
	char *result = calloc(result_len, sizeof(char));
	if (result_len > 1) {
		result[0] = tolower(str[0]);
	}
	int j = 1;
	for (int i = 1; str[i]; i++, j++) {
		if (isupper(str[i])) {
			result[j++] = '_';
			result[j] = tolower(str[i]);
		} else if (str[i] == '/') {
			result[j] = '_';
		} else {
			result[j] = str[i];
		}
	}
	return result;
}

__attribute__((warn_unused_result)) char *
upper_snake_case(const char *str) {
	char *snake = snake_case(str);
	for (int i = 0; snake[i]; i++) {
		snake[i] = toupper(snake[i]);
	}
	return snake;
}
