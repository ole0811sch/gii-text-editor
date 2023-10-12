#ifndef EDITOR_H_
#define EDITOR_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "fxlib.h"
#include "keybios.h"
#ifdef __cplusplus
}
#endif

#include "font.h"
#include "util.h"
#include "dyn_arrs.h"
#include "line.h"

#include <stddef.h>

typedef enum editor_code {
	CODE_NONE = -1,
	CODE_RESERVED = -2,
	CODE_UP = -3,
	CODE_DOWN = -4,
	CODE_LEFT = -5,
	CODE_RIGHT = -6,
	CODE_TOGGLE_SELECTION = -7,
	CODE_COPY = -8,
	CODE_PASTE = -9,
	CODE_PAGE_DOWN = -10,
	CODE_PAGE_UP = -11,
} editor_code_t;

/* boundaries (inclusive) of the text box of the editor as pixel coordinates */
#define EDITOR_TOP 0
#define EDITOR_LEFT 0
#define EDITOR_BOTTOM 63
#define EDITOR_RIGHT 127

// both at least 1, in pixels
#define MARGIN_TOP 1
#define MARGIN_LEFT 1

// height and width of one character in pixels
#define CHAR_HEIGHT (sizeof(font[0]) / sizeof(font[0][0]))
#define CHAR_WIDTH (sizeof(font[0][0]))

// returns how many pixels n characters need to be displayed horizontally
#define CHARW_TO_PX(n) (CHAR_WIDTH_OUTER * (n))
#define CHARH_TO_PX(n) (CHAR_HEIGHT_OUTER * (n))

#define CHAR_WIDTH_OUTER (CHAR_WIDTH + MARGIN_LEFT)
#define CHAR_HEIGHT_OUTER (CHAR_HEIGHT + MARGIN_TOP)

#define EDITOR_LINES ((EDITOR_BOTTOM - EDITOR_TOP + 1) \
		/ CHAR_HEIGHT_OUTER)
#define EDITOR_COLUMNS ((EDITOR_RIGHT - EDITOR_LEFT + 1) \
		/ CHAR_WIDTH_OUTER)


typedef struct {
	// index of line (in lines)
	size_t line;
	// index of the char in the line's string
	size_t char_i;
} line_chi_t;

typedef enum { 
	/**
	 * text is editable, you navigate with a cursor 
	 */
	CURSOR,
	/**
	 * text is not editable, you navigate by scrolling
	 */
	SCROLL 
} interaction_mode_t;

/**
 * describes what parts of the box need to be redrawn to realize the last
 * operation. If something needs to be redrawn then this should be indicated by
 * this struct.
 */
typedef struct {
	/**
	 * true iff vvlines_begin changed (i.e. we scrolled)
	 */
	char vvlines_begin_changed;
	/**
	 * vline and column of old cursor which needs to be overwritten if it has
	 * changed.
	 */
	char_point_t old_cursor;
	/**
	 * true iff we were in selection mode yet the cursor was printed
	 */
	char selection_cursor;
	/**
	 * begin of changes (inclusive)
	 * changes_begin and changes_end only describe the changes in line_chi_t
	 * to char_point_t mappings that result from insertions and deletions.
	 */
	line_chi_t changes_begin;
	/**
	 * end of changes (exclusive)
	 */
	line_chi_t changes_end;
} redraw_areas_t;
/**
 * stores state and config of one text box
 */
typedef struct {
	/**
	 * pixel x coordinate of the top left
	 */
	unsigned short left_px;
	/**
	 * pixel y coordinate of the top left
	 */
	unsigned short top_px;
	/**
	 * number of chars that fit in one vline
	 */
	unsigned short width;
	/**
	 *
	 * number of vlines that can displayed at once
	 */
	unsigned short height;
	/**
	 * scroll vs cursor
	 */
	interaction_mode_t interaction_mode;

	/**
	 * lines of the text box, ordered in ascending order
	 */
	dyn_arr_line_t lines;
	/**
	 * first visible vline
	 */
	size_t vvlines_begin;
	/**
	 * description of what needs to be redrawn
	 */
	redraw_areas_t redraw_areas;
	/**
	 * when in scroll mode, this stores state related to the cursor
	 */
	struct cursor_state {
		/**
		 * position of the cursor (the character it is before)
		 */
		line_chi_t position;
		/** 
		 * vertical moves will try to get as close possible to cursor_x_target 
		 * with the cursor's x value if cursor_x_target is greater than the 
		 * previous x value.
		 * Setting the target to 0 means that the current x value is the target.
		 * Successful horizontal moves (including inserts and deletes) set 
		 * cursor_x_target to 0. If a vertical move can't needs to reduce x (due
		 * to the line length), it updates cursor_x_target to the old x value.
		 */
		unsigned char cursor_x_target;
		/**
		 * true iff in visual (selection) mode
		 */
		char visual_mode;
		/**
		 * defines the range of the selection together with position. The
		 * smaller of the two is the begin (index of the first character) and
		 * the other is the end (exclusive). The selection can also be empty
		 * (when they are identical).
		 */
		line_chi_t selection_begin;
		/**
		 * stores whether the text box can be edited
		 */
		char editable;
		struct editable_state {
			/**
			 * if true, the next key stroke will be affect
			 */
			char capitalization_on;
			/**
			 * if true, all key strokes will be affect until it is turned off 
			 * again
			 */
			char capitalization_on_locked;
		} editable_state;
	} cursor;
} text_box_t;

/**
 * initializes text box but doesn't draw it
 */
void initialize_text_box(unsigned short left_px, unsigned short top_px,
		unsigned short width, unsigned short height, 
		interaction_mode_t interaction_mode, char editable, 
		const char* content, text_box_t* box);
/**
 * clears area of the text box and then draws it
 * To begin interacting with it, also call focus_text_box
 */
void draw_text_box(const text_box_t* box);

/**
 * Handles key events until one of escape_keys is pressed. draw_text_box should
 * be called beforehand. Also updates DD.
 * @param escape_keys array of key codes (as returned by GetKey)
 * @param count_escape_keys length of escape_keys
 */
unsigned int focus_text_box(text_box_t* box, unsigned int escape_keys[], 
		unsigned int count_escape_keys);
/**
 * This function tries to write the box's contents into buf. In
 * any case buf will be null terminated.
 * If the text of box (plus '\0') does not fit into buf, only the 
 * first (buf_size - 1) bytes are written, the last byte is '\0'.
 * The function returns how many bytes (excluding '\0') would have written 
 * or have been written when all could be written. 
 */
size_t get_text_box_string(const text_box_t* box, char buf[], size_t buf_size);

/**
 * frees all resources associated with the text box
 */
void destruct_text_box(text_box_t* box);

/**
 * returns whether line_chi is currently even visible
 * If line_chi is visible, point is set to the character coordinate of
 * that character coordinate. If cursor is 1, a char point that is behind the
 * last character of a line, and that line's last vline is completely filled,
 * then the coordinates to first char of the next vline will be used. If cursor
 * is 0, then in this case the y value be the last vline of the line and the x
 * value will be the width of the box, and 0 will be returned.
 */
char line_chi_to_char_point(const text_box_t* box, line_chi_t line_chi, 
		char_point_t* point, int cursor);
/**
 * returns whether the box is in cursor mode and editable. It may also return
 * true when it is in visual mode, as long as the editable flag is set too.
 */
int box_is_editable(const text_box_t* box);
/**
 * returns whether the box is in cursor mode and in visual mode
 */
int box_is_in_visual_mode(const text_box_t* box);
/**
 * end may be anywhere after the last char if you want to get the content up to
 * the end. If begin is not before end, only the NULL terminator will be
 * written. See get_text_box_string.
 */
size_t get_text_box_partial_string(const text_box_t* box, char buf[], 
		size_t buf_size, line_chi_t* begin, line_chi_t* end);
/**
 * lc should not be (0, 0). lc->line should be at most box->lines.count.
 * returns the decremented lc. If lc->char_i is 0, the decremented value will be
 * index of the char after the last char of the previous line. 
 */
line_chi_t line_chi_decrement(const text_box_t* box, const line_chi_t* lc);
char* get_debug_representation_of_box(text_box_t* box);

#endif
