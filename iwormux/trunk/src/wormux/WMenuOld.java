/*package wormux;

import com.nttdocomo.ui.*;

public class WMenuOld implements Drawable{
	
	final static String[] items = {"Play", "Help", "Quit"};
	final static int PLAY = 0;
	final static int HELP = 1;
	final static int QUIT = 2;
	static int halfHeight, halfWidth;
	final static int cellHeight = 15;
	int currentItem;
	boolean fullDraw;
	
	public WMenuOld() {
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
			g.setColor(Graphics.getColorOfName(Graphics.MAROON));
			g.fillRect(0, 0, Wormux.width, Wormux.height);
			
			//Draws background
			MediaImage bg = MediaManager.getImage("resource:///iwormux-back.jpg");
			try {bg.use();} catch (Exception e) {e.printStackTrace();}
			g.drawImage(bg.getImage(), 0, 0);
			bg.unuse();
			
			int y = Wormux.height-10-cellHeight;
			for (int i=items.length-1; i>=0; i--) {
				g.setColor(Graphics.getColorOfName(Graphics.YELLOW));
				g.fillRect(5, y, Wormux.width-10, cellHeight);
				g.setColor(Graphics.getColorOfName(Graphics.BLUE));
				int labelWidth = Font.getDefaultFont().stringWidth(items[i])/2;
				g.drawString(items[i], halfWidth-labelWidth, y+cellHeight-5);
				y -= 15;
			}
		}
		int y = Wormux.height - 10 - items.length*cellHeight;
		for (int i=0; i<items.length; i++) {
			if (i == currentItem) {
				g.setColor(Graphics.getColorOfName(Graphics.WHITE));
			} else {
				g.setColor(Graphics.getColorOfName(Graphics.YELLOW));
			}
			g.drawRect(10, y+1, Wormux.width-20, cellHeight-4);
			y += 15;
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
				Wormux.getGame().newGame();
				break;
			case Display.KEY_SOFT2:
				Wormux.getGame().terminate();
				break;
			case Display.KEY_SELECT:
				switch (currentItem) {
				case PLAY: Wormux.getGame().newGame(); break;
				case HELP: fullDraw=true; Wormux.getGame().showHelp(); break;
				case QUIT: Wormux.getGame().terminate();
				}
			}
		}
    }
	
}
*/