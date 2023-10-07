#ifndef EDITOR_RENDER_H_
#define EDITOR_RENDER_H_

#include "editor.h"

void redraw_changes(text_box_t* box);
void print_lines(text_box_t* box);
void move_cursor(text_box_t* box, int mode);

#endif // EDITOR_RENDER_H_
