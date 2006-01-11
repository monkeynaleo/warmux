package wormux;

import com.nttdocomo.ui.*;

public class WControls extends Panel implements SoftKeyListener {

	final static String[] labels = {"$How to play ?",
									"---------------------------",
									"- To move :                                             ",
									"£Right/Left arrows",
									"- To aim :                                              ",
									"£Up/Down arrows",
									"- To fire :                                             ",
									"£Select key",
									"- To jump :                                             ",
									"£Star key",
									"- To scroll :                                           ",
									"£2/4/6/8 keys"};
	
	public WControls() {
		setTitle("iWormux HELP");
		setSoftLabel(Frame.SOFT_KEY_2, "OK");
		setSoftKeyListener(this);
		
		Label obj;
		for (int i=0; i<labels.length; i++) {
			switch (labels[i].charAt(0)) {
			case '$': obj = new Label(labels[i].substring(1), Label.CENTER); break;
			case '£': obj = new Label(labels[i].substring(1), Label.RIGHT); break;
			default: obj = new Label(labels[i], Label.LEFT);
			}	
			add(obj);
		}
	}

	public void softKeyPressed(int arg0) { ; }

	public void softKeyReleased(int arg0) {
		Wormux.getGame().closeControls();
	}

}
