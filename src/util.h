#ifndef UTIL_H_
#define UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "fxlib.h"
#ifdef __cplusplus
}
#endif

#define MSG_ENOMEM "Out of memory"

#define TOP 0
#define LEFT 0
#define BOTTOM 63
#define RIGHT 127
#define EVAL(x) x
#define SCND_ARG(x, y) y
#define FIRST_ARG(x, y) x
#define ARR_LEN(arr) (sizeof(arr)/sizeof((arr)[0]))
#define MAX(dest, a, b) do {\
		if (a > b) *dest = a; else *dest = b; \
	while (0)

#define MIN(dest, a, b) do {\
		if (a < b) *dest = a; else *dest = b; \
	while (0)

#define DEBUG

#ifdef DEBUG
#define dbg_print(msg) do {\
	dbg_print_xy((msg), 1, 1);\
} while(0)

#define dbg_print_xy(msg, x, y) do {\
	locate((x), (y));\
	Print((unsigned char*) (msg));\
} while(0)

#define dbg_printf(ignore) do {\
	dbg_print_xy(dbg_buf, 1, 1);\
} while(0)

#define dbg_printf_xy(x, y) do {\
	locate((x), (y));\
	Print((unsigned char*) dbg_buf);\
} while(0)

#define wait_for_key(times) do {\
	unsigned int key;\
	for (size_t i = 0; i < (times); ++i)\
		GetKey(&(key));\
} while(0)
extern char dbg_buf[256];

#else
#define dbg_print(msg)
#define dbg_print_xy(msg, x, y)
#define dbg_printf(ignore)
#define dbg_printf_xy(x, y)
#define wait_for_key(times)
#endif

// pixel coordinates 
typedef struct {
	unsigned char x, y;
} point_t;

// coordinates of a character in the character grid
typedef point_t char_point_t;
// variation on display_error that quit the entire application when the dialog
// is closed
void display_fatal_error(const char* msg);
// can be exited with EXIT (function only returns then)
void display_error(const char* msg);
/**
 * draws a 3px high separator horizontal line over the entire widht of the
 * screen, with its last line having y = bottom_px
 */
void draw_separator(int bottom_px);

extern char tmp_buf[256];


#endif
