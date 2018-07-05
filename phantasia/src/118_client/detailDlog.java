import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.event.*;

public class detailDlog extends Dialog implements ActionListener, constants {

    private pClient parent = null;
    Panel bottom_panel = new Panel();
    TextArea textArea = new TextArea(20, 40);
    Button theButton = new Button(OK_LABEL);

    private String title = null;          /* player title */

    public detailDlog(pClient c) {

	super(c.f, false);	

	parent = c;

	theButton.addActionListener(this);
	bottom_panel.add(theButton);

	setLayout(new BorderLayout());
	add("South", bottom_panel);
	add("Center", textArea);

	title = parent.readString();          /* player title */
	setTitle(title);
	textArea.setText("Character Information -\n");

	textArea.append("Modified Name: " + title + "\n");
	textArea.append("Name: " + parent.readString() + "\n");
	textArea.append("Faithful: " + parent.readString() + "\n");
	textArea.append("Parent Account: " + parent.readString() + "\n");
	textArea.append("Parent Network: " + parent.readString() + "\n\n");
	textArea.append("Mute Count: " + parent.readString() + "\n\n");

	textArea.append("Account Information -\n");
	textArea.append("Account: " + parent.readString() + "\n");
	textArea.append("E-mail: " + parent.readString() + "\n");
	textArea.append("Parent Network: " + parent.readString() + "\n\n");

	textArea.append("Connection Information -\n");
	textArea.append("IP Address: " + parent.readString() + "\n");
	textArea.append("Network: " + parent.readString() + "\n");
	textArea.append("Machine ID: " + parent.readString() + "\n");
	textArea.append("Connected On: " + parent.readString() + "\n");

	pack();		
	setVisible(true);
    }

    public void actionPerformed(ActionEvent evt) {
	setVisible(false);
	dispose();
	return;
    }
}
