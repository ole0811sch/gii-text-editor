#ifndef RFC_H_
#define RFC_H_

/**
 * protocol:
 * (Caller)
 * byte 0: function_code
 * byte 1 - byte bytes_args: args
 * if bytes_args == -1: variable size:
 * 	byte 1 - 4: length
 * 	byte 5 - (byte length + 4): args
 *
 * (Callee)
 * byte 0: function_code
 * return value the same way as args
 *
 * Arguments are supplied in the same order as they are declared. Ints are 4
 * byte, little endian. Arrays are transmitted starting with the lowest index.
 * If a function has void return type, no response is sent.
 *
 * Special cases:
 * RFC declarations:
 * int[2] GetKey(void)	// [0]: original return value, [1] key
 */
#define RFCS \
	X(GetKey, 0, 0, 8) \
	X(Bdisp_SetPoint_DDVRAM, 1, 9, 0) \
	X(locate, 2, 8, 0) \
	X(Print, 3, -1, 0) \
	X(Bdisp_AllClr_DDVRAM, 4, 0, 0) \
	X(PopUpWin, 5, 4, 0)

// X(function, function_code, bytes_args, bytes_return)
#endif // RFC_H_