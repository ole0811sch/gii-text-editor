#include "../commands.cpp"

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

int test_commands(void) {
	return test_parse_cli();
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
