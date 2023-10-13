#include "line_utils.h"
#include "util.h"
#include "dyn_arrs.h"

#include <string.h>
#include <stdlib.h>

static void remove_last_n_softbreaks(line_t* line, size_t target_count);


/**
 * returns the array in the vline_index of line. If there are no softbreaks,
 * NULL is returned. If count_softbreaks isn't NULL, it is set.
 */
const unsigned char* get_vline_lens(const line_t* line, 
		size_t* count_softbreaks) {
#define GET_VLINE_LENS \
	if (count_softbreaks != NULL)\
		*count_softbreaks = line->count_softbreaks;\
	if (line->count_softbreaks == 0)\
		return NULL;\
	else if (line->count_softbreaks <= VLINE_INDEX_STATIC_ARR_SIZE)\
		return line->vline_index.s_arr;\
	else\
		return line->vline_index.d_arr->arr;
	GET_VLINE_LENS
}

unsigned char* get_vline_lens_mut(line_t* line, size_t* count_softbreaks) {
	GET_VLINE_LENS
}
#undef GET_VLINE_LENS

/**
 * starts should have count_lens + 1 space
 */
void vline_lens_to_starts(const unsigned char* lens, size_t starts[], 
		size_t count_lens) {
	starts[0] = 0;
	if (count_lens == 0) {
		return;
	}

	for (size_t i = 0; i < count_lens; ++i) {
		starts[i + 1] = starts[i] + lens[i];
	}
}

/**
 * starts should have count_lens + 1 space
 */
void vline_starts_to_lens(unsigned char* lens, const size_t* starts, 
		size_t count_lens) {
	if (count_lens == 0) {
		return;
	}

	for (size_t i = 0; i < count_lens; ++i) {
		lens[i] = starts[i + 1] - starts[i];
	}
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
ptrdiff_t recalculate_vline_index(text_box_t* box, line_t* line, 
		size_t vline_offs) {
	size_t count_softbreaks_start;
	unsigned char* vline_lens =
		get_vline_lens_mut(line, &count_softbreaks_start);
	size_t* vline_starts = (size_t*) malloc((count_softbreaks_start + 1) 
			* sizeof(size_t));
	if (!vline_starts) {
		display_fatal_error(MSG_ENOMEM);
	}
	vline_lens_to_starts(vline_lens, vline_starts, count_softbreaks_start);
	size_t  char_i; 
	char_i = vline_starts[vline_offs];


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

	vline_starts_to_lens(vline_lens, vline_starts, count_softbreaks_start);
	free(vline_starts);
	
	return line->count_softbreaks - count_softbreaks_start;
}

/**
 * adds an entry to vline_index of current_line with index i. returns the array
 * that is currently used
 */
unsigned char* add_softbreak_to_index(line_t* current_line, unsigned char len) {
	unsigned char* ret_val;
	if (current_line->count_softbreaks < 
			VLINE_INDEX_STATIC_ARR_SIZE) { // put into s_arr
		ret_val = current_line->vline_index.s_arr;
		ret_val[current_line->count_softbreaks] = len;
	}
	else {
		dyn_arr_uchar_t** d_arr = &current_line->vline_index.d_arr;
		if (current_line->count_softbreaks == VLINE_INDEX_STATIC_ARR_SIZE) {
			// move s_arr to d_arr
			unsigned char* s_arr = current_line->vline_index.s_arr;
			*d_arr = (dyn_arr_uchar_t*) malloc(sizeof(dyn_arr_uchar_t));
			unsigned char intermediate[VLINE_INDEX_STATIC_ARR_SIZE];
			memcpy(intermediate, s_arr, 
					VLINE_INDEX_STATIC_ARR_SIZE);
			if (dyn_arr_uchar_create(VLINE_INDEX_STATIC_ARR_SIZE + 1, 
						*d_arr) == -1
					|| dyn_arr_uchar_add_all(*d_arr, intermediate, 
					VLINE_INDEX_STATIC_ARR_SIZE) == -1) {
				display_fatal_error(MSG_ENOMEM);
			}
		}

		ret_val = (*d_arr)->arr;

		if (dyn_arr_uchar_add(*d_arr, len) == -1) {
			display_fatal_error(MSG_ENOMEM);
		}
	}
	++current_line->count_softbreaks;
	return ret_val;
}

size_t line_chi_to_vline(const text_box_t* box, line_chi_t line_chi, 
		unsigned char* x, char cursor_mode) {
	line_t* line = &box->lines.arr[line_chi.line];
	unsigned char tentative_x;
	size_t tentative_vline;

	// find the correct array
	size_t count_lens;
	const unsigned char* vline_lens = get_vline_lens(line, &count_lens);
	size_t* vline_starts = (size_t*) malloc((count_lens + 1) * sizeof(size_t));
	if (!vline_starts) {
		display_fatal_error(MSG_ENOMEM);
	}
	vline_lens_to_starts(vline_lens, vline_starts, count_lens);

	// check if the line_chi is in first vline of the line
	if (count_lens == 0 || line_chi.char_i < vline_starts[1]) {
		tentative_x = line_chi.char_i;
		tentative_vline = line->vline_begin;
	} else {
		size_t vline_index = dyn_arr_size_raw_bsearch(vline_starts, 
				line->count_softbreaks, line_chi.char_i);

		tentative_x = line_chi.char_i - vline_starts[vline_index];
		tentative_vline = vline_index + line->vline_begin;
	}
	free(vline_starts);
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
			dyn_arr_uchar_t* heap_arr = line->vline_index.d_arr;
			memmove(line->vline_index.s_arr, line->vline_index.d_arr->arr, 
					target_count);
			dyn_arr_uchar_destroy(heap_arr);
			free(heap_arr);
		}
		else
			// keep using d_arr
			dyn_arr_uchar_pop_some(line->vline_index.d_arr, count_to_remove);
	}
	line->count_softbreaks = target_count;
}

void add_new_line(text_box_t* box, size_t vline_begin) {
	line_t new_line;
	new_line.vline_begin = vline_begin;
	new_line.count_softbreaks = 0;
	dyn_arr_line_add(&box->lines, new_line);
	dyn_arr_char_t* string = &DYN_ARR_LAST(&box->lines)->string;
	if (dyn_arr_char_create(1, string) == -1)
		display_fatal_error("Out of memory");
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
