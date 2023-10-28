#include "../bt_char.cpp"

#include "bt_char.h"
#include "main.h"

int test_bt_char(unsigned int base_indentation, void* _1, void* _2) {
	struct named_test tests[1];
	tests[0].name = "test_bt_strcmp";
	tests[0].supply_index = 0;
	tests[0].supply_indentation = 1;
	tests[0].f.id = &test_bt_strcmp;
	tests[0].report_success = 1;
	return run_test_suite(NULL, tests, ARR_LEN(tests), base_indentation, 0, 0,
			NULL, NULL);
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

	struct named_test test;
	test.name = NULL;
	test.supply_indentation = 0;
	test.supply_index = 1;
	test.f.ix = &test_char_true;
	test.report_success = 0;
	return run_test_suite(NULL, &test, ARR_LEN(results),
			base_indentation, 0, 1, results, NULL);
}
