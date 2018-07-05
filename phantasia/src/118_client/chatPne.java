import java.awt.Panel;
import java.awt.Button;
import java.awt.TextField;
import java.awt.TextArea;
import java.awt.List;
import java.awt.BorderLayout;
import java.io.DataInputStream;
import java.awt.Color;
import java.awt.event.*;

public class chatPne extends Panel implements ActionListener, KeyListener, constants {

    pClient parent = null;
    TextArea textArea = null;
    Button sendButton = null;
    Button clearButton = null;
    TextField theTextField = null;
    boolean active = false;
    Panel controlPane = new Panel();
    Panel buttonPane = new Panel();

    public chatPne(pClient c) {

	parent = c;

	setBackground(backgroundColor);

	textArea = new TextArea("", 3, 60, TextArea.SCROLLBARS_VERTICAL_ONLY);
	textArea.setFont(ChatFont);
	textArea.setEditable(false);
	textArea.addKeyListener(parent);

	sendButton = new Button("Send");
	sendButton.setFont(ButtonFont);
	sendButton.addActionListener(this);
	sendButton.setEnabled(false);

	clearButton = new Button("Clear");
	clearButton.setFont(ButtonFont);
	clearButton.setActionCommand("Clear");
	clearButton.addActionListener(this);

	theTextField = new TextField();
	theTextField.setFont(ChatFont);
	theTextField.addActionListener(this);
	theTextField.addKeyListener(this);

	buttonPane.setLayout(new BorderLayout());
	buttonPane.add("West", clearButton);
	buttonPane.add("East", sendButton);

	controlPane.setLayout(new BorderLayout());
	controlPane.add("West", buttonPane);
	controlPane.add("Center", theTextField);

	this.setLayout(new BorderLayout());
	this.add("South", controlPane);
	this.add("Center", textArea);
    }

    public void PrintLine() {

	textArea.append(parent.readString() + "\n");
/*
	textArea.addItem(parent.readString());
	if (textArea.countItems() > 50)
	    textArea.delItem(1);
*/
    }

    public void actionPerformed(ActionEvent evt) {

	if (active) {
	    if (evt.getActionCommand().equals("Clear")) {

		textArea.selectAll();
		textArea.replaceRange("", textArea.getSelectionStart(), textArea.getSelectionEnd());
	    }
	    else {
	        String theString = theTextField.getText();
		parent.sendString(C_CHAT_PACKET + theString + "\0");
	        theTextField.setText("");
	    }
	}

	return;
    }

    public void activateChat() {
	active = true;
	sendButton.setEnabled(true);
    }

    public void deactivateChat() {
	active = false;
	sendButton.setEnabled(false);
    }

    public void takeFocus() {
	textArea.requestFocus();
    }

	/* handle function keys pressed in the chat window */
    public void keyPressed(KeyEvent evt) {

	int theKey;

	theKey = evt.getKeyCode();

	if (theKey == KeyEvent.VK_F1 || theKey == KeyEvent.VK_F2 ||
		theKey == KeyEvent.VK_F3 || theKey == KeyEvent.VK_F4 ||
		theKey == KeyEvent.VK_F5 || theKey == KeyEvent.VK_F6 ||
		theKey == KeyEvent.VK_F7 || theKey == KeyEvent.VK_F8) {

	    parent.keyPressed(evt);
	}

	return;
    }

    public void keyReleased(KeyEvent evt) {;}
    public void keyTyped(KeyEvent evt) {;}
}
