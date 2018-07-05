import java.awt.*;
import java.net.*;
import java.io.*;
import java.awt.Event;
import java.applet.*;

public class pClient extends Applet implements constants {

	/* Window components */
    Frame f = new Frame("Phantasia v4");
    statusPne status = new statusPne(this);
    buttonPne buttons = new buttonPne(this);
    msgPne messages = new msgPne(this);
    userPne users = new userPne(this);
    chatPne chat = new chatPne(this);
    compassPne compass = new compassPne(this);
    Panel rightPane = new Panel();

    errorDlog errorDialog = new errorDlog(this);

    Socket socket = null;
    lThread listen = null;
    DataOutputStream output = null;
    DataInputStream input = null;

    int ioStatus = NO_REQUEST;


    public void init()
    {
	f.resize(600, 400);
	f.setLayout(new GridBagLayout());
	
	GridBagConstraints constraints = new GridBagConstraints();

	addComponent(status, 0, 0, 2, 1, 0, 0);
	addComponent(messages, 0, 1, 1, 1, 1, 1);
	addComponent(buttons, 0, 2, 1, 1, 0, 0);
	addComponent(chat, 0, 3, 1, 1, 1, 2);
	addComponent(rightPane, 1, 1, 1, 3, 0, 0);

	rightPane.setLayout(new BorderLayout(0, 1));
	rightPane.add("South", compass);
	rightPane.add("Center", users);

	f.setBackground(backgroundColor);

/*
	f.pack();
*/
		
	    /* show the frame */
	f.show();

	status.setImages();

	    /* set up the socket */
        try {
	    socket = new Socket(this.getCodeBase().getHost(), 43302);
	    output = new DataOutputStream(socket.getOutputStream());
	    input = new DataInputStream(socket.getInputStream());
        }
        catch (Exception e) {
            System.out.println("Error: " + e);
	    errorDialog.bringUp("The system can not connect to the server.",
		    "The server could be down or a firewall could exist between you and it.",
		    "Please try again later.");
	    pClientQuit();
        }

	    /* start the listen thread */
	listen = new lThread(this);
	listen.start();
    }

    private void addComponent(Panel item, int x, int y, int width, int height, int weightx, int weighty) {

	GridBagConstraints constraints = new GridBagConstraints();

	    /* add the status area to the frame */
	constraints.gridx = x;
	constraints.gridy = y;
	constraints.gridwidth = width;
	constraints.gridheight = height;
	constraints.weightx = weightx;
	constraints.weighty = weighty;
	constraints.fill = GridBagConstraints.BOTH;
	constraints.anchor = GridBagConstraints.CENTER;
	constraints.insets.top = 2;
	constraints.insets.bottom = 2;
	constraints.insets.left = 2;
	constraints.insets.right = 2;
	((GridBagLayout)f.getLayout()).setConstraints(item, constraints);
	f.add(item);
    }

    public void pClientQuit() {

	    /* destroy the listen thread */
	if (listen != null) {
	    listen.stop();
	    listen = null;
	}

	    /* close the socket */
	if (socket != null) {
	    try {
	        socket.close();
	    }
	    catch (IOException e) {
	        System.out.println("Error: " + e);
	    }
	    socket = null;
	}

	    /* remove the main windw */
	f.hide();
	f.dispose();
	stop();
    }

    public boolean handleEvent(Event evt) {

	    /* if the user closed the frame, destroy the game */
	if (evt.id == Event.WINDOW_DESTROY) {
	    pClientQuit();
	}
	    /* otherwise handle normally */
	return(super.handleEvent(evt));
    }

    public String readString() {

	String message = "";
	
	    try {
		message = input.readLine();
	    }
	    catch (Exception e) {
		System.out.println("Error: " + e);
	        errorDialog.bringUp("There was an error reading from the socket.",
		        "readString error: " + e, "The game will now terminate.");
	    }
/*
System.out.println("Returning from socket: " + message);
*/

	return(message);
    }

    public long readLong() {
	return(Long.parseLong(readString()));
    }

    synchronized public void sendString(String theString) {
	try {
	    output.writeBytes(theString);
	}
	catch (Exception e) {
	        System.out.println("Error: " + e);
	        errorDialog.bringUp("There was an error writing to the socket.",
		        "sendString error: " + e, "The game will now terminate.");
	}
    }

    synchronized public boolean pollSendFlag(int ioArea) {
	
	if (ioStatus == ioArea) {
	    ioStatus = NO_REQUEST;
	    return true;
	}

	return false;
    }

    public void raiseSendFlag(int ioArea) {

	if (ioStatus != NO_REQUEST) {
	    errorDialog.bringUp("Client attempted to have two i/o sources.",
		    "The game will now terminate.", "");
	}

	ioStatus = ioArea;
    }

    public void handlePing() {

	int ioArea = ioStatus;

	if (ioStatus != NO_REQUEST && pollSendFlag(ioArea)) {

	    sendString(C_PING_PACKET);

	    switch(ioArea) {

	    case BUTTONS:
	        buttons.timeout();
		break;

	    case STRING_DLOG:
		listen.stringDialog.timeout();
		break;

	    case COORD_DLOG:
		listen.coordinateDialog.timeout();
		break;

	    case PLAYER_DLOG:
		listen.playerDialog.timeout();
		break;

	    case PASS_DLOG:
		listen.passwordDialog.timeout();
		break;
	    }
	}
    }
}
