import java.awt.Panel;
import java.awt.Button;
import java.awt.TextField;
import java.awt.TextArea;
import java.awt.List;
import java.awt.BorderLayout;
import java.io.DataInputStream;
import java.awt.Color;
import java.awt.Event;

public class chatPne extends Panel implements constants {

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

	textArea = new TextArea(3, 60);
/*
	textArea.addItem("");
*/
	textArea.setFont(ChatFont);
	textArea.setEditable(false);

	sendButton = new Button("Send");
	sendButton.disable();

	clearButton = new Button("Clear");

	theTextField = new TextField();

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

	textArea.insertText(parent.readString() + "\n", 0);
/*
	textArea.appendText(parent.readString() + "\n");
	textArea.addItem(parent.readString());
	if (textArea.countItems() > 50)
	    textArea.delItem(1);
*/
    }

    public void takeFocus() {
        textArea.requestFocus();
    }

    public boolean action(Event evt, Object arg) {

	if (active) {
	    if (evt.target instanceof TextField || arg.equals("Send")) {

	        String theString = theTextField.getText();
		parent.sendString(C_CHAT_PACKET + theString + "\0");
	        theTextField.setText("");
		return true;
	    }

	    if (arg.equals("Clear")) {

		textArea.selectAll();
		textArea.replaceText("", textArea.getSelectionStart(), textArea.getSelectionEnd());
		return true;
	    }
	}

	return false;
    }

    public void activateChat() {
	active = true;
	sendButton.enable();
    }

    public void deactivateChat() {
	active = false;
	sendButton.disable();
    }
}
