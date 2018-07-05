/*
 * misc.c       Multiple small utility procedures
 */

#include "include.h"
extern randData;
extern server_hook;

/***************************************************************************
/ FUNCTION NAME: Do_init_mutex(pthread_mutex_t *the_mutex)
/
/ FUNCTION: Initializes the passed mutex
/
/ AUTHOR:  Brian Kelly, 4/19/99
/
/ ARGUMENTS: 
/	pthread_mutex_t the_mutex - The mutex to be initialized
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Procedure handles errors from mutex operations.
/
****************************************************************************/

Do_init_mutex(pthread_mutex_t *the_mutex)
{
    char error_msg[SZ_ERROR_MESSAGE];
    int error;

	/* initialize the passed mutex */
    error = pthread_mutex_init(the_mutex, NULL);
    if (error) {

	sprintf(error_msg, "[0.0.0.0:?] pthread_mutex_init failed with %d return code in Do_init_mutex.\n", error);

	Do_log_error(error_msg);
	exit(MUTEX_INIT_ERROR);
    }
}


/***************************************************************************
/ FUNCTION NAME: Do_destroy_mutex(pthread_mutex_t *the_mutex)
/
/ FUNCTION: Initializes the passed mutex
/
/ AUTHOR:  Brian Kelly, 4/19/99
/
/ ARGUMENTS: 
/	pthread_mutex_t the_mutex - The mutex to be destroyed
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Procedure handles errors from mutex operations.
/
****************************************************************************/

void Do_destroy_mutex(pthread_mutex_t *the_mutex)
{
    char error_msg[SZ_ERROR_MESSAGE];
    int error;

	/* destroy the passed mutex */
    error = pthread_mutex_destroy(the_mutex);
    if (error) {

	sprintf(error_msg, "[0.0.0.0:?] pthread_mutex_destroy failed with %d return code in Do_destroy_mutex.\n", error);

	Do_log_error(error_msg);
	exit(MUTEX_DESTROY_ERROR);
    }
}


/***************************************************************************
/ FUNCTION NAME: Do_lock_mutex(pthread_mutex_t *the_mutex)
/
/ FUNCTION: Locks the passed mutex
/
/ AUTHOR:  Brian Kelly, 4/19/99
/
/ ARGUMENTS: 
/	pthread_mutex_t the_mutex - The mutex to be locked
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Procedure handles errors from mutex operations.
/
****************************************************************************/

void Do_lock_mutex(pthread_mutex_t *the_mutex)
{
    char error_msg[SZ_ERROR_MESSAGE];
    int error;

#ifdef MUTEX_DEBUG
sprintf(error_msg, "Request mutex %x lock.\n", the_mutex);
Do_log(DEBUG_LOG, error_msg);
#endif

	/* lock the passed mutex */
    error = pthread_mutex_lock(the_mutex);
    if (error) {

	sprintf(error_msg, "[?:?] pthread_mutex_lock failed with %d return code in Do_lock_mutex.\n", error);

	Do_log_error(error_msg);
	exit(MUTEX_LOCK_ERROR);
    }
#ifdef MUTEX_DEBUG
sprintf(error_msg, "Mutex %x locked.\n", the_mutex);
Do_log(DEBUG_LOG, error_msg);
#endif
}


/***************************************************************************
/ FUNCTION NAME: Do_unlock_mutex(pthread_mutex_t *the_mutex)
/
/ FUNCTION: Unlocks the passed mutex
/
/ AUTHOR:  Brian Kelly, 4/19/99
/
/ ARGUMENTS: 
/	pthread_mutex_t the_mutex - The mutex to be unlocked
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Procedure handles errors from mutex operations.
/
****************************************************************************/

void Do_unlock_mutex(pthread_mutex_t *the_mutex)
{
    char error_msg[SZ_ERROR_MESSAGE];
    int error;

	/* unlock the passed mutex */
    error = pthread_mutex_unlock(the_mutex);
    if (error) {

	sprintf(error_msg, "[?:?] pthread_mutex_unlock failed with a %d return code in Do_unlock_mutex.\n", error);

	Do_log_error(error_msg);
	exit(MUTEX_UNLOCK_ERROR);
    }
#ifdef MUTEX_DEBUG
sprintf(error_msg, "Mutex %x unlocked.\n", the_mutex);
Do_log(DEBUG_LOG, error_msg);
#endif
}


/***************************************************************************
/ FUNCTION NAME: void *Do_malloc(struct size_t the_size)
/
/ FUNCTION: Returns the pointer to a new memory space
/
/ AUTHOR:  Brian Kelly, 4/20/99
/
/ ARGUMENTS:
/	struct size_t the_size - The size of the memory space to allocate
/
/ RETURN VALUE: 
/	void * - pointer to the new memory space
/
/ DESCRIPTION:
/       Procedure handles errors for malloc calls.
/
****************************************************************************/

void *Do_malloc(size_t the_size)
{
    void *void_ptr;
    char error_msg[SZ_ERROR_MESSAGE];

	/* create the new memory space */
    void_ptr = malloc(the_size);
    if (void_ptr == NULL) {

	sprintf(error_msg, "[?:?] malloc failed on size %d in Do_malloc.\n",
		the_size);

	Do_log_error(error_msg);
	exit(MALLOC_ERROR);
    }
}


/************************************************************************
/
/ FUNCTION NAME: int Do_random(void)
/
/ FUNCTION: return a random floating point number from 0.0 < 1.0
/
/ AUTHOR: E. A. Estes, 2/7/86
/	  Brian Kelly, 4/20/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: random()
/
/ GLOBAL INPUTS: none
/
/ GLOBAL OUTPUTS: none
/
/ DESCRIPTION:
/       Convert random integer from library routine into a floating
/       point number, and divide by the largest possible random number.
/       We mask large integers with 32767 to handle sites that return
/       31 bit random integers.
/
*************************************************************************/

int Do_random()
{
    char error_msg[SZ_ERROR_MESSAGE];
    int error, result;

    if (error = random_r((struct random_data *)&randData, &result)) {

	sprintf(error_msg,
		"[?:?] random_r failed with error code of %d in Do_random.\n",
		error);

	Do_log_error(error_msg);
    }

	/* return an unsigned 4 byte number */
    return result;
}

/************************************************************************
/
/ FUNCTION NAME: Do_name_location(struct client_t *c)
/
/ FUNCTION: return a formatted description of location
/
/ AUTHOR: E. A. Estes, 12/4/85
/
/ ARGUMENTS:
/       struct player playerp - pointer to player structure
/       bool shortflag - set if short form is desired
/
/ RETURN VALUE: pointer to string containing result
/
/ MODULES CALLED: fabs(), floor(), sprintf(), distance()
/
/ DESCRIPTION:
/       Look at coordinates and return an appropriately formatted
/       string.
/
*************************************************************************/

Do_name_location(struct client_t *c)
{
int    quadrant;       /* quandrant of grid */
char	error_msg[SZ_ERROR_MESSAGE];
char     *nametable[4][4] =   /* names of places */
        {
        "Anorien",      "Ithilien",     "Rohan",        "Lorien",
        "Gondor",       "Mordor",       "Dunland",      "Rovanion",
        "South Gondor", "Khand",        "Eriador",      "The Iron Hills",
        "Far Harad",    "Near Harad",   "The Northern Waste", "Rhun"
        };

    if (c->player.location == PL_REALM) {

        if (c->player.beyond) {
            strcpy(c->player.area, "The Point of No Return");
	}
	else if (c->player.circle >= 400.0) 
            strcpy(c->player.area, "The Ashen Mountains");
        else if (c->player.circle >= 100)
            strcpy(c->player.area, "Kennaquahir");
        else if (c->player.circle >= 36)
            strcpy(c->player.area, "Morannon");
        else if ((c->player.circle == 27) || (c->player.circle == 28)) 
            strcpy(c->player.area, "The Cracks of Doom");
        else if ((c->player.circle > 24) && (c->player.circle < 31)) 
            strcpy(c->player.area, "The Plateau of Gorgoroth");
        else if (c->player.circle >= 20)
            strcpy(c->player.area, "The Dead Marshes");
        else if (c->player.circle >= 10)
            strcpy(c->player.area, "The Outer Waste");
        else if (c->player.circle >= 5)
            strcpy(c->player.area, "The Moors Adventurous");
        else {

    /* this expression is split to prevent compiler loop with some compilers */
            quadrant = ((c->player.x > 0.0) ? 1 : 0);
            quadrant += ((c->player.y >= 0.0) ? 2 : 0);

            strcpy(c->player.area,
		    nametable[((int) c->player.circle) - 1][quadrant]);
        }
    }
    else if (c->player.location == PL_THRONE) {
        strcpy(c->player.area, "The Lord's Chamber");
    }
    else if (c->player.location == PL_EDGE) {
        strcpy(c->player.area, "Edge Of The Realm");
    }
    else if (c->player.location == PL_VALHALLA) {
        strcpy(c->player.area, "Valhalla");
    }
    else if (c->player.location == PL_PURGATORY) {
        strcpy(c->player.area, "Purgatory");
    }
	/* no other places to be */
    else {
        strcpy(c->player.area, "State Of Insanity");

        sprintf(error_msg,
		"[%s] Bad c->player.area of %hd in Do_name_location.\n",
		c->connection_id, c->player.area);
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_truncstring()
/
/ FUNCTION: truncate trailing blanks off a string
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/4/99
/
/ ARGUMENTS:
/       char *string - pointer to null terminated string
/
/ RETURN VALUE: none
/
/ MODULES CALLED: strlen()
/
/ DESCRIPTION:
/       Put nul characters in place of spaces at the end of the string.
/
*************************************************************************/

Do_truncstring(char *string)
{
    int length;         /* length of string */

    length = strlen(string);
    while (length && !isgraph(string[--length]))
        string[length] = '\0';
}


/************************************************************************
/
/ FUNCTION NAME: Do_distance()
/
/ FUNCTION: calculate distance between two points
/
/ AUTHOR: E. A. Estes, 12/4/85
/         Brian Kelly, 6/20/99
/
/ ARGUMENTS:
/       double x1, y1 - x, y coordinates of first point
/       double x2, y2 - x, y coordinates of second point
/
/ RETURN VALUE: distance between the two points
/
/ MODULES CALLED: sqrt()
/
/ GLOBAL INPUTS: none
/
/ GLOBAL OUTPUTS: none
/
/ DESCRIPTION:
/       This function is provided because someone's hypot() library function
/       fails if x1 == x2 && y1 == y2.
/
*************************************************************************/

Do_distance(double x1, double x2, double y1, double y2, double *answer)
{
    double  deltax, deltay;

    deltax = x1 - x2;
    deltay = y1 - y2;

    *answer = sqrt(deltax * deltax + deltay * deltay);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_move(struct client_t c, float new_x, float new_y, int operation)
/
/ FUNCTION: truncate trailing blanks off a string
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/8/99
/
/ ARGUMENTS:
/       char *string - pointer to null terminated string
/
/ RETURN VALUE: none
/
/ MODULES CALLED: strlen()
/
/ DESCRIPTION:
/       Put nul characters in place of spaces at the end of the string.
/
*************************************************************************/

Do_move(struct client_t *c, struct event_t *the_event)
{
    struct event_t *event_ptr, *event_ptr_two, **event_ptr_ptr;
    struct realm_object_t *object_ptr, **object_ptr_ptr;
    struct game_t *game_ptr;
    double dtemp, x = 0, y = 0;
    double distance;
    bool pause = FALSE;
    char error_msg[SZ_ERROR_MESSAGE];
    long answer;
    short oldLocation;

	/* determine the destination coordinates */
    switch (the_event->arg3) {

    case A_NEAR:
	the_event->arg1 = c->player.x;
	the_event->arg2 = c->player.y;
	dtemp = (double) Do_maxmove(c);
	Do_move_close(&the_event->arg1, &the_event->arg2, dtemp);
        break;

    case A_FAR:     /* pick random coordinates far */

	the_event->arg1 = SGN(c->player.x) * (D_CIRCLE + fabs(c->player.x)) *
		(2 * RND() + 2);

	the_event->arg2 = SGN(c->player.y) * (D_CIRCLE + fabs(c->player.y)) *
		(2 * RND() + 2);
        break;

    case A_TRANSPORT:	/* send out aways */

	Do_move_close(&x, &y, 2000 * RND());

	    /* use whichever x is larger of old and new */
	if (fabs(c->player.x) > fabs(x)) {
	    the_event->arg1 = c->player.x;
        }
        else {
	    the_event->arg1 = x;
        }

	    /* use whichever y is larger of old and new */
	if (fabs(c->player.y) > fabs(y)) {
	    the_event->arg2 = c->player.y;
        }
        else {
	    the_event->arg2 = y;
        }

	break;

    case A_OUST:	/* send out aways more */

	Do_move_close(&x, &y, 50000 * RND());

	    /* use whichever x is larger of old and new */
	if (fabs(c->player.x) > fabs(x)) {
	    the_event->arg1 = c->player.x;
        }
        else {
	    the_event->arg1 = x;
        }

	    /* use whichever y is larger of old and new */
	if (fabs(c->player.y) > fabs(y)) {
	    the_event->arg2 = c->player.y;
        }
        else {
	    the_event->arg2 = y;
        }

	break;

	/* send beyond */
    case A_BANISH:

        if (fabs(c->player.x) > fabs(c->player.y)) {

            the_event->arg1 = c->player.x;
	    the_event->arg2 = c->player.y;

	    if (fabs(c->player.x) < D_BEYOND) {
                the_event->arg1 = SGN(c->player.x) * D_BEYOND;
	    }

	}
        else {

            the_event->arg1 = c->player.x;
	    the_event->arg2 = c->player.y;

	    if (fabs(c->player.y) < D_BEYOND) {
                the_event->arg2 = SGN(c->player.y) * D_BEYOND;
	    }
        }
	break;
    }

	/* round the new location down */
    the_event->arg1 = floor(the_event->arg1);
    the_event->arg2 = floor(the_event->arg2);

	/* lock the realm */
    Do_lock_mutex(&c->realm->realm_lock);

	/* check to make sure there are no it_combat events not received */
    if (Do_check_encountered(c)) {
	Do_unlock_mutex(&c->realm->realm_lock);
	c->stuck = TRUE;
	return;
    }

	/* the move is successful - handle any events in the queue */
	/* only orphan events if the player is leaving the square -
	this closes the king safe send through deliberate itcombat */
    if (the_event->arg1 != c->player.x || the_event->arg2 != c->player.y) {
	Do_orphan_events(c);
    }

	/* if returning from beyond */
    if (the_event->arg3 != A_FORCED && c->player.beyond &&
	    fabs(the_event->arg1) < D_BEYOND && fabs(the_event->arg2)
	    < D_BEYOND) {

            /* cannot move back from point of no return */
            /* pick the largest coordinate to remain unchanged */
        if (fabs(c->player.x) > fabs(c->player.y)) {

	    the_event->arg1 = SGN(c->player.x) * D_BEYOND;
/*
	    if (fabs(c->player.x) < D_BEYOND) {
		the_event->arg1 = SGN(c->player.x) * D_BEYOND;
	    }
*/
	}
        else {

	    the_event->arg2 = SGN(c->player.y) * D_BEYOND;
/*
	    if (fabs(c->player.y) < D_BEYOND) {
		the_event->arg2 = SGN(c->player.y) * D_BEYOND;
	    }
*/
        }
    }

	/* see if the player is beyond */
    c->player.beyond = FALSE;
    oldLocation = c->player.location;

    if (fabs(the_event->arg1) >= D_BEYOND || fabs(the_event->arg2)
	    >= D_BEYOND) {

        c->player.beyond = TRUE;
    }

	/* if moving off the board's edge */
    if (fabs(the_event->arg1) >= D_EDGE || fabs(the_event->arg2) >= D_EDGE) {

	        /* stop a character at the edge */
	        /* send over if they move that way again */
        if ((fabs(the_event->arg1) > D_EDGE || fabs(the_event->arg2) > D_EDGE)
		&& c->player.location == PL_EDGE && 
                (the_event->arg3 == A_SPECIFIC || 
                 the_event->arg3 == A_TELEPORT)) {

	    Do_unlock_mutex(&c->realm->realm_lock);
	    event_ptr = (struct event_t *) Do_create_event();
	    event_ptr->type = DEATH_EVENT;
	    event_ptr->arg3 = K_FELL_OFF;
	    Do_file_event(c, event_ptr);
	    return;
	}

	c->player.location = PL_EDGE;

	if (fabs(the_event->arg1) > D_EDGE) {
	    the_event->arg1 = SGN(the_event->arg1) * D_EDGE;
	}

	if (fabs(the_event->arg2) > D_EDGE) {
	    the_event->arg2 = SGN(the_event->arg2) * D_EDGE;
	}
    }
    else {
	c->player.location = PL_REALM;
    }

	/* see if we're in the throne room */
    if (the_event->arg1 == 0 && the_event->arg2 == 0) {

	    /* if this player is not the king/steward */
	if (c->game != c->realm->king) {

		/* if this player believes himself to be, quit */
		/* we will have an event pending */
	    if (c->player.special_type == SC_STEWARD || c->player.special_type
		    == SC_KING) {

	        Do_unlock_mutex(&c->realm->realm_lock);
	        return;
	    }

		/* if this character is to be steward */
	    if (c->player.level >= 10 && c->player.level < 200) {

		    /* and if this player has a staff */
	        if (c->player.crowns) {

			    /* and there is no king */
		    if (!c->realm->king_flag) {

		            /* enter the throne room */
		        c->player.location = PL_THRONE;

 		            /* if there is a steward currently on the throne */
	                if (c->realm->king != NULL && c->realm->king->x == 0 &&
			    c->realm->king->y == 0) {

		        /* do nothing.  If victorious, he'll be crowned in
				the it_combat routines */
		        }
		        else {

		                /* become steward */
		            Do_send_self_event(c, STEWARD_EVENT);
	                }
		    }

			/* there is a king when we're trying to be steward */
		    else {

		        Do_send_line(c,
		    "There is no need for a steward while a king presides!\n");

		        pause = TRUE;

		        Do_move_close(&the_event->arg1, &the_event->arg2,
			        Do_maxmove(c));
		    }
		}

		    /* if the (10-200) player has no staff */
	        else {

		    Do_send_line(c,
			 "You require a staff to enter The Lord's Chamber.\n");

		    pause = TRUE;

		    Do_move_close(&the_event->arg1, &the_event->arg2,
			    Do_maxmove(c));
		}
	    }

		/* if this character is to be king */
	    else if (c->player.level >= MIN_KING && c->player.level < MAX_KING) {

		    /* and if this player has a crown */
	        if (c->player.crowns) {

		        /* enter the throne room */
		    c->player.location = PL_THRONE;

 		        /* if there is a king currently on the throne */
	            if (c->realm->king != NULL && c->realm->king->x == 0 &&
			c->realm->king->y == 0) {

				/* if that character is a king */
			    if (c->realm->king_flag) {

		                /* do nothing.  If victorious, he'll be crowned
				in the it_combat routines */
			    }
			    else {

				    /* make the curent steward virtual */
				c->realm->king->virtual = TRUE;

				    /* take the throne */
		         	Do_send_self_event(c, KING_EVENT);
			    }
		    }
		    else {

		            /* become king */
		        Do_send_self_event(c, KING_EVENT);
	            }
		}

		    /* if the (1000-2000) player has no crown */
	        else {

		    Do_send_line(c,
			 "You require a crown to enter The Lord's Chamber.\n");

		    pause = TRUE;

		    Do_move_close(&the_event->arg1, &the_event->arg2,
			    Do_maxmove(c));
		}
	    }

		/* the character is above the king level */
	    else if (c->player.level >= 2000) {

		Do_send_line(c, "The head page says, 'Get out of here, you greedy bastard.' and throws you out of The Lord's Chamber.\n");

		pause = TRUE;
		Do_move_close(&the_event->arg1, &the_event->arg2, Do_maxmove(c));
	    }

		/* the character is of the wrong level */
	    else {

		Do_send_line(c, "The head page stops you and says, 'Characters of your level may not enter The Lord's Chamber.'\n");

		pause = TRUE;
		Do_move_close(&the_event->arg1, &the_event->arg2, Do_maxmove(c));
	    }
	}
	    /* if the player already is king or steward */
	else {
	    c->player.location = PL_THRONE;
	}
    }

	/* check for possible combat */
    dtemp = FALSE;
    game_ptr = c->realm->games;
    while (game_ptr != NULL) {

	if (game_ptr->description != NULL && !game_ptr->virtual &&
		game_ptr->x == the_event->arg1 && game_ptr->y ==
		the_event->arg2 && game_ptr != c->game) {

	    dtemp = TRUE;
	    break;
	}
	game_ptr = game_ptr->next_game;
    }

	/* if we've encountered someone */
    if (dtemp) {

	    /* confirm the move if player moved under own power */
	if (event_ptr->arg3 == A_SPECIFIC || event_ptr->arg3 == A_TELEPORT) {

	        /* release the realm */
            Do_unlock_mutex(&c->realm->realm_lock);
	    pause = FALSE;

	    Do_send_line(c, "The square you're moving into is currently occupied by another player.  Do you still wish to move there?\n");

	    if (Do_yes_no(c, &answer) != S_NORM || answer == 1) {

		    /* abort the move if the player says "no" */
                Do_send_clear(c);
	        c->player.location = oldLocation;
                return;
	    }

            Do_send_clear(c);

	        /* relock the realm */
	    Do_lock_mutex(&c->realm->realm_lock);

	        /* see if someone else has attacked this player */
            if (Do_check_encountered(c)) {
	        Do_unlock_mutex(&c->realm->realm_lock);
	        c->stuck = TRUE;
	        return;
	    }

	        /* make sure a player is still there */
	    dtemp = 0;
            game_ptr = c->realm->games;
            while (game_ptr != NULL) {

	        if (game_ptr->description != NULL && !game_ptr->virtual &&
		        game_ptr->x == the_event->arg1 && game_ptr->y ==
		        the_event->arg2 && game_ptr != c->game) {

	            dtemp = 1;
	            break;
		}

	        game_ptr = game_ptr->next_game;
	    }
	}
    }

	/* set public and private variables to the new coordinates */
    if (c->wizard > 2) {
	c->player.location = PL_VALHALLA;
    }

    Do_location(c, the_event->arg1, the_event->arg2, FALSE);

        /* move the grail if player teleported close to it */
/*
    if (the_event->arg3 == A_TELEPORT) {
	object_ptr = c->realm->objects;

	while (object_ptr != NULL) {

	    if (object_ptr->type == HOLY_GRAIL) {
		break;
	    }

	    object_ptr = object_ptr->next_object;
	}

	if (object_ptr == NULL) {

	    Do_unlock_mutex(&c->realm->realm_lock);

	    sprintf(error_msg,
		    "[%s] No grail found in realm objects in Do_move.\n",
		    c->connection_id);

	    Do_log_error(error_msg);

	    return;
	}

	Do_distance(c->player.x, object_ptr->x, c->player.y, object_ptr->y,
		&distance);


        if (distance <= c->player.level / 500.0) {

	    sprintf(error_msg,
		    "Teleporting grail.\n");

	    Do_log_error(error_msg);

            free((void *)object_ptr);
	    Do_hide_grail(c->realm, c->player.level);
        }

    }
*/
        

	/* look for realm objects if destination has no players */
    if (dtemp == 0) {

            /* if a player is cloaked, he/she can't find realm objects */
        if (!c->player.cloaked) {

	        /* check for realm objects in the new location */
            object_ptr_ptr = &c->realm->objects;

            while(*object_ptr_ptr != NULL) {

	        if ((*object_ptr_ptr)->x == c->player.x && (*object_ptr_ptr)->y
			== c->player.y) {

	            object_ptr = *object_ptr_ptr;
	            *object_ptr_ptr = object_ptr->next_object;

	            event_ptr = (struct event_t *) Do_create_event();
	    
	            switch (object_ptr->type) {

	            case CORPSE:
		        event_ptr->type = CORPSE_EVENT;

			    /* the corpse can be cursed */
		        event_ptr->arg1 = TRUE;
		        event_ptr->arg4 = object_ptr->arg1;
		        break;

	            case HOLY_GRAIL:
		        event_ptr->type = GRAIL_EVENT;
		        Do_hide_grail(c->realm, c->player.level);
		        break;

	            case ENERGY_VOID:
		        event_ptr->type = ENERGY_VOID_EVENT;
		        break;

		    case TREASURE_TROVE:
		        event_ptr->type = TROVE_EVENT;
			Do_hide_trove(c->realm);
			break;

	            default:

	                sprintf(error_msg,
		      "[%s] encountered realm object of type %d in Do_move.\n",
	    	      c->connection_id, object_ptr->type);

		        Do_log_error(error_msg);	
	            }

	            Do_file_event(c, event_ptr);

	            free((void *)object_ptr);
	        }
	        else {
	            object_ptr_ptr = &((*object_ptr_ptr)->next_object);
	        }
	    }
        }

	Do_unlock_mutex(&c->realm->realm_lock);

        if (pause) {
	    Do_more(c);
            Do_send_clear(c);
        }
    }

	/* else we have found a player to fight */
    else {
 
	    /* pass this function with realm locked! */
	Do_setup_it_combat(c, game_ptr);
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_check_weight(c)
/
/ FUNCTION: truncate trailing blanks off a string
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/16/99
/
/ ARGUMENTS:
/       char *string - pointer to null terminated string
/
/ RETURN VALUE: none
/
/ MODULES CALLED: strlen()
/
/ DESCRIPTION:
/       Put nul characters in place of spaces at the end of the string.
/
*************************************************************************/

Do_check_weight(struct client_t *c)
{
    Do_speed(c, c->player.max_quickness, c->player.quicksilver,
	    c->battle.speedSpell, FALSE);
}


/************************************************************************
/
/ FUNCTION NAME: Do_medic(struct client_t *c)
/
/ FUNCTION: do random stuff
/
/ AUTHOR: E. A. Estes, 3/3/86
/         Brian Kelly, 6/20/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: collecttaxes(), floor(), wmove(), drandom(), infloat(),
/       waddstr(), mvprintw(), getanswer()
/
/ GLOBAL INPUTS: Player, *stdscr, *Statptr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle gurus, medics, etc.
/
*************************************************************************/

Do_medic(struct client_t *c)
{
    double gold;
    char string_buffer[SZ_LINE];
    float ftemp;

    if (c->player.blind) {

        Do_send_line(c, "You've found a medic!\n");

	if (Do_double_dialog(c, &gold,
		"How much will you offer to heal your blindness?\n")) {

            Do_send_clear(c);
	    return;
	}

                /* negative gold, or more than available */
        if (gold < 0.0 || gold > c->player.gold) {
            Do_send_line(c, "He was not amused, and gave you a disease.\n");
            Do_adjusted_poison(c, 1.0);
	    Do_strength(c, c->player.max_strength, c->player.sword, 0, FALSE);
        }
        else if (gold < c->player.level * 10) {
            Do_send_line(c, "Sorry, he thinks you're too powerful an adventurer to be offering him so little.\n");
	} 
        else if (RND() > (gold + 1.0) / (c->player.gold + 4.0)) {

                /* medic wants nearly all of available gold */
            Do_send_line(c, "Sorry, he wasn't interested.\n");
	}
        else {
            Do_send_line(c, "He accepted.\n");
	    c->player.blind = FALSE;
	    Do_gold(c, -gold, FALSE);
        }
        Do_more(c);
        Do_send_clear(c);
    }
    else if (c->player.poison > 0.0) {

        Do_send_line(c, "You've found a medic!\n");

	if (Do_double_dialog(c, &gold,
		"How much will you offer to be cured?\n")) {

            Do_send_clear(c);
	    return;
	}

                /* negative gold, or more than available */
        if (gold < 0.0 || gold > c->player.gold) {
            Do_send_line(c, "He was not amused, and made you worse.\n");
            Do_adjusted_poison(c, 1.0);
	    Do_strength(c, c->player.max_strength, c->player.sword, 0, FALSE);
        }
                /* medic needs at least a certain amount of gold to cure */
        else if (gold < c->player.level) {
            Do_send_line(c, "Sorry, he thinks you're too powerful an adventurer to be offering him so little.\n");
	} 
        else if (gold < pow(c->player.poison, 2.5)) {
            Do_send_line(c, "Sorry, you didn't offer enough to treat your virulent poison.\n");
	} 
        else if (RND() > (1 / c->player.poison) * sqrt(c->player.circle)) {
            Do_send_line(c, "Sorry, the medic says the poison is beyond his ability to cure.  Find a better one.\n");
	} 
        else if (RND() > (1 + MAX(0, (1 - .1 * c->player.poison))) 
                          * (gold + 1.0) / 
                          (c->player.gold + 2.0)) {

                /* medic wants 1/2 of available gold for 1 poison */
                /* as poison goes to infinity, the medic will only cure
                   25% of the time when player gives all gold */

            Do_send_line(c, "Sorry, he wasn't interested.\n");
	}
        else {
            Do_send_line(c, "He accepted.\n");
	    Do_poison(c, (double) -c->player.poison);
	    Do_gold(c, -gold, FALSE);
        }
        Do_more(c);
        Do_send_clear(c);
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_tax(struct client_t *c)
/
/ FUNCTION: Tax time!
/
/ AUTHOR: Eugene Hung, 6/16/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: none
/
/ GLOBAL INPUTS: none
/
/ GLOBAL OUTPUTS: none
/
/ DESCRIPTION:
/       Handle tax collecting
/
*************************************************************************/

Do_tax(struct client_t *c)
{
    double gold, gems;
    char error_msg[SZ_ERROR_MESSAGE];

    /* no tax collectors in the PoNR */
    if (c->player.beyond) {
        return;
    }

    Do_send_line(c, "You've encountered a tax collector!\n");

    if ((c->player.special_type == SC_VALAR) || (c->wizard >= 3)) {
        Do_send_line(c, "He takes one look at you, screams, and tries to run away!\n");
	Do_more(c);
        Do_send_line(c, "You reach out, and squish him with your little finger!\n");
	Do_more(c);
	Do_send_clear(c);
        return;
    }
    else if (c->player.special_type >= SC_COUNCIL) {
        Do_send_line(c, "He takes one look at you and runs away screaming in terror!\n");
	Do_more(c);
	Do_send_clear(c);
        return;
    }
    else if (c->player.special_type >= SC_STEWARD) {
        Do_send_line(c, "He bows before you and walks off.\n");
	Do_more(c);
	Do_send_clear(c);
        return;
    }

	/* at 7% we'll only take if there's at least 15 of something */
    if (c->player.gems >= 15.0) {

	Do_send_line(c, "His eyes glow at the sight of your gems.  He swipes a portion of your wealth and disappears!\n");

    }
	/* at 10% we'll only take if there's at least 10 of something */
    else if (c->player.gold >= 10.0) {

	Do_send_line(c, "He cackles gleefully and takes some of your gold before disappearing!\n");

    }
    else if (c->player.gems == 0.0) {

	Do_send_line(c, "'Ah,' he says, 'A candidate for our welfare program.'  He flips you a gold piece, gives a satisfied smile, and disappears!\n");

	Do_more(c);
	Do_send_clear(c);
	Do_gold(c, 1.0, FALSE);
        return;
    }
    else {

	Do_send_line(c, "He lectures, 'How can our kingdom survive when you are making such a pittance?  Work harder!' then disappears!\n");

	Do_more(c);
	Do_send_clear(c);
        return;
    }

    Do_more(c);
    Do_send_clear(c);

    gold = floor(c->player.gold * 0.1);
    Do_gold(c, -gold, FALSE);

    gems = floor(c->player.gems * 0.07);
    Do_gems(c, -gems, FALSE);

    gold += gems * N_GEMVALUE;

	/* see if there is a steward */
    /*
    Do_lock_mutex(&c->realm->realm_lock);

    if (c->realm->king != NULL && c->realm->king_flag) {
    */

	Do_unlock_mutex(&c->realm->realm_lock);
        Do_lock_mutex(&c->realm->kings_gold_lock);

	    /* steward coffers are capped at 40K */
	if (RND() > c->realm->steward_gold / 40000.0) {

		/* stewards get a maximum of 1K donation */
	    if (gold > 1000.0) {
		c->realm->steward_gold += 1000.0;
		gold -= 1000.0;
	    }
	    else {
	        c->realm->steward_gold += gold;
	        Do_unlock_mutex(&c->realm->kings_gold_lock);
	        return;
	    }
	}

	Do_unlock_mutex(&c->realm->kings_gold_lock);
    /*
    }
    else {
	Do_unlock_mutex(&c->realm->realm_lock);
    }
    */

	/* add to the king's coffers */
    Do_lock_mutex(&c->realm->kings_gold_lock);

	/* kings coffers are capped at 2M */
    if (RND() > c->realm->kings_gold / 2000000.0) {

	    /* maximum contribution of 20K */
	if (gold > 20000.0) {
	    c->realm->kings_gold += 20000.0;
	    gold -= 20000.0;
	}
	else {
	    c->realm->kings_gold += gold;
	    gold = 0.0;
	}
    }

    Do_unlock_mutex(&c->realm->kings_gold_lock);

if (gold > 0.0) {
sprintf(error_msg, "[%s] Do_tax did not allocate %lf in wealth.\n", c->connection_id, gold);
Do_log(DEBUG_LOG, error_msg);
}

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_guru(struct client_t *c)
/
/ FUNCTION: do random stuff
/
/ AUTHOR: E. A. Estes, 3/3/86
/         Brian Kelly, 6/20/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: collecttaxes(), floor(), wmove(), drandom(), infloat(),
/       waddstr(), mvprintw(), getanswer()
/
/ GLOBAL INPUTS: Player, *stdscr, *Statptr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle gurus, medics, etc.
/
*************************************************************************/

Do_guru(struct client_t *c)
{
    float ftemp;

    Do_send_line(c, "You've met a Guru. . .\n");

    if (RND() * c->player.sin > 1.0) {
        Do_send_line(c, "You disgusted him with your sins!\n");
        Do_more(c);
        Do_send_clear(c);

    }
    else if (c->player.poison > 0.0) {
        Do_send_line(c, "He looked kindly upon you, and cured you.\n");
        Do_more(c);
        Do_send_clear(c);
	Do_poison(c, (double) -c->player.poison);
    }
    else if ((RND() / 10 > c->player.sin) && (c->player.circle > 1)) {
        Do_send_line(c, "He slips something into your charm pouch as a reward for your saintly behavior!\n");
	Do_mana(c, 40.0 + 15 * c->player.circle, FALSE);
	Do_more(c);
        Do_send_clear(c);

        c->player.charms += 1 + (c->player.circle / 20);
           } else {
        Do_send_line(c, "He rewarded you for your virtue.\n");
	Do_more(c);
        Do_send_clear(c);
	Do_mana(c, 40.0 + 10 * c->player.circle, FALSE);

	Do_energy(c, c->player.energy + 2 + c->player.circle / 5,
		c->player.max_energy, c->player.shield + 2 + c->player.circle
		/ 5, 0, FALSE);
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_village(struct client_t *c)
/
/ FUNCTION: do random stuff
/
/ AUTHOR: E. A. Estes, 3/3/86
/         Brian Kelly, 6/20/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: collecttaxes(), floor(), wmove(), drandom(), infloat(),
/       waddstr(), mvprintw(), getanswer()
/
/ GLOBAL INPUTS: Player, *stdscr, *Statptr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle gurus, medics, etc.
/
*************************************************************************/

Do_village(struct client_t *c)
{
    struct event_t *event_ptr;
    long answer;
    float ftemp;

    if (c->player.beyond || (c->player.level + 1 < c->player.circle)) {
       Do_send_line(c, "You have found a village.  But the inhabitants are too strong for you!\n");
       Do_more(c);
       Do_send_clear(c);
       return;
    }

       
    Do_send_line(c, "You have found a village.  Do you wish to pillage it?\n");

    if (Do_yes_no(c, &answer) != S_NORM || answer == 1) {

	    /* return if the player says no */
        Do_send_clear(c);
        return;
    }

    Do_send_clear(c);

    Do_sin(c, 0.5);
    Do_experience(c, (.75 + RND() / 2) * 1000.0 * c->player.circle, FALSE);
    Do_gold(c, (.75 + RND() / 2) * 50.0 * c->player.circle, FALSE);
}


/************************************************************************
/
/ FUNCTION NAME: Do_plague(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: do random stuff
/
/ AUTHOR: E. A. Estes, 3/3/86
/         Brian Kelly, 6/20/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: collecttaxes(), floor(), wmove(), drandom(), infloat(),
/       waddstr(), mvprintw(), getanswer()
/
/ GLOBAL INPUTS: Player, *stdscr, *Statptr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle plague.  event->arg3 contains 1000x the amount of poison to
/       increase the player by.
/
*************************************************************************/

Do_plague(struct client_t *c, struct event_t *the_event)
{
    float ftemp;

    if (the_event->arg3 == 0) {
        if (c->player.poison < sqrt(c->player.circle)) {
            Do_send_line(c, "You've been hit with a plague!\n");

            if (c->player.type == C_ELF) {
                Do_send_line(c, "But as an elf, you are unaffected!\n");
            } else if (c->player.charms > 0) { 
                Do_send_line(c, "But your charm saved you!\n");
                --c->player.charms;
            } else {
                Do_adjusted_poison(c, 1.0);
            } 

            if (c->player.virgin && RND() < 0.2) {
                Do_send_line(c, "Your virgin catches it and perishes!\n");
                Do_virgin(c, FALSE, FALSE);
            } 

            Do_more(c);
            Do_send_clear(c);
        }
    } else {
        Do_send_line(c, "You absorb some of the poison you cured!\n");
        Do_poison(c, (double) the_event->arg3 / 1000);
    }

}


/************************************************************************
/
/ FUNCTION NAME: Do_volcano(struct client_t *c)
/
/ FUNCTION: generate a place for the One Ring to be thrown into
/
/ AUTHOR: Eugene Hung, 8/2/01
/
/ ARGUMENTS: c
/
/ RETURN VALUE: none
/
/ MODULES CALLED: 
/
/ GLOBAL INPUTS: 
/
/ GLOBAL OUTPUTS: 
/
/ DESCRIPTION:
/       Gives crowns
/
*************************************************************************/

Do_volcano(struct client_t *c)
{
    long answer;
    float ftemp;
    char string_buffer[SZ_LINE];


    Do_send_line(c, "You've come upon a pool of lava!\n");

    if (c->player.ring_type != R_NONE) {
        Do_send_line(c, "Do you wish to destroy your ring?\n");
    } else {
        Do_send_line(c, "It bubbles and hisses, and you wisely avoid it.\n");
        Do_more(c);
        Do_send_clear(c);
        return;
    }

    if (Do_yes_no(c, &answer) != S_NORM || answer == 1) {

	    /* return if the player says no */
        Do_send_line(c, "You hang on to Your Precious Ring.  You hear a roar of evil laughter in the distance.\n");
        Do_sin(c, 2.5);
        Do_more(c);
        Do_send_clear(c);
        return;
    } 

    Do_send_line(c, "Sweating, you attempt to throw the Ring in . . .\n");

    Do_send_buffer(c);
    sleep(3);

    ftemp = RND();


        /* certain races have more willpower */
    if (((c->player.type == C_HALFLING) && (c->player.sin <= ftemp + .25)) ||
        ((c->player.type == C_DWARF)    && (c->player.sin * 2 <= ftemp + .25)) ||
        ((c->player.type == C_ELF)      && (c->player.sin * 3 <= ftemp + .25)) ||
        (c->player.sin * 4 <= ftemp + .25)
       )
         {

        Do_send_line(c, "You throw the ring into the pool...\n");
        Do_more(c);
        Do_send_clear(c);
        
        
        if (c->player.ring_type == R_DLREG) {
		Do_send_line(c, "and watch as the symbol of ultimate evil melts in its birth place!\n");
        Do_more(c);
        Do_send_line(c, "Having rid yourself of the One Ring, you feel a great burden lift from your heart!\n");
        Do_more(c);
        Do_send_clear(c);
        
        
            Do_sin(c, -c->player.sin);
            Do_experience(c, 50000.0 * c->player.circle, FALSE);
            sprintf(string_buffer, "%s has thrown the One Ring into the Fire!  Praise %s with great praise!\n", 
                    c->modifiedName, (c->player.gender ? "her" : "him"));
            Do_broadcast(c, string_buffer);
            Do_ring(c, R_NONE, FALSE);
          
            if ((c->player.level > MIN_KING) && (c->player.level < MAX_KING)) {
                Do_send_line(c, "For your great deed, you have been given a golden crown!\n");
                Do_crowns(c, 1, FALSE);
                Do_more(c);
            }
        } else {
            Do_send_line(c, "Unfortunately, nothing else seems to happen.\n");
            Do_ring(c, R_NONE, FALSE);
            Do_more(c);
        }

    } else {
        Do_send_line(c, "You lack the willpower to do so!  You hear a sinister cackle in the distance.\n");
        Do_sin(c, 1.0);

        if (c->player.ring_type == R_BAD) {
            c->player.ring_duration = 0;
            Do_ring(c, R_SPOILED, FALSE);
        }
        Do_more(c);
    }

    Do_send_clear(c);

    return;
}

/************************************************************************
/
/ FUNCTION NAME: Do_shutdown_check(struct client_t *c)
/
/ FUNCTION: do random stuff
/
/ AUTHOR: E. A. Estes, 3/3/86
/         Brian Kelly, 6/20/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: collecttaxes(), floor(), wmove(), drandom(), infloat(),
/       waddstr(), mvprintw(), getanswer()
/
/ GLOBAL INPUTS: Player, *stdscr, *Statptr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle gurus, medics, etc.
/
*************************************************************************/

Do_shutdown_check(struct client_t *c)
{
    if (server_hook != RUN_SERVER && server_hook != LEISURE_SHUTDOWN) {
        Do_send_int(c, SHUTDOWN_PACKET);
        Do_send_buffer(c);
	c->socket_up = FALSE;
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_information(struct client_t *c)
/
/ FUNCTION: do random stuff
/
/ AUTHOR: Brian Kelly, 11/8/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: collecttaxes(), floor(), wmove(), drandom(), infloat(),
/       waddstr(), mvprintw(), getanswer()
/
/ GLOBAL INPUTS: Player, *stdscr, *Statptr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle gurus, medics, etc.
/
*************************************************************************/

Do_information(struct client_t *c)
{
    struct event_t *event_ptr;
    struct button_t buttons;
    long answer;
    char error_msg[SZ_ERROR_MESSAGE];
    char string_buffer[SZ_LINE];

    Do_clear_buttons(&buttons, 0);

    strcpy(buttons.button[0], "Stats\n");
    strcpy(buttons.button[1], "Examine\n");
    strcpy(buttons.button[2], "Players\n");
    strcpy(buttons.button[3], "Scoreboard\n");
    strcpy(buttons.button[4], "Channel\n");

    if (c->hearBroadcasts) {
        strcpy(buttons.button[5], "No Announce\n");
    }
    else {
        strcpy(buttons.button[5], "Announce\n");
    }


    if (c->game->chatFilter) {
        strcpy(buttons.button[6], "Filter Off\n");
    }
    else {
        strcpy(buttons.button[6], "Filter On\n");
    }
	
    strcpy(buttons.button[7], "Cancel\n");

    if (Do_buttons(c, &answer, &buttons) != S_NORM || answer == 7) {
        Do_send_clear(c);
	return;
    }

    Do_send_clear(c);

    event_ptr = (struct event_t *) Do_create_event();
    event_ptr->to = c->game;
    event_ptr->from = c->game;

    switch (answer) {

	/* see this player's stats */
    case 0:
	event_ptr->type = EXAMINE_EVENT;
	event_ptr->arg3 = FALSE;
	break;

    case 1:
	event_ptr->type = EXAMINE_EVENT;
	event_ptr->arg3 = TRUE;
	break;

    case 2:
	event_ptr->type = LIST_PLAYER_EVENT;
	break;

    case 3:
	Do_scoreboard(c, 0, TRUE);
	break;

    case 4:

	sprintf(string_buffer, "You are currently on channel %d.\n",
		c->channel);

	Do_send_line(c, string_buffer);

	Do_send_line(c, "Channel 8 is reserved for players with a palantir.\n");
	Do_send_line(c, "Which channel do you want to change to?\n");

        Do_clear_buttons(&buttons, 0);

        strcpy(buttons.button[0], "1\n");
        strcpy(buttons.button[1], "2\n");
        strcpy(buttons.button[2], "3\n");
        strcpy(buttons.button[3], "4\n");
        strcpy(buttons.button[4], "5\n");
        strcpy(buttons.button[5], "6\n");
        strcpy(buttons.button[6], "7\n");

        if (c->player.palantir || c->wizard > 3) {
            strcpy(buttons.button[7], "8\n");
        } 

        if (Do_buttons(c, &answer, &buttons) != S_NORM) {
	    Do_send_clear(c);
	    free((void *)event_ptr);
	    return;
        }

	Do_send_clear(c);

	    /* update the player info, if necessary */
	if (c->channel != (int)answer +1) {
	
                /* if we were palantiring, turn off snooping  */
            if (c->channel == 8) {
                c->game->hearAllChannels = HEAR_SELF;
            } 


	    c->channel = (int)answer + 1;

                /* if we used our palantir, turn on snooping */
            if (c->channel == 8) {
                c->game->hearAllChannels = HEAR_ONE;
            } 

            Do_lock_mutex(&c->realm->realm_lock);
            Do_player_description(c);
            Do_unlock_mutex(&c->realm->realm_lock);

	    Do_send_specification(c, CHANGE_PLAYER_EVENT);
        }

	break;

/*
    case 5:
	free((void *)event_ptr);

	sprintf(string_buffer, "/var/phantasia/backup2/%s", c->player.lcname);

	errno = 0;
        if ((character_file=fopen(string_buffer, "w")) == NULL) {

            sprintf(error_msg,
		    "[%s] fopen of %s failed in Do_information: %s\n",
		    c->connection_id, string_buffer, strerror(errno));

            Do_log_error(error_msg);
	    return;
        }

        errno = 0;
        if (fwrite((void *)&c->player, SZ_PLAYER, 1, character_file) != 1) {

            sprintf(error_msg,
		    "[%s] fwrite of %s failed in Do_information: %s\n",
                    c->connection_id, string_buffer, strerror(errno));

            Do_log_error(error_msg);
            fclose(character_file);
            return;
        }

        fclose(character_file);

        sprintf(string_buffer, "%s has been backed up for a server crash.\n",
		c->modifiedName);

	Do_send_line(c, string_buffer);
	Do_send_line(c,
	    "Backing up this character again will destroy the old backup.\n");

	Do_more(c);
	return;
*/

    case 5:
	free((void *)event_ptr);

	if (c->hearBroadcasts) {
	    Do_send_line(c, "Server announcements are now blocked.\n");
	    c->hearBroadcasts = FALSE;
	}
	else {
	    Do_send_line(c, "You will now hear server announcements.\n");
	    c->hearBroadcasts = TRUE;
	}

	Do_more(c);
	Do_send_clear(c);
	return;

    case 6:
	c->game->chatFilter = !c->game->chatFilter;
        return;

    default:

	sprintf(error_msg,
		"[%s] Returned non-option %ld in Do_information.\n",
		c->connection_id, answer);
		
	Do_log_error(error_msg);
	free((void *)event_ptr);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

    if (event_ptr->type != NULL_EVENT) {
        Do_handle_event(c, event_ptr);
    }
    else {
	free((void *)event_ptr);
    }
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_scoreboard(struct client_t *c)
/
/ FUNCTION: do random stuff
/
/ AUTHOR: Brian Kelly, 12/30/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: collecttaxes(), floor(), wmove(), drandom(), infloat(),
/       waddstr(), mvprintw(), getanswer()
/
/ GLOBAL INPUTS: Player, *stdscr, *Statptr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle gurus, medics, etc.
/
*************************************************************************/

Do_scoreboard(struct client_t *c, int start, int new)
{
    char string_buffer[SZ_LINE];
    char error_msg[SZ_ERROR_MESSAGE];
    struct scoreboard_t scoreboard;
    FILE *scoreboard_file;
    long theEOF; 
    int i, records, EOF_flag;

    Do_send_int(c, SCOREBOARD_DIALOG_PACKET);

    Do_lock_mutex(&c->realm->scoreboard_lock);

        /* open the scoreboard file */
    errno = 0;
    
    if (new) {
      if ((scoreboard_file=fopen(SCOREBOARD_FILE, "r")) == NULL) {

        sprintf(error_msg, "[%s] fopen of %s failed in Do_scoreboard: %s\n",
		c->connection_id, SCOREBOARD_FILE, strerror(errno));

        Do_log_error(error_msg);

        Do_send_line(c,
		"1\nCurrently, there is nobody on the scoreboard.\n"); 
        Do_unlock_mutex(&c->realm->scoreboard_lock);
        return;
      } else {

        /* drop to next part */
      }

    } else {
      if ((scoreboard_file=fopen(OLD_SCOREBOARD_FILE, "r")) == NULL) {

        sprintf(error_msg, "[%s] fopen of %s failed in Do_scoreboard: %s\n",
		c->connection_id, OLD_SCOREBOARD_FILE, strerror(errno));

        Do_log_error(error_msg);

        Do_send_line(c,
		"1\nCurrently, there is nobody on the scoreboard.\n"); 

        Do_unlock_mutex(&c->realm->scoreboard_lock);
        return;
      } else {

        /* drop to next part */
      }
    }


	    /* find the end of the scoreboard file */
	fseek(scoreboard_file, 0, SEEK_END);
	theEOF = ftell(scoreboard_file);

	    /* calculate how many records are in the file */
	records = theEOF / SZ_SCOREBOARD;

	    /* the earliest we can start is the beginning */
	if (start < 0) {
	    start = 0;
	}

	   /* we'll only show 50 records maximum */
	records = 50;

	    /* tell the client number of records and their start */
	Do_send_int(c, start);
	Do_send_int(c, records);

	    /* move to the starting record */
        EOF_flag = FALSE;
        fseek(scoreboard_file, SZ_SCOREBOARD * start, SEEK_SET);

	    /* read the next 50 or so records */
        for (i = start + 1; i < records + start + 1; i++) {

            if (!EOF_flag && fread((void *)&scoreboard, SZ_SCOREBOARD, 1,
		    scoreboard_file) == 1) {

/*
	        sprintf(string_buffer,
			"%d> %s, the level %0.lf %s, was %s at ", i,
		        scoreboard.name, scoreboard.level, scoreboard.class,
		        scoreboard.how_died);
*/
	        if (scoreboard.level == 9999) {

	            sprintf(string_buffer,
	         "%d> %s the %s ascended to the position of Valar on ",
		 i, scoreboard.name, scoreboard.class);
		    
	            Do_send_string(c, string_buffer);

		    ctime_r(&scoreboard.time, string_buffer);
		    Do_truncstring(string_buffer);
	            strcat(string_buffer, ".\n");
	            Do_send_string(c, string_buffer);
		}
		else {
	            sprintf(string_buffer,
			"%d> %s, the level %0.lf %s, %s on ", i,
		        scoreboard.name, scoreboard.level, scoreboard.class,
		        scoreboard.how_died);
/*
	            sprintf(string_buffer,
			    "%d> %s, the level %0.lf %s,", i,
		            scoreboard.name, scoreboard.level,
			    scoreboard.class);
*/

	            Do_send_string(c, string_buffer);

		    ctime_r(&scoreboard.time, string_buffer);
		    Do_truncstring(string_buffer);
	            strcat(string_buffer, ".\n");
	            Do_send_string(c, string_buffer);
		}
	    }
	    else {
		EOF_flag = TRUE;
	        sprintf(string_buffer, "%d> No entry.\n", i);
                Do_send_string(c, string_buffer);
	    }
        } 

            /* close the file handles */
        fclose(scoreboard_file);
	Do_send_buffer(c);

    Do_unlock_mutex(&c->realm->scoreboard_lock);
}


/************************************************************************
/
/ FUNCTION NAME: int Do_profanity_check(char *theString)
/
/ FUNCTION: do random stuff
/
/ AUTHOR: Brian Kelly, 1/2/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: collecttaxes(), floor(), wmove(), drandom(), infloat(),
/       waddstr(), mvprintw(), getanswer()
/
/ GLOBAL INPUTS: Player, *stdscr, *Statptr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle gurus, medics, etc.
/
*************************************************************************/

int Do_profanity_check(char *theString)
{
    char *swears[] = { "shit", "fuck", "fuc", "fuk", "fock", "piss", "cunt", "cock", "ass", "dick", "penis", "pussy", "clit", "bitch", "butt", "twat", "vagina", "dork", "dildo", "masturbat", "fag", "homo", "lesbian", "screw", "anal", "poo", "nigger", "nigga", "chink", "jap", "wop", "kike", "bitch", "whore", "crap", "hell", "damn", "rape", "suck", "abortion", "mute", "sex", "kkk", "balls", "bush", "tits", "dammit", "snatch", "sauron", "morgoth", "osama", "alatar", "stfu", "wtf", "" };

    char currentChar;
    int stringPtr, currentSwear, swearPtr, wordBeginning, swearBeginning, spaceCount;

    if (theString[0] == '\0') {
	return FALSE;
    }

    currentSwear = 0;

	/* loop through all the swear words */
    while (swears[currentSwear][0] != '\0') {

	stringPtr = swearPtr = wordBeginning = swearBeginning = spaceCount = 0;

	    /* look for the next letter in the swear */
	while (theString[stringPtr] != '\0') {

	    currentChar = theString[stringPtr];

		/* replace numbers that could be letters */
	    if (currentChar == '1' && swears[currentSwear][swearPtr] ==
		    'i') {

		currentChar = 'i';
	    }

	    else if (currentChar == '1' && swears[currentSwear][swearPtr] ==
		    'l') {

		currentChar = 'l';
	    }
	    else if (currentChar == 'z' && swears[currentSwear][swearPtr] ==
		    's') {

		currentChar = 's';
	    }
	    else if (currentChar == '5' && swears[currentSwear][swearPtr] ==
		    's') {

		currentChar = 's';
	    }
	    else if (currentChar == '0' && swears[currentSwear][swearPtr] ==
		    'o') {

		currentChar = 'o';
	    }
	    else if (currentChar == '$' && swears[currentSwear][swearPtr] ==
		    's') {

		currentChar = 's';
	    }
	    else if (currentChar == '@' && swears[currentSwear][swearPtr] ==
		    'a') {

		currentChar = 'a';
	    }

		/* Is this character a space? */
            if (currentChar == '_') {

		if (swearPtr == 0) {
		    wordBeginning = stringPtr + 1;
		}
		else {
		    ++spaceCount;
		}
	    }

		/* If the character is not a letter, skip it */
	    else if (isalpha(currentChar)) {

	        if (currentChar == swears[currentSwear][swearPtr]) {
		    ++swearPtr;

		        /* See if we've found a complete swear */
		    if (swears[currentSwear][swearPtr] == '\0') {

			    /* if the swear did not start at the beginning
			    of a word and contains one space, let it pass.
			    "is_hit" would be caught */
			if (wordBeginning != swearBeginning || spaceCount
				!= 1) {

			    return TRUE;
			}

		        swearPtr = 0;
		        spaceCount = 0;
		        stringPtr = swearBeginning;
			++swearBeginning;
		    }
		}

		    /* Start over if anything but previous swear letter */
		    /* This is to catch "ffuucckk" */
	        else if (swearPtr == 0 || currentChar !=
		        swears[currentSwear][swearPtr - 1]) {

		    swearPtr = 0;
		    spaceCount = 0;
		    stringPtr = swearBeginning;
		    ++swearBeginning;
	        }
	    }

		/* move to the next letter */
	    ++stringPtr;
	}

        ++currentSwear;
    }

    return FALSE;
}
		
		
/************************************************************************
/
/ FUNCTION NAME: Do_replace_profanity(struct client_t *c, char *theString)
/
/ FUNCTION: do random stuff
/
/ AUTHOR: Brian Kelly, 10/11/00
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: collecttaxes(), floor(), wmove(), drandom(), infloat(),
/       waddstr(), mvprintw(), getanswer()
/
/ GLOBAL INPUTS: Player, *stdscr, *Statptr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle gurus, medics, etc.
/
*************************************************************************/

int Do_replace_profanity(char *theString)
{
    char *swears[] = { "shit", "fock", "fuk", "fuc", "cunt", "nigger", "crap", "damn", "dammit", "@ss", "a$$", "" };
    char replacements[] = "smurf ";
    char currentChar;
    int stringPos, currentSwear, swearPos, wordStart, swearStart, smurfPos;
    int spaceCount, bleepPos;
    bool profane, spaceExceptionPossible;

    if (theString[0] == '\0') {
	return FALSE;
    }

    profane = FALSE;
    currentSwear = 0;

	/* loop through all the swear words */
    while (swears[currentSwear][0] != '\0') {

	stringPos = swearPos = wordStart = swearStart = 0;
        spaceExceptionPossible = FALSE;

	    /* look until the we reach the end of the string */
	while (theString[stringPos] != '\0') {

	    currentChar = tolower(theString[stringPos]);

		/* Convert similar symbols */
	    if (currentChar == '!' && swears[currentSwear][swearPos] == 'i') {
		currentChar = 'i';
	    }

	    else if (currentChar == '!' && swears[currentSwear][swearPos] ==
		    'l') {

		currentChar = 'l';
	    }

	    else if (currentChar == '1' && swears[currentSwear][swearPos] ==
		    'i') {

		currentChar = 'i';
	    }

	    else if (currentChar == '1' && swears[currentSwear][swearPos] ==
		    'l') {

		currentChar = 'l';
	    }

	    else if (currentChar == '#' && swears[currentSwear][swearPos] ==
		    'h') {

		currentChar = 'h';
	    }

	    else if (currentChar == '$' && swears[currentSwear][swearPos] ==
		    's') {

		currentChar = 's';
	    }

	    else if (currentChar == '(' && swears[currentSwear][swearPos] ==
		    'c') {

		currentChar = 'c';
	    }


		/* Is this character a space? */
            if (currentChar == ' ') {

		    /* if this space is within a possible swear */
		if (swearPos > 0) {

			/* count the space for possible exception */
		    ++spaceCount;
		}

		    /* if we've found no swear letters */
		else {

			/* we won't find anything before this */
		    wordStart = stringPos + 1;
		    swearStart = wordStart;

			/* since the swear starts a word, no exception */
		    spaceExceptionPossible = FALSE;
		}

		    /* move to the next letter of the string */
		++stringPos;
		continue;
	    }

		/* If the character is not a letter, skip it */
	    else if (! isalpha(currentChar)) {
		++stringPos;
		continue;
	    }

		/* Is this character the next of this swear? */
	    else if (currentChar == swears[currentSwear][swearPos]) {

		    /* start looking for the next letter */
		++swearPos;

		    /* have we found all the swear letters? */
		if (swears[currentSwear][swearPos] == '\0') {

			/* we don't want to bleep "is hit" or "if u c", 
                        so pass over if swear didn't start a word and there 
                        was 1 or 2 spaces */

		    if (!spaceExceptionPossible || spaceCount != 1) {

			    /* bleep out the whole word */
			bleepPos = wordStart;
		        profane = TRUE;
		    
			    /* loop until space past current location */
		        while (bleepPos < stringPos ||
			        (theString[bleepPos] != ' ' &&
			        theString[bleepPos] != '\0')) {

			        /* replace the letter */
                            
                            smurfPos = bleepPos - wordStart;
                            if (smurfPos > 4) {
                                smurfPos = 5;
                            } 

			    theString[bleepPos++] =
				    replacements[smurfPos];
			}

                        if (smurfPos == 3) {
                            theString[bleepPos + 1] = theString[bleepPos];
                            theString[bleepPos] =
				    replacements[smurfPos + 1];
                        }

			    /* start again at the last replacement */
			    /* bleepPos is a space or null, so the
			    spaceExceptionPossible will be handled */
			stringPos = bleepPos;
			swearPos = 0;
			continue;
		    }

			/* this is a swear, but we're passing it over */
		    else {

			    /* start one after where we did last time */
			stringPos = swearStart + 1;
			swearPos = 0;
			continue;
		    }
		}

		    /* found the next swear letter */
		++stringPos;
		continue;
	    }

		/* See if this letter is the same as the last */
		/* This is to catch "ffuucckk" */
	    else if (swearPos != 0 && currentChar ==
		    swears[currentSwear][swearPos - 1]) {

		    /* move to the next letter */
		++stringPos;
		continue;
	    }

		/* different letter - start one after where we started before */
	    stringPos = swearStart = swearStart + 1;
	    swearPos = 0;
	    spaceExceptionPossible = TRUE;
	    spaceCount = 0;
	}

	++currentSwear;
    }

    return profane;
}


/************************************************************************
/
/ FUNCTION NAME: Do_censor(char *destString, char *sourceString)
/
/ FUNCTION: do random stuff
/
/ AUTHOR: Brian Kelly, 06/25/02
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: collecttaxes(), floor(), wmove(), drandom(), infloat(),
/       waddstr(), mvprintw(), getanswer()
/
/ GLOBAL INPUTS: Player, *stdscr, *Statptr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle gurus, medics, etc.
/
*************************************************************************/


int Do_censor(char *destString, char *sourceString)
{
    char *swears[] = { "shit", "fuck", "cunt", "nigger", "cock", "dick", "penis", "pussy", "clit", "vagina", "dildo", "masturbate", "masturbation", "anal", "chink", "kike", "tits", "" };

    char substitute[8][5] = {
	    { 'a', '@', '\0' },
	    { 'c', '(', '\0' },
	    { 'h', '#', '\0' },
	    { 'i', 'l', '!', '1', '\0' },
	    { 'l', 'i', '!', '1', '\0' },
	    { 'o', '0', '\0' },
	    { 's', '$', '5', 'z', '\0' },
	    { '\0' } };

    int sourceLetter, swearNumber, swearLetter, letterPtr, row, column, count;
    bool newWordFlag, matchFlag, spaceFlag, swearFlag;
    char lcSource[SZ_CHAT + 64], currentLetter;

	/* make a lowercase version of the string */
    sourceLetter = 0;
    do {
      lcSource[sourceLetter] = tolower(sourceString[sourceLetter]);
    }
    while (sourceString[sourceLetter++] != '\0');

        /* start at the first letter of the source */
    sourceLetter = 0;
    newWordFlag = TRUE;
    count = 0;

        /* run through all the characters of the source string */
    while (sourceString[sourceLetter] != '\0') {

	/* if this character is a space */
      if (sourceString[sourceLetter] == ' ') {

	  /* no swears start with a space */
	*destString++ = ' ';
        newWordFlag = TRUE;
	++sourceLetter;
	continue;
      }

        /* start at the front of the list of swears */
      swearFlag = FALSE;
      swearNumber = 0; 

        /* loop through all the swear words */
      while (swears[swearNumber][0] != '\0') {

          /* start with the first letter of this swear */
        swearLetter = 0;

	  /* we start checking from our current position */
        letterPtr = sourceLetter;
        spaceFlag = FALSE;

	  /* check the swear from this letter */
        do {

	    /* assume no match */
          matchFlag = FALSE;

	    /* convert the current character to lowercase */
          currentLetter = lcSource[letterPtr];

	    /* if the current letter is a space and
		we're in the middle of a swear */
	  if (currentLetter == ' ' && swears[swearNumber][swearLetter] != '\0') {

	      /* set the space flag */
	    spaceFlag = TRUE;

	      /* increment to the next string character and re-loop */
	    ++letterPtr;
	    continue;
	  }

	    /* if the letter is the next in the swear */
          if (swears[swearNumber][swearLetter] == currentLetter) {

	      /* swear matches */
	    matchFlag = TRUE;
	  }

	      /* see if a substitute character was used (swear isn't NULL) */
	  else if (swears[swearNumber][swearLetter] != '\0') {

            row = 0;

	      /* run through the substitute list */
	    while (substitute[row][0] != '\0') {

	        /* if this sub char is the next in the swear */
	      if (substitute[row][0] == swears[swearNumber][swearLetter]) {

	        column = 1;

		  /* run through the list */
	        while (substitute[row][column] != '\0') {

		    /* is the string letter this replacement character? */
		  if (currentLetter == substitute[row][column]) {

	              /* swear matches */
	            matchFlag = TRUE;
		    break;
		  }

		  ++column;
		}

		  /* a letter is only listed as a sub once */
		break;
	      }

	      ++row;
	    }
          }

	    /* if this letter matches the next in the swear */
	  if (matchFlag) {

	      /* move to the next letter of the string and swear */
	    ++letterPtr;
	    ++swearLetter;
	  }

	    /* otherwise, if we're in the middle of a swear */
	  else if (swearLetter) {

	      /* if the letter is a repeat of the last */
            if (swears[swearNumber][swearLetter - 1] == currentLetter) {

	        /* swear is a repeat */
	      matchFlag = TRUE;
	    }

	      /* see if it could be a substitute character repeat */
	    else {

              row = 0;

	        /* run through the substitute list */
	      while (substitute[row][0] != '\0') {

	          /* if this sub char is the previous one in the swear */
	        if (substitute[row][0] ==
		    swears[swearNumber][swearLetter - 1]) {

	          column = 1;

		    /* run through the list */
	          while (substitute[row][column] != '\0') {

		      /* is the string letter this replacement character? */
		    if (currentLetter == substitute[row][column]) {

	                /* swear matches */
	              matchFlag = TRUE;
		      break;
		    }

		    ++column;
		  }

		    /* a letter is only listed as a sub once */
		  break;
	        }

	        ++row;
	      }
            }

	      /* if this letter matches the previous in the swear */
	    if (matchFlag) {

	        /* move to the next letter of the string only */
	      ++letterPtr;
	    }
	  }
        }
        while (matchFlag);

	  /* see if a complete swear was found */
	if (swears[swearNumber][swearLetter] == '\0') {

	    /* if the swear contains spaces and didn't start a word */
	  if (!newWordFlag && spaceFlag) {

	      /* reject the word for replacement */
	    swearLetter = 0;
	  }

	    /* if the swear is valid */
	  else {

	    swearFlag = TRUE;

	      /* break out of the search, we'll replace it below */
	    break;
	  }
	}

	++swearNumber;
      }

	/* if a complete swear was found */
      if (swearFlag) {

	  /* if the original was capitalized */
	if (isupper(sourceString[sourceLetter])) {

	    /* if the second letter was uppercase */
	  if (isupper(sourceString[sourceLetter + 1])) {

	      /* assume it was all uppercase */
	    strcpy(destString, "SMURF");
	  }
	  else {

	      /* just capitalize it */
	    strcpy(destString, "Smurf");
	  }
	}
	  /* the word was lowercase */
	else {

	    /* replace the word */
	  strcpy(destString, "smurf");
	}

	  /* move the pointer over the replacement */
	destString += 5;

	  /* the we continue from the end of the swear */
	sourceLetter = letterPtr;

	  /* the next character is never the start of a new word */
	newWordFlag = FALSE;

	  /* increment the counter */
	++count;
      }

	/* no swear was found */
      else {

	  /* if this letter is a space */
	if (sourceString[sourceLetter] == ' ') {

	    /* the next character will be the start of a new word */
	  newWordFlag = TRUE;
	}
	else {
	  newWordFlag = FALSE;
	}

	  /* copy over this letter and move to the next letter */
	*destString++ = sourceString[sourceLetter++];
	  
      }

    }

      /* add a null */
    *destString = '\0';

        /* see if this was an buffer overflow attempt */
    if (count > 20) {

	return TRUE;
    }

    return FALSE;
}


/************************************************************************
/
/ FUNCTION NAME: int Do_spam_check(struct client_t *c, char *message)
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

int Do_spam_check(struct client_t *c, char *message)
{
    int i, count;

        /* move up all the previous chat times and length */
    for (i = 8; i >= 0; i--) {
        c->chatTimes[i + 1] = c->chatTimes[i];
        c->chatLength[i + 1] = c->chatLength[i];
    }

        /* add the current moment into the lists */
    c->chatTimes[0] = time(NULL);
    c->chatLength[0] = (int) floor(strlen(message) / 80.0);

        /* see if we can find a chat infraction */
    for (i = 3; i < 9; i++) {

             /* player can send 3 + n messages every n^2 seconds */
        if (c->chatTimes[0] - c->chatTimes[i] <= (i - 2) * (i - 2)) {

                /* tag this player as muted */
            Do_caught_spam(c, H_SPAM);
            return TRUE;
        }
    }

        /* add up the lines of text from the last nine messages */
    count = 0;
    for (i = 0; i <= 9; i++) {
        count += c->chatLength[i];
    }

printf("count check: %d.\n", count);
        /* if the number of lines exceeds 14 */
    if (count >= 14) {

            /* clear the history so we don't nail them forever */
        for (i = 0; i <= 9; i++) {
            c->chatLength[i] = 0;
        }

            /* tag this player as muted */
        Do_caught_spam(c, H_FLOOD);
        return TRUE;
    }

        /* no problems */
    return FALSE;
}


/******************************************************************
/
/ FUNCTION NAME: Do_title_page(struct client_t *c)
/
/ FUNCTION: print title page
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/3/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: fread(), fseek(), fopen(), fgets(), wmove(), strcpy(),
/       fclose(), strlen(), waddstr(), sprintf(), wrefresh()
/
/ DESCRIPTION:
/       Print important information about game, players, etc.
/
*************************************************************************/

Do_title_page(struct client_t *c)
{
struct game_t *game_ptr, *first_ptr, *second_ptr;
short    councilfound;   /* set if we find a member of the council */
char	string_buffer[SZ_LINE]; 
char	error_msg[SZ_ERROR_MESSAGE];
int	error;

	/* print a header */
    Do_send_line(c,
	    "W e l c o m e   t o   P h a n t a s i a (vers. 4.03)!\n");

	/* lock the realm */
    Do_lock_mutex(&c->realm->realm_lock);

	/* display the current king, if any */
    if (c->realm->king == NULL || !c->realm->king_flag) {

      if (c->realm->king == NULL) {
        Do_send_line(c, "There is no ruler at this time.\n");
      } else {
	if (c->realm->king->description != NULL) {

                sprintf(string_buffer,
		        "The present steward is %s  Level: %.0lf\n",
                        c->realm->king->description->name,
		        c->realm->king->description->level);

	        Do_send_line(c, string_buffer);
	}
	else {
	    sprintf(string_buffer,
	 "[%s] Information on present steward unavailable in Do_title_page.\n",
	   c->connection_id);

	    Do_log_error(string_buffer);
	}
      }
    

    }
    else {
	    /* make sure the character has a description in place */
	if (c->realm->king->description != NULL) {

	        /* print out the present king/queen */
	    if (c->realm->king->description->gender == MALE) {

                sprintf(string_buffer,
		        "The present ruler is King %s  Level: %.0lf\n",
                        c->realm->king->description->name,
		        c->realm->king->description->level);
	    }
	    else {

                sprintf(string_buffer,
		        "The present ruler is Queen %s  Level: %.0lf\n",
                        c->realm->king->description->name,
		        c->realm->king->description->level);
	    }
	}
	else {
	    sprintf(string_buffer,
	   "[%s] Information on present ruler unavailable in Do_title_page.\n",
	   c->connection_id);

	    Do_log_error(string_buffer);
	}

	Do_send_line(c, string_buffer);
    }

	/* display the current valar, in any */
    if (c->realm->valar_name[0] != '\0') {

            sprintf(string_buffer, "The Valar is %s\n",
                    c->realm->valar_name);

/*
	if (c->realm->valar->description != NULL) {

            sprintf(string_buffer, "The Valar is %s   Level:  %.0lf\n",
                    c->realm->valar->description->name,
		    c->realm->valar->description->level);
	}
	else {
	    sprintf(string_buffer,
		   "[%s] Information on valar unavailable in Do_title_page.\n",
		   c->connection_id);

	    Do_log_error(string_buffer);
	}

*/
	Do_send_line(c, string_buffer);
    }


	/* search for council members */
    councilfound = 0;
    game_ptr = c->realm->games;

	/* loop through all the games */
    while (game_ptr != NULL && councilfound < 5) {

	    /* if the player is playing and is on the council */
	if (game_ptr->description != NULL &&
		(game_ptr->description->special_type == SC_COUNCIL ||
		game_ptr->description->special_type == SC_EXVALAR)) {

		/* print a header if this is the first council member */	
            if (councilfound == 0) {
                Do_send_line(c, "Council of the Wise:\n");
            }

		/* print out the member */

            sprintf(string_buffer, "%s  Level: %.0lf\n",
		    game_ptr->description->name,
		    game_ptr->description->level);

	    Do_send_line(c, &string_buffer);

		/* indicate that we've found another member */

	    councilfound++;
	}


	    /* look at the next game */

	game_ptr = game_ptr->next_game;
    }


        /* search for the two highest players */
    first_ptr = NULL;
    second_ptr = NULL;
    game_ptr = c->realm->games;

    while (game_ptr != NULL) {

	    /* if the player is in the game */
	if (game_ptr->description != NULL) {

	        /* see if the current game is higher level than the current */
	    if ((first_ptr == NULL || (game_ptr->description->level > first_ptr->description->level)) && 
              (game_ptr->description->special_type < SC_STEWARD) &&
              (game_ptr->description->wizard < 3)) {
        
          

		    /* put the current game into first position */
	        second_ptr = first_ptr;
	        first_ptr = game_ptr;
	    }
	    else if ((second_ptr == NULL || 
              (game_ptr->description->level > second_ptr->description->level)) && 
              (game_ptr->description->special_type < SC_STEWARD) &&
              (game_ptr->description->wizard < 3)) {

		    /* put the current game as the second */
	        second_ptr = game_ptr;
	    }
	}
	    /* point to the next game */
	game_ptr = game_ptr->next_game;
    }

	/* print out the search results */
    if (first_ptr != NULL) {
	if (second_ptr != NULL) {

	    sprintf(string_buffer, "Highest commoners are: %s  Level: %.0lf  and  %s  Level: %.0lf\n",
		    first_ptr->description->name,
		    first_ptr->description->level,
		    second_ptr->description->name,
		    second_ptr->description->level);
	}
	else {

	    sprintf(string_buffer, "Highest commoner is: %s  Level:%.0lf\n",
		    first_ptr->description->name,
		    first_ptr->description->level);
	}
	Do_send_line(c, &string_buffer);
    }

    /* print last to die */

/*
    if ((fp = fopen(_PATH_LASTDEAD,"r")) != NULL
        && fgets(Databuf, SZ_DATABUF, fp) != NULL)
        {
        mvaddstr(19, 25, "The last character to die was:\n");
        mvaddstr(20, 40 - strlen(Databuf) / 2,Databuf);
        fclose(fp);
        }

*/

    Do_unlock_mutex(&c->realm->realm_lock);

}


/***************************************************************************
/ FUNCTION NAME: Do_lowercase(char *dest, char *source);
/
/ FUNCTION: Handles error messages
/
/ AUTHOR:  Brian Kelly, 1/1/01
/
/ ARGUMENTS: 
/	int error - the error code to be returned
/	char *message - the error message to be printed
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_lowercase(char *dest, char *source)
{
    int len, i;

    len = strlen(source);

    for (i = 0; i< len; i++) {
	dest[i] = tolower(source[i]);
    }

    dest[len] = '\0';
    return;
}


/***************************************************************************
/ FUNCTION NAME: Do_create_password(char *string);
/
/ FUNCTION: Handles error messages
/
/ AUTHOR:  Brian Kelly, 1/1/01
/
/ ARGUMENTS: 
/	int error - the error code to be returned
/	char *message - the error message to be printed
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_create_password(char *string)
{
    int len, i, letter;

	/* choose a password length */
    len = 5 * RND() + 8;

	/* pick a random character */
    for (i = 0; i< len; i++) {
	letter = 50 + RND() * 55;

	    /* skip over non characters and numbers plus "01IOilo" */
	if (letter > 57)
	    letter += 7;
	if (letter > 72)
	    ++letter;
	if (letter > 78)
	    ++letter;
	if (letter > 90)
	    letter += 6;
	if (letter > 104)
	    ++letter;
	if (letter > 107)
	    ++letter;
	if (letter > 110)
	    ++letter;

	string[i] = (char) letter;
    }

    string[len] = '\0';
    return;
}


/***************************************************************************
/ FUNCTION NAME: Do_move_close(double *x, double *y, float distance);
/
/ FUNCTION: Handles error messages
/
/ AUTHOR:  Brian Kelly, 1/5/01
/
/ ARGUMENTS: 
/	int error - the error code to be returned
/	char *message - the error message to be printed
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_move_close(double *x, double *y, double maxDistance) 
{
    double angle, distance;

    angle = RND() * 2 * 3.14159;

    distance = RND() * maxDistance;
    if (distance < 1.0) {
	distance = 1.0;
    }

	/* add half a point because floor(-3.25) = -4 */
    *x += floor(cos(angle) * distance + .5);
    *y += floor(sin(angle) * distance + .5);

    return;
}

/************************************************************************
/
/ FUNCTION NAME: Do_replace_repetition(char *theString)
/
/ FUNCTION: do random stuff
/
/ AUTHOR: Brian Kelly, 1/2/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: collecttaxes(), floor(), wmove(), drandom(), infloat(),
/       waddstr(), mvprintw(), getanswer()
/
/ GLOBAL INPUTS: Player, *stdscr, *Statptr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle gurus, medics, etc.
/
*************************************************************************/

Do_replace_repetition(char *theString)
{
    char previousChar, thisChar;
    char *writePtr, *readPtr;
    int count;

    if (theString[0] == '\0') {
	return;
    }

    writePtr = readPtr = &theString[0];

    while (*readPtr == ' ') {
	++readPtr;
    }

    count = 1;
    previousChar = tolower(*readPtr);
    *writePtr++ = *readPtr++;

    while (*readPtr != '\0') {

	thisChar = tolower(*readPtr);

	    /* if this is the same character */
	if (thisChar == previousChar) {

	    ++count;

		/* see if this is the fourth character that's the same */
	    if (count > 4) {

		    /* skip over it */
		readPtr++;
		continue;
	    }

	}
	else if (thisChar != ' ') {
	    previousChar = thisChar;
	    count = 1;
	}

	    /* copy the character over */
	*writePtr++ = *readPtr++;
    }

	/* add a null */
    *writePtr = '\0';
}


/***************************************************************************
/ FUNCTION NAME: Do_direction(struct client_t *c, double *x, double *y, char *direction)
/
/ FUNCTION: Derives the direction of a target coordinate from current location
/
/ AUTHOR:  Brian Kelly, 5/9/01
/
/ ARGUMENTS: 
/	
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       
/
****************************************************************************/

Do_direction(struct client_t *c, double *x, double *y, char *direction)
{
    double radians;

	/* if we're on the X coordinate, get radians manually */
    if (*x - c->player.x == 0) {

	if (*y - c->player.y > 0) {
	    radians = 1.5708;
	}
	else if (*y - c->player.y < 0) {
	    radians = 4.7124;
	}
	else {
	    strcat(direction, "down");
	    return;
	}
    }
    else {

	    /* find the angle */
        radians = atan((*y - c->player.y)/(*x - c->player.x));

	    /* add 180 degrees if on the other side of the plane */
	if (*x - c->player.x < 0) {
	    radians += 3.1416;
	}
    }

	/* run around the circle */
    if (radians > 4.3197) {
	strcat(direction, "south");
    }
    else if (radians > 3.5343) {
	strcat(direction, "south-west");
    }
    else if (radians > 2.7489) {
	strcat(direction, "west");
    }
    else if (radians > 1.9635) {
	strcat(direction, "north-west");
    }
    else if (radians > 1.1781) {
	strcat(direction, "north");
    }
    else if (radians > .3927) {
	strcat(direction, "north-east");
    }
    else if (radians > -.3927) {
	strcat(direction, "east");
    }
    else if (radians > -1.1781) {
	strcat(direction, "south-east");
    }
    else {
	strcat(direction, "south");
    }
	
    return;
}

int Do_maxmove(struct client_t *c) {
    if ((c->player.circle > 19) && (c->player.circle < 36)) {
        return (int) MIN(ceil(c->player.level / 50.0) + 1.5, 10.0);
    } else {
        return (int) MIN(floor((c->player.level * 1.5) + 1.5), 100.0);
    }
}

int Do_anglemove(struct client_t *c) {
    return (int) MAX(1.0, floor(Do_maxmove(c) * .707106781));
}

