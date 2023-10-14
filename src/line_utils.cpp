#include "line_utils.h"
#include "util.h"
#include "dyn_arrs.h"

#include <string.h>
#include <stdlib.h>

size_t get_next_vline_begin(const text_box_t* box, size_t index_old_vline_begin, 
		const line_t* line, int* last) {
	if (!line->str) {
		*last = 1;
		return index_old_vline_begin;
	}

	size_t i;
	for (i = index_old_vline_begin;
			i < index_old_vline_begin + box->width; ++i) {
		switch (is_line_end(line->str[i])) {
			case AFTER_LINE_END:
			case LAST_CHAR:
				*last = 1;
				return index_old_vline_begin;
			default:
				break;
		}
	}
	*last = 0;
	return i;
}

size_t count_softbreaks(const text_box_t* box, const line_t* line) {
	size_t sbreaks = 0;
	int last = 0;
	size_t vline_i = 0;
	while (1) {
		vline_i = get_next_vline_begin(box, vline_i, line, &last);
		if (last) {
			break;
		}
		++sbreaks;
	}
	return sbreaks;
}

/**
 * if vline_begin_char isn't NULL, it will be set to the index of the first char
 * of that vline. The offset of the vline in which char_i is located will be 
 * returned.
 */
size_t char_i_to_vline_offset(const text_box_t* box, const line_t* line, 
		size_t char_i, size_t* vline_begin_char) {
	size_t vline_begin_char_old = 0;
	size_t vline_begin_char_cur = 0;
	size_t vline_index = 0;
	int last = 0;
	while (1) {
		if (char_i < vline_begin_char_cur) {
			break;
		}

		vline_begin_char_old = vline_begin_char_cur;
		vline_begin_char_cur = get_next_vline_begin(box, vline_begin_char_cur, 
				line, &last);
		if (last) {
			break;
		}
		++vline_index;
	}
	if (vline_begin_char) {
		*vline_begin_char = vline_begin_char_old;
	}
	return vline_index;
}

size_t vline_offset_to_char_i(const text_box_t* box, const line_t* line,
		const size_t vline_offset) {
	size_t vline_index = 0;
	size_t char_i = 0;
	while (1) {
		if (vline_index == vline_offset) {
			break;
		}

		int last;
		char_i = get_next_vline_begin(box, char_i, line, &last);
		if (last) {
			break;
		}
		++vline_index;
	}
	return char_i;
}

size_t line_chi_to_vline(const text_box_t* box, line_chi_t line_chi, 
		unsigned char* x, char cursor_mode) {
	line_t* line = &box->lines.arr[line_chi.line];
	unsigned char tentative_x;
	size_t tentative_vline;


	size_t vline_begin_char_old = 0;
	size_t vline_index = char_i_to_vline_offset(box, line, line_chi.char_i,
			&vline_begin_char_old);

	tentative_x = line_chi.char_i - vline_begin_char_old;
	tentative_vline = vline_index + line->vline_begin;

	if (cursor_mode && tentative_x >= EDITOR_COLUMNS) {
		// use first char in next vline
		if (x != NULL) {
			*x = 0;
		}
		return tentative_vline + 1;
	}
	if (x != NULL) {
		*x = tentative_x;
	}
	return tentative_vline;
}

void add_new_line(text_box_t* box, size_t vline_begin) {
	line_t new_line;
	new_line.vline_begin = vline_begin;
	new_line.str = NULL;
	dyn_arr_line_add(&box->lines, new_line);
}

void line_chi_min_max(const line_chi_t* a, const line_chi_t* b, line_chi_t* min, 
		line_chi_t* max) {
	if (line_chi_greater_than(*a, *b)) {
		if (min)
			*min = *b;
		if (max)
			*max = *a;
	} else {
		if (min)
			*min = *a;
		if (max)
			*max = *b;
	}
}

int line_chi_greater_than(line_chi_t a, line_chi_t b) {
	return a.line > b.line || (a.line == b.line && a.char_i > b.char_i);
}

int line_chi_equals(line_chi_t a, line_chi_t b) {
	return a.line == b.line && a.char_i == b.char_i;
}

int compare_lines_vline_begin(const void* vline_void, 
		const line_t* other) {
	size_t vline = *(size_t*) vline_void;
	if (vline < other->vline_begin)
		return -1;
	else if (vline > other->vline_begin)
		return 1;
	return 0;
}
