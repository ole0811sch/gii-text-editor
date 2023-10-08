#ifndef EDITOR_RENDER_H_
#define EDITOR_RENDER_H_

#include "editor.h"

void redraw_changes(const text_box_t* box);
void print_lines(const text_box_t* box);
void move_cursor(const text_box_t* box, int mode);

#endif // EDITOR_RENDER_H_
