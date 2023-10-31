#ifndef LINE_UTILS_H_
#define LINE_UTILS_H_

#include "bt_char.h"
#include "line.h"
#include "editor.h"


/**
 * returns the next index that begins a new vline, or the index of the end of
 * the string. last will be set to 1 if index_old_vline_begin was the begin of
 * the last vline in the line, and to 0 otherwise. In the case where last is set
 * to 1, index_old_vline_begin will be returned.
 */
size_t get_next_vline_begin(const text_box_t* box, size_t index_old_vline_begin, 
		const line_t* line, int* last);
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
		unsigned char* x, char cursor_mode);
/**
 * sets min to min(*a, *b) and max to max(*a, *b). Destionations and sources
 * should be different. If min or max are NULL, they are ignored.
 */
void line_chi_min_max(const line_chi_t* a, const line_chi_t* b, line_chi_t* min, 
		line_chi_t* max);
int line_chi_greater_than(line_chi_t a, line_chi_t b);
int line_chi_greater_equals(line_chi_t a, line_chi_t b);
int line_chi_less_equals(line_chi_t a, line_chi_t b);
int line_chi_less_than(line_chi_t a, line_chi_t b);
int line_chi_equals(line_chi_t a, line_chi_t b);
/**
 * vline_void is a pointer to a (size_t) vline.
 * returns sign(vline - other->vline_begin);
 */
int compare_lines_vline_begin(const void* vline_void, 
		const line_t* other);
/**
 * adds an empty line_t to box->lines
 */
void add_new_line(text_box_t* box, size_t vline_begin);
/**
 * returns the number of softbreaks in line (i.e. #vlines - 1)
 */
size_t count_softbreaks(const text_box_t* box, const line_t* line);
/**
 * if vline_begin_char isn't NULL, it will be set to the index of the first char
 * of that vline. The offset of the vline in which char_i is located will be 
 * returned.
 */
size_t char_i_to_vline_offset(const text_box_t* box, const line_t* line, 
		size_t char_i, size_t* vline_begin_char);
size_t vline_offset_to_char_i(const text_box_t* box, const line_t* line,
		const size_t vline_offset);
#endif // LINE_UTILS_H_
