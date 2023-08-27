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

#define CODE_UP -3
#define CODE_DOWN -4
#define CODE_LEFT -5
#define CODE_RIGHT -6

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
	 * when in scroll mode, this stores state related to the cursor
	 */
	struct cursor_state {
		/**
		 * position of the cursor
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

void initialize_text_box(unsigned short left_px, unsigned short top_px,
		unsigned short width, unsigned short height, 
		interaction_mode_t interaction_mode, char editable, 
		const char* content, text_box_t* box);
void draw_text_box(text_box_t* box);
unsigned int focus_text_box(text_box_t* box, unsigned int escape_keys[], 
		unsigned int count_escape_keys);
size_t get_text_box_string(const text_box_t* box, char buf[], size_t buf_size);
void destruct_text_box(text_box_t* box);

#endif
