#include "commands.h"
#include "editor.h"
#include "line.h"
#include "util.h"

#include "fxlib.h"
#include "keybios.h"
#include "filebios.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
 * Converts the path represented by txt and slice to a FONTCHARACTER array that
 * can be used e.g. for Bfile_OpenFile. At the moment it always uses storage
 * memory. The returned pointer should be freed.
 */
static FONTCHARACTER* get_path(const char* txt, index_slice_t slice) {
	// cpy file name
	size_t path_len = slice.end - slice.begin + 7; 	// + 7 for path
	FONTCHARACTER* path = (FONTCHARACTER*) malloc((path_len + 1)
			* sizeof(FONTCHARACTER));
	if (!path) display_fatal_error(MSG_ENOMEM);
	for (size_t i = slice.begin; i < slice.end; ++i) {
		path[7 + i - slice.begin] = txt[i];
	}
	// set path
	FONTCHARACTER pre[7] = { '\\', '\\', 'f', 'l', 's', '0', '\\' };
	memcpy(path, pre, sizeof(pre));
	path[path_len] = 0;
	return path;
}

static void close_file(int file) {
	int close_res = -1;
	for (int i = 0; i < 10 && close_res < 0; ++i) {
		close_res = Bfile_CloseFile(file);
	}
	if (close_res < 0) {
		sprintf(tmp_buf, "Could not close to file (%d)", close_res);
		display_error(tmp_buf);
	}
}

/**
 * replaces the editor with a new text box with the contents of a specific file.
 * txt and slice represent the name of that file.
 */
static void edit_file(text_box_t* editor, const char* txt, 
		index_slice_t slice) {
	int (*open_file)(const FONTCHARACTER*, int) = &Bfile_OpenFile;
	char* editor_contents = NULL;

	FONTCHARACTER* path = get_path(txt, slice);
	// open file
	int f = open_file(path, _OPENMODE_READ_SHARE);
	if (f < 0) {
		sprintf(tmp_buf, "Could not open file (%d)", f);
		display_error(tmp_buf);
		goto cleanup;
	}

	// get file size
	int f_size = Bfile_GetFileSize(f);
	if (f_size < 0) {
		sprintf(tmp_buf, "Could not get size of file (%d)", f_size);
		display_error(tmp_buf);
		close_file(f);
		goto cleanup;
	}

	// alloc buffer for file
	editor_contents = (char*) malloc(f_size + 1);
	editor_contents[f_size] = '\0';
	if (!editor_contents) display_fatal_error(MSG_ENOMEM);
	// read file
	int read_sum = 0;
	while (read_sum < f_size) {
		int read_res = Bfile_ReadFile(f, &editor_contents[read_sum], 
				f_size - read_sum, read_sum);
		if (read_res < 0) {
			sprintf(tmp_buf, "Could not from file (%d)", read_res);
			display_error(tmp_buf);
			close_file(f);
			goto cleanup;
		}
		read_sum += f_size;
	}
	close_file(f);

	// update editor
	destruct_text_box(editor);
	initialize_text_box(editor->left_px, editor->top_px, editor->width,
			editor->height, editor->interaction_mode, editor->cursor.editable, 
			editor_contents, editor);

#ifdef MOCKUP
	fputs("Opened file: ", stderr);
	for (int i = slice.begin; i < slice.end; ++i) {
		fputc(txt[i], stderr);
	}
	fputc('\n', stderr);
#endif

cleanup:
	free(path);
	free(editor_contents);
}

/**
 * writes the contents of editor to the file with the name represented by txt
 * and slice
 */
static void write_file(text_box_t* editor, const char* txt, 
		index_slice_t slice) {
	int (*create_file)(const FONTCHARACTER*, int) = &Bfile_CreateFile;
	int (*open_file)(const FONTCHARACTER*, int) = &Bfile_OpenFile;
	int (*delete_file)(const FONTCHARACTER*) = &Bfile_DeleteFile;

	FONTCHARACTER* path = get_path(txt, slice);
	// cpy editor content
	size_t buf_len = 64;
	char* editor_content = (char*) malloc(buf_len * sizeof(char));
	if (!editor_content) display_fatal_error(MSG_ENOMEM);
	size_t editor_len; // num of bytes in editor_content (including \0)
	size_t num_editor_chars = get_text_box_string(editor, editor_content, buf_len) + 1;
	editor_len = num_editor_chars + 1; // +1 for '\0'
	if (editor_len > buf_len) {
		// we need additional bytes for '\0' and possibly further chars from 
		// the editor
		editor_content = (char*) realloc(editor_content,
				editor_len * sizeof(char));
		if (!editor_content) display_fatal_error(MSG_ENOMEM);
		if (num_editor_chars > buf_len) {
			// not even all bytes from the editor have been 
			// written to editor_content
			get_text_box_string(editor, editor_content, editor_len);
		}
	}
	editor_content[num_editor_chars] = '\0';

	// try to create file
	int create_res = create_file(path, editor_len);
	if (create_res < 0) {
		if (create_res == IML_FILEERR_ALREADYEXISTENTRY) {
			// we need to delete the old file. Changing bits is apparently not
			// supported.
			// TODO: safely delete (i.e., copy the file or at least check sizes
			// beforehand)
			int del_res = delete_file(path);
			if (del_res < 0) {
				sprintf(tmp_buf, "Could not delete file (%d)", del_res);
				display_error(tmp_buf);
				goto cleanup;
			}
			int create_res = create_file(path, editor_len);
			if (create_res < 0) {
				sprintf(tmp_buf, "Could not create file (%d)", create_res);
				display_error(tmp_buf);
				goto cleanup;
			}
		}
		else {
			sprintf(tmp_buf, "Could not create file (%d)", create_res);
			display_error(tmp_buf);
			goto cleanup;
		}
	}

	// open file
	int f = open_file(path, _OPENMODE_READWRITE_SHARE);
	if (f < 0) {
		sprintf(tmp_buf, "Could not open file (%d)", f);
		display_error(tmp_buf);
		goto cleanup;
	}
	// write
	int bytes_written = 0;
	while (bytes_written < editor_len) {
		int write_res = Bfile_WriteFile(f, &editor_content[bytes_written], 
				editor_len - bytes_written);
		if (write_res < 0) {
			sprintf(tmp_buf, "Could not write to file (%d)", write_res);
			display_error(tmp_buf);
			close_file(f);
			goto cleanup;
		}
		bytes_written += write_res;
	}
	close_file(f);

#ifdef MOCKUP
	fputs("Write file: ", stderr);
	for (int i = slice.begin; i < slice.end; ++i) {
		fputc(txt[i], stderr);
	}
	fputc('\n', stderr);
	fputs("Contents: ", stderr);
	fputc('\n', stderr);
	fputs(editor_content, stderr);
#endif

cleanup:
	free(path);
	free(editor_content);
}


#ifdef TEST_MODE 
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
#endif // TEST_MODE 
