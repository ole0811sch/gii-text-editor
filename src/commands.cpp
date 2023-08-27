#include "commands.h"
#include "editor.h"
#include "keybios.h"
#include "fxlib.h"
#include "line.h"
#include "util.h"

#include <stdlib.h>

/**
 * initializes command_box as the command_line that belongs to editor.
 */
static void initialize_command_line(text_box_t* editor, 
		text_box_t* command_line) {
	unsigned int cl_height = 2;
	if (editor->height < 2) {
		cl_height = 1;
	}
	unsigned int cl_top = editor->height - cl_height;
	unsigned int cl_top_px = CHARH_TO_PX(cl_top);
	initialize_text_box(editor->left_px, cl_top_px, editor->width, cl_height, 
			CURSOR, 1, "", command_line);
}

/**
 * draws and focuses on command_line. Returns when an escape_key (KEY_CTRL_EXE
 * or KEY_CTRL_EXIT) is pressed, and returns that key. The caller can then
 * extract the string from command line
 */
static unsigned int start_and_draw_command_line(text_box_t* command_line) {
	draw_text_box(command_line);
	unsigned int escape_keys[] = { KEY_CTRL_EXE, KEY_CTRL_EXIT };
	return focus_text_box(command_line, escape_keys, 2);
}

void open_command_line(text_box_t* editor) {
	text_box_t command_line;
	initialize_command_line(editor, &command_line);
	unsigned int esc_key = start_and_draw_command_line(&command_line);
	if (esc_key != KEY_CTRL_EXE) {
		destruct_text_box(&command_line);
		return;
	}


	char* cmd = tmp_buf;
	size_t tmp_buf_len = sizeof(tmp_buf) / sizeof(tmp_buf[0]);
	size_t bytes_written = get_text_box_string(&command_line, tmp_buf, 
			tmp_buf_len);
	if (bytes_written >= tmp_buf_len) {
		// did not fit, use malloc
		char* d_buf = (char*) malloc((bytes_written + 1) * sizeof(char));
		if (d_buf == NULL) {
			display_error("Out of memory");
		}
		cmd = d_buf;
		get_text_box_string(&command_line, d_buf, bytes_written + 1);
	}
	Print((unsigned char*) cmd);

	destruct_text_box(&command_line);
}
