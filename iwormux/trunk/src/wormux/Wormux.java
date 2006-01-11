package wormux;

import com.nttdocomo.ui.*;
import java.util.Random;

public class Wormux extends IApplication implements Runnable {

	private static Wormux game;
	private static Random rbox;
	final static String version = "0.1";
	static int width;
	static int height;

	//private Thread t;
	private WCanvas canvas;
	private WControls help;

	Drawable mode;
	
	public Wormux() {
		super();
		Wormux.game = this;
		Wormux.rbox = new Random(System.currentTimeMillis());
	}
	
	public static Wormux getGame() {
		return Wormux.game;
	}
	
	public static int random(int bound) {
		return Math.abs(rbox.nextInt()) % bound;
	}
	
	public void start() {
		canvas = new WCanvas();
		mode = new WMenu();
		Display.setCurrent(canvas);
		
		//t = new Thread(this);
		// t.start()
	}
	
	public void run() {
		// TODO Auto-generated method stub

	}

	public void refresh() {
		canvas.repaint();
	}
	
	public void showControls() {
		help = new WControls();
		Display.setCurrent(help);
	}
	
	public void closeControls() {
		Display.setCurrent(canvas);
		help = null;
		System.gc();
	}

	public void showAbout() {
		mode = new WAbout();
		System.gc();
		refresh();
	}
	
	public void showMenu() {
		mode = new WMenu();
		System.gc();
		refresh();
	}

	public void playGame() {
		mode = new WGame();
		System.gc();
		refresh();
	}
	
	public void setSoftLabel(int key, String label) {
		canvas.setSoftLabel(key, label);
	}
	
	class WCanvas extends Canvas {
		
		//private int x = 20;
		//private int y = 10;
		
		public WCanvas() {
			Wormux.width = this.getWidth();
			Wormux.height = this.getHeight();
		}
		
		public void paint(Graphics g) {
			g.lock();
			Wormux.getGame().mode.draw(g);
			g.unlock(true);
		}
		
		public void processEvent(int type, int param) {
	        if (type == Display.KEY_PRESSED_EVENT || type == Display.KEY_RELEASED_EVENT) {
	            Wormux.getGame().mode.keyEvent(param, type==Display.KEY_PRESSED_EVENT);
	        }
	    }
		
	}

}