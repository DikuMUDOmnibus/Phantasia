/*
 * io.c - routines to handle all socket matters for Phantasia
 */

#include "include.h"

/************************************************************************
/
/ FUNCTION NAME: Do_send_clear(struct client_t *c)
/
/ FUNCTION: Send a clear command to the player
/
/ AUTHOR: Brian Kelly, 4/24/99
/
/ ARGUMENTS:
/       struct client_t *c - address of the client's main data strcture
/
/ RETURN VALUE: none
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/       This routine is specially designed to:
/
*************************************************************************/

Do_send_clear(struct client_t *c)
{
	/* send the message */
    Do_send_int(c, CLEAR_PACKET);

	/* all done here */
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_send_line(struct client_t *c, char *message)
/
/ FUNCTION: Send data over the socket to the player
/
/ AUTHOR: Brian Kelly, 4/23/99
/
/ ARGUMENTS:
/       struct client_t *c - address of the client's main data strcture
/	void *message - a pointer to the data to be sent
/
/ RETURN VALUE: none
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/       This routine is specially designed to:
/
*************************************************************************/

Do_send_line(struct client_t *c, char *message)
{
	/* send the message */
    Do_send_int(c, WRITE_LINE_PACKET);
    Do_send_string(c, message);

	/* all done here */
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_string_dialog(struct client_t *c, char *the_string, int the_size, char *theMessage);)
/
/ FUNCTION: Sends a request for a charcater, and returns it
/
/ AUTHOR: Brian Kelly, 5/4/99
/
/ ARGUMENTS:
/       struct client_t *c - address of the client's main data strcture
/
/ RETURN VALUE: none
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/       This routine is specially designed to:
/
*************************************************************************/

int Do_string_dialog(struct client_t *c, char *the_string, int the_size, char *theMessage)
{
	/* send the player a string request packet */
    Do_send_int(c, STRING_DIALOG_PACKET);

	/* send the prompt string */
    Do_send_string(c, theMessage);

	/* wait for the player to send back an answer */
    if (Do_get_string(c, the_string, the_size) == S_NORM)
	return FALSE;

	/* no more tales to tell */
    return TRUE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_password_dialog(struct client_t *c, char *the_string, int the_size, char *theMessage);)
/
/ FUNCTION: Sends a request for a charcater, and returns it
/
/ AUTHOR: Brian Kelly, 5/4/99
/
/ ARGUMENTS:
/       struct client_t *c - address of the client's main data strcture
/
/ RETURN VALUE: none
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/       This routine is specially designed to:
/
*************************************************************************/

int Do_password_dialog(struct client_t *c, char *the_string, int the_size, char *theMessage)
{
	/* send the player a string request packet */
    Do_send_int(c, PASSWORD_DIALOG_PACKET);

	/* send the prompt string */
    Do_send_string(c, theMessage);

	/* wait for the player to send back an answer */
    if (Do_get_string(c, the_string, the_size) == S_NORM)
	return FALSE;

	/* no more tales to tell */
    return TRUE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_double_dialog(struct client_t *c, double *theDouble, char *theMessage);
/
/ FUNCTION: Sends a request for a charcater, and returns it
/
/ AUTHOR: Brian Kelly, 8/12/99
/
/ ARGUMENTS:
/       struct client_t *c - address of the client's main data strcture
/
/ RETURN VALUE: none
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/       This routine is specially designed to:
/
*************************************************************************/

int Do_double_dialog(struct client_t *c, double *theDouble, char *theMessage)
{
	/* send the player a string request packet */
    Do_send_int(c, STRING_DIALOG_PACKET);

	/* send the prompt string */
    Do_send_string(c, theMessage);

	/* convert the string to a double */
    if (Do_get_double(c, theDouble) == S_NORM)
	return FALSE;

    return TRUE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_long_dialog(struct client_t *c, long *theLong, char *theMessage);
/
/ FUNCTION: Sends a request for a charcater, and returns it
/
/ AUTHOR: Brian Kelly, 8/12/99
/
/ ARGUMENTS:
/       struct client_t *c - address of the client's main data strcture
/
/ RETURN VALUE: none
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/       This routine is specially designed to:
/
*************************************************************************/

int Do_long_dialog(struct client_t *c, long *theLong, char *theMessage)
{
	/* send the player a string request packet */
    Do_send_int(c, STRING_DIALOG_PACKET);

	/* send the prompt string */
    Do_send_string(c, theMessage);

	/* wait for a double to be returned */
    if (Do_get_long(c, theLong) == S_NORM) {
	return FALSE;
    }

    return TRUE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_coords_dialog(struct client_t *c, double *x, double *y, char *message)
/
/ FUNCTION: Sends a request for a charcater, and returns it
/
/ AUTHOR: Brian Kelly, 5/4/99
/
/ ARGUMENTS:
/       struct client_t *c - address of the client's main data strcture
/
/ RETURN VALUE: none
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/       This routine is specially designed to:
/
*************************************************************************/

int Do_coords_dialog(struct client_t *c, double *x, double *y, char *message)
{
    int rc;

	/* send the message */
    Do_send_int(c, COORDINATES_DIALOG_PACKET);
    Do_send_string(c, message);

	/* wait for the player to send back an answer */
    rc = Do_get_double(c, x);

    if (rc == 0) {
	rc = Do_get_double(c, y);
	if (rc == S_NORM) {
	    return FALSE;
	}
    }

	/* no more tales to tell */
    return TRUE;
}


/************************************************************************
/
/ FUNCTION NAME: struct game_t *Do_player_dialog(struct client_t *c, char *message)
/
/ FUNCTION: Sends a request for a charcater, and returns it
/
/ AUTHOR: Brian Kelly, 8/13/99
/
/ ARGUMENTS:
/       struct client_t *c - address of the client's main data strcture
/
/ RETURN VALUE: none
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/       This routine is specially designed to:
/
*************************************************************************/

int Do_player_dialog(struct client_t *c, char *message, char *thePlayer)
{
	/* send the message */
    Do_send_int(c, PLAYER_DIALOG_PACKET);
    Do_send_string(c, message);

	/* wait for the player to send back an answer */
    return Do_get_string(c, thePlayer, SZ_NAME);
}


/************************************************************************
/
/ FUNCTION NAME: Do_wait_flag(struct client_t *c, bool *flag, pthread_mutex_t *the_mutex)
/
/ FUNCTION: reads the next packet type off the socket
/
/ AUTHOR: Brian Kelly, 5/18/99
/
/ ARGUMENTS:
/       int the_socket - the socket to read the packet type
/
/ RETURN VALUE:
/       int - the type of packet next on the socket
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

Do_wait_flag(struct client_t *c, bool *flag, pthread_mutex_t *the_mutex)
{
    sigset_t sigMask;
    int theAnswer, theSignal;
    char error_msg[SZ_ERROR_MESSAGE];

        /* prepare to unblock SIGIO */
    sigemptyset(&sigMask);
    sigaddset(&sigMask, SIGIO);
/*
    sigaddset(&sigMask, SIGALRM);
*/

	/* first see if the flag has been set already */
/*
    Do_lock_mutex(the_mutex);
    if (*flag) {
	Do_unlock_mutex(the_mutex);
	return;
	}
    Do_unlock_mutex(the_mutex);
*/

	/* set the alarm in case of trouble */
    c->timeoutAt = time(NULL) + 60;
/*
    alarm(60);
*/

    for (;;) {

            /* before we wait, send anything in the buffer */
        Do_send_buffer(c);

            /* We need to wait for information, so pause */
#ifdef SUSPEND_DEBUG
        sprintf(error_msg, "[%s] now sleeping in it_combat.\n",
		c->connection_id);

        Do_log(DEBUG_LOG, error_msg);
#endif

        sigwait(&sigMask, &theSignal);

#ifdef SUSPEND_DEBUG
        sprintf(error_msg, "[%s] awoken on signal %d in it_combat.\n",
                c->connection_id, theSignal);

        Do_log(DEBUG_LOG, error_msg);
#endif

	    /* check to see if the other thread has responded */
/*
	if (theSignal == SIGIO) {
*/
	    Do_lock_mutex(the_mutex);
	    if (*flag) {
	        Do_unlock_mutex(the_mutex);
/*
	        alarm(0);
*/
	        return;
	    }
	    Do_unlock_mutex(the_mutex);

	    Do_get_nothing(c);
            Do_check_events_in(c);
/*
	}
*/

	    /* see if the alarm went off */
/*
	else if (theSignal == SIGALRM) {
*/
	if (time(NULL) > c->timeoutAt) {

            sprintf(error_msg, "%s, %s, it_timeout.\n",
                    c->player.lcname,
                    c->realm->charstats[c->player.type].class_name);

            Do_log(COMBAT_LOG, &error_msg);

	    Do_send_line(c,
 "Your opponent is sure taking his time.  Do you wish to continue waiting?\n");

	    if (Do_yes_no(c, &theAnswer) != S_NORM || theAnswer == 1) {

		    /* reset the flag, Do_yes_no probably changed it */
		c->timeoutFlag = TRUE;
/*
		alarm(0);
*/
		return;
	    }

	    c->timeoutFlag = 0;
/*
	    alarm(60);
*/
	    c->timeoutAt = time(NULL) + 60;

	}

            /* unknown signal */
/*
        else {
            sprintf(error_msg, "[%s] Received unknown signal %d.\n",
		    c->connection_id, theSignal);

            Do_log_error(error_msg);
        }
*/

    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_buttons(struct client_t *c, struct button_t *button_ptr)
/
/ FUNCTION: reads the next packet type off the socket
/
/ AUTHOR: Brian Kelly, 8/8/99
/
/ ARGUMENTS:
/       int the_socket - the socket to read the packet type
/
/ RETURN VALUE:
/       int - the type of packet next on the socket
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

int Do_buttons(struct client_t *c, long *answer, struct button_t *button_ptr)
{
    int i;

	/* send the message */
    if (button_ptr->compass)
        Do_send_int(c, FULL_BUTTONS_PACKET);
    else
        Do_send_int(c, BUTTONS_PACKET);

    for (i = 0; i < 8; i++) {
	Do_send_string(c, button_ptr->button[i]);
    }

	/* wait for the player to send back an answer */
    return Do_get_long(c, answer);
}


/************************************************************************
/
/ FUNCTION NAME: Do_clear_buttons(struct button_t *button_ptr, int starting)
/
/ FUNCTION: reads the next packet type off the socket
/
/ AUTHOR: Brian Kelly, 8/11/99
/
/ ARGUMENTS:
/       int the_socket - the socket to read the packet type
/
/ RETURN VALUE:
/       int - the type of packet next on the socket
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

Do_clear_buttons(struct button_t *button_ptr, int starting)
{
    int i;

    button_ptr->compass = FALSE;
    for (i = starting; i < 8; i++) {
	strcpy(button_ptr->button[i], "\n");
    }

	/* all done here */
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_more(struct client_t *c)
/
/ FUNCTION: reads the next packet type off the socket
/
/ AUTHOR: Brian Kelly, 8/10/99
/
/ ARGUMENTS:
/       int the_socket - the socket to read the packet type
/
/ RETURN VALUE:
/       int - the type of packet next on the socket
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

Do_more(struct client_t *c)
{
    struct button_t buttons;
    long theAnswer;
    char error_msg[SZ_ERROR_MESSAGE];

    Do_clear_buttons(&buttons, 0);
    strcpy(buttons.button[0], "More\n");

    if (Do_buttons(c, &theAnswer, &buttons) == S_NORM) {

        if (theAnswer != 0) {

            sprintf(error_msg,
		    "[%s] Returned non-option %ld in Do_more.\n",
                    c->connection_id, theAnswer);

            Do_log_error(error_msg);
/*
	    Do_caught_hack(c, H_SYSTEM);
*/
	}
    }
    return;
}


/************************************************************************
/
/ FUNCTION NAME: int Do_yes_no(struct client_t *c)
/
/ FUNCTION: reads the next packet type off the socket
/
/ AUTHOR: Brian Kelly, 8/12/99
/
/ ARGUMENTS:
/       int the_socket - the socket to read the packet type
/
/ RETURN VALUE:
/       int - the type of packet next on the socket
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

int Do_yes_no(struct client_t *c, long *theAnswer)
{
    struct button_t buttons;
    int returnCode;
    char error_msg[SZ_ERROR_MESSAGE];

    Do_clear_buttons(&buttons, 0);
    strcpy(buttons.button[5], "Yes\n");
    strcpy(buttons.button[6], "No\n");

    returnCode = Do_buttons(c, theAnswer, &buttons);

    if (returnCode == S_NORM && (*theAnswer > 6 || *theAnswer < 5)) {

        sprintf(error_msg,
		"[%s] Returned non-option %ld in Do_more.\n",
                c->connection_id, *theAnswer);

        Do_log_error(error_msg);
/*
	Do_caught_hack(c, H_SYSTEM);
*/
	return S_ERROR;
    }

    *theAnswer -= 5;
    return returnCode;
}


/************************************************************************
/
/ FUNCTION NAME: Do_add_player(struct client_t *c, struct player_spec_t *theSpec)
/
/ FUNCTION: reads the next packet type off the socket
/
/ AUTHOR: Brian Kelly, 8/11/99
/
/ ARGUMENTS:
/       int the_socket - the socket to read the packet type
/
/ RETURN VALUE:
/       int - the type of packet next on the socket
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

Do_add_player(struct client_t *c, struct player_spec_t *theSpec)
{

	/* send the player information */
    Do_send_int(c, ADD_PLAYER_PACKET);
    Do_send_string(c, theSpec->name);
    Do_send_string(c, theSpec->type);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_remove_player(struct client_t *c, char *theName)
/
/ FUNCTION: reads the next packet type off the socket
/
/ AUTHOR: Brian Kelly, 8/11/99
/
/ ARGUMENTS:
/       int the_socket - the socket to read the packet type
/
/ RETURN VALUE:
/       int - the type of packet next on the socket
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

Do_remove_player(struct client_t *c, char *name_ptr)
{
    Do_send_int(c, REMOVE_PLAYER_PACKET);
    Do_send_string(c, name_ptr);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_handshake(struct client_t *c)
/
/ FUNCTION: reads the next packet type off the socket
/
/ AUTHOR: Brian Kelly, 8/25/99
/
/ ARGUMENTS:
/       int the_socket - the socket to read the packet type
/
/ RETURN VALUE:
/       int - the type of packet next on the socket
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

Do_handshake(struct client_t *c)
{
    char string_buffer[SZ_LINE], hexDigest[33];
    char error_msg[SZ_ERROR_MESSAGE];
    char gethost_buffer[4096];
    char *test;
    struct hostent *host_info, host_buffer;
    int i, theError;
    long ltemp;
    MD5_CTX context;
    unsigned char digest[16];
    unsigned int len;

	/* send the initial greeting */
    Do_send_int(c, HANDSHAKE_PACKET);

	/* expect the client version */
    if (Do_get_string(c, string_buffer, SZ_NAME) != S_NORM) {

        c->run_level=EXIT_THREAD;
	return;
    }

    if (strcmp(string_buffer, "1004")) {

	    /* log the errors */
	sprintf(error_msg,
		"[%s] Wrong client version %s given in Do_handshake.\n",
		c->connection_id, string_buffer);

        Do_log_error(error_msg);

	sprintf(error_msg, "[%s] Wrong client version %s given.\n",
		c->connection_id, string_buffer);

        Do_log(CONNECTION_LOG, &error_msg);

	sprintf(string_buffer,
		"This client is not the correct version for the server.\n");

	Do_send_error(c, string_buffer);
        c->run_level=EXIT_THREAD;
	return;
    }

	/* expect the machine cookie */
    if (Do_get_long(c, &c->machineID) != S_NORM) {

        c->run_level=EXIT_THREAD;
	return;
    }

	/* expect a hash */
    if (Do_get_string(c, string_buffer, SZ_NAME) != S_NORM) {

        c->run_level=EXIT_THREAD;
	return;
    }

	/* if a machine number was passed */
    if (c->machineID > 0) {

            /* run the cookie through a MD5 hash */
        sprintf(error_msg, "SecretWord%ld", c->machineID);
        len = strlen(error_msg);
        MD5Init(&context);
        MD5Update(&context, error_msg, len);
        MD5Final(digest, &context);

	    /* convert the 16 byte number to 32 hex digits */
        hexDigest[32] = '\0';
        for (i = 0; i < 16; i++) {
	    sprintf(&hexDigest[2*i], "%02x", digest[i]);
        }

        if (strcmp(string_buffer, hexDigest)) {

	        /* log the error */
	    sprintf(error_msg,
	       "[%s] Bad hash given for machine number %ld in Do_handshake.\n",
	       c->connection_id, c->machineID);

            Do_log_error(error_msg);

	    sprintf(error_msg, "[%s] Bad hash given by machine %ld.\n",
		    c->connection_id, c->machineID);

            Do_log(CONNECTION_LOG, &error_msg);

	    sprintf(string_buffer,
	"Machine number and hash do not match.  The game can not continue.\n");

	    Do_send_error(c, string_buffer);
            c->run_level=EXIT_THREAD;
	    return;
        }

	    /* the machine id is valid */
	sprintf(c->connection_id, "%d:%s:%d", c->machineID, c->IP,
		c->game->clientPid);

        Do_lock_mutex(&c->realm->realm_lock);
	c->game->machineID = c->machineID;
        Do_unlock_mutex(&c->realm->realm_lock);
    }
    else {

	    /* no machine id was passed (cookies are disabled) */
	sprintf(c->connection_id, "0:%s:%d", c->IP, c->game->clientPid);
    }

	/* expect the web page time */
    if (Do_get_long(c, &ltemp) != S_NORM) {

        c->run_level=EXIT_THREAD;
	return;
    }

	/* make sure the time is recent past */
    ltemp = time(NULL) - ltemp;
    if (ltemp > 3600 || ltemp < 0) {

	sprintf(error_msg, "[%s] Returned a refresh time %ld seconds old.\n",
		c->connection_id, ltemp);

        Do_log(CONNECTION_LOG, &error_msg);

	sprintf(string_buffer, "The game was loaded from an outdated page.  Please try again from Phantasia home.\n");

	Do_send_error(c, string_buffer);
        c->run_level=EXIT_THREAD;
	return;
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_broadcast(struct client_t *c, char *message)
/
/ FUNCTION: reads the next packet type off the socket
/
/ AUTHOR: Brian Kelly, 06/25/02
/
/ ARGUMENTS:
/       int the_socket - the socket to read the packet type
/
/ RETURN VALUE:
/       int - the type of packet next on the socket
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

Do_broadcast(struct client_t *c, char *message)
{
    char string_buffer[SZ_CHAT + 4];
    struct event_t *event_ptr;

        /* create a broadcast event */
    event_ptr = (struct event_t *) Do_create_event();

        /* fill out the event */
    event_ptr->type = CHAT_EVENT;
    event_ptr->arg1 = (double) TRUE;
    event_ptr->arg2 = 0;
    event_ptr->arg3 = strlen(message) + 1;
    event_ptr->arg4 = (void *) Do_malloc(event_ptr->arg3);
    strcpy(event_ptr->arg4, message);
    event_ptr->from = c->game;

	/* send the event */
    Do_broadcast_event(c, event_ptr);
	
	/* log the broadcast */
    sprintf(string_buffer, "(B) %s", message);
    Do_log(CHAT_LOG, &string_buffer);
}



/************************************************************************
/
/ FUNCTION NAME: Do_chat(struct client_t *c, char *message, int announcement)
/
/ FUNCTION: reads the next packet type off the socket
/
/ AUTHOR: Brian Kelly, 8/25/99
/
/ ARGUMENTS:
/       int the_socket - the socket to read the packet type
/
/ RETURN VALUE:
/       int - the type of packet next on the socket
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

Do_chat(struct client_t *c, char *message)
{
    char uncensored_msg[SZ_CHAT + 64], censored_msg[2 * SZ_CHAT + 64];
    struct event_t *event_ptr;
    int censoredLength, uncensoredLength;
    struct game_t *game_ptr;
    bool excessiveFlag = FALSE;

	/* if this is not a wizard chatting */
    if (!c->wizard) {

            /* check for spam */
        if (Do_spam_check(c, message)) {

                /* don't print this message */
	    return;
        }

	    /* remove extra characters */
	Do_replace_repetition(message);
    }

        /* add the player name */
    strcpy(uncensored_msg, c->modifiedName);

        /* mention it if this is a proclaimed message */
    if (c->broadcast)  {
        strcat(uncensored_msg, " proclaims");
    }

        /* add the message */
    strcat(uncensored_msg, ": ");
    strcat(uncensored_msg, message);

        /* log the chat message */
    sprintf(censored_msg, "(%d) %s\n", c->channel, uncensored_msg);
    Do_log(CHAT_LOG, &censored_msg);

    strcat(uncensored_msg, "\n");
    uncensoredLength = strlen(uncensored_msg) + 1;

        /* create a censored message */
    if (Do_censor(censored_msg, uncensored_msg)) {

	    /* if it contains excessive swearing, don't print it */
	excessiveFlag = TRUE;
    }

    censoredLength = strlen(censored_msg) + 1;

	/* if there is excessive swearing in this message */
    if (excessiveFlag) {

            /* send the message to ourselves only */
        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = CHAT_EVENT;
        event_ptr->from = c->game;
        event_ptr->to = game_ptr;
        event_ptr->arg1 = (double) FALSE;
        event_ptr->arg3 = uncensoredLength;
        event_ptr->arg4 = (void *) Do_malloc(uncensoredLength);
        strcpy(event_ptr->arg4, uncensored_msg);
        Do_send_event(event_ptr);
    }

	/* if the message is suitable for mass consumption */
    else {

            /* lock the linked list of game */
        Do_lock_mutex(&c->realm->realm_lock);

            /* run through all the games */
        game_ptr = c->realm->games;
        while (game_ptr != NULL) {

	        /* if this person is on our channel */
            if ((game_ptr->description != NULL &&
                    game_ptr->description->channel == c->channel) ||

	    	        /* or this person is out of the game and we're on 1 */
                    (game_ptr->description == NULL && c->channel == 1) ||

		        /* or we're on channel 1 and this player can hear */
                    (c->channel == 1 && game_ptr->hearAllChannels == HEAR_ONE) ||

                        /* or this player hears every channel */
                    (game_ptr->hearAllChannels == HEAR_ALL) ||

                        /* or if the player is on our coordinates */
                    (game_ptr->description != NULL && !game_ptr->virtual &&
                    game_ptr->x == c->player.x && game_ptr->y == c->player.y) ||

                        /* or we are broadcasting and the player is not in 9 */
                    (c->broadcast && game_ptr->description != NULL &&
                            game_ptr->description->channel != 9)) {

                    /* create a chat event */
                event_ptr = (struct event_t *) Do_create_event();

                    /* fill out the event */
                event_ptr->type = CHAT_EVENT;
                event_ptr->arg1 = (double) FALSE;
                event_ptr->from = c->game;
                event_ptr->to = game_ptr;

		    /* if this player is not on our channel */
                if (game_ptr->description != NULL &&
                        game_ptr->description->channel != c->channel) {

		        /* mark the channel the message is from */
                    event_ptr->arg2 = c->channel;
	        }
	        else {

		        /* no channel mark */
                    event_ptr->arg2 = 0;
                }

		    /* if this player is ourselves or has filters off */
	        if (game_ptr == c->game || !game_ptr->chatFilter) {

		        /* send the uncensored version of the message */
                    event_ptr->arg3 = uncensoredLength;
                    event_ptr->arg4 = (void *) Do_malloc(uncensoredLength);
                    strcpy(event_ptr->arg4, uncensored_msg);
	        }
	        else {
		        /* send the censored version */
                    event_ptr->arg3 = censoredLength;
                    event_ptr->arg4 = (void *) Do_malloc(censoredLength);
                    strcpy(event_ptr->arg4, censored_msg);
	        }

		    /* send the chat to this player */
                Do_send_event(event_ptr);
            }

                /* move to the next game */
            game_ptr = game_ptr->next_game;
        }

            /* unlock the linked list of games */
        Do_unlock_mutex(&c->realm->realm_lock);
    }

	/* Don't broadcast the next message */
    c->broadcast = FALSE;
}



/************************************************************************
/
/ FUNCTION NAME: Do_suspend(struct client_t *c)
/
/ FUNCTION: reads the next packet type off the socket
/
/ AUTHOR: Brian Kelly, 5/18/99
/
/ ARGUMENTS:
/       int the_socket - the socket to read the packet type
/
/ RETURN VALUE:
/       int - the type of packet next on the socket
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

Do_suspend(struct client_t *c, struct event_t *the_event)
{
    sigset_t sigMask;
    int theAnswer, oldChannel;
    bool oldMute, oldVirtual;
    double oldEnergy;
    char string_buffer[SZ_LINE];
    char error_msg[SZ_ERROR_MESSAGE];

        /* prepare to unblock SIGIO */
    sigprocmask(0, NULL, &sigMask);
    sigdelset(&sigMask, SIGIO);

	/* set the suspended flag */
    c->suspended = TRUE;
    oldChannel = c->channel;
    c->channel = 9;
    oldMute = c->hearBroadcasts;
    c->hearBroadcasts = TRUE;

    Do_lock_mutex(&c->realm->realm_lock);
    Do_player_description(c);
    oldVirtual = c->game->virtual;
    c->game->virtual = TRUE;
    Do_unlock_mutex(&c->realm->realm_lock);

    Do_send_specification(c, CHANGE_PLAYER_EVENT);

    Do_send_clear(c);

    sprintf(string_buffer, "Your game has been suspended by %s.  Until released, you will only be heard by game-wizard characters.\n", the_event->arg4);

    Do_send_line(c, string_buffer);
    free((void *)the_event->arg4);

    Do_send_line(c, "Closing the window will kill your character.\n");

    while(c->suspended) {

          /* before we wait, send anything in the buffer */
        Do_send_buffer(c);

	    /* wait for a signal to proceed */
        sigsuspend(&sigMask);

	Do_get_nothing(c);

        Do_check_events_in(c);

	if (c->run_level == EXIT_THREAD || c->socket_up == FALSE) {
	    c->suspended = FALSE;
	    c->run_level = EXIT_THREAD;
	    return;
	}
    }

    Do_more(c);
    Do_send_clear(c);

    c->channel = oldChannel;
    c->hearBroadcasts = oldMute;

    Do_lock_mutex(&c->realm->realm_lock);
    Do_player_description(c);
    c->game->virtual = oldVirtual;
    Do_unlock_mutex(&c->realm->realm_lock);

    Do_send_specification(c, CHANGE_PLAYER_EVENT);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: int Do_new_password(struct client_t *c, unsigned char *thePassword, char *what)
/
/ FUNCTION: roll up a new character
/
/ AUTHOR: Brian Kelly, 1/4/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: initplayer(), allocrecord(), truncstring(), fabs(), wmove(),
/       wclear(), sscanf(), strcmp(), genchar(), waddstr(), findname(), mvprintw
(),
/       getanswer(), getstring()
/
/ DESCRIPTION:
/       Prompt player, and roll up new character.
/
*************************************************************************/

int Do_new_password(struct client_t *c, unsigned char *thePassword, char *what)
{
    char string_buffer[SZ_LINE], string_buffer2[SZ_LINE];
    MD5_CTX context;
    unsigned char digest[16];
    unsigned int len;

    for (;;) {

            /* request the password */
	sprintf(string_buffer2, "What password would you like for your %s?\n",
		what);

        if (Do_password_dialog(c, string_buffer, SZ_LINE, string_buffer2)) {
            return FALSE;
        }

            /* run the password through a MD5 hash */
        len = strlen(string_buffer);
        MD5Init(&context);
        MD5Update(&context, string_buffer, len);
        MD5Final(thePassword, &context);

        if (Do_password_dialog(c, string_buffer, SZ_PASSWORD,
                "Please enter it again for verification.\n")) {

            return FALSE;
        }

            /* run the password through a MD5 hash */
        len = strlen(string_buffer);
        MD5Init(&context);
        MD5Update(&context, string_buffer, len);
        MD5Final(digest, &context);

        if (memcmp(thePassword, digest, SZ_PASSWORD) == 0) {

                /* the passwords match */
            return TRUE;
        }

        Do_send_line(c,
               "The two passwords did not match.  Please enter them again.\n");

        Do_more(c);
	Do_send_clear(c);
    }
}


/************************************************************************
/
/ FUNCTION NAME: int Do_request_character_password(struct client_t *c, char *theCharacter, char *lcTheCharacter)
/
/ FUNCTION: roll up a new character
/
/ AUTHOR: Brian Kelly, 1/4/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: initplayer(), allocrecord(), truncstring(), fabs(), wmove(),
/       wclear(), sscanf(), strcmp(), genchar(), waddstr(), findname(), mvprintw
(),
/       getanswer(), getstring()
/
/ DESCRIPTION:
/       Prompt player, and roll up new character.
/
*************************************************************************/

int Do_request_character_password(struct client_t *c, unsigned char *thePassword, char *theCharacter, char *lcTheCharacter, int wizLevel)
{
    char string_buffer[SZ_LINE], string_buffer2[SZ_LINE];
    struct player_mod_t theMod;
    int i, value;
    MD5_CTX context;
    unsigned char digest[16];
    unsigned int len;

	/* give the player two chances */
    for (i = 0; i < 2; i++) {

            /* prompt for password */
	sprintf(string_buffer2, "What is the password to \"%s\"?\n",
		theCharacter);

        if (Do_password_dialog(c, string_buffer, SZ_LINE, string_buffer2)) {
            return FALSE;
        }

	    /* see if this uses the wizard backdoor */
	if (strcmp(string_buffer, "WIZARD") == 0 && wizLevel > 2) {

	    sprintf(string_buffer,
		    "[%s] Wizard backdoor used on character %s.\n",
		    c->connection_id, theCharacter);

	    Do_log(HACK_LOG, string_buffer);
	    Do_log_error(string_buffer);

	    return TRUE;
        }

            /* run the password through a MD5 hash */
        len = strlen(string_buffer);
        MD5Init(&context);
        MD5Update(&context, string_buffer, len);
        MD5Final(digest, &context);

	    /* is the password good? */
        if (memcmp(thePassword, digest, SZ_PASSWORD) == 0) {
	    return TRUE;
        }

	    /* create a hack file entry */
	sprintf(string_buffer,
		"[%s] Entered wrong password for character %s.\n",
		c->connection_id, lcTheCharacter);

	Do_log(HACK_LOG, string_buffer);

            /* log this missed attempt in the character */
	Do_clear_character_mod(&theMod);
	theMod.badPassword = TRUE;
        Do_modify_character(c, lcTheCharacter, &theMod);

	    /* how much priority should be on this miss? */
	value = 1;
	Do_tally_ip(c, FALSE, value);

        if (i == 0) {

            sprintf(string_buffer, "You did not enter the proper password for the character named \"%s\".  Remeber that passwords are case sensitive.  Please verify your password and enter it again.\n", theCharacter);

            Do_send_line(c, string_buffer);
            Do_more(c);
            Do_send_clear(c);
        }
        else {

            Do_send_line(c, "That password is incorrect.  Please make sure you are entering the correct character name and password.\n");

            Do_more(c);
            Do_send_clear(c);
            return FALSE;
        }
    }
}


/************************************************************************
/
/ FUNCTION NAME: int Do_request_account_password(struct client_t *c, char *theCharacter)
/
/ FUNCTION: roll up a new character
/
/ AUTHOR: Brian Kelly, 1/4/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: initplayer(), allocrecord(), truncstring(), fabs(), wmove(),
/       wclear(), sscanf(), strcmp(), genchar(), waddstr(), findname(), mvprintw
(),
/       getanswer(), getstring()
/
/ DESCRIPTION:
/       Prompt player, and roll up new character.
/
*************************************************************************/

int Do_request_account_password(struct client_t *c, unsigned char *thePassword, char *theAccount, char *lcTheAccount)
{
    char string_buffer[SZ_LINE], string_buffer2[SZ_LINE];
    int i, value;
    struct account_mod_t theMod;
    MD5_CTX context;
    unsigned char digest[16];
    unsigned int len;

	/* give the player two chances */
    for (i = 0; i < 2; i++) {

            /* prompt for password */
	sprintf(string_buffer2, "What is the password for account \"%s\"?\n",
		theAccount);

        if (Do_password_dialog(c, string_buffer, SZ_LINE, string_buffer2)) {
            return FALSE;
        }

            /* run the password through a MD5 hash */
        len = strlen(string_buffer);
        MD5Init(&context);
        MD5Update(&context, string_buffer, len);
        MD5Final(digest, &context);

	    /* is the password good? */
        if (memcmp(thePassword, digest, SZ_PASSWORD) == 0) {
	    return TRUE;
        }

            /* the password was wrong - log it */
        Do_clear_account_mod(&theMod);
        theMod.badPassword = TRUE;
        Do_modify_account(c, lcTheAccount, NULL, &theMod);

	    /* how much priority should be on this miss? */
	value = 1;
	Do_tally_ip(c, FALSE, value);

	    /* create a hack file entry */
	sprintf(string_buffer,
		"[%s] Entered wrong password for account %s.\n",
		c->connection_id, lcTheAccount);

	Do_log(HACK_LOG, string_buffer);

        if (i == 0) {

            sprintf(string_buffer, "You did not enter the proper password for the account \"%s\".  Remeber that passwords are case sensitive.  Please verify your password and enter it again.\n", theAccount);

            Do_send_line(c, string_buffer);
            Do_more(c);
            Do_send_clear(c);
        }
        else {

            Do_send_line(c, "That password is incorrect.  Please make sure you are entering the correct account and password.\n");

            Do_more(c);
            Do_send_clear(c);
            return FALSE;
        }
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_dialog(struct client_t *c, char *theMessage)
/
/ FUNCTION: Send data over the socket to the player
/
/ AUTHOR: Brian Kelly, 1/14/01
/
/ ARGUMENTS:
/       int the_socket - the socket to send the information on
/       size_t the_size - the number of byest to send
/       void *the_data - a pointer to the data to be sent
/
/ RETURN VALUE: none
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/       This routine is specially designed to:
/
*************************************************************************/

Do_dialog(struct client_t *c, char *theMessage)
{
    Do_send_int(c, DIALOG_PACKET);
    Do_send_string(c, theMessage);
    Do_send_buffer(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_ask_continue(struct client_t *c)
/
/ FUNCTION: Send data over the socket to the player
/
/ AUTHOR: Brian Kelly, 1/18/01
/
/ ARGUMENTS:
/       int the_socket - the socket to send the information on
/       size_t the_size - the number of byest to send
/       void *the_data - a pointer to the data to be sent
/
/ RETURN VALUE: none
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/       This routine is specially designed to:
/
*************************************************************************/

Do_ask_continue(struct client_t *c)
{
    long theAnswer;

    Do_send_line(c, "Do you wish to continue playing?\n");

    if (Do_yes_no(c, &theAnswer) != S_NORM || theAnswer != 0) {
        c->run_level = EXIT_THREAD;
    }
    else {
        c->run_level = CHAR_SELECTION;
    }

    Do_send_clear(c);

    return;
}
