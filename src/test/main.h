#ifndef TEST_MAIN_H_
#define TEST_MAIN_H_

#include <stddef.h>
#include <stdio.h>

struct named_test {
	/**
	 * name of the test. May be NULL
	 */
	const char* name;
	/**
	 * flags that signal what arguments func receives:
	 * 	supply_indentation: whether base_indentation should be supplied (first
	 * 	arg)
	 * 	supply_index: whether the index of the test should be supplied (last
	 * 	arg)
	 * 	Each combination responds to a member of f
	 */
	char supply_indentation, supply_index;
	/**
	 * receives base_indentation for any printing (see run_test_suite), some
	 * pointer which can be supplied to run_test_suite, and the index of the
	 * test in the tests array. Runs and returns whether the test succeeded 
	 * (failure = 0)
	 */
	/**
	 * if 0, only failures will be printed
	 */
	char report_success;
	union {
	int (*ix)(void*, void*, unsigned int);
	int (*f)(void*, void*);
	int (*ix_id)(unsigned int, void*, void*, unsigned int);
	int (*id)(unsigned int, void*, void*);
	} f;
};

static inline int const_true(void* _1, void* _2) {
	return 1;
}

static inline int const_false(void* _1, void* _2) {
	return 0;
}

static inline int test_char_true(void* arr, void* _, unsigned int i) {
	return ((char*) arr)[i] != 0;
}

/**
 * prints indentation times '\t' to stdout
 */
static inline void indent(unsigned int indentation) {
	for (unsigned int j = 0; j < indentation; ++j) {
		putc('\t', stdout);
	}
}

/**
 * Only prints total result of suite if root is set to true. Else it only prints
 * the results of the individual tests. Runs all tests, even if one fails.
 * 
 * @param suite_name only relevant if root 
 * @param base_indentation refers to the indentation that the results of the 
 * entire suite would be printed at, if root is true.
 * @param tests array of tests
 * @param n number of tests
 * @param if multi_test, the function will call the first the of tests n times
 */
static inline int run_test_suite(const char* suite_name, 
		struct named_test* tests, size_t n, unsigned int base_indentation, 
		int root, int multi_test, void* arg1, void* arg2) {
	if (root) {
		indent(base_indentation);
		printf("Test suite \"%s\":\n", suite_name);
	}
	int all_good = 1;
	for (size_t i = 0; i < n; ++i) {
		int i_ = multi_test ? 0 : i;
		if (tests[i_].report_success) {
			indent(base_indentation + 1);
			if (tests[i_].name) {
				printf("Test \"%s\":\n", tests[i_].name);
			} else {
				printf("Test %zu:\n", i);
			}
		}
		int res;
		if (tests[i_].supply_index) {
			if (tests[i_].supply_indentation) {
				res = tests[i_].f.ix_id(base_indentation + 1, arg1, arg2, i);
			} else {
				res = tests[i_].f.ix(arg1, arg2, i);
			}
		} else {
			if (tests[i_].supply_indentation) {
				res = tests[i_].f.id(base_indentation + 1, arg1, arg2);
			} else {
				res = tests[i_].f.f(arg1, arg2);
			}
		}
		if (!res && !tests[i_].report_success) {
			indent(base_indentation + 1);
			if (tests[i_].name) {
				printf("Test \"%s\":\n", tests[i_].name);
			} else {
				printf("Test %zu:\n", i);
			}
		}
		if (tests[i_].report_success || !res) {
			indent(base_indentation + 1);
			printf("Test %s\n", res ? "passed" : "failed");
		}
		all_good = res && all_good;
	}
	if (root) {
		indent(base_indentation);
		printf("Test suite \"%s\" %s\n", suite_name,
				all_good ? "passed" : "failed");
	}
	return all_good;
}

#endif // TEST_MAIN_H_
