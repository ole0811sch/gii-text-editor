#include "keybios.h"

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


int GetKey(unsigned int *keycode) {
	rfc_info_t info = GetKey_rfc;
	if (fputc(info.function_code, stdout) == EOF)
		exit(EXIT_FAILURE);
	if (fflush(stdout) == EOF) {
		perror("fflush");
		exit(EXIT_FAILURE);
	}
	int res = fgetc(stdin);
	if (res == EOF || res != info.function_code)
		exit(EXIT_FAILURE);
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
	return;
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
	if (fputc(info.function_code, stdout) == EOF)
		exit(EXIT_FAILURE);
	unsigned int arg_length = (unsigned int) strlen((const char*) str);
	unsigned char length_bytes[5];
	int_to_bytes(arg_length, length_bytes);
	length_bytes[4] = '\0';
	if (fputs((const char*) length_bytes, stdout) == EOF)
		exit(EXIT_FAILURE);
	if (fputs((const char*) str, stdout) == EOF)
		exit(EXIT_FAILURE);
	if (fflush(stdout) == EOF) {
		perror("fflush");
		exit(EXIT_FAILURE);
	}
}

void Bdisp_AllClr_DDVRAM(void) {
	return;
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
		execlp("java", "java", "-jar", jar_path, NULL);
		// char (*args)[] = { "java", "-jar", jar_path, NULL };
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
		return;
	} else { // error
		perror("fork");
		exit(EXIT_FAILURE);
	}
}
