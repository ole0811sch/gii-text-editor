#include "editor_render.h"
#include "editor.h"
#include "line_utils.h"

/**
 * Unless stated otherwise, functions only update VRAM, not DD.
 */


static void print_char_xy(unsigned short offs_x, unsigned short offs_y, 
		unsigned char x, unsigned char y, char c, 
		char negative);
static void print_char(unsigned short offs_x, unsigned short offs_y, 
char_point_t point, char c, char negative);
static int print_line(text_box_t* box, size_t line_i);
static int print_partial_line(const text_box_t* box, line_chi_t line_chi);
static void print_cursor_at(text_box_t* box, char_point_t point, int mode);
static void clear_below_vline(const text_box_t* box, size_t vline);
static void get_full_area(text_box_t* box, DISPBOX* area);
static int compare_lines_vline_begin(const void* vline_void, 
		const line_t* other);
static point_t char_point_to_point(char_point_t point);
static int line_chi_greater_than(line_chi_t a, line_chi_t b);
static char line_chi_to_point(text_box_t* box, line_chi_t line_chi, 
		point_t* point);

/**
 * clears screen and then prints all lines in box. Also updates DD.
 */
void print_lines(text_box_t* box) {
	DISPBOX area;
	get_full_area(box, &area);
	Bdisp_AreaClr_VRAM(&area);
	size_t first_line = dyn_arr_line_bsearch_cb(&box->lines, 
			(void*) &box->vvlines_begin, &compare_lines_vline_begin);
	for (size_t i = first_line; i < box->lines.count; ++i)
		if (!print_line(box, i)) {
			break;
		}
	Bdisp_PutDispArea_DD(&area);
}

/**
 * vline_void is a pointer to a (size_t) vline.
 * returns sign(vline - other->vline_begin);
 */
static int compare_lines_vline_begin(const void* vline_void, 
		const line_t* other) {
	size_t vline = *(size_t*) vline_void;
	if (vline < other->vline_begin)
		return -1;
	else if (vline > other->vline_begin)
		return 1;
	return 0;
}


static void 
print_char_xy(unsigned short offs_x, unsigned short offs_y, unsigned char x, 
		unsigned char y, char c, char negative) {
	char_point_t point = { x, y };
	print_char(offs_x, offs_y, point, c, negative);
}

/**
 * if negative true, the colors will be inversed
 */
static void print_char(unsigned short offs_x, unsigned short offs_y, 
		char_point_t point, char c, char negative) {
	if (c >= 127 || c < ' ') {
		negative = 0;
		if (c > 127) {
			c = '\0';
		}
	}
	point_t px = char_point_to_point(point);
	px.x += offs_x;
	px.y += offs_y;
	for (unsigned int y = 0; y < CHAR_HEIGHT; ++y) {
		for (unsigned int x = 0; x < CHAR_WIDTH; ++x) {
			Bdisp_SetPoint_VRAM(px.x + x, px.y + y, 
					font[(unsigned char) c][y][x] ^ negative);
		}
	}
}

static void get_full_area(text_box_t* box, DISPBOX* area) {
	DISPBOX temp = {
		box->left_px, 
		box->top_px, 
		box->left_px + CHARW_TO_PX(box->width) - 1, 
		box->top_px + CHARH_TO_PX(box->height) - 1 
	};
	*area = temp;
}

/**
 * redraws parts of box specified by box->redraw_areas. Also updates DD
 */
void redraw_changes(text_box_t* box) {
	redraw_areas_t* ras = &box->redraw_areas;
	DISPBOX a;
	if (ras->vvlines_begin_changed) {
		get_full_area(box, &a);
		Bdisp_AreaClr_VRAM(&a);
		draw_text_box(box);
		Bdisp_PutDispArea_DD(&a);
		return;
	}

	if (box->interaction_mode == CURSOR) {
		print_cursor_at(box, ras->old_cursor, 0);
	}
	if (line_chi_greater_than(ras->changes_end, ras->changes_begin)) {
		print_partial_line(box, ras->changes_begin);
		for (size_t line_i = ras->changes_begin.line + 1;
				line_i < ras->changes_end.line; 
				++line_i) {
			if (!print_line(box, line_i))
				// line_i was (partially) below visible area
				break;
		}
		size_t new_last_vline = DYN_ARR_LAST(&box->lines)->vline_begin 
			+ DYN_ARR_LAST(&box->lines)->count_softbreaks;
		// in case there's still old stuff on the screen
		clear_below_vline(box, new_last_vline);
		point_t begin;
		line_chi_to_point(box, ras->changes_begin, &begin);
		a.top = begin.y;
		a.left = box->left_px;
		a.right = box->left_px + CHARW_TO_PX(box->width) - 1;
		a.bottom = box->top_px + CHARH_TO_PX(box->height) - 1;
		Bdisp_PutDispArea_DD(&a);
	}
	if (box->interaction_mode == CURSOR) {
		move_cursor(box, 1);
	}
}


static void print_cursor_at(text_box_t* box, char_point_t point, int mode) {
	point_t px = {
		box->left_px + CHARW_TO_PX(point.x),
		box->top_px + CHARH_TO_PX(point.y)
	};
	if (px.x > 127 || px.y > 63 - CHAR_HEIGHT_OUTER + 1) {
		return;
	}

	for (int y = 0; y <= (int) CHAR_HEIGHT; ++y)
		Bdisp_SetPoint_VRAM(px.x, px.y + y, mode);
	DISPBOX a;
	a.left = px.x;
	a.right = px.x;
	a.top = px.y;
	a.bottom = px.y + CHAR_HEIGHT;
	Bdisp_PutDispArea_DD(&a);
}

/**
 * mode = 1: print, mode = 0: erase. If box isn't in CURSOR mode, nothing
 * happens.
 */
void move_cursor(text_box_t* box, int mode) {
	if (mode && box->interaction_mode != CURSOR) {
		return;
	}

	char_point_t cpoint;
	size_t vline = line_chi_to_vline(box, box->cursor.position, &cpoint.x, 1);
	cpoint.y = vline - box->vvlines_begin;
	print_cursor_at(box, cpoint, mode);
}

/**
 * clears the screen of box below vline
 */
void clear_below_vline(const text_box_t* box, size_t vline) {
	DISPBOX area;
	area.left = box->left_px;
	area.right = box->left_px + box->width * CHAR_WIDTH_OUTER - 1;
	area.bottom = box->top_px + box->height * CHAR_HEIGHT_OUTER - 1;
	int vlines_to_spare = vline + 1 - box->vvlines_begin;
	if (vlines_to_spare < 0)
		vlines_to_spare = 0;
	area.top = box->top_px + vlines_to_spare * CHAR_HEIGHT_OUTER;
	if (area.bottom < area.top || area.right < area.left)
		return;
	Bdisp_AreaClr_VRAM(&area);
}

static int print_line(text_box_t* box, size_t line_i) {
	line_chi_t line_chi = { line_i, 0 };
	return print_partial_line(box, line_chi);
}

/**
 * prints starting from line_chi
 *
 * Return 1 if none of the vlines that should be printed were below what is
 * currently visible, otherwise returns 0.
 */
static int print_partial_line(const text_box_t* box, line_chi_t line_chi) {
	const dyn_arr_line_t* lines = &box->lines;
	const line_t* line = &lines->arr[line_chi.line];
	// exclusive, end of the visible vlines of this line
	size_t vvlines_local_end;	
	if (line_chi.line == lines->count - 1) { // line_chi.line is the last line 
		vvlines_local_end = line->vline_begin + line->count_softbreaks + 1;
	}
	else // is this necessary ???
		vvlines_local_end = lines->arr[line_chi.line + 1].vline_begin;

	if (vvlines_local_end > box->vvlines_begin + box->height) 
		// line ends outside of visible area
		vvlines_local_end = box->vvlines_begin + box->height;

	// find vline and x value that correspond to line_chi or if these aren't
	// visible, the first that are.
	size_t char_i; // index in the string
	unsigned char x;
	size_t vline = line_chi_to_vline(box, line_chi, &x, 1); 
	if (vline < box->vvlines_begin) {
		if (line->vline_begin + line->count_softbreaks < box->vvlines_begin)
			// even last vline of line isn't visible, nothing to print
			return 0;
		// first vline is before vvlines_begin, last is not
		// => vvlines_begin is a vline of the line
		vline = box->vvlines_begin;
		x = 0;
		// find new starting index
		const size_t* vline_starts = get_vline_starts(line, NULL);
		char_i = vline_starts[vline - line->vline_begin - 1];
	}
	else
		char_i = line_chi.char_i;


	// print chars of each vline 
	while (char_i < line->string.count && vline < vvlines_local_end) {
		if (x >= box->width) { // next vline
			x = 0;
			++vline;
			continue;
		}
		print_char_xy(box->left_px, box->top_px, x, vline 
				- box->vvlines_begin, line->string.arr[char_i], 0);
		++char_i;
		++x;
	}

	if (vline >= vvlines_local_end)
		// line out of screen space
		return 0;

	while (x < box->width) {
		print_char_xy(box->left_px, box->top_px, x, vline - box->vvlines_begin, 
				' ', 0);
		++x;
	}

	return 1;
}

/**
 * returns pixel coordinate of top left corner of that character coordinate.
 */
static point_t char_point_to_point(char_point_t point) {
	point_t ret_val = { 0, 0 };
	ret_val.x += point.x * CHAR_WIDTH_OUTER + MARGIN_LEFT;
	ret_val.y += point.y * CHAR_HEIGHT_OUTER + MARGIN_TOP;
	return ret_val;
}

static int line_chi_greater_than(line_chi_t a, line_chi_t b) {
	return a.line > b.line || (a.line == b.line && a.char_i > b.char_i);
}

/**
 * returns whether line_chi is currently even visible
 * If line_chi is visible, point is set to the pixel coordinate of top left 
 * corner of that character coordinate.
 */
static char line_chi_to_point(text_box_t* box, line_chi_t line_chi, 
		point_t* point) {
	char_point_t ch_point;
	if(!line_chi_to_char_point(box, line_chi, &ch_point)) {
		return 0;
	}
	*point = char_point_to_point(ch_point);
	point->x += box->left_px;
	point->y += box->top_px;
	return 1;
}
