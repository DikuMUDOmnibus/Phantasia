import java.awt.Panel;
import java.awt.TextArea;
import java.awt.List;
import java.io.DataInputStream;
import java.awt.BorderLayout;

public class msgPne extends Panel implements constants {

    pClient parent = null;
    public TextArea textArea = null;

    public msgPne(pClient c) {

	parent = c;

	textArea = new TextArea("", 5, 60, TextArea.SCROLLBARS_VERTICAL_ONLY);
	textArea.setFont(MsgFont);
	textArea.setEditable(false);
	textArea.addKeyListener(parent);

	setLayout(new BorderLayout());
	add("Center", textArea);
    }

    public void PrintLine() {
	textArea.append(parent.readString() + "\n");
    }

    public void ClearScreen() {
	textArea.selectAll();
	textArea.replaceRange("", textArea.getSelectionStart(), textArea.getSelectionEnd());
    }
}
