#ifndef E_MESSAGE_H
#define E_MESSAGE_H

#include "e_common.h"

struct EMessage {
	uint64_t sender_id;
	struct Rope content;
};

#endif /* E_MESSAGE_H */
