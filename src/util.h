#ifndef UTIL_H_
#define UTIL_H_

#define TOP 0
#define LEFT 0
#define BOTTOM 63
#define RIGHT 127

// pixel coordinates 
typedef struct {
	unsigned char x, y;
} point_t;

// coordinates of a character in the character grid
typedef point_t char_point_t;
void display_error(const char* msg, int n);

#endif
