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
 * void Bdisp_AreaClr_DD(int left, int top, int right, int bottom)
 * void Bdisp_PutDispArea_DD(uint32_t left, uint32_t top, uint32_t right, 
 * 		uint32_t bottom, uint8_t bits[]) // bits is a stream of bytes in which
 * 		each bit represents one point. They are sent line-wise from left to
 * 		right, top to bottom. 
 */
#define RFCS \
	X(GetKey, 0, 0, 8) \
	X(Bdisp_SetPoint_DD, 1, 9, 0) \
	X(locate, 2, 8, 0) \
	X(Print, 3, -1, 0) \
	X(Bdisp_AreaClr_DD, 4, 16, 0) \
	X(PopUpWin, 5, 4, 0) \
	X(Bdisp_PutDispArea_DD, 6, -1, 0)

// X(function, function_code, bytes_args, bytes_return)

#define HANDSHAKE 0xb4d1d345
#define HANDSHAKE_LENGTH 4
#define GETBYTE(i, bytes) ((bytes & (0xFF << 8 * i)) >> 8 * i)
#endif // RFC_H_
