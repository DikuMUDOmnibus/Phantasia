import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.event.*;

public class scoreDlog extends Dialog implements ActionListener, constants {

    private pClient parent = null;
    Panel top_panel = new Panel();
    Panel bottom_panel = new Panel();
    TextArea textArea = new TextArea(20, 80);
    Button nextButton = new Button("Next");
    Button prevButton = new Button("Prev");
    Button theButton = new Button(OK_LABEL);

    private long start, records;

    public scoreDlog(pClient c) {

	super(c.f, false);	

	parent = c;

	nextButton.addActionListener(this);
	nextButton.setActionCommand("next");
	bottom_panel.add(nextButton);

	prevButton.addActionListener(this);
	prevButton.setActionCommand("prev");
	bottom_panel.add(prevButton);

	theButton.addActionListener(this);
	theButton.setActionCommand("ok");
	bottom_panel.add(theButton);

	setLayout(new BorderLayout());
	add("South", bottom_panel);
	add("Center", textArea);

	start = parent.readLong();	/* Starting record */
	records = parent.readLong();          /* Records to print */
	setTitle("Scoreboard");

	if (start == 0) {
	  prevButton.setEnabled(false);
	}

	if (records == 1) {
	  nextButton.setEnabled(false);
	}

	    /* print out all the high scores */
	for (int i = 0; i < records; i++) {
	    textArea.append(parent.readString() + "\n");
	}

	pack();		
	setVisible(true);
    }

    public void actionPerformed(ActionEvent evt) {

	if ((evt.getActionCommand()).equals("next")) {
	    parent.sendString(C_SCOREBOARD_PACKET);
	    parent.sendString(Long.toString(start + 50) + "\0");
	}
	else if ((evt.getActionCommand()).equals("prev")) {
	    parent.sendString(C_SCOREBOARD_PACKET);
	    parent.sendString(Long.toString(start - 50) + "\0");
	}

	setVisible(false);
	dispose();
	return;
    }
}
