#ifndef TEST_MAIN_H_
#define TEST_MAIN_H_

#include <stddef.h>
#include <stdio.h>
#include "../util.h"



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
	union test_funcs {
		int (*ix)(void*, void*, unsigned int);
		int (*f)(void*, void*);
		int (*ix_id)(unsigned int, void*, void*, unsigned int);
		int (*id)(unsigned int, void*, void*);
	} f;
};

static inline int const_true(void* _1, void* _2) {
	return 1;
}

#define CREATE_TEST_IX_ID(f) create_test_ix_id(#f, &(f))
#define CREATE_TEST_ID(f) create_test_id(#f, &(f))
#define CREATE_TEST_IX(f) create_test_ix(#f, &(f))
#define CREATE_TEST_F(f) create_test_f(#f, &(f))
struct named_test create_test_ix_id(const char* name, int (*ix_id)(unsigned int, 
			void*, void*, unsigned int));
struct named_test create_test_ix(const char* name, int (*ix)(void*, void*,
			unsigned int));
struct named_test create_test_id(const char* name, int (*id)(unsigned int, 
			void*, void*));
struct named_test create_test_f(const char* name, int (*f)(void*, void*));

static inline int const_false(void* _1, void* _2) {
	return 0;
}

static inline int test_char_true(void* arr, void* _, unsigned int i) {
	return ((char*) arr)[i] != 0;
}

static inline struct named_test result_check_test(void) {
	return create_test_ix(NULL, &test_char_true);
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
		char root, char multi_test, char report_success, void* arg1, 
		void* arg2) {
	if (root) {
		indent(base_indentation);
		printf("Test suite \"%s\":\n", suite_name);
	}
	int all_good = 1;
	for (size_t i = 0; i < n; ++i) {
		int i_ = multi_test ? 0 : i;
		if (report_success) {
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
		if (!res && !report_success) {
			indent(base_indentation + 1);
			if (tests[i_].name) {
				printf("Test \"%s\":\n", tests[i_].name);
			} else {
				printf("Test %zu:\n", i);
			}
		}
		if (report_success || !res) {
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

/**
 * runs a test that checks that all n entries in results are true. The tests
 * have no name and only failures will be printed.
 * @return the result of the test
 */
static inline int run_test_suite_check_results(char results[], size_t n,
		unsigned int base_indentation) {
	struct named_test test = result_check_test();
	return run_test_suite(NULL, &test, n, base_indentation, 0, 1, 0, 
			results, NULL);
}

/**
 * no root, no args, no multiple_test, do report_success
 */
static inline int run_test_suite_nrnanmrs(struct named_test* tests, 
		size_t n, unsigned int base_indentation) {
	return run_test_suite(NULL, tests, n, base_indentation, 0, 0, 1, NULL, 
			NULL);
}

#endif // TEST_MAIN_H_
