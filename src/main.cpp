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
static const char* str = "\
eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\
eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n\
#include <stdio.h>\n\
int main(void) {\n\
    printf(\"Hello World mmm\"); return 0;\n\
}\n\
eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\
eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n\
1\n\
2\n\
3\n\
4\n\
5\n\
6\n\
7\n\
8\n\
9\n\
10 Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras quis feugiat purus. Etiam commodo luctus odio sed imperdiet. Sed vulputate tristique neque, at auctor diam dignissim eu. Ut ac rhoncus neque. Aenean varius luctus lectus. Mauris sit amet felis sagittis, molestie tortor at, pellentesque ipsum. Ut non rhoncus ligula. Maecenas vulputate ullamcorper elit, eget iaculis sem scelerisque id. Proin tincidunt facilisis elit, ut auctor nibh scelerisque iaculis. Nunc pharetra velit id tempor semper. In quis nulla blandit, pulvinar sapien eget, iaculis nunc. Ut nunc magna, pellentesque vitae hendrerit in, lacinia ac urna. Nulla suscipit, quam at commodo viverra, purus purus scelerisque augue, at pellentesque odio nisi quis augue. Nam facilisis neque tortor, eget imperdiet eros pharetra nec.\n\
\n\
Maecenas pellentesque magna in condimentum laoreet. Ut venenatis justo non libero porttitor rhoncus non ut arcu. Aliquam venenatis sed mi id iaculis. Suspendisse vel ligula laoreet, iaculis libero vel, egestas nunc. Fusce eget viverra dolor. Quisque sodales molestie elit, ut condimentum dui consequat in. Pellentesque in sodales ex. Nullam consectetur iaculis lectus. Nam vitae molestie justo. Ut dignissim lorem justo, ac ullamcorper nisi vehicula at. Cras non sollicitudin felis. ";

#ifdef MOCKUP
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

