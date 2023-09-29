#include <setjmp.h>
#include <stddef.h>
#include <keybios.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "dispbios.h"
#include "fxlib.h"
#ifdef __cplusplus
}
#endif
#include "main.h"
#include "util.h"

#ifdef DEBUG
char dbg_buf[256];
#endif
char tmp_buf[256];

void display_error(const char* msg) {
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
	Bdisp_AllClr_DDVRAM();
}

void display_fatal_error(const char* msg) {
	display_error(msg);
	longjmp(error_jmp, 1);
}

void draw_separator(int bottom_px) {
	DISPBOX line;
	line.left = 0;
	line.right = 127;
	line.top = bottom_px - 2;
	line.bottom = bottom_px;
	Bdisp_AreaClr_VRAM(&line);
	Bdisp_DrawLineVRAM(0, bottom_px - 1, 127, bottom_px - 1);
	Bdisp_PutDispArea_DD(&line);
}
