#include <stdint.h>

struct CharacterList {
	uint32_t code_point;
	struct CharacterList *next;
};
