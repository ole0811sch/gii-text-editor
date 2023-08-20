#include "keybios.h"
#include <stdio.h>
#include "editor.h"


int GetKey(unsigned int *keycode) {
	while (1) {
		int result = getc(stdin);
		if (ferror(stdin)) {
			*keycode = KEY_CTRL_NOP;
			return 0;
		}
#define EASY_CASES \
		X(0, '0') \
		X(1, '1') \
		X(2, '2') \
		X(3, '3') \
		X(4, '4') \
		X(5, '5') \
		X(6, '6') \
		X(7, '7') \
		X(8, '8') \
		X(9, '9') \
		X(DP, '.') \
		X(PLUS, '+') \
		X(MINUS, '-') \
		X(MULT, '*') \
		X(DIV, '/') \
		X(LPAR, '(') \
		X(RPAR, ')') \
		X(COMMA, '\x2c') \
		X(POW, '^') \
		X(EQUAL, '=') \
		X(LBRCKT, '[') \
		X(RBRCKT, ']') \
		X(LBRACE, '{') \
		X(RBRACE, '}') \
		X(A, 'A') \
		X(B, 'B') \
		X(C, 'C') \
		X(D, 'D') \
		X(E, 'E') \
		X(F, 'F') \
		X(G, 'G') \
		X(H, 'H') \
		X(I, 'I') \
		X(J, 'J') \
		X(K, 'K') \
		X(L, 'L') \
		X(M, 'M') \
		X(N, 'N') \
		X(O, 'O') \
		X(P, 'P') \
		X(Q, 'Q') \
		X(R, 'R') \
		X(S, 'S') \
		X(T, 'T') \
		X(U, 'U') \
		X(V, 'V') \
		X(W, 'W') \
		X(X, 'X') \
		X(Y, 'Y') \
		X(Z, 'Z') \
		X(SPACE, ' ') \
		X(PMINUS, '\\') \
		X(DQUATE, '"')
#define EASY_CASES_CTRL \
		X(XTT, '`') \
		X(EXE, '\n') \
		X(DEL, '\x08')
		switch (result) {
#define X(code, char) case char: *keycode = KEY_CHAR_##code; return 1;
			EASY_CASES
#undef X
#define X(code, char) case char: *keycode = KEY_CTRL_##code; return 0;
			EASY_CASES_CTRL
#undef X
		}
	}
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
	puts((const char*) str);
}

void Bdisp_AllClr_DDVRAM(void) {
	return;
}

void PopUpWin(
	int n // size of lines
) {
	return;
}

