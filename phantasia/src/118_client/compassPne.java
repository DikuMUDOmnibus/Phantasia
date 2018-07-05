import java.awt.Panel;
import java.awt.Button;
import java.awt.GridLayout;
import java.io.DataInputStream;
import java.awt.Font;
import java.awt.event.*;
import java.lang.Integer;

public class compassPne extends Panel implements ActionListener, constants {

    private pClient parent = null;
    public Button theButtons[] = new Button[9];
    private boolean compassStatus;

    public compassPne(pClient c) {

	parent = c;

        String[] buttonTitle = { "NW", "N", "NE", "W", "Rest", "E", "SW", "S", "SE" };

	this.setLayout(new GridLayout(3, 3));

	for (int i = 0; i < 9; i++) {
	    theButtons[i] = new Button(buttonTitle[i]);
	    theButtons[i].setFont(CompassFont);
	    theButtons[i].setSize(18, 18);
	    theButtons[i].setActionCommand(Integer.toString(i));
	    theButtons[i].addActionListener(this);
	    theButtons[i].addKeyListener(parent);
	    this.add(theButtons[i]);
	}

	Deactivate();
    }

    public void Activate() {

	for (int i = 0; i < 9; i++) {
	    theButtons[i].setEnabled(true);
	}
        compassStatus = true;
    }

    public void Deactivate() {

	for (int i = 0; i < 9; i++) {
	    theButtons[i].setEnabled(false);
	}
        compassStatus = false;
    }

    public void DoButton(int theNumber) {

	if (compassStatus) {

	    if (parent.pollSendFlag(BUTTONS)) {

                parent.chat.takeFocus();
	        parent.buttons.Deactivate();
	        Deactivate();
	        parent.sendString(C_RESPONSE_PACKET + Integer.toString(theNumber+8) + "\0");
	    }
	}
	return;	
    }

    public void actionPerformed(ActionEvent evt) {

	DoButton(Integer.parseInt(evt.getActionCommand()));
	return;
    }

}
