package mockup_gui;
import javax.swing.*;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.util.function.Consumer;

public class GUI {
	private static ScreenPanel p;
	/**
	 * Create the GUI and show it. For thread safety,
	 * this method should be invoked from the
	 * event-dispatching thread.
	 */
	public static void createAndShowGUI() {
		//Create and set up the window.
		JFrame frame = new JFrame("HelloWorldSwing");
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		p = new ScreenPanel();
		frame.add(p);

		//Display the window.
		frame.pack();
		frame.setVisible(true);
		synchronized (Main.sem)  {
			Main.sem.notify(); 
		}
	}

	public static ScreenPanel getScreenPanel() {
		return p;
	}
}

class ScreenPanel extends JPanel {
	boolean[][] space;
	int pixelSize = 8;
	private Consumer<Graphics> painter;

	public ScreenPanel() {
		space = new boolean[Main.getWidth()][Main.getHeight()];
		setBorder(BorderFactory.createLineBorder(Color.black));
	}

	@Override
	public Dimension getPreferredSize() {
		return new Dimension(pixelSize * Main.getWidth(), 
				pixelSize * Main.getHeight());
	}

	public void setPx(int x, int y, boolean on) {
		synchronized (space) {
			if (space.length <= x || space[x].length <= y)
				return;

			space[x][y] = on;
		}
		repaint(0, pixelSize * x, pixelSize * y, pixelSize, pixelSize);
	}

	public void clear(int left, int top, int right, int bottom) {
		synchronized (space) {
			for (int x = left; x < space.length && x <= right; x++) {
				for (int y = top; y < space[x].length && y <= bottom; y++) {
					space[x][y] = false;
				}
			}
		}
		repaint(0, left * pixelSize, top * pixelSize, pixelSize * (right - left + 1), pixelSize * (bottom - top + 1));
	}

	@Override
	public void paintComponent(Graphics g) {
		super.paintComponent(g);

		synchronized (space) {
			for (int x = 0; x < space.length; x++) {
				for (int y = 0; y < space[x].length; y++) {
					g.setColor(space[x][y] ? Color.BLACK : Color.WHITE);
					g.fillRect(pixelSize * x, pixelSize * y, pixelSize, 
							pixelSize);
				}
			}
		}
	}
}
