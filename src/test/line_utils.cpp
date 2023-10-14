#include "../line_utils.cpp"
#include "line_utils.h"
#include <string.h>
#include "../util.h"

static int test_find_line_len(void);
static int test_find_line_capacity(void);

int test_line_utils(void) {
	return test_find_line_len()
		&& test_find_line_capacity();
}

static int test_find_line_len(void) {
	line_t line = {0};
	char str[4];
	char results[7];
	results[0] = find_line_len(&line) == 0;
	line.str = str;
	str[0] = 0;
	str[1] = 0;
	str[2] = 0;
	str[3] = 0x80;
	results[1] = find_line_len(&line) == 0;
	str[0] = 'a';
	results[2] = find_line_len(&line) == 1;
	str[0] = 0x80;
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
	return all_true(results, ARR_LEN(results));
}

static int test_find_line_capacity(void) {
	line_t line = {0};
	char str[4];
	char results[7];
	results[0] = find_line_capacity(&line) == 0;
	line.str = str;
	str[0] = 0;
	str[1] = 0;
	str[2] = 0;
	str[3] = 0x80;
	results[1] = find_line_capacity(&line) == 4;
	str[0] = 0x80;
	results[2] = find_line_capacity(&line) == 1;
	str[0] = (char) 0x80 | 'a';
	results[2] = find_line_capacity(&line) == 1;
	str[0] = 'a';
	str[1] = (char) 0x80 | 'a';
	results[4] = find_line_capacity(&line) == 2;
	str[0] = 'a';
	str[1] = 'a';
	str[2] = 0;
	str[3] = 0x80;
	results[1] = find_line_capacity(&line) == 4;
	str[0] = 'a';
	str[1] = 'a';
	str[2] = 'a';
	str[3] = 'a' | (char) 0x80;
	results[1] = find_line_capacity(&line) == 4;
	return all_true(results, ARR_LEN(results));
}
