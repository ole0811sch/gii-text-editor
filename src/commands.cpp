#include "commands.h"
#include "editor.h"
#include "fxlib.h"
#include "keybios.h"
#include "line.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>

enum CLIResultType { ERROR, EDIT, WRITE };

typedef struct {
	enum CLIResultType enum_cmd;
	char* ascii_cmd;
} cmd_ascii_enum_pair_t;

enum ErrorType { MISSING_ARGS, TOO_MANY_ARGS, INVALID_CMD, NO_CMD };

/**
 * represents a slice of an array or string that is bound by begin and end
 */
typedef struct {
	size_t begin; // inclusive
	size_t end;   // exclusive
} index_slice_t;

/**
 * represents the result of parsing a command (the slice if presents refers back
 * to the string of the command)
 */
typedef struct {
	enum CLIResultType type;
	union {
		struct FileContainer {
			index_slice_t file_name;
		} one_arg;
		struct {
			enum ErrorType type;
		} error;
	} content;
} cli_result_t;

static void initialize_command_line(text_box_t *editor,
		text_box_t *command_line);
static unsigned int start_and_draw_command_line(text_box_t *command_line);
static char find_next_non_whitespace(size_t start_i, const char *str,
		index_slice_t *next_non_ws);
static cli_result_t parse_cli(const char* txt);
static void display_cli_error(cli_result_t* result);
static void edit_file(text_box_t* editor, const char* txt, 
		index_slice_t slice);
static void write_file(text_box_t* editor, const char* txt, 
		index_slice_t slice);

/**
 * creates and displays new command line window, then executes the command
 */
void open_command_line(text_box_t *editor) {
	text_box_t command_line;
	initialize_command_line(editor, &command_line);
	unsigned int esc_key = start_and_draw_command_line(&command_line);
	if (esc_key != KEY_CTRL_EXE) {
		destruct_text_box(&command_line);
		return;
	}

	char *cmd = tmp_buf;
	char is_malloced = 0;
	size_t tmp_buf_len = sizeof(tmp_buf) / sizeof(tmp_buf[0]);
	size_t bytes_written =
		get_text_box_string(&command_line, tmp_buf, tmp_buf_len);
	if (bytes_written >= tmp_buf_len) {
		// did not fit, use malloc
		char *d_buf = (char *)malloc((bytes_written + 1) * sizeof(char));
		if (d_buf == NULL) {
			display_fatal_error("Out of memory");
		}
		is_malloced = 1;
		cmd = d_buf;
		get_text_box_string(&command_line, d_buf, bytes_written + 1);
	}
	cli_result_t cli_result = parse_cli(cmd);
	switch (cli_result.type) {
		case ERROR:
			display_cli_error(&cli_result);
			break;
		case WRITE:
			write_file(editor, cmd, cli_result.content.one_arg.file_name);
			break;
		case EDIT:
			edit_file(editor, cmd, cli_result.content.one_arg.file_name);
			break;
		default:
			dbg_print("Error: invalid enum type (commands.cpp:"
				"open_command_line");
	}
	if (is_malloced) {
		free(cmd);
	}

	destruct_text_box(&command_line);
}

/**
 * initializes command_box as the command_line that belongs to editor.
 */
static void initialize_command_line(text_box_t *editor,
		text_box_t *command_line) {
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
static unsigned int start_and_draw_command_line(text_box_t *command_line) {
	draw_text_box(command_line);
	unsigned int escape_keys[] = {KEY_CTRL_EXE, KEY_CTRL_EXIT};
	return focus_text_box(command_line, escape_keys, 2);
}

/**
 * returns whether there was any non-whitespace starting from start_i.
 * If yes, it sets next_non_ws the next non-whitespace group, i.e., the slice's
 * begin is the next char that isn't whitespace and the end is the first char
 * after that that is whitespace.
 */
static char find_next_non_whitespace(size_t start_i, const char *str,
		index_slice_t *next_non_ws) {
	const char filter[] = " \t\n";
	size_t filter_len = 3;
	char reached_end = 1; // whether the loop ended because we found non-ws or
						  // because the end of the string was reached
	for (size_t i = start_i; str[i] != 0; ++i) {
		for (size_t filter_i = 0; filter_i < filter_len; ++filter_i) {
			if (str[i] == filter[filter_i]) {
				goto continue_outer;
			}
		}
		reached_end = 0;
		next_non_ws->begin = i;
		break;
continue_outer:;
	}

	if (reached_end) {
		return 0;
	}

	size_t i;
	for (i = next_non_ws->begin + 1; str[i] != 0; ++i) {
		for (size_t filter_i = 0; filter_i < filter_len; ++filter_i) {
			if (str[i] == filter[filter_i]) {
				goto break_outer2;
			}
		}
	}
break_outer2:
	next_non_ws->end = i;
	return 1;
}

/**
 * returns the representation of txt
 */
cli_result_t parse_cli(const char* txt) {
	cli_result_t ret_val;
	const cmd_ascii_enum_pair_t cmds[] = { { EDIT, "e" }, { WRITE, "w" } };
	const size_t cmds_len = sizeof(cmds) / sizeof(cmds[0]);
	index_slice_t cmd;
	if (!find_next_non_whitespace(0, txt, &cmd)) {
		ret_val.type = ERROR;
		ret_val.content.error.type = NO_CMD;
		return ret_val;
	}
	index_slice_t arg1;
	if (!find_next_non_whitespace(cmd.end, txt, &arg1)) {
		ret_val.type = ERROR;
		ret_val.content.error.type = MISSING_ARGS;
		return ret_val;
	}
	index_slice_t arg2;
	if (find_next_non_whitespace(arg1.end, txt, &arg2)) {
		ret_val.type = ERROR;
		ret_val.content.error.type = TOO_MANY_ARGS;
		return ret_val;
	}

	char matched = 0;
	size_t cmd_i;
	for (cmd_i = 0; cmd_i < cmds_len; ++cmd_i) {
		for (size_t ch_i = cmd.begin; txt[ch_i]; ++ch_i) {
			if (ch_i == cmd.end) {
				matched = 1;
				goto break_outer;
			}
			if (txt[ch_i] != cmds[cmd_i].ascii_cmd[ch_i - cmd.begin]) {
				break; // continue with next command
			}
		}
	}
break_outer:
	if (!matched) {
		ret_val.type = ERROR;
		ret_val.content.error.type = INVALID_CMD;
		return ret_val;
	}

	ret_val.type = cmds[cmd_i].enum_cmd;
	ret_val.content.one_arg.file_name = arg1;
	return ret_val;
}

/**
 * displays the error that result represents. If result isn't an error this
 * function should not be called
 */
static void display_cli_error(cli_result_t* result) {
	switch (result->content.error.type) {
		case INVALID_CMD:
			display_error("Invalid command");
			break;
		case NO_CMD:
			break;
		case TOO_MANY_ARGS:
			display_error("Too many arguments");
			break;
		case MISSING_ARGS:
			display_error("Too few arguments");
			break;
		default:
			display_error("Could not parse command");
	}
}

/**
 * replaces the editor with a new text box with the contents of a specific file.
 * txt and slice represent the name of that file.
 */
static void edit_file(text_box_t* editor, const char* txt, 
		index_slice_t slice) {
	// TODO
	destruct_text_box(editor);
	initialize_text_box(editor->left_px, editor->top_px, editor->width,
			editor->height, editor->interaction_mode, editor->cursor.editable, 
			"Opened file", editor);
#ifdef MOCKUP
	fputs("Opened file: ", stderr);
	for (int i = slice.begin; i < slice.end; ++i) {
		fputc(txt[i], stderr);
	}
	fputc('\n', stderr);
#endif
}

/**
 * writes the contents of editor to the file with the name represented by txt
 * and slice
 */
static void write_file(text_box_t* editor, const char* txt, 
		index_slice_t slice) {
	// TODO
#ifdef MOCKUP
	fputs("Write file: ", stderr);
	for (int i = slice.begin; i < slice.end; ++i) {
		fputc(txt[i], stderr);
	}
	fputc('\n', stderr);
#endif
}



// tests
#include "assert.h"

static cli_result_t create_pos_cli_result(enum CLIResultType type, size_t begin,
		size_t end) {
	cli_result_t ret_val;
	ret_val.type = type;
	ret_val.content.one_arg.file_name.begin = begin;
	ret_val.content.one_arg.file_name.end = end;
	return ret_val;
}

static cli_result_t create_neg_cli_result(enum ErrorType error_type) {
	cli_result_t ret_val;
	ret_val.type = ERROR;
	ret_val.content.error.type = error_type;
	return ret_val;
}

static char cli_result_equals(cli_result_t a, cli_result_t b) {
	if (a.type != b.type) {
		return 0;
	}

	if (a.type == ERROR) {
		return a.content.error.type == b.content.error.type;
	} else if (a.type == EDIT || a.type == WRITE) {
		return a.content.one_arg.file_name.begin == 
			b.content.one_arg.file_name.begin
			&& a.content.one_arg.file_name.end == 
			b.content.one_arg.file_name.end;
	} else {
		fprintf(stderr, "Invalid CLIResultType with value %u", a.type);
		return 0;
	}
}

char test_parse_cli(void) {
#define num_tests 11
	const char* const cls[] = { "e file.txt",
		"w file.txt", 
		" e file.txt",
		"e  file.txt",
		"e file.txt ",
		"\te file.txt",
		"  e  file.txt  ",
		"e",
		"e arg0 arg1" ,
		"invalid_cmd file.txt",
		"" 
	};
	assert(sizeof(cls) / sizeof(cls[0]) == num_tests);
	
	cli_result_t results[num_tests];
	results[0] = create_pos_cli_result(EDIT, 2, 10);
	results[1] = create_pos_cli_result(WRITE, 2, 10);
	results[2] = create_pos_cli_result(EDIT, 3, 11);
	results[3] = create_pos_cli_result(EDIT, 3, 11);
	results[4] = create_pos_cli_result(EDIT, 2, 10);
	results[5] = create_pos_cli_result(EDIT, 3, 11);
	results[6] = create_pos_cli_result(EDIT, 5, 13);
	results[7] = create_neg_cli_result(MISSING_ARGS);
	results[8] = create_neg_cli_result(TOO_MANY_ARGS);
	results[9] = create_neg_cli_result(INVALID_CMD);
	results[10] = create_neg_cli_result(NO_CMD);

	for (size_t i = 0; i < num_tests; ++i) {
		if (!cli_result_equals(parse_cli(cls[i]), results[i])) {
			return 0;
		}
	}
	return 1;
#undef num_tests
}
