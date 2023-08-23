#include <stdio.h>
#include <math.h>
#include <stddef.h>

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
#include "util.h"

struct line_col {
	// index of line (in lines)
	size_t line;
	// index of the char in the line's string
	size_t char_i;
};
typedef struct line_col line_col_t;


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
static int print_line(size_t line_i);
static int print_partial_line(line_col_t line_col);
static void print_chars(unsigned char x, unsigned char y, const char* str);
static char line_col_to_point(line_col_t line_col, point_t* point);
static size_t line_col_to_vline(line_col_t line_col, unsigned char* x);
static int key_code_to_ascii(unsigned int code);
static void handle_char(char c);
static void handle_cursor_move(int move);
static void	handle_cursor_move_vertical(int move);
static size_t* get_vline_starts(line_t* line, size_t* count_softbreaks);
static size_t* add_softbreak_to_index(line_t* current_line, size_t i);
static void move_cursor(int mode);
static int maybe_scroll_down(size_t cursor_vline);
static int maybe_scroll_up(size_t vline);
static void update_changes_from(line_col_t begin);
static ptrdiff_t recalculate_vline_index(line_t* line, size_t vline_offs);


// static variables


static char capitalization_on = 0;
static char capitalization_on_locked = 0;
static line_col_t cursor_pos = { 0, 0 };
/** 
 * vertical moves will try to get as close possible to cursor_x_target with the
 * cursor's x value if cursor_x_target is greater than the previous x value.
 * Setting the target to 0 means that the current x value is the
 * target.
 * Successful horizontal moves (including inserts and deletes) set 
 * cursor_x_target to 0. If a vertical 
 * move can't needs to reduce x (due to the line length), it updates
 * cursor_x_target to the old x value.
 */
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

static void insert_line_break(void) {

	// insert new line
	line_t uninitialized;
	dyn_arr_line_insert(&lines, uninitialized, cursor_pos.line + 1);
	line_t* line = &lines.arr[cursor_pos.line];
	line_t* new_line = &lines.arr[cursor_pos.line + 1];
	new_line->count_softbreaks = 0;
	new_line->vline_begin = 0;
	dyn_arr_char_create(0, 2, 1, &new_line->string);
	
	size_t transfer_count = line->string.count - cursor_pos.char_i;
	// copy chars from old line
	dyn_arr_char_add_all(&new_line->string, 
			&line->string.arr[cursor_pos.char_i], transfer_count);

	// calculate vline_index
	recalculate_vline_index(new_line, 0);

	// remove chars from old line
	dyn_arr_char_pop_some(&line->string, transfer_count);

	line_col_t changes_begin = { cursor_pos.line, cursor_pos.char_i };
	// this is necessary because the x value can be 0 and after a softbreak. 
	// update_changes_from would
	// then only start in the vline after that, even though we also need to
	// remove the vline of the cursor position
	if (changes_begin.char_i > 0)
		--changes_begin.char_i;
	move_cursor(0);
	++cursor_pos.line;
	cursor_pos.char_i = 0;
	update_changes_from(changes_begin);

	cursor_x_target = 0;
}

static void backspace(void) {
	move_cursor(0);

	line_t* line = &lines.arr[cursor_pos.line];

	if (cursor_pos.char_i == 0) {
		if (cursor_pos.line == 0) {
			// nothing to do
			move_cursor(1);
			return;
		}
		// concat with previous line
		line_t* previous = &lines.arr[cursor_pos.line - 1];
		line_col_t begin_change = { cursor_pos.line - 1, 
			previous->string.count };
		if (dyn_arr_char_add_all(&previous->string, 
					line->string.arr, line->string.count) == -1)
			display_error("Out of memory");
		dyn_arr_char_destroy(&line->string);
		if (dyn_arr_line_remove(&lines, cursor_pos.line) == -1)
			display_error("Out of memory");
		cursor_pos = begin_change;
		update_changes_from(begin_change);
	}
	else {
		if (dyn_arr_char_remove(&line->string, cursor_pos.char_i - 1) == -1)
			display_error("Out of memory");
		--cursor_pos.char_i;

		line_col_t begin_change = { cursor_pos.line, cursor_pos.char_i };
		update_changes_from(begin_change);
	}
	cursor_x_target = 0;
}

/**
 * removes the softbreaks from the line's vline_index until only target_count
 * remain
 */
static void remove_last_n_softbreaks(line_t* line, size_t target_count) {
	size_t count_to_remove = line->count_softbreaks - target_count;
	if (line->count_softbreaks > VLINE_INDEX_STATIC_ARR_SIZE) {
		// we're currently using d_arr
		if (target_count <= VLINE_INDEX_STATIC_ARR_SIZE) {
			// move first target_count elements of d_arr to s_arr
			size_t* heap_arr = line->vline_index.d_arr.arr;
			memmove(line->vline_index.s_arr, 
					line->vline_index.d_arr.arr, 
					target_count * sizeof(size_t));
			free(heap_arr);
		}
		else
			// keep using d_arr
			dyn_arr_size_pop_some(&line->vline_index.d_arr, count_to_remove);
	}
	line->count_softbreaks = target_count;
}

/**
 * recalculates the vline_index of line, starting with the starting index of
 * the vline after vline_offs. If the line's count_softbreaks is 0 and
 * vline_offs is 0, this functions just adds all indices to vline_index and 
 * correctly sets count_softbreaks.
 *
 * returns the difference in how many vlines there were before and after. If new
 * vlines were added, that number is positive.
 */
static ptrdiff_t recalculate_vline_index(line_t* line, size_t vline_offs) {
	size_t count_softbreaks_start;
	size_t* vline_starts = get_vline_starts(line, &count_softbreaks_start);
	size_t  char_i; 
	if (vline_offs == 0)
		char_i = 0;
	else
		char_i = vline_starts[vline_offs - 1];


	// recalculate existing starts
	for (size_t sb_i = vline_offs; sb_i < line->count_softbreaks; ++sb_i) {
		char_i += EDITOR_COLUMNS;
		if (char_i >= line->string.count) {
			remove_last_n_softbreaks(line, sb_i);
			break;
		}
		vline_starts[sb_i] = char_i;
	}

	// add softbreaks until char_i is out of range of the line's string
	while ((char_i += EDITOR_COLUMNS) < line->string.count)
		add_softbreak_to_index(line, char_i);
	
	return line->count_softbreaks - count_softbreaks_start;
}

/**
 * updates lines, cursor and screen
 */
static void insert_char(char c) {
	move_cursor(0);

	line_t* line = &lines.arr[cursor_pos.line];
	if (dyn_arr_char_insert(&line->string, c, cursor_pos.char_i) == -1)
		display_error("Out of memory");
	++cursor_pos.char_i;

	line_col_t begin;
	begin.line = cursor_pos.line;
	begin.char_i = cursor_pos.char_i - 1;
	cursor_x_target = 0;
	update_changes_from(begin);
}

/**
 * updates vline_indices, cursor and screen according to a change that starts at
 * begin. begin is used to determine the last vline whose index in vline_index
 * is still correct. The cursor should be at the correct position already, 
 * to determine if scrolling is necessary. 
 * The vlines of the subsequent lines are set so one directly follows its 
 * predecessor's last vline. 
 */
static void update_changes_from(line_col_t begin) {
	// find vline of begin
	size_t vline = line_col_to_vline(begin, NULL);
	line_t* line = &lines.arr[begin.line];

	// recalculate vline_index of line, starting from vline's successor
	recalculate_vline_index(line, vline - line->vline_begin);
	// shift all subsequent vline_indices if necessary
	char shifted = 0;
	for (size_t line_i = begin.line + 1;
			line_i < lines.count; 
			++line_i) {
		line_t* last_line = &lines.arr[line_i - 1];
		size_t new_vline_begin = last_line->vline_begin 
			+ last_line->count_softbreaks + 1;
		line_t* shift_line = &lines.arr[line_i];
		ptrdiff_t offset = new_vline_begin - shift_line->vline_begin;
		shifted |= offset != 0;
		shift_line->vline_begin += offset;
	}

	// scroll if necessary
	size_t new_vline = line_col_to_vline(cursor_pos, NULL);
	line_t* last_line = DYN_ARR_LAST(&lines);
	size_t last_vline = last_line->count_softbreaks + last_line->vline_begin;
	char scrolled = 1;
	if (!maybe_scroll_down(new_vline)) {
		if (last_vline >= EDITOR_LINES - 1) { 
			// without this the last lines could be blank when deleting
			if (!maybe_scroll_up(last_vline - (EDITOR_LINES - 1))) {
				if (!maybe_scroll_up(new_vline)) {
					scrolled = 0;
				}
			}
		}
		else {
			if (!maybe_scroll_up(new_vline)) {
				scrolled = 0;
			}
		}
	}
	if (!scrolled) {
		// did not scroll, need to print manually
		print_partial_line(begin);
		if (shifted) {
			for (size_t line_i = begin.line + 1;
					line_i < lines.count; 
					++line_i) {
				if (!print_line(line_i))
					// line_i was (partially) below visible area
					break;
			}
		}
	}


	move_cursor(1);
}

static void handle_char(char c) {
	switch(c) {
		case '\n': 
			insert_line_break();
			break;
		case '\x08':
			backspace();
			break;
		default:
			insert_char(c);
			break;
	}
}


/**
 * mode = 1: print, mode = 0: erase
 */
static void move_cursor(int mode) {
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

	cursor_pos.line = 0;
	cursor_pos.char_i = 0;

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

/**
 * if vline is before the first vvline, it adjusts vvlines so that 
 * vline is the first vvline
 *
 * returns whether it scrolled
 */
static int maybe_scroll_up(size_t vline) {
	if (vline < vvlines_begin) {
		vvlines_begin = vline;
		print_lines();
		return 1;
	}
	return 0;
}

/**
 * if vline is after the last vvline, it adjusts vvlines so that 
 * vline is the last vvline
 *
 * returns whether it scrolled
 */
static int maybe_scroll_down(size_t vline) {
	if (vline >= vvlines_begin + EDITOR_LINES) {
		vvlines_begin = vline - EDITOR_LINES + 1;
		print_lines();
		return 1;
	}
	return 0;
}

static void handle_cursor_move(int move) {
	move_cursor(0);
	switch (move) {
		case CODE_LEFT:
			if (cursor_pos.char_i > 0) {
				--cursor_pos.char_i;
				size_t vline = line_col_to_vline(cursor_pos, NULL);
				cursor_x_target = 0;
				maybe_scroll_up(vline);
			}
			break;
		case CODE_RIGHT:
			if (cursor_pos.char_i < lines.arr[cursor_pos.line].string.count) {
				++cursor_pos.char_i;
				size_t vline = line_col_to_vline(cursor_pos, NULL);
				cursor_x_target = 0;
				maybe_scroll_down(vline);
			}
			break;
		default:
			handle_cursor_move_vertical(move);
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
				|| cursor_pos.char_i < vline_starts_old[0]) { 
			// in first vline of line
			if (cursor_pos.line == 0)
				return;
			new_line_i = cursor_pos.line - 1;
			new_vline_offs = lines.arr[new_line_i].count_softbreaks;
			x = cursor_pos.char_i;
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
				|| cursor_pos.char_i >= vline_starts_old[count_softbreaks - 1]) { 
			// last vline of line
			if (cursor_pos.line == lines.count - 1)
				return;
			new_line_i = cursor_pos.line + 1;
			new_vline_offs = 0;
			line_col_to_vline(cursor_pos, &x);
		}
		else { // we stay in the same line
			new_line_i = cursor_pos.line;
			size_t current_vline = line_col_to_vline(cursor_pos, &x);
			new_vline_offs = current_vline - lines.arr[new_line_i].vline_begin 
				+ 1;
		}
	}

	// tentatively set x to max(x, cursor_x_target)
	if (cursor_x_target > x)
		x = cursor_x_target;

	line_t* new_line = &lines.arr[new_line_i];
	size_t count_softbreaks_new;
	size_t* vline_starts_new = // vline starts of new line of cursor
		get_vline_starts(new_line, &count_softbreaks_new);

	// figure out cursor_pos.char_i
	size_t begin_i; // index in line of first char in vline
	if (new_vline_offs == 0)
		begin_i = 0;
	else
		begin_i = vline_starts_new[new_vline_offs - 1];
	// number of chars in vline
	size_t vline_length = new_line->string.count - begin_i;
	if (x >= vline_length) { // there is no char at x pos in the vline,
		// cursor_pos.char_i is after last char in line
		cursor_pos.char_i = new_line->string.count;
		cursor_x_target = x;
	}
	else {
		cursor_pos.char_i = begin_i + x;
	}

	cursor_pos.line = new_line_i;

	if (move == CODE_UP)
		maybe_scroll_up(new_line->vline_begin + new_vline_offs);
	else 
		maybe_scroll_down(new_line->vline_begin + new_vline_offs);
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
	signed char ch;
	switch (code) {
#define X(c, c_literal) case EVAL(EVAL(KEY_CHAR_##c)): ch = c_literal; break;
		EASY_CASES
#undef X
#define X(c, c_literal) case EVAL(EVAL(KEY_CTRL_##c)): ch = c_literal; break;
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
 * corresponds to. If char_i is greater than the index of the last char of the
 * line, the last vline of the line is returned, x will be greater than
 * EDITOR_COLUMNS.
 */
static size_t line_col_to_vline(line_col_t line_col, unsigned char* x) {
	line_t* line = &lines.arr[line_col.line];
	if (line->count_softbreaks == 0) {
		// line is contained in one vline
		if (x != NULL)
			*x = line_col.char_i;

		return line->vline_begin;
	}


	// find the correct array
	size_t* vline_starts = get_vline_starts(line, NULL);

	// check if the the cursor is in first vline of the line
	if (line_col.char_i < vline_starts[0]) {
		if (x != NULL)
			*x = line_col.char_i;
		return line->vline_begin;
	}

	size_t vline_index = dyn_arr_size_raw_bsearch(vline_starts, 
			line->count_softbreaks, line_col.char_i);

	if (x != NULL)
		*x = line_col.char_i - vline_starts[vline_index];

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
		display_error("Out of memory");
}

/**
 * adds an entry to vline_index of current_line with index i. returns the array
 * that is currently used
 */
static size_t* add_softbreak_to_index(line_t* current_line, size_t i) {
	size_t* ret_val;
	if (current_line->count_softbreaks < 
			VLINE_INDEX_STATIC_ARR_SIZE) { // put into s_arr
		ret_val = current_line->vline_index.s_arr;
		ret_val[current_line->count_softbreaks] = i;
	}
	else {
		dyn_arr_size_t* d_arr = &current_line->vline_index.d_arr;
		if (current_line->count_softbreaks == VLINE_INDEX_STATIC_ARR_SIZE) {
			// move s_arr to d_arr
			size_t* s_arr = current_line->vline_index.s_arr;
			size_t intermediate[VLINE_INDEX_STATIC_ARR_SIZE];
			memcpy(intermediate, s_arr, 
					VLINE_INDEX_STATIC_ARR_SIZE * sizeof(size_t));
			dyn_arr_size_create(VLINE_INDEX_STATIC_ARR_SIZE + 1, 2, 1, d_arr);
			dyn_arr_size_add_all(d_arr, intermediate, 
					VLINE_INDEX_STATIC_ARR_SIZE);
		}

		ret_val = d_arr->arr;
		dyn_arr_size_add(d_arr, i);
	}
	++current_line->count_softbreaks;
	return ret_val;
}

/**
 * fills lines with str. lines should be uninitialized at this point.
 */
static void initialize_lines(const char* str) {
	if(dyn_arr_line_create(10, 2, 1, &lines) == -1)
		display_error("Out of memory");

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

static int print_line(size_t line_i) {
	line_col_t line_col = { line_i, 0 };
	return print_partial_line(line_col);
}

/**
 * prints starting from line_col
 *
 * Return 1 if none of the vlines that should be printed were below what is
 * currently visible, otherwise returns 0.
 */
static int print_partial_line(line_col_t line_col) {
	line_t* line = &lines.arr[line_col.line];
	// exclusive, end of the visible vlines of this line
	size_t vvlines_local_end;	
	if (line_col.line == lines.count - 1) { // line_col.line is the last line 
		vvlines_local_end = line->vline_begin + line->count_softbreaks + 1;
	}
	else
		vvlines_local_end = lines.arr[line_col.line + 1].vline_begin;

	if (vvlines_local_end > vvlines_begin + EDITOR_LINES) 
		// line ends outside of visible area
		vvlines_local_end = vvlines_begin + EDITOR_LINES;

	// find vline and x value that correspond to line_col or if these aren't
	// visible, the first that are.
	size_t char_i; // index in the string
	unsigned char x;
	size_t vline = line_col_to_vline(line_col, &x); 
	if (vline < vvlines_begin) {
		if (line->vline_begin + line->count_softbreaks < vvlines_begin)
			// last vline of line isn't even visible, nothing to print
			return 0;
		// first vline is before vvlines_begin, last is not before 
		// => vvlines_begin is a vline of the line
		vline = vvlines_begin;
		x = 0;
		// find new starting index
		size_t* vline_starts = get_vline_starts(line, NULL);
		char_i = vline_starts[vline - line->vline_begin - 1];
	}
	else
		char_i = line_col.char_i;


	
	while (char_i < line->string.count && vline < vvlines_local_end) {
		if (x >= EDITOR_COLUMNS) { // next vline
			x = 0;
			++vline;
			continue;
		}
		print_char_xy(x, vline - vvlines_begin, line->string.arr[char_i], 0);
		++char_i;
		++x;
	}

	if (vline >= vvlines_local_end)
		// line out of screen space
		return 0;

	while (x < EDITOR_COLUMNS) {
		print_char_xy(x, vline - vvlines_begin, ' ', 0);
		++x;
	}

	return 1;
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

/**
 * vline_void is a pointer to a (size_t) vline.
 * returns sign(vline - other->vline_begin);
 */
static int compare_lines_vline_begin(const void* vline_void, 
		const line_t* other) {
	size_t vline = *(size_t*) vline_void;
	if (vline < other->vline_begin)
		return -1;
	else if (vline > other->vline_begin)
		return 1;
	return 0;
}

static void print_lines() {
	size_t first_line = dyn_arr_line_bsearch_cb(&lines, (void*) &vvlines_begin, 
			&compare_lines_vline_begin);
	for (size_t i = first_line; i < lines.count; ++i)
		if (!print_line(i))
			break;
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
	for (unsigned int y = 0; y < CHAR_HEIGHT; ++y) {
		for (unsigned int x = 0; x < CHAR_WIDTH; ++x) {
			Bdisp_SetPoint_DDVRAM(px.x + x, px.y + y, 
					font[(unsigned char) c][y][x] ^ negative);
		}
	}
}
