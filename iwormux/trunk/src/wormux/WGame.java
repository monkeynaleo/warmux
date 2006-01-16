package wormux;

import com.nttdocomo.ui.*;
import com.nttdocomo.util.Timer;
import com.nttdocomo.util.TimerListener;

public class WGame implements Drawable, TimerListener {

	private static WGame instance;
	
	final static int UP = 2;
	final static int LEFT = 4;
	final static int RIGHT = 6;
	final static int DOWN = 8;
	static int screenX, screenY;
	static int scrollStepWidth, scrollStepHeight;
	static MediaImage arrow;
	final static String[] skins= {"worm", "hypo"};
	boolean fullDraw;
	WGround ground;
	WTeam[] teams = new WTeam[2];
	int activeTeam;
	Timer endTimer, teamTimer;
	boolean showTeam;
	boolean keyBlocked;
	
	public WGame() {
		arrow = MediaManager.getImage("resource:///arrow.gif");
		instance = this;
		fullDraw = true;
		Wormux.getGame().setSoftLabel(Canvas.SOFT_KEY_1, "Help");
		Wormux.getGame().setSoftLabel(Canvas.SOFT_KEY_2, "Stop");
		screenX = 0;
		screenY = 0;
		scrollStepWidth = Wormux.width / 4;
		scrollStepHeight = Wormux.height / 4;
		ground = new WGround();
		for (int i=0; i<teams.length; i++) {
			teams[i] = new WTeam(i, skins[i]);
		}
		activeTeam = Wormux.random(teams.length);
		endTimer = new Timer();
		endTimer.setRepeat(false);
		endTimer.setTime(2000);
		endTimer.setListener(this);
		teamTimer = new Timer();
		teamTimer.setRepeat(true);
		teamTimer.setTime(200);
		teamTimer.setListener(this);
		showTeam = false;
		keyBlocked = false;
		nextTurn();
	}
	
	public static WGame getInstance() {
		return instance;
	}
	
	public void nextTurn() {
		teams[activeTeam].endTurn();
		keyBlocked = true;
		endTimer.start();
		teamTimer.start();
		
	}
	
	public void newTurn() {
		teamTimer.stop();
		showTeam = false;
		keyBlocked = false;
		activeTeam = (activeTeam+1) % teams.length;
		teams[activeTeam].initTurn();
	}
	
	public void centerView(int x, int y) {
		screenX = x - Wormux.width / 2;
		screenY = y - Wormux.height / 2;
		Wormux.getGame().refresh();
	}

	private void blinkTeam() {
		showTeam = !showTeam;
		Wormux.getGame().refresh();
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
		if (showTeam) {
			String strTeam = "Team " + activeTeam;
			int strTeamW = Font.getDefaultFont().stringWidth(strTeam);
			int strTeamH = Font.getDefaultFont().getHeight();
			int strTeamX = (Wormux.width - strTeamW) / 2;
			int strTeamY = (Wormux.height - strTeamH) / 2;
			g.setColor(Graphics.getColorOfName(Graphics.YELLOW));
			g.fillRect(strTeamX - 2, strTeamY - strTeamH, strTeamW + 4, strTeamH + 4);
			g.setColor(Graphics.getColorOfName(Graphics.BLACK));
			g.drawString(strTeam, strTeamX, strTeamY);
		}
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
			if (screenX < WGround.gWidth - Wormux.width) {
				screenX += scrollStepWidth;
			}
			break;
		case DOWN:
			if (screenY < WGround.gHeight - Wormux.height) {
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
			case Display.KEY_0:
				if (!keyBlocked) {
					nextTurn();
				}
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
			case Display.KEY_RIGHT:
			case Display.KEY_LEFT:
			case Display.KEY_POUND:
				if (keyBlocked) {
					break;
				}
			case Display.KEY_5:
				teams[activeTeam].keyEvent(key, isPressed);
			}
		}
	}

	public void timerExpired(Timer t) {
		if (t == endTimer) {
			newTurn();
		} else if (t == teamTimer) {
			blinkTeam();
		}
	}

}
