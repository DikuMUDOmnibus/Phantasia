import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.Event;

public class detailDlog extends Dialog implements constants {

    private pClient parent = null;
    Panel bottom_panel = new Panel();
    TextArea textArea = new TextArea(20, 40);
    Button theButton = new Button(OK_LABEL);

    private String title = null;          /* player title */

    public detailDlog(pClient c) {

	super(c.f, false);	

	parent = c;

	bottom_panel.add(theButton);

	setLayout(new BorderLayout());
	add("South", bottom_panel);
	add("Center", textArea);

	title = parent.readString();          /* player title */
	setTitle(title);
	textArea.setText("Character Information -\n");

	textArea.appendText("Modified Name: " + title + "\n");
	textArea.appendText("Name: " + parent.readString() + "\n");
	textArea.appendText("Faithful: " + parent.readString() + "\n");
	textArea.appendText("Parent Account: " + parent.readString() + "\n");
	textArea.appendText("Parent Network: " + parent.readString() + "\n\n");
	textArea.appendText("Mute Count: " + parent.readString() + "\n\n");

	textArea.appendText("Account Information -\n");
	textArea.appendText("Account: " + parent.readString() + "\n");
	textArea.appendText("E-mail: " + parent.readString() + "\n");
	textArea.appendText("Parent Network: " + parent.readString() + "\n\n");

	textArea.appendText("Connection Information -\n");
	textArea.appendText("IP Address: " + parent.readString() + "\n");
	textArea.appendText("Network: " + parent.readString() + "\n");
	textArea.appendText("Machine ID: " + parent.readString() + "\n");
	textArea.appendText("Connected On: " + parent.readString() + "\n");

	pack();		
	show();
    }

    public boolean action(Event evt, Object arg) {
	hide();
	dispose();
	return true;
    }
}
