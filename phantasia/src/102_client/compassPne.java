import java.awt.Panel;
import java.awt.Button;
import java.awt.GridLayout;
import java.io.DataInputStream;
import java.awt.Font;
import java.awt.Event;
import java.lang.Integer;

public class compassPne extends Panel implements constants {

    private pClient parent = null;
    public Button theButtons[] = new Button[9];
    private Font theFont = new Font("TimesRoman", Font.PLAIN, 10);

    public compassPne(pClient c) {

	parent = c;

        String[] buttonTitle = { "NW", "N", "NE", "W", "Rest", "E", "SW", "S", "SE" };

	this.setLayout(new GridLayout(3, 3));

	for (int i = 0; i < 9; i++) {
	    theButtons[i] = new Button(buttonTitle[i]);
	    theButtons[i].setFont(theFont);
	    theButtons[i].resize(18, 18);
	    this.add(theButtons[i]);
	}

	Deactivate();
    }

    public void Activate() {

	for (int i = 0; i < 9; i++) {
	    theButtons[i].enable();
	}
    }

    public void Deactivate() {

	for (int i = 0; i < 9; i++) {
	    theButtons[i].disable();
	}
    }

    public boolean action(Event evt, Object arg) {

	int theResult = 0;
	
	if (parent.pollSendFlag(BUTTONS)) {
	    parent.chat.takeFocus();
	    parent.buttons.Deactivate();
	    Deactivate();

	    for (int i = 0; i < 9; i++) {
	        if (arg.equals(theButtons[i].getLabel())) {
		    theResult = i + 8;
		    break;
	        }
	    }

	    parent.sendString(C_RESPONSE_PACKET + Integer.toString(theResult) + "\0");
	}
	return true;
    }

}
