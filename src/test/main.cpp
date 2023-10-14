#include "line_utils.h"
#include <stdio.h>
#include <stdlib.h>

// for LSP, should be defined anyway when building
#define TEST_MODE

#include "editor.h"
#include "commands.h"


#include "../main.cpp"

int main(int argc, char** argv) {
	return (test_editor()
		&& test_commands()
		&& test_line_utils()
		? EXIT_SUCCESS : EXIT_FAILURE);
}
