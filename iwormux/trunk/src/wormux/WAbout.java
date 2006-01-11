package wormux;

import com.nttdocomo.ui.*;
import com.nttdocomo.util.*;

public class WAbout implements Drawable, TimerListener {

	final static int fontHeight = Font.getDefaultFont().getHeight();
	final static LongText[] labels = {	new LongText("(c)2005, Damien Boucard", Label.CENTER),
										new LongText("iWormux is free software under GNU General Public License."),
										new LongText("Source code is available on http://wormux.org/iWormux", Label.RIGHT),
										new LongText("Graphical datas are owned by the Wormux Team."),
										new LongText("Thanks for playing this game!", Label.CENTER)};
	static int halfWidth;
	
	int offset;
	Timer refresher, stopper;
	
	public WAbout() {
		Wormux.getGame().setSoftLabel(Frame.SOFT_KEY_1, null);
		Wormux.getGame().setSoftLabel(Frame.SOFT_KEY_2, "OK");
		for (int i=0; i<labels.length; i++) {
			labels[i].justify();
		}
		halfWidth = Wormux.width / 2;
		offset = Wormux.height;
		
		refresher = new Timer();
		refresher.setListener(this);
		refresher.setTime(100);
		
		stopper = new Timer();
		stopper.setListener(this);
		refresher.setTime(0);
	}

	public void draw(Graphics g) {
		g.clearRect(0, 0, Wormux.width, Wormux.height);
		int cpt = 1;
		for (int i=0; i<labels.length; i++) {
			for (int j=0; j<labels[i].lines.length; j++) {
				if (offset+(cpt-1)*fontHeight > Wormux.height) {
					break;
				} else if (offset+cpt*fontHeight < 0) {
					cpt++;
					continue;
				}
				int x, tWidth;
				switch (labels[i].alignment) {
				case Label.CENTER:
					tWidth = Font.getDefaultFont().stringWidth(labels[i].lines[j])/2;
					x = halfWidth - tWidth;
					break;
				case Label.RIGHT:
					tWidth = Font.getDefaultFont().stringWidth(labels[i].lines[j]);
					x = Wormux.width - tWidth;
					break;
				default:
					x = 0;
				}
				g.drawString(labels[i].lines[j], x, offset+cpt++*fontHeight);
			}
			cpt++;
			if (offset+(cpt-1)*fontHeight > Wormux.height) {
				break;
			}
		}
		if (offset+(cpt-2)*fontHeight < 0) {
			stopper.start();
		} else {
			offset--;
			refresher.start();
		}
	}

	public void keyEvent(int key, boolean isPressed) {
		if (!isPressed) {
			Wormux.getGame().showMenu();
		}
	}

	public void timerExpired(Timer t) {
		if (t == refresher) {
			Wormux.getGame().refresh();
		} else if (t == stopper) {
			stopper.dispose();
			refresher.dispose();
			Wormux.getGame().showMenu();
		}
	}

}

class LongText {
	
	int alignment;
	private String label;
	String[] lines;
	
	
	private LongText() { ; }
	
	public LongText(String label) {
		this(label, Label.LEFT);
	}
	
	public LongText(String label, int alignment) {
		this.alignment = alignment;
		this.label = label;
	}
	
	public String[] justify() {
		String text = label;
		if (text.length()==0 || text.equals(" ")) {
			lines = new String[1];
			lines[0] = "";
			return lines;
		}
		if (text.length()==1) {
			lines = new String[1];
			lines[0] = text;
			return lines;
		}
		// Finds tokens
		String[] tokens = new String[text.length()/2];
		int nbtok = 0;
		int begin = 0;
		while (begin < text.length() && text.charAt(begin) == ' ') {
			begin++;
		}
		if (begin == text.length()) {
			lines = new String[1];
			lines[0] = "";
			return lines;
		}
		for (int end=begin; end<text.length(); end++) {
			if (text.charAt(end) == ' ') {
				if (end > begin) {
					tokens[nbtok++] = text.substring(begin, end);
				}
				begin = end+1;
			}
		}
		if (begin < text.length()) {
			tokens[nbtok++] = text.substring(begin);
		}
		// Makes lines
		int nbl = 0;
		String[] lines = new String[tokens.length];
		StringBuffer line = new StringBuffer(tokens[0]);
		for (int cpt=1; cpt < nbtok; cpt++) {
			//try{
				if (Font.getDefaultFont().stringWidth(line.toString()+" "+tokens[cpt]) <= Wormux.width) {
					line.append(' ').append(tokens[cpt]);
				} else {
					lines[nbl++] = line.toString();
					line = new StringBuffer(tokens[cpt]);
				}
			//} catch (NullPointerException npe) { System.out.println("PROUT"); throw npe; }
		}
		lines[nbl++] = line.toString();
		this.lines = new String[nbl];
		for (int i=0; i<nbl; i++) {
			this.lines[i] = lines[i];
		}
		return lines;
	}
	
}