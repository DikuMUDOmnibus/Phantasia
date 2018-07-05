import java.awt.GridLayout;
import java.awt.BorderLayout;
import java.io.*;
import java.awt.Panel;
import java.awt.Button;
import java.awt.Font;
import java.awt.event.*;
import java.lang.Integer;

public class buttonPne extends Panel implements ActionListener, constants {

    private pClient parent = null;
    private labelPne buttonLabel[] = new labelPne[8];
    private Panel buttonPanel[] = new Panel[8];
    public Button theButtons[] = new Button[8];
    private boolean buttonStatus[] = new boolean[8];

    public buttonPne(pClient c) {

	parent = c;

	this.setLayout(new GridLayout(1, 8, 1, 0));

	for (int i = 0; i < 8; i++) {

	    buttonLabel[i] = new labelPne(parent);
	    buttonLabel[i].setSize(9, 9);

	    theButtons[i] = new Button();
	    theButtons[i].setFont(ButtonFont);
	    theButtons[i].setActionCommand(Integer.toString(i));
	    theButtons[i].addActionListener(this);
	    theButtons[i].addKeyListener(parent);

	    buttonPanel[i] = new Panel();
	    buttonPanel[i].setBackground(backgroundColor);
	    buttonPanel[i].setLayout(new BorderLayout());
	    buttonPanel[i].add("North", theButtons[i]);
	    buttonPanel[i].add("South", buttonLabel[i]);

	    this.add(buttonPanel[i]);
	}

	Deactivate();
    }

    public void Deactivate() {

	for (int i = 0; i < 8; i++) {
	    theButtons[i].setEnabled(false);
	    buttonStatus[i] = false;
	}
    }

    public void Question() {

	String buttonLabel = null;

	for (int i = 0; i < 8; i++) {

	    buttonLabel = parent.readString();

	    theButtons[i].setLabel(buttonLabel);

	    if (buttonLabel.length() > 0) {
		theButtons[i].setEnabled(true);
		buttonStatus[i] = true;
	    }
	}
	parent.raiseSendFlag(BUTTONS);
    }

    public void turn() {
	parent.compass.Activate();
	Question();
    }

    public void DoButton(int theNumber) {

	if (buttonStatus[theNumber]) {

	    if (parent.pollSendFlag(BUTTONS)) {

		parent.chat.takeFocus();	
	        parent.compass.Deactivate();
	        Deactivate();
	        parent.sendString(C_RESPONSE_PACKET + Integer.toString(theNumber) + "\0");
	    }
	}
	return;	
    }

    public void actionPerformed(ActionEvent evt) {

	DoButton(Integer.parseInt(evt.getActionCommand()));
	return;
    }

    public void timeout() {
	parent.compass.Deactivate();
	Deactivate();
    }

    public void setImages() {

	for (int i = 0; i < 8; i++) {

	    buttonLabel[i].setImage(19 + i);
	}

    }

    public void spacebar() {

	for (int i = 0; i < 8; i++) {

	    if (theButtons[i].isEnabled()) {
		DoButton(i);
		return;
	    }
	}

	return;
    }

}
