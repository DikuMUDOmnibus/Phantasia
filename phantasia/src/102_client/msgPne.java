import java.awt.Panel;
import java.awt.TextArea;
import java.awt.List;
import java.io.DataInputStream;
import java.awt.BorderLayout;

public class msgPne extends Panel implements constants {

/*
    public TextArea textArea = null;
*/
    pClient parent = null;
    public TextArea textArea = null;

    public msgPne(pClient c) {

	parent = c;

	textArea = new TextArea(5, 60);
/*
	textArea.addItem("");
*/
	textArea.setFont(MsgFont);
	textArea.setEditable(false);

	setLayout(new BorderLayout());
	add("Center", textArea);
    }

    public void PrintLine() {
	textArea.insertText(parent.readString() + "\n", 0);
/*
	textArea.appendText(parent.readString() + "\n");
	textArea.addItem(parent.readString());
*/
    }

    public void ClearScreen() {
	textArea.selectAll();
	textArea.replaceText("", textArea.getSelectionStart(), textArea.getSelectionEnd());
/*
	textArea.clear();
	textArea.setText(null);
*/
    }
}
