import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.Event;
import java.awt.*;

public class playerDlog extends Dialog implements constants {

    private pClient parent = null;
    Panel top_panel = new Panel();
    Panel middle_panel = new Panel();
    Panel bottom_panel = new Panel();
    Label textLabel = new Label();
    Choice popUpMenu = new Choice();
    Button okButton = new Button(OK_LABEL);
    Button cancelButton = new Button(CANCEL_LABEL);

    public playerDlog(pClient c) {

	super(c.f, true);	

	parent = c;

	top_panel.add(textLabel);
	middle_panel.add(popUpMenu);
	bottom_panel.add(okButton);
	bottom_panel.add(cancelButton);

	setLayout(new BorderLayout());
	add("North", top_panel);
	add("Center", middle_panel);
	add("South", bottom_panel);
    }

    public void bringUp() {

	textLabel.setText(parent.readString());

	middle_panel.remove(popUpMenu);
	popUpMenu = new Choice();
	middle_panel.add(popUpMenu);
	
	for (int i = 0; i < parent.users.theList.size(); i++)
	    popUpMenu.addItem((String) parent.users.theList.elementAt(i));

	parent.raiseSendFlag(PLAYER_DLOG);
	pack();
	toFront();
	show();
    }

    public void timeout() {
	hide();
    }

    public boolean action(Event evt, Object arg) {

	if (evt.target instanceof Choice) {
	    return true;
	}

	if (parent.pollSendFlag(PLAYER_DLOG)) {
	    hide();
	    if (arg.equals(OK_LABEL)) {
	        parent.sendString(C_RESPONSE_PACKET + popUpMenu.getSelectedItem() + "\0");
	    }
	    else {
		parent.sendString(C_CANCEL_PACKET);
	    }
	}
	return true;
    }
}