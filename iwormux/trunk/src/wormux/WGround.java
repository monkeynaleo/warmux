package wormux;

import com.nttdocomo.ui.Graphics;


public class WGround implements Drawable {

	static int gWidth, gHeight;
	Stair[] stairs;
	
	boolean fullDraw;
	
	public WGround() {
		gWidth = Wormux.width * 3;
		gHeight = Wormux.height * 2;
		fullDraw = true;
		stairs = new Stair[5];
		for (int i=0; i<stairs.length; i++) {
			stairs[i] = new Stair();
		}
	}
	
	public void draw(Graphics g) {
		if (fullDraw) {
			g.setColor(Graphics.getColorOfName(Graphics.MAROON));
			for (int i=0; i<stairs.length; i++) {
				stairs[i].draw(g);
			}
		}
	}

	public void keyEvent(int key, boolean isPressed) { ; }

	public int yProjection(int x, int y) {
		Stair s;
		for (int i=0; i<stairs.length; i++) {
			s = stairs[i];
			if (x >= s.x && x <= s.x + s.width && y <= s.y) {
				return s.y;
			}
		}
		return -1;
	}

	public boolean isOn(int x, int y) {
		Stair s;
		for (int i=0; i<stairs.length; i++) {
			s = stairs[i];
			if (x >= s.x && x <= s.x + s.width && y == s.y) {
				return true;
			}
		}
		return false;
	}

}

class Stair implements Drawable {

	int x, y, width, height;
	
	public Stair() {
		x = Wormux.random(WGround.gWidth-10);
		y = Wormux.random(WGround.gHeight-10)+10;
		width = Math.min(WGround.gWidth-x, Wormux.random(WGround.gWidth / 4)*2);
		height = 5;
	}
	
	public void draw(Graphics g) {
		g.fillRect(x-WGame.screenX, y-WGame.screenY, width, height);
	}

	public void keyEvent(int key, boolean isPressed) { ; }
	
}