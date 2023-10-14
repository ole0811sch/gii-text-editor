#include "line.h"

#define DYN_ARR_IMPLEMENTATION
#define DYN_ARR_CG_TYPE line_t
#define DYN_ARR_CG_SUFFIX line
#include "dyn_arr.h"
#undef DYN_ARR_CG_SUFFIX
#undef DYN_ARR_CG_TYPE
#undef DYN_ARR_IMPLEMENTATION

#define LINE_GROWTH_NUMERATOR 5
#define LINE_GROWTH_DENOMINATOR 4
#define LINE_SHRINK_THRESHOLD_NUMERATOR 3
#define LINE_SHRINK_THRESHOLD_DENOMINATOR 4

void destruct_line(line_t* line) {
	free(line->str);
}

size_t find_line_capacity(const line_t* line) {
	if (!line->str) {
		return 0;
	}
	size_t i;
	for (i = 0; !(line->str[i] & 0x80); ++i);
	return i + 1;
}

size_t find_line_len(const line_t* line) {
	if (!line->str) {
		return 0;
	}
	size_t i;
	for (i = 0;; ++i) {
		if ((line->str[i] & 0x7F) == '\0') {
			return i;
		}
		if (line->str[i] & 0x80) {
			return i + 1;
		}
	}
}

/**
 * Can be used for expansion and shrinking. Sets bytes to 0 from bytes len to
 * new_capacity - 1. Sets new eol marker and erases old one. old_capacity and
 * new_capacity may both be 0.
 */
int line_change_size(line_t* line, size_t old_capacity, size_t len,
		size_t new_capacity) {
	char* new_str = (char*) realloc(line->str, new_capacity * sizeof(char*));
	if (new_str == NULL && new_capacity > 0) {
		return -1;
	}

	if (new_capacity > old_capacity && old_capacity > 0) {
		// clear eol marker bit
		new_str[old_capacity - 1] &= ~0x80;
	}
	for (size_t i = len; i < new_capacity; ++i) {
		new_str[i] = '\0';
	}

	if (new_capacity > 0) {
		new_str[new_capacity - 1] |= 0x80;
	}
	line->str = new_str;
	return 0;
}

int line_ensure_capacity(line_t* line, size_t old_capacity, size_t len,
		size_t new_capacity, size_t* cap_ret) {
	if (new_capacity <= old_capacity) {
		*cap_ret = old_capacity;
		return 0;
	}
	size_t new_capacity_actual = old_capacity;
	while (new_capacity > new_capacity_actual) {
		size_t new_capacity_actual_2 = new_capacity_actual 
			* LINE_GROWTH_NUMERATOR 
			/ LINE_GROWTH_DENOMINATOR;
		if (new_capacity_actual_2 <= new_capacity_actual)
			++new_capacity_actual;
		else
			new_capacity_actual = new_capacity_actual_2;
	}

	*cap_ret = new_capacity_actual;
	return line_change_size(line, old_capacity, len, new_capacity_actual);
}

/**
 * the MSB in every char in char will not be copied.
 */
int line_add_all(line_t* line, const char* chars, size_t n) {
	return line_insert_some(line, chars, n, (size_t) 0 - 1);
}

int line_pop_some(line_t* line, size_t n) {
	size_t len = find_line_len(line);
	size_t cap = find_line_capacity(line);
	if (n > len)
		n = len;
	len -= n;

	// find smallest capacity that's over the threshold
	size_t next_capacity = cap;
	size_t new_cap;
	do {
		new_cap = next_capacity;
		next_capacity = new_cap * LINE_GROWTH_DENOMINATOR 
			/ LINE_GROWTH_NUMERATOR;
	} while (next_capacity * LINE_SHRINK_THRESHOLD_NUMERATOR 
						/ LINE_SHRINK_THRESHOLD_DENOMINATOR 
						> len);

	return line_change_size(line, cap, len, new_cap);
}

/**
 * the MSB in every char in char will not be copied.
 * index will be set to len(line) if it's larger than that.
 */
int line_insert_some(line_t* line, const char* chars, size_t n, size_t index) {
	if (n == 0) {
		return 0;
	}

	size_t len = find_line_len(line);
	size_t cap = find_line_capacity(line);
	size_t new_cap = cap;
	if (len + n > cap
			&& line_ensure_capacity(line, cap, len, len + n, &new_cap) == -1) {
		return -1;
	}

	if (index > len) {
		index = len;
	}

	len += n;
	for (size_t i = len - 1; i >= index + n; --i) {
		line->str[i] = line->str[i - n];
	}
	for (size_t i = 0; i < n; ++i) {
		line->str[index + i] = chars[i] & ~0x80;
	}
	line->str[new_cap - 1] |= 0x80;
	return 0;
}

int line_remove_some(line_t* line, size_t start_i, size_t end_i) {
	size_t len = find_line_len(line);

	if (end_i > len) {
		end_i = len;
	}
	if (start_i >= end_i) {
		return 0;
	}

	size_t diff = end_i - start_i;
	for (size_t i = start_i; i < len - diff; ++i) {
		line->str[i] = line->str[i + diff];
	}
	return line_pop_some(line, diff);
}

int line_remove(line_t* line, size_t index) {
	return line_remove_some(line, index, index + 1);
}

int line_pop(line_t* line) {
	return line_pop_some(line, 1);
}

/**
 * the MSB in c will not be copied.
 */
int line_add(line_t* line, char c) {
	return line_add_all(line, &c, 1);
}

/**
 * the MSB in c will not be copied.
 */
int line_insert(line_t* line, char c, size_t index) {
	return line_insert_some(line, &c, 1, index);
}
