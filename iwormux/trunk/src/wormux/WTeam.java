package wormux;

import com.nttdocomo.ui.*;

public class WTeam implements Drawable {

	int id;
	int active;
	WCharacter[] characters = new WCharacter[4];
	
	private WTeam() { ; }
	
	public WTeam(int id) {
		this.id = id;
		for (int i=0; i<characters.length; i++) {
			characters[i] = new WCharacter();
		}
		active = 0;
	}
	
	public int shiftActiveCharacter() {
		active = (active + 1) % characters.length;
		return active;
	}
	
	public int getActiveCharacter() {
		return active;
	}

	public void draw(Graphics g) {
		for (int i=0; i<characters.length; i++) {
			if (i != active) {
				characters[i].draw(g);
			}
		}
		characters[active].draw(g);
	}

	public void keyEvent(int key, boolean isPressed) {
		// TODO Auto-generated method stub
		
	}
	
}
