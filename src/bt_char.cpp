#include "bt_char.h"
#include "line_utils.h"

int bt_strcmp(const bt_char_t* s1, const bt_char_t* s2) {
	for (size_t i = 0;; ++i) {
		int res1 = is_line_end(s1, i);
		int res2 = is_line_end(s2, i);
		if (res1) {
			if (res2) {
				return 0;
			} else {
				return -1;
			}
		} else if (res2) {
			return 1;
		}
		char c1 = (char) ~0x80 & s1[i];
		char c2 = (char) ~0x80 & s2[i];

		if (c1 != c2) {
			return c1 - c2;
		}
	}
}
