#ifndef DYN_ARRS_H_
#define DYN_ARRS_H_

#define DYN_ARR_CG_SIMPLE_COMPARISON
#define DYN_ARR_CG_TYPE char
#define DYN_ARR_CG_SUFFIX char
#include "dyn_arr.h"
#undef DYN_ARR_CG_SUFFIX
#undef DYN_ARR_CG_TYPE
#undef DYN_ARR_CG_SIMPLE_COMPARISON

#undef DYN_ARR_H_
#define DYN_ARR_CG_SIMPLE_COMPARISON
#define DYN_ARR_CG_TYPE size_t
#define DYN_ARR_CG_SUFFIX size
#include "dyn_arr.h"
#undef DYN_ARR_CG_SUFFIX
#undef DYN_ARR_CG_TYPE
#undef DYN_ARR_CG_SIMPLE_COMPARISON

#endif // DYN_ARRS_H_
