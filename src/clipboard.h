#ifndef CLIPBOARD_H_
#define CLIPBOARD_H_
#include <stddef.h>

/**
 * str doesn't have to be NULL terminated, since the number of chars is supplied
 * by len.
 * @param len strlen(str)
 * @return 0 on success, -1 if str could not be copied
 */
int copy_to_clipboard(const char* str, size_t len);
extern char* clipboard_contents;

#endif // CLIPBOARD_H_
