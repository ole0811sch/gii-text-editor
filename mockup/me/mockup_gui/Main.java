package me.mockup_gui;

import me.build.mockup_gui.RFC.RFCSpec;
import java.util.Arrays;
import java.io.IOException;
import me.build.mockup_gui.*;

public class Main {
	private static int SCREEN_WIDTH;
	private static int SCREEN_HEIGHT;
	private static final int[] keycode_seq = new int[]{ 
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_RIGHT_,
		KeyBios.KEY_CTRL_RIGHT_,
		KeyBios.KEY_CTRL_RIGHT_,
		KeyBios.KEY_CTRL_EXE_,
		KeyBios.KEY_CTRL_LEFT_,
		KeyBios.KEY_CTRL_UP_,
		KeyBios.KEY_CTRL_F3_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_RIGHT_,
		KeyBios.KEY_CTRL_LEFT_,
		KeyBios.KEY_CTRL_RIGHT_,
		KeyBios.KEY_CTRL_LEFT_,
		KeyBios.KEY_CTRL_RIGHT_,
		KeyBios.KEY_CTRL_LEFT_,
		KeyBios.KEY_CTRL_RIGHT_,
		KeyBios.KEY_CTRL_LEFT_,
		KeyBios.KEY_CTRL_RIGHT_,
		KeyBios.KEY_CTRL_LEFT_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CHAR_0_,
		KeyBios.KEY_CTRL_UP_,
		KeyBios.KEY_CTRL_UP_,
		KeyBios.KEY_CTRL_UP_,
		KeyBios.KEY_CTRL_UP_,
		KeyBios.KEY_CTRL_UP_,
	};
	private static int seq_i = 0;
	public static final Object sem = new Object();

	public static void main(String[] argv) {
		if (argv.length != 2) {
			System.err.println("Wrong number of arguments: [1] screen width, "
					+ "[2] screen height");
			return;
		}
		try {
			SCREEN_WIDTH = Integer.parseUnsignedInt(argv[0]);
			SCREEN_HEIGHT = Integer.parseUnsignedInt(argv[1]);
		} catch (NumberFormatException e) {
			throw new RuntimeException(e);
		}

		javax.swing.SwingUtilities.invokeLater(GUI::createAndShowGUI);
		try {
			synchronized (sem) {
				sem.wait();
			}
		} catch (InterruptedException e) {
			throw new RuntimeException(e);
		}

		// send handshake
		for (int i = 0; i < RFC.HANDSHAKE_LENGTH; i++) {
			System.out.write(RFC.getHandshakeByte(i));
		}
		System.out.flush();
		// wait for handshake
		int j = 0;
		int next_byte; // index of the next byte in the handshake
		for (next_byte = 0; next_byte < RFC.HANDSHAKE_LENGTH;) {
			try {
				int res = System.in.read();
				if (res == -1) {
					throw new RuntimeException("EOF");
				}
				if ((byte) res == RFC.getHandshakeByte(next_byte)) {
					next_byte++;
				}
				else {
					next_byte = 0;
				}
			}
			catch (IOException e) {
				throw new RuntimeException(e);
			}
		}

		while (true) {
			try {
				// read function code
				int code = System.in.read();
				if (code == -1) {
					throw new RuntimeException("EOF");
				}
				byte current_code = (byte) code;
				RFCSpec spec = RFC.map.get(code);
				assert spec != null : "current_code == " + current_code;

				// figure out length
				int length;
				if (spec.bytes_args == -1) { // variable argument size
					byte[] length_bytes = new byte[4];
					for (int i = 0; i < length_bytes.length; i++) {
						int length_byte = System.in.read();
						if (length_byte == -1)
							throw new RuntimeException("EOF");
						length_bytes[i] = (byte) length_byte;
					}
					length = bytesToInt(length_bytes, 0);
				}
				else
					length = spec.bytes_args;
				if (length == -1) {
					throw new RuntimeException("EOF");
				}

				// read args
				byte[] args = new byte[length];
				for (int i = 0; i < args.length; ++i) {
					int read = System.in.read();
					if (read == -1) {
						throw new RuntimeException("EOF");
					}
					args[i] = (byte) read;
				}

				// execute function
				byte[] rets = spec.function.apply(args);
				assert spec.bytes_return == -1 && rets.length > 0
					|| spec.bytes_return != -1 
					&& rets.length == spec.bytes_return;

				// write function_code return and return values
				System.out.write(current_code);
				for (byte b : rets) {
					System.out.write(b);
				}
				System.out.flush();
			} catch (java.io.IOException e) {
				throw new RuntimeException(e);
			}
		}
	}

	// returns the int as a little endian byte array
	private static byte[] intToBytes(int i) {
		return new byte[]{(byte) (i & 0xFF), (byte) ((i & 0xFF << 8) >> 8), 
			(byte) ((i & 0xFF << 16) >> 16), (byte) ((i & 0xFF << 24) >> 24)};
	}

	private static int bytesToInt(byte[] bytes, int offs) {
		return (0xFF & bytes[offs]) | (0xFF & bytes[offs + 1]) << 8 
			| (0xFF & bytes[offs + 2]) << 16 
			| (0xFF & bytes[offs + 3]) << 24;
	}

	public static byte[] call_GetKey(byte[] args) {
		int isChar = 1;
		try {
			Thread.sleep(500);
		} catch (InterruptedException e) {
		}
		int code;
		if (seq_i < keycode_seq.length) {
			code = keycode_seq[seq_i++];
		} else {
			code = KeyBios.KEY_CHAR_A_;
		}
		byte[] retVal = new byte[8];
		System.arraycopy(intToBytes(isChar), 0, retVal, 0, 4);
		System.arraycopy(intToBytes(code), 0, retVal, 4, 4);
		return retVal;
	}

	public static byte[] call_Bdisp_SetPoint_DD(byte[] args) {
		int x = bytesToInt(args, 0);
		int y = bytesToInt(args, 4);
		GUI.getScreenPanel().setPx(x, y, args[8] != 0);
		return new byte[0];
	}

	public static byte[] call_locate(byte[] args) {
		return new byte[0];
	}

	public static byte[] call_Print(byte[] args) {
		for (byte b : args)
			System.err.write(b);
		System.err.flush();
		return new byte[0];
	}

	public static byte[] call_Bdisp_AreaClr_DD(byte[] args) {
		int left = bytesToInt(args, 0);
		int top = bytesToInt(args, 4);
		int right = bytesToInt(args, 8);
		int bottom = bytesToInt(args, 12);
		GUI.getScreenPanel().clear(left, top, right, bottom);
		return new byte[0];
	}

	public static byte[] call_Bdisp_PutDispArea_DD(byte[] args) {
		int left = bytesToInt(args, 0);
		int top = bytesToInt(args, 4);
		int right = bytesToInt(args, 8);
		int bottom = bytesToInt(args, 12);
		byte[] px = Arrays.copyOfRange(args, 16, args.length);
		GUI.getScreenPanel().setArea(left, top, right, bottom, px);
		return new byte[0];
	}

	public static byte[] call_PopUpWin(byte[] args) {
		return new byte[0];
	}

	public static int getWidth() {
		return SCREEN_WIDTH;
	}

	public static int getHeight() {
		return SCREEN_HEIGHT;
	}
}
