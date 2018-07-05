import java.awt.*;
import java.net.*;
import java.io.*;

class lThread extends Thread implements constants {

    pClient parent = null;

    examinDlog examinePlayer = null;
    stringDlog stringDialog = null;
    coordDlog coordinateDialog = null;
    playerDlog playerDialog = null;
    passDlog passwordDialog = null;
    scoreDlog scoreDialog = null;

    public lThread(pClient c) {
	parent = c;

        stringDialog = new stringDlog(parent);
        coordinateDialog = new coordDlog(parent);
        playerDialog = new playerDlog(parent);
	passwordDialog = new passDlog(parent);

    }

    public void run() {

	int thePacket = 0;
	stdDlog stdDialog = null;
	detailDlog detailDialog = null;
        examinePlayer = null;

	for (;;) {

	    thePacket = (int) parent.readLong();
	    switch(thePacket) {

	    case HANDSHAKE_PACKET:
                parent.sendString(C_RESPONSE_PACKET + clientVersion + "\0");

                parent.sendString(C_RESPONSE_PACKET +
                        parent.getParameter("num") + "\0");

                parent.sendString(C_RESPONSE_PACKET +
                        parent.getParameter("hash") + "\0");

                parent.sendString(C_RESPONSE_PACKET +
                        parent.getParameter("time") + "\0");

		break;

	    case CLOSE_CONNECTION_PACKET:
		lThreadQuit();
		break;

	    case PING_PACKET:
		parent.handlePing();
		break;

	    case ADD_PLAYER_PACKET:
		parent.users.AddUser();
		break;

	    case REMOVE_PLAYER_PACKET:
		parent.users.RemoveUser();
		break;

	    case SHUTDOWN_PACKET:
		parent.errorDialog.bringUp("The server is being brought down for maintenance.",
			"Now saving your game and logging you off.",
			"Please try back again in a few minutes.");
		break;

	    case CLEAR_PACKET:
		parent.messages.ClearScreen();
		break;

	    case WRITE_LINE_PACKET:		
		parent.messages.PrintLine();
		break;

	    case BUTTONS_PACKET:
		parent.buttons.Question();
		break;

	    case FULL_BUTTONS_PACKET:
		parent.buttons.turn();
		break;

	    case STRING_DIALOG_PACKET:
		stringDialog.bringUp();
		break;

	    case COORDINATES_DIALOG_PACKET:
		coordinateDialog.bringUp();
		break;

	    case PLAYER_DIALOG_PACKET:
		playerDialog.bringUp();
		break;

	    case PASSWORD_DIALOG_PACKET:
		passwordDialog.bringUp();
		break;

            case SCOREBOARD_DIALOG_PACKET:
                scoreDialog = new scoreDlog(parent);
                break;

            case DIALOG_PACKET:
                stdDialog = new stdDlog(parent);
                break;

	    case CHAT_PACKET:
		parent.chat.PrintLine();
		break;

	    case ACTIVATE_CHAT_PACKET:
		parent.chat.activateChat();
		break;

	    case DEACTIVATE_CHAT_PACKET:
		parent.chat.deactivateChat();
		break;

/*
	    case PLAYER_INFO_PACKET:
		examinePlayer.bringUp();
		break;
*/
            case PLAYER_INFO_PACKET:
                examinePlayer = new examinDlog(parent);
                break;

            case CONNECTION_DETAIL_PACKET:
                detailDialog = new detailDlog(parent);
                break;


	    case ERROR_PACKET:
	        parent.errorDialog.bringUp("Server sent an error packet.",
			"Error: " + parent.readString(), "The game will now terminate.");
		lThreadQuit();
		break;

	    default:
		if (thePacket >= NAME_PACKET && thePacket <= VIRGIN_PACKET) {
		    parent.status.UpdateStatusPane(thePacket);
		}
		else {
	            parent.errorDialog.bringUp("Client received an unknown packet.",
		        "readString error: " + thePacket, "The game will now terminate.");
		    lThreadQuit();
		}
	    }
        }
    }

    public void lThreadQuit() {

            /* close the socket */
        if (parent.socket != null) {
            try {
                parent.socket.close();
            }
            catch (IOException e) {
                System.out.println("Error: " + e);
            }
            parent.socket = null;
        }

            /* remove the main windw */
        parent.f.hide();
        parent.f.dispose();

            /* destroy the listen thread */
        stop();
    }
}
