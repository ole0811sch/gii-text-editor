#include <string.h>

#include "../editor.cpp"
#include "editor.h"
#include "main.h"

int test_editor(unsigned int base_indentation, void* _1, void* _2) {
	struct named_test tests[1];
	tests[0].supply_indentation = 1;
	tests[0].supply_index = 0;
	tests[0].name = "test_initialize_text_box";
	tests[0].report_success = 1;
	tests[0].f.id = &test_initialize_text_box;
	return run_test_suite(NULL, tests, ARR_LEN(tests), base_indentation, 0, 0,
			NULL, NULL);
}

int test_initialize_text_box(unsigned int base_indentation, void* _1, void* _2) {
	text_box_t box;
	initialize_text_box(12, 34, 32, 10, CURSOR, 1, 
			"0123456789ABCDEFGHIJ\n012\n\n0123456789\n", &box);
	char results[3];
	results[0] = box.left_px == 12 
		&& box.top_px == 34 
		&& box.width == 32 
		&& box.height == 10
		&& box.interaction_mode == CURSOR
		&& box.cursor.editable == 1;
	results[1] = box.vvlines_begin == 0
		&& box.cursor.position.line == 0
		&& box.cursor.position.char_i == 0
		&& box.cursor.visual_mode == 0
		&& box.cursor.cursor_x_target == 0
		&& box.cursor.editable_state.capitalization_on == 0
		&& box.cursor.editable_state.capitalization_on_locked == 0;
	results[2] = box.lines.count == 5
		&& bt_strcmp(box.lines.arr[0].str, "0123456789ABCDEFGHIJ") == 0
		&& bt_strcmp(box.lines.arr[1].str, "012") == 0
		&& bt_strcmp(box.lines.arr[2].str, "") == 0
		&& bt_strcmp(box.lines.arr[3].str, "0123456789") == 0
		&& bt_strcmp(box.lines.arr[4].str, "") == 0;

	struct named_test test;
	test.name = NULL;
	test.supply_index = 1;
	test.supply_indentation = 0;
	test.f.ix = &test_char_true;
	test.report_success = 0;
	return run_test_suite(NULL, &test, ARR_LEN(results), base_indentation, 0, 1,
			results, NULL);
}
