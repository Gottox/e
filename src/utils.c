#include <e_utils.h>
#include <stdlib.h>
#include <string.h>

int
e_array_push(void **array, int *size, void *value) {
	size_t new_size = *size + 1;
	void **new_array = realloc(*array, new_size * sizeof(void *));
	if (!new_array) {
		return -1; // Memory allocation failed
	}
	new_array[*size] = value;
	*array = new_array;
	*size = new_size;
	return 0;
}

int
e_array_remove(void **array, int *size, int index) {
	if (index < 0 || index >= *size || *size == 0) {
		return -1; // Index out of bounds
	}
	size_t move_size = (*size - index - 1) * sizeof(void *);
	memmove(&array[index], &array[index + 1], move_size);
	*size -= 1;
	return 0;
}
