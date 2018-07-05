import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.event.*;
import java.awt.*;

public class playerDlog extends Dialog implements ActionListener, constants {

    private pClient parent = null;
    Panel top_panel = new Panel();
    Panel middle_panel = new Panel();
    Panel bottom_panel = new Panel();
    Label textLabel = null;
    Choice popUpMenu = new Choice();
    Button okButton = new Button(OK_LABEL);
    Button cancelButton = new Button(CANCEL_LABEL);

    public playerDlog(pClient c) {

	super(c.f, true);	

	parent = c;

	okButton.addActionListener(this);
	cancelButton.addActionListener(this);
	cancelButton.setActionCommand("Cancel");

	textLabel = new Label(parent.readString());

	top_panel.add(textLabel);
	middle_panel.add(popUpMenu);
	bottom_panel.add(okButton);
	bottom_panel.add(cancelButton);

	setLayout(new BorderLayout());
	add("North", top_panel);
	add("Center", middle_panel);
	add("South", bottom_panel);

	middle_panel.remove(popUpMenu);
	popUpMenu = new Choice();
	middle_panel.add(popUpMenu);
	
	for (int i = 0; i < parent.users.theList.size(); i++)
	    popUpMenu.addItem((String) parent.users.theList.elementAt(i));

	    /* this must go before setVisible, because it doesn't return */
	parent.raiseSendFlag(PLAYER_DLOG);

	pack();
	setVisible(true);
	okButton.requestFocus();
    }

    public void timeout() {
	setVisible(false);
    }

    public void actionPerformed(ActionEvent evt) {
	if (parent.pollSendFlag(PLAYER_DLOG)) {
	    setVisible(false);
	    if (evt.getActionCommand().equals("Cancel")) {
		parent.sendString(C_CANCEL_PACKET);
	    }
	    else {
	        parent.sendString(C_RESPONSE_PACKET + popUpMenu.getSelectedItem() + "\0");
	    }
	    dispose();
	}
	return;
    }
}