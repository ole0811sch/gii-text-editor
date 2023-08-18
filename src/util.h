#ifndef UTIL_H_
#define UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "fxlib.h"
#ifdef __cplusplus
}
#endif

#define TOP 0
#define LEFT 0
#define BOTTOM 63
#define RIGHT 127

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
void display_error(const char* msg);

extern char dbg_buf[256];

#endif
