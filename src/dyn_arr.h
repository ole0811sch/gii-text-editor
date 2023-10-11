/**
 * guards:
 * #ifndef DYN_ARR_H_
 * // declarations
 * #endif
 * #ifdef DYN_ARR_IMPLEMENTATION
 * // implementation
 * #endif
 * The implementation relies on the declarations.
 *
 * If you want to get the declarations, you need to make sure that
 * DYN_ARR_H_ is not defined. In case you already included the declarations
 * for another type, you need to undefine DYN_ARR_H_.
 * If you want to get the implementation, you also need to make sure that
 * DYN_ARR_H_ is not defined, or that you already have the declaration for that
 * type.
 *
 * Configuration macros:
 * DYN_ARR_CG_TYPE: the type to store, e.g. `size_t` or `struct some_struct`.
 *
 * DYN_ARR_CG_SUFFIX: the string that is used in the names (denoted with $type
 * in the documentation). Must not contain anything that cannot be in the middle
 * of an identifier (like spaces or dashes).
 *
 * DYN_ARR_CG_STATIC (optional): define as static if you want the functions to
 * be static (default not static).
 *
 * DYN_ARR_CG_SHRINK_THRESHOLD_NUMERATOR and 
 * DYN_ARR_CG_SHRINK_THRESHOLD_DENOMINATOR (optional): the next smaller 
 * capacity nc is taken if nc * DYN_ARR_CG_SHRINK_THRESHOLD_NUMERATOR / 
 * DYN_ARR_CG_SHRINK_THRESHOLD_DENOMINATOR is still greater than the number of
 * elements in the array (default 3 and 4).
 *
 * DYN_ARR_CG_SIMPLE_COMPARISON (optional): define this to include the binary 
 * search functions that directly use <, > and = for comparison.
 *
 * DYN_ARR_CG_MALLOC, DYN_ARR_CG_FREE and DYN_ARR_CG_REALLOC (optional): 
 * functions used for malloc, free and realloc, respectively. Should behave 
 * like the standard implementation, which is the default. If one is defined,
 * all need to be defined.
 *
 * None of the configuration macros will be undefined after the header.
 *
 * Example usage:
 *
 * -- dynamic_string.c --
 *  #define DYN_ARR_IMPLEMENTATION
 *  #define DYN_ARR_CG_TYPE char
 *  #define DYN_ARR_CG_SUFFIX char
 *  #define DYN_ARR_CG_SIMPLE_COMPARISON
 *  #include "dyn_arr.h"
 *  #undef DYN_ARR_IMPLEMENTATION
 *  #undef DYN_ARR_CG_TYPE
 *  #undef DYN_ARR_CG_SUFFIX
 *  #undef DYN_ARR_CG_SIMPLE_COMPARISON
 *
 * -- dynamic_string.h -- 
 *  #define DYN_ARR_CG_TYPE char
 *  #define DYN_ARR_CG_SUFFIX char
 *  #define DYN_ARR_CG_SIMPLE_COMPARISON
 *  #include "dyn_arr.h"
 *  #undef DYN_ARR_CG_TYPE
 *  #undef DYN_ARR_CG_SUFFIX
 *  #undef DYN_ARR_CG_SIMPLE_COMPARISON
 *
 * -- some_other_file.c --
 *  #include "dynamic_string.h"
 *  dyn_arr_char_t str;
 *  dyn_arr_char_create(10, //	initial capacity
 *  	3,	// growth factor numerator
 *  	2, 	// growth factor denominator
*  		&str);
*  	...
 */

#ifndef DYN_ARR_CG_TYPE
#error "You need to define DYN_ARR_CG_TYPE to use the implementation of dyn_arr.h" 
#endif // DYN_ARR_CG_TYPE
#ifndef DYN_ARR_CG_SUFFIX
#error "You need to define DYN_ARR_CG_SUFFIX"
#endif // DYN_ARR_CG_SUFFIX

#define _cat3(w, x, y) w##x##y
#define _cat4(w, x, y, z) w##x##y##z
#define _cat3E(w, x, y) _cat3(w, x, y)
#define _cat4E(w, x, y, z) _cat4(w, x, y, z)
#define _DYN_ARR_F1S(y) _cat4E(dyn_arr_, DYN_ARR_CG_SUFFIX, _, y)
#define _DYN_ARR_F1SP(y) _cat4E(_dyn_arr_, DYN_ARR_CG_SUFFIX, _, y)
#define DYN_ARR_T _DYN_ARR_F1S(t)
#define DYN_ARR_CREATE _DYN_ARR_F1S(create)
#define DYN_ARR_ADD _DYN_ARR_F1S(add)
#define DYN_ARR_INSERT _DYN_ARR_F1S(insert)
#define DYN_ARR_INSERT_SOME _DYN_ARR_F1S(insert_some)
#define _DYN_ARR_EXTEND _DYN_ARR_F1SP(extend)
#define DYN_ARR_ENSURE_CAPACITY _DYN_ARR_F1S(ensure_capacity)
#define _DYN_ARR_SHRINK _DYN_ARR_F1SP(shrink)
#define _DYN_ARR_CHANGE_SIZE _DYN_ARR_F1SP(change_size)
#define DYN_ARR_REMOVE _DYN_ARR_F1S(remove)
#define DYN_ARR_POP_SOME _DYN_ARR_F1S(pop_some)
#define DYN_ARR_POP _DYN_ARR_F1S(pop)
#define _DYN_ARR_MAYBE_SHRINK _DYN_ARR_F1SP(check_shrink)
#define DYN_ARR_DESTROY _DYN_ARR_F1S(destroy)
#define DYN_ARR_BSEARCH _DYN_ARR_F1S(bsearch)
#define DYN_ARR_BSEARCH_CB _DYN_ARR_F1S(bsearch_cb)
#define DYN_ARR_RAW_BSEARCH _DYN_ARR_F1S(raw_bsearch)
#define DYN_ARR_RAW_BSEARCH_CB _DYN_ARR_F1S(raw_bsearch_cb)
#define DYN_ARR_ADD_ALL _DYN_ARR_F1S(add_all)
#define DYN_ARR_REMOVE_SOME _DYN_ARR_F1S(remove_some)



#ifndef DYN_ARR_CG_STATIC
#define DYN_ARR_CG_STATIC
#endif // DYN_ARR_CG_STATIC

#ifndef DYN_ARR_CG_SHRINK_THRESHOLD_NUMERATOR 
#define DYN_ARR_CG_SHRINK_THRESHOLD_NUMERATOR 3
#endif // DYN_ARR_CG_SHRINK_THRESHOLD_NUMERATOR 
#ifndef DYN_ARR_CG_SHRINK_THRESHOLD_DENOMINATOR 
#define DYN_ARR_CG_SHRINK_THRESHOLD_DENOMINATOR 4
#endif // DYN_ARR_CG_SHRINK_THRESHOLD_DENOMINATOR 

#define PFX DYN_ARR_CG_STATIC

#ifndef DYN_ARR_H_
#define DYN_ARR_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * macro for getting pointer to the last element of the array.
 */
#define DYN_ARR_LAST(dyn_arr) (&(dyn_arr)->arr[(dyn_arr)->count - 1]) 

/**
 * name: dyn_arr_$type_t
 */
typedef struct {
	DYN_ARR_CG_TYPE* arr;
	size_t capacity;
	size_t count;
	unsigned char growth_factor_denominator, growth_factor_numerator;
} DYN_ARR_T;

/**
 * name: dyn_arr_$type_create
 * returns 0 on success, -1 if it could not allocate the array.
 */
PFX int DYN_ARR_CREATE(size_t initial_capacity, 
		unsigned char growth_factor_numerator, 
		unsigned char growth_factor_denominator,
		DYN_ARR_T* dyn_arr);

/**
 * name: dyn_arr_$type_insert
 * returns 0 on success, -1 if it needed to grow the array and could not 
 * reallocate the array. In this case the operation had no effect.
 */
PFX int DYN_ARR_INSERT(DYN_ARR_T* dyn_arr, DYN_ARR_CG_TYPE element, 
		size_t index);

/**
 * name: dyn_arr_$type_insert_some
 * returns 0 on success, -1 if it needed to grow the array and could not 
 * reallocate the array. In this case the operation had no effect.
 */
PFX int DYN_ARR_INSERT_SOME(DYN_ARR_T* dyn_arr, const DYN_ARR_CG_TYPE* arr,
		size_t n, size_t index);

/**
 * name: dyn_arr_$type_add
 * returns 0 on success, -1 if it needed to grow the array and could not 
 * reallocate the array. In this case the operation had no effect.
 */
PFX int DYN_ARR_ADD(DYN_ARR_T* dyn_arr, DYN_ARR_CG_TYPE element);

/**
 * name: dyn_arr_$type_add_all
 * Adds the first n elements from arr to dyn_arr.
 * returns 0 on success, -1 if it needed to grow the array and could not 
 * reallocate the array. In this case the operation had no effect.
 */
PFX int DYN_ARR_ADD_ALL(DYN_ARR_T* dyn_arr, const DYN_ARR_CG_TYPE* arr, 
		size_t n);

/**
 * name: dyn_arr_$type_ensure_capacity
 * Ensures that dyn_arr has at least cap capcity. Continues the exponential
 * growth pattern. Does not shrink the array.
 * returns 0 on success, -1 if it needed to grow the array and could not 
 * reallocate the array. In this case the operation had no effect.
 */
PFX int DYN_ARR_ENSURE_CAPACITY(DYN_ARR_T* dyn_arr, size_t cap);

/**
 * name: dyn_arr_$type_remove
 * returns 0 on success, -1 if it tried to shrink the array and could not 
 * reallocate the array. In this case the operation had no effect.
 */
PFX int DYN_ARR_REMOVE(DYN_ARR_T* dyn_arr, size_t index);

/**
 * name: dyn_arr_$type_pop
 * returns 0 on success, -1 if it tried to shrink the array and could not 
 * reallocate the array. In this case the operation had no effect.
 */
PFX int DYN_ARR_POP(DYN_ARR_T* dyn_arr);

/**
 * name: dyn_arr_$type_pop_some
 * returns 0 on success, -1 if it tried to shrink the array and could not 
 * reallocate the array. In this case the operation had no effect. The functions
 * is equivalent to calling dyn_arr_$type_pop n times. Even if n is 0, it tries
 * to shrink the dyn_arr.
 */
PFX int DYN_ARR_POP_SOME (DYN_ARR_T* dyn_arr, size_t n);

/**
 * name: dyn_arr_$type_remove
 * returns 0 on success, -1 if it tried to shrink the array and could not 
 * reallocate the array. In this case the operation had no effect. The function
 * is equivalent to calling dyn_arr_$type_remove end_i - start_i times with
 * index = start_i.
 * @param start_i begin of range to delete (inclusive)
 * @param end_i end of range to delete (exclusive)
 */
PFX int DYN_ARR_REMOVE_SOME(DYN_ARR_T* dyn_arr, size_t start_i, size_t end_i);

/**
 * name: dyn_arr_$type_destroy
 * frees all resources associated with dyn_arr.
 */
PFX void DYN_ARR_DESTROY(DYN_ARR_T* dyn_arr);

#ifdef DYN_ARR_CG_SIMPLE_COMPARISON
/**
 * name: dyn_arr_$type_bsearch
 * Requires the array to be sorted. 
 * Returns the index of a greatest element smaller than or equal to element. If
 * no such element exist, i.e., the first element is already greater than the
 * supplied element, then 0 is returned as well.
 * It uses <, = and > for comparisons. If dyn_arr is empty, 0 is returned.
 */
PFX size_t DYN_ARR_BSEARCH(const DYN_ARR_T* dyn_arr, DYN_ARR_CG_TYPE element);

/**
 * name: dyn_arr_$type_raw_bsearch
 * n is the size of arr. 
 * Like the non-raw version, but you directly supply the array
 */
PFX size_t DYN_ARR_RAW_BSEARCH(const DYN_ARR_CG_TYPE* arr, size_t n, DYN_ARR_CG_TYPE element);
#endif // DYN_ARR_CG_SIMPLE_COMPARISON

/**
 * name: dyn_arr_$type_bsearch_cb
 * Like dyn_arr_$type_besearch, but callback is used for comparison. callback is
 * called with cb_arg and another element other from dyn_arr. It should return 
 * <0 when cb_arg comes before other, 0 when they are equal with this key, and
 * >0 when cb_arg comes after other.
 */
PFX size_t DYN_ARR_BSEARCH_CB(const DYN_ARR_T* dyn_arr, const void* cb_arg, 
		int (*callback) (const void* cb_arg, 
			const DYN_ARR_CG_TYPE* other));

/**
 * name: dyn_arr_$type_raw_bsearch_cb
 * n is the size of arr. 
 * Like the non-raw version, but you directly supply the array
 */
PFX size_t DYN_ARR_RAW_BSEARCH_CB(const DYN_ARR_CG_TYPE* arr, size_t n, 
		const void* cb_arg, 
		int (*callback) (const void* cb_arg, 
			const DYN_ARR_CG_TYPE* other));

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DYN_ARR_H_
#ifdef DYN_ARR_IMPLEMENTATION

#define DYN_ARR_TMP (defined DYN_ARR_CG_FREE) + (defined DYN_ARR_CG_REALLOC) \
	+ (defined DYN_ARR_CG_MALLOC)
#if (defined DYN_ARR_CG_FREE) + (defined DYN_ARR_CG_REALLOC) \
	+ (defined DYN_ARR_CG_MALLOC) != 0 \
	&& (defined DYN_ARR_CG_FREE) + (defined DYN_ARR_CG_REALLOC) \
	+ (defined DYN_ARR_CG_MALLOC) != 3
#error "If you define any of the allocation related macros you need to define" \
	" all of them"
#endif // allocator functions
#undef DYN_ARR_TMP

#ifndef DYN_ARR_CG_MALLOC
#include <stdlib.h>
#define DYN_ARR_CG_MALLOC(x) malloc(x)
#endif // DYN_ARR_CG_MALLOC

#ifndef DYN_ARR_CG_FREE
#include <stdlib.h>
#define DYN_ARR_CG_FREE(x) free(x)
#endif // DYN_ARR_CG_FREE

#ifndef DYN_ARR_CG_REALLOC
#include <stdlib.h>
#define DYN_ARR_CG_REALLOC(x, y) realloc((x), (y))
#endif // DYN_ARR_CG_REALLOC

#include <string.h>

static int _DYN_ARR_EXTEND(DYN_ARR_T* dyn_arr);
static int _DYN_ARR_SHRINK(DYN_ARR_T* dyn_arr);

PFX int DYN_ARR_CREATE(size_t initial_capacity, 
		unsigned char growth_factor_numerator, 
		unsigned char growth_factor_denominator,
		DYN_ARR_T* dyn_arr) {
	DYN_ARR_CG_TYPE* arr = (DYN_ARR_CG_TYPE*) DYN_ARR_CG_MALLOC(initial_capacity 
			* sizeof(DYN_ARR_CG_TYPE));
	if (initial_capacity > 0 && arr == NULL)
		return -1; 

	dyn_arr->arr = arr,
	dyn_arr->capacity = initial_capacity;
	dyn_arr->count = 0;
	dyn_arr->growth_factor_denominator = growth_factor_denominator;
	dyn_arr->growth_factor_numerator = growth_factor_numerator;
	return 0;
}

static int _DYN_ARR_CHANGE_SIZE(DYN_ARR_T* dyn_arr, size_t new_capacity) {
	DYN_ARR_CG_TYPE* arr = (DYN_ARR_CG_TYPE*) DYN_ARR_CG_REALLOC(
				dyn_arr->arr, new_capacity * sizeof(DYN_ARR_CG_TYPE));
	if (arr == NULL && new_capacity > 0)
		return -1;

	dyn_arr->capacity = new_capacity;
	dyn_arr->arr = arr;
	return 0;
}

static int _DYN_ARR_EXTEND(DYN_ARR_T* dyn_arr) {
	size_t capacity = dyn_arr->capacity * dyn_arr->growth_factor_numerator 
			/ dyn_arr->growth_factor_denominator;
	if (capacity <= dyn_arr->capacity)
		capacity = dyn_arr->capacity + 1;
	return _DYN_ARR_CHANGE_SIZE(dyn_arr, capacity);
}

static int _DYN_ARR_SHRINK(DYN_ARR_T* dyn_arr) {
	size_t capacity = dyn_arr->capacity * dyn_arr->growth_factor_denominator 
			/ dyn_arr->growth_factor_numerator;
	return _DYN_ARR_CHANGE_SIZE(dyn_arr, capacity);
}

PFX int DYN_ARR_INSERT(DYN_ARR_T* dyn_arr, DYN_ARR_CG_TYPE element,
		size_t index) {
	if (dyn_arr->count == dyn_arr->capacity 
			&& _DYN_ARR_EXTEND(dyn_arr) == -1)
		return -1;
	
	size_t i = dyn_arr->count;
	for (; i > index; --i) {
		dyn_arr->arr[i] = dyn_arr->arr[i - 1];
	}
	dyn_arr->arr[index] = element;
	++dyn_arr->count;
	return 0;
}

PFX int DYN_ARR_INSERT_SOME(DYN_ARR_T* dyn_arr, const DYN_ARR_CG_TYPE* arr, 
		size_t n, size_t index) {
	if (n == 0) {
		return 0;
	}

	if (dyn_arr->count + n >= dyn_arr->capacity 
			&& DYN_ARR_ENSURE_CAPACITY(dyn_arr, dyn_arr->count + n) == -1) {
		return -1;
	}

	if (index > dyn_arr->count) {
		index = dyn_arr->count;
	}

	dyn_arr->count += n;
	for (size_t i = dyn_arr->count - 1; i >= index + n; --i) {
		dyn_arr->arr[i] = dyn_arr->arr[i - n];
	}
	memcpy(dyn_arr->arr + index, arr, n * sizeof(DYN_ARR_CG_TYPE));
	return 0;
}

PFX int DYN_ARR_ADD(DYN_ARR_T* dyn_arr, DYN_ARR_CG_TYPE element) {
	if (dyn_arr->count == dyn_arr->capacity 
			&& _DYN_ARR_EXTEND(dyn_arr) == -1)
		return -1;
	
	dyn_arr->arr[dyn_arr->count] = element;
	++dyn_arr->count;
	return 0;
}

PFX int DYN_ARR_ENSURE_CAPACITY(DYN_ARR_T* dyn_arr, size_t cap) {
	if (cap > dyn_arr->capacity) {
		size_t new_capacity = dyn_arr->capacity;
		while (cap > new_capacity) {
			size_t new_capacity_2 = new_capacity * dyn_arr
				->growth_factor_numerator / dyn_arr->growth_factor_denominator;
			if (new_capacity_2 <= new_capacity)
				++new_capacity;
			else
				new_capacity = new_capacity_2;
		}

		return _DYN_ARR_CHANGE_SIZE(dyn_arr, new_capacity);
	}
	return 0;
}

PFX int DYN_ARR_ADD_ALL(DYN_ARR_T* dyn_arr, const DYN_ARR_CG_TYPE* arr, 
		size_t n) {
	size_t required_capacity = dyn_arr->count + n;
	if (DYN_ARR_ENSURE_CAPACITY(dyn_arr, required_capacity) == -1) {
		return -1;
	}

	memcpy(dyn_arr->arr + dyn_arr->count, arr, n * sizeof(DYN_ARR_CG_TYPE));
	dyn_arr->count += n;
	return 0;
}

PFX int _DYN_ARR_MAYBE_SHRINK(DYN_ARR_T* dyn_arr) {
	size_t next_capacity = dyn_arr->capacity 
		* dyn_arr->growth_factor_denominator / dyn_arr->growth_factor_numerator;
	if (next_capacity * DYN_ARR_CG_SHRINK_THRESHOLD_NUMERATOR 
			/ DYN_ARR_CG_SHRINK_THRESHOLD_DENOMINATOR > dyn_arr->count 
			&& _DYN_ARR_SHRINK(dyn_arr) == -1)
		return -1;

	return 0;
}

PFX int DYN_ARR_REMOVE(DYN_ARR_T* dyn_arr, size_t index) {
	size_t i = index;
	for (; i < dyn_arr->count - 1; ++i) {
		dyn_arr->arr[i] = dyn_arr->arr[i + 1];
	}
	--dyn_arr->count;

	return _DYN_ARR_MAYBE_SHRINK(dyn_arr);
}

PFX int DYN_ARR_POP(DYN_ARR_T* dyn_arr) {
	--dyn_arr->count;

	return _DYN_ARR_MAYBE_SHRINK(dyn_arr);
}

PFX int DYN_ARR_POP_SOME(DYN_ARR_T* dyn_arr, size_t n) {
	if (n > dyn_arr->count)
		n = dyn_arr->count;
	dyn_arr->count -= n;

	// find smallest capacity that's over the threshold
	size_t next_capacity = dyn_arr->capacity;
	size_t capacity;
	do {
		capacity = next_capacity;
		next_capacity = capacity * dyn_arr->growth_factor_denominator 
			/ dyn_arr->growth_factor_numerator;
	} while (next_capacity * DYN_ARR_CG_SHRINK_THRESHOLD_NUMERATOR 
						/ DYN_ARR_CG_SHRINK_THRESHOLD_DENOMINATOR 
						> dyn_arr->count);

	return _DYN_ARR_CHANGE_SIZE(dyn_arr, capacity);
}

PFX int DYN_ARR_REMOVE_SOME(DYN_ARR_T* dyn_arr, size_t start_i, size_t end_i) {
	if (end_i > dyn_arr->count) {
		end_i = dyn_arr->count;
	}
	if (start_i >= end_i) {
		return 0;
	}

	size_t diff = end_i - start_i;
	for (size_t i = start_i; i < dyn_arr->count - diff; ++i) {
		dyn_arr->arr[i] = dyn_arr->arr[i + diff];
	}
	return DYN_ARR_POP_SOME(dyn_arr, diff);
}

#ifdef DYN_ARR_CG_SIMPLE_COMPARISON
PFX size_t DYN_ARR_BSEARCH(const DYN_ARR_T* dyn_arr, DYN_ARR_CG_TYPE element) {
	return DYN_ARR_RAW_BSEARCH(dyn_arr->arr, dyn_arr->count, element);
}

PFX size_t DYN_ARR_RAW_BSEARCH(const DYN_ARR_CG_TYPE* arr, size_t n, 
		DYN_ARR_CG_TYPE element) {
	size_t begin = 0;
	size_t end = n;
	while (end - begin > 1) {
		size_t mid = begin + (end - begin) / 2;
		DYN_ARR_CG_TYPE e = arr[mid];
		if (element < e)
			end = mid;
		else if (element > e)
			begin = mid;
		else
			return mid;
	}
	return begin;
}
#endif // DYN_ARR_CG_SIMPLE_COMPARISON

PFX size_t DYN_ARR_BSEARCH_CB(const DYN_ARR_T* dyn_arr, const void* cb_arg, 
		int (*callback) (const void* cb_arg, 
			const DYN_ARR_CG_TYPE* other)) {
	return DYN_ARR_RAW_BSEARCH_CB(dyn_arr->arr, dyn_arr->count, cb_arg, 
			callback);
}

PFX size_t DYN_ARR_RAW_BSEARCH_CB(const DYN_ARR_CG_TYPE* arr, size_t n, 
		const void* cb_arg, 
		int (*callback) (const void* cb_arg, 
			const DYN_ARR_CG_TYPE* other)) {
	size_t begin = 0;
	size_t end = n;
	while (end - begin > 1) {
		size_t mid = begin + (end - begin) / 2;
		int comparison = callback(cb_arg, &arr[mid]);
		if (comparison < 0)
			end = mid;
		else if (comparison > 0)
			begin = mid;
		else
			return mid;
	}
	return begin;
}


PFX void DYN_ARR_DESTROY(DYN_ARR_T* dyn_arr) {
	DYN_ARR_CG_FREE(dyn_arr->arr);
}

#endif // DYN_ARR_IMPLEMENTATION

#undef DYN_ARR_T
#undef PFX
#undef _cat3
#undef _cat4
#undef _cat3E
#undef _cat4E
#undef _DYN_ARR_F1S
#undef _DYN_ARR_F1SP
#undef DYN_ARR_CREATE
#undef DYN_ARR_ADD
#undef DYN_ARR_INSERT
#undef _DYN_ARR_EXTEND
#undef _DYN_ARR_SHRINK
#undef DYN_ARR_REMOVE
#undef DYN_ARR_POP
#undef DYN_ARR_POP_SOME
#undef _DYN_ARR_MAYBE_SHRINK
#undef DYN_ARR_DESTROY
#undef DYN_ARR_BSEARCH
#undef DYN_ARR_BSEARCH_CB
#undef DYN_ARR_ADD_ALL
#undef _DYN_ARR_CHANGE_SIZE
#undef DYN_ARR_RAW_BSEARCH
#undef DYN_ARR_RAW_BSEARCH_CB
#undef DYN_ARR_REMOVE_SOME
#undef DYN_ARR_ENSURE_CAPACITY
#undef DYN_ARR_INSERT_SOME
