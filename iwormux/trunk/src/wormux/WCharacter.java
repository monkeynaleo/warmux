package wormux;

import com.nttdocomo.ui.*;
import com.nttdocomo.util.Timer;
import com.nttdocomo.util.TimerListener;

public class WCharacter implements Drawable, TimerListener {

	final static int maxHealth = 100;
	
	WTeam team;
	int health;
	boolean toLeft;
	int x, y;
	boolean isFalling;
	Timer fallTimer;
	
	private WCharacter() { ; }
	
	public WCharacter(WTeam team) {
		this.team = team;
		health = maxHealth;
		toLeft = (Wormux.random(2) == 0);
		isFalling = false;
		fallTimer = new Timer();
		fallTimer.setRepeat(false);
		fallTimer.setTime(15);
		fallTimer.setListener(this);
		do {
			x = Wormux.random(WGround.gWidth-10);
			y = Wormux.random(WGround.gHeight-10)+10;
		} while ((y = WGame.getInstance().ground.yProjection(x, y)) == -1);
	}
	
	public boolean testFall() {
		if (y > WGround.gWidth || WGame.getInstance().ground.isOn(x, y)) {
			isFalling = false;
		} else {
			isFalling = true;
			y++;
			Wormux.getGame().refresh();
			fallTimer.start();
		}
		return isFalling;
	}
	
	public void draw(Graphics g) {
		MediaImage skin;
		if (toLeft) {
			skin = team.skinLeft;
		} else {
			skin = team.skinRight;
		}
		g.drawImage(skin.getImage(), x - team.skinHalfWidth - WGame.screenX, y - team.skinHeight - WGame.screenY);
	}

	public void keyEvent(int key, boolean isPressed) {
		if (!isPressed) {
			switch (key) {
			case Display.KEY_LEFT:
				if (toLeft) {
					x -= 5;
				} else {
					toLeft = true;
				}
				break;
			case Display.KEY_RIGHT:
				if (!toLeft) {
					x += 5;
				} else {
					toLeft = false;
				}
				break;
			}
			
			if (!isFalling) {
				if (!testFall()) {
					Wormux.getGame().refresh();
				}
			}
			
		}
	}

	public void timerExpired(Timer arg0) {
		testFall();
	}

}
