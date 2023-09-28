#include "includeFX/filebios.h"
#include "keybios.h"
#include "dispbios.h"

#include "editor.h"
#include "util.h"
#include "../mockup/rfc.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	unsigned char function_code;
	int bytes_args;
	int btes_return;
} rfc_info_t;
#define X(function, function_code, bytes_args, bytes_return) const rfc_info_t \
	function##_rfc = { function_code, bytes_args, bytes_return };
RFCS
#undef X

static void int_to_bytes(unsigned int i, unsigned char bytes[]) {
	bytes[0] = (unsigned char) (i & 0xFF);
	bytes[1] = (unsigned char) ((i & 0xFF << 8) >> 8); 
	bytes[2] = (unsigned char) ((i & 0xFF << 16) >> 16);
	bytes[3] = (unsigned char) ((i & 0xFF << 24) >> 24);
}

static unsigned int bytes_to_int(unsigned char bytes[]) {
	return bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24);
}

static void check_return_code(rfc_info_t info) {
	int fcode = fgetc(stdin);
	if (fcode != info.function_code) {
		fprintf(stderr, "Invalid return function code (%u, expected %u)", fcode,
				info.function_code);
		exit(EXIT_FAILURE);
	}
}

static void flush() {
	if (fflush(stdout) == EOF) {
		perror("fflush");
		exit(EXIT_FAILURE);
	}
}

static void write_bytes(unsigned char out[], unsigned int n) {
	for (size_t i = 0; i < n; ++i) {
		if (fputc(out[i], stdout) == EOF) {
			exit(EXIT_FAILURE);
		}
	}
}

static void write_byte(unsigned char out) {
	if (fputc(out, stdout) == EOF) {
		exit(EXIT_FAILURE);
	}
}


int GetKey(unsigned int *keycode) {
	rfc_info_t info = GetKey_rfc;
	write_byte(info.function_code);
	flush();
	check_return_code(info);
	unsigned char rets[8];
	for (size_t i = 0; i < 8; ++i) {
		int res = fgetc(stdin);
		if (res == EOF)
			exit(EXIT_FAILURE);
		rets[i] = (unsigned char) res;
	}
	*keycode = bytes_to_int(&rets[4]);
	return (int) bytes_to_int(rets);
}

void Bdisp_SetPoint_DDVRAM(
	int x, // x coordinate
	int y, // y coordinate
	unsigned char point // kind of dot
) {
	rfc_info_t info = Bdisp_SetPoint_DDVRAM_rfc;
	unsigned char out[10];
	out[0] = info.function_code;
	int_to_bytes((unsigned int) x, &out[1]);
	int_to_bytes((unsigned int) y, &out[5]);
	out[9] = point;
	write_bytes(out, sizeof(out));
	flush();
	check_return_code(info);
}


void locate(
	int x, // x position
	int y // y position
) {
	return;
}

void Print(
	const unsigned char *str // pointer to string
) {
	rfc_info_t info = Print_rfc;
	write_byte(info.function_code);
	unsigned int arg_length = (unsigned int) strlen((const char*) str);
	unsigned char length_bytes[4];
	int_to_bytes(arg_length, length_bytes);
	write_bytes(length_bytes, sizeof(length_bytes) / sizeof(length_bytes[0]));
	write_bytes((unsigned char*) str, arg_length);
	flush();
	check_return_code(info);
}

void Bdisp_AreaClr_DDVRAM(const DISPBOX* pArea) {
	rfc_info_t info = Bdisp_AreaClr_DDVRAM_rfc;
	write_byte(info.function_code);
	unsigned char buf[16];
	int_to_bytes(pArea->left, buf);
	int_to_bytes(pArea->top, buf + 4);
	int_to_bytes(pArea->right, buf + 8);
	int_to_bytes(pArea->bottom, buf + 12);
	write_bytes(buf, 16);
	flush();
	check_return_code(info);
}

void Bdisp_AllClr_DDVRAM(void) {
	const DISPBOX pArea = { 0, 0, 127, 63 };
	Bdisp_AreaClr_DDVRAM(&pArea);
}

void PopUpWin(
	int n // size of lines
) {
	return;
}


void start_gui(const char* jar_path) {
	int pipe_keys[2];
	int pipe_screen[2];
	int pipe_res = pipe(pipe_keys);
	if (pipe_res == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}
	pipe_res = pipe(pipe_screen);
	if (pipe_res == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	pid_t res = fork();
	if (res == 0) { // child
		if (close(pipe_keys[0]) == -1)
			perror("close");
		if (close(pipe_screen[1]) == -1)
			perror("close");
		if (dup2(pipe_screen[0], 0) == -1)
			perror("dup2");
		if (dup2(pipe_keys[1], 1) == -1)
			perror("dup2");
		if (close(pipe_keys[1]) == -1)
			perror("close");
		if (close(pipe_screen[0]) == -1)
			perror("close");
		char buf_width[20];
		char buf_height[20];
		if (sprintf(buf_width, "%u", RIGHT - LEFT + 1) 
				>= sizeof(buf_width) / sizeof(buf_width[0])
				|| sprintf(buf_height, "%u", BOTTOM - TOP + 1) 
				>= sizeof(buf_height) / sizeof(buf_height[0])) {
			fprintf(stderr, "sprintf in start_gui");
			exit(EXIT_FAILURE);
		}
		execlp("java", "java", "-jar", "-ea", jar_path, buf_width, buf_height,
				NULL);
		perror("execlp");
		exit(EXIT_FAILURE);
	} else if (res != -1) { // parent
		if (close(pipe_keys[1]) == -1)
			perror("close");
		if (close(pipe_screen[0]) == -1)
			perror("close");
		if (dup2(pipe_keys[0], 0) == -1)
			perror("dup2");
		if (dup2(pipe_screen[1], 1) == -1)
			perror("dup2");
		if (close(pipe_keys[0]) == -1)
			perror("close");
		if (close(pipe_screen[1]) == -1)
			perror("close");
		// send handshake
		for (size_t i = 0; i < HANDSHAKE_LENGTH; ++i) {
			if (fputc(GETBYTE(i, HANDSHAKE), stdout) == EOF) {
				exit(EXIT_FAILURE);
			}
		}
		if (fflush(stdout) == EOF) {
			perror("fflush");
			exit(EXIT_FAILURE);
		}
		// wait for handshake
		size_t next_byte; // index of the next byte in the handshake
		for (next_byte = 0; next_byte < HANDSHAKE_LENGTH;) {
			int res = fgetc(stdin);
			if (res == EOF) {
				exit(EXIT_FAILURE);
			}
			if (res == GETBYTE(next_byte, HANDSHAKE)) {
				++next_byte;
			}
			else {
				next_byte = 0;
			}
		}
		return;
	} else { // error
		perror("fork");
		exit(EXIT_FAILURE);
	}
}

int Bfile_OpenFile(const FONTCHARACTER *filename, int mode) {
	fputs("Function \"Bfile_OpenFile\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_OpenMainMemory(const unsigned char *name) {
	fputs("Function \"Bfile_OpenMainMemory\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_ReadFile(int HANDLE, void *buf, int size, int readpos) {
	if (!size) return 0;
	for (int i = 0; i < size - 1; ++i) {
		((char*) buf)[i] = '?';
	}
	((char*) buf)[size - 1] = '\0';
	fputs("Function \"Bfile_ReadFile\" isn't implemented\n", stderr);
	return size;
}

int Bfile_WriteFile(int HANDLE, const void *buf, int size) {
	if (size == 0) return 0;

	fputc(*(char*) buf, stderr);
	return 1;
}

int Bfile_SeekFile(int HANDLE, int pos) {
	fputs("Function \"Bfile_SeekFile\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_CloseFile(int HANDLE) {
	fputs("Function \"Bfile_CloseFile\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_GetMediaFree(enum DEVICE_TYPE devicetype, int *freebytes) {
	fputs("Function \"Bfile_GetMediaFree\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_GetFileSize(int HANDLE) {
	fputs("Function \"Bfile_GetFileSize\" isn't implemented\n", stderr);
	return 20;
}

int Bfile_CreateFile(const FONTCHARACTER *filename, int size) {
	fputs("Function \"Bfile_CreateFile\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_CreateDirectory(const FONTCHARACTER *pathname) {
	fputs("Function \"Bfile_CreateDirectory\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_CreateMainMemory(const unsigned char *name) {
	fputs("Function \"Bfile_CreateMainMemory\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_RenameMainMemory(const unsigned char *oldname, 
		const unsigned char *newname) {
	fputs("Function \"Bfile_RenameMainMemory\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_DeleteFile(const FONTCHARACTER *filename) {
	fputs("Function \"Bfile_DeleteFile\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_DeleteDirectory(const FONTCHARACTER *pathname) {
	fputs("Function \"Bfile_DeleteDirectory\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_DeleteMainMemory(const unsigned char *name) {
	fputs("Function \"Bfile_DeleteMainMemory\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_FindFirst(const FONTCHARACTER *pathname, int *FindHandle, 
		FONTCHARACTER *foundfile, FILE_INFO *fileinfo) {
	fputs("Function \"Bfile_FindFirst\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_FindNext(int FindHandle, FONTCHARACTER *foundfile, 
		FILE_INFO *fileinfo) {
	fputs("Function \"Bfile_FindNext\" isn't implemented\n", stderr);
	return 0;
}

int Bfile_FindClose(int FindHandle) {
	fputs("Function \"Bfile_FindClose\" isn't implemented\n", stderr);
	return 0;
}

