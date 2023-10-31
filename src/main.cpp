/*****************************************************************/
/*                                                               */
/*   CASIO fx-9860G SDK Library                                  */
/*                                                               */
/*   File name : [ProjectName].c                                 */
/*                                                               */
/*   Copyright (c) 2006 CASIO COMPUTER CO., LTD.                 */
/*                                                               */
/*****************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
#include "fxlib.h"
#include "keybios.h"
#ifdef __cplusplus
}
#endif
#include "editor.h"
#include "util.h"
#include "commands.h"
#include "strings.h"

#ifdef MOCKUP
#include "mockup.h"
#endif

#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>


//****************************************************************************
//  AddIn_main (Sample program main function)
//
//  param   :   isAppli   : 1 = This application is launched by MAIN MENU.
//                        : 0 = This application is launched by a strip in eACT application.
//
//              OptionNum : Strip number (0~3)
//                         (This parameter is only used when isAppli parameter is 0.)
//
//  retval  :   1 = No error / 0 = Error
//
//****************************************************************************
//

#ifdef __cplusplus
extern "C" {
#endif

int AddIn_main(int isAppli, unsigned short OptionNum);

jmp_buf error_jmp;

#if defined(MOCKUP) && !defined(TEST_MODE)
int main(int argc, char** argv) {
	if (argc != 2) {
		puts("Please supply the path to the jar");
		exit(EXIT_FAILURE);
	}
	start_gui(argv[1]);
	return AddIn_main(1, 0);
}
#endif // MOCKUP

int AddIn_main(int isAppli, unsigned short OptionNum)
{
	if (setjmp(error_jmp))
		return 0;

	unsigned int escape_keys_main[] = { KEY_CTRL_F1, KEY_CTRL_F2, KEY_CTRL_F6 };
	text_box_t box;
	initialize_text_box(0, 0, EDITOR_COLUMNS, EDITOR_LINES, 
			CURSOR, 1, str2, &box);

#if 0
	size_t req_size = get_text_box_string(&box, NULL, 0) + 1;
	char* restr = (char*) malloc(req_size);
	if (!restr) {
		display_fatal_error(MSG_ENOMEM);
	}
	get_text_box_string(&box, restr, req_size);

	destruct_text_box(&box);
	initialize_text_box(0, 0, EDITOR_COLUMNS, EDITOR_LINES, 
			CURSOR, 1, restr, &box);
#endif

	while (1) {
		draw_text_box(&box);
		unsigned int res = focus_text_box(&box, escape_keys_main, 
				sizeof(escape_keys_main) / sizeof(escape_keys_main[0]));		
		if (res == KEY_CTRL_F1) {
			unsigned int escape_keys_help[1] = { KEY_CTRL_EXIT };
			text_box_t help_box;
			initialize_text_box(0, 0, EDITOR_COLUMNS, EDITOR_LINES, SCROLL, 1, 
					help_str, &help_box);
			draw_text_box(&help_box);
			focus_text_box(&help_box, escape_keys_help, 1);
		}
		else if (res == KEY_CTRL_F2) {
			open_command_line(&box);
		} else if (res == KEY_CTRL_F6) {
			unsigned int escape_keys_help[1] = { KEY_CTRL_EXIT };
			text_box_t debug_box;
			char* dbg_info = get_debug_representation_of_box(&box);
			destruct_text_box(&box);
			if (dbg_info)
				initialize_text_box(0, 0, EDITOR_COLUMNS, EDITOR_LINES, SCROLL,
						1, dbg_info, &debug_box);
			free(dbg_info);
			draw_text_box(&debug_box);
			focus_text_box(&debug_box, escape_keys_help, 1);
			destruct_text_box(&debug_box);
			initialize_text_box(0, 0, EDITOR_COLUMNS, EDITOR_LINES, 
					CURSOR, 1, "Box was destroyed", &box);
		}
	}
	return 1;
}


#ifdef __cplusplus
}
#endif


//****************************************************************************
//**************                                              ****************
//**************                 Notice!                      ****************
//**************                                              ****************
//**************  Please do not change the following source.  ****************
//**************                                              ****************
//****************************************************************************


#pragma section _BR_Size
unsigned long BR_Size;
#pragma section


#pragma section _TOP

//****************************************************************************
//  InitializeSystem
//
//  param   :   isAppli   : 1 = Application / 0 = eActivity
//              OptionNum : Option Number (only eActivity)
//
//  retval  :   1 = No error / 0 = Error
//
//****************************************************************************
int InitializeSystem(int isAppli, unsigned short OptionNum)
{
	return INIT_ADDIN_APPLICATION(isAppli, OptionNum);
}

#pragma section

