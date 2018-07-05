import java.awt.*;
import java.net.*;
import java.io.*;
import java.applet.*;
import java.awt.event.*;
import java.lang.Thread;
import java.awt.event.KeyEvent;

public class pClient extends Applet implements KeyListener, constants {

	/* Main Thread */
    Thread mainThread = null;

	/* Window components */
    Frame f = new Frame("Phantasia v4");
    statusPne status = new statusPne(this);
    buttonPne buttons = new buttonPne(this);
    msgPne messages = new msgPne(this);
    userPne users = new userPne(this);
    chatPne chat = new chatPne(this);
    compassPne compass = new compassPne(this);
    Panel rightPane = new Panel();
    Image theImages[] = new Image[27];

    errorDlog errorDialog = new errorDlog(this);

    Socket socket = null;
    lThread listen = null;
    DataOutputStream output = null;
    BufferedReader input = null;

    int ioStatus = NO_REQUEST;

    public void init()
    {
	mainThread = Thread.currentThread();

	f.addKeyListener(this);

	f.setSize(600, 400);
	f.setLayout(new GridBagLayout());

	    /* handle the closing of the window */
        f.addWindowListener(new WindowAdapter() {
	    public void windowClosing(WindowEvent evt) {
	        pClientQuit();
	    }
        });
 
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
	f.setVisible(true);

	status.loadImages();
	buttons.setImages();

	    /* set up the socket */
        try {
	    socket = new Socket(this.getCodeBase().getHost(), 43302);
	    output = new DataOutputStream(socket.getOutputStream());
	    input = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        }
        catch (Exception e) {
            System.out.println("Error: " + e);
	    errorDialog.bringUp("The system can not connect to the server.",
		    "The server could be down or a firewall could exist between you and it.",
		    "Please try again later.");
	    stop();
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

System.out.println("pClientQuit called.");

	    /* destroy the listen thread */
	if (listen != null) {
	    listen.stop();
	    listen = null;
	}

System.out.println("Listen Thread Stopped.");
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
System.out.println("Socket Closed.");

	    /* remove the main windw */
	f.setVisible(false);
System.out.println("Main window hidden.");
	f.dispose();
System.out.println("Main window disposed.");
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


    public int readBool() {

	String message = "";
	
	try {
	    message = input.readLine();
	}
	catch (Exception e) {
	    System.out.println("Error: " + e);
	    errorDialog.bringUp("There was an error reading from the socket.",
	            "readString error: " + e, "The game will now terminate.");
	}

	if (message.equals("No")) {
	    return 0;
	}
	else if (message.equals("Yes")) {
	    return 1;
	}
	else {
	    System.out.println("Error: readBool read the string " + message);
	    errorDialog.bringUp("There was an error reading from the socket.",
	            "readBool read the string " + message,
		    "The game will now terminate.");
   	} 
	return 0;
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
/*
System.out.println("Request Send flag - Poll=" + Integer.toString(ioArea) + "  Status=" + Integer.toString(ioStatus));
*/
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
	    }
	}
    }

    public void keyPressed(KeyEvent evt) {

	int theKey;

	theKey = evt.getKeyCode();

	switch(theKey) {

	case KeyEvent.VK_1:
	case KeyEvent.VK_F1:
	    buttons.DoButton(0);
	    break;
	case KeyEvent.VK_2:
	case KeyEvent.VK_F2:
	    buttons.DoButton(1);
	    break;
	case KeyEvent.VK_3:
	case KeyEvent.VK_F3:
	    buttons.DoButton(2);
	    break;
	case KeyEvent.VK_4:
	case KeyEvent.VK_F4:
	    buttons.DoButton(3);
	    break;
	case KeyEvent.VK_5:
	case KeyEvent.VK_F5:
	    buttons.DoButton(4);
	    break;
	case KeyEvent.VK_6:
	case KeyEvent.VK_F6:
	    buttons.DoButton(5);
	    break;
	case KeyEvent.VK_7:
	case KeyEvent.VK_F7:
	    buttons.DoButton(6);
	    break;
	case KeyEvent.VK_8:
	case KeyEvent.VK_F8:
	    buttons.DoButton(7);
	    break;
	case KeyEvent.VK_NUMPAD1:
	    compass.DoButton(6);
	    break;
	case KeyEvent.VK_NUMPAD2:
	    compass.DoButton(7);
	    break;
	case KeyEvent.VK_NUMPAD3:
	    compass.DoButton(8);
	    break;
	case KeyEvent.VK_NUMPAD4:
	    compass.DoButton(3);
	    break;
	case KeyEvent.VK_NUMPAD5:
	    compass.DoButton(4);
	    break;
	case KeyEvent.VK_NUMPAD6:
	    compass.DoButton(5);
	    break;
	case KeyEvent.VK_NUMPAD7:
	    compass.DoButton(0);
	    break;
	case KeyEvent.VK_NUMPAD8:
	    compass.DoButton(1);
	    break;
	case KeyEvent.VK_NUMPAD9:
	    compass.DoButton(2);
	    break;
	case KeyEvent.VK_SPACE:
	    buttons.spacebar();
	    break;
	}

	return;
    }

    public void keyReleased(KeyEvent evt) {;}
    public void keyTyped(KeyEvent evt) {;}
}
