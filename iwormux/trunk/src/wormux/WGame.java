package wormux;

import com.nttdocomo.ui.*;

public class WGame implements Drawable {

	final static int UP = 2;
	final static int LEFT = 4;
	final static int RIGHT = 6;
	final static int DOWN = 8;
	static int screenX, screenY;
	static int scrollStepWidth, scrollStepHeight;
	boolean fullDraw;
	WGround ground;
	WTeam[] teams = new WTeam[2];
	int activeTeam;
	
	public WGame() {
		fullDraw = true;
		Wormux.getGame().setSoftLabel(Canvas.SOFT_KEY_1, "Help");
		Wormux.getGame().setSoftLabel(Canvas.SOFT_KEY_2, "Stop");
		screenX = 0;
		screenY = 0;
		scrollStepWidth = Wormux.width / 4;
		scrollStepHeight = Wormux.height / 4;
		ground = new WGround();
		for (int i=0; i<teams.length; i++) {
			teams[i] = new WTeam(i);
		}
		activeTeam = 0;
	}
	
	public void draw(Graphics g) {
		if (fullDraw) {
			g.setColor(Graphics.getColorOfName(Graphics.BLUE));
			g.fillRect(0, 0, Wormux.width, Wormux.height);
		}
		ground.draw(g);
		for (int i=0; i<teams.length; i++) {
			if (i != activeTeam) {
				teams[i].draw(g);
			}
		}
		teams[activeTeam].draw(g);
	}

	private void scrollScreen(int direction) {
		switch (direction) {
		case UP:
			if (screenY - scrollStepHeight >= 0) {
				screenY -= scrollStepHeight;
			}
			break;
		case LEFT:
			if (screenX - scrollStepWidth >= 0) {
				screenX -= scrollStepWidth;
			}
			break;
		case RIGHT:
			if (screenX < WGround.gWidth - scrollStepWidth) {
				screenX += scrollStepWidth;
			}
			break;
		case DOWN:
			if (screenY < WGround.gHeight - scrollStepHeight) {
				screenY += scrollStepHeight;
			}
			break;
		}
		//System.out.println("screenX="+screenX+"; screenY="+screenY);
		Wormux.getGame().refresh();
	}
	
	public void keyEvent(int key, boolean isPressed) {
		if (!isPressed) {
			switch (key) {
			case Display.KEY_SOFT1:
				fullDraw = true;
				Wormux.getGame().showControls();
				break;
			case Display.KEY_SOFT2:
				Wormux.getGame().showMenu();
				break;
			case Display.KEY_2:
				scrollScreen(UP);
				break;
			case Display.KEY_4:
				scrollScreen(LEFT);
				break;
			case Display.KEY_6:
				scrollScreen(RIGHT);
				break;
			case Display.KEY_8:
				scrollScreen(DOWN);
				break;
			}
		}
	}

}
