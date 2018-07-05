import java.awt.Panel;
import java.awt.List;
import java.awt.BorderLayout;
import java.io.DataInputStream;
import java.util.Vector;
import java.awt.event.*;

public class userPne extends Panel implements ActionListener, constants {

    private List theDisplay = new List();
    public Vector theList = new Vector();

    private pClient parent = null;

    public userPne(pClient c) {

	parent = c;

	theDisplay.setFont(UserFont);
	theDisplay.addActionListener(this);

	this.setLayout(new BorderLayout());
	this.add("Center", theDisplay);
    }

    public void AddUser() {

	String name = parent.readString();

	theDisplay.addItem(parent.readString() + "- " + name);

	theList.addElement(name);
    }

    public void RemoveUser() {

	int index = 0;

	index = theList.indexOf(parent.readString());

	theDisplay.delItem(index);
	theList.removeElementAt(index);
    }

    public void actionPerformed(ActionEvent evt) {

	parent.sendString(C_EXAMINE_PACKET + theList.elementAt(theDisplay.getSelectedIndex()) + "\0");
	return;
    }
}
