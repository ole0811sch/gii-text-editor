package mockup_gui;

import build.mockup_gui.RFC.RFCSpec;
import java.util.Arrays;
import build.mockup_gui.*;

public class Main {
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

	public static void main(String[] argv) {
		javax.swing.SwingUtilities.invokeLater(GUI::createAndShowGUI);
		while (true) {
			try {
				int code = System.in.read();
				if (code == -1)
					return;
				current_code = (byte) code;
				RFCSpec spec = RFC.map.get(code);
				assert spec != null;
				int length;
				if (spec.bytes_args == -1) // variable argument size
					length = System.in.read();
				else
					length = spec.bytes_args;
				if (length == -1)
					return;
				byte[] args = new byte[length];
				for (int i = 0; i < args.length; ++i) {
					int read = System.in.read();
					if (read == -1)
						return;
					args[i] = (byte) read;
				}
				byte[] rets = spec.function.apply(args);
				assert spec.bytes_return == -1 && rets.length > 0
					|| spec.bytes_return != -1 
					&& rets.length == spec.bytes_return;
				for (byte b : rets) {
					System.out.write(b);
				}
				System.out.flush();
			} catch (java.io.IOException e) {
				return;
			}
		}
	}

	// returns the int as a little endian byte array
	private static byte[] intToBytes(int i) {
		return new byte[]{(byte) (i & 0xFF), (byte) ((i & 0xFF << 8) >> 8), 
			(byte) ((i & 0xFF << 16) >> 16), (byte) ((i & 0xFF << 24) >> 24)};
	}

	public static byte[] call_GetKey(byte[] args) {
		int isChar = 1;
		try {
			Thread.sleep(1000);
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

	public static byte[] call_Bdisp_AllClr_DDVRAM(byte[] args) {
		return new byte[0];
	}

	public static byte[] call_PopUpWin(byte[] args) {
		return new byte[0];
	}
}
