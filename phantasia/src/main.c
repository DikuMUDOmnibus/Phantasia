/*
 * Phantasia 4 -- Interterminal fantasy game
 *
 * Edward A. Estes
 * AT&T, March 12, 1986
 *
 * Client-Server port for internet web use:
 * Brian Kelly
 * bkelly@dimensional.com
 * April 7, 1999
 */

/* DISCLAIMER:
 *
 * This game is distributed for free as is.  It is not guaranteed to work
 * in every conceivable environment.  It is not even guaranteed to work
 * in ANY environment.
 *
 * This game is distributed without notice of copyright, therefore it
 * may be used in any manner the recipient sees fit.  However, the
 * author assumes no responsibility for maintaining or revising this
 * game, in its original form, or any derivitives thereof.
 *
 * The author shall not be responsible for any loss, cost, or damage,
 * including consequential damage, caused by reliance on this material.
 *
 * The author makes no warranties, express or implied, including warranties
 * of merchantability or fitness for a particular purpose or use.
 *
 * AT&T is in no way connected with this game.
 * (Nor is any company Brian Kelly was pretending to work for while working
 * on this game)
 */

/* Phantasia 4 is a complete rewrite of the Phantasia 3 code put together by
 * Edward Estes.  I played Phantasia quite a bit back at the Colorado School
 * of Mines, and thought it would make a great internet game as I became
 * more invoved with the web.  I believe the main advantage this game has
 * over others is that each player, although interacting, are independent
 * and one slow player over a 300 baud connection will not affect another
 * player going at top speed.
 *
 * Detailed changes can be found in the attached document, CHANGES, but
 * for this release, most of the changes center around the problems and
 * advantages of client-server connections.
*/

/*
 * All source files are set up for 'vi' with shiftwidth=4, tabstop=8.
 */

/**/

#include "include.h"

/* global variables */
int server_hook;
char randomStateBuffer[STATELEN];
struct random_data randData;

/*
 * main.c       Main entry point for Phantasia
 */

/***************************************************************************
/ FUNCTION NAME: main()
/
/ FUNCTION: initialize state, and call main process
/
/ AUTHOR: E. A. Estes, 12/4/85
/ MODIFIED:  Brian Kelly, 4/6/99
/
/ ARGUMENTS:
/       int     argc - argument count
/       char    **argv - argument vector
/
/ RETURN VALUE: none
/
/ MODULES CALLED: monstlist(), checkenemy(), activelist(),
/       throneroom(), checkbattle(), readmessage(), changestats(), writerecord()
,
/       tradingpost(), adjuststats(), recallplayer(), displaystats(), checktampe
red(),
/       fabs(), rollnewplayer(), time(), exit(), sqrt(), floor(), wmove(),
/       signal(), strcat(), purgeoldplayers(), getuid(), isatty(), wclear(),
/       strcpy(), system(), altercoordinates(), cleanup(), waddstr(), procmain()
,
/       playinit(), leavegame(), localtime(), getanswer(), neatstuff(), initials
tate(),
/       scorelist(), titlelist()
/
/ GLOBAL INPUTS: *Login, Throne, Wizard, Player, *stdscr, Changed, Databuf[],
/       Fileloc, Stattable[]
/
/ GLOBAL OUTPUTS: Wizard, Player, Changed, Fileloc, Timeout, *Statptr
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

main(argc, argv)
int     argc;
char    *argv[];
{
    	/* set up the main server structure */
    struct server_t server;

	/* set up a signal structre */
    struct sigaction sigAct;

	/* set the program to run */
    server.run_level = RUN_SERVER;

        /* process arguments */
    while (--argc && (*++argv)[0] == '-')
    {
        switch ((*argv)[1])
        {

	    /* purge character file */
        case 'c':
            Do_purge_characters();
            break;

            /* initialize, then bring down server */
        case 'q':
            server.run_level = FAST_SHUTDOWN;
            break;

	    /* purge scoreboard file */
        case 's':
            Do_purge_scoreboard();
            break;

        default:    /* for all other answers */
            printf("usage: phantasia [-c][-q][-s]\n");
            printf("  -c: Purge character roster\n");
            printf("  -q: Quit as soon as possible\n");
            printf("  -s: Purge scoreboard\n");
            exit(1);
        }
    }

	/* block SIGINT, SIGTERM, SIGIO, SIGALRM and SIGUSR1 til we're ready */
    sigemptyset(&sigAct.sa_mask);
    sigaddset(&sigAct.sa_mask, SIGINT);
    sigaddset(&sigAct.sa_mask, SIGTERM);
    sigaddset(&sigAct.sa_mask, SIGIO);
    sigaddset(&sigAct.sa_mask, SIGALRM);
    sigaddset(&sigAct.sa_mask, SIGUSR1);

	/* this routine will block for created threads as well */
    if (pthread_sigmask(SIG_BLOCK, &sigAct.sa_mask, NULL) < 0) {
        printf("Error blocking signals.\n");
        exit(1);
    }

	/* set up structures, variables and files */
    Init_server(&server);

	/* run the main loop */
    Do_main_loop(&server);

	/* destroy structures and close files */
    Do_close(&server);

    Do_log(SERVER_LOG, "Server is shut down.\n");
    exit(0);	/* thanks for playing */
}


/***************************************************************************
/ FUNCTION NAME: Do_main_loop(struct server_t *server)
/
/ FUNCTION: initialize state, and call main process
/
/ AUTHOR:  Brian Kelly, 4/12/99
/
/ ARGUMENTS: 
/	struct server_t *s - address of the sever's main data structure
/
/ RETURN VALUE: none
/
/ MODULES CALLED: monstlist(), checkenemy(), activelist(),
/       throneroom(), checkbattle(), readmessage(), changestats(), writerecord()
,
/       tradingpost(), adjuststats(), recallplayer(), displaystats(), checktampe
red(),
/       fabs(), rollnewplayer(), time(), exit(), sqrt(), floor(), wmove(),
/       signal(), strcat(), purgeoldplayers(), getuid(), isatty(), wclear(),
/       strcpy(), system(), altercoordinates(), cleanup(), waddstr(), procmain()
,
/       playinit(), leavegame(), localtime(), getanswer(), neatstuff(), initials
tate(),
/       scorelist(), titlelist()
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_main_loop(struct server_t *s)
{
    sigset_t sigMask;
    struct it_combat_t *combat_ptr;
    struct game_t *game_ptr, **game_ptr_ptr;
    struct event_t *event_ptr;
    int error, theSignal;
    time_t shutdownStart;
    char error_msg[SZ_ERROR_MESSAGE];

    sprintf(error_msg, "Server started up.  pid=%d\n",
            s->realm.serverPid);

    Do_log(SERVER_LOG, error_msg);

    sigemptyset(&sigMask);
    sigaddset(&sigMask, SIGINT);
    sigaddset(&sigMask, SIGTERM);
    sigaddset(&sigMask, SIGIO);
/*
    sigaddset(&sigMask, SIGALRM);
*/
    sigaddset(&sigMask, SIGUSR1);

    while (s->run_level != FAST_SHUTDOWN) {

	    /* wait for a signal to do anything */
	sigwait(&sigMask, &theSignal);
/*
	sleep(.1);
	theSignal = SIGIO;
*/

	    /* on SIGIO, check for new connections and inactive games */
	if (theSignal == SIGIO) {

	        /* if we're not shutting down check for new connections */
	    if (s->run_level == RUN_SERVER) {
                error = Do_accept_connections(s);
	        if (error) {

		    sprintf(error_msg,
	       "[0.0.0.0:%d] Do_accept_connections returned an error of %d.\n",
	       s->realm.serverPid, error);

		    Do_log_error(error_msg);
	            server_hook = SHUTDOWN;
	        }
            }
	}

	    /* a SIGUSER says to clean up a thread */
	else if (theSignal == SIGUSR1) {

	        /* check for inactive games */
	    Do_lock_mutex(&s->realm.realm_lock);
	    game_ptr_ptr = &s->realm.games;

	    while (*game_ptr_ptr != NULL) {

	        if ((*game_ptr_ptr)->cleanup_thread) {

		    game_ptr = *game_ptr_ptr; 
		    *game_ptr_ptr = game_ptr->next_game;

		    close(game_ptr->the_socket);
		    pthread_join(game_ptr->the_thread, NULL);
	            Do_destroy_mutex(&game_ptr->events_in_lock);

		    free((void *)game_ptr);
        	    --s->num_games;
	        }
	        else {
	            game_ptr_ptr = &((*game_ptr_ptr)->next_game);
	        }
	    }

	    Do_unlock_mutex(&s->realm.realm_lock);
	}

	    /* SIGTERM shuts the server down quickly */
	else if (theSignal == SIGTERM) {
	    server_hook = FAST_SHUTDOWN;
	}

	    /* SIGIO is a normal shutdown */
	else if (theSignal == SIGINT) {
	    server_hook = SHUTDOWN;
	}
	else {
	    sprintf(error_msg,
	        "[0.0.0.0:%d] sigwait caught bad signal %d in Do_main_loop.\n",
	        s->realm.serverPid, theSignal);

	    Do_log_error(error_msg);
	}

	    /* check to see if a thread wants the server down */
	if (server_hook != s->run_level) {

	    if (server_hook == LEISURE_SHUTDOWN && s->num_games == 0) {

		Do_log(SERVER_LOG, "Starting a leisure shutdown.\n");
		server_hook = SHUTDOWN;
		shutdownStart = time(NULL);
	    }

		/* if requested to die now */
	    else if (server_hook == HARD_SHUTDOWN) {

		Do_log(SERVER_LOG, "Hard shutdown ordered.  Exiting Now.\n");
		    /* hasta la vista, baby */
		exit(0);
	    }

		/* check for a quick shutdown */
	    else if (server_hook == FAST_SHUTDOWN) {

		Do_log(SERVER_LOG, "Starting a fast shutdown.\n");
	        s->run_level = FAST_SHUTDOWN;
	    }

		/* normal shutdown */
	    else if (server_hook == SHUTDOWN) {

		Do_log(SERVER_LOG, "Starting a normal shutdown.\n");

	        s->run_level = SHUTDOWN;
		shutdownStart = time(NULL);
	    }
	}

	    /* If shutting down,  quit if nobody's on or time's expired */
        if (s->run_level == SHUTDOWN && (s->num_games == 0 ||
		time(NULL) - shutdownStart > 300)) {

	    s->run_level = FAST_SHUTDOWN;
	}
    }

    return;
}


/***************************************************************************
/ FUNCTION NAME: Do_start_thread(struct client_t *c)
/
/ FUNCTION: Handle thread startup
/
/ AUTHOR:  Brian Kelly, 4/23/99
/
/ ARGUMENTS: 
/	struct client_t *c - structure containing all thread info
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_start_thread(struct client_t *c)
{
    int error;
    char error_msg[SZ_ERROR_MESSAGE], string_buffer[SZ_LINE];
    char *out;
    struct sigaction sigAct;
    struct event_t *eventPtr;

	/* set the thread's process id */
    Do_lock_mutex(&c->realm->realm_lock);
    c->game->clientPid = getpid();
    Do_unlock_mutex(&c->realm->realm_lock);

	/* record this connection */
    sprintf(c->connection_id, "?:%s:%d", c->IP, c->game->clientPid);

    sprintf(error_msg, "[%s] Connection on socket %d.\n",
	    c->connection_id, c->socket);

    Do_log(CONNECTION_LOG, error_msg);

	/* set the socket so I/O notifies this thread */
    if (fcntl(c->socket, F_SETOWN, c->game->clientPid) < 0) {

	sprintf(error_msg,
		"Thread %d returned an error on fcntl F_SETOWN.\n",
		c->socket);

	Do_log_error(error_msg);
	Do_send_error(c, error_msg);
	c->run_level = EXIT_THREAD;
	c->socket_up = FALSE;
    }

    if (fcntl(c->socket, F_SETFL, O_ASYNC) < 0) {

	sprintf(error_msg,
		"Thread %d returned an error on fcntl F_SETFL.\n",
		c->socket);

	Do_log_error(error_msg);
	Do_send_error(c, error_msg);
	c->run_level = EXIT_THREAD;
	c->socket_up = FALSE;
    }

	/* handshake the client */
    Do_handshake(c);

	/* see if this socket should be rejected */
    Do_check_tags(c);

	/* modify stats from this ip - (checks for excessive connections) */
    Do_tally_ip(c, TRUE, 0);

	    /* find all the current players and list them */
    Do_lock_mutex(&c->realm->realm_lock);
    Do_starting_spec(c);
    c->game->sendEvents = TRUE;
    Do_unlock_mutex(&c->realm->realm_lock);

	/* call the main thread loop */
    Do_thread_loop(c);

	/* clean up any events we may have */
    Do_lock_mutex(&c->realm->realm_lock);
    c->game->sendEvents = FALSE;
    Do_unlock_mutex(&c->realm->realm_lock);
    Do_check_events_in(c);

	/* pull death name out of limbo if necessary */
    if (c->previousName[0] != '\0') {
	Do_release_name(c, c->previousName);
    }

	/* clean up socket connection */
    if (c->socket_up) {
        Do_send_int(c, CLOSE_CONNECTION_PACKET);
        Do_send_buffer(c);
    }

        /* log the connection */
    sprintf(error_msg, "[%s] Leaving the game after %d seconds.\n",
	    c->connection_id, time(NULL) - c->date_connected);

    Do_log(CONNECTION_LOG, &error_msg);

	/* tell the server to clean up */
    c->game->cleanup_thread = TRUE;
    free((void *) c);
    kill(c->realm->serverPid, SIGUSR1);

        /* leave the game */
    pthread_exit(0);
}


/***************************************************************************
/ FUNCTION NAME: Do_thread_loop(struct client_t *c)
/
/ FUNCTION: The main loop for threads
/
/ AUTHOR:  Brian Kelly, 4/23/99
/
/ ARGUMENTS: 
/	struct client_t *c - structure containing all thread info
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_thread_loop(struct client_t *c)
{
    char error_msg[SZ_ERROR_MESSAGE];

	/* start the main loop */
    while (c->run_level != EXIT_THREAD) {

 	    /* make characters log in first */
	if (c->run_level == SIGNING_IN) {

	        /* print out the title page */
	    Do_title_page(c);

		/* have the user log in */
	    Do_get_account(c);

		/* see if the game is shutting down */
	    Do_shutdown_check(c);
	}

 	    /* if the player has no character - get one */
	if (c->run_level == CHAR_SELECTION) {

		/* empty out the player strcture */
	    Do_init_player(c);

		/* Get a character */
	    Do_get_character(c);
	}

	if (c->run_level == PLAY_GAME) {

		/* approve the entrance */
	    Do_approve_entrance(c);
	}

		/* if ready to play, enter the game */
	if (c->run_level == PLAY_GAME) {

		/* announce character entrance */
	    Do_entering_character(c);

		/* play the game */
	    Do_play_loop(c);

		/* remove the character from play */
	    Do_leaving_character(c);
	}

	    /* see if the character needs to be saved */
	if (c->run_level == SAVE_AND_CONTINUE ||
		c->run_level == SAVE_AND_EXIT) {

	    Do_handle_save(c);
	}

	    /* regardless, remove the backup */
	else {
	    Do_backup_save(c, FALSE);
	}

	    /* ask if player wishes to go again */
	if (c->run_level == GO_AGAIN) {

		/* ask if the player wants to continue */
	    Do_ask_continue(c);
	}

	    /* see if the game is shutting down */
	Do_shutdown_check(c);
    }
    return;
}


/***************************************************************************
/ FUNCTION NAME: Do_play_loop(struct client_t *c)
/
/ FUNCTION: Default activity when no events are pending
/
/ AUTHOR:  Brian Kelly, 4/23/99
/
/ ARGUMENTS: 
/	struct client_t *c - structure containing all thread info
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_play_loop(struct client_t *c)
{
    struct event_t *event_ptr;

    while (c->run_level == PLAY_GAME 
           || (c->events != NULL && c->player.energy > 0)) {

	    /* age the player */
	Do_age(c);

	    /* if there are events to handle */
	if (c->events != NULL) {

		/* remove the next event */
	    event_ptr = c->events;
	    c->events = event_ptr->next_event;

		/* take care of it */
	    Do_handle_event(c, event_ptr);
	}
	else {

		/* If no events, start a new turn */
	    Do_game_turn(c);

	        /* check for monsters, guru's, etc. */
	    if (!c->player.cloaked 
                && (c->wizard < 3) 
                && (c->player.location == PL_REALM || c->player.location == PL_EDGE)
                && (c->run_level == PLAY_GAME))

	        Do_random_events(c);
        }

	    /* file any events from outside the thread */
	Do_check_events_in(c);

	    /* see if the game is shutting down */
	Do_shutdown_check(c);

	    /* if the socket is down, leave the game */
	if (!c->socket_up && c->run_level == PLAY_GAME) {
	    c->run_level = SAVE_AND_EXIT;
	}
    }

    return;
}


/***************************************************************************
/ FUNCTION NAME: Do_game_turn(struct client_t *c)
/
/ FUNCTION: Default activity when no events are pending
/
/ AUTHOR:  E. A. Estes, 12/4/85
/	   Brian Kelly, 5/8/99
/
/ ARGUMENTS: 
/	struct client_t *c - structure containing all thread info
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_game_turn(struct client_t *c)
{
    struct event_t *event_ptr;
    struct button_t buttons;                     /* input */
    long ch;
    int rc;
    int loop;
    char string_buffer[SZ_LINE];
    double dtemp;
    bool postFlag;
    float ftemp;

    if (c->player.location == PL_EDGE) {

	Do_send_line(c,
	   "An old wooden sign here says, 'This is the edge of the realm.\n");

	Do_send_line(c, "I would strongly advise going no further.'\n");
    }

	/* see if the player is on a trading post  and not cloaked */
    postFlag = FALSE;
    if (fabs(c->player.x) == fabs(c->player.y) && c->player.location !=
                PL_THRONE && !c->player.cloaked) {

        dtemp = sqrt(fabs(c->player.x)/100.0);

        if (floor(dtemp) == dtemp) {
	    postFlag = TRUE;
	    Do_send_line(c, "You stand in front of a trading post.\n");
        }
    }

    Do_clear_buttons(&buttons, 0);

/*
    if (c->player.special_type != SC_VALAR) {
*/
	strcpy(buttons.button[0], "Rest\n");
/*
    }
*/

        /* print status line */
    strcpy(buttons.button[1], "Move To\n");
    buttons.compass = TRUE;
    strcpy(buttons.button[2], "Info\n");
    strcpy(buttons.button[7], "Quit\n");
/*
examine
rest
do nothing
*/
    if (postFlag) {
        strcpy(buttons.button[3], "Enter Post\n");
    } else if (!c->player.cloaked && c->player.location == PL_REALM) {
        strcpy(buttons.button[3], "Hunt\n");
    }

    if (c->player.cloaked) {
        strcpy(buttons.button[4], "Uncloak\n");
    }
    else if (c->player.level >= MEL_CLOAK && c->player.magiclvl >= ML_CLOAK) {
        strcpy(buttons.button[4], "Cloak\n");
    }

/*
    if (c->player.level >= MEL_TELEPORT && c->player.magiclvl >= ML_TELEPORT &&
	    c->player.special_type != SC_VALAR) {
*/
    if (c->player.level >= MEL_TELEPORT && c->player.magiclvl >= ML_TELEPORT &&
	    !c->player.cloaked) {

        strcpy(buttons.button[5], "Teleport\n");
    }


    if (c->wizard == 2) {
        strcpy(buttons.button[6], "Cantrip\n");
    }
    else if (c->wizard > 3) {
        strcpy(buttons.button[6], "Administrate\n");
    }
    else if (c->wizard) {
        strcpy(buttons.button[6], "Moderate\n");
    }
    else if (c->player.location == PL_THRONE && c->player.special_type
	    == SC_STEWARD && !c->player.cloaked) {

        strcpy(buttons.button[6], "Enact\n");
    }
    else if (c->player.location == PL_THRONE && c->player.special_type
	    == SC_KING && !c->player.cloaked) {

        strcpy(buttons.button[6], "Decree\n");
    }
    else if ((c->player.special_type == SC_COUNCIL || c->player.special_type
	== SC_EXVALAR) && !c->player.cloaked) {

        strcpy(buttons.button[6], "Intervene\n");
    }
    else if (c->player.special_type == SC_VALAR && !c->player.cloaked) {
        strcpy(buttons.button[6], "Command\n");
    }
    else if (c->player.level < 10) {
        strcpy(buttons.button[6], "Help\n");
    }

    if (Do_buttons(c, &ch, &buttons) != S_NORM) {
        Do_send_clear(c);
	return;
    }

    Do_send_clear(c);


	/* if the valar asks to move, change his request to a null */
/*
    if (c->player.special_type == SC_VALAR && (ch == 0 || (ch >= 8 &&
	    ch != 12))) { 

	ch = 127;	
    }
*/

	/* create an event to handle the player action */
    event_ptr = (struct event_t *) Do_create_event();
    event_ptr->to = c->game;
    event_ptr->from = c->game;

    switch (ch) {

    case 8:               /* move north-west */
	event_ptr->type = MOVE_EVENT;
	event_ptr->arg1 = c->player.x - Do_anglemove(c);
	event_ptr->arg2 = c->player.y + Do_anglemove(c);
	event_ptr->arg3 = A_SPECIFIC;
	break;

    case 9:               /* move up/north */
	event_ptr->type = MOVE_EVENT;
	event_ptr->arg1 = c->player.x;
	event_ptr->arg2 = c->player.y + Do_maxmove(c);
	event_ptr->arg3 = A_SPECIFIC;
	break;

    case 10:               /* move north-east */
	event_ptr->type = MOVE_EVENT;
	event_ptr->arg1 = c->player.x + Do_anglemove(c);
	event_ptr->arg2 = c->player.y + Do_anglemove(c);
	event_ptr->arg3 = A_SPECIFIC;
	break;

    case 11:               /* move left/west */
	event_ptr->type = MOVE_EVENT;
	event_ptr->arg1 = c->player.x - Do_maxmove(c);
	event_ptr->arg2 = c->player.y;
	event_ptr->arg3 = A_SPECIFIC;
	break;

    case 13:               /* move right/east */
	event_ptr->type = MOVE_EVENT;
	event_ptr->arg1 = c->player.x + Do_maxmove(c);
	event_ptr->arg2 = c->player.y;
	event_ptr->arg3 = A_SPECIFIC;
	break;

    case 14:               /* move south-west */
	event_ptr->type = MOVE_EVENT;
	event_ptr->arg1 = c->player.x - Do_anglemove(c);
	event_ptr->arg2 = c->player.y - Do_anglemove(c);
	event_ptr->arg3 = A_SPECIFIC;
	break;

    case 15:               /* move down/south */
	event_ptr->type = MOVE_EVENT;
	event_ptr->arg1 = c->player.x;
	event_ptr->arg2 = c->player.y - Do_maxmove(c);
	event_ptr->arg3 = A_SPECIFIC;
	break;

    case 16:               /* move south-west */
	event_ptr->type = MOVE_EVENT;
	event_ptr->arg1 = c->player.x + Do_anglemove(c);
	event_ptr->arg2 = c->player.y - Do_anglemove(c);
	event_ptr->arg3 = A_SPECIFIC;
	break;

    case 1:               /* move */

        if (Do_coords_dialog(c, &event_ptr->arg1, &event_ptr->arg2,
	    "Where do you wish to move to?\n")) {

	    break;
	}

        Do_distance(c->player.x, event_ptr->arg1, c->player.y,
		event_ptr->arg2, &dtemp);

        if (dtemp > Do_maxmove(c)) {
            Do_send_line(c, "That's too far to move in 1 step.  Get closer by using the compass buttons in the lower-right corner.\n");
	    Do_more(c);
	    Do_send_clear(c);
	}
        else {
	    event_ptr->type = MOVE_EVENT;
	    event_ptr->arg3 = A_SPECIFIC;
        }

        break;

	/* get information */
    case 2:
	event_ptr->type = INFORMATION_EVENT;
	break;

    case 7:               /* good-bye */
	c->run_level = SAVE_AND_CONTINUE;
	break;

    case 4:               /* cloak */

        if (c->player.cloaked || (c->player.level >= MEL_CLOAK &&
		c->player.magiclvl >= ML_CLOAK)) {

	    event_ptr->type = CLOAK_EVENT;
	}
	break;

    case 5:		    /* teleport */
	event_ptr->type = TELEPORT_EVENT;
        event_ptr->arg2 = FALSE;        /* not Gwaihir */
	if (c->player.location == PL_THRONE) {
	    event_ptr->arg3 = FALSE;	/* teleport costs no mana */
	}
	else {
	    event_ptr->arg3 = TRUE;	/* teleport costs mana */
	}
	break;

    case 6:               /* decree and intervention */

            /* cantrips handled first since all other actions are available
               off the cantrip menu */
	if (c->wizard == 2) {
	    event_ptr->type = CANTRIP_EVENT;
	}
	else if (c->wizard > 3) {
	    event_ptr->type = ADMINISTRATE_EVENT;
	}
	else if (c->wizard) {
	    event_ptr->type = MODERATE_EVENT;
	}
	    /* see if the player is allowed to enact */
        else if (c->player.special_type == SC_STEWARD && c->player.location
		== PL_THRONE && !c->player.cloaked) {

	    event_ptr->type = ENACT_EVENT;	
	}
	    /* see if the player is allowed to decree */
        else if (c->player.special_type == SC_KING && c->player.location
		== PL_THRONE && !c->player.cloaked) {

	    event_ptr->type = DECREE_EVENT;	
	}
        else if ((c->player.special_type == SC_COUNCIL || 
	    c->player.special_type == SC_EXVALAR) && !c->player.cloaked) {

	    event_ptr->type = INTERVENE_EVENT;
	}
        else if (c->player.special_type == SC_VALAR && !c->player.cloaked) {
	    event_ptr->type = COMMAND_EVENT;
	}
        else if (c->player.level < 10) {
	    event_ptr->type = HELP_EVENT;
        }
        break;

    case 3:               /* hunt */
	
            /* the valar cannot call monsters and no monsters on throne */
	    /* being cloaked is right out too */
	if (postFlag) {
            event_ptr->type = TRADING_EVENT;
	} else if (!c->player.cloaked && c->player.location == PL_REALM) {

	    Do_sin(c, .001);
            event_ptr->type = MONSTER_EVENT;
	    event_ptr->arg1 = MONSTER_CALL;
	    event_ptr->arg3 = SM_RANDOM;	/* pick a monster normally */
        }
        break;

    case 0:
    case 12:		    /* rest */
	event_ptr->type = REST_EVENT;
        break;

/*    default: NULL_EVENT	stupid people deserve no reward */
    }

    if (event_ptr->type != NULL_EVENT) {
	Do_handle_event(c, event_ptr);
    }
    else {
	free((void *)event_ptr);
    }

    return;
}


/***************************************************************************
/ FUNCTION NAME: Do_random_events(struct client_t *c)
/
/ FUNCTION: Default activity when no events are pending
/
/ AUTHOR:  Brian Kelly, 8/17/99
/
/ ARGUMENTS: 
/	struct client_t *c - structure containing all thread info
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_random_events(struct client_t *c)
{
    struct event_t *event_ptr;

        /* to shred "backups" */
        if ((c->player.quickness == 0)
             && (RND() >= .1 * c->player.degenerated)) {
	    event_ptr = (struct event_t *)Do_create_event();
	    event_ptr->type = MONSTER_EVENT;
	    event_ptr->arg1 = MONSTER_RANDOM;
	    event_ptr->arg3 = 16;
	    Do_file_event(c, event_ptr);
        }
            
        if (c->player.blind && RND() <= 0.0075) {
	    Do_send_line(c, "You've regained your sight!\n");
            c->player.blind = FALSE;

	        /* update the player description */
	    Do_lock_mutex(&c->realm->realm_lock);
	    Do_player_description(c);
	    Do_unlock_mutex(&c->realm->realm_lock);

	    Do_more(c);
	    Do_send_clear(c);
        }

        if (RND() <= 0.0133) {
	    Do_send_self_event(c, MEDIC_EVENT);
	}

        if (RND() <= 0.0075) {
	    Do_send_self_event(c, GURU_EVENT);
	}

        if (RND() <= 0.005) {
	    event_ptr = (struct event_t *)Do_create_event();
	    event_ptr->type = PLAGUE_EVENT;
	    event_ptr->arg3 = 0;
	    Do_file_event(c, event_ptr);
	}

        if (RND() <= 0.0075) {
	    Do_send_self_event(c, VILLAGE_EVENT);
	}

        if (c->player.level < 3000) {
            if (RND() <= 0.0033 + (c->player.level * .00000125)) {
	        Do_send_self_event(c, TAX_EVENT);
	    }
        } else if (RND() <= 0.0033) {
	    Do_send_self_event(c, TAX_EVENT);
	}

        if (RND() <= 0.015) {
	    event_ptr = (struct event_t *)Do_create_event();
	    event_ptr->type = TREASURE_EVENT;
	    event_ptr->arg1 = c->player.circle;
	    event_ptr->arg3 = 1;
	    Do_file_event(c, event_ptr);
        }

        if (RND() <= 0.0075) {
	    event_ptr = (struct event_t *)Do_create_event();
	    event_ptr->type = TREASURE_EVENT;
	    event_ptr->arg1 = c->player.circle;
	    event_ptr->arg3 = 2;
	    Do_file_event(c, event_ptr);
        }

        if (RND() <= 0.20) {
	    event_ptr = (struct event_t *)Do_create_event();
	    event_ptr->type = MONSTER_EVENT;
	    event_ptr->arg1 = MONSTER_RANDOM;
	    event_ptr->arg3 = SM_RANDOM;
	    Do_file_event(c, event_ptr);
        }
}


