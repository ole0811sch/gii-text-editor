#include "clipboard.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

/**
 * contents of the clipboard. Heap allocated, global throughout application.
 * Initially NULL.
 */
char* clipboard_contents = NULL;

int copy_to_clipboard(const char* str, size_t len) {
	void* new_ptr = realloc(clipboard_contents, len + 1);
	if (!new_ptr) {
		display_error("Could not copy to clipboard: not enough memory");
		return -1;
	}
	memcpy(new_ptr, str, len);
	clipboard_contents = (char*) new_ptr;
	clipboard_contents[len] = '\0';
	return 0;
}
