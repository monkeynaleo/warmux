package wormux;

import com.nttdocomo.ui.*;

public class WMenu implements Drawable{
	
	final static int PLAY = 0;
	final static int CTRL = 1;
	final static int ABOUT = 2;
	final static int QUIT = 3;
	
	final static Cellular[] items = {	new Cellular(PLAY ,"Play", 43),
										new Cellular(CTRL, "Keys", 58),
										new Cellular(ABOUT, "Credits", 73),
										new Cellular(QUIT, "Quit", 103)};
	static int halfHeight, halfWidth;
	final static int cellX = 77;
	final static int cellMiddleRelX = 21;
	final static int cellMiddleRelY = 5; 
	final static int fontHalfHeight = Font.getDefaultFont().getHeight()/2;
	int currentItem;
	boolean fullDraw;
	
	public WMenu() {
		currentItem = 0;
		fullDraw = true;
		halfHeight = Wormux.height/2;
		halfWidth = Wormux.width/2;
		Wormux.getGame().setSoftLabel(Canvas.SOFT_KEY_1, "Play");
		Wormux.getGame().setSoftLabel(Canvas.SOFT_KEY_2, "Quit");
	}
	
	public void draw(Graphics g) {
		if (fullDraw) {
			fullDraw = false;
			currentItem = 0;
			g.setColor(Graphics.getColorOfRGB(30, 103, 10));
			g.fillRect(0, 0, Wormux.width, Wormux.height);
			
			//Draws background
			MediaImage bg = MediaManager.getImage("resource:///iwormux-back.jpg");
			try {bg.use();} catch (Exception e) {e.printStackTrace();}
			g.drawImage(bg.getImage(), 0, 0);
			bg.unuse();
			
			//Draws cellular labels
			for (int i=items.length-1; i>=0; i--) {
				//g.setColor(Graphics.getColorOfName(Graphics.YELLOW));
				//g.fillRect(5, y, Wormux.width-10, cellHeight);
				g.setColor(Graphics.getColorOfName(Graphics.WHITE));
				int labelWidth = Font.getDefaultFont().stringWidth(items[i].label)/2;
				g.drawString(items[i].label, cellX+cellMiddleRelX-labelWidth, items[i].y+cellMiddleRelY+fontHalfHeight);
			}
			
			// Draws version string
			String bString = "version " + Wormux.version;
			int bStrWidth = Font.getDefaultFont().stringWidth(bString)/2;
			g.drawString(bString, halfWidth - bStrWidth, 131);
		}
		for (int i=0; i<items.length; i++) {
			if (i == currentItem) {
				g.setColor(Graphics.getColorOfName(Graphics.YELLOW));
			} else {
				g.setColor(Graphics.getColorOfName(Graphics.BLACK));
			}
			g.drawRect(cellX-2, items[i].y-2, cellMiddleRelX*2+4, cellMiddleRelY*2+3);
			g.drawRect(cellX-1, items[i].y-1, cellMiddleRelX*2+2, cellMiddleRelY*2+1);
		}
	}
	
	public void keyEvent(int key, boolean isPressed) {
		if (!isPressed) {
			switch (key) {
			case Display.KEY_DOWN:
				currentItem = (currentItem + 1) % items.length;
				Wormux.getGame().refresh();
				break;
			case Display.KEY_UP:
				currentItem = (currentItem + items.length-1) % items.length;
				Wormux.getGame().refresh();
				break;
			case Display.KEY_SOFT1:
				Wormux.getGame().playGame();
				break;
			case Display.KEY_SOFT2:
				Wormux.getGame().terminate();
				break;
			case Display.KEY_SELECT:
				switch (items[currentItem].id) {
				case PLAY: Wormux.getGame().playGame(); break;
				case CTRL: fullDraw=true; Wormux.getGame().showControls(); break;
				case ABOUT: fullDraw=true; Wormux.getGame().showAbout(); break;
				case QUIT: Wormux.getGame().terminate();
				}
			}
		}
    }
	
}

class Cellular {
	
	public int id;
	public String label;
	public int y;
	
	private Cellular() { ; }
	
	public Cellular(int id, String label, int y) {
		this.id = id;
		this.label = label;
		this.y = y;
	}
	
}