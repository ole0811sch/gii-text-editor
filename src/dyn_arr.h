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
#define _DYN_ARR_EXTEND _DYN_ARR_F1SP(extend)
#define _DYN_ARR_SHRINK _DYN_ARR_F1SP(shrink)
#define DYN_ARR_REMOVE _DYN_ARR_F1S(remove)
#define DYN_ARR_POP _DYN_ARR_F1S(pop)
#define _DYN_ARR_CHECK_SHRINK _DYN_ARR_F1SP(check_shrink)
#define DYN_ARR_DESTROY _DYN_ARR_F1S(destroy)
#define DYN_ARR_BSEARCH _DYN_ARR_F1S(bsearch)
#define DYN_ARR_BSEARCH_CB _DYN_ARR_F1S(bsearch_cb)


#ifndef DYN_ARR_CG_STATIC
#define DYN_ARR_CG_STATIC
#endif // DYN_ARR_CG_STATIC

#define PFX DYN_ARR_CG_STATIC

#ifndef DYN_ARR_H_
#define DYN_ARR_H_

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
PFX int DYN_ARR_INSERT(DYN_ARR_T* dyn_arr, DYN_ARR_CG_TYPE element, size_t index);

/**
 * name: dyn_arr_$type_add
 * returns 0 on success, -1 if it needed to grow the array and could not 
 * reallocate the array. In this case the operation had no effect.
 */
PFX int DYN_ARR_ADD(DYN_ARR_T* dyn_arr, DYN_ARR_CG_TYPE element);

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
 * name: dyn_arr_$type_destroy
 * frees all resources associated with dyn_arr.
 */
PFX void DYN_ARR_DESTROY(DYN_ARR_T* dyn_arr);

#ifdef DYN_ARR_CG_SIMPLE_COMPARISON
/**
 * name: dyn_arr_$type_bsearch
 * returns the index of  a greatest element smaller than or equal to element. 
 * It uses <, = and > for comparisons. If dyn_arr is empty, 0 is returned.
 */
PFX size_t DYN_ARR_BSEARCH(DYN_ARR_T* dyn_arr, DYN_ARR_CG_TYPE element);
#endif // DYN_ARR_CG_SIMPLE_COMPARISON

/**
 * name: dyn_arr_$type_bsearch_cb
 * Like dyn_arr_$type_besearch, but callback is used for comparison. callback is
 * called with cb_arg and another element other from dyn_arr. It should return 
 * <0 when cb_arg comes before other, 0 when they are equal with this key, and
 * >0 when cb_arg comes after other.
 */
PFX size_t DYN_ARR_BSEARCH_CB(DYN_ARR_T* dyn_arr, DYN_ARR_CG_TYPE* cb_arg, 
		signed int (*callback) (const DYN_ARR_CG_TYPE* cb_arg, 
			const DYN_ARR_CG_TYPE* other));

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef DYN_ARR_IMPLEMENTATION

#define DYN_ARR_TMP (defined DYN_ARR_CG_FREE) + (defined DYN_ARR_CG_REALLOC) \
	+ (defined DYN_ARR_CG_MALLOC)
#if DYN_ARR_TMP != 0 && DYN_ARR_TMP != 3
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

static int _DYN_ARR_EXTEND(DYN_ARR_T* dyn_arr) {
	size_t capacity = dyn_arr->capacity * dyn_arr->growth_factor_numerator 
			/ dyn_arr->growth_factor_denominator;
	DYN_ARR_CG_TYPE* arr = (DYN_ARR_CG_TYPE*) DYN_ARR_CG_REALLOC(dyn_arr->arr, capacity 
			* sizeof(DYN_ARR_CG_TYPE));
	if (arr == NULL)
		return -1;

	dyn_arr->capacity = capacity;
	dyn_arr->arr = arr;
	return 0;
}

static int _DYN_ARR_SHRINK(DYN_ARR_T* dyn_arr) {
	size_t capacity = dyn_arr->capacity * dyn_arr->growth_factor_denominator 
			/ dyn_arr->growth_factor_numerator;
	DYN_ARR_CG_TYPE* arr = (DYN_ARR_CG_TYPE*) DYN_ARR_CG_REALLOC(dyn_arr->arr, capacity 
			* sizeof(DYN_ARR_CG_TYPE));
	if (arr == NULL)
		return -1;

	dyn_arr->capacity = capacity;
	dyn_arr->arr = arr;
	return 0;
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

PFX int DYN_ARR_ADD(DYN_ARR_T* dyn_arr, DYN_ARR_CG_TYPE element) {
	if (dyn_arr->count == dyn_arr->capacity 
			&& _DYN_ARR_EXTEND(dyn_arr) == -1)
		return -1;
	
	dyn_arr->arr[dyn_arr->count] = element;
	++dyn_arr->count;
	return 0;
}

PFX int _DYN_ARR_CHECK_SHRINK(DYN_ARR_T* dyn_arr) {
	size_t next_capacity = dyn_arr->capacity 
		* dyn_arr->growth_factor_denominator / dyn_arr->growth_factor_numerator;
	if (next_capacity * 3 / 4 > dyn_arr->count 
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

	return _DYN_ARR_CHECK_SHRINK(dyn_arr);
}

PFX int DYN_ARR_POP(DYN_ARR_T* dyn_arr) {
	--dyn_arr->count;

	return _DYN_ARR_CHECK_SHRINK(dyn_arr);
}

#ifdef DYN_ARR_CG_SIMPLE_COMPARISON
PFX size_t DYN_ARR_BSEARCH(DYN_ARR_T* dyn_arr, DYN_ARR_CG_TYPE element) {
	size_t begin = 0;
	size_t end = dyn_arr->count;
	while (end - begin > 2) {
		size_t mid = begin + (end - begin) / 2;
		DYN_ARR_CG_TYPE e = dyn_arr->arr[mid];
		if (element < e)
			end = mid;
		else if (element > e)
			begin = mid + 1;
		else
			return mid;
	}
	return begin;
}
#endif // DYN_ARR_CG_SIMPLE_COMPARISON

PFX size_t DYN_ARR_BSEARCH_CB(DYN_ARR_T* dyn_arr, DYN_ARR_CG_TYPE* cb_arg, 
		int (*callback) (const DYN_ARR_CG_TYPE* cb_arg, 
			const DYN_ARR_CG_TYPE* other)) {
	size_t begin = 0;
	size_t end = dyn_arr->count;
	while (end - begin > 2) {
		size_t mid = begin + (end - begin) / 2;
		int comparison = callback(cb_arg, &dyn_arr->arr[mid]);
		if (comparison < 0)
			end = mid;
		else if (comparison > 0)
			begin = mid + 1;
		else
			return mid;
	}
	return begin;
}

PFX void DYN_ARR_DESTROY(DYN_ARR_T* dyn_arr) {
	DYN_ARR_CG_FREE(dyn_arr->arr);
}

#endif // DYN_ARR_IMPLEMENTATION
#endif // DYN_ARR_H_

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
#undef _DYN_ARR_CHECK_SHRINK
#undef DYN_ARR_DESTROY
#undef DYN_ARR_BSEARCH
#undef DYN_ARR_BSEARCH_CB
