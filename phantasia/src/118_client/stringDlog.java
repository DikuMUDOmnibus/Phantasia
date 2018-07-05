import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.event.*;
import java.awt.*;

public class stringDlog extends Dialog implements ActionListener, constants {

    private pClient parent = null;
    Panel top_panel = new Panel();
    Panel middle_panel = new Panel();
    Panel bottom_panel = new Panel();
    Label textLabel = null;
    TextField textField = new TextField(12);
    Button okButton = new Button(OK_LABEL);
    Button cancelButton = new Button(CANCEL_LABEL);

    public stringDlog(pClient c, boolean hidden) {

	super(c.f, false);	

	parent = c;

	okButton.addActionListener(this);
	cancelButton.addActionListener(this);
	textField.addActionListener(this);

	cancelButton.setActionCommand("Cancel");

	textLabel = new Label(parent.readString(), Label.CENTER);

	top_panel.add(textLabel);
	middle_panel.add(textField);
	bottom_panel.add(okButton);
	bottom_panel.add(cancelButton);

	setLayout(new BorderLayout());
	add("North", top_panel);
	add("Center", middle_panel);
	add("South", bottom_panel);

	if (hidden) {
	    textField.setEchoChar('X');
	}

	pack();
	setVisible(true);
	textField.requestFocus();
	parent.raiseSendFlag(STRING_DLOG);
    }

    public void timeout() {
	setVisible(false);
    }

    public void actionPerformed(ActionEvent evt) {
	if (parent.pollSendFlag(STRING_DLOG)) {
	    setVisible(false);
	    if (evt.getActionCommand().equals("Cancel")) {
		parent.sendString(C_CANCEL_PACKET);
	    }
	    else {
	        parent.sendString(C_RESPONSE_PACKET + textField.getText() + "\0");
	    }
	    dispose();
	}
	return;
    }
}