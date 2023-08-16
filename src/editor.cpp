#include <stdio.h>
#include <math.h>

#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "fxlib.h"
#include "keybios.h"
#ifdef __cplusplus
}
#endif

#include "font.h"
#include "editor.h"
#include "dyn_arrs.h"

#define CODE_UP -3
#define CODE_DOWN -4
#define CODE_LEFT -5
#define CODE_RIGHT -6

// static function declarations
static void 
print_char_xy(unsigned char x, unsigned char y, char c, char negative);
static void print_char(char_point_t point, char c, char negative);
static void initialize_lines(const char* str);
static void print_lines();
static void print_chars(unsigned char x, unsigned char y, const char* str);



static char capitalization_on = 0;
static char capitalization_on_locked = 0;
typedef struct {
	size_t line;
	size_t column;
} line_col_t;
static line_col_t cursor_pos = { 0, 0 };
static unsigned char cursor_x_target = 0;

/**
 * the plus one is necessary to distinguish softbreaks from line breaks. Each 
 * line contains the chars and if it ends with a line break that as well. All
 * chars after that are '\0'. The last char must be either '\0' (soft break) or
 * '\n' (line break)
 */
// static char lines[EDITOR_LINES][EDITOR_COLUMNS + 1];

typedef struct {
	// index of the line that's printed where this line starts. One line with
	// one soft break correspond to two vlines.
	size_t vline_begin;
	// text of the line. Does not include '\n', not NUL-terminated.
	dyn_arr_char_t string;
} line_t;

#undef DYN_ARR_H_
#define DYN_ARR_IMPLEMENTATION
#define DYN_ARR_CG_TYPE line_t
#define DYN_ARR_CG_SUFFIX line
#include "dyn_arr.h"
#undef DYN_ARR_CG_SUFFIX
#undef DYN_ARR_CG_TYPE
#undef DYN_ARR_IMPLEMENTATION

/**
 * starting with line 0, ordered.
 */
static dyn_arr_line_t lines;
/**
 * number of vlines in total (not only those that are visible)
 */
static size_t vlines_count;

/**
 * first visible vline
 */
static size_t vvlines_begin = 0;

static void insert_line_break() {
	// TODO
}

static void backspace() {
	/*
	if (cursor_pos.x == 0)
		; // TODO
	else {
		// dyn_arr of the chars starting with the one after the one to delete
		// and ending with the next line break (or not if it is eof)
		dyn_arr_char_t rest;
		dyn_arr_char_create(10, 3, 2, &rest);
		size_t x = cursor_pos.x;
		for (size_t y = cursor_pos.y; y < EDITOR_LINES; ++y) {
			for (; x < EDITOR_COLUMNS; ++x) {
				char c = lines[y][x];
				dyn_arr_char_add(&rest, c);
				if (c == '\n') {
					dyn_arr_char_add(&rest, '\0');
					goto break_outer_loop;
				}
				else if (c == '\0') {
					goto break_outer_loop;
				}
			}
			x = 0;
		}
break_outer_loop:
		for (int y = 0; y < 2; ++y)
			for (int x = 0; x < EDITOR_COLUMNS; ++x)
				print_char(x, EDITOR_LINES - 2 + y, ' ', 0);
		print_chars(0, EDITOR_LINES - 2, rest.arr);
	}
	*/
}

static void insert_char(char c) {
	// TODO
}

static void handle_char(char c) {
	switch(c) {
		case '\n': 
			insert_line_break();
			break;
		case '\x08':
			display_error("Not implemented", 1);
			backspace();
			break;
		default:
			insert_char(c);
			break;
	}
}

/**
 * if horizontal_move and the cursor_x_target is greater than its current x, 
 * it tries to move it there. If the cursor comes behind '\0' or '\n', i.e., 
 * non-text, it moves it to the first '\0' or '\n' ,i.e., non-text, of the line.
 * If not horizontal_move, then the target will be overwritten.
 */
static void correct_char_pos(char horizontal_move) {
	/*
	const char* line = lines[cursor_pos.y];
	char tmp;
	if (cursor_pos.x == 0 || (tmp = line[cursor_pos.x - 1],
				tmp != '\0' && tmp  != '\n')) { 
		// cursor is right behind text
		if (!horizontal_move && cursor_x_target > cursor_pos.x) { 
			// cursor needs to be moved 
			cursor_pos.x = cursor_x_target;
			correct_char_pos(horizontal_move);
		}
		else { // cursor is in correct position;
			cursor_x_target = cursor_pos.x;
		}
	}
	else { // cursor isn't on text, move it to the left
		cursor_pos.x = 0;
		while ((tmp = line[cursor_pos.x], 
					tmp != '\0' && tmp != '\n'))
			++cursor_pos.x;

		if (horizontal_move)
			cursor_x_target = cursor_pos.x;
	}
	*/
}

/**
 * mode = 1: print, mode = 0: erase
 */
static void move_cursor(char mode) {
	/*
	point_t px = char_point_to_point(cursor_pos);
	for (int y = -1; y < (int) CHAR_HEIGHT; ++y)
		Bdisp_SetPoint_DDVRAM(px.x - 1, px.y + y, mode);
		*/
}

void initialize_editor(const char* content) { 
	Bdisp_AllClr_DDVRAM();
	initialize_lines(content);
	print_lines();

	unsigned int key;
	while (1)
		GetKey(&key);
	/*
	move_cursor(1);
	while (1) {
		unsigned int key;
		GetKey(&key);
		int c = key_code_to_ascii(key);
		if (c >= 0) {
			handle_char((char) c);
			// char char_str[] = { (char) c, '\0' };
			// print_chars(0, EDITOR_LINES - 1, char_str);
		} else if (c <= CODE_UP && c >= CODE_RIGHT) {
			move_cursor(0);
			char horizontal_move = 1;
			char moved = 1;
			switch (c) {
				case CODE_UP:
					if (cursor_pos.y > 0)
						--cursor_pos.y;
					else
						moved = 0;
					horizontal_move = 0;
					break;
				case CODE_DOWN:
					if (cursor_pos.y < EDITOR_LINES - 1)
						++cursor_pos.y;
					else
						moved = 0;
					horizontal_move = 0;
					break;
				case CODE_LEFT:
					if (cursor_pos.x > 0)
						--cursor_pos.x;
					else
						moved = 0;
					break;
				case CODE_RIGHT:
					if (cursor_pos.x < EDITOR_COLUMNS - 1)
						++cursor_pos.x;
					else
						moved = 0;
					break;
			}
			if (moved)
				correct_char_pos(horizontal_move);
			
			move_cursor(1);
		}
	}
	*/
}

/**
 * returns the corresponding ascii code, or -1 if this key does not correpond to
 * any ascii code, or -3 to -6 (CODE_{UP, DOWN, RIGHT, LEFT}) when the
 * navigation buttons were used. Backspace is '\x08', enter is '\n'.
 */
int key_code_to_ascii(unsigned int code) {
	const signed char capitalization_once = -1;
	const signed char capitalization_lock = -2;
#define EASY_CASES \
	X(0, '0') \
	X(1, '1') \
	X(2, '2') \
	X(3, '3') \
	X(4, '4') \
	X(5, '5') \
	X(6, '6') \
	X(7, '7') \
	X(8, '8') \
	X(9, '9') \
	X(DP, '.') \
	X(PLUS, '+') \
	X(MINUS, '-') \
	X(MULT, '*') \
	X(DIV, '/') \
	X(LPAR, '(') \
	X(RPAR, ')') \
	X(COMMA, '\x2c') \
	X(POW, '^') \
	X(EQUAL, '=') \
	X(LBRCKT, '[') \
	X(RBRCKT, ']') \
	X(LBRACE, '{') \
	X(RBRACE, '}') \
	X(A, 'A') \
	X(B, 'B') \
	X(C, 'C') \
	X(D, 'D') \
	X(E, 'E') \
	X(F, 'F') \
	X(G, 'G') \
	X(H, 'H') \
	X(I, 'I') \
	X(J, 'J') \
	X(K, 'K') \
	X(L, 'L') \
	X(M, 'M') \
	X(N, 'N') \
	X(O, 'O') \
	X(P, 'P') \
	X(Q, 'Q') \
	X(R, 'R') \
	X(S, 'S') \
	X(T, 'T') \
	X(U, 'U') \
	X(V, 'V') \
	X(W, 'W') \
	X(X, 'X') \
	X(Y, 'Y') \
	X(Z, 'Z') \
	X(SPACE, ' ') \
	X(VALR, capitalization_once) \
	X(SQUARE, capitalization_once) \
	X(ROOT, capitalization_once) \
	X(PMINUS, '\\') \
	X(DQUATE, '"')
#define EASY_CASES_CTRL \
	X(XTT, '`') \
	X(OPTN, capitalization_lock) \
	X(UP, CODE_UP) \
	X(DOWN, CODE_DOWN) \
	X(LEFT, CODE_LEFT) \
	X(RIGHT, CODE_RIGHT) \
	X(EXE, '\n') \
	X(DEL, '\x08')
#define EASY_CASE(c, c_literal) case KEY_CHAR_##c: ch = c_literal; break;
	signed char ch;
	switch (code) {
#define X(c, c_literal) EASY_CASE(c, c_literal);
		EASY_CASES
#undef X
#define X(c, c_literal) case KEY_CTRL_##c: ch = c_literal; break;
		EASY_CASES_CTRL
#undef X
#undef EASY_CASE
		default: return -1;
	}
	if (ch == capitalization_once) {
		capitalization_on = true;
		return -1;
	} else if (ch == capitalization_lock) {
		capitalization_on_locked ^= true;
		return -1;
	} else if (ch < 0) {
		return ch;
	}

	if (capitalization_on || capitalization_on_locked) {
		capitalization_on = false;
		switch (ch) {
			case '"': return '\'';
			case '(': return '<';
			case ')': return '>';
			case '1': return '!';
			case '2': return '@';
			case '3': return '#';
			case '4': return '$';
			case '5': return '%';
			case ',': return ';';
			case '.': return ':';
			case '7': return '&';
			case '/': return '?';
			case '-': return '_';
			case '`': return '~';
			case '+': return '|';
			case '\\': return '|';
		}
	}
	else if (ch >= 'A' && ch <= 'Z')
		return ch + 32;
	return ch;
}

/**
 * returns pixel coordinate of top left corner of that character coordinate.
 * If point is outside of editor space, (-1, -1) is returned.
 */
static point_t char_point_to_point(char_point_t point) {
	if (point.x > EDITOR_COLUMNS || point.y > EDITOR_LINES) {
		point_t err_point = { (unsigned char) -1, (unsigned char) -1 };
		return err_point;
	}

	point_t ret_val = { EDITOR_LEFT, EDITOR_TOP };
	ret_val.x += point.x * CHAR_WIDTH_OUTER + MARGIN_LEFT;
	ret_val.y += point.y * CHAR_HEIGHT_OUTER + MARGIN_TOP;
	return ret_val;
}

/**
 * adds an empty line_t to lines
 */
static void add_new_line(size_t vline_begin) {
	line_t new_line;
	new_line.vline_begin = vline_begin;
	dyn_arr_line_add(&lines, new_line);
	dyn_arr_char_t* string = &DYN_ARR_LAST(&lines)->string;
	if (dyn_arr_char_create(1, 2, 1, string) == -1)
		display_error("Out of memory", 1);
}

/**
 * fills lines with str. lines should be uninitialized at this point.
 */
static void initialize_lines(const char* str) {
	if(dyn_arr_line_create(10, 2, 1, &lines) == -1)
		display_error("Out of memory", 1);

	if (*str == '\0')
		return;
	
	add_new_line(0);
	line_t* current_line = DYN_ARR_LAST(&lines);
	unsigned char col = 0;
	size_t vline = 0;
	size_t i = 0;
	for (; str[i]; ++i) {
		if (str[i] == '\n') {
			++vline;
			add_new_line(vline);
			current_line = DYN_ARR_LAST(&lines);
			col = 0;
		} 
		else {
			// only check if col is out of bounds before adding new char. This
			// prevents unnecessary soft breaks before '/n' and EOF
			if (col >= EDITOR_COLUMNS) { 
				col = 0;
				++vline;
			}
			dyn_arr_char_add(&current_line->string, str[i]);
			++col;
		}
	}
	vlines_count = vline + 1;
}

static void print_line(size_t line_i) {
	line_t* line = &lines.arr[line_i];
	size_t vvline_end;	// exclusive, end of the visible vlines of this line
	if (lines.count - 1 <= line_i) // last line
		vvline_end = vlines_count;
	else
		vvline_end = lines.arr[line_i + 1].vline_begin;

	if (vvline_end > vvlines_begin + EDITOR_LINES)
		vvline_end = vvlines_begin + EDITOR_LINES;

	size_t char_i = 0; // index in the string
	unsigned char col = 0;
	size_t vline = vvlines_begin; // first visible vline of this line
	if (vline < line->vline_begin)
		vline = line->vline_begin;

	while (char_i < line->string.count && vline < vvline_end) {
		if (col >= EDITOR_COLUMNS) { // next vline
			col = 0;
			++vline;
			continue;
		}
		print_char_xy(col, vline - vvlines_begin, line->string.arr[char_i], 0);
		++char_i;
		++col;
	}
}

static void print_chars(unsigned char x, unsigned char y, const char* str) {
	char_point_t current_pos = { x, y };
	size_t i = 0;
	for (; str[i]; ++i) {
		if (current_pos.x >= EDITOR_COLUMNS) {
			++current_pos.y;
			current_pos.x = 0;
			if (current_pos.y >= EDITOR_LINES)
				break;
		}
		if (str[i] == '\n') {
			++current_pos.y;
			current_pos.x = 0;
			if (current_pos.y >= EDITOR_LINES)
				break;
			continue;
		}
		// if (cursor_pos.x == current_pos.x && cursor_pos.y == current_pos.y) {
			// print_char(current_pos, str[i], 1);
		// } 
		// else 
			print_char(current_pos, str[i], 0);
		++current_pos.x;
	}
}

static void print_lines() {
	for (size_t i = 0; i < lines.count; ++i)
		print_line(i);
}


static void 
print_char_xy(unsigned char x, unsigned char y, char c, char negative) {
	char_point_t point = { x, y };
	print_char(point, c, negative);
}

/**
 * if negative true, the colors will be inversed
 */
static void print_char(char_point_t point, char c, char negative) {
	point_t px = char_point_to_point(point);
	for (int y = 0; y < CHAR_HEIGHT; ++y) {
		for (int x = 0; x < CHAR_WIDTH; ++x) {
			Bdisp_SetPoint_DDVRAM(px.x + x, px.y + y, 
					font[(unsigned char) c][y][x] ^ negative);
		}
	}
}
