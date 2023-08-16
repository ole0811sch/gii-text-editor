#ifdef __cplusplus
extern "C" {
#endif
#include "fxlib.h"
#ifdef __cplusplus
}
#endif
#include <setjmp.h>
#include "main.h"

void display_error(const char* msg, int n) {
	PopUpWin(6);
	locate(2, 2);
	unsigned char char_str[2];
	char_str[1] = '\0';
	unsigned char x = 3;
	unsigned char y = 2;
	for (size_t char_i = 0; msg[char_i]; ++char_i) {
		if (msg[char_i] == '\n') {
			++y;
			x = 3;
			continue;
		}
		if (x >= 20) {
			++y;
			x = 3;
		}
		locate(x, y);
		char_str[0] = msg[char_i];
		Print(char_str);
		++x;
	}
	unsigned int key;
	while ((GetKey(&key), key != KEY_CTRL_EXIT));
	longjmp(error_jmp, 1);
}
