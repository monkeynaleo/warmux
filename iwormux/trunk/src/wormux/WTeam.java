package wormux;

import com.nttdocomo.io.ConnectionException;
import com.nttdocomo.ui.*;

public class WTeam implements Drawable {

	int id;
	int active;
	MediaImage skinLeft, skinRight;
	int skinHalfWidth, skinHeight;
	WCharacter[] characters = new WCharacter[4];
	
	private WTeam() { ; }
	
	public WTeam(int id, String skin) {
		this.id = id;
		System.out.println("Loading resource:///"+skin+"_l.gif");
		skinLeft = MediaManager.getImage("resource:///"+skin+"_l.gif");
		System.out.println("Loading resource:///"+skin+"_r.gif");
		skinRight = MediaManager.getImage("resource:///"+skin+"_r.gif");
		System.out.println("Done.");
		try {skinLeft.use();} catch (Exception e) {e.printStackTrace();}
		skinHalfWidth = skinLeft.getWidth() / 2;
		skinHeight = skinLeft.getHeight();
		skinLeft.unuse();
		for (int i=0; i<characters.length; i++) {
			characters[i] = new WCharacter(this);
		}
		active = 0;
		System.out.println("Team "+id+" created.");
	}
	
	public int shiftActiveCharacter() {
		active = (active + 1) % characters.length;
		return active;
	}
	
	public int getActiveCharacter() {
		return active;
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
	}

	public void keyEvent(int key, boolean isPressed) {
		// TODO Auto-generated method stub
		
	}
	
}
