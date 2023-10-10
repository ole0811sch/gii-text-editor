#include "editor_render.h"
#include "editor.h"
#include "fxlib.h"
#include "line_utils.h"

/**
 * Unless stated otherwise, functions only update VRAM, not DD.
 */


static void print_char_xy(unsigned short offs_x, unsigned short offs_y, 
		unsigned char x, unsigned char y, char c, 
		int negative);
static void print_char(unsigned short offs_x, unsigned short offs_y, 
char_point_t point, char c, char negative);
static int print_line(const text_box_t* box, size_t line_i);
static int print_partial_line(const text_box_t* box, line_chi_t line_chi);
static void print_cursor_at(const text_box_t* box, char_point_t point, int mode);
static void clear_below_vline(const text_box_t* box, size_t vline);
static void get_full_area(const text_box_t* box, DISPBOX* area);
static int compare_lines_vline_begin(const void* vline_void, 
		const line_t* other);
static point_t char_point_to_point(const text_box_t* box, char_point_t point);
static char line_chi_to_point(const text_box_t* box, line_chi_t line_chi, 
		point_t* point);
static void fill_linewise_with(const text_box_t* box, line_chi_t begin, 
		line_chi_t end, int point, int actual_begin);
static int in_selection(const text_box_t* box, size_t line, size_t char_i);
static void redraw_background(const text_box_t* box);

/**
 * redraws parts of box specified by box->redraw_areas. Also updates DD.
 */
void redraw_changes(const text_box_t* box) {
	// TODO make work with selections
	const redraw_areas_t* ras = &box->redraw_areas;
	DISPBOX a; 
	// reprint everything because we scrolled
	if (ras->vvlines_begin_changed) {
		get_full_area(box, &a);
		Bdisp_AreaClr_VRAM(&a);
		draw_text_box(box);
		Bdisp_PutDispArea_DD(&a);
		goto print_cursor;
	}

	// erase cursor if necessary
	if (box->interaction_mode == CURSOR && (!box->cursor.visual_mode 
				|| ras->selection_cursor)) {
		print_cursor_at(box, ras->old_cursor, 0);
	}
	// if changes are marked, redraw them
	if (line_chi_greater_than(ras->changes_end, ras->changes_begin)) {
		redraw_background(box);

		// print chars
		print_partial_line(box, ras->changes_begin);
		for (size_t line_i = ras->changes_begin.line + 1;
				line_i < ras->changes_end.line 
				|| (line_i == ras->changes_end.line 
				&& ras->changes_end.char_i > 0); 
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
		a.top = begin.y - 1;
		a.left = box->left_px;
		a.right = box->left_px + CHARW_TO_PX(box->width) - 1;
		a.bottom = box->top_px + CHARH_TO_PX(box->height) - 1;
		Bdisp_PutDispArea_DD(&a);
	}
print_cursor:
	if (box->interaction_mode == CURSOR && (!box->cursor.visual_mode 
				|| line_chi_equals(box->cursor.position, 
					box->cursor.selection_begin))) {
		move_cursor(box, 1);
	}
}

/**
 * first clears the background in the area marked by box->redraw_areas,
 * then prints the parts of selection that intersect with that area
 */
static void redraw_background(const text_box_t* box) {
	const redraw_areas_t* ras = &box->redraw_areas;
	// clear bg
	fill_linewise_with(box, ras->changes_begin, ras->changes_end, 0, 0);
	if (box_is_in_visual_mode(box)) {
		// print background for selection
		const line_chi_t* begin_redr_sel;
		const line_chi_t* end_redr_sel;
		int actual_begin_redr_sel; // whether the begin is the begin of 
								   // the selection
		line_chi_t begin_sel, end_sel;
		line_chi_min_max(&box->cursor.position, 
				&box->cursor.selection_begin, &begin_sel, &end_sel);
		if (line_chi_greater_than(end_sel, ras->changes_begin)) {
			// the intersection isn't empty
			if (line_chi_greater_than(ras->changes_begin, begin_sel)) {
				// we're starting redrawing within the selection
				actual_begin_redr_sel = 0;
				begin_redr_sel = &ras->changes_begin;
			} else {
				actual_begin_redr_sel = 1;
				begin_redr_sel = &begin_sel;
			}
			if (line_chi_greater_than(end_sel, ras->changes_end)) {
				// we're ending redrawing within the selection
				end_redr_sel = &ras->changes_end;
			} else {
				end_redr_sel = &end_sel;
			}
			fill_linewise_with(box, *begin_redr_sel, *end_redr_sel, 1, 
					actual_begin_redr_sel);
		}
	}
}

/**
 * clears screen and then prints all lines in box. Also updates DD.
 */
void print_lines(const text_box_t* box) {
	DISPBOX area;
	get_full_area(box, &area);
	Bdisp_AreaClr_VRAM(&area);
	if (box->interaction_mode == CURSOR && box->cursor.visual_mode) {
		line_chi_t begin;
		line_chi_t end;
		line_chi_min_max(&box->cursor.position, &box->cursor.selection_begin, 
				&begin, &end);
		fill_linewise_with(box, begin, end, 1, 1);
	}
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
		unsigned char y, char c, int negative) {
	char_point_t point = { x, y };
	print_char(offs_x, offs_y, point, c, negative);
}

/**
 * if negative true, the colors will be inversed
 */
static void print_char(unsigned short offs_x, unsigned short offs_y, 
		char_point_t point, char c, char negative) {
	if (c > 127) {
		c = '\0';
	}
	point_t px = char_point_to_point(NULL, point);
	px.x += offs_x;
	px.y += offs_y;
	for (unsigned int y = 0; y < CHAR_HEIGHT; ++y) {
		for (unsigned int x = 0; x < CHAR_WIDTH; ++x) {
			if (font[(unsigned char) c][y][x]) {
				Bdisp_SetPoint_VRAM(px.x + x, px.y + y, 
						!negative);
			}
		}
	}
}

static void get_full_area(const text_box_t* box, DISPBOX* area) {
	DISPBOX temp = {
		box->left_px, 
		box->top_px, 
		box->left_px + CHARW_TO_PX(box->width) - 1, 
		box->top_px + CHARH_TO_PX(box->height) - 1 
	};
	*area = temp;
}



static void print_cursor_at(const text_box_t* box, char_point_t point, 
		int mode) {
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
void move_cursor(const text_box_t* box, int mode) {
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

static int print_line(const text_box_t* box, size_t line_i) {
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
	vvlines_local_end = line->vline_begin + line->count_softbreaks + 1;

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
				- box->vvlines_begin, line->string.arr[char_i],
				in_selection(box, line_chi.line, char_i));
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
 * returns pixel coordinate of top left corner (not including the margins) of 
 * that character coordinate. If box is NULL, (0, 0) is used as top left corner
 * of the text_box.
 */
static point_t char_point_to_point(const text_box_t* box, char_point_t point) {
	point_t ret_val = { 0, 0 };
	ret_val.x += CHARW_TO_PX(point.x) + MARGIN_LEFT;
	ret_val.x += (box ? box->left_px : 0);
	ret_val.y += CHARH_TO_PX(point.y) + MARGIN_TOP;
	ret_val.y += (box ? box->top_px : 0);
	return ret_val;
}

/**
 * returns whether line_chi is currently even visible
 * If line_chi is visible, point is set to the pixel coordinate of top left 
 * corner of that character coordinate.
 */
static char line_chi_to_point(const text_box_t* box, line_chi_t line_chi, 
		point_t* point) {
	char_point_t ch_point;
	if(!line_chi_to_char_point(box, line_chi, &ch_point, 1)) {
		return 0;
	}
	*point = char_point_to_point(box, ch_point);
	return 1;
}

/**
 * Sets or clears the space from begin to end. Also prints on the space defined
 * by the margins, except for the left margin at the start, if
 * respect_margin_left is set to true.
 * @param point 1 = dark, 0 = light
 * @param end exclusive
 */
static void fill_linewise_with(const text_box_t* box, line_chi_t begin, 
		line_chi_t end, int point, int respect_margin_left) {
	if (!line_chi_greater_than(end, begin)) {
		return;
	}

	
	point_t begin_pt; // top left corner of first char
	point_t end_pt; // bottom right corner of last char, inclusive
	char_point_t begin_cpt;
	char_point_t end_cpt; // exclusive
	if (!line_chi_to_char_point(box, begin, &begin_cpt, 1)) {
		begin_cpt.x = 0;
		begin_cpt.y = 0;
	}
	begin_pt = char_point_to_point(box, begin_cpt);
	begin_pt.y -= MARGIN_TOP;
	if (!respect_margin_left) {
		begin_pt.x -= MARGIN_LEFT;
	}

	line_chi_t endi = end; // inclusive end
	if (end.char_i == 0) {
		--endi.line;
		endi.char_i = box->lines.arr[endi.line].string.count;
		// if endi points to an empty line, it will end up point to the
		// non-existing char at index 0
		if (endi.char_i > 0) {
			--endi.char_i;
		}
	} else {
		--endi.char_i;
	}

	if (!line_chi_to_char_point(box, endi, &end_cpt, 1)) {
		end_cpt.x = 0;
		end_cpt.y = box->height;
		end_pt.x = box->left_px + CHARW_TO_PX(box->width) - 1;
		end_pt.y = box->top_px + CHARH_TO_PX(box->height) - 1;
	} else {
		// first get the end_pt (inclusive), then increment end_cpt to make it
		// exclusive
		end_pt = char_point_to_point(box, end_cpt);
		if (end.char_i == 0) {
			// set end_pt to right end of vline
			end_pt.x = box->left_px + CHARW_TO_PX(box->width) - 1;
		} else {
			end_pt.x += CHAR_WIDTH - 1;
		}
		end_pt.y += CHAR_HEIGHT - 1;
		if (end_cpt.x == box->width - 1) {
			end_cpt.x = 0;
			++end_cpt.y;
		} else {
			++end_cpt.x;
		}
	}
	
	for (unsigned char y = begin_cpt.y; (end_cpt.x == 0 && y < end_cpt.y) 
			|| (end_cpt.x != 0 && y <= end_cpt.y); ++y) {
		DISPBOX a;
		if (y == begin_cpt.y) {
			a.top = begin_pt.y;
			a.left = begin_pt.x;
		} else {
			a.top = box->top_px + CHARH_TO_PX(y);
			a.left = box->left_px;
		}
		if (y == end_cpt.y || (end_cpt.x == 0 && y == end_cpt.y - 1)) {
			a.bottom = end_pt.y;
			a.right = end_pt.x;
		} else {
			a.bottom = box->top_px + CHARH_TO_PX(y + 1) - 1;
			a.right = box->left_px + CHARW_TO_PX(box->width) - 1;
		}
		Bdisp_AreaClr_VRAM(&a);
		if (point) {
			Bdisp_AreaReverseVRAM(a.left, a.top, a.right, a.bottom);
		}
	}
}

/**
 * returns whether the line/char pair is in the selection. If box isn't in
 * selection mode (or even in cursor mode), 0 is returned as well.
 */
static int in_selection(const text_box_t* box, size_t line, size_t char_i) {
	if (box->interaction_mode != CURSOR || !box->cursor.visual_mode) {
		return 0;
	}
	line_chi_t begin, end;
	line_chi_min_max(&box->cursor.selection_begin, &box->cursor.position, &begin,
			&end);

	line_chi_t lc = { line, char_i };
	return (line_chi_equals(lc, begin) || line_chi_greater_than(lc, begin))
		&& line_chi_greater_than(end, lc);
}
