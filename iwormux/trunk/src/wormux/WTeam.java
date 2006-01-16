package wormux;

import com.nttdocomo.io.ConnectionException;
import com.nttdocomo.ui.*;
import com.nttdocomo.util.Timer;
import com.nttdocomo.util.TimerListener;

public class WTeam implements Drawable, TimerListener {

	int id;
	int active;
	MediaImage skinLeft, skinRight;
	int skinHalfWidth, skinHeight;
	WCharacter[] characters = new WCharacter[4];
	boolean canShift;
	Timer arrowTimer;
	int arrowY;
	
	private WTeam() { ; }
	
	public WTeam(int id, String skin) {
		this.id = id;
		skinLeft = MediaManager.getImage("resource:///"+skin+"_l.gif");
		skinRight = MediaManager.getImage("resource:///"+skin+"_r.gif");
		try {skinLeft.use();} catch (Exception e) {e.printStackTrace();}
		skinHalfWidth = skinLeft.getWidth() / 2;
		skinHeight = skinLeft.getHeight();
		skinLeft.unuse();
		for (int i=0; i<characters.length; i++) {
			characters[i] = new WCharacter(this);
		}
		active = 0;
		canShift = false;
		arrowTimer = new Timer();
		arrowTimer.setRepeat(true);
		arrowTimer.setTime(30);
		arrowTimer.setListener(this);
		arrowY = 0;
	}
	
	public WCharacter shiftActiveCharacter() {
		active = (active + 1) % characters.length;
		return characters[active];
	}
	
	public WCharacter getActiveCharacter() {
		return characters[active];
	}
	
	public void initTurn() {
		WCharacter wc = getActiveCharacter();
		WGame.getInstance().centerView(wc.x, wc.y);
		canShift = true;
		arrowTimer.start();
	}
	
	public void endTurn() {
		canShift = false;
		arrowTimer.stop();
	}

	public void draw(Graphics g) {
		try {
			skinLeft.use();
			skinRight.use();
			for (int i=0; i<characters.length; i++) {
				if (i != active) {
					characters[i].draw(g);
				}
			}
			characters[active].draw(g);
			skinLeft.unuse();
			skinRight.unuse();
		} catch (ConnectionException e) {
			e.printStackTrace();
		} catch (UIException e) {
			e.printStackTrace();
		}
		if (WGame.getInstance().activeTeam == id && canShift) {
			int y = (arrowY < 0) ? -arrowY : arrowY;
			try {
				WGame.arrow.use();
				g.drawImage(WGame.arrow.getImage(), characters[active].x - WGame.screenX - skinHalfWidth, characters[active].y - WGame.screenY - skinHeight * 3 + y);
				WGame.arrow.unuse();
			} catch (ConnectionException e) {
				e.printStackTrace();
			} catch (UIException e) {
				e.printStackTrace();
			}
		}
		
	}

	public void keyEvent(int key, boolean isPressed) {
		switch (key) {
		case Display.KEY_POUND:
			if (canShift) {
				WCharacter wc = shiftActiveCharacter();
				WGame.getInstance().centerView(wc.x, wc.y);
			}
			break;
		case Display.KEY_5:
			WCharacter wc = getActiveCharacter();
			WGame.getInstance().centerView(wc.x, wc.y);
			break;
		default:
			canShift = false;
			arrowTimer.stop();
			characters[active].keyEvent(key, isPressed);
		}
	}

	public void timerExpired(Timer arg0) {
		if (arrowY == skinHeight/2) {
			arrowY = -arrowY;
		}
		arrowY++;
		Wormux.getGame().refresh();
	}
	
}
