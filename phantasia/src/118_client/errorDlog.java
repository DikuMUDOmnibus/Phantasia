import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.event.*;
import java.awt.*;

public class errorDlog extends Dialog implements ActionListener {

    private pClient parent = null;
    Panel top_panel = new Panel();
    Panel bottom_panel = new Panel();
    Label textLabel1 = new Label();
    Label textLabel2 = new Label();
    Label textLabel3 = new Label();
    Button theButton = new Button("OK");

    public errorDlog(pClient c) {

	super(c.f, true);	

	parent = c;

	theButton.addActionListener(this);

	top_panel.setLayout(new BorderLayout());
	top_panel.add("North", textLabel1);
	top_panel.add("Center", textLabel2);
	top_panel.add("South", textLabel3);

	bottom_panel.add(theButton);

	setLayout(new BorderLayout());
	add("North", top_panel);
	add("South", bottom_panel);
    }

    public void bringUp(String string1, String string2, String string3) {
	textLabel1.setAlignment(Label.CENTER);
	textLabel1.setText(string1);
	textLabel2.setAlignment(Label.CENTER);
	textLabel2.setText(string2);
	textLabel3.setAlignment(Label.CENTER);
	textLabel3.setText(string3);

	pack();
	toFront();
	setVisible(true);
    }

    public void actionPerformed(ActionEvent evt) {

	setVisible(false);
	return;
    }
}
