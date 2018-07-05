import java.awt.Color;
import java.awt.Font;

public interface constants {

    public static final String clientVersion= new String("1004");

	/* socket to client protocol */
    public static final int HANDSHAKE_PACKET = 2;
    public static final int CLOSE_CONNECTION_PACKET = 3;
    public static final int PING_PACKET = 4;
    public static final int ADD_PLAYER_PACKET = 5;
    public static final int REMOVE_PLAYER_PACKET = 6;
    public static final int SHUTDOWN_PACKET = 7;
    public static final int ERROR_PACKET = 8;

    public static final int CLEAR_PACKET = 10;
    public static final int WRITE_LINE_PACKET = 11;

    public static final int BUTTONS_PACKET = 20;
    public static final int FULL_BUTTONS_PACKET = 21;
    public static final int STRING_DIALOG_PACKET = 22;
    public static final int COORDINATES_DIALOG_PACKET = 23;
    public static final int PLAYER_DIALOG_PACKET = 24;
    public static final int PASSWORD_DIALOG_PACKET = 25;
    public static final int SCOREBOARD_DIALOG_PACKET = 26;
    public static final int DIALOG_PACKET = 27;

    public static final int CHAT_PACKET = 30;
    public static final int ACTIVATE_CHAT_PACKET = 31;
    public static final int DEACTIVATE_CHAT_PACKET = 32;
    public static final int PLAYER_INFO_PACKET = 33;
    public static final int CONNECTION_DETAIL_PACKET = 34;

    public static final int NAME_PACKET = 40;
    public static final int LOCATION_PACKET = 41;
    public static final int ENERGY_PACKET = 42;
    public static final int STRENGTH_PACKET = 43;
    public static final int SPEED_PACKET = 44;
    public static final int SHIELD_PACKET = 45;
    public static final int SWORD_PACKET = 46;
    public static final int QUICKSILVER_PACKET = 47;
    public static final int MANA_PACKET = 48;
    public static final int LEVEL_PACKET = 49;
    public static final int GOLD_PACKET = 50;
    public static final int GEMS_PACKET = 51;
    public static final int CLOAK_PACKET = 52;
    public static final int BLESSING_PACKET = 53;
    public static final int CROWN_PACKET = 54;
    public static final int PALANTIR_PACKET = 55;
    public static final int RING_PACKET = 56;
    public static final int VIRGIN_PACKET = 57;

    public static final String C_RESPONSE_PACKET = new String("1\0");
    public static final String C_CANCEL_PACKET = new String("2\0");
    public static final String C_PING_PACKET = new String("3\0");
    public static final String C_CHAT_PACKET = new String("4\0");
    public static final String C_EXAMINE_PACKET = new String("5\0");
    public static final String C_ERROR_PACKET = new String("6\0");
    public static final String C_SCOREBOARD_PACKET = new String("7\0");

	/* various colors */
/*
    public static final Color foregroundColor = new Color(10, 70, 17);
    public static final Color backgroundColor = new Color(240, 236, 199);
    public static final Color highlightColor = new Color(232, 220, 183);
*/
    public static final Color foregroundColor = new Color(000, 51, 000);
    public static final Color backgroundColor = new Color(255, 255, 204);
    public static final Color highlightColor = new Color(204, 204, 153);

	/* i/o identifiers */
    public static final int NO_REQUEST = 0;
    public static final int BUTTONS = 1;
    public static final int STRING_DLOG = 2;
    public static final int COORD_DLOG = 3;
    public static final int PLAYER_DLOG = 4;

	/* general text */
    public static final String OK_LABEL = new String("OK");
    public static final String CANCEL_LABEL = new String("Cancel");

	/* fonts */
    public static final Font MsgFont = new Font("TimesRoman", Font.PLAIN, 12);
    public static final Font ChatFont = new Font("TimesRoman", Font.PLAIN, 12);
    public static final Font ButtonFont = new Font("TimesRoman", Font.PLAIN, 9);
    public static final Font UserFont = new Font("TimesRoman", Font.PLAIN, 12);
    public static final Font CompassFont = new Font("Helvetica", Font.BOLD, 12);
}
