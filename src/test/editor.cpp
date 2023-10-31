#include <string.h>

#include "../editor.cpp"
#include "editor.h"
#include "main.h"

static int test_count_softbreaks(unsigned int bi, void* _1, void* _2);
static int test_initialize_text_box(unsigned int bi, void* _1, void* _2);
static int test_update_changes_from(unsigned int bi, void* _1, void* _2);
static int test_insert_string(unsigned int indent, void* _1, void* _2);

int test_editor(unsigned int base_indentation, void* _1, void* _2) {
	struct named_test tests[4];
	tests[0] = CREATE_TEST_ID(test_initialize_text_box);
	tests[1] = CREATE_TEST_ID(test_count_softbreaks);
	tests[2] = CREATE_TEST_ID(test_update_changes_from);
	tests[3] = CREATE_TEST_ID(test_insert_string);
	return run_test_suite_nrnanmrs(tests, ARR_LEN(tests), base_indentation);
}

static int test_initialize_text_box(unsigned int bi, void* _1, void* _2) {
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
			bi);
}

static int test_count_softbreaks(unsigned int bi, void* _1, void* _2) {
	text_box_t box;
	initialize_text_box(12, 34, 10, 10, CURSOR, 1,
			"0123456789ABCDEFGHIJ\n012\n\n0123456789ABCDEFGHIJ012345678\n", 
			&box);
	char results[5];
	results[0] = count_softbreaks(&box, &box.lines.arr[0]) == 1;
	results[1] = count_softbreaks(&box, &box.lines.arr[1]) == 0;
	results[2] = count_softbreaks(&box, &box.lines.arr[2]) == 0;
	results[3] = count_softbreaks(&box, &box.lines.arr[3]) == 2;
	results[4] = count_softbreaks(&box, &box.lines.arr[4]) == 0;

	return run_test_suite_check_results(results, ARR_LEN(results), 
			bi);
}

static int test_update_changes_from(unsigned int bi, void* _1, void* _2) {
	text_box_t box;
	initialize_text_box(12, 34, 10, 10, CURSOR, 1,
			"0123456789ABCDEFGHIJ\n012\n\n0123456789ABCDEFGHIJ012345678\n", 
			&box);
	char results[2];
	line_add(&box.lines.arr[0], 'a');
	line_chi_t lc0 = { 0, 19 };
	update_changes_from(&box, lc0);
	results[0] = box.lines.arr[0].vline_begin == 0
		&& box.lines.arr[1].vline_begin == 3
		&& box.lines.arr[2].vline_begin == 4
		&& box.lines.arr[3].vline_begin == 5
		&& box.lines.arr[4].vline_begin == 8;
	line_add(&box.lines.arr[0], 'a');
	line_chi_t lc1 = { 0, 20 };
	update_changes_from(&box, lc1);
	results[1] = box.lines.arr[0].vline_begin == 0
		&& box.lines.arr[1].vline_begin == 3
		&& box.lines.arr[2].vline_begin == 4
		&& box.lines.arr[3].vline_begin == 5
		&& box.lines.arr[4].vline_begin == 8;
	return run_test_suite_check_results(results, ARR_LEN(results), bi);
}

static int test_insert_string(unsigned int indent, void* _1, void* _2) {
	text_box_t box;
	initialize_text_box(12, 34, 10, 10, CURSOR, 1,
			"0123456789ABCDEFGHIJ\n012\n\n0123456789ABCDEFGHIJ012345678\n", 
			&box);
	char results[22];
	insert_string(&box, "a", 1);
	results[0] = bt_strcmp(box.lines.arr[0].str, "a0123456789ABCDEFGHIJ") == 0
		&& bt_strcmp(box.lines.arr[1].str, "012") == 0;
	results[1] = box.lines.arr[0].vline_begin == 0
		&& box.lines.arr[1].vline_begin == 3
		&& box.lines.arr[2].vline_begin == 4
		&& box.lines.arr[3].vline_begin == 5
		&& box.lines.arr[4].vline_begin == 8;
	results[2] = box.cursor.position.line == 0
		&& box.cursor.position.char_i == 1;
	line_chi_t end_redraw = { box.lines.count - 1, 
		find_line_len(&box.lines.arr[box.lines.count - 1]) };
	line_chi_t begin_redraw = { 0, 0 };
	results[3] = line_chi_greater_equals(box.redraw_areas.changes_end, 
			end_redraw);
	results[4] = line_chi_less_equals(box.redraw_areas.changes_begin, 
			begin_redraw);
	results[5] = box.redraw_areas.vvlines_begin_changed == 0;

	insert_string(&box, "\n", 1);
	results[6] = bt_strcmp(box.lines.arr[0].str, "a") == 0
		&& bt_strcmp(box.lines.arr[1].str, "0123456789ABCDEFGHIJ") == 0
		&& bt_strcmp(box.lines.arr[2].str, "012") == 0;
	results[7] = box.cursor.position.line == 1
		&& box.cursor.position.char_i == 0;
	end_redraw.line = box.lines.count - 1;
	end_redraw.char_i = find_line_len(&box.lines.arr[box.lines.count - 1]);
	begin_redraw.line = 0;
	begin_redraw.char_i = 1;
	results[8] = line_chi_greater_equals(box.redraw_areas.changes_end, 
			end_redraw);
	results[9] = line_chi_less_equals(box.redraw_areas.changes_begin, 
			begin_redraw);
	results[10] = box.redraw_areas.vvlines_begin_changed == 0;
	results[11] = box.lines.arr[0].vline_begin == 0
		&& box.lines.arr[1].vline_begin == 1
		&& box.lines.arr[2].vline_begin == 3
		&& box.lines.arr[3].vline_begin == 4
		&& box.lines.arr[4].vline_begin == 5
		&& box.lines.arr[5].vline_begin == 8;

	destruct_text_box(&box);
	initialize_text_box(12, 34, 10, 10, CURSOR, 1,
			"01234567\n012\n\n0123456789ABCDEFGHIJ012345678\n", 
			&box);
	insert_string(&box, "aaaa", 1);
	results[12] = bt_strcmp(box.lines.arr[0].str, "aaaa01234567") == 0
		&& bt_strcmp(box.lines.arr[1].str, "012") == 0
		&& bt_strcmp(box.lines.arr[2].str, "") == 0;
	results[13] = box.cursor.position.line == 0
		&& box.cursor.position.char_i == 4;
	end_redraw.line = box.lines.count - 1;
	end_redraw.char_i = find_line_len(&box.lines.arr[box.lines.count - 1]);
	begin_redraw.line = 0;
	begin_redraw.char_i = 0;
	results[14] = line_chi_greater_equals(box.redraw_areas.changes_end, 
			end_redraw);
	results[15] = line_chi_less_equals(box.redraw_areas.changes_begin, 
			begin_redraw);
	results[16] = box.redraw_areas.vvlines_begin_changed == 0;
	results[17] = box.lines.arr[0].vline_begin == 0
		&& box.lines.arr[1].vline_begin == 2
		&& box.lines.arr[2].vline_begin == 3
		&& box.lines.arr[3].vline_begin == 4
		&& box.lines.arr[4].vline_begin == 7;

	destruct_text_box(&box);
	initialize_text_box(12, 34, 10, 10, CURSOR, 1,
			"01234567\n012\n\n0123456789ABCDEFGHIJ012345678\n", 
			&box);
	insert_string(&box, "a", 1);
	insert_string(&box, "a", 1);
	insert_string(&box, "a", 1);
	insert_string(&box, "a", 1);
	results[18] = bt_strcmp(box.lines.arr[0].str, "aaaa01234567") == 0
		&& bt_strcmp(box.lines.arr[1].str, "012") == 0
		&& bt_strcmp(box.lines.arr[2].str, "") == 0;
	results[19] = box.cursor.position.line == 0
		&& box.cursor.position.char_i == 4;
	end_redraw.line = box.lines.count - 1;
	end_redraw.char_i = find_line_len(&box.lines.arr[box.lines.count - 1]);
	begin_redraw.line = 0;
	begin_redraw.char_i = 0;
	results[20] = box.redraw_areas.vvlines_begin_changed == 0;
	results[21] = box.lines.arr[0].vline_begin == 0
		&& box.lines.arr[1].vline_begin == 2
		&& box.lines.arr[2].vline_begin == 3
		&& box.lines.arr[3].vline_begin == 4
		&& box.lines.arr[4].vline_begin == 7;


	return run_test_suite_check_results(results, ARR_LEN(results), indent);
}
