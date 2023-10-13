#include "line.h"

#define DYN_ARR_IMPLEMENTATION
#define DYN_ARR_CG_TYPE line_t
#define DYN_ARR_CG_SUFFIX line
#include "dyn_arr.h"
#undef DYN_ARR_CG_SUFFIX
#undef DYN_ARR_CG_TYPE
#undef DYN_ARR_IMPLEMENTATION

void destruct_line(line_t* line) {
	if (line->count_softbreaks > VLINE_INDEX_STATIC_ARR_SIZE) {
		// free vline_index
		dyn_arr_uchar_destroy(line->vline_index.d_arr);
		free(line->vline_index.d_arr);
	}
	dyn_arr_char_destroy(&line->string);
}
