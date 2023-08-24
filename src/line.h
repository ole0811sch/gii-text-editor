#ifndef LINE_H_
#define LINE_H_

#include "dyn_arrs.h"

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

#undef DYN_ARR_H_
#define DYN_ARR_CG_TYPE line_t
#define DYN_ARR_CG_SUFFIX line
#include "dyn_arr.h"
#undef DYN_ARR_CG_SUFFIX
#undef DYN_ARR_CG_TYPE

#endif // LINE_H_
