import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.Event;

public class examinDlog extends Dialog implements constants {

    private pClient parent = null;
    Panel bottom_panel = new Panel();
    TextArea textArea = new TextArea(20, 40);
    Button theButton = new Button(OK_LABEL);

    private String title = null;          /* player title */

    public examinDlog(pClient c) {

	super(c.f, false);	

	parent = c;

	bottom_panel.add(theButton);

	setLayout(new BorderLayout());
	add("South", bottom_panel);
	add("Center", textArea);

	title = parent.readString();          /* player title */
	setTitle(title);
	textArea.setText(title + "\n");

	textArea.appendText("Location: " + parent.readString() + "\n\n");

	textArea.appendText("Account: " + parent.readString() + "\n");
	textArea.appendText("Network: " + parent.readString() + "\n");
	textArea.appendText("Channel: " + parent.readString() + "\n\n");

	textArea.appendText("Level: " + parent.readString() + "\n");
	textArea.appendText("Experience: " + parent.readString() + "\n");
	textArea.appendText("Next Level At: " + parent.readString() + "\n\n");

	textArea.appendText("Energy: " + parent.readString() + " / (" + parent.readString() + " + " + parent.readString() + " shield)\n");
	textArea.appendText("Strength: " + parent.readString() + " / (" + parent.readString() + " + " + parent.readString() + " sword)\n");
	textArea.appendText("Quickness: " + parent.readString() + " / (" + parent.readString() + " + " + parent.readString() + " quicksilver)\n");
	textArea.appendText("Brains: " + parent.readString() + "\n");
	textArea.appendText("Magic Level: " + parent.readString() + "\n");
	textArea.appendText("Mana: " + parent.readString() + "\n");
	textArea.appendText("Gender: " + parent.readString() + "\n");
	textArea.appendText("Poison: " + parent.readString() + "\n");
	textArea.appendText("Sin: " + parent.readString() + "\n");
	textArea.appendText("Lives: " + parent.readString() + "\n\n");

	textArea.appendText("Gold: " + parent.readString() + "\n");
	textArea.appendText("Gems: " + parent.readString() + "\n");
	textArea.appendText("Holy Water: " + parent.readString() + "\n");
	textArea.appendText("Amulets: " + parent.readString() + "\n");
	textArea.appendText("Charms: " + parent.readString() + "\n");
	textArea.appendText("Staves/Crowns: " + parent.readString() + "\n");
	textArea.appendText("Virgin: " + parent.readString() + "\n");
	textArea.appendText("Blessing: " + parent.readString() + "\n");
	textArea.appendText("Palantir: " + parent.readString() + "\n");
	textArea.appendText("Ring: " + parent.readString() + "\n\n");

	textArea.appendText("Cloaked: " + parent.readString() + "\n");
	textArea.appendText("Blind: " + parent.readString() + "\n");
	textArea.appendText("Age: " + parent.readString() + "\n");
	textArea.appendText("Degenerated: " + parent.readString() + "\n");
	textArea.appendText("Played For: " + parent.readString() + "\n");
	textArea.appendText("Loaded On: " + parent.readString() + "\n");
	textArea.appendText("Created On: " + parent.readString() + "\n");

	pack();		
	show();
    }

    public boolean action(Event evt, Object arg) {
	hide();
	dispose();
	return true;
    }
}
