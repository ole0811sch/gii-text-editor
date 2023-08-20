#ifndef EDITOR_H_
#define EDITOR_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "fxlib.h"
#ifdef __cplusplus
}
#endif

#include <stddef.h>
#include "font.h"
#include "util.h"
#include "dyn_arrs.h"

#define CODE_UP -3
#define CODE_DOWN -4
#define CODE_LEFT -5
#define CODE_RIGHT -6


void initialize_editor(const char* content);

/* boundaries (inclusive) of the text box of the editor as pixel coordinates */
#define EDITOR_TOP 0
#define EDITOR_LEFT 0
#define EDITOR_BOTTOM 63
#define EDITOR_RIGHT 127

// both at least 1
#define MARGIN_TOP 1
#define MARGIN_LEFT 1

#define CHAR_HEIGHT (sizeof(font[0]) / sizeof(font[0][0]))
#define CHAR_WIDTH (sizeof(font[0][0]))

#define CHAR_WIDTH_OUTER (CHAR_WIDTH + MARGIN_LEFT)
#define CHAR_HEIGHT_OUTER (CHAR_HEIGHT + MARGIN_TOP)

#define EDITOR_LINES ((EDITOR_BOTTOM - EDITOR_TOP + 1) \
		/ CHAR_HEIGHT_OUTER)
#define EDITOR_COLUMNS ((EDITOR_RIGHT - EDITOR_LEFT + 1) \
		/ CHAR_WIDTH_OUTER)

#endif
