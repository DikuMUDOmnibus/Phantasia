import java.awt.*;
import java.net.*;
import java.io.*;

public class scoreDlog extends Dialog implements constants {

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

	bottom_panel.add(nextButton);
	bottom_panel.add(prevButton);
	bottom_panel.add(theButton);

	setLayout(new BorderLayout());
	add("South", bottom_panel);
	add("Center", textArea);

	start = parent.readLong();	/* Starting record */
	records = parent.readLong();          /* Records to print */
	setTitle("Scoreboard");

	if (start == 0) {
	  prevButton.disable();
	}

	if (records == 1) {
	  nextButton.disable();
	}

	    /* print out all the high scores */
	for (int i = 0; i < records; i++) {
	    textArea.appendText(parent.readString() + "\n");
	}

	pack();		
	show();
    }

    public boolean action(Event evt, Object arg) {

	if (arg.equals("next")) {
	    parent.sendString(C_SCOREBOARD_PACKET);
	    parent.sendString(Long.toString(start + 50) + "\0");
	}
	else if (arg.equals("prev")) {
	    parent.sendString(C_SCOREBOARD_PACKET);
	    parent.sendString(Long.toString(start - 50) + "\0");
	}

	hide();
	dispose();
	return true;
    }
}
