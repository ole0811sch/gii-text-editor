#include "line_utils.h"
#include "util.h"
#include "dyn_arrs.h"

#include <string.h>
#include <stdlib.h>

size_t get_next_vline_begin(const text_box_t* box, size_t index_old_vline_begin, 
		const line_t* line, int* last) {
	for (size_t i = index_old_vline_begin;; ++i) {
		if (is_line_end(line->str, i)) {
			*last = 1;
			return index_old_vline_begin;
		}
		if (i >= index_old_vline_begin + box->width) {
			*last = 0;
			return i;
		}
	}
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

size_t char_i_to_vline_offset(const text_box_t* box, const line_t* line, 
		size_t char_i, size_t* vline_begin_char) {
	size_t vline_begin_char_old = 0;	// char_i of first char of vline_index--
	size_t vline_begin_char_cur = 0;	// char_i of first char of vline_index
	size_t vline_index = 0;	// index of the last vline we have the begin ind. of
	int last = 0;
	while (1) {	// iterate over all vline beginnings
		vline_begin_char_cur = get_next_vline_begin(box, vline_begin_char_cur, 
				line, &last);
		++vline_index;
		if (last || char_i < vline_begin_char_cur) { 
			// char_i must be in this vline
			break;
		}

		vline_begin_char_old = vline_begin_char_cur;
	}
	if (vline_begin_char) {
		*vline_begin_char = vline_begin_char_old;
	}
	return vline_index - 1;
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


	size_t vline_begin_char_old;	// char_i of first char in vline of line_chi
	size_t vline_index = char_i_to_vline_offset(box, line, line_chi.char_i,
			&vline_begin_char_old);

	tentative_x = line_chi.char_i - vline_begin_char_old;
	tentative_vline = vline_index + line->vline_begin;

	if (cursor_mode && tentative_x >= box->width) {
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

int line_chi_greater_equals(line_chi_t a, line_chi_t b) {
	return !line_chi_greater_than(b, a);
}

int line_chi_less_than(line_chi_t a, line_chi_t b) {
	return line_chi_greater_than(b, a);
}

int line_chi_less_equals(line_chi_t a, line_chi_t b) {
	return !line_chi_greater_than(a, b);
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

