import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.Event;

public class stdDlog extends Dialog {

    private pClient parent = null;
    Panel top_panel = new Panel();
    Panel bottom_panel = new Panel();
    Label textLabel = new Label();
    Button theButton = new Button("OK");

    public stdDlog(pClient c) {

	super(c.f, true);	

	parent = c;

	textLabel.setText(parent.readString());
	top_panel.add(textLabel);
	bottom_panel.add(theButton);

	setLayout(new BorderLayout());
	add("North", top_panel);
	add("South", bottom_panel);

	pack();
	toFront();
	show();
    }

    public boolean action(Event evt, Object arg) {
	hide();
	dispose();
	return true;
    }
}
