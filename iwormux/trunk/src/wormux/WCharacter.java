package wormux;

import com.nttdocomo.ui.*;

public class WCharacter implements Drawable {

	final static int maxHealth = 100;
	
	WTeam team;
	int health;
	boolean toLeft;
	int x, y;
	
	private WCharacter() { ; }
	
	public WCharacter(WTeam team) {
		this.team = team;
		health = maxHealth;
		toLeft = (Wormux.random(2) == 0);
		x = Wormux.random(WGround.gWidth-10);
		y = Wormux.random(WGround.gHeight-10)+10;
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
		// TODO Auto-generated method stub

	}

}
