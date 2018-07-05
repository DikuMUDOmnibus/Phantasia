/*
 * event.c - routines to handle event matters for Phantasia
 */

#include "include.h"

/************************************************************************
/
/ FUNCTION NAME: Do_create_event(void)
/
/ FUNCTION: To create event structures
/
/ AUTHOR: Brian Kelly, 4/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE:
/       event_t - pointer to the new event
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

struct event_t *Do_create_event(void)
{
    struct event_t *temp_event_ptr;

        /* create an event structure */
    temp_event_ptr = (struct event_t *)Do_malloc(SZ_EVENT);

	/* initialize event information */
    temp_event_ptr->type = NULL_EVENT;
    temp_event_ptr->arg1 = 0.0;
    temp_event_ptr->arg2 = 0.0;
    temp_event_ptr->arg3 = 0;
    temp_event_ptr->arg4 = NULL;
    temp_event_ptr->next_event = NULL;
    temp_event_ptr->from = NULL;
    temp_event_ptr->to = NULL;

	/* return the address of the new event */
    return temp_event_ptr;
}


/************************************************************************
/
/ FUNCTION NAME: Do_duplicate_event(struct event_t *event)
/
/ FUNCTION: To duplicate event structures
/
/ AUTHOR: Brian Kelly, 4/12/99
/
/ ARGUMENTS:
/       event_t *orig - the source event
/
/ RETURN VALUE:
/       event_t - pointer to the new event
/
/ MODULES CALLED: wmove(), _filbuf(), clearok(), waddstr(), wrefresh(),
/       wclrtoeol()
/
/ GLOBAL INPUTS: Echo, _iob[], Wizard, *stdscr
/
/ GLOBAL OUTPUTS: _iob[]
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

struct event_t *Do_duplicate_event(struct event_t *the_event)
{
    struct event_t *event_ptr;

        /* create a duplicate event structure */
    event_ptr = (struct event_t *)Do_create_event();

	/* duplicate event information */
    memcpy((void *)event_ptr, (void *)the_event, SZ_EVENT);

	/* copy an attached object, if necessary */
    if (the_event->arg4 != NULL) {
	event_ptr->arg4 = (void *) Do_malloc((int)the_event->arg3);
        memcpy(event_ptr->arg4, the_event->arg4, (int)the_event->arg3);
    }

	/* return the address of the new event */
    return event_ptr;
}


/************************************************************************
/
/ FUNCTION NAME: Do_send_event(event_t *event_ptr)
/
/ FUNCTION: Send an event to its proper destination
/
/ AUTHOR: Brian Kelly, 4/12/99
/
/ ARGUMENTS:
/       event_t *event - the event to be placed
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
/           - strip non-printing characters (unless Wizard)
/           - echo, if desired
/           - redraw the screen if CH_REDRAW is entered
/           - read in only 'mx - 1' characters or less characters
/           - nul-terminate string, and throw away newline
/
/       'mx' is assumed to be at least 2.
/
*************************************************************************/

void Do_send_event(struct event_t *the_event)
{
/*
    struct event_t **event_ptr_ptr;
*/

	/* lock the destination event queue */
    Do_lock_mutex(&the_event->to->events_in_lock);

        /* point the current list after this event */
    the_event->next_event = the_event->to->events_in;

        /* point the list to start with this event */
    the_event->to->events_in = the_event;

	/* move to the last event in the queue */
/*
    event_ptr_ptr = &the_event->to->events_in;
    while (*event_ptr_ptr != NULL) {
	event_ptr_ptr = &(*event_ptr_ptr)->next_event;
    }
*/

	/* point the last event at this new one */
/*
    *event_ptr_ptr = the_event;
*/

	/* send an interrupt to the thread */
    pthread_kill(the_event->to->the_thread, SIGIO);

	/* unlock the destination event queue */
    Do_unlock_mutex(&the_event->to->events_in_lock);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_send_self_event(struct client_t *c, int type)
/
/ FUNCTION: Send an event to its proper destination
/
/ AUTHOR: Brian Kelly, 5/8/99
/
/ ARGUMENTS:
/       event_t *event - the event to be placed
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
/           - strip non-printing characters (unless Wizard)
/           - echo, if desired
/           - redraw the screen if CH_REDRAW is entered
/           - read in only 'mx - 1' characters or less characters
/           - nul-terminate string, and throw away newline
/
/       'mx' is assumed to be at least 2.
/
*************************************************************************/

Do_send_self_event(struct client_t *c, int type)
{
    struct event_t *event_ptr;

	/* create a new event */
    event_ptr = (struct event_t *) Do_create_event();

	/* fill out the necessary information */
    event_ptr->type = type;
    event_ptr->from = c->game;
    event_ptr->to = c->game;

	/* send off the event */
    Do_file_event(c, event_ptr);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_broadcast_event()
/
/ FUNCTION: Send an event to every game 
/
/ AUTHOR: Brian Kelly, 4/26/99
/
/ ARGUMENTS:
/	struct client_t c - a pointer to the client strcture
/       struct event_t *event - the event to be placed
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
/           - strip non-printing characters (unless Wizard)
/           - echo, if desired
/           - redraw the screen if CH_REDRAW is entered
/           - read in only 'mx - 1' characters or less characters
/           - nul-terminate string, and throw away newline
/
/       'mx' is assumed to be at least 2.
/
*************************************************************************/

Do_broadcast_event(struct client_t *c, struct event_t *the_event)
{
    struct game_t *game_ptr;
    struct event_t *event_ptr;

	/* lock the linked list of game */
    Do_lock_mutex(&c->realm->realm_lock);

	/* run thorough all the games */
    game_ptr = c->realm->games;
    while (game_ptr != NULL) {

	    /* if we're not point to ourselves and this game hears */
	if (game_ptr != c->game && game_ptr->sendEvents) {

		/* duplicate the event */
	    event_ptr = Do_duplicate_event(the_event);

		/* address the event to the current game */
	    event_ptr->to = game_ptr;

		/* send off the event */
	    Do_send_event(event_ptr);
	}

	    /* move to the next game */
	game_ptr = game_ptr->next_game;
    }

	/* unlock the linked list of games */
    Do_unlock_mutex(&c->realm->realm_lock);

	/* send the original event to this game */
    the_event->to = c->game;
    Do_file_event(c, the_event);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_file_event(struct client_t *c, struct event_t *event_ptr)
/
/ FUNCTION: Send an event from a client
/
/ AUTHOR: Brian Kelly, 4/24/99
/
/ ARGUMENTS:
/       client_t *c - the pointer to the main client strcture
/       event_t *event - the event to be placed
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
/           - strip non-printing characters (unless Wizard)
/           - echo, if desired
/           - redraw the screen if CH_REDRAW is entered
/           - read in only 'mx - 1' characters or less characters
/           - nul-terminate string, and throw away newline
/
/       'mx' is assumed to be at least 2.
/
*************************************************************************/

Do_file_event(struct client_t *c, struct event_t *the_event)
{
    struct event_t *event_ptr, **event_ptr_ptr;
    struct player_info_t *pinfo_ptr;
    bool flag;
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE];
    FILE *ban_file;

	/* switch on the kind of event */
    switch (the_event->type) {

        /* kick this player off the game NOW */
    case KICK_EVENT:

        strcpy(error_msg, the_event->arg4);
        strcat(error_msg, " has kicked you out of the game.\n");

            /* send an error packet */
        Do_send_error(c, error_msg);

            /* accept no more input from this player */
        c->socket_up = FALSE;

            /* get out of the game ASAP */
        c->run_level = EXIT_THREAD;

            /* delete the event */
        free(the_event->arg4);
        free((void *)the_event);

            /* nothing more to do here */
        break;

	/* if the game is being tagged */
    case TAG_EVENT:
	Do_tag_self(c, (struct tag_t *) the_event->arg4);

	if(the_event->arg3 == T_MUTE)
	{
		Do_tag_muted(c, (struct tag_t *) the_event->arg4);
	}

	    /* delete the event */
	free(the_event->arg4);
	free((void *)the_event);

	break;

	/* remove a prefix or suffix tag from the game */
    case UNTAG_EVENT:
	Do_remove_prefix_suffix(c);
	free((void *)the_event);

	break;

	/* chat messages go straight to the player */
    case CHAT_EVENT:

	    /* if this message is from another channel */
	if (the_event->arg2 > 0) {

	    sprintf(string_buffer, "(%.0lf) - ", the_event->arg2);

	        /* send the message */
	    Do_send_int(c, CHAT_PACKET);
	    Do_send_string(c, string_buffer);
	    Do_send_string(c, the_event->arg4);
	    Do_send_buffer(c);

	}
	    /* unless it's an announcement and we're muted */
	    /* or if we're suspended and this isn't from a wizard or himself */
	else if (c->hearBroadcasts || !the_event->arg1) {

	        /* send it marked as such */
	    Do_send_int(c, CHAT_PACKET);
	    Do_send_string(c, the_event->arg4);
	    Do_send_buffer(c);
	}

	    /* delete the string */
	free(the_event->arg4);

	    /* delete the event */
	free((void *)the_event);

	    /* nothing more to do here */
	break;

	/* bad player, no cookie */
    case REPRIMAND_EVENT:
	
	Do_reprimand(c, the_event);
	break;

    case REQUEST_DETAIL_EVENT:

	    /* create an detail record on arg4 */
	the_event->arg4 = (void *) Do_create_detail(c);

	    /* take the event and return it with the information */
	the_event->type = CONNECTION_DETAIL_EVENT;
	the_event->to = the_event->from;
	the_event->from = c->game;

	    /* send off the information */
	Do_send_event(the_event);

	    /* all done here */
	break;

    case CONNECTION_DETAIL_EVENT:

	Do_detail_connection(c, (struct detail_t *)the_event->arg4);
	Do_send_buffer(c);

	    /* delete the event */
	free((void *) the_event->arg4);
	free((void *) the_event);

	    /* that's all folkes */
	break;

    case REQUEST_RECORD_EVENT:

            /* respond only if we're still in the game */
        if (c->game->description == NULL) {
            break;
        }

	    /* create an examine record on arg4 */
	the_event->arg4 = (void *) Do_create_examine(c, the_event->from);

	    /* take the event and return it with the information */
	the_event->type = PLAYER_RECORD_EVENT;
	the_event->to = the_event->from;
	the_event->from = c->game;

	    /* send off the information */
	Do_send_event(the_event);

	    /* all done here */
	break;

    case PLAYER_RECORD_EVENT:

	Do_examine_character(c, (struct examine_t *)the_event->arg4);
	Do_send_buffer(c);

	    /* delete the event */
	free((void *) the_event->arg4);
	free((void *) the_event);

	    /* that's all folkes */
	break;

    case ADD_PLAYER_EVENT:

	    /* send the packet */
	Do_add_player(c, (struct player_spec_t *)the_event->arg4);
	Do_send_buffer(c);

	    /* delete the event */
	free((void *) the_event->arg4);
	free((void *) the_event);

	    /* Nothing else to do */
	break;

    case REMOVE_PLAYER_EVENT:

	    /* send the packet */
	Do_remove_player(c, ((struct player_spec_t *)the_event->arg4)->name);
	Do_send_buffer(c);

	    /* delete the event */
	free((void *) the_event->arg4);
	free((void *) the_event);


	    /* Nothing else to do */
	break;

    case CHANGE_PLAYER_EVENT:

	    /* send the packet */
	Do_add_player(c, (struct player_spec_t *)the_event->arg4);
	Do_remove_player(c, ((struct player_spec_t *)the_event->arg4)->name);
	Do_send_buffer(c);

	    /* delete the event */
	free((void *) the_event->arg4);
	free((void *) the_event);

	    /* Nothing else to do */
	break;

	/* remove a player from suspended status */
    case UNSUSPEND_EVENT:

	    /* if this character is suspended */
        if (c->suspended) {

            sprintf(string_buffer, "You have been released by %s.\n",
		    the_event->arg4);

	    Do_send_line(c, string_buffer);

	        /* unsuspend the player */
	    c->suspended = FALSE;
        }

	    /* delete the event */
	free(the_event->arg4);
	free((void *)the_event);

	    /* nothing more to do here */
	break;

    case NULL_EVENT:

	sprintf(error_msg, "[%s] attempted to file a null event.\n",
		c->connection_id);

	Do_log_error(error_msg);
	free((void *) event_ptr);

	break;

	/* all other events get queued for the future */
    default:

            /* getting some impossible events - checking here */
        if (the_event->type > LAST_EVENT || the_event->type <= NULL_EVENT) {

	    sprintf(error_msg,
                    "[%s] bad event number %hd in Do_file_event.\n",
		    c->connection_id, the_event->type);

	    Do_log_error(error_msg);
	    free((void *) event_ptr);

	    break;
        }

	    /* point to the local event queue */
        event_ptr_ptr = &c->events;

	    /* find the place to put the event in */
        while (*event_ptr_ptr != NULL && (*event_ptr_ptr)->type <=
	        the_event->type) {

	    event_ptr_ptr = &(*event_ptr_ptr)->next_event;
        }

	    /* put the event in that location */
        the_event->next_event = *event_ptr_ptr;
        *event_ptr_ptr = the_event;

	break;
    }
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_file_event_first(struct client_t *c, struct event_t *event_ptr)
/
/ FUNCTION: Send an event from a client
/
/ AUTHOR: Brian Kelly, 01/21/01
/
/ ARGUMENTS:
/       client_t *c - the pointer to the main client strcture
/       event_t *event - the event to be placed
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
/           - strip non-printing characters (unless Wizard)
/           - echo, if desired
/           - redraw the screen if CH_REDRAW is entered
/           - read in only 'mx - 1' characters or less characters
/           - nul-terminate string, and throw away newline
/
/       'mx' is assumed to be at least 2.
/
*************************************************************************/

Do_file_event_first(struct client_t *c, struct event_t *the_event)
{
    struct event_t *event_ptr, **event_ptr_ptr;

	/* point to the local event queue */
    event_ptr_ptr = &c->events;

	/* find the place to put the event in */
    while (*event_ptr_ptr != NULL && (*event_ptr_ptr)->type <
	    the_event->type) {

	event_ptr_ptr = &(*event_ptr_ptr)->next_event;
    }

	/* put the event in that location */
    the_event->next_event = *event_ptr_ptr;
    *event_ptr_ptr = the_event;

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_check_events_in(struct client_t *c)
/
/ FUNCTION: Send an event from a client
/
/ AUTHOR: Brian Kelly, 4/24/99
/
/ ARGUMENTS:
/       client_t *c - the pointer to the main client strcture
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
/           - strip non-printing characters (unless Wizard)
/           - echo, if desired
/           - redraw the screen if CH_REDRAW is entered
/           - read in only 'mx - 1' characters or less characters
/           - nul-terminate string, and throw away newline
/
/       'mx' is assumed to be at least 2.
/
*************************************************************************/

Do_check_events_in(struct client_t *c)
{
    struct event_t *event_ptr, *event_ptr_two;
    struct player_spec_t *spec_ptr;
    char error_msg[SZ_ERROR_MESSAGE];

	/* lock the events_in queue */
    Do_lock_mutex(&c->game->events_in_lock);

	/* copy over all the events */
    event_ptr = c->game->events_in;
    c->game->events_in = NULL;

	/* unlock the events_in queue */
    Do_unlock_mutex(&c->game->events_in_lock);

	/* run though every event */
    while (event_ptr != NULL) {

	    /* save the next event since this one may end up deleted */
	event_ptr_two = event_ptr->next_event;

	if (event_ptr->type <= NULL_EVENT || event_ptr->type > LAST_EVENT) {

	    sprintf(error_msg,
		    "[%s] bad event number %hd in Do_check_events_in.\n",
		    c->connection_id, event_ptr->type);

	    Do_log_error(error_msg);
	    free((void *) event_ptr);
	}
	else {

	    Do_file_event(c, event_ptr);
	}

	    /* move to the next event */
	event_ptr = event_ptr_two;
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_handle_event(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: Send an event from a client
/
/ AUTHOR: Brian Kelly, 4/24/99
/
/ ARGUMENTS:
/       client_t *c - the pointer to the main client strcture
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
/           - strip non-printing characters (unless Wizard)
/           - echo, if desired
/           - redraw the screen if CH_REDRAW is entered
/           - read in only 'mx - 1' characters or less characters
/           - nul-terminate string, and throw away newline
/
/       'mx' is assumed to be at least 2.
/
*************************************************************************/

Do_handle_event(struct client_t *c, struct event_t *the_event)
{
    char error_msg[SZ_ERROR_MESSAGE];
    double dtemp;

    sprintf(error_msg, "%s: event %hd (1 = %.2lf, 2 = %.2lf, 3 = %ld)\n",
	    c->player.lcname, the_event->type, the_event->arg1, the_event->arg2,
	    the_event->arg3);

    Do_log(EVENT_LOG, &error_msg);

    switch (the_event->type) {

	/* player died */
    case DEATH_EVENT:

	Do_death(c, the_event);
	break;

    case IT_COMBAT_EVENT:

	Do_it_combat(c, the_event, TRUE);
	break;

    case EXPERIENCE_EVENT:

	Do_experience(c, the_event->arg1, FALSE);
	break;

    case SUSPEND_EVENT:

	Do_suspend(c, the_event);
	break;

    case CANTRIP_EVENT:

	Do_cantrip(c);
	break;

    case MODERATE_EVENT:

	Do_moderate(c);
	break;

    case ADMINISTRATE_EVENT:

	Do_administrate(c);
	break;

    case VALAR_EVENT:

	Do_valar(c);
	break;

    case KING_EVENT:

	Do_king(c);
	break;

    case STEWARD_EVENT:

	Do_steward(c);
	break;

    case DETHRONE_EVENT:

	Do_dethrone(c);
	break;

    case RELOCATE_EVENT:

	Do_relocate(c, the_event);
	break;

    case TRANSPORT_EVENT:

	Do_transport(c, the_event);
	break;

    case CURSE_EVENT:

	Do_curse(c, the_event);
	break;

    case SLAP_EVENT:

	Do_slap(c, the_event);
	break;
	
	case REPRIMAND_EVENT:
	
	Do_reprimand(c, the_event);
	break;
	
    case BLIND_EVENT:

	Do_blind(c, the_event);
	break;

    case DEGENERATE_EVENT:

	Do_caused_degenerate(c, the_event);
	break;

    case HELP_EVENT:

	Do_help(c);
	break;

    case BESTOW_EVENT:

	Do_bestow(c, the_event);
	break;

    case SUMMON_EVENT:

	Do_summon(c, the_event);
	break;

    case BLESS_EVENT:

	Do_bless(c, the_event);
	break;

    case HEAL_EVENT:

	Do_heal(c, the_event);
	break;

    case STRONG_NF_EVENT:

	Do_strong_nf(c, the_event);
	break;

    case SAVE_EVENT:

	c->run_level = SAVE_AND_CONTINUE;
	break;

    case SEX_CHANGE_EVENT:          

	if (c->player.gender == MALE) {
          c->player.gender = FEMALE;
	}else{
          c->player.gender = MALE;
    }
     Do_send_line(c, "You feel an odd change come over you...\n");

	break;

    case MOVE_EVENT:

	Do_move(c, the_event);
	break;

    case EXAMINE_EVENT:

	Do_examine(c, the_event);
	break;

    case DECREE_EVENT:

	Do_decree(c);
	break;

    case ENACT_EVENT:

	Do_enact(c);
	break;

    case KNIGHT_EVENT:

	Do_knight(c, the_event);
	break;

    case LIST_PLAYER_EVENT:

	Do_list_characters(c);
	break;

    case CLOAK_EVENT:

	Do_cloak(c, !c->player.cloaked, FALSE);
	break;

    case TELEPORT_EVENT:

	Do_teleport(c, the_event);
	break;

    case INTERVENE_EVENT:

	Do_intervene(c);
	break;

    case COMMAND_EVENT:

	Do_command(c);
	break;

    case REST_EVENT:

	Do_rest(c);
	break;

    case INFORMATION_EVENT:

	Do_information(c);
	break;

    case ENERGY_VOID_EVENT:

	Do_energy_void(c);
	break;

    case MONSTER_EVENT:

	Do_monster(c, the_event);
	break;

    case MEDIC_EVENT:

	Do_medic(c);
	break;

    case GURU_EVENT:

	Do_guru(c);
	break;

    case PLAGUE_EVENT:

	Do_plague(c, the_event);
	break;

    case VILLAGE_EVENT:

        if ((c->player.circle < 26) || (c->player.circle > 29)) {
	    Do_village(c);
        }
	break;

    case TRADING_EVENT:

	Do_trading_post(c);
	break;

    case TREASURE_EVENT:

	Do_treasure(c, the_event);
	break;

    case TROVE_EVENT:

	Do_treasure_trove(c);
	break;

    case TAX_EVENT:

	Do_tax(c);
	break;

    case CORPSE_EVENT:

	Do_corpse(c, the_event);
	break;

    case GRAIL_EVENT:

	Do_holy_grail(c, the_event);
	break;

    default:

        sprintf(error_msg, "[%s] bad event number %hd in Do_handle_event.\n",
                c->connection_id, the_event->type);

        Do_log_error(error_msg);
    }

    free((void *)the_event);
}


/************************************************************************
/
/ FUNCTION NAME: Do_orphan_events(struct client_t *c)
/
/ FUNCTION: Send an event from a client
/
/ AUTHOR: Brian Kelly, 8/25/99
/
/ ARGUMENTS:
/       client_t *c - the pointer to the main client strcture
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
/           - strip non-printing characters (unless Wizard)
/           - echo, if desired
/           - redraw the screen if CH_REDRAW is entered
/           - read in only 'mx - 1' characters or less characters
/           - nul-terminate string, and throw away newline
/
/       'mx' is assumed to be at least 2.
/
*************************************************************************/

Do_orphan_events(struct client_t *c)
{
    struct event_t *event_ptr;
    struct event_t **event_ptr_ptr;
    struct realm_object_t *object_ptr;

        /* handle any events in the queue */
    event_ptr_ptr = &c->events;
    while (*event_ptr_ptr != NULL) {

            /* set the event_ptr to the event pointed to */
        event_ptr = *event_ptr_ptr;

            /* if the event is a realm object */
        if (event_ptr->type > REALM_MARKER) {

                /* create a new realm object */
            object_ptr = (struct realm_object_t *) Do_malloc(SZ_REALM_OBJECT);
            object_ptr->x = c->player.x;
            object_ptr->y = c->player.y;

                /* if we're dealing with a corpse */
            if (event_ptr->type == CORPSE_EVENT) {

                    /* identify it as such */
                object_ptr->type = CORPSE;

                    /* point the object to the player_t */
                object_ptr->arg1 = event_ptr->arg4;
		
            }
            else if (event_ptr->type == GRAIL_EVENT) {

                    /* identify it as a grail object */
                event_ptr->type = HOLY_GRAIL;
            }
            else if (event_ptr->type == TROVE_EVENT) {

                    /* identify it as a grail object */
                event_ptr->type = TREASURE_TROVE;
            }

                /* put the object in the list of realm objects */
            object_ptr->next_object = c->realm->objects;
            c->realm->objects = object_ptr;
        }

	if (event_ptr->type == IT_COMBAT_EVENT) {
	    Do_it_combat(c, event_ptr, FALSE);
	}

        if (event_ptr->type > DESTROY_MARKER || event_ptr->type ==
		IT_COMBAT_EVENT) {

                /* delete normal and realm object events */
            *event_ptr_ptr = event_ptr->next_event;
            free((void *)event_ptr);
        }
	else {
                /* move to the next event */
            event_ptr_ptr = &event_ptr->next_event;
	}
    }
}


/************************************************************************
/
/ FUNCTION NAME: int Do_send_character_event(struct client_t *c, struct event_t *theEvent, char *characterName)
/
/ FUNCTION: Send an event from a client
/
/ AUTHOR: Brian Kelly, 01/03/01
/
/ ARGUMENTS:
/       client_t *c - the pointer to the main client strcture
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
/           - strip non-printing characters (unless Wizard)
/           - echo, if desired
/           - redraw the screen if CH_REDRAW is entered
/           - read in only 'mx - 1' characters or less characters
/           - nul-terminate string, and throw away newline
/
/       'mx' is assumed to be at least 2.
/
*************************************************************************/

int Do_send_character_event(struct client_t *c, struct event_t *theEvent, char *characterName)
{

    struct game_t *game_ptr;

    if(theEvent->type == UNTAG_EVENT)
	    Do_log(DEBUG_LOG, "Do_send_character_event reached with untag event\n");

        /* lock the realm */
    Do_lock_mutex(&c->realm->realm_lock);

        /* run though all the games and check the names */
    game_ptr = c->realm->games;
    while (game_ptr != NULL) {

            /* check for a name match */
        if (game_ptr->description != NULL &&
                !strcmp(characterName, game_ptr->description->name)) {

            theEvent->to = game_ptr;
            Do_send_event(theEvent);
	    Do_unlock_mutex(&c->realm->realm_lock);
	    return TRUE;
	}
            
        game_ptr = game_ptr->next_game;
    }

	/* the character was not found */
    Do_unlock_mutex(&c->realm->realm_lock);
    return FALSE;
}


/************************************************************************
/
/ FUNCTION NAME: int Do_check_encountered(struct client_t *c)
/
/ FUNCTION: Send an event from a client
/
/ AUTHOR: Brian Kelly, 05/08/01
/
/ ARGUMENTS:
/       client_t *c - the pointer to the main client strcture
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
/           - strip non-printing characters (unless Wizard)
/           - echo, if desired
/           - redraw the screen if CH_REDRAW is entered
/           - read in only 'mx - 1' characters or less characters
/           - nul-terminate string, and throw away newline
/
/       'mx' is assumed to be at least 2.
/
*************************************************************************/

int Do_check_encountered(struct client_t *c)
{
    struct event_t *event_ptr;

/* Realm must be locked when this function is called! */

/*      Don't check events in!  If examine request, we'll deadlock
    Do_check_events_in(c);
*/
    event_ptr = c->events;

    while (event_ptr != NULL) {
        if (event_ptr->type == IT_COMBAT_EVENT) {
            return TRUE;
        }
        event_ptr = event_ptr->next_event;
    }

        /* now check the event-in queue */
    Do_lock_mutex(&c->game->events_in_lock);
    event_ptr = c->game->events_in;

    while (event_ptr != NULL) {
        if (event_ptr->type == IT_COMBAT_EVENT) {
            Do_unlock_mutex(&c->game->events_in_lock);
            return TRUE;
        }
        event_ptr = event_ptr->next_event;
    }

    Do_unlock_mutex(&c->game->events_in_lock);
    return FALSE;
}

