import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.Event;
import java.awt.*;

public class passDlog extends Dialog implements constants {

    private pClient parent = null;
    Panel top_panel = new Panel();
    Panel middle_panel = new Panel();
    Panel bottom_panel = new Panel();
    Label textLabel = new Label();
    TextField textField = new TextField(12);
    Button okButton = new Button(OK_LABEL);
    Button cancelButton = new Button(CANCEL_LABEL);

    public passDlog(pClient c) {

	super(c.f, false);	

	parent = c;

	top_panel.add(textLabel);
	middle_panel.add(textField);
	bottom_panel.add(okButton);
	bottom_panel.add(cancelButton);

	setLayout(new BorderLayout());
	add("North", top_panel);
	add("Center", middle_panel);
	add("South", bottom_panel);

	textField.setEchoCharacter('X');
    }

    public void bringUp() {
	top_panel.remove(textLabel);
	textLabel = new Label(parent.readString(), Label.CENTER);
	top_panel.add(textLabel);
	textField.setText("");
	pack();
	show();
	textField.requestFocus();
	toFront();
	parent.raiseSendFlag(PASS_DLOG);
    }

    public void timeout() {
	hide();
    }

    public boolean action(Event evt, Object arg) {
	if (parent.pollSendFlag(PASS_DLOG)) {
	    hide();
	    if (evt.target instanceof TextField || arg.equals(OK_LABEL)) {
	        parent.sendString(C_RESPONSE_PACKET + textField.getText() + "\0");
	    }
	    else {
		parent.sendString(C_CANCEL_PACKET);
	    }
	}
	return true;
    }
}