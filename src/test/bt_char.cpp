#include "../bt_char.cpp"

#include "bt_char.h"
#include "main.h"

int test_bt_char(unsigned int base_indentation, void* _1, void* _2) {
	struct named_test tests[1];
	tests[0] = CREATE_TEST_ID(test_bt_strcmp);
	return run_test_suite_nrnanmrs(tests, ARR_LEN(tests), base_indentation);
}

int test_bt_strcmp(unsigned int base_indentation, void* _1, void* _2) {
	char results[10];
	char s0[] = "Test";
	char s1[] = "Test";
	s0[4] |= 0x80;
	s1[3] |= 0x80;
	results[0] = bt_strcmp(s0, s1) == 0;
	results[1] = bt_strcmp(s0, "Test") == 0;
	results[2] = bt_strcmp(s1, s0) == 0;
	s1[2] |= 0x80;
	results[3] = bt_strcmp(s0, s1) > 0;
	results[4] = bt_strcmp(s1, s0) < 0;
	s1[2] = '\0';
	results[5] = bt_strcmp(s0, s1) > 0;
	results[6] = bt_strcmp(s1, s0) < 0;
	results[7] = bt_strcmp(s0, NULL) > 0;
	results[8] = bt_strcmp(NULL, s0) < 0;
	results[9] = bt_strcmp(NULL, NULL) == 0;

	return run_test_suite_check_results(results, ARR_LEN(results),
			base_indentation);
}
