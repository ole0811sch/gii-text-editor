package mockup_gui;

import build.mockup_gui.RFC.RFCSpec;
import java.util.Arrays;
import java.io.IOException;
import build.mockup_gui.*;

public class Main {
	private static int SCREEN_WIDTH;
	private static int SCREEN_HEIGHT;
	private static byte current_code;
	private static final int[] keycode_seq = new int[]{ KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_EXE_,
		KeyBios.KEY_CTRL_EXE_,
		KeyBios.KEY_CTRL_EXE_,
		KeyBios.KEY_CHAR_SQUARE_,
		KeyBios.KEY_CHAR_H_,
		KeyBios.KEY_CHAR_E_,
		KeyBios.KEY_CHAR_L_,
		KeyBios.KEY_CHAR_L_,
		KeyBios.KEY_CHAR_O_,
		KeyBios.KEY_CHAR_SPACE_,
		KeyBios.KEY_CHAR_W_,
		KeyBios.KEY_CHAR_O_,
		KeyBios.KEY_CHAR_R_,
		KeyBios.KEY_CHAR_L_,
		KeyBios.KEY_CHAR_D_,
		KeyBios.KEY_CTRL_EXE_,
		KeyBios.KEY_CTRL_EXE_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_DOWN_,
		KeyBios.KEY_CTRL_EXE_, };
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
				int code = System.in.read();
				if (code == -1) {
					throw new RuntimeException("EOF");
				}
				current_code = (byte) code;
				RFCSpec spec = RFC.map.get(code);
				assert spec != null : "current_code == " + current_code;
				int length;
				if (spec.bytes_args == -1) // variable argument size
					length = System.in.read();
				else
					length = spec.bytes_args;
				if (length == -1) {
					throw new RuntimeException("EOF");
				}
				byte[] args = new byte[length];
				for (int i = 0; i < args.length; ++i) {
					int read = System.in.read();
					if (read == -1) {
						throw new RuntimeException("EOF");
					}
					args[i] = (byte) read;
				}
				byte[] rets = spec.function.apply(args);
				assert spec.bytes_return == -1 && rets.length > 0
					|| spec.bytes_return != -1 
					&& rets.length == spec.bytes_return + 1;
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
		return bytes[offs] + (bytes[offs + 1] << 8) + (bytes[offs + 2] << 16) 
			+ (bytes[offs + 3] << 24);
	}

	public static byte[] call_GetKey(byte[] args) {
		int isChar = 1;
		try {
			Thread.sleep(500);
		} catch (InterruptedException e) {
		}
		int code;
		if (seq_i < keycode_seq.length)
			code = keycode_seq[seq_i++];
		else
			code = KeyBios.KEY_CHAR_A_;
		byte[] retVal = new byte[9];
		retVal[0] = current_code;
		System.arraycopy(intToBytes(isChar), 0, retVal, 1, 4);
		System.arraycopy(intToBytes(code), 0, retVal, 5, 4);
		String str = "[";
		boolean isFirst = true;
		for (byte b : retVal) {
			if (isFirst)
				isFirst = false;
			else
				str += "][";
			str += b;
		}
		str += "]";
		return retVal;
	}

	public static byte[] call_Bdisp_SetPoint_DDVRAM(byte[] args) {
		int x = bytesToInt(args, 0);
		int y = bytesToInt(args, 4);
		GUI.getScreenPanel().setPx(x, y, args[8] != 0);
		return new byte[]{current_code};
	}

	public static byte[] call_locate(byte[] args) {
		return new byte[]{current_code};
	}

	public static byte[] call_Print(byte[] args) {
		// for (byte b : args)
		return new byte[]{current_code};
	}

	public static byte[] call_Bdisp_AllClr_DDVRAM(byte[] args) {
		GUI.getScreenPanel().clear();
		return new byte[]{current_code};
	}

	public static byte[] call_PopUpWin(byte[] args) {
		return new byte[]{current_code};
	}

	public static int getWidth() {
		return SCREEN_WIDTH;
	}

	public static int getHeight() {
		return SCREEN_HEIGHT;
	}
}
