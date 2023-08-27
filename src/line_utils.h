#ifndef LINE_UTILS_H_
#define LINE_UTILS_H_

#include "line.h"
#include "editor.h"

size_t* get_vline_starts(line_t* line, size_t* count_softbreaks);
ptrdiff_t recalculate_vline_index(text_box_t* box, line_t* line,
		size_t vline_offs);
size_t* add_softbreak_to_index(line_t* current_line, size_t i);
void initialize_lines(text_box_t* box, const char* str);
size_t line_chi_to_vline(text_box_t* box, line_chi_t line_chi, 
		unsigned char* x);
void add_new_line(text_box_t* box, size_t vline_begin);

#endif // LINE_UTILS_H_
