#include "editor.h"
#include "editor_render.h"
#include "fxlib.h"
#include "line_utils.h"
#include "line.h"
#include "clipboard.h"

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
 *
 */

// static function declarations
static int key_code_to_ascii(text_box_t* box, unsigned int code);
static void handle_char(text_box_t* box, char c);
static void handle_cursor_move(text_box_t* box, editor_code_t move);
static void	handle_cursor_move_vertical(text_box_t* box, int move);
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
static void scroll_up(text_box_t* box, size_t n);
static void scroll_down(text_box_t* box, size_t n);
static void handle_editor_code(text_box_t* box, editor_code_t code);
static void start_selection(text_box_t* box);
static void cancel_selection(text_box_t* box);
static void copy_selection(text_box_t* box);
static void paste_clipboard(text_box_t* box);
static void delete_selection(text_box_t* box);
static void delete_range(text_box_t* box, const line_chi_t* start, 
		const line_chi_t* end);
static void insert_string(text_box_t* box, const char* str, int move_cursor);
static void abort_model_change(text_box_t* box, const char* msg);


/**
 * sets box->redraw_areas to a state that states that nothing has changed and
 * otherwise reflects the current editor state (in particular old_cursor =
 * cursor.position and selection_cursor is set to the correct value)
 */
static void reinit_changes(text_box_t* box) {
	redraw_areas_t* ras = &box->redraw_areas;
	line_chi_to_char_point(box, box->cursor.position, &ras->old_cursor, 1);
	ras->vvlines_begin_changed = 0;
	ras->changes_begin.line = 0;
	ras->changes_begin.char_i = 0;
	ras->changes_end = ras->changes_begin;
	ras->selection_cursor = box->interaction_mode == CURSOR 
		&& box->cursor.visual_mode
		&& line_chi_equals(box->cursor.position, box->cursor.selection_begin);
}

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

/**
 * Prints the entire text box, not including the cursor. Does not clear the area
 * first.
 */
void draw_text_box(const text_box_t* box) {
	print_lines(box);
}

unsigned int focus_text_box(text_box_t* box, unsigned int escape_keys[], 
		unsigned int count_escape_keys) {
	unsigned int ret_val;
	while (1) {
		unsigned int key;
		redraw_changes(box);
		reinit_changes(box);
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
		} else {
			handle_editor_code(box, (editor_code_t) c);
		}
	}
end_while:
	
	move_cursor(box, 0);
	return ret_val;
}


/**
 * creates and initializes text box. The first vvline is 0, cursor_pos is (0, 0)
 * and there is one empty line. If interaction_mode isn't CURSOR, editable is
 * ignored. Nothing is drawn yet.
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
	box.cursor.visual_mode = 0;
	box.redraw_areas.vvlines_begin_changed = 0;
	box.redraw_areas.changes_begin.line = 0;
	box.redraw_areas.changes_begin.char_i = 0;
	box.redraw_areas.changes_end = box.redraw_areas.changes_begin;
	box.redraw_areas.old_cursor.y = 0;
	box.redraw_areas.old_cursor.x = 0;
	initialize_lines(&box, text);
	if (box.lines.count == 0) {
		add_new_line(&box, 0);
	}
	return box;
}


void initialize_lines(text_box_t* box, const char* str) {
	if(dyn_arr_line_create(10, &box->lines) == -1)
		display_fatal_error(MSG_ENOMEM);

// #define A
#ifdef A
	add_new_line(box, 0);
	insert_string(box, str, 0);
#endif
	
#ifndef A
	dyn_arr_line_t* lines = &box->lines;
	line_t* current_line = DYN_ARR_LAST(lines);
	unsigned char col = 0;
	size_t next_vline_begin = 0;
	size_t vline = 0;
	size_t i = 0;
	size_t line_starting_index = 0; // index of the first char in str in line
	for (;; ++i) {
		if (str[i] == '\0' || str[i] == '\n') {
			add_new_line(box, next_vline_begin);
			current_line = DYN_ARR_LAST(lines);
			size_t len = i - line_starting_index;
			if (len > 0) {
				current_line->str = (char*) malloc(len);
				if (!current_line->str) {
					display_fatal_error(MSG_ENOMEM);
				}
				memcpy(current_line->str, &str[line_starting_index], len);
				current_line->str[len - 1] |= 0x80;
			}
			if (str[i] == '\0') {
				break;
			}
			
			++vline;
			next_vline_begin = vline;
			line_starting_index = i + 1;
			col = 0;
		} 
		else {
			// only check if col is out of bounds before reading new char. This
			// prevents unnecessary soft breaks before '\n' and EOF
			if (col >= box->width) { 
				col = 0;
				++vline;
			}
			++col;
		}
	}
#endif
#undef A
}

/**
 * Expect box to be editable and in cursor mode
 */
static void backspace(text_box_t* box) {
	if (box->cursor.position.char_i == 0 && box->cursor.position.line == 0) {
		return;
	}

	line_chi_t begin = line_chi_decrement(box, &box->cursor.position);
	delete_range(box, &begin, &box->cursor.position);
}


/**
 * expects start and end to both point chars (or linebreaks) within the box.
 * Expects box to be in cursor mode and editable.
 * @param start inclusive
 * @param end exclusive
 */
static void delete_range(text_box_t* box, const line_chi_t* begin, 
		const line_chi_t* end) {
	if (!line_chi_greater_than(*end, *begin)) {
		return;
	}
	line_t* first = &box->lines.arr[begin->line]; // first line in range
	line_t* last = &box->lines.arr[end->line]; // line containing suffix to
											   // concat the prefix of first 
											   // with
	size_t first_len = find_line_len(first);
	size_t last_len = find_line_len(last);
	if (first == last) {
		// simple case, just remove the chars from that line since we don't need
		// to merge lines
		if (line_remove_some(first, begin->char_i, 
					end->char_i) == -1) {
			display_fatal_error(MSG_ENOMEM);
		}
	} else {
		// remove suffix from first, concat first with suffix of last, then
		// remove [begin->line + 1, end->line + 1) from box->lines.
		if (line_remove_some(first, begin->char_i, 
					first_len) == -1
				|| line_add_all(first, &last->str[end->char_i],
					last_len - end->char_i) == -1) {
			display_fatal_error(MSG_ENOMEM);
		}
		for (size_t i = begin->line + 1;
				i < end->line + 1 && i < box->lines.count; ++i) {
			destruct_line(&box->lines.arr[i]);
		}
		if (dyn_arr_line_remove_some(&box->lines, begin->line + 1, 
					end->line + 1) == -1) {
			display_fatal_error(MSG_ENOMEM);
		}
	}

	box->cursor.position = *begin;
	box->cursor.cursor_x_target = 0;
	line_chi_t changes_begin = *begin;
	// in case be remove the last char left in a vline
	if (begin->char_i != 0) {
		--changes_begin.char_i;
	}
	update_changes_from(box, changes_begin);

	if (first != last) {
		box->redraw_areas.changes_end.line = box->lines.count;
		box->redraw_areas.changes_end.char_i = 0;

	}
}

/**
 * Expects box to be editable and in cursor
 * mode.
 */
static void insert_char(text_box_t* box, char c) {
	char str[] = { c, '\0' };
	insert_string(box, str, 1);
}

/**
 * doesn't check whether box is editable, but works as well if it isn't,
 * provided that cursor is set to the correct value.
 * @param move_cursor iff true, the cursor will be move to be before the first
 * char after the inserted string
 */
static void insert_string(text_box_t* box, const char* str, int move_cursor) {
	line_chi_t* cursor_pos = &box->cursor.position;
	line_t* line = &box->lines.arr[cursor_pos->line];
	size_t len = strlen(str);

	// remove suffix of first line, store it somewhere else. Iteratively add
	// the strings between the linebreaks in str to the current line. After
	// the str has been completely added, append the old suffix from the
	// first line to the current line
	size_t line_len = find_line_len(line);
	size_t cursor_chi = cursor_pos->char_i;
	if (cursor_chi > line_len) {
#ifdef MOCKUP
		Print((const unsigned char*) "editor.cpp, insert_string: cursor_chi "
				"> line_len");
#endif
		cursor_chi = line_len;
	}
	const size_t suffix_len = line_len - cursor_chi;
	char* suffix = (char*) malloc(suffix_len);
	if (!suffix && suffix_len > 0) {
		display_error(MSG_ENOMEM);
		return;
	}
	memcpy(suffix, &line->str[cursor_chi], suffix_len);
	if (line_pop_some(line, suffix_len) == -1) {
		display_error(MSG_ENOMEM);
		free(suffix);
		line_chi_t zero = { 0, 0 };
		update_changes_from(box, zero);
		return;
	}

	const char* str_part = str; // part of string between '\n' that we want to
								// add at the moment
	size_t cur_i = cursor_pos->line; // index of current line. Should have the
									 // value of the last iteration after the
									 // loop
	line_t* cur = &box->lines.arr[cur_i];	// line that we want to add to.
											// Should have the value of the last
											// iteration after the loop
	const char* next_br;		// ptr to next '\n', or to the '\0' if there is
								// no (further or at all)
	int first = 1;
	int reached_end = 0; 	// set to 1 to end after iteration
	do {
		next_br = strstr(str_part, "\n");
		if (!next_br) {
			// set next_br to char after last char of str in last iteration
			next_br = str + len;
			reached_end = 1;
		}

		const size_t str_part_len = next_br - str_part;
		if (first) {
			first = 0;
		} else {
			// create new line
			++cur_i;
			line_t new_line_stack = {0};
			if (dyn_arr_line_insert(&box->lines, new_line_stack, cur_i) 
					== -1) {
				free(suffix);
				abort_model_change(box, MSG_ENOMEM);
				return;
			}
			// init line
			cur = &box->lines.arr[cur_i];
			cur->vline_begin = 0;
			cur->str = NULL;
			size_t new_cap;
			if (line_ensure_capacity(cur, 0, 0, str_part_len, &new_cap) 
					== -1) {
				free(suffix);
				abort_model_change(box, MSG_ENOMEM);
				return;
			}
		}
		// append str_part (in all but the first this will be the entire
		// line)
		if (line_add_all(cur, str_part, str_part_len) == -1) {
			free(suffix);
			abort_model_change(box, MSG_ENOMEM);
			return;
		}

		str_part = next_br + 1;
	} while (!reached_end);

	const size_t new_cursor_chi = find_line_len(cur);
	// add suffix to last line
	if (line_add_all(cur, suffix, suffix_len) == -1) {
		display_error(MSG_ENOMEM);
	}

	line_chi_t changes_begin = *cursor_pos;
	if (changes_begin.char_i > 0) {
		--changes_begin.char_i;
	}
	if (move_cursor) {
		cursor_pos->line = cur_i;
		cursor_pos->char_i = new_cursor_chi;
	}
	update_changes_from(box, changes_begin);
	
	free(suffix);
}

static void abort_model_change(text_box_t* box, const char* msg) {
	display_error(msg);
	line_chi_t zero = { 0, 0 };
	update_changes_from(box, zero);
}

/**
 * if the interaction_mode isn't CURSOR and editable isn't true, or if the box
 * is in visual mode, this does nothing
 */
static void handle_char(text_box_t* box, char c) {
	if (box->interaction_mode != CURSOR || !box->cursor.editable 
			|| (box->cursor.visual_mode && c != '\x08'))
		return;

	switch(c) {
		case '\x08':
			if (box_is_in_visual_mode(box)) {
				delete_selection(box);
			} else {
				backspace(box);
			}
			break;
		default:
			insert_char(box, c);
			break;
	}
}

/**
 * use auto_scroll_up instead.
 *
 * if vline is before the first vvline, it adjusts vvlines so that 
 * vline is the first vvline
 *
 * returns whether it scrolled
 */
static int auto_scroll_up_simple(text_box_t* box, size_t vline) {
	if (vline < box->vvlines_begin) {
		box->redraw_areas.vvlines_begin_changed = 1;
		box->vvlines_begin = vline;
		return 1;
	}
	return 0;
}

/**
 * if vline is before the first vvline, it adjusts vvlines so that 
 * vline is the first vvline. It also makes sure that there are as few as
 * possible empty
 * lines beyond vline.
 *
 * returns whether it scrolled
 */
static int auto_scroll_up(text_box_t* box, size_t vline) {
	line_t* last_line = DYN_ARR_LAST(&box->lines);
	size_t last_vvline = last_line->vline_begin 
		+ count_softbreaks(box, last_line);
	if (last_vvline < vline) {
		last_vvline = vline;
	}
	// make sure there aren't any non-existing lines at the bottom
	size_t scroll_target = 0;
	if (last_vvline + 1 >= box->height) {
		scroll_target = last_vvline + 1 - box->height;
	}
	char scrolled = 0;
	scrolled |= auto_scroll_up_simple(box, scroll_target);
	scrolled |= auto_scroll_up_simple(box, vline);
	return scrolled;
}

/**
 * if vline is after the last vvline, it adjusts vvlines so that 
 * vline is the last vvline
 *
 * returns whether it scrolled
 */
static int auto_scroll_down(text_box_t* box, size_t vline) {
	if (vline >= box->vvlines_begin + box->height) {
		box->redraw_areas.vvlines_begin_changed = 1;
		box->vvlines_begin = vline - box->height + 1;
		return 1;
	}
	return 0;
}

/**
 * updates box. If vvlines_begin is 0 already, nothing happens.
 */
static void scroll_up(text_box_t* box, size_t n) {
	if (box->vvlines_begin < n) {
		n = box->vvlines_begin;
	}

	box->vvlines_begin -= n;
	box->redraw_areas.vvlines_begin_changed = 1;
}

/**
 * updates box. If vvlines_begin is 0 already, nothing happens.
 */
static void scroll_down(text_box_t* box, size_t n) {
	line_t* last_line = DYN_ARR_LAST(&box->lines);
	size_t last_vline = last_line->vline_begin 
		+ count_softbreaks(box, last_line);
	size_t last_vvline = box->vvlines_begin + box->height - 1;
	if (last_vvline + n > last_vline) {
		if (last_vline < last_vvline) {
			n = 0;
		} else {
			n = last_vline - last_vvline;
		}
	}

	box->vvlines_begin += n;
	box->redraw_areas.vvlines_begin_changed = 1;
}

/**
 * expects box to be in cursor mode. 
 * @param move which type of move 
 */
static void handle_cursor_move(text_box_t* box, editor_code_t move) {
	line_chi_t* cursor_pos = &box->cursor.position;
	line_chi_t old_cursor = *cursor_pos;
	if (!box->cursor.visual_mode) {
		// mark no changes
		box->redraw_areas.changes_begin.line = 0;
		box->redraw_areas.changes_begin.char_i = 0;
		box->redraw_areas.changes_end = box->redraw_areas.changes_begin;
	}
	switch (move) {
		case CODE_LEFT:
		{
			if (cursor_pos->char_i == 0 && cursor_pos->line == 0)
				break;

			if (cursor_pos->char_i > 0) {
				--cursor_pos->char_i;
			} else if (cursor_pos->line > 0) {
				--cursor_pos->line;
				cursor_pos->char_i = find_line_len(
						&box->lines.arr[cursor_pos->line]);
			}
			size_t vline = line_chi_to_vline(box, *cursor_pos, NULL, 1);
			box->cursor.cursor_x_target = 0;
			auto_scroll_up(box, vline);
			break;
		}
		case CODE_RIGHT:
		{
			size_t old_line_len = find_line_len(
					&box->lines.arr[cursor_pos->line]);
			if (cursor_pos->char_i == old_line_len
					&& cursor_pos->line == box->lines.count - 1)
				break;

			if (cursor_pos->char_i < old_line_len) {
				++cursor_pos->char_i;
			} else if (cursor_pos->line < box->lines.count - 1) {
				++cursor_pos->line;
				cursor_pos->char_i = 0;
			}
			size_t vline = line_chi_to_vline(box, *cursor_pos, NULL, 1);
			box->cursor.cursor_x_target = 0;
			auto_scroll_down(box, vline);
			break;
		}
		default:
			handle_cursor_move_vertical(box, move);
	}
	if (box->cursor.visual_mode) {
		// set changes as ranging from old cursor position to new cursor
		// position (or the other way round)
		line_chi_min_max(&box->cursor.position, &old_cursor, 
				&box->redraw_areas.changes_begin,
				&box->redraw_areas.changes_end);
		if (line_chi_greater_than(box->cursor.selection_begin, 
					box->cursor.position)) {
			// fix the left margin of the first and previously first char of
			// selection
			if (move == CODE_UP || move == CODE_LEFT || move == CODE_PAGE_UP) {
				// include previous first char of selection in changes since the
				// left margin needs to be printed over 
				const line_t* const old_line = &box->lines.arr[old_cursor.line];
				const size_t old_line_len = find_line_len(old_line);
				// increment changes_end
				// <= because that's allowed (due to the cursor)
				if (old_cursor.char_i + 1 <= old_line_len) {
					++box->redraw_areas.changes_end.char_i;
				} else {
					box->redraw_areas.changes_end.char_i = 0;
					box->redraw_areas.changes_end.line = old_cursor.line + 1;
					if (old_cursor.char_i == old_line_len
							&& old_cursor.line + 1 < box->lines.count
							&& 1 < find_line_len(
								&box->lines.arr[old_cursor.line + 1])) {
						// we were in the next vline due to cursor overflow so 
						// we need to redraw the first char of the next line 
						// (which exists) as well
						box->redraw_areas.changes_end.char_i = 1;
					}
				}
			} else {
				// include character after new cursor (first char in sel) as
				// well, in order to clear the left margin
				line_chi_t* const c_end = &box->redraw_areas.changes_end;
				line_t* const cur_line = &box->lines.arr[c_end->line];
				const size_t cur_line_len = find_line_len(cur_line);
				if (c_end->char_i + 1 <= cur_line_len) {
					++c_end->char_i;
				} else {
					if (c_end->char_i == cur_line_len
							&& c_end->line + 1 < box->lines.count
							&& 1 < find_line_len(
								&box->lines.arr[c_end->line + 1])) {
						// we were in the next vline due to cursor overflow so 
						// we need to redraw the first char of the next line 
						// (which exists) as well
						c_end->char_i = 1;
					} else {
						c_end->char_i = 0;
					}
					++c_end->line;
				}
			}
		}
	}
}

/**
 * updates cursor_pos according to move. Expects box to be in cursor mode
 */
static void	handle_cursor_move_vertical(text_box_t* box, int move) {
	line_chi_t* cursor_pos = &box->cursor.position;
	dyn_arr_line_t* lines = &box->lines;
	line_t* current_line = &box->lines.arr[cursor_pos->line];

	// index of new line
	size_t new_line_i;
	// offset of the vline of the new position relative to the line's
	// vline_begin
	size_t new_vline_offs; 
	// alias for cursor_x_target in box
	unsigned char* cursor_x_target = &box->cursor.cursor_x_target;
	// visual cursor column
	unsigned char target_x;

	unsigned char current_x;
	const size_t current_vline = 
		line_chi_to_vline(box, *cursor_pos, &current_x, 1);
	const size_t n = ((move == CODE_UP || move == CODE_DOWN) ? 1 : box->height);
	size_t target_vline; // the vline index the cursor should be at
	if (move == CODE_UP || move == CODE_PAGE_UP) {
		if (current_vline == current_line->vline_begin 
				&& cursor_pos->line == 0)
			// we're in first of vline of box, do nothing
			return;
		// saturate target_vline at 0
		if (current_vline < n) {
			target_vline = 0;
		} else {
			target_vline = current_vline - n;
		}
	} else {
		const line_t* last_line = DYN_ARR_LAST(&box->lines);
		const size_t last_vline =
			last_line->vline_begin + count_softbreaks(box, last_line);
		// saturate target_vline at last_vline
		if (current_vline + n > last_vline) {
			target_vline = last_vline;
		} else {
			target_vline = current_vline + n;
		}
	}

	new_line_i = dyn_arr_line_bsearch_cb(&box->lines, 
			&target_vline, &compare_lines_vline_begin);
	new_vline_offs = target_vline - lines->arr[new_line_i].vline_begin;
	target_x = current_x;

	// tentatively set target_x to max(target_x, *cursor_x_target)
	if (*cursor_x_target > target_x)
		target_x = *cursor_x_target;

	const line_t* const new_line = &lines->arr[new_line_i];
	const size_t new_line_len = find_line_len(new_line);

	// figure out cursor_pos.char_i
	// index in line of first char in vline
	size_t begin_i = vline_offset_to_char_i(box, new_line, new_vline_offs);
	// number of chars in vline
	size_t vline_length = new_line_len - begin_i;
	if (target_x >= vline_length) { // there is no char at x pos in the vline,
		// cursor_pos->char_i is after last char in line
		cursor_pos->char_i = new_line_len;
		// store ideal column as cursor_x_target
		if (vline_length < box->width) {
			*cursor_x_target = target_x;
		} else {
			*cursor_x_target = 0;
		}
	}
	else {
		cursor_pos->char_i = begin_i + target_x;
	}

	cursor_pos->line = new_line_i;
	size_t cursor_vline = line_chi_to_vline(box, *cursor_pos, NULL, 1);

	if (move == CODE_UP || move == CODE_PAGE_UP)
		auto_scroll_up(box, cursor_vline);
	else 
		auto_scroll_down(box, cursor_vline);
}

/**
 * returns the corresponding ascii code or a value in enum editor_code (all
 * negative). Backspace is '\x08', enter is '\n'.
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
	X(DEL, '\x08') \
	X(F3, CODE_TOGGLE_SELECTION) \
	X(F4, CODE_PAGE_UP) \
	X(F5, CODE_PAGE_DOWN) \
	X(CLIP, CODE_COPY) \
	X(PASTE, CODE_PASTE)
	signed char ch;
	switch (code) {
#define X(c, c_literal) case EVAL(EVAL(KEY_CHAR_##c)): ch = c_literal; break;
		EASY_CASES
#undef X
#define X(c, c_literal) case EVAL(EVAL(KEY_CTRL_##c)): ch = c_literal; break;
		EASY_CASES_CTRL
#undef X
#undef EASY_CASE
		default: return CODE_NONE;
	}
	char* capitalization_on = &box->cursor.editable_state.capitalization_on;
	char* capitalization_on_locked = &box->cursor.editable_state
		.capitalization_on_locked;
	if (ch == capitalization_once) {
		*capitalization_on = 1;
		return CODE_NONE;
	} else if (ch == capitalization_lock) {
		*capitalization_on_locked ^= 1;
		return CODE_NONE;
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



char line_chi_to_char_point(const text_box_t* box, line_chi_t line_chi, 
		char_point_t* point, int cursor) {
	unsigned char x;
	size_t vline = line_chi_to_vline(box, line_chi, &x, cursor);
	
	if (vline < box->vvlines_begin 
			|| vline >= box->vvlines_begin + box->height)
		return 0;

	point->x = x;
	point->y = vline - box->vvlines_begin;
	if (x == box->width) {
		return 0;
	}
	return 1;
}

/**
 * updates vline_begins of lines and cursor according to a change that starts at
 * begin. begin is used to determine the last vline whose index in vline_index
 * is still correct. The cursor should be at the correct position already, 
 * to determine if scrolling is necessary. 
 * The vlines of the subsequent lines are set so one directly follows its 
 * predecessor's last vline. 
 */
static void update_changes_from(text_box_t* box, line_chi_t begin) {
	// find vline of begin
	dyn_arr_line_t* lines = &box->lines;
	const line_t* const line = &lines->arr[begin.line];

	const size_t line_last_vline 
		= line->vline_begin + count_softbreaks(box, line);
	// shift all subsequent vline_indices if necessary
	char shifted = 0;
	if (begin.line + 1 < lines->count) {
		shifted = line_last_vline + 1 != lines->arr[begin.line + 1].vline_begin;
	}
	for (size_t line_i = begin.line + 1;
			line_i < lines->count; 
			++line_i) {
		line_t* last_line = &lines->arr[line_i - 1];
		size_t new_vline_begin = last_line->vline_begin 
			+ count_softbreaks(box, last_line) + 1;
		line_t* shift_line = &lines->arr[line_i];
		shifted |= (new_vline_begin != shift_line->vline_begin);
		shift_line->vline_begin = new_vline_begin;
	}

	// scroll if necessary
	size_t new_vline = line_chi_to_vline(box, box->cursor.position, NULL, 1);
	line_t* last_line = DYN_ARR_LAST(lines);
	size_t last_vline = count_softbreaks(box, last_line) 
		+ last_line->vline_begin;
	if (last_vline < new_vline) {
		// the cursor can be after the last line
		last_vline = new_vline;
	}

	if (!auto_scroll_down(box, new_vline)) {
		auto_scroll_up(box, new_vline);
	}

	if (!box->redraw_areas.vvlines_begin_changed) {
		// did not scroll, need to print manually
		box->redraw_areas.changes_begin = begin;
		if (shifted) {
			box->redraw_areas.changes_end.line = lines->count;
			box->redraw_areas.changes_end.char_i = 0;
		} else {
			box->redraw_areas.changes_end = begin;
			++box->redraw_areas.changes_end.line;
			box->redraw_areas.changes_end.char_i = 0;
		}
	}
}

line_chi_t line_chi_decrement(const text_box_t* box, const line_chi_t* lc) {
	line_chi_t lci = *lc; // inclusive end
	if (lc->char_i == 0) {
		--lci.line;
		lci.char_i = find_line_len(&box->lines.arr[lci.line]);
	} else {
		--lci.char_i;
	}
	return lci;
}

size_t get_text_box_string(const text_box_t* box, char buf[], size_t buf_size) {
	line_chi_t begin = { 0, 0 };
	line_chi_t end = { box->lines.count, 0 };
	return get_text_box_partial_string(box, buf, buf_size, &begin, &end);
}

size_t get_text_box_partial_string(const text_box_t* box, char buf[], 
		size_t buf_size, line_chi_t* begin, line_chi_t* end) {
	line_chi_t real_end;
	line_chi_t abs_end = { box->lines.count - 1, 
			find_line_len(DYN_ARR_LAST(&box->lines)) };
	line_chi_min_max(end, &abs_end, &real_end, NULL);
	if (box->lines.count == 0 
			|| begin->line >= box->lines.count
			|| !line_chi_greater_than(real_end, *begin)) {
		if (buf_size > 0) {
			buf[0] = '\0';
		}
		return 0;
	}

	// iterate over all characters in all lines starting at begin, until either 
	// the buffer's end, the lines' end, or `real_end` is reached
	size_t buf_i = 0;
	line_chi_t cl_pos = *begin;
	line_t* current_line = &box->lines.arr[cl_pos.line];
	for (; buf_i + 1 < buf_size
			&& line_chi_greater_than(real_end, cl_pos); ++buf_i) {
		if (is_line_end(current_line->str, cl_pos.char_i)) {
			if (cl_pos.line >= box->lines.count - 1) {
				// reached real_end of box
				break;
			}
			// continue with next line
			cl_pos.char_i = 0;
			++cl_pos.line;
			buf[buf_i] = '\n';
			current_line = &box->lines.arr[cl_pos.line];
			continue;
		}

		buf[buf_i] = (current_line->str[cl_pos.char_i] & ~0x80);

		++cl_pos.char_i;
	}

	
	if (!line_chi_greater_than(real_end, cl_pos)) { 
		// cl_pos >= real_end, i.e. we wrote every char. There's also one
		// guaranteed index for NULL terminator due to the condition in the 
		// for loop (except when buf_size == 0). This index is buf_i since 
		// we always increment buf_i after we write.
		if (buf_size > 0) {
			buf[buf_i] = '\0';
		}
		return buf_i;
	}

	// we didn't write every => set terminator, figure out necessary buffer size
	// and return it
	if (buf_size > 0) {
		buf[buf_size - 1] = '\0';
	}
	size_t sum_chars = 0; 
	if (begin->line == real_end.line) {
		sum_chars += real_end.char_i - begin->char_i;
	} else {
		sum_chars += find_line_len(&box->lines.arr[begin->line]) - begin->char_i 
			+ 1; // +1 for line break
		for (size_t i = begin->line + 1; i < real_end.line; ++i) {
			sum_chars += find_line_len(&box->lines.arr[i]);
			sum_chars += 1; // for line break
		}
		sum_chars += real_end.char_i + 1;
	}
	return sum_chars;
}

int box_is_in_visual_mode(const text_box_t* box) {
	return box->interaction_mode == CURSOR && box->cursor.visual_mode;
}

int box_is_editable(const text_box_t* box) {
	return box->interaction_mode == CURSOR && box->cursor.editable;
}

static void handle_editor_code(text_box_t* box, editor_code_t code) {
	if (box_is_editable(box) && code == CODE_PASTE) {
		paste_clipboard(box);
	} else if (box->interaction_mode == CURSOR) {
		if (code == CODE_UP || code == CODE_LEFT 
				|| code == CODE_DOWN || code == CODE_RIGHT 
				|| code == CODE_PAGE_UP || code == CODE_PAGE_DOWN) {
			handle_cursor_move(box, code);
		} else if (box->cursor.visual_mode) {
			if (code == CODE_TOGGLE_SELECTION) {
				cancel_selection(box);
			} else if (code == CODE_COPY) {
				copy_selection(box);
			} 
		} else if (code == CODE_TOGGLE_SELECTION) {
			start_selection(box);
		}
	} else {
		switch (code) {
			case CODE_UP:
				scroll_up(box, 1);
				break;
			case CODE_DOWN:
				scroll_down(box, 1);
				break;
			case CODE_PAGE_UP:
				scroll_up(box, box->height);
				break;
			case CODE_PAGE_DOWN:
				scroll_down(box, box->height);
				break;
			default:
				break;
		}
	}
}

/**
 * expects box to be in cursor mode
 */
static void start_selection(text_box_t* box) {
	box->cursor.selection_begin = box->cursor.position;
	box->cursor.visual_mode = 1;
}

/**
 * expects box to be in cursor mode
 */
static void cancel_selection(text_box_t* box) {
	line_chi_min_max(&box->cursor.position, &box->cursor.selection_begin, 
			&box->redraw_areas.changes_begin, &box->redraw_areas.changes_end);
	box->cursor.visual_mode = 0;
}

/**
 * expects box to be in selection mode
 */
static void copy_selection(text_box_t* box) {
	char buf[1024];
	line_chi_t begin;
	line_chi_t end;
	line_chi_min_max(&box->cursor.position, &box->cursor.selection_begin, 
			&begin, &end);
	size_t req_size = 
		get_text_box_partial_string(box, buf, sizeof(buf), &begin, &end) + 1;
	char* text = buf;
	if (req_size > sizeof(buf)) {
		char* tmp = (char*) malloc(req_size);
		if (!tmp && req_size > 0) {
			display_error("Could not copy to clipboard: Not enough memory");
			return;
		}
		text = tmp;
		get_text_box_partial_string(box, text, req_size, &begin, &end);
	}
	copy_to_clipboard(text, req_size - 1);
	if (req_size > sizeof(buf)) {
		free(text);
	}
}

/**
 * expects box to be in cursor mode and editable
 */
static void paste_clipboard(text_box_t* box) {
	const char* str;
	if (clipboard_contents) {
		str = clipboard_contents;
	} else {
		str = "";
	}
	if (box->cursor.visual_mode) {
		delete_selection(box);
	}
	insert_string(box, str, 0);
}

static void delete_selection(text_box_t* box) {
	line_chi_t begin;
	line_chi_t end;
	line_chi_min_max(&box->cursor.position, &box->cursor.selection_begin, 
			&begin, &end);
	delete_range(box, &begin, &end);
	box->cursor.visual_mode = 0;
}

char* get_debug_representation_of_box(text_box_t* box) {
#define A
#ifndef A
	size_t len = 512 + 110 * (box->lines.count - box->cursor.position.line);
#else
	size_t len = 7 * 50;
#endif
	char* str = (char*) malloc(len);
	if (!str) {
		return NULL;
	}

	char* cursor = str;
#ifdef A
	cursor += sprintf(cursor, "sizeof(line_t): %lu, \n", sizeof(line_t));
	cursor += sprintf(cursor, "sizeof(size_t): %lu, \n", sizeof(size_t));
	cursor += sprintf(cursor, "#(line): %lu, \n", box->lines.count);
	cursor += sprintf(cursor, "#(line_cap): %lu, \n", box->lines.capacity);
	size_t chars = 0;
	size_t chars_cap = 0;
	for (size_t i = 0; i < box->lines.count; ++i) {
		chars += find_line_len(&box->lines.arr[i]);
		chars_cap += find_line_capacity(&box->lines.arr[i]);
	}
	cursor += sprintf(cursor, "#(chars): %lu, \n", chars);
	cursor += sprintf(cursor, "#(chars_cap): %lu, \n", chars_cap);
#else
	cursor += sprintf(cursor, "{\n\tvvlines_begin: %lu,\n", box->vvlines_begin);
	if (box->interaction_mode == CURSOR) {
		cursor += sprintf(cursor, 
				"\tcursor: {\n\t\tposition (l, c): %lu, %lu\n\t},\n", 
				box->cursor.position.line, box->cursor.position.char_i);
	}
	cursor += sprintf(cursor, "\tlines: {\n\t\tarr: [\n");
	for (size_t i = box->cursor.position.line; i < box->lines.count; ++i) {
		cursor += sprintf(cursor, "\t\t\t{\n\t\t\t\tvline_begin: %lu,\n", 
				box->lines.arr[i].vline_begin);
		size_t count_sbs;
		const unsigned char* vline_starts = get_vline_lens(&box->lines.arr[i],
				&count_sbs);
		cursor += sprintf(cursor, "\t\t\t\tcount_softbreaks: %lu,\n", count_sbs);
		cursor += sprintf(cursor, "\t\t\t\tstring.count: %lu,\n", 
				box->lines.arr[i].string.count);
		cursor += sprintf(cursor, "\t\t\t\tvline_index: [");
		for (size_t j = 0; j < count_sbs; ++j) {
			cursor += sprintf(cursor, "%u, ", vline_starts[j]);
		}
		cursor += sprintf(cursor, "]\n\t\t\t},\n");
	}
	cursor += sprintf(cursor, "\t\t]\n\t}\n");
	cursor += sprintf(cursor, "}\n");
#endif
#undef A

	if (strlen(str) >= len) {
		sprintf(dbg_buf, "Debug buffer too small: %lu, %lu\n", 
				strlen(str), len);
		display_fatal_error(dbg_buf);
	}
	return str;
}
