#include "line_utils.h"
#include <stdio.h>
#include <stdlib.h>

// for LSP, should be defined anyway when building
#ifndef TEST_MODE
#define TEST_MODE
#endif // TEST_MODE

#include "editor.h"
#include "commands.h"
#include "main.h"
#include "bt_char.h"


#include "../main.cpp"

int main(int argc, char** argv) {
	struct named_test tests[4];
	tests[0].name = "test_editor";
	tests[0].supply_index = 0;
	tests[0].supply_indentation = 1;
	tests[0].f.id = &test_editor;
	tests[0].report_success = 1;
	tests[1].name = "test_commands";
	tests[1].supply_index = 0;
	tests[1].supply_indentation = 1;
	tests[1].f.id = &test_commands;
	tests[1].report_success = 1;
	tests[2].name = "test_line_utils";
	tests[2].supply_index = 0;
	tests[2].supply_indentation = 1;
	tests[2].f.id = &test_line_utils;
	tests[2].report_success = 1;
	tests[3].name = "test_bt_char";
	tests[3].supply_index = 0;
	tests[3].supply_indentation = 1;
	tests[3].f.id = &test_bt_char;
	tests[3].report_success = 1;
	return run_test_suite("Unit tests", tests, ARR_LEN(tests), 0, 1, 0, 
				NULL, NULL) ? EXIT_SUCCESS : EXIT_FAILURE;
}
