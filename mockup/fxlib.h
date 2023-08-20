#ifndef FX_LIB_H_
#define FX_LIB_H_

#define INIT_ADDIN_APPLICATION(a, b) 0

int GetKey(unsigned int *keycode);

void Bdisp_SetPoint_DDVRAM(
	int x, // x coordinate
	int y, // y coordinate
	unsigned char point // kind of dot
);

void locate(
	int x, // x position
	int y // y position
);

void Print(
	const unsigned char *str // pointer to string
);

void Bdisp_AllClr_DDVRAM(void);

void PopUpWin(
	int n // size of lines
);

#endif // FX_LIB_H_
