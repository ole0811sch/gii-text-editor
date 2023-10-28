#ifndef LINE_H_
#define LINE_H_

#include "bt_char.h"
#include "dyn_arrs.h"

typedef struct {
	/** 
	 * index of the line that's printed where this line starts. One line with
	 * one soft break correspond to two vlines.
	 */
	size_t vline_begin;
	/**
	 * text of the line. Heap allocated. The end of the allocated area is marked
	 * by '\n'. Between the end of the string end '\n', all bytes are '\0'. It
	 * is legal to have zero '\0' bytes, i.e. an '\n'-terminated string. It may
	 * also be NULL to signal an empty line.
	 */
	bt_char_t* str;
} line_t;

void destruct_line(line_t* line);

#define DYN_ARR_CG_GROWTH_NUMERATOR 5
#define DYN_ARR_CG_GROWTH_DENOMINATOR 4
#undef DYN_ARR_H_
#define DYN_ARR_CG_TYPE line_t
#define DYN_ARR_CG_SUFFIX line
#include "dyn_arr.h"
#undef DYN_ARR_CG_SUFFIX
#undef DYN_ARR_CG_TYPE
#undef DYN_ARR_CG_GROWTH_NUMERATOR
#undef DYN_ARR_CG_GROWTH_DENOMINATOR

size_t find_line_capacity(const line_t* line);

size_t find_line_len(const line_t* line);

/**
 * Can be used for expansion and shrinking. Sets bytes to 0 from bytes len to
 * new_capacity - 1. Sets new eol marker and erases old one. old_capacity and
 * new_capacity may both be 0.
 */
int line_change_size(line_t* line, size_t old_capacity, size_t len,
		size_t new_capacity);

int line_ensure_capacity(line_t* line, size_t old_capacity, size_t len,
		size_t new_capacity, size_t* cap_ret);

int line_add_all(line_t* line, const char* chars, size_t n);

int line_pop_some(line_t* line, size_t n);


int line_insert_some(line_t* line, const char* chars, size_t n, size_t index);

int line_remove_some(line_t* line, size_t start_i, size_t end_i);

int line_remove(line_t* line, size_t index);

int line_pop(line_t* line);

int line_add(line_t* line, char c);

int line_insert(line_t* line, char c, size_t index);

#endif // LINE_H_
