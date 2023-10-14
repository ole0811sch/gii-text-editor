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
static const char* help_str = "\
Help: \n\
\n\
[Nothing to see yet]\n\
1\n\
2\n\
3\n\
4\n\
5\n\
6\n\
7\n\
8\n\
9";

static const char* str2 = "#include 'fxlib.h'\n"
"#include 'includeFX/filebios.h'\n"
"#include 'keybios.h'\n"
"#include 'dispbios.h'\n"
"\n"
"#include 'editor.h'\n"
"#include 'util.h'\n"
"#include '../mockup/rfc.h'\n"
"\n"
"#include <stdio.h>\n"
"#include <sys/types.h>\n"
"#include <unistd.h>\n"
"#include <stdlib.h>\n"
"#include <string.h>\n"
"\n"
"/**\n"
 "* pixels on the screen: 0 = off (light), 1 = on (dark)\n"
 "*/\n"
"char screen[64][128];\n"
"\n"
"typedef struct {\n"
	"unsigned char function_code;\n"
	"int bytes_args;\n"
	"int btes_return;\n"
"} rfc_info_t;\n"
"#define X(function, function_code, bytes_args, bytes_return) const rfc_info_t \\\n"
	"function##_rfc = { function_code, bytes_args, bytes_return };\n"
"RFCS\n"
"#undef X\n"
"\n"
"static void int_to_bytes(unsigned int i, unsigned char bytes[]) {\n"
	"bytes[0] = (unsigned char) (i & 0xFF);\n"
	"bytes[1] = (unsigned char) ((i & 0xFF << 8) >> 8); \n"
	"bytes[2] = (unsigned char) ((i & 0xFF << 16) >> 16);\n"
	"bytes[3] = (unsigned char) ((i & 0xFF << 24) >> 24);\n"
"}\n"
"\n"
"static unsigned int bytes_to_int(unsigned char bytes[]) {\n"
	"return bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24);\n"
"}\n"
"\n"
"static void check_return_code(rfc_info_t info) {\n"
	"int fcode = fgetc(stdin);\n"
	"if (fcode != info.function_code) {\n"
		"fprintf(stderr, 'Invalid return function code (%u, expected %u)', fcode,\n"
				"info.function_code);\n"
		"exit(EXIT_FAILURE);\n"
	"}\n"
"}\n"
"\n"
"static void flush(void) {\n"
	"if (fflush(stdout) == EOF) {\n"
		"perror('fflush');\n"
		"exit(EXIT_FAILURE);\n"
	"}\n"
"}\n"
"\n"
"static void write_bytes(unsigned char out[], unsigned int n) {\n"
	"for (size_t i = 0; i < n; ++i) {\n"
		"if (fputc(out[i], stdout) == EOF) {\n"
			"exit(EXIT_FAILURE);\n"
		"}\n"
	"}\n"
"}\n"
"\n"
"static void write_byte(unsigned char out) {\n"
	"if (fputc(out, stdout) == EOF) {\n"
		"exit(EXIT_FAILURE);\n"
	"}\n"
"}\n"
"\n"
"\n"
"int GetKey(unsigned int *keycode) {\n"
	"rfc_info_t info = GetKey_rfc;\n"
	"write_byte(info.function_code);\n"
	"flush();\n"
	"check_return_code(info);\n"
	"unsigned char rets[8];\n"
	"for (size_t i = 0; i < 8; ++i) {\n"
		"int res = fgetc(stdin);\n"
		"if (res == EOF)\n"
			"exit(EXIT_FAILURE);\n"
		"rets[i] = (unsigned char) res;\n"
	"}\n"
	"*keycode = bytes_to_int(&rets[4]);\n"
	"return (int) bytes_to_int(rets);\n"
"}\n"
"\n"
"/**\n"
 "* returns true iff the point is on screen. Otherwise it logs an error.\n"
 "*/\n"
"static char check_point_on_screen(int x, int y) {\n"
	"if (y >= ARR_LEN(screen) || x >= ARR_LEN(screen[0]) || y < 0 || x < 0) {\n"
		"fprintf(stderr, '(%d, %d) is outside the screen space', x, y);\n"
		"return 0;\n"
	"}\n"
	"return 1;\n"
"}\n"
"\n"
"void Bdisp_SetPoint_DDVRAM(\n"
	"int x, // x coordinate\n"
	"int y, // y coordinate\n"
	"unsigned char point // kind of dot\n"
") {\n"
	"if (!check_point_on_screen(x, y)) return;\n"
	"screen[y][x] = point;\n"
	"Bdisp_SetPoint_DD(x, y, point);\n"
"}\n"
"\n"
"\n"
"void locate(\n"
	"int x, // x position\n"
	"int y // y position\n"
") {\n"
	"return;\n"
"}\n"
"\n"
"void Print(\n"
	"const unsigned char *str // pointer to string\n"
") {\n"
	"rfc_info_t info = Print_rfc;\n"
	"write_byte(info.function_code);\n"
	"unsigned int arg_length = (unsigned int) strlen((const char*) str);\n"
	"unsigned char length_bytes[4];\n"
	"int_to_bytes(arg_length, length_bytes);\n"
	"write_bytes(length_bytes, sizeof(length_bytes) / sizeof(length_bytes[0]));\n"
	"write_bytes((unsigned char*) str, arg_length);\n"
	"flush();\n"
	"check_return_code(info);\n"
"}\n"
"\n"
"void Bdisp_AreaClr_DDVRAM(const DISPBOX* pArea) {\n"
	"Bdisp_AreaClr_VRAM(pArea);\n"
	"Bdisp_AreaClr_DD(pArea);\n"
"}\n"
"\n"
"void Bdisp_AllClr_DDVRAM(void) {\n"
	"const DISPBOX pArea = { 0, 0, 127, 63 };\n"
	"Bdisp_AreaClr_DDVRAM(&pArea);\n"
"}\n"
"\n"
"void PopUpWin(\n"
	"int n // size of lines\n"
") {\n"
	"return;\n"
"}\n"
"\n"
"\n"
"void start_gui(const char* jar_path) {\n"
	"int pipe_keys[2];\n"
	"int pipe_screen[2];\n"
	"int pipe_res = pipe(pipe_keys);\n"
	"if (pipe_res == -1) {\n"
		"perror('pipe');\n"
		"exit(EXIT_FAILURE);\n"
	"}\n"
	"pipe_res = pipe(pipe_screen);\n"
	"if (pipe_res == -1) {\n"
		"perror('pipe');\n"
		"exit(EXIT_FAILURE);\n"
	"}\n"
"\n"
	"pid_t res = fork();\n"
	"if (res == 0) { // child\n"
		"if (close(pipe_keys[0]) == -1)\n"
			"perror('close');\n"
		"if (close(pipe_screen[1]) == -1)\n"
			"perror('close');\n"
		"if (dup2(pipe_screen[0], 0) == -1)\n"
			"perror('dup2');\n"
		"if (dup2(pipe_keys[1], 1) == -1)\n"
			"perror('dup2');\n"
		"if (close(pipe_keys[1]) == -1)\n"
			"perror('close');\n"
		"if (close(pipe_screen[0]) == -1)\n"
			"perror('close');\n"
		"char buf_width[20];\n"
		"char buf_height[20];\n"
		"if (sprintf(buf_width, '%u', RIGHT - LEFT + 1) \n"
				">= sizeof(buf_width) / sizeof(buf_width[0])\n"
				"|| sprintf(buf_height, '%u', BOTTOM - TOP + 1) \n"
				">= sizeof(buf_height) / sizeof(buf_height[0])) {\n"
			"fprintf(stderr, 'sprintf in start_gui');\n"
			"exit(EXIT_FAILURE);\n"
		"}\n"
		"execlp('java', 'java', '-jar', '-ea', jar_path, buf_width, buf_height,\n"
				"NULL);\n"
		"perror('execlp');\n"
		"exit(EXIT_FAILURE);\n"
	"} else if (res != -1) { // parent\n"
		"if (close(pipe_keys[1]) == -1)\n"
			"perror('close');\n"
		"if (close(pipe_screen[0]) == -1)\n"
			"perror('close');\n"
		"if (dup2(pipe_keys[0], 0) == -1)\n"
			"perror('dup2');\n"
		"if (dup2(pipe_screen[1], 1) == -1)\n"
			"perror('dup2');\n"
		"if (close(pipe_keys[0]) == -1)\n"
			"perror('close');\n"
		"if (close(pipe_screen[1]) == -1)\n"
			"perror('close');\n"
		"// send handshake\n"
		"for (size_t i = 0; i < HANDSHAKE_LENGTH; ++i) {\n"
			"if (fputc(GETBYTE(i, HANDSHAKE), stdout) == EOF) {\n"
				"exit(EXIT_FAILURE);\n"
			"}\n"
		"}\n"
		"if (fflush(stdout) == EOF) {\n"
			"perror('fflush');\n"
			"exit(EXIT_FAILURE);\n"
		"}\n"
		"// wait for handshake\n"
		"size_t next_byte; // index of the next byte in the handshake\n"
		"for (next_byte = 0; next_byte < HANDSHAKE_LENGTH;) {\n"
			"int res = fgetc(stdin);\n"
			"if (res == EOF) {\n"
				"exit(EXIT_FAILURE);\n"
			"}\n"
			"if (res == GETBYTE(next_byte, HANDSHAKE)) {\n"
				"++next_byte;\n"
			"}\n"
			"else {\n"
				"next_byte = 0;\n"
			"}\n"
		"}\n"
		"return;\n"
	"} else { // error\n"
		"perror('fork');\n"
		"exit(EXIT_FAILURE);\n"
	"}\n"
"}\n"
"\n"
"int Bfile_OpenFile(const FONTCHARACTER *filename, int mode) {\n"
	"fputs('Function \\'Bfile_OpenFile\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_OpenMainMemory(const unsigned char *name) {\n"
	"fputs('Function \\'Bfile_OpenMainMemory\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_ReadFile(int HANDLE, void *buf, int size, int readpos) {\n"
	"if (!size) return 0;\n"
	"for (int i = 0; i < size - 1; ++i) {\n"
		"((char*) buf)[i] = '?';\n"
	"}\n"
	"((char*) buf)[size - 1] = '\\0';\n"
	"fputs('Function \\'Bfile_ReadFile\\' isn't implemented\\n', stderr);\n"
	"return size;\n"
"}\n"
"\n"
"int Bfile_WriteFile(int HANDLE, const void *buf, int size) {\n"
	"if (size == 0) return 0;\n"
"\n"
	"fputc(*(char*) buf, stderr);\n"
	"return 1;\n"
"}\n"
"\n"
"int Bfile_SeekFile(int HANDLE, int pos) {\n"
	"fputs('Function \\'Bfile_SeekFile\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_CloseFile(int HANDLE) {\n"
	"fputs('Function \\'Bfile_CloseFile\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_GetMediaFree(enum DEVICE_TYPE devicetype, int *freebytes) {\n"
	"fputs('Function \\'Bfile_GetMediaFree\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_GetFileSize(int HANDLE) {\n"
	"fputs('Function \\'Bfile_GetFileSize\\' isn't implemented\\n', stderr);\n"
	"return 20;\n"
"}\n"
"\n"
"int Bfile_CreateFile(const FONTCHARACTER *filename, int size) {\n"
	"fputs('Function \\'Bfile_CreateFile\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_CreateDirectory(const FONTCHARACTER *pathname) {\n"
	"fputs('Function \\'Bfile_CreateDirectory\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_CreateMainMemory(const unsigned char *name) {\n"
	"fputs('Function \\'Bfile_CreateMainMemory\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_RenameMainMemory(const unsigned char *oldname, \n"
		"const unsigned char *newname) {\n"
	"fputs('Function \\'Bfile_RenameMainMemory\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_DeleteFile(const FONTCHARACTER *filename) {\n"
	"fputs('Function \\'Bfile_DeleteFile\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_DeleteDirectory(const FONTCHARACTER *pathname) {\n"
	"fputs('Function \\'Bfile_DeleteDirectory\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_DeleteMainMemory(const unsigned char *name) {\n"
	"fputs('Function \\'Bfile_DeleteMainMemory\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_FindFirst(const FONTCHARACTER *pathname, int *FindHandle, \n"
		"FONTCHARACTER *foundfile, FILE_INFO *fileinfo) {\n"
	"fputs('Function \\'Bfile_FindFirst\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_FindNext(int FindHandle, FONTCHARACTER *foundfile, \n"
		"FILE_INFO *fileinfo) {\n"
	"fputs('Function \\'Bfile_FindNext\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"int Bfile_FindClose(int FindHandle) {\n"
	"fputs('Function \\'Bfile_FindClose\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"void Bdisp_AllClr_DD(void) {\n"
	"DISPBOX a;\n"
	"a.top = 0;\n"
	"a.left = 0;\n"
	"a.bottom = 63;\n"
	"a.right = 127;\n"
	"Bdisp_AreaClr_DD(&a);\n"
"}\n"
"\n"
"void Bdisp_AllClr_VRAM(void) {\n"
	"DISPBOX a;\n"
	"a.top = 0;\n"
	"a.left = 0;\n"
	"a.bottom = 63;\n"
	"a.right = 127;\n"
	"Bdisp_AreaClr_VRAM(&a);\n"
"}\n"
"\n"
"void Bdisp_AreaClr_DD(const DISPBOX *pArea) {\n"
	"rfc_info_t info = Bdisp_AreaClr_DD_rfc;\n"
	"write_byte(info.function_code);\n"
	"unsigned char buf[16];\n"
	"int_to_bytes(pArea->left, buf);\n"
	"int_to_bytes(pArea->top, buf + 4);\n"
	"int_to_bytes(pArea->right, buf + 8);\n"
	"int_to_bytes(pArea->bottom, buf + 12);\n"
	"write_bytes(buf, 16);\n"
	"flush();\n"
	"check_return_code(info);\n"
"}\n"
"\n"
"void Bdisp_AreaClr_VRAM(const DISPBOX *pArea) {\n"
	"if (!check_point_on_screen(pArea->left, pArea->top) \n"
			"|| !check_point_on_screen(pArea->right, pArea->bottom)) \n"
		"return;\n"
	"for (size_t y = pArea->top; y <= pArea->bottom; ++y) {\n"
		"for (size_t x = pArea->left; x <= pArea->right; ++x) {\n"
			"screen[y][x] = 0;\n"
		"}\n"
	"}\n"
"}\n"
"\n"
"void Bdisp_AreaReverseVRAM(int x1, int y1, int x2, int y2) {\n"
	"fputs('Function \\'Bdisp_AreaReverseVRAM\\' isn't implemented\\n', stderr);\n"
"}\n"
"\n"
"void Bdisp_GetDisp_DD(unsigned char *pData) {\n"
	"fputs('Function \\'Bdisp_GetDisp_DD\\' isn't implemented\\n', stderr);\n"
"}\n"
"\n"
"void Bdisp_GetDisp_VRAM(unsigned char *pData) {\n"
	"fputs('Function \\'Bdisp_GetDisp_VRAM\\' isn't implemented\\n', stderr);\n"
"}\n"
"\n"
"void Bdisp_PutDisp_DD(void) {\n"
	"DISPBOX a;\n"
	"a.top = 0;\n"
	"a.left = 0;\n"
	"a.bottom = 63;\n"
	"a.right = 127;\n"
	"Bdisp_PutDispArea_DD(&a);\n"
"}\n"
"\n"
"void Bdisp_PutDispArea_DD(const DISPBOX *pArea) {\n"
	"if (!check_point_on_screen(pArea->left, pArea->top) \n"
			"|| !check_point_on_screen(pArea->right, pArea->bottom)) \n"
		"return;\n"
"\n"
	"// write function code\n"
	"rfc_info_t info = Bdisp_PutDispArea_DD_rfc;\n"
	"write_byte(info.function_code);\n"
"\n"
	"// write length\n"
	"unsigned int width = pArea->right - pArea->left + 1;\n"
	"unsigned int height = pArea->bottom - pArea->top + 1;\n"
	"unsigned int bufx8 = width * height; // number of bits to transmit for screen area\n"
	"unsigned int buf_len = bufx8 / 8;\n"
	"if (bufx8 % 8) {\n"
		"++buf_len;\n"
	"}\n"
	"unsigned char length_bytes[4];\n"
	"int_to_bytes(buf_len + 16, length_bytes);\n"
	"write_bytes(length_bytes, ARR_LEN(length_bytes));\n"
"\n"
	"// write area\n"
	"unsigned char buf[16];\n"
	"int_to_bytes(pArea->left, buf);\n"
	"int_to_bytes(pArea->top, buf + 4);\n"
	"int_to_bytes(pArea->right, buf + 8);\n"
	"int_to_bytes(pArea->bottom, buf + 12);\n"
	"write_bytes(buf, 16);\n"
"\n"
	"// write content\n"
	"unsigned char buf2[buf_len];\n"
	"size_t bit_i = 0;\n"
	"for (size_t y = pArea->top; y <= pArea->bottom; ++y) {\n"
		"for (size_t x = pArea->left; x <= pArea->right; ++bit_i, ++x) {\n"
			"if (screen[y][x]) {\n"
				"buf2[bit_i / 8] |= 1 << (7 - bit_i % 8);\n"
			"} else {\n"
				"buf2[bit_i / 8] &= ~(1 << (7 - bit_i % 8));\n"
			"}\n"
		"}\n"
	"}\n"
	"write_bytes(buf2, buf_len);\n"
"\n"
	"flush();\n"
	"check_return_code(info);\n"
"}\n"
"\n"
"void Bdisp_SetPoint_DD(int x, int y, unsigned char point) {\n"
	"rfc_info_t info = Bdisp_SetPoint_DD_rfc;\n"
	"unsigned char out[10];\n"
	"out[0] = info.function_code;\n"
	"int_to_bytes((unsigned int) x, &out[1]);\n"
	"int_to_bytes((unsigned int) y, &out[5]);\n"
	"out[9] = point;\n"
	"write_bytes(out, sizeof(out));\n"
	"flush();\n"
	"check_return_code(info);\n"
"}\n"
"\n"
"void Bdisp_SetPoint_VRAM(int x, int y, unsigned char point) {\n"
	"if (!check_point_on_screen(x, y)) return;\n"
	"screen[y][x] = point;\n"
"}\n"
"\n"
"int  Bdisp_GetPoint_VRAM(int x, int y) {\n"
	"fputs('Function \\'Bdisp_GetPoint_VRAM\\' isn't implemented\\n', stderr);\n"
	"return 0;\n"
"}\n"
"\n"
"void Bdisp_WriteGraph_DD(const DISPGRAPH *WriteGraph) {\n"
	"fputs('Function \\'Bdisp_WriteGraph_DD\\' isn't implemented\\n', stderr);\n"
"}\n"
"\n"
"void Bdisp_WriteGraph_VRAM(const DISPGRAPH *WriteGraph) {\n"
	"fputs('Function \\'Bdisp_WriteGraph_VRAM\\' isn't implemented\\n', stderr);\n"
"}\n"
"\n"
"void Bdisp_WriteGraph_DDVRAM(const DISPGRAPH *WriteGraph) {\n"
	"fputs('Function \\'Bdisp_WriteGraph_DDVRAM\\' isn't implemented\\n', stderr);\n"
"}\n"
"\n"
"void Bdisp_ReadArea_DD(const DISPBOX *ReadArea, unsigned char *ReadData) {\n"
	"fputs('Function \\'Bdisp_ReadArea_DD\\' isn't implemented\\n', stderr);\n"
"}\n"
"\n"
"void Bdisp_ReadArea_VRAM(const DISPBOX *ReadArea, unsigned char *ReadData) {\n"
	"fputs('Function \\'Bdisp_ReadArea_VRAM\\' isn't implemented\\n', stderr);\n"
"}\n"
"\n"
"void Bdisp_DrawLineVRAM(int x1, int y1, int x2, int y2) {\n"
	"if(!check_point_on_screen(x1, y1) || !check_point_on_screen(x2, y2))\n"
		"return;\n"
"\n"
	"if (x1 == x2) {\n"
		"int y_min, y_max;\n"
		"if (y1 > y2)\n"
			"y_min = y2, y_max = y1;\n"
		"else\n"
			"y_min = y1, y_max = y2;\n"
		"for (int y = y_min; y <= y_max; ++y) {\n"
			"screen[y][x1] = 1;\n"
		"}\n"
	"} else if (y1 == y2) {\n"
		"int x_min, x_max;\n"
		"if (x1 > x2)\n"
			"x_min = x2, x_max = x1;\n"
		"else\n"
			"x_min = x1, x_max = x2;\n"
		"for (int x = x_min; x <= x_max; ++x) {\n"
			"screen[y1][x] = 1;\n"
		"}\n"
	"} else\n"
		"fputs('Function \\'Bdisp_DrawLineVRAM\\' isn't implemented for '\n"
				"'non-orthogonal lines\\n', stderr);\n"
"}\n"
"\n"
"void Bdisp_ClearLineVRAM(int x1, int y1, int x2, int y2) {\n"
	"fputs('Function \\'Bdisp_ClearLineVRAM\\' isn't implemented\\n', stderr);\n"
"}\n"
"\n"
"\n";
static const char* char_set = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKL"
	"MNOPQRSTUVWXYZ!\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r\x0b\x0c\n";
static const char* str = "\
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

