#include "editor.h"
#include "line_utils.h"
#include "line.h"

#include "dispbios.h"
#include "util.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

/**
 * Text boxes must always have at least one line.
 *
 * vline = view line. A regular line will not be split by a softbreak, but a
 * line with n softbreaks will consist of n+1 vlines.
 * vvline = visible view line
 */


// static function declarations
static void print_char_xy(unsigned short offs_x, unsigned short offs_y, 
unsigned char x, unsigned char y, char c, 
		char negative);
static void print_char(unsigned short offs_x, unsigned short offs_y, 
char_point_t point, char c, char negative);
static void print_lines(text_box_t* box);
static int print_line(text_box_t* box, size_t line_i);
static int print_partial_line(text_box_t* box, line_chi_t line_chi);
static void print_chars(unsigned char x, unsigned char y, const char* str);
static char line_chi_to_point(text_box_t* box, line_chi_t line_chi, 
		point_t* point);
static int key_code_to_ascii(text_box_t* box, unsigned int code);
static void handle_char(text_box_t* box, char c);
static void handle_cursor_move(text_box_t* box, int move);
static void	handle_cursor_move_vertical(text_box_t* box, int move);
static void move_cursor(text_box_t* box, int mode);
static int auto_scroll_down(text_box_t* box, size_t cursor_vline);
static int auto_scroll_up(text_box_t* box, size_t vline);
static void update_changes_from(text_box_t* box, line_chi_t begin);
static text_box_t create_text_box(unsigned short left_px,
		unsigned short top_px,
		unsigned short width,
		unsigned short height, 
		interaction_mode_t interaction_mode, 
		char editable,
		const char* text);
static void scroll_up(text_box_t* box);
static void scroll_down(text_box_t* box);

/**
 * initializes and start
 */
void initialize_text_box(unsigned short left_px, unsigned short top_px,
		unsigned short width, unsigned short height, 
		interaction_mode_t interaction_mode, char editable, 
		const char* content, text_box_t* box) {
	*box = create_text_box(left_px, top_px, width, height, 
			interaction_mode, editable, content);
}

void destruct_text_box(text_box_t* box) {
	for (size_t i = 0; i < box->lines.count; ++i) {
		destruct_line(&box->lines.arr[i]);
	}
	dyn_arr_line_destroy(&box->lines);
}

void draw_text_box(text_box_t* box) {
	int left_px = box->left_px;
	int top_px = box->top_px;
	const DISPBOX area = { 
		left_px, 
		top_px, 
		left_px + CHARW_TO_PX(box->width) - 1, 
		top_px + CHARH_TO_PX(box->height) - 1 
	};
	Bdisp_AreaClr_DDVRAM(&area);
	print_lines(box);
}

unsigned int focus_text_box(text_box_t* box, unsigned int escape_keys[], 
		unsigned int count_escape_keys) {
	move_cursor(box, 1);

	unsigned int ret_val;
	while (1) {
		unsigned int key;
		GetKey(&key);
		for (unsigned int i = 0; i < count_escape_keys; ++i) {
			if (escape_keys[i] == key) {
				ret_val = key;
				goto end_while;
			}
		}
		int c = key_code_to_ascii(box, key);
		if (c >= 0) {
			handle_char(box, (char) c);
		}
		else if (c <= CODE_UP && c >= CODE_RIGHT) {
			if (box->interaction_mode == CURSOR) {
				handle_cursor_move(box, c);
			}
			else { // SCROLL
				if (c == CODE_UP) {
					scroll_up(box);
				}
				else if (c == CODE_DOWN) {
					scroll_down(box);
				}
			}
		}
	}
end_while:
	
	move_cursor(box, 0);
	return ret_val;
}


/**
 * creates text box. The first vvline is 0, cursor_pos is (0, 0) and there is
 * one empty line. If interaction_mode isn't CURSOR, editable is ignored.
 */
static text_box_t create_text_box(unsigned short left_px,
		unsigned short top_px,
		unsigned short width,
		unsigned short height, 
		interaction_mode_t interaction_mode, 
		char editable,
		const char* text) {
	text_box_t box;
	box.left_px = left_px;
	box.top_px = top_px;
	box.width = width;
	box.height = height;
	box.interaction_mode = interaction_mode;
	box.cursor.editable = interaction_mode == CURSOR && editable;
	box.vvlines_begin = 0;
	box.cursor.position.line = 0;
	box.cursor.position.char_i = 0;
	box.cursor.cursor_x_target = 0;
	box.cursor.editable_state.capitalization_on = 0;
	box.cursor.editable_state.capitalization_on_locked = 0;
	initialize_lines(&box, text);
	if (box.lines.count == 0) {
		add_new_line(&box, 0);
	}
	return box;
}

/**
 * Expect box to be editable and in cursor mode
 */
static void insert_line_break(text_box_t* box) {
	dyn_arr_line_t* lines = &box->lines;
	line_chi_t* cursor_pos = &box->cursor.position;

	// insert new line
	line_t uninitialized = {0};
	dyn_arr_line_insert(lines, uninitialized, cursor_pos->line + 1);
	line_t* line = &lines->arr[cursor_pos->line];
	line_t* new_line = &lines->arr[cursor_pos->line + 1];
	new_line->count_softbreaks = 0;
	new_line->vline_begin = 0;
	dyn_arr_char_create(0, 2, 1, &new_line->string);
	
	size_t transfer_count = line->string.count - cursor_pos->char_i;
	// copy chars from old line
	dyn_arr_char_add_all(&new_line->string, 
			&line->string.arr[cursor_pos->char_i], transfer_count);

	// calculate vline_index
	recalculate_vline_index(box, new_line, 0);

	// remove chars from old line
	dyn_arr_char_pop_some(&line->string, transfer_count);

	line_chi_t changes_begin = { cursor_pos->line, cursor_pos->char_i };
	// this is necessary because the x value can be 0 and after a softbreak. 
	// update_changes_from would
	// then only start in the vline after that, even though we also need to
	// remove the vline of the cursor position
	if (changes_begin.char_i > 0)
		--changes_begin.char_i;
	move_cursor(box, 0);
	++cursor_pos->line;
	cursor_pos->char_i = 0;
	update_changes_from(box, changes_begin);

	box->cursor.cursor_x_target = 0;
}

/**
 * Expect box to be editable and in cursor mode
 */
static void backspace(text_box_t* box) {
	move_cursor(box, 0);

	dyn_arr_line_t* lines = &box->lines;
	line_t* line = &lines->arr[box->cursor.position.line];

	if (box->cursor.position.char_i == 0) {
		if (box->cursor.position.line == 0) {
			// nothing to do
			move_cursor(box, 1);
			return;
		}
		// concat with previous line
		line_t* previous = &lines->arr[box->cursor.position.line - 1];
		line_chi_t begin_change = { box->cursor.position.line - 1, 
			previous->string.count };
		if (dyn_arr_char_add_all(&previous->string, 
					line->string.arr, line->string.count) == -1)
			display_error("Out of memory");
		dyn_arr_char_destroy(&line->string);
		if (dyn_arr_line_remove(lines, box->cursor.position.line) == -1)
			display_error("Out of memory");
		box->cursor.position = begin_change;
		update_changes_from(box, begin_change);
	}
	else {
		if (dyn_arr_char_remove(&line->string, box->cursor.position.char_i - 1) 
				== -1)
			display_error("Out of memory");
		--box->cursor.position.char_i;

		line_chi_t begin_change = { box->cursor.position.line, 
			box->cursor.position.char_i };
		update_changes_from(box, begin_change);
	}
	box->cursor.cursor_x_target = 0;
}


/**
 * updates lines, cursor and screen. Expects box to be editable and in cursor
 * mode.
 */
static void insert_char(text_box_t* box, char c) {
	move_cursor(box, 0);
	line_chi_t* cursor_pos = &box->cursor.position;

	line_t* line = &box->lines.arr[cursor_pos->line];
	if (dyn_arr_char_insert(&line->string, c, cursor_pos->char_i) == -1)
		display_error("Out of memory");
	++cursor_pos->char_i;

	line_chi_t begin = { cursor_pos->line, cursor_pos->char_i - 1 };
	box->cursor.cursor_x_target = 0;
	update_changes_from(box, begin);
}


/**
 * if the interaction_mode isn't CURSOR and editable isn't true, this does
 * nothing
 */
static void handle_char(text_box_t* box, char c) {
	if (box->interaction_mode != CURSOR || !box->cursor.editable)
		return;

	switch(c) {
		case '\n': 
			insert_line_break(box);
			break;
		case '\x08':
			backspace(box);
			break;
		default:
			insert_char(box, c);
			break;
	}
}


/**
 * mode = 1: print, mode = 0: erase. If box isn't in CURSOR mode, nothing
 * happens.
 */
static void move_cursor(text_box_t* box, int mode) {
	if (mode && box->interaction_mode != CURSOR) {
		return;
	}

	point_t px;
	if (!line_chi_to_point(box, box->cursor.position, &px))
		return;
	px.x += box->left_px;
	px.y += box->top_px;

	for (int y = -1; y < (int) CHAR_HEIGHT; ++y)
		Bdisp_SetPoint_DDVRAM(px.x - 1, px.y + y, mode);
}

/**
 * if vline is before the first vvline, it adjusts vvlines so that 
 * vline is the first vvline
 *
 * returns whether it scrolled
 */
static int auto_scroll_up(text_box_t* box, size_t vline) {
	if (vline < box->vvlines_begin) {
		box->vvlines_begin = vline;
		print_lines(box);
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
static int auto_scroll_down(text_box_t* box, size_t vline) {
	if (vline >= box->vvlines_begin + box->height) {
		box->vvlines_begin = vline - box->height + 1;
		print_lines(box);
		return 1;
	}
	return 0;
}

/**
 * updates box and screen. If vvlines_begin is 0 already, nothing happens.
 */
static void scroll_up(text_box_t* box) {
	if (box->vvlines_begin == 0) {
		return;
	}

	--box->vvlines_begin;
	print_lines(box);
}

/**
 * updates box and screen. If vvlines_begin is 0 already, nothing happens.
 */
static void scroll_down(text_box_t* box) {
	line_t* last_line = DYN_ARR_LAST(&box->lines);
	size_t last_vline = last_line->vline_begin + last_line->count_softbreaks;
	size_t last_vvline = box->vvlines_begin + box->height - 1;
	if (last_vvline >= last_vline) {
		return;
	}

	++box->vvlines_begin;
	print_lines(box);
}

/**
 * expects box to be in cursor mode. 
 */
static void handle_cursor_move(text_box_t* box, int move) {
	line_chi_t* cursor_pos = &box->cursor.position;
	move_cursor(box, 0);
	switch (move) {
		case CODE_LEFT:
			if (cursor_pos->char_i > 0) {
				--cursor_pos->char_i;
				size_t vline = line_chi_to_vline(box, *cursor_pos, NULL);
				box->cursor.cursor_x_target = 0;
				auto_scroll_up(box, vline);
			}
			break;
		case CODE_RIGHT:
			if (cursor_pos->char_i < box->lines.arr[cursor_pos->line].string
					.count) {
				++cursor_pos->char_i;
				size_t vline = line_chi_to_vline(box, *cursor_pos, NULL);
				box->cursor.cursor_x_target = 0;
				auto_scroll_down(box, vline);
			}
			break;
		default:
			handle_cursor_move_vertical(box, move);
	}

	move_cursor(box, 1);
}

/**
 * updates cursor_pos according to move. Expects box to be in cursor mode
 */
static void	handle_cursor_move_vertical(text_box_t* box, int move) {
	line_chi_t* cursor_pos = &box->cursor.position;
	dyn_arr_line_t* lines = &box->lines;

	size_t count_softbreaks;
	size_t* vline_starts_old = // vline starts of old line of cursor
		get_vline_starts(&lines->arr[cursor_pos->line], &count_softbreaks);
	size_t new_line_i;
	// offset of the vline of the new position relative to the line's
	// vline_begin
	size_t new_vline_offs; 
	unsigned char x;

	if (move == CODE_UP) {
		if (vline_starts_old == NULL 
				|| cursor_pos->char_i < vline_starts_old[0]) { 
			// in first vline of line
			if (cursor_pos->line == 0)
				return;
			new_line_i = cursor_pos->line - 1;
			new_vline_offs = lines->arr[new_line_i].count_softbreaks;
			x = cursor_pos->char_i;
		}
		else { // we stay in the same line
			new_line_i = cursor_pos->line;
			size_t current_vline = line_chi_to_vline(box, *cursor_pos, &x);
			new_vline_offs = current_vline - lines->arr[new_line_i].vline_begin 
				- 1;
		}
	}
	else { // down
		if (vline_starts_old == NULL 
				|| cursor_pos->char_i >= vline_starts_old[count_softbreaks - 1]) { 
			// last vline of line
			if (cursor_pos->line == lines->count - 1)
				return;
			new_line_i = cursor_pos->line + 1;
			new_vline_offs = 0;
			line_chi_to_vline(box, *cursor_pos, &x);
		}
		else { // we stay in the same line
			new_line_i = cursor_pos->line;
			size_t current_vline = line_chi_to_vline(box, *cursor_pos, &x);
			new_vline_offs = current_vline - lines->arr[new_line_i].vline_begin 
				+ 1;
		}
	}

	unsigned char* cursor_x_target = &box->cursor.cursor_x_target;
	// tentatively set x to max(x, *cursor_x_target)
	if (*cursor_x_target > x)
		x = *cursor_x_target;

	line_t* new_line = &lines->arr[new_line_i];
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
		// cursor_pos->char_i is after last char in line
		cursor_pos->char_i = new_line->string.count;
		*cursor_x_target = x;
	}
	else {
		cursor_pos->char_i = begin_i + x;
	}

	cursor_pos->line = new_line_i;

	if (move == CODE_UP)
		auto_scroll_up(box, new_line->vline_begin + new_vline_offs);
	else 
		auto_scroll_down(box, new_line->vline_begin + new_vline_offs);
}

/**
 * returns the corresponding ascii code, or -1 if this key does not correpond to
 * any ascii code, or -3 to -6 (CODE_{UP, DOWN, RIGHT, LEFT}) when the
 * navigation buttons were used. Backspace is '\x08', enter is '\n'.
 */
static int key_code_to_ascii(text_box_t* box, unsigned int code) {
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
	char* capitalization_on = &box->cursor.editable_state.capitalization_on;
	char* capitalization_on_locked = &box->cursor.editable_state
		.capitalization_on_locked;
	if (ch == capitalization_once) {
		*capitalization_on = 1;
		return -1;
	} else if (ch == capitalization_lock) {
		*capitalization_on_locked ^= 1;
		return -1;
	} else if (ch < 0) {
		return ch;
	}

	if (*capitalization_on || *capitalization_on_locked) {
		*capitalization_on = 0;
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
 */
static point_t char_point_to_point(char_point_t point) {
	point_t ret_val = { EDITOR_LEFT, EDITOR_TOP };
	ret_val.x += point.x * CHAR_WIDTH_OUTER + MARGIN_LEFT;
	ret_val.y += point.y * CHAR_HEIGHT_OUTER + MARGIN_TOP;
	return ret_val;
}

/**
 * returns whether line_chi is currently even visible
 * If line_chi is visible, point is set to the pixel coordinate of top left 
 * corner of that character coordinate.
 */
static char line_chi_to_point(text_box_t* box, line_chi_t line_chi, 
		point_t* point) {
	unsigned char x;
	size_t vline = line_chi_to_vline(box, line_chi, &x);
	
	if (vline < box->vvlines_begin || vline >= box->vvlines_begin + box->height)
		return 0;

	char_point_t ch_point;
	ch_point.x = x;
	ch_point.y = vline - box->vvlines_begin;
	*point = char_point_to_point(ch_point);
	return 1;
}




static int print_line(text_box_t* box, size_t line_i) {
	line_chi_t line_chi = { line_i, 0 };
	return print_partial_line(box, line_chi);
}

/**
 * prints starting from line_chi
 *
 * Return 1 if none of the vlines that should be printed were below what is
 * currently visible, otherwise returns 0.
 */
static int print_partial_line(text_box_t* box, line_chi_t line_chi) {
	dyn_arr_line_t* lines = &box->lines;
	line_t* line = &lines->arr[line_chi.line];
	// exclusive, end of the visible vlines of this line
	size_t vvlines_local_end;	
	if (line_chi.line == lines->count - 1) { // line_chi.line is the last line 
		vvlines_local_end = line->vline_begin + line->count_softbreaks + 1;
	}
	else
		vvlines_local_end = lines->arr[line_chi.line + 1].vline_begin;

	if (vvlines_local_end > box->vvlines_begin + box->height) 
		// line ends outside of visible area
		vvlines_local_end = box->vvlines_begin + box->height;

	// find vline and x value that correspond to line_chi or if these aren't
	// visible, the first that are.
	size_t char_i; // index in the string
	unsigned char x;
	size_t vline = line_chi_to_vline(box, line_chi, &x); 
	if (vline < box->vvlines_begin) {
		if (line->vline_begin + line->count_softbreaks < box->vvlines_begin)
			// even last vline of line isn't visible, nothing to print
			return 0;
		// first vline is before vvlines_begin, last is not
		// => vvlines_begin is a vline of the line
		vline = box->vvlines_begin;
		x = 0;
		// find new starting index
		size_t* vline_starts = get_vline_starts(line, NULL);
		char_i = vline_starts[vline - line->vline_begin - 1];
	}
	else
		char_i = line_chi.char_i;


	// print chars of each vline 
	while (char_i < line->string.count && vline < vvlines_local_end) {
		if (x >= box->width) { // next vline
			x = 0;
			++vline;
			continue;
		}
		print_char_xy(box->left_px, box->top_px, x, vline - box->vvlines_begin, 
				line->string.arr[char_i], 0);
		++char_i;
		++x;
	}

	if (vline >= vvlines_local_end)
		// line out of screen space
		return 0;

	while (x < box->width) {
		print_char_xy(box->left_px, box->top_px, x, vline - box->vvlines_begin, 
				' ', 0);
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


		print_char(0, 0, current_pos, str[i], 0);
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

/**
 * prints all lines in box and makes sure that beyond the end of the line there
 * is only spaces. Does not clear cursor, or anything else between the
 * characters
 */
static void print_lines(text_box_t* box) {
	size_t first_line = dyn_arr_line_bsearch_cb(&box->lines, 
			(void*) &box->vvlines_begin, &compare_lines_vline_begin);
	for (size_t i = first_line; i < box->lines.count; ++i)
		if (!print_line(box, i))
			break;
}


static void 
print_char_xy(unsigned short offs_x, unsigned short offs_y, unsigned char x, 
		unsigned char y, char c, char negative) {
	char_point_t point = { x, y };
	print_char(offs_x, offs_y, point, c, negative);
}

/**
 * if negative true, the colors will be inversed
 */
static void print_char(unsigned short offs_x, unsigned short offs_y, 
		char_point_t point, char c, char negative) {
	point_t px = char_point_to_point(point);
	px.x += offs_x;
	px.y += offs_y;
	for (unsigned int y = 0; y < CHAR_HEIGHT; ++y) {
		for (unsigned int x = 0; x < CHAR_WIDTH; ++x) {
			Bdisp_SetPoint_DDVRAM(px.x + x, px.y + y, 
					font[(unsigned char) c][y][x] ^ negative);
		}
	}
}

/**
 * updates vline_indices, cursor and screen according to a change that starts at
 * begin. begin is used to determine the last vline whose index in vline_index
 * is still correct. The cursor should be at the correct position already, 
 * to determine if scrolling is necessary. 
 * The vlines of the subsequent lines are set so one directly follows its 
 * predecessor's last vline. 
 */
static void update_changes_from(text_box_t* box, line_chi_t begin) {
	// find vline of begin
	dyn_arr_line_t* lines = &box->lines;
	size_t vline = line_chi_to_vline(box, begin, NULL);
	line_t* line = &lines->arr[begin.line];

	// recalculate vline_index of line, starting from vline's successor
	recalculate_vline_index(box, line, vline - line->vline_begin);
	// shift all subsequent vline_indices if necessary
	char shifted = 0;
	for (size_t line_i = begin.line + 1;
			line_i < lines->count; 
			++line_i) {
		line_t* last_line = &lines->arr[line_i - 1];
		size_t new_vline_begin = last_line->vline_begin 
			+ last_line->count_softbreaks + 1;
		line_t* shift_line = &lines->arr[line_i];
		ptrdiff_t offset = new_vline_begin - shift_line->vline_begin;
		shifted |= offset != 0;
		shift_line->vline_begin += offset;
	}

	// scroll if necessary
	size_t new_vline = line_chi_to_vline(box, box->cursor.position, NULL);
	line_t* last_line = DYN_ARR_LAST(lines);
	size_t last_vline = last_line->count_softbreaks + last_line->vline_begin;
	char scrolled = 1;
	if (!auto_scroll_down(box, new_vline)) {
		if (last_vline >= box->height - 1) { 
			// without this the last lines could be blank when deleting
			if (!auto_scroll_up(box, last_vline - (box->height - 1))) {
				if (!auto_scroll_up(box, new_vline)) {
					scrolled = 0;
				}
			}
		}
		else {
			if (!auto_scroll_up(box, new_vline)) {
				scrolled = 0;
			}
		}
	}
	if (!scrolled) {
		// did not scroll, need to print manually
		print_partial_line(box, begin);
		if (shifted) {
			for (size_t line_i = begin.line + 1;
					line_i < lines->count; 
					++line_i) {
				if (!print_line(box, line_i))
					// line_i was (partially) below visible area
					break;
			}
		}
	}


	move_cursor(box, 1);
}

/**
 * This function tries to write the box's contents into buf. In
 * any case buf will be null terminated.
 * If the text of box (plus '\0') does not fit into buf, only the 
 * first (buf_size - 1) bytes are written, the last byte is '\0'.
 * The function returns how many bytes (excluding '\0') would have written 
 * or have been written when all could be written. 
 */
size_t get_text_box_string(const text_box_t* box, char buf[], size_t buf_size) {
	if (box->lines.count == 0) {
		if (buf_size > 0) {
			buf[0] = '\0';
		}
		return 0;
	}

	// iterate over all characters in all lines until either the buffer's or the
	// lines end is reached
	size_t buf_i = 0;
	line_chi_t cl_pos = { 0, 0 };
	line_t* current_line = &box->lines.arr[0];
	for (;buf_i < buf_size - 1; ++buf_i) {
		// increment cl_pos on linebreak
		if (cl_pos.char_i == current_line->string.count) {
			// reached end of box
			if (cl_pos.line == box->lines.count - 1) {
				break;
			}
			// continue with next line
			cl_pos.char_i = 0;
			++cl_pos.line;
			buf[buf_i] = '\n';
			current_line = &box->lines.arr[cl_pos.line];
			continue;
		}

		buf[buf_i] = current_line->string.arr[cl_pos.char_i];

		++cl_pos.char_i;
	}

	// could fit all bytes
	// both when the condition of the for loop triggered and when we break,
	// cl_pos must be after the last char of the last line (we don't set
	// cl_pos.char_i to zero if the last line was reached). We also know that
	// the third state in the for-header was executed one last time after the 
	// last byte was written.
	if (cl_pos.char_i >= current_line->string.count 
			&& cl_pos.line == box->lines.count - 1) {
		buf[buf_i] = '\0';
		return buf_i;
	}

	buf[buf_size - 1] = '\0';
	size_t sum_chars = box->lines.count - 1; // for the line breaks
	for (size_t i = 0; i < box->lines.count; ++i) {
		sum_chars += box->lines.arr[i].string.count;
	}
	return sum_chars;
}
