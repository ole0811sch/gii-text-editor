#include "../line_utils.cpp"

#include "../editor.h"
#include "line_utils.h"
#include <string.h>
#include "../util.h"
#include "main.h"

static int test_find_line_len(unsigned int, void* _1, void* _2);
static int test_find_line_capacity(unsigned int, void* _1, void* _2);
static int test_is_line_end(unsigned int base_indentation, void* _1, void* _2);
static int test_char_i_to_vline_offset(unsigned int indent, void* _, void* _1);

int test_line_utils(unsigned int base_indentation, void* _1, void* _2) {
	struct named_test tests[4];
	tests[0] = CREATE_TEST_ID(test_find_line_len);
	tests[1] = CREATE_TEST_ID(test_find_line_capacity);
	tests[2] = CREATE_TEST_ID(test_is_line_end);
	tests[3] = CREATE_TEST_ID(test_char_i_to_vline_offset);
	return run_test_suite_nrnanmrs(tests, ARR_LEN(tests), base_indentation);
}

static int test_find_line_len(unsigned int base_indentation, void* _1, 
		void* _2) {
	line_t line = {0};
	char str[4];
	char results[7];
	results[0] = find_line_len(&line) == 0;
	line.str = str;
	str[0] = 0;
	str[1] = 0;
	str[2] = 0;
	str[3] = (char) 0x80;
	results[1] = find_line_len(&line) == 0;
	str[0] = 'a';
	results[2] = find_line_len(&line) == 1;
	str[0] = (char) 0x80;
	results[3] = find_line_len(&line) == 0;
	str[0] = (char) 0x80 | 'a';
	results[3] = find_line_len(&line) == 1;
	str[0] = 'a';
	str[1] = (char) 0x80 | 'a';
	results[4] = find_line_len(&line) == 2;
	str[0] = 'a';
	str[1] = (char) 0x80;
	results[5] = find_line_len(&line) == 1;
	str[0] = 'a';
	str[1] = 'a';
	str[2] = '\t';
	str[3] = 'a' | (char) 0x80;
	results[6] = find_line_len(&line) == 4;
	return run_test_suite_check_results(results, ARR_LEN(results), 
			base_indentation);
}

static int test_find_line_capacity(unsigned int base_indentation, 
		void* _1, void* _2) {
	line_t line = {0};
	char str[4];
	char results[7];
	results[0] = find_line_capacity(&line) == 0;
	line.str = str;
	str[0] = 0;
	str[1] = 0;
	str[2] = 0;
	str[3] = (char) 0x80;
	results[1] = find_line_capacity(&line) == 4;
	str[0] = (char) 0x80;
	results[2] = find_line_capacity(&line) == 1;
	str[0] = (char) 0x80 | 'a';
	results[2] = find_line_capacity(&line) == 1;
	str[0] = 'a';
	str[1] = (char) 0x80 | 'a';
	results[4] = find_line_capacity(&line) == 2;
	str[0] = 'a';
	str[1] = 'a';
	str[2] = 0;
	str[3] = (char) 0x80;
	results[1] = find_line_capacity(&line) == 4;
	str[0] = 'a';
	str[1] = 'a';
	str[2] = 'a';
	str[3] = 'a' | (char) 0x80;
	results[1] = find_line_capacity(&line) == 4;

	return run_test_suite_check_results(results, ARR_LEN(results), 
			base_indentation);
}

static int test_is_line_end(unsigned int base_indentation, void* _1, void* _2) {
	char results[9];
	char c;
	c = '\0';
	results[0] = is_line_end(&c, 0);
	c = (char) 0x80;
	results[1] = is_line_end(&c, 0);
	c = 'a';
	results[2] = !is_line_end(&c, 0);
	c = 'a' | (char) 0x80;
	results[3] = !is_line_end(&c, 0);
	results[4] = is_line_end(NULL, 0);
	char str[2] = { 'a', '\0' };
	results[5] = is_line_end(str, 1);
	str[0] |= (char) 0x80;
	results[6] = is_line_end(str, 1);
	str[0] &= (char) ~0x80;
	str[1] = 'a' | (char) 0x80;
	results[7] = !is_line_end(str, 1);
	results[8] = is_line_end(str, 2);

	return run_test_suite_check_results(results, ARR_LEN(results), 
			base_indentation);
}

static int test_char_i_to_vline_offset(unsigned int indent, void* _, void* _1) {
	text_box_t box;
	initialize_text_box(0, 0, 3, 5, CURSOR, 1, "ab\nabc\n\nabcd\nabcdefgh", 
			&box);
	char results[11];
	size_t vl_chi;
	size_t vline;
	vline = char_i_to_vline_offset(&box, &box.lines.arr[0], 0, &vl_chi);
	results[0] = vline == 0 && vl_chi == 0;

	vline = char_i_to_vline_offset(&box, &box.lines.arr[1], 0, &vl_chi);
	results[1] = vline == 0 && vl_chi == 0;
	vline = char_i_to_vline_offset(&box, &box.lines.arr[1], 3, &vl_chi);
	results[2] = vline == 0 && vl_chi == 0;

	vline = char_i_to_vline_offset(&box, &box.lines.arr[2], 0, &vl_chi);
	results[3] = vline == 0 && vl_chi == 0;

	vline = char_i_to_vline_offset(&box, &box.lines.arr[3], 2, &vl_chi);
	results[4] = vline == 0 && vl_chi == 0;
	vline = char_i_to_vline_offset(&box, &box.lines.arr[3], 3, &vl_chi);
	results[5] = vline == 1 && vl_chi == 3;
	vline = char_i_to_vline_offset(&box, &box.lines.arr[3], 4, &vl_chi);
	results[6] = vline == 1 && vl_chi == 3;

	vline = char_i_to_vline_offset(&box, &box.lines.arr[4], 1, &vl_chi);
	results[7] = vline == 0 && vl_chi == 0;
	vline = char_i_to_vline_offset(&box, &box.lines.arr[4], 4, &vl_chi);
	results[8] = vline == 1 && vl_chi == 3;
	vline = char_i_to_vline_offset(&box, &box.lines.arr[4], 7, &vl_chi);
	results[9] = vline == 2 && vl_chi == 6;
	vline = char_i_to_vline_offset(&box, &box.lines.arr[4], 8, &vl_chi);
	results[10] = vline == 2 && vl_chi == 6;

	return run_test_suite_check_results(results, ARR_LEN(results), indent);
}
