#ifndef BT_CHAR_H_
#define BT_CHAR_H_

#include <stdlib.h>

typedef char bt_char_t;

/**
 * returns 1 if i is after the last char of s. Either s must be NULL, or if i is
 * 0, it must be within the allocated memory, and if i is > 0, i - 1 must be
 * within the allocated memory
 */
static inline int is_line_end(const bt_char_t* s, size_t i) {
	if (s == NULL) {
		return 1;
	} else if (i > 0 && (s[i - 1] & 0x80)) {
		// last char was end of capacity
		return 1;
	} else if ((s[i] & (char) ~0x80) == '\0') {
		return 1;
	} else {
		return 0;
	}
}

int bt_strcmp(const bt_char_t* s1, const bt_char_t* s2);

#endif // BT_CHAR_H_
