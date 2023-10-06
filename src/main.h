#ifndef MAIN_H_
#define MAIN_H_

#include <setjmp.h>
#ifdef __cplusplus
extern "C"
#endif
int AddIn_main(int isAppli, unsigned short OptionNum);

extern jmp_buf error_jmp;

#endif
