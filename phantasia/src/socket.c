/*
 * socket.c - routines to handle all socket matters for Phantasia
 */

#include "include.h"

extern void *Do_start_thread(void *c);
extern server_hook;
extern randData;

/************************************************************************
/
/ FUNCTION NAME: Do_initialize_socket(struct server_t *server)
/
/ FUNCTION: To initialize the program's socket
/
/ AUTHOR: Brian Kelly, 4/12/99
/
/ ARGUMENTS:
/       struct server_t *s - address of the server's main data strcture
/
/ RETURN VALUE: 
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/       This routine is specially designed to:
/
/           - strip non-printing characters (unless Wizard)
/           - echo, if desired
/           - redraw the screen if CH_REDRAW is entered
/           - read in only 'mx - 1' characters or less characters
/           - nul-terminate string, and throw away newline
/
/       'mx' is assumed to be at least 2.
/
*************************************************************************/

int Do_init_server_socket()
{
    struct sockaddr_in bind_address;
    char error_msg[SZ_ERROR_MESSAGE];
    int the_socket, error, on=1;

	/* create a socket */
    errno = 0;
    if ((the_socket=socket(AF_INET, SOCK_STREAM, 0)) == -1) {

        sprintf(error_msg,
	   "[0.0.0.0:?] Socket creation failed in Do_init_server_socket: %s\n",
	   strerror(errno));

        Do_log_error(error_msg);
        exit(SOCKET_CREATE_ERROR);
    }

    error = setsockopt(the_socket, SOL_SOCKET, SO_REUSEADDR,
            (char *) &on, sizeof(on));

    if (error != 0) {

        sprintf(error_msg, "[0.0.0.0:?] setsockopt failed with error code of %d in Do_init_server_socket.\n", error);

        Do_log_error(error_msg);
        exit(SOCKET_CREATE_ERROR);
    }

	/* set up the bind address */
    bind_address.sin_family = AF_INET;
    bind_address.sin_addr.s_addr = INADDR_ANY;
    bind_address.sin_port = PHANTASIA_PORT;

	/* bind to that socket */
    error = bind(the_socket, (struct sockaddr *) &bind_address,
	    sizeof(bind_address));

    if (error != 0) {

        sprintf(error_msg, "[0.0.0.0:?] bind to socket failed with error code of %d in Do_init_server_socket.\n", error);

        Do_log_error(error_msg);
        exit(SOCKET_BIND_ERROR);
    }

	/* start listening on the socket */
    error = listen(the_socket, SOMAXCONN);
    if (error != 0) {

        sprintf(error_msg, "[0.0.0.0:?] listen command failed with error code of %d in Do_init_server_socket\n", error);

        Do_log_error(error_msg);
        exit(SOCKET_LISTEN_ERROR);
    }

    if (error = fcntl(the_socket, F_SETOWN, getpid()) < 0) {

	sprintf(error_msg, "[0.0.0.0:?] fcntl F_SETOWN failed with error code of %d in Do_init_server_socket.\n", error);

        Do_log_error(error_msg);
	exit(SOCKET_BIND_ERROR);
    }

    if (error = fcntl(the_socket, F_SETFL, O_ASYNC) < 0) {

	sprintf(error_msg, "[0.0.0.0:?] fcntl F_SETFL failed with error code of %d in Do_init_server_socket.\n", error);

        Do_log_error(error_msg);
	exit(SOCKET_BIND_ERROR);
    }

    return the_socket; /* no problems */
}


/************************************************************************
/
/ FUNCTION NAME: Do_accept_connections(struct server_t *s)
/
/ FUNCTION: Create new games for new connections on the socket
/
/ AUTHOR: Brian Kelly, 4/12/99
/
/ ARGUMENTS:
/       struct server_t *s - address of the server's main data strcture
/
/ RETURN VALUE: int error
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/       This routine is specially designed to:
/
/           - strip non-printing characters (unless Wizard)
/           - echo, if desired
/           - redraw the screen if CH_REDRAW is entered
/           - read in only 'mx - 1' characters or less characters
/           - nul-terminate string, and throw away newline
/
/       'mx' is assumed to be at least 2.
/
*************************************************************************/

int Do_accept_connections(struct server_t *s)
{
    struct game_t *game_ptr;
    struct client_t *client_ptr;
    pthread_attr_t thread_attr;
    size_t addrlen;
    char error_msg[SZ_ERROR_MESSAGE];
    int theError, on=1, terms, itemp;
    char gethost_buffer[16384], *string_ptr, *string_ptr_two;
    struct hostent *host_info, host_buffer;
    struct in_addr theNetwork;

    while (Do_check_socket(s->the_socket)) {

	    /* on a new connection, seed the random number generator */
	srandom_r (time(NULL), (struct random_data *)&randData);

	    /* create a structure for the thread information */
        client_ptr = (struct client_t *) Do_malloc(SZ_CLIENT);

	    /* accept the new connection */
	addrlen = sizeof(client_ptr->address);

	errno = 0;
        client_ptr->socket = accept(s->the_socket,
		(struct sockaddr *)&client_ptr->address, &addrlen);

        if (client_ptr->socket == -1) {

            sprintf(error_msg,
	   "[0.0.0.0:%d] accept socket failed in Do_accept_connections: %s.\n", 
	   s->realm.serverPid, strerror(errno));

	    Do_log_error(error_msg);
            free((void *)client_ptr);
            return SOCKET_ACCEPT_ERROR; 
        }

	    /* log the connection */
	sprintf(error_msg, "New connection on socket %d.\n",
		client_ptr->socket);

	Do_log(SERVER_LOG, error_msg);

	    /* set the socket variables */
	client_ptr->socket_up = TRUE;
	client_ptr->out_buffer_size = 0;
	client_ptr->in_buffer_size = 0;
        client_ptr->date_connected = time(NULL);
	client_ptr->run_level = SIGNING_IN;
	client_ptr->machineID = 0;
	client_ptr->events = NULL;
	client_ptr->channel = 1;
	client_ptr->timeout = 120;
	client_ptr->timeoutFlag = 0;
	client_ptr->broadcast = FALSE;
	client_ptr->stuck = FALSE;
        client_ptr->knightEnergy = 0;
        client_ptr->knightQuickness = 0;
        client_ptr->ageCount = 0;
        client_ptr->morgothCount = 0;
      	client_ptr->suspended = FALSE;
	client_ptr->accountLoaded = FALSE;
	client_ptr->characterLoaded = FALSE;
	client_ptr->characterAnnounced = FALSE;
	client_ptr->muteUntil = 0;
	client_ptr->tagUntil = 0;
	client_ptr->hearBroadcasts = TRUE;
	client_ptr->swearCount = 0;
	client_ptr->battle.rounds = 0;
	client_ptr->battle.timeouts = 0;
	client_ptr->previousName[0] = '\0';
	client_ptr->wizaccount[0] = '\0';
	client_ptr->wizIP[0] = '\0';

	for (terms = 0; terms < 10; terms++) {
	    client_ptr->chatTimes[terms] = 0;
            client_ptr->chatLength[terms] = 0;
	}


            /* set the from and name fields */
            /* determine the dns entry of this connection */
	    /* I've had major problems getting the reentrant fuction
	    working with linux as it is not standard.  I hope the normal
	    function run in main process is okay */
	host_info = NULL;
	theError = 0;
/*
        gethostbyaddr_r((char *)&client_ptr->address.sin_addr,
                sizeof(client_ptr->address.sin_addr), AF_INET, &host_buffer,
		gethost_buffer, 16384, &host_info, &theError);
*/

/*
	errno = 0;
	host_info = gethostbyaddr((char *)&client_ptr->address.sin_addr, 
		sizeof(client_ptr->address.sin_addr), AF_INET);
	theError = errno;
*/

            /* if we received host information */
        if (host_info != NULL) {

                /* copy over the hostname */
            strncpy(&client_ptr->IP, host_info->h_name, SZ_FROM - 1);
	    client_ptr->IP[SZ_FROM - 1] = '\0';

	        /* determine the network address of this connection */
	    string_ptr = client_ptr->IP;

	        /* skip the first term (assume host name) */
	    while (*string_ptr != '\0' && *string_ptr++ != '.') {}

		/* if we found a null, there is no network (local machine) */
	    if (*string_ptr == '\0') {
                strncpy(client_ptr->network, client_ptr->IP, SZ_FROM - 1);
		client_ptr->network[SZ_FROM - 1] = '\0';
		client_ptr->addressResolved = FALSE;
	    }
	    else {
	            /* count the number or remaining terms */
	        terms = 1;
	        string_ptr_two = string_ptr;
	        while (*string_ptr_two != '\0') {

	            if (*string_ptr_two++ == '.')
		        ++terms;
	        }

	            /* remove terms until we find one without numbers or hex or
	            we have only two terms left */

	        string_ptr_two = string_ptr;
	        while (*string_ptr != '\0' || terms > 2) {

	            if (isxdigit(*string_ptr)) {
		        ++string_ptr;
	            }
	            else if (*string_ptr == '.') {
		        string_ptr_two = ++string_ptr;
	            }
	            else {
		        break;
	            }
	        }

	            /* put this shortened hostname into place */
                strncpy(client_ptr->network, string_ptr_two, SZ_FROM - 1);
		client_ptr->network[SZ_FROM - 1];
		client_ptr->addressResolved = TRUE;
	    }
	}
	else {
	        /* use the IP address */
            string_ptr = (char *) inet_ntoa(client_ptr->address.sin_addr);
            strncpy(client_ptr->IP, string_ptr, SZ_FROM - 1);
	    client_ptr->IP[SZ_FROM - 1] = '\0';

		/* get the class C network address */
	    theNetwork.s_addr = client_ptr->address.sin_addr.s_addr &
		    0x00FFFFFF;

	    client_ptr->addressResolved = FALSE;

            string_ptr = (char *) inet_ntoa(theNetwork);
            strncpy(client_ptr->network, string_ptr, SZ_FROM - 1);
	    client_ptr->network[SZ_FROM - 1] = '\0';

                /* stop logging this an an error - too common */
/*
            sprintf(error_msg,
       "[%s:?] gethostbyaddress returned error %d in Do_accept_connections.\n",
       client_ptr->IP, theError);

            Do_log_error(error_msg);
*/
        }

sprintf(error_msg, "Connection, IP=%s, Network=%s.\n", client_ptr->IP, client_ptr->network);

Do_log(DEBUG_LOG, error_msg);

	    /* create a new game object */
        game_ptr = (struct game_t *) Do_malloc(SZ_GAME);

	    /* set up the game variables */
        game_ptr->cleanup_thread = FALSE;
        game_ptr->virtual = FALSE;
        game_ptr->hearAllChannels = HEAR_SELF;
        game_ptr->chatFilter = TRUE;
        game_ptr->sendEvents = FALSE;
        game_ptr->the_socket = client_ptr->socket;
        game_ptr->description = NULL;
	game_ptr->it_combat = NULL;
        game_ptr->events_in = NULL;
	game_ptr->account[0] = '\0';
	strcpy(game_ptr->IP, client_ptr->IP);
	strcpy(game_ptr->network, client_ptr->network);
	game_ptr->machineID = 0;

	    /* initialize the event queue locks */
        Do_init_mutex(&game_ptr->events_in_lock);

	    /* lock the game list */
	Do_lock_mutex(&s->realm.realm_lock);

	    /* put the temp game into the list of games */
        game_ptr->next_game = s->realm.games;
        s->realm.games = game_ptr;

	    /* unlock the game list */
	Do_unlock_mutex(&s->realm.realm_lock);

	    /* init the pthread_att strcture */
	theError = pthread_attr_init(&thread_attr);
	if (theError) {

            sprintf(error_msg, "[0.0.0.0:%d] init of pthread_attr_t failed with error code of %d in Do_accept_connections.\n", s->realm.serverPid, theError);

	    Do_log_error(error_msg);
            free((void *)client_ptr);
            return PTHREAD_ATTR_ERROR;
	}
/*
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&thread_attr, 0xfffffff);
*/

	    /* set up all information to be passed to the thread */
        client_ptr->realm = &s->realm;
        client_ptr->game = game_ptr;

	    /* create the new thread */
        theError = pthread_create(&game_ptr->the_thread, NULL,
		Do_start_thread, (void *)client_ptr);

        if (theError) {

            sprintf(error_msg, "[0.0.0.0:%d] thread creation failed with an error code of %d in Do_accept_connections.\n", s->realm.serverPid, theError);

	    Do_log_error(error_msg);
            free((void *)client_ptr);
            return PTHREAD_CREATE_ERROR;
        }

        ++s->num_games;
    }

    return 0;
}


/************************************************************************
/
/ FUNCTION NAME: Do_check_socket(int the_socket)
/
/ FUNCTION: Checks for data waiting on the socket
/
/ AUTHOR: Brian Kelly, 4/23/99
/
/ ARGUMENTS:
/       int the_socket - the socket to check for data
/
/ RETURN VALUE: 
/	bool - true if there is data waiting on the socket
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/       This routine is specially designed to:
/
*************************************************************************/

int Do_check_socket(int the_socket)
{
    fd_set rmask;
    static struct timeval timeout = { 0, 0 };	/* no timeout */
    char error_msg[SZ_ERROR_MESSAGE];
    int error;

	/* set rmask to check our socket */
    FD_ZERO(&rmask);
    FD_SET(the_socket, &rmask);

	/* check for connections to be accepted */
    error = select(the_socket + 1, &rmask, 0, 0, &timeout);
    if (error < 0) {

        sprintf(error_msg, "[0.0.0.0:?] select on socket failed with error code of %d in Do_check_socket\n", error);

	Do_log_error(error_msg);
        exit(SOCKET_SELECT_ERROR);
    }

        /* if select found no matches, return */
    if (error == 0) {
        return FALSE;
    }

        /* if our socket flag is not set, something is wrong */
    if (!FD_ISSET(the_socket, &rmask)) {

        sprintf(error_msg, "[0.0.0.0:?] select returned %d, but socket flag is off in Do_check_socket.\n", error);

	Do_log_error(error_msg);
        exit (SOCKET_SELECT_ERROR); 
    }
    return TRUE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_send_buffer(int the_socket)
/
/ FUNCTION: Send data over the socket to the player
/
/ AUTHOR: Brian Kelly, 4/23/99
/
/ ARGUMENTS:
/	int the_socket - the socket to send the information on
/	size_t the_size - the number of byest to send
/	void *the_data - a pointer to the data to be sent
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

Do_send_buffer(struct client_t *c)
{
/*
    char error_msg[SZ_ERROR_MESSAGE];
*/
    char error_msg[2048];
    int bytes_sent;

    if (c->socket_up && c->out_buffer_size > 0) {

	    /* send off the data */
        errno = 0;
        c->out_buffer[c->out_buffer_size] = '\0';

#ifdef SEND_DEBUG
        sprintf(error_msg, "[%s] sending %d bytes\n", c->connection_id,
		c->out_buffer_size);

	Do_log(DEBUG_LOG, error_msg);
#endif

#ifdef SEND_PACKET_DEBUG
        sprintf(error_msg, "[%s] (%s)\n", c->connection_id, c->out_buffer);
	Do_log(DEBUG_LOG, error_msg);
#endif


        bytes_sent = send(c->socket, c->out_buffer, c->out_buffer_size, 0);

        if (bytes_sent != c->out_buffer_size) {

            sprintf(error_msg,
	"[%s] send on socket sent %d out of %d bytes in Do_send_buffer: %s.\n",
	c->connection_id, bytes_sent, c->out_buffer_size, strerror(errno));

	    Do_log_error(error_msg);
	    c->socket_up = FALSE;

            sprintf(error_msg, "[%s] Error on socket while sending.\n",
		    c->connection_id);

            Do_log(CONNECTION_LOG, error_msg);

        }
        c->out_buffer_size = 0;
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_send_out(struct client_t *c, void *the_data, size_t the_size)
/
/ FUNCTION: Send data over the socket to the player
/
/ AUTHOR: Brian Kelly, 5/6/99
/
/ ARGUMENTS:
/	int the_socket - the socket to send the information on
/	size_t the_size - the number of byest to send
/	void *the_data - a pointer to the data to be sent
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

Do_send_out(struct client_t *c, void *the_data, size_t the_size)
{
    char error_msg[SZ_ERROR_MESSAGE];
    char string_buffer[SZ_LINE], string_buffer2[SZ_LINE];

    if (c->socket_up) {

        if (the_size > SZ_OUT_BUFFER) {

            sprintf(error_msg,
		"[%s] buffer_out overflow with %d bytes in Do_send_out.\n",
		c->connection_id, the_size);

	    Do_log_error(error_msg);
	    Do_send_error(c, error_msg);
            c->socket_up = FALSE;
	    return;
        }

        if (the_size + c->out_buffer_size > SZ_OUT_BUFFER) {
            Do_send_buffer(c);

	    if (the_size + c->out_buffer_size > SZ_OUT_BUFFER) {
		return;
	    }
        }

#ifdef SEND_QUEUE_DEBUG
        memcpy(string_buffer, the_data, the_size);

	    /* remove the "\n" */
	string_buffer[the_size - 1] = '\0';

       	sprintf(string_buffer2, "[%s] Queued (%s)\n", c->connection_id,
		string_buffer);

	Do_log(DEBUG_LOG, string_buffer2);
#endif

        memcpy(&c->out_buffer[c->out_buffer_size], the_data, the_size);
        c->out_buffer_size += the_size;

    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_send_string(struct client_t *c, char *theMessage)
/
/ FUNCTION: Send data over the socket to the player
/
/ AUTHOR: Brian Kelly, 8/11/99
/
/ ARGUMENTS:
/	int the_socket - the socket to send the information on
/	size_t the_size - the number of byest to send
/	void *the_data - a pointer to the data to be sent
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

Do_send_string(struct client_t *c, char *theMessage)
{
    int theSize;

	/* determine the string size */
    theSize = strlen(theMessage);

	/* send the data */
    Do_send_out(c, theMessage, theSize);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_send_double(struct client_t *c, double theDouble)
/
/ FUNCTION: Send data over the socket to the player
/
/ AUTHOR: Brian Kelly, 8/13/99
/
/ ARGUMENTS:
/	int the_socket - the socket to send the information on
/	size_t the_size - the number of byest to send
/	void *the_data - a pointer to the data to be sent
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

Do_send_double(struct client_t *c, double theDouble)
{
    char tmpDouble[SZ_NUMBER];

	/* determine the string size */
    sprintf(tmpDouble, "%0.lf\n", theDouble);

	/* send the data */
    Do_send_string(c, tmpDouble);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_send_float(struct client_t *c, float theFloat)
/
/ FUNCTION: Send data over the socket to the player
/
/ AUTHOR: Brian Kelly, 8/13/99
/
/ ARGUMENTS:
/	int the_socket - the socket to send the information on
/	size_t the_size - the number of byest to send
/	void *the_data - a pointer to the data to be sent
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

Do_send_float(struct client_t *c, double theFloat)
{
    char tmpFloat[SZ_NUMBER];

	/* determine the string size */
    sprintf(tmpFloat, "%0.lf\n", theFloat);

	/* send the data */
    Do_send_string(c, tmpFloat);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_send_fpfloat(struct client_t *c, float theFloat)
/
/ FUNCTION: Send a full-precision float to the client
/
/ AUTHOR: Brian Kelly, 8/25/99
/
/ ARGUMENTS:
/	int the_socket - the socket to send the information on
/	size_t the_size - the number of byest to send
/	void *the_data - a pointer to the data to be sent
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

Do_send_fpfloat(struct client_t *c, double theFloat)
{
    char tmpFloat[SZ_NUMBER];

	/* determine the string size */
    sprintf(tmpFloat, "%.4lf\n", theFloat);

	/* send the data */
    Do_send_string(c, tmpFloat);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_send_int(struct client_t *c, int theInt)
/
/ FUNCTION: Send data over the socket to the player
/
/ AUTHOR: Brian Kelly, 8/13/99
/
/ ARGUMENTS:
/	int the_socket - the socket to send the information on
/	size_t the_size - the number of byest to send
/	void *the_data - a pointer to the data to be sent
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

Do_send_int(struct client_t *c, int theInt)
{
    char tmpInt[SZ_NUMBER];

	/* determine the string size */
    sprintf(tmpInt, "%d\n", theInt);

	/* send the data */
    Do_send_string(c, tmpInt);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_send_short(struct client_t *c, short theShort)
/
/ FUNCTION: Send data over the socket to the player
/
/ AUTHOR: Brian Kelly, 8/13/99
/
/ ARGUMENTS:
/	int the_socket - the socket to send the information on
/	size_t the_size - the number of byest to send
/	void *the_data - a pointer to the data to be sent
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

Do_send_short(struct client_t *c, short theShort)
{
    char tmpShort[SZ_NUMBER];

	/* determine the string size */
    sprintf(tmpShort, "%hd\n", theShort);

	/* send the data */
    Do_send_string(c, tmpShort);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_send_bool(struct client_t *c, bool theBool)
/
/ FUNCTION: Send data over the socket to the player
/
/ AUTHOR: Brian Kelly, 8/13/99
/
/ ARGUMENTS:
/	int the_socket - the socket to send the information on
/	size_t the_size - the number of byest to send
/	void *the_data - a pointer to the data to be sent
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

Do_send_bool(struct client_t *c, short theBool)
{
    if (theBool == FALSE)

	    /* send the data */
        Do_send_string(c, "No\n");
    else

	    /* send the data */
        Do_send_string(c, "Yes\n");

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_send_error(struct client_t *c, char *theError)
/
/ FUNCTION: Send data over the socket to the player
/
/ AUTHOR: Brian Kelly, 10/2/99
/
/ ARGUMENTS:
/	int the_socket - the socket to send the information on
/	size_t the_size - the number of byest to send
/	void *the_data - a pointer to the data to be sent
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

Do_send_error(struct client_t *c, char *theError)
{
    Do_send_int(c, ERROR_PACKET);
    Do_send_string(c, theError);
    Do_send_buffer(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_read_socket(struct client_t *c)
/
/ FUNCTION: Send data over the socket to the player
/
/ AUTHOR: Brian Kelly, 4/24/99
/
/ ARGUMENTS:
/       int the_socket - the socket to read data from
/	size_t the_size - the number of bytes to read from the socket
/	void *the_data - a pointer to the data to be read
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

Do_read_socket(struct client_t *c)
{
    char error_msg[SZ_ERROR_MESSAGE];
    int bytes_read;
    struct event_t *event_ptr;

    errno = 0;

	/* read the data from the socket */
    bytes_read = recv(c->socket, &c->in_buffer[c->in_buffer_size],
	    (SZ_IN_BUFFER - c->in_buffer_size), 0);

    if (bytes_read <= 0) {

	    /* the socket is no longer any good */
	c->socket_up = FALSE;

	    /* if client closed abruptly, the connection will be reset */
	if (errno = ECONNRESET) {

		/* Error too common for the error log */
/*
	    sprintf(error_msg,
	           "[%s] Received a ECONNRESET on socket in Do_read_socket.\n",
		   c->connection_id);

            Do_log_error(error_msg);
*/
	}
	else {

            sprintf(error_msg,
	       "[%s] read on socket returned %d bytes in Do_read_socket: %s\n",
	       c->connection_id, bytes_read, strerror(errno));

            Do_log_error(error_msg);
	}

	sprintf(error_msg, "[%s] Received an error on the socket.\n",
		c->connection_id);

	Do_log(CONNECTION_LOG, error_msg);
    }

    else if (bytes_read + c->in_buffer_size > SZ_IN_BUFFER) {

	    /* the socket is no longer any good */
	c->socket_up = FALSE;

	    /* log an error */
        sprintf(error_msg, "[%s] added %d to %d bytes for in_buffer overvlow in Do_read_socket.\n", c->connection_id, bytes_read, c->in_buffer_size);

	Do_log_error(error_msg);
    }

    else {
	c->in_buffer_size += bytes_read;
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_get_socket_string(struct client_t *c, void *the_data, size_t the_size)
/
/ FUNCTION: reads the next packet type off the socket
/
/ AUTHOR: Brian Kelly, 4/23/99
/
/ ARGUMENTS:
/	int the_socket - the socket to read the packet type
/
/ RETURN VALUE: 
/	int - the type of packet next on the socket
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

int Do_get_socket_string(struct client_t *c, char *theString, size_t theSize)
{
    size_t theLength;
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE + SZ_OUT_BUFFER];
    sigset_t sigMask;
    int theSignal;

	/* prepare to unblock SIGIO */
    sigemptyset(&sigMask);
    sigaddset(&sigMask, SIGIO);
    sigaddset(&sigMask, SIGALRM);

    for (;;) {

	    /* if the socket is down, return an error */
	if (!c->socket_up) {
	    return S_ERROR;
	}

	    /* if we have information in the buffer */
	if (c->in_buffer_size) {

	        /* see if the entire string has been downloaded */
	    theLength = strlen(c->in_buffer);

	    if (theLength < c->in_buffer_size) {

		    /* check that the passed pointer can handle the size */
		if (theSize - 1 < theLength) {

	                /* log the error */
		    sprintf(error_msg, "[%s] Client returned a string of %d bytes, %d max in Do_get_socket_string.\n", c->connection_id, theLength, theSize);

		    Do_log_error(error_msg);

	    		/* get as much of the string as we can */
		    strncpy(theString, c->in_buffer, theSize - 1);
		    theString[theSize - 1] = '\0';
		}
		else {
		    strcpy(theString, c->in_buffer);
		}

		    /* add the terminating null to the string */
		++theLength;

		    /* move up information in the buffer */
	        if (theLength < c->in_buffer_size) {
	            c->in_buffer_size -= theLength;
	            memmove(c->in_buffer, &c->in_buffer[theLength],
			    c->in_buffer_size);
	        }
	        else {
	            c->in_buffer_size = 0;
	        }

#ifdef RECEIVE_DEBUG
	        sprintf(error_msg, "[%s] received %d bytes\n",
			c->connection_id, theLength);

		Do_log(DEBUG_LOG, error_msg);
#endif

#ifdef RECEIVE_PACKET_DEBUG
        	sprintf(error_msg, "[%s] (%s)\n", c->connection_id, theString);
		Do_log(DEBUG_LOG, error_msg);
#endif

	        return S_NORM;
            }

		/* the the buffer is maxed and we're here,
			the string is too large for the buffer */

	    if (c->in_buffer_size == SZ_IN_BUFFER) {

		sprintf(error_msg, "[%s] Request for a string, %d bytes, larger than buffer in Do_get_socket_string.\n", c->connection_id, theLength);

		Do_log_error(error_msg);

		c->socket_up = FALSE;
		return S_ERROR;
	    }
	}

	    /* We need to wait for information, so pause */
	if (c->socket_up) {

#ifdef SUSPEND_DEBUG
	    sprintf(error_msg,
		    "[%s] now sleeping with alarm set for %d seconds.\n",
		    c->connection_id, c->timeout);

	    Do_log(DEBUG_LOG, error_msg);
#endif
	    sigwait(&sigMask, &theSignal);

#ifdef SUSPEND_DEBUG
	    sprintf(error_msg, "[%s] awoken on signal %d.\n",
		    c->connection_id, theSignal);

	    Do_log(DEBUG_LOG, error_msg);
#endif
/*
	    sleep(.1);
*/

	}
	else {
	    theSignal = SIGIO;
	}

	    /* check events and the socket on a SIGIO */
	if (theSignal == SIGIO) {

	        /* if the socket is up, we have room in the buffer and
		        there is info waiting */

            if (c->socket_up && c->in_buffer_size < SZ_IN_BUFFER &&
		    Do_check_socket(c->socket)) {

                Do_read_socket(c);
            }

	        /* see if any other threads have sent us an event */
	    Do_check_events_in(c);
	}

	    /* see if the tread alarm went off */
	else if (theSignal == SIGALRM) {
/*
	if (time(NULL) > c->timeoutAt) {
*/

	    switch(++c->timeoutFlag) {

	        /* alarm has gone off once */
	    case 1:
	        Do_send_int(c, PING_PACKET);
	        Do_send_buffer(c);

		    /* give the client 15 seconds to respond */
	        alarm(15);
		c->timeoutAt = time(NULL) + 15;
	        break;

	        /* gone off twice */
	    case 2:

                sprintf(error_msg, "[%s] Socket connection timed out.\n",
		        c->connection_id);

                Do_log(CONNECTION_LOG, error_msg);
	        Do_send_error(c, "The socket connection timed out.\n");
	        Do_send_buffer(c);

		    /* assume the network connection is down */
	        sprintf(error_msg,
		 "[%s] Socket connection timed out in Do_get_socket_string.\n",
		 c->connection_id);

	        Do_log_error(error_msg);
	        c->socket_up = FALSE;
	        return S_ERROR;
	    }
	}

	    /* unknown signal */
	else {
	    sprintf(error_msg, "[%s] Received unknown signal %d.\n",
		    c->connection_id, theSignal);

	    Do_log_error(error_msg);
	}
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_get_string(struct client_t *c, char *theString,
/	int maxSize);
/
/ FUNCTION: Wait until char is returned from player
/
/ AUTHOR: Brian Kelly, 8/11/99
/
/ ARGUMENTS:
/	client_t c - the client data strcture
/	size_t the_size - the number of bytes fo data expected
/	void *the_data - a pointer to where the data should be written
/	bool exact - Does there need to be exactly the_size bytes?
/
/ RETURN VALUE: 
/	bool - Was the data or a timeout registered
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

int Do_get_string(struct client_t *c, char *theString, int maxSize)
{
    char packetTmp[SZ_PACKET_TYPE], error_msg[SZ_ERROR_MESSAGE];
    int packetType;
    int returnCode;

    Do_send_buffer(c);

    c->timeoutFlag = 0;
    alarm(c->timeout);
    c->timeoutAt = time(NULL) + c->timeout;

    for(;;) {

	    /* get the header of the next packet */
	errno = 0;
        if (Do_get_socket_string(c, packetTmp, SZ_PACKET_TYPE) == S_ERROR) {
	    alarm(0);
	    return S_ERROR;
	}

	    /* convert the string to an integer */
	packetType = strtol(packetTmp, NULL, 10);

	switch(packetType) {

	    /* if the packet type is returning an answer */
        case C_RESPONSE_PACKET:

	        /* read the string */
	    returnCode = Do_get_socket_string(c, theString, maxSize);
	    alarm(0);
	    return returnCode;

	case C_CANCEL_PACKET:

	    alarm(0);
	    return S_CANCEL;

	case C_PING_PACKET:

	    if (c->timeoutFlag == 1) {

		alarm(0);
	        return S_TIMEOUT;
	    }
	    else {
	        sprintf(error_msg, "[%s] Received unexpected ping packet.\n",
		    c->connection_id);

	        Do_log_error(error_msg);
	    }
	    break;

	default:

	    if (Do_packet(c, packetType) == S_ERROR) {
		alarm(0);
	        return S_ERROR;
	    }
	}
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_get_double(struct client_t *c, double *theDouble);
/
/ FUNCTION: Wait until char is returned from player
/
/ AUTHOR: Brian Kelly, 8/11/99
/
/ ARGUMENTS:
/	client_t c - the client data strcture
/	size_t the_size - the number of bytes fo data expected
/	void *the_data - a pointer to where the data should be written
/	bool exact - Does there need to be exactly the_size bytes?
/
/ RETURN VALUE: 
/	bool - Was the data or a timeout registered
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

int Do_get_double(struct client_t *c, double *theDouble)
{
    char tmpDouble[SZ_NUMBER];
    int rc;

    if (rc = Do_get_string(c, tmpDouble, SZ_NUMBER) != S_NORM) {
	return rc;
    }

    *theDouble = floor(strtod(tmpDouble, NULL));

        /* insure that the number is finite */
    if (!finite(*theDouble)) {

	    /* fix the number */
        *theDouble = 0;
    }

    return S_NORM;
}


/************************************************************************
/
/ FUNCTION NAME: Do_get_long(struct client_t *c, long *theLong);
/
/ FUNCTION: Wait until char is returned from player
/
/ AUTHOR: Brian Kelly, 10/2/99
/
/ ARGUMENTS:
/	client_t c - the client data strcture
/	size_t the_size - the number of bytes fo data expected
/	void *the_data - a pointer to where the data should be written
/	bool exact - Does there need to be exactly the_size bytes?
/
/ RETURN VALUE: 
/	bool - Was the data or a timeout registered
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

int Do_get_long(struct client_t *c, long *theLong)
{
    char tmpLong[SZ_NUMBER];
    int returnCode;

    returnCode = Do_get_string(c, tmpLong, SZ_NUMBER);

    if (returnCode != S_NORM) {
	return returnCode;
    }

    *theLong = strtol(tmpLong, NULL, 10);

    return S_NORM;
}


/************************************************************************
/
/ FUNCTION NAME: Do_get_nothing(struct client_t *c);
/
/ FUNCTION: Wait until char is returned from player
/
/ AUTHOR: Brian Kelly, 11/3/99
/
/ ARGUMENTS:
/	client_t c - the client data strcture
/	size_t the_size - the number of bytes fo data expected
/	void *the_data - a pointer to where the data should be written
/	bool exact - Does there need to be exactly the_size bytes?
/
/ RETURN VALUE: 
/	bool - Was the data or a timeout registered
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

int Do_get_nothing(struct client_t *c)
{
    char packetTmp[SZ_PACKET_TYPE];
    char error_msg[SZ_ERROR_MESSAGE];
    int packetType;

    for(;;) {

	    /* if the socket is up, we have room in the buffer and
		    there is info waiting */

        if (c->socket_up && c->in_buffer_size < SZ_IN_BUFFER &&
		Do_check_socket(c->socket)) {

            Do_read_socket(c);
        }

	    /* if the socket is up, and the buffer has data */
        if (c->socket_up && c->in_buffer_size) {

	    c->timeoutAt = time(NULL) + c->timeout;

	        /* get the header of the next packet */
            if (Do_get_socket_string(c, packetTmp, SZ_PACKET_TYPE) == S_ERROR) 
	        return S_ERROR;

	        /* convert the string to an integer */
	    packetType = strtol(packetTmp, NULL, 10);

	    switch(packetType) {

	        /* we're not expecting an answer */
            case C_RESPONSE_PACKET:
	    case C_CANCEL_PACKET:
	    case C_PING_PACKET:

	            /* print an error message */
	        sprintf(error_msg,
		"[%s] Unexpected packet type %d returned in Do_get_nothing.\n",
		c->connection_id, packetType);

	        Do_log_error(error_msg);
	        Do_send_error(c, error_msg);
                c->socket_up = FALSE;

	        return S_ERROR;

	    default:

	        if (Do_packet(c, packetType) == S_ERROR) {
	            return S_ERROR;
		}
	    }
	}
	else {
	    return S_NORM;
	}
    }
}


/************************************************************************
/
/ FUNCTION NAME: int Do_packet(client_t *c, char packetType);
/
/ FUNCTION: Wait until char is returned from player
/
/ AUTHOR: Brian Kelly, 8/11/99
/
/ ARGUMENTS:
/	client_t c - the client data strcture
/	size_t the_size - the number of bytes fo data expected
/	void *the_data - a pointer to where the data should be written
/	bool exact - Does there need to be exactly the_size bytes?
/
/ RETURN VALUE: 
/	bool - Was the data or a timeout registered
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

int Do_packet(struct client_t *c, int thePacket)
{
    char numChars;
    time_t timeNow;
    struct event_t *event_ptr;
    char error_msg[SZ_ERROR_MESSAGE];
    char player_name[SZ_NAME];
    char string_buffer[SZ_CHAT - 20];
    long theLong;

	/* switch on the type of event */
    switch(thePacket) {

    case C_CHAT_PACKET:

	    /* get the string to be sent and quit on error*/
	if (Do_get_socket_string(c, string_buffer, SZ_CHAT - 20) == S_ERROR)
	    return S_ERROR;

	    /* mute null chat messages */
	if (strlen(string_buffer) > 0) {

		/* if the player is muted */
	    if (c->muteUntil < time(NULL)) {

                    /* send the chat message */
	        Do_chat(c, string_buffer);
	    }
	}

	/* get rid of old tags */
	if(c->tagUntil < time(NULL))
	{
		Do_remove_prefix_suffix(c);
		/* if(c->characterLoaded)
		{
			strcpy(string_buffer, c->player.name);
			if(c->characterAnnounced)
			{
				Do_send_specification(c, REMOVE_PLAYER_EVENT);
			}
			strncpy(c->modifiedName, string_buffer, SZ_NAME - 1);
			if(c->characterAnnounced)
			{
				Do_send_specification(c, ADD_PLAYER_EVENT);
				Do_name(c);
			}
		} */
	}

	    /* finished */
	return S_NORM;

    case C_EXAMINE_PACKET:

	    /* read player name and quit if there's an error */
	if (Do_get_socket_string(c, player_name, SZ_NAME) == S_ERROR)
	    return S_ERROR;

	    /* request the info from the player */
	event_ptr = (struct event_t *) Do_create_event();
	event_ptr->type = REQUEST_RECORD_EVENT;
	event_ptr->from = c->game;

	if (!Do_send_character_event(c, event_ptr, player_name)) {
	    free((void *)event_ptr);
	}

	    /* finished */
	return S_NORM;

    case C_SCOREBOARD_PACKET:

	    /* read starting record and quit if there's an error */
	if (Do_get_socket_string(c, string_buffer, SZ_NAME) == S_ERROR)
	    return S_ERROR;

	theLong = strtol(string_buffer, NULL, 10);
	    /* convert the string to an int */
	Do_scoreboard(c, (int) theLong, TRUE);

	    /* finished */
	return S_NORM;

    case C_ERROR_PACKET:

	    /* see if the client included a message */
	if (Do_get_socket_string(c, error_msg, SZ_ERROR_MESSAGE) !=
		S_NORM) {

	    error_msg[0] = '\0';
	}

	    /* log the error */
	sprintf(string_buffer,
		"[%s] Client returned an error packet in Do_packet: %s.\n",
		c->connection_id, error_msg);

	Do_log_error(string_buffer);

	c->socket_up = FALSE;
	return S_ERROR;

    default:

	    /* print an error message */
	sprintf(error_msg,
		"[%s] Received unknown packet type %d in Do_packet.\n",
		c->connection_id, thePacket);

	Do_log_error(error_msg);
	Do_send_error(c, error_msg);
        c->socket_up = FALSE;

	    /* exit gracefully */
	return S_ERROR;
    }
}


/************************************************************************
/
/ FUNCTION NAME: int Do_get_network_address(client_t *c, int hostAddress, int subnetMask);
/
/ FUNCTION: Wait until char is returned from player
/
/ AUTHOR: Brian Kelly, 6/8/00
/
/ ARGUMENTS:
/	client_t c - the client data strcture
/	size_t the_size - the number of bytes fo data expected
/	void *the_data - a pointer to where the data should be written
/	bool exact - Does there need to be exactly the_size bytes?
/
/ RETURN VALUE: 
/	bool - Was the data or a timeout registered
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

Do_get_network_address(struct in_addr *hostAddress, int subnetMask)
{
    unsigned int bitMask;

	/* create a bitmask.  Ex: 8 mask = 0xffffff00 */
    bitMask = pow(2, subnetMask) - 1;
    bitMask = (unsigned int) 0xffffffff ^ bitMask;

	/* return only the network portion of the hostAddress */
    hostAddress->s_addr = bitMask & htonl(hostAddress->s_addr);
    hostAddress->s_addr = ntohl(hostAddress->s_addr);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: int Do_check_host_address(int hostAddress, int networkAddress, int subnetMask);
/
/ FUNCTION: Wait until char is returned from player
/
/ AUTHOR: Brian Kelly, 6/8/00
/
/ ARGUMENTS:
/	client_t c - the client data strcture
/	size_t the_size - the number of bytes fo data expected
/	void *the_data - a pointer to where the data should be written
/	bool exact - Does there need to be exactly the_size bytes?
/
/ RETURN VALUE: 
/	bool - Was the data or a timeout registered
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ DESCRIPTION:
/       Read a string from the keyboard.
/
/************************************************************************/

int Do_check_host_address(struct in_addr *hostAddress, struct in_addr *networkAddress, int subnetMask)
{
    struct in_addr checkAddress;

    if (subnetMask < 0 || subnetMask > 24) {
	return FALSE;
    }

    checkAddress.s_addr = hostAddress->s_addr;

    Do_get_network_address(&checkAddress, subnetMask);

    if (networkAddress->s_addr == checkAddress.s_addr) {
        return TRUE;
    }
    
    return FALSE;
}

