#include <string.h>

#include "../editor.cpp"
#include "editor.h"
#include "main.h"

int test_editor(unsigned int base_indentation, void* _1, void* _2) {
	struct named_test tests[1];
	tests[0] = CREATE_TEST_ID(test_initialize_text_box);
	return run_test_suite_nrnanmrs(tests, ARR_LEN(tests), base_indentation);
}

int test_initialize_text_box(unsigned int base_indentation, void* _1, void* _2) {
	text_box_t box;
	initialize_text_box(12, 34, 10, 5, CURSOR, 1, 
			"0123456789ABCDEFGHIJ\n012\n\n0123456789\n", &box);
	char results[4];
	results[0] = box.left_px == 12 
		&& box.top_px == 34 
		&& box.width == 10 
		&& box.height == 5
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
	results[3] = box.lines.arr[0].vline_begin == 0
		&& box.lines.arr[1].vline_begin == 2
		&& box.lines.arr[2].vline_begin == 3
		&& box.lines.arr[3].vline_begin == 4
		&& box.lines.arr[4].vline_begin == 5;

	return run_test_suite_check_results(results, ARR_LEN(results), 
			base_indentation);
}

int test_count_softbreaks(unsigned int base_indentation, void* _1, void* _2) {
	text_box_t box;
	initialize_text_box(12, 34, 32, 10, CURSOR, 1,
			"0123456789ABCDEFGHIJ\n012\n\n0123456789ABCDEFGHIJ012345678\n", 
			&box);
	char results[5];
	results[0] = count_softbreaks(&box, &box.lines.arr[0]) == 1;
	results[1] = count_softbreaks(&box, &box.lines.arr[1]) == 0;
	results[2] = count_softbreaks(&box, &box.lines.arr[2]) == 0;
	results[3] = count_softbreaks(&box, &box.lines.arr[3]) == 2;
	results[4] = count_softbreaks(&box, &box.lines.arr[4]) == 0;

	return run_test_suite_check_results(results, ARR_LEN(results), 
			base_indentation);
}
