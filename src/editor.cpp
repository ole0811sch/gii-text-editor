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

typedef struct {
	size_t line;
	size_t col;
} line_col_t;


#define VLINE_INDEX_STATIC_ARR_SIZE 4

/**
 * stores any number of size_t values. If they're at most 
 * VLINE_INDEX_STATIC_ARR_SIZE, they are stored in a static array, otherwise in
 * a dynamic array.
 */
typedef union {
	size_t s_arr[VLINE_INDEX_STATIC_ARR_SIZE];
	dyn_arr_size_t d_arr;
} vline_index_t;

typedef struct {
	/**
	 * Number of softbreaks in this line. If count is at most 
	 * VLINE_INDEX_STATIC_ARR_SIZE, vline_index stores the indices in s_arr,
	 * otherwise in d_arr.
	 */
	size_t count_softbreaks;
	/** 
	 * index of the line that's printed where this line starts. One line with
	 * one soft break correspond to two vlines.
	 */
	size_t vline_begin;
	/**
	 * indices of the softbreaks. Represented as the index of the first char in
	 * the new vline
	 */
	vline_index_t vline_index;
	/**
	 * text of the line. Does not include '\n', not NUL-terminated.
	 */
	dyn_arr_char_t string;
} line_t;

// static function declarations
static void print_char_xy(unsigned char x, unsigned char y, char c, 
		char negative);
static void print_char(char_point_t point, char c, char negative);
static void initialize_lines(const char* str);
static void print_lines();
static void print_chars(unsigned char x, unsigned char y, const char* str);
static char line_col_to_point(line_col_t line_col, point_t* point);
static size_t line_col_to_vline(line_col_t line_col, unsigned char* x);
static int key_code_to_ascii(unsigned int code);
static void handle_char(char c);
static void handle_cursor_move(int move);
static void	handle_cursor_move_vertical(int move);
static size_t* get_vline_starts(line_t* line, size_t* count_softbreaks);


// static variables

static char dbg_buf[256];

static char capitalization_on = 0;
static char capitalization_on_locked = 0;
static line_col_t cursor_pos = { 0, 0 };
static unsigned char cursor_x_target = 0;


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
	point_t px;
	if (!line_col_to_point(cursor_pos, &px))
		return;

	for (int y = -1; y < (int) CHAR_HEIGHT; ++y)
		Bdisp_SetPoint_DDVRAM(px.x - 1, px.y + y, mode);
}

void initialize_editor(const char* content) { 
	Bdisp_AllClr_DDVRAM();
	initialize_lines(content);
	print_lines();

	cursor_pos.line = 4;
	cursor_pos.col = 0;

	move_cursor(1);

	
	while (1) {
		unsigned int key;
		GetKey(&key);
		int c = key_code_to_ascii(key);
		if (c >= 0)
			handle_char((char) c);
		else if (c <= CODE_UP && c >= CODE_RIGHT)
			handle_cursor_move(c);
	}
}

static void handle_cursor_move(int move) {
	move_cursor(0);
	char is_horizontal = 1;
	switch (move) {
		case CODE_LEFT:
			if (cursor_pos.col > 0) {
				--cursor_pos.col;
				line_col_to_vline(cursor_pos, &cursor_x_target);
			}
			break;
		case CODE_RIGHT:
			if (cursor_pos.col < lines.arr[cursor_pos.line].string.count) {
				++cursor_pos.col;
				line_col_to_vline(cursor_pos, &cursor_x_target);
			}
			break;
		default:
			handle_cursor_move_vertical(move);
			is_horizontal = 0;
	}

	// if (moved)
		// correct_char_pos(horizontal_move);
	move_cursor(1);
}

/**
 * updates cursor_pos according to move
 */
static void	handle_cursor_move_vertical(int move) {
	size_t count_softbreaks;
	size_t* vline_starts_old = // vline starts of old line of cursor
		get_vline_starts(&lines.arr[cursor_pos.line], &count_softbreaks);
	size_t new_line_i;
	// offset of the vline of the new position relative to the line's
	// vline_begin
	size_t new_vline_offs; 
	unsigned char x;

	if (move == CODE_UP) {
		if (vline_starts_old == NULL 
				|| cursor_pos.col < vline_starts_old[0]) { 
			// in first vline of line
			if (cursor_pos.line == 0)
				return;
			new_line_i = cursor_pos.line - 1;
			new_vline_offs = lines.arr[new_line_i].count_softbreaks;
			x = cursor_pos.col;
		}
		else { // we stay in the same line
			new_line_i = cursor_pos.line;
			size_t current_vline = line_col_to_vline(cursor_pos, &x);
			new_vline_offs = current_vline - lines.arr[new_line_i].vline_begin 
				- 1;
		}
	}
	else { // down
		if (vline_starts_old == NULL 
				|| cursor_pos.col >= vline_starts_old[count_softbreaks - 1]) { 
			// last vline of line
			if (cursor_pos.line == lines.count - 1)
				return;
			new_line_i = cursor_pos.line + 1;
			new_vline_offs = 0;
			x = cursor_pos.col;
		}
		else { // we stay in the same line
			new_line_i = cursor_pos.line;
			size_t current_vline = line_col_to_vline(cursor_pos, &x);
			new_vline_offs = current_vline - lines.arr[new_line_i].vline_begin 
				+ 1;
		}
	}

	if (cursor_x_target > x)
		x = cursor_x_target;

	line_t* new_line = &lines.arr[new_line_i];
	size_t count_softbreaks_new;
	size_t* vline_starts_new = // vline starts of new line of cursor
		get_vline_starts(new_line, &count_softbreaks_new);

	// figure out cursor_pos.col
	size_t begin_i; // index in line of first char in vline
	if (new_vline_offs == 0)
		begin_i = 0;
	else
		begin_i = vline_starts_new[new_vline_offs - 1];
	// number of chars in vline
	size_t vline_length = new_line->string.count - begin_i;
	if (x >= vline_length) { // there is no char at x pos in the vline
		// cursor_pos.col is after last char in line
		cursor_pos.col = new_line->string.count;
	}
	else {
		cursor_pos.col = begin_i + x;
	}

	cursor_pos.line = new_line_i;
}

/**
 * returns the array in the vline_index of line. If there are no softbreaks,
 * NULL is returned. If count_softbreaks isn't NULL, it is set.
 */
static size_t* get_vline_starts(line_t* line, size_t* count_softbreaks) {
	if (count_softbreaks != NULL)
		*count_softbreaks = line->count_softbreaks;
	if (line->count_softbreaks == 0)
		return NULL;
	else if (line->count_softbreaks <= VLINE_INDEX_STATIC_ARR_SIZE)
		return line->vline_index.s_arr;
	else
		return line->vline_index.d_arr.arr;
}

/**
 * returns the corresponding ascii code, or -1 if this key does not correpond to
 * any ascii code, or -3 to -6 (CODE_{UP, DOWN, RIGHT, LEFT}) when the
 * navigation buttons were used. Backspace is '\x08', enter is '\n'.
 */
static int key_code_to_ascii(unsigned int code) {
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
 * if x isn't null, it is set to the char_point_t x value that line_col
 * corresponds to.
 */
static size_t line_col_to_vline(line_col_t line_col, unsigned char* x) {
	line_t* line = &lines.arr[line_col.line];
	if (line->count_softbreaks == 0) {
		// line is contained in one vline
		if (x != NULL)
			*x = line_col.col;
		return line->vline_begin;
	}


	// find the correct array
	size_t* vline_starts = get_vline_starts(line, NULL);

	// check if the the cursor is in first vline of the line
	if (line_col.col < vline_starts[0]) {
		if (x != NULL)
			*x = line_col.col;
		return line->vline_begin;
	}

	size_t vline_index = dyn_arr_size_raw_bsearch(vline_starts, 
			line->count_softbreaks, line_col.col);

	if (x != NULL)
		*x = line_col.col - vline_starts[vline_index];
	return vline_index + 1 + line->vline_begin;
}

/**
 * returns pixel coordinate of top left corner of that character coordinate.
 */
static point_t char_point_to_point(char_point_t point) {
	point_t ret_val = { EDITOR_LEFT, EDITOR_TOP };
	ret_val.x += point.x * CHAR_WIDTH_OUTER + MARGIN_LEFT;
	ret_val.y += point.y * CHAR_HEIGHT_OUTER + MARGIN_TOP;
	return ret_val;
}

/**
 * returns whether line_col is currently even visible
 * If line_col is visible, point is set to the pixel coordinate of top left 
 * corner of that character coordinate.
 */
static char line_col_to_point(line_col_t line_col, point_t* point) {
	unsigned char x;
	size_t vline = line_col_to_vline(line_col, &x);
	
	if (vline < vvlines_begin || vline >= vvlines_begin + EDITOR_LINES)
		return 0;

	char_point_t ch_point;
	ch_point.x = x;
	ch_point.y = vline - vvlines_begin;
	*point = char_point_to_point(ch_point);
	return 1;
}

/**
 * adds an empty line_t to lines
 */
static void add_new_line(size_t vline_begin) {
	line_t new_line;
	new_line.vline_begin = vline_begin;
	new_line.count_softbreaks = 0;
	dyn_arr_line_add(&lines, new_line);
	dyn_arr_char_t* string = &DYN_ARR_LAST(&lines)->string;
	if (dyn_arr_char_create(1, 2, 1, string) == -1)
		display_error("Out of memory", 1);
}

/**
 * adds an entry to vline_index of current_line with index i
 */
static void add_softbreak_to_index(line_t* current_line, size_t i) {
	if (current_line->count_softbreaks < 
			VLINE_INDEX_STATIC_ARR_SIZE) { // put into s_arr
		current_line->vline_index.s_arr[current_line->count_softbreaks] = i;
	}
	else {
		dyn_arr_size_t* d_arr = &current_line->vline_index.d_arr;
		if (current_line->count_softbreaks == VLINE_INDEX_STATIC_ARR_SIZE) {
			// move s_arr to d_arr
			dyn_arr_size_create(VLINE_INDEX_STATIC_ARR_SIZE + 1, 2, 1, d_arr);
			dyn_arr_size_add_all(d_arr, current_line->vline_index.s_arr, 
					VLINE_INDEX_STATIC_ARR_SIZE);
		}

		dyn_arr_size_add(d_arr, i);
	}
	++current_line->count_softbreaks;
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
	size_t line_starting_index = 0; // index of the first char in str in line
	for (; str[i]; ++i) {
		if (str[i] == '\n') {
			++vline;
			line_starting_index = i + 1;
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
				add_softbreak_to_index(current_line, i - line_starting_index);
			}
			dyn_arr_char_add(&current_line->string, str[i]);
			++col;
		}
	}
}

static void print_line(size_t line_i) {
	line_t* line = &lines.arr[line_i];
	size_t vvline_end;	// exclusive, end of the visible vlines of this line
	if (line_i == lines.count - 1) { // line_i is the last line 
		line_t* last_line = DYN_ARR_LAST(&lines);
		vvline_end = last_line->vline_begin + last_line->count_softbreaks + 1;
	}
	else
		vvline_end = lines.arr[line_i + 1].vline_begin;

	if (vvline_end > vvlines_begin + EDITOR_LINES) 
		// line ends outside of visible area
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
