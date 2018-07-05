import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.Event;
import java.lang.Long;

public class coordDlog extends Dialog implements constants {

    private pClient parent = null;
    Panel top_panel = new Panel();
    Panel middle_panel = new Panel();
    Panel bottom_panel = new Panel();
    Panel x_panel = new Panel();
    Panel y_panel = new Panel();
    Label topLabel = new Label();
    Label xLabel = new Label("X Coordinate:");
    Label yLabel = new Label("Y Coordinate:");
    TextField xField = new TextField(15);
    TextField yField = new TextField(15);
    Button okButton = new Button(OK_LABEL);
    Button cancelButton = new Button(CANCEL_LABEL);

    public coordDlog(pClient c) {

	super(c.f, false);	

	parent = c;

	top_panel.add(topLabel);

	x_panel.add(xLabel);
	x_panel.add(xField);

	y_panel.add(yLabel);
	y_panel.add(yField);

	bottom_panel.add(okButton);
	bottom_panel.add(cancelButton);

	middle_panel.setLayout(new BorderLayout());
	middle_panel.add("North", x_panel);
	middle_panel.add("South", y_panel);

	setLayout(new BorderLayout());
	add("North", top_panel);
	add("South", bottom_panel);
	add("Center", middle_panel);
    }

    public void bringUp() {
	top_panel.remove(topLabel);
	topLabel = new Label(parent.readString(), Label.CENTER);
	top_panel.add(topLabel);
	xField.setText("");
	yField.setText("");
	pack();
	show();
	xField.requestFocus();
	toFront();
	parent.raiseSendFlag(COORD_DLOG);
    }

    public void timeout() {
	hide();
    }

    public boolean action(Event evt, Object arg) {
	if (parent.pollSendFlag(COORD_DLOG)) {
	    hide();
	    if (evt.target instanceof TextField || arg.equals(OK_LABEL)) {
	        parent.sendString(C_RESPONSE_PACKET + xField.getText() + "\0" +
			C_RESPONSE_PACKET + yField.getText() + "\0"); 
	    }
	    else {
		parent.sendString(C_CANCEL_PACKET);
	    }
	}
	return true;
    }
}