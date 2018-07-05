import java.awt.GridLayout;
import java.io.*;
import java.awt.Panel;
import java.awt.Button;
import java.awt.Font;
import java.awt.Event;
import java.lang.Integer;

public class buttonPne extends Panel implements constants {

    private pClient parent = null;
    public Button theButtons[] = new Button[8];
    private Font theFont = new Font("TimesRoman", Font.PLAIN, 9);

    public buttonPne(pClient c) {

	parent = c;

	this.setLayout(new GridLayout(1, 8));

	for (int i = 0; i < 8; i++) {
	    theButtons[i] = new Button();
	    theButtons[i].setFont(theFont);
	    this.add(theButtons[i]);
	}

	Deactivate();
    }

    public void Deactivate() {

	for (int i = 0; i < 8; i++) {
	    theButtons[i].disable();
	}
    }

    public void Question() {

	String buttonLabel = null;

	for (int i = 0; i < 8; i++) {

	    buttonLabel = parent.readString();

	    theButtons[i].setLabel(buttonLabel);

	    if (buttonLabel.length() > 0) {
		theButtons[i].enable();

		if (i == 0) {
		    theButtons[i].requestFocus();
		}
	    }
	}
	parent.chat.takeFocus();
	parent.raiseSendFlag(BUTTONS);
    }

    public void turn() {
	parent.compass.Activate();
	Question();
    }

    public boolean action(Event evt, Object arg) {

	int theResult = 0;
	
	if (parent.pollSendFlag(BUTTONS)) {
	
	    parent.chat.takeFocus();
	    parent.compass.Deactivate();
	    Deactivate();

	    for (int i = 0; i < 8; i++) {
	        if (arg.equals(theButtons[i].getLabel())) {
		    theResult = i;
		    break;
	        }
	    }

	    parent.sendString(C_RESPONSE_PACKET + Integer.toString(theResult) + "\0");
	}

	return true;
    }

    public void timeout() {
	parent.compass.Deactivate();
	Deactivate();
    }
}
