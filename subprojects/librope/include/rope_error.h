#ifndef ROPE_ERROR_H
#define ROPE_ERROR_H

enum RopeError {
	ROPE_SUCCESS = 0,

	// Let the error codes start from 512 to avoid conflichts with errno.h
	ROPE_ERROR_START = 1 << 9,
	ROPE_ERROR_OOM,
	ROPE_ERROR_INVALID_TYPE,
	ROPE_ERROR_OOB,
};

#endif /* ROPE_ERROR_H */
