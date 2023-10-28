#include "../line_utils.cpp"
#include "line_utils.h"
#include <string.h>
#include "../util.h"
#include "main.h"

static int test_find_line_len(unsigned int, void* _1, void* _2);
static int test_find_line_capacity(unsigned int, void* _1, void* _2);
static int test_is_line_end(unsigned int base_indentation, void* _1, void* _2);

int test_line_utils(unsigned int base_indentation, void* _1, void* _2) {
	struct named_test tests[3];
	tests[0] = CREATE_TEST_ID(test_find_line_len);
	tests[1] = CREATE_TEST_ID(test_find_line_capacity);
	tests[2] = CREATE_TEST_ID(test_is_line_end);
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
