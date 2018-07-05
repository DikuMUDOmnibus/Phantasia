import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.event.*;

public class stdDlog extends Dialog implements ActionListener {

    private pClient parent = null;
    Panel top_panel = new Panel();
    Panel bottom_panel = new Panel();
    Label textLabel = new Label();
    Button theButton = new Button("OK");

    public stdDlog(pClient c) {

	super(c.f, true);	

	parent = c;

	theButton.addActionListener(this);

	textLabel.setText(parent.readString());
	top_panel.add(textLabel);
	bottom_panel.add(theButton);

	setLayout(new BorderLayout());
	add("North", top_panel);
	add("South", bottom_panel);

	pack();
	toFront();
	setVisible(true);
    }

    public void actionPerformed(ActionEvent evt) {
	setVisible(false);
	dispose();
	return;
    }
}
