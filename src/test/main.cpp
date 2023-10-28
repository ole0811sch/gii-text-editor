#include "line_utils.h"
#include <stdio.h>
#include <stdlib.h>

// for LSP, should be defined anyway when building
#ifndef TEST_MODE
#define TEST_MODE
#endif // TEST_MODE

#include "editor.h"
#include "commands.h"
#include "main.h"
#include "bt_char.h"


#include "../main.cpp"

int main(int argc, char** argv) {
	struct named_test tests[4];
	tests[0] = CREATE_TEST_ID(test_editor);
	tests[1] = CREATE_TEST_ID(test_commands);
	tests[2] = CREATE_TEST_ID(test_line_utils);
	tests[3] = CREATE_TEST_ID(test_bt_char);
	return run_test_suite("Unit tests", tests, ARR_LEN(tests), 0, 1, 0, 1,
				NULL, NULL) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static struct named_test create_test(const char* name, 
		char supply_indentation, char supply_index, 
		void* f) {
	struct named_test test;
	test.name = name;
	test.supply_indentation = supply_indentation;
	test.supply_index = supply_index;
	if (supply_indentation) {
		if (supply_index)
			test.f.ix_id = (int (*)(unsigned int, void*, void*, unsigned int))
				f;
		else
			test.f.id = (int (*)(unsigned int, void*, void*)) f;
	} else if (supply_index) {
		test.f.ix = (int (*)(void*, void*, unsigned int)) f;
	} else {
		test.f.f = (int (*)(void*, void*)) f;
	}

	return test;
}

struct named_test create_test_f(const char* name, int (*f)(void*, void*)) {
	return create_test(name, 0, 0, (void*) f);
}

struct named_test create_test_id(const char* name, int (*id)(unsigned int, 
			void*, void*)) {
	return create_test(name, 1, 0, (void*) id);
}

struct named_test create_test_ix(const char* name, int (*ix)(void*, void*,
			unsigned int)) {
	return create_test(name, 0, 1, (void*) ix);
}

struct named_test create_test_ix_id(const char* name, int (*ix_id)(unsigned int, 
			void*, void*,
			unsigned int)) {
	return create_test(name, 1, 1, (void*) ix_id);
}
