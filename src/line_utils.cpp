#include "line_utils.h"

#include <string.h>
#include <stdlib.h>

static void remove_last_n_softbreaks(line_t* line, size_t target_count);


/**
 * returns the array in the vline_index of line. If there are no softbreaks,
 * NULL is returned. If count_softbreaks isn't NULL, it is set.
 */
const size_t* get_vline_starts(const line_t* line, size_t* count_softbreaks) {
#define GET_VLINE_STARTS \
	if (count_softbreaks != NULL)\
		*count_softbreaks = line->count_softbreaks;\
	if (line->count_softbreaks == 0)\
		return NULL;\
	else if (line->count_softbreaks <= VLINE_INDEX_STATIC_ARR_SIZE)\
		return line->vline_index.s_arr;\
	else\
		return line->vline_index.d_arr.arr;
	GET_VLINE_STARTS
}

size_t* get_vline_starts_mut(line_t* line, size_t* count_softbreaks) {
	GET_VLINE_STARTS
}
#undef GET_VLINE_STARTS

/**
 * recalculates the vline_index of line, starting with the starting index of
 * the vline after vline_offs. If the line's count_softbreaks is 0 and
 * vline_offs is 0, this functions just adds all indices to vline_index and 
 * correctly sets count_softbreaks.
 *
 * returns the difference in how many vlines there were before and after. If new
 * vlines were added, that number is positive.
 */
ptrdiff_t recalculate_vline_index(text_box_t* box, line_t* line, 
		size_t vline_offs) {
	size_t count_softbreaks_start;
	size_t* vline_starts =
		get_vline_starts_mut(line, &count_softbreaks_start);
	size_t  char_i; 
	if (vline_offs == 0)
		char_i = 0;
	else
		char_i = vline_starts[vline_offs - 1];


	// recalculate existing starts
	for (size_t sb_i = vline_offs; sb_i < line->count_softbreaks; ++sb_i) {
		char_i += box->width;
		if (char_i >= line->string.count) {
			remove_last_n_softbreaks(line, sb_i);
			break;
		}
		vline_starts[sb_i] = char_i;
	}

	// add softbreaks until char_i is out of range of the line's string
	while ((char_i += box->width) < line->string.count)
		add_softbreak_to_index(line, char_i);
	
	return line->count_softbreaks - count_softbreaks_start;
}

/**
 * adds an entry to vline_index of current_line with index i. returns the array
 * that is currently used
 */
size_t* add_softbreak_to_index(line_t* current_line, size_t i) {
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
 * fills box->lines with str. box->lines should be uninitialized at this
 * point.
 */
void initialize_lines(text_box_t* box, const char* str) {
	if(dyn_arr_line_create(10, 2, 1, &box->lines) == -1)
		display_fatal_error("Out of memory");

	dyn_arr_line_t* lines = &box->lines;
	add_new_line(box, 0);
	line_t* current_line = DYN_ARR_LAST(lines);
	unsigned char col = 0;
	size_t vline = 0;
	size_t i = 0;
	size_t line_starting_index = 0; // index of the first char in str in line
	for (; str[i]; ++i) {
		if (str[i] == '\n') {
			++vline;
			line_starting_index = i + 1;
			add_new_line(box, vline);
			current_line = DYN_ARR_LAST(lines);
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

/**
 * if x isn't null, it is set to the char_point_t x value that line_chi
 * corresponds to. 
 * If cursor_mode and if char_i is greater than the index of the last char of 
 * the line, then if the last char of the line is already at the x coordinate,
 * then the first vline of the next line is returned and x is set to 0. Else 
 * the last vline of this line is returned and x will be set to be char_x + 1
 * where char_x is the column of the last character. 
 * If not cursor_mode, then the last vline of this line will be returned and x
 * might be equal to EDITOR_COLUMNS.
 */
size_t line_chi_to_vline(const text_box_t* box, line_chi_t line_chi, 
		unsigned char* x, char cursor_mode) {
	line_t* line = &box->lines.arr[line_chi.line];
	unsigned char tentative_x;
	size_t tentative_vline;

	// find the correct array
	const size_t* vline_starts = get_vline_starts(line, NULL);

	// check if the line_chi is in first vline of the line
	if (line->count_softbreaks == 0 || line_chi.char_i < vline_starts[0]) {
		tentative_x = line_chi.char_i;
		tentative_vline = line->vline_begin;
	} else {
		size_t vline_index = dyn_arr_size_raw_bsearch(vline_starts, 
				line->count_softbreaks, line_chi.char_i);

		tentative_x = line_chi.char_i - vline_starts[vline_index];
		tentative_vline = vline_index + 1 + line->vline_begin;
	}
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
 * adds an empty line_t to box->lines
 */
void add_new_line(text_box_t* box, size_t vline_begin) {
	line_t new_line;
	new_line.vline_begin = vline_begin;
	new_line.count_softbreaks = 0;
	dyn_arr_line_add(&box->lines, new_line);
	dyn_arr_char_t* string = &DYN_ARR_LAST(&box->lines)->string;
	if (dyn_arr_char_create(1, 2, 1, string) == -1)
		display_fatal_error("Out of memory");
}
