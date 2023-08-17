/*****************************************************************/
/*                                                               */
/*   CASIO fx-9860G SDK Library                                  */
/*                                                               */
/*   File name : [ProjectName].c                                 */
/*                                                               */
/*   Copyright (c) 2006 CASIO COMPUTER CO., LTD.                 */
/*                                                               */
/*****************************************************************/
extern "C" {
#include "fxlib.h"
}
#include "editor.h"
#include "util.h"
#include <string.h>
#include <setjmp.h>


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

jmp_buf error_jmp;
static const char* str = "\
#include <stdio.h>\n\
int main(void) {\n\
    printf(\"Hello World mmm\"); return 0;\n\
}\n\
abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYYYZ1234567890!@#$%^&*()_+-=]\
[;'\\/.,<>?|\":}{~`";

int AddIn_main(int isAppli, unsigned short OptionNum)
{
	if (setjmp(error_jmp))
		return 0;

	initialize_editor(str);
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

