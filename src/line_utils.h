#ifndef LINE_UTILS_H_
#define LINE_UTILS_H_

#include "line.h"
#include "editor.h"

const size_t* get_vline_starts(const line_t* line, size_t* count_softbreaks);
size_t* get_vline_starts_mut(line_t* line, size_t* count_softbreaks);
ptrdiff_t recalculate_vline_index(text_box_t* box, line_t* line,
		size_t vline_offs);
size_t* add_softbreak_to_index(line_t* current_line, size_t i);
void initialize_lines(text_box_t* box, const char* str);
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
 * adds an empty line_t to box->lines
 */
void add_new_line(text_box_t* box, size_t vline_begin);
/**
 * sets min to min(*a, *b) and max to max(*a, *b). Destionations and sources
 * should be different. If min or max are NULL, they are ignored.
 */
void line_chi_min_max(const line_chi_t* a, const line_chi_t* b, line_chi_t* min, 
		line_chi_t* max);
int line_chi_greater_than(line_chi_t a, line_chi_t b);
int line_chi_equals(line_chi_t a, line_chi_t b);
/**
 * vline_void is a pointer to a (size_t) vline.
 * returns sign(vline - other->vline_begin);
 */
int compare_lines_vline_begin(const void* vline_void, 
		const line_t* other);
#endif // LINE_UTILS_H_
