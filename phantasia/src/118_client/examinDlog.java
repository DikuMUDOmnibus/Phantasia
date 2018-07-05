import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.event.*;

public class examinDlog extends Dialog implements ActionListener, constants {

    private pClient parent = null;
    Panel bottom_panel = new Panel();
    TextArea textArea = new TextArea(20, 40);
    Button theButton = new Button(OK_LABEL);

    private String title = null;          /* player title */

    public examinDlog(pClient c) {

	super(c.f, false);	

	parent = c;

	theButton.addActionListener(this);
	bottom_panel.add(theButton);

	setLayout(new BorderLayout());
	add("South", bottom_panel);
	add("Center", textArea);

	title = parent.readString();          /* player title */
	setTitle(title);
	textArea.setText(title + "\n");

	textArea.append("Location: " + parent.readString() + "\n\n");

	textArea.append("Account: " + parent.readString() + "\n");
	textArea.append("Network: " + parent.readString() + "\n");
	textArea.append("Channel: " + parent.readString() + "\n\n");

	textArea.append("Level: " + parent.readString() + "\n");
	textArea.append("Experience: " + parent.readString() + "\n");
	textArea.append("Next Level At: " + parent.readString() + "\n\n");

	textArea.append("Energy: " + parent.readString() + " / (" + parent.readString() + " + " + parent.readString() + " shield)\n");
	textArea.append("Strength: " + parent.readString() + " / (" + parent.readString() + " + " + parent.readString() + " sword)\n");
	textArea.append("Quickness: " + parent.readString() + " / (" + parent.readString() + " + " + parent.readString() + " quicksilver)\n");
	textArea.append("Brains: " + parent.readString() + "\n");
	textArea.append("Magic Level: " + parent.readString() + "\n");
	textArea.append("Mana: " + parent.readString() + "\n");
	textArea.append("Gender: " + parent.readString() + "\n");
	textArea.append("Poison: " + parent.readString() + "\n");
	textArea.append("Sin: " + parent.readString() + "\n");
	textArea.append("Lives: " + parent.readString() + "\n\n");

	textArea.append("Gold: " + parent.readString() + "\n");
	textArea.append("Gems: " + parent.readString() + "\n");
	textArea.append("Holy Water: " + parent.readString() + "\n");
	textArea.append("Amulets: " + parent.readString() + "\n");
	textArea.append("Charms: " + parent.readString() + "\n");
	textArea.append("Staves/Crowns: " + parent.readString() + "\n");
	textArea.append("Virgin: " + parent.readString() + "\n");
	textArea.append("Blessing: " + parent.readString() + "\n");
	textArea.append("Palantir: " + parent.readString() + "\n");
	textArea.append("Ring: " + parent.readString() + "\n\n");

	textArea.append("Cloaked: " + parent.readString() + "\n");
	textArea.append("Blind: " + parent.readString() + "\n");
	textArea.append("Age: " + parent.readString() + "\n");
	textArea.append("Degenerated: " + parent.readString() + "\n");
	textArea.append("Played For: " + parent.readString() + "\n");
	textArea.append("Loaded On: " + parent.readString() + "\n");
	textArea.append("Created On: " + parent.readString() + "\n");

	pack();		
	setVisible(true);
    }

    public void actionPerformed(ActionEvent evt) {
	setVisible(false);
	dispose();
	return;
    }
}
