/*
 * itcombat.c       Routines to handle inter-terminal combat
 */

#include "include.h"

/***************************************************************************
/ FUNCTION NAME: Do_opponent_struct(struct client_t *c, struct opponent_t *theOpponent)
/
/ FUNCTION: Check to see if a player entered an occupied square
/
/ AUTHOR:  Brian Kelly, 8/16/99
/
/ ARGUMENTS: 
/	struct server_t s - the server strcture
/	struct game_t this_game - pointer to the player we're checking
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

void Do_opponent_struct(struct client_t *c, struct opponent_t *theOpponent)
{
    char string_buffer[SZ_LINE];

    c->battle.ring_in_use = FALSE;
    c->battle.melee_damage = 0.0;
    c->battle.skirmish_damage = 0.0;
    c->battle.tried_luckout = FALSE;

    strcpy(theOpponent->name, c->modifiedName);
    theOpponent->processID = c->game->clientPid;

      /* passable experience = all player exp now that nicking is capped */
      /* Kings and higher are immune to nicking */
    if  (c->player.special_type >= SC_KING) {
	theOpponent->experience = 0.0;
    }
    else {
        theOpponent->experience = floor(c->player.experience /
                (30 * c->player.level + 1));
    }

    theOpponent->strength = c->player.strength + c->player.sword;
    theOpponent->max_strength = theOpponent->strength;
    theOpponent->energy = c->player.energy;
    theOpponent->max_energy = c->player.max_energy + c->player.shield;
    theOpponent->speed = c->player.quickness + 1;
    theOpponent->max_speed = theOpponent->speed;
    theOpponent->brains = c->player.brains;
    theOpponent->size = c->player.level;

    /* the bigger they are, the harder they fall to all or nothing */
    theOpponent->sin = c->player.sin + floor(c->player.level / 100);

    sprintf(string_buffer, 
            "%s, %s, Level %.0lf, Sin %.0f has effective Sin of %.0lf\n",
            c->player.lcname, 
            c->realm->charstats[c->player.type].class_name, 
            c->player.level,
            c->player.sin,
            theOpponent->sin);

    Do_log(COMBAT_LOG, string_buffer);

    theOpponent->shield = 0;
    theOpponent->special_type = SM_IT_COMBAT;
    theOpponent->treasure_type = 0;
    theOpponent->flock_percent = 0;

        /* throw up spells for combat */
    Do_starting_spells(c);
}


/***************************************************************************
/ FUNCTION NAME: Do_setup_it_combat(struct client_t *c, struct game_t *theGame)
/
/ FUNCTION: Check to see if a player entered an occupied square
/
/ AUTHOR:  Brian Kelly, 8/16/99
/
/ ARGUMENTS: 
/	struct server_t s - the server strcture
/	struct game_t this_game - pointer to the player we're checking
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_setup_it_combat(struct client_t *c, struct game_t *theGame)
{
float ftemp;

/* WARNING: c->realm->realm_lock is passed LOCKED! */
/*
    Do_lock_mutex(&c->realm->realm_lock);
*/
    struct it_combat_t *theCombat;
    struct event_t *event_ptr;
    struct it_combat_t *it_combat_ptr;
    char string_buffer[SZ_LINE];

    theCombat = (struct it_combat_t *) Do_malloc(SZ_IT_COMBAT);
    theCombat->opponentFlag[0] = FALSE;
    theCombat->opponentFlag[1] = FALSE;
    theCombat->next_opponent = NULL;
    theCombat->player_ptr = NULL;
    Do_init_mutex(&theCombat->theLock);

    c->game->it_combat = theCombat;

    if (theGame->it_combat == NULL) {

	theGame->it_combat = theCombat;
        Do_unlock_mutex(&c->realm->realm_lock);

	event_ptr = (struct event_t *) Do_create_event();
	event_ptr->type = IT_COMBAT_EVENT;
	event_ptr->arg4 = theCombat;
	event_ptr->to = theGame;
	event_ptr->from = c->game;

	Do_send_event(event_ptr);

        Do_send_clear(c);

        if (c->player.blind)
            sprintf(string_buffer,
		    "You hear another player nearby!\n");
	else
            sprintf(string_buffer,
		    "You see another player in the distance!\n");
    }
    else {
	
	it_combat_ptr = theGame->it_combat;

	while (it_combat_ptr->next_opponent != NULL)
	    it_combat_ptr = it_combat_ptr->next_opponent;

	it_combat_ptr->next_opponent = theCombat;
        Do_unlock_mutex(&c->realm->realm_lock);

        Do_send_clear(c);

        if (c->player.blind)
            sprintf(string_buffer,
		    "You sense that several players are nearby!\n");
	else
            sprintf(string_buffer,
		    "You see several players in the distance!\n");
    }

    Do_send_line(c, string_buffer);

    Do_lock_mutex(&theCombat->theLock);
    Do_opponent_struct(c, &theCombat->opponent[0]);
    c->battle.opponent = &theCombat->opponent[1];
    Do_unlock_mutex(&theCombat->theLock);

	/* for attacking another player, pick up sin */
    Do_sin(c, .5);
    sprintf(string_buffer, "%s ATTACKED another player.\n", c->player.lcname);
    Do_log(BATTLE_LOG, string_buffer);

    Do_send_line(c, "Waiting for the other player(s)...\n");

    Do_it_combat_turns(c, theCombat, &theCombat->opponent[0],
	    &theCombat->opponentFlag[0], &theCombat->opponentFlag[1]);

    return;
}


/***************************************************************************
/ FUNCTION NAME: Do_it_combat(struct client_t *c, struct event_t *theEvent)
/
/ FUNCTION: Check to see if a player entered an occupied square
/
/ AUTHOR:  Brian Kelly, 8/16/99
/
/ ARGUMENTS: 
/	struct server_t s - the server strcture
/	struct game_t this_game - pointer to the player we're checking
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_it_combat(struct client_t *c, struct event_t *theEvent, int available)
{
    struct it_combat_t *theCombat;
    char string_buffer[SZ_LINE];

    Do_send_clear(c);

    theCombat = (struct it_combat_t *) theEvent->arg4;

    Do_lock_mutex(&theCombat->theLock);

	/* if our flag is set, there is a problem */
    if (!theCombat->opponentFlag[1]) {

        theCombat->opponentFlag[0] = TRUE;
        theCombat->opponentFlag[1] = FALSE;

        Do_opponent_struct(c, &theCombat->opponent[1]);

        if (c->player.energy <= 0) {

	    theCombat->message = IT_JUST_DIED;
            Do_unlock_mutex(&theCombat->theLock);
            kill(theCombat->opponent[0].processID ,SIGIO);
            Do_lock_mutex(&c->realm->realm_lock);
            c->game->it_combat = NULL;
            Do_unlock_mutex(&c->realm->realm_lock);
	    return;
        }
        else if (!available) {

	    theCombat->message = IT_JUST_LEFT;
            Do_unlock_mutex(&theCombat->theLock);
            kill(theCombat->opponent[0].processID ,SIGIO);
            Do_lock_mutex(&c->realm->realm_lock);
            c->game->it_combat = NULL;
            Do_unlock_mutex(&c->realm->realm_lock);
	    return;
        }
        else {
            theCombat->message = IT_REPORT;
	}
    }

    c->battle.opponent = &theCombat->opponent[0];

    if (c->player.blind) {
	if (c->stuck) {
	    c->stuck = FALSE;
            sprintf(string_buffer,
	       "Before you can move, you hear another player in the area!\n");
	}
	else {
            sprintf(string_buffer,
		    "You hear another player approach!\n");
	}
    }
    else {
	if (c->stuck) {
	    c->stuck = FALSE;
	    sprintf(string_buffer,
		    "Before you can move, %s enters the area!\n",
		    theCombat->opponent[0].name);
	}
	else {
	    sprintf(string_buffer,
		    "You see %s approaching from the distance!\n",
		    theCombat->opponent[0].name);
	}
    }

    Do_unlock_mutex(&theCombat->theLock);

    kill(theCombat->opponent[0].processID ,SIGIO);

    Do_send_line(c, string_buffer);
    Do_send_buffer(c);
    sleep(2);

    Do_it_combat_turns(c, theCombat, &theCombat->opponent[1],
	    &theCombat->opponentFlag[1], &theCombat->opponentFlag[0]);

    return;
}


/***************************************************************************
/ FUNCTION NAME: void Do_it_combat_turn(struct client_t *c, struct it_combat_t *theCombat, struct opponent_t *myStats, bool *my Flag, bool *hisFlag)
/
/ FUNCTION: Check to see if a player entered an occupied square
/
/ AUTHOR:  Brian Kelly, 8/16/99
/
/ ARGUMENTS: 
/	struct server_t s - the server strcture
/	struct game_t this_game - pointer to the player we're checking
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
Do_it_combat_turns(struct client_t *c, struct it_combat_t *theCombat,
	struct opponent_t *myStats, bool *myFlag, bool *hisFlag)
{
    short theMessage;
    double theArg;
    short newMessage;
    double newArg;
    char string_buffer[SZ_LINE];
    char error_msg[SZ_ERROR_MESSAGE];
    struct event_t *event_ptr;
    struct realm_object_t *object_ptr;
    float ftemp;

    for (;;) {

	c->timeoutFlag = 0;
	Do_wait_flag(c, myFlag, &theCombat->theLock); 

            /* handle our flag  */
	Do_lock_mutex(&theCombat->theLock);

	    /* see if the player timed out */
	if (c->timeoutFlag && theCombat->message != IT_BORED) {

	    theMessage = IT_ABANDON;
	}
	else {
	    theMessage = theCombat->message;
	    theArg = theCombat->arg1;
	    *myFlag = FALSE;
	}

            /* unlock the flags so the other thread can check it  */
	Do_unlock_mutex(&theCombat->theLock);

	    /* if the other thread received an event, we're server */
	if (theMessage == IT_REPORT) {

	    if (c->player.blind)

	        sprintf(string_buffer,
			"You have encountered another player.\n");

	    else {
		sprintf(string_buffer, "You have encountered %s.\n",
			c->battle.opponent->name);

	        Do_send_line(c, string_buffer);
	    }

		/* defender always attacks first */
	    theMessage = IT_DEFEND;
	}

	    /* battle continues, decide who goes next */
	if (theMessage == IT_CONTINUE) {

	    if (RND() * c->battle.opponent->speed > RND() *
		    myStats->speed) {

		theMessage = IT_DEFEND;
	    }
	    else
		theMessage = IT_ECHO;
	}

	    /* if the opponent just left the game */
	if (theMessage == IT_JUST_LEFT) {

	    sprintf(string_buffer,
		    "You arrive just to see %s vanish from the realm.\n",
		    c->battle.opponent->name);

	    Do_send_line(c, string_buffer);

	    c->battle.opponent->experience = 0.0;
	    theMessage = IT_DEFEAT;
	}

	    /* if the opponent passes on combat */
	if (theMessage == IT_JUST_DIED) {

	    sprintf(string_buffer, "%s falls dead at your feet.\n",
		    c->battle.opponent->name);

	    Do_send_line(c, string_buffer);

	    c->battle.opponent->experience = 0.0;
	    theMessage = IT_DEFEAT;
	}

	    /* if the opponent passes on combat */
	if (theMessage == IT_BORED) {

	    sprintf(string_buffer,
		    "%s became bored and left the battlefield.\n",
		    c->battle.opponent->name);

	    Do_send_line(c, string_buffer);

	    c->battle.opponent->experience = 0.0;
	    theMessage = IT_DEFEAT;
	}

	    /* if the opponent is having network trouble */
	if (theMessage == IT_CONCEDE) {

	    sprintf(string_buffer, "%s concedes the battle.\n",
		    c->battle.opponent->name);

	    Do_send_line(c, string_buffer);

	    theMessage = IT_DEFEAT;
	}

	newMessage = IT_CONTINUE;
	string_buffer[0] = '\0';

	switch (theMessage) {

	    /* we've encountered a problem - every thread for himself */
	case IT_ABANDON:

            sprintf(error_msg,
		    "[%s] now abandoning it_combat in Do_it_combat_turns.\n",
                    c->connection_id);

            Do_log_error(error_msg);

		/* leave a message for our opponent */
	    theCombat->message = IT_BORED;
	    Do_lock_mutex(&theCombat->theLock);
	    *hisFlag = TRUE;
	    Do_unlock_mutex(&theCombat->theLock);
	    kill(c->battle.opponent->processID, SIGIO);

                /* clear any spells */
            Do_energy(c, c->player.energy, c->player.max_energy,
                    c->player.shield, 0.0, FALSE);

            Do_strength(c, c->player.max_strength, c->player.sword, 0, FALSE);

            Do_speed(c, c->player.max_quickness, c->player.quicksilver, 0,
                    FALSE);

                /* pull ourselves out of combat */
            Do_lock_mutex(&c->realm->realm_lock);
            c->game->it_combat = NULL;
	    c->game->virtual = TRUE;
            Do_unlock_mutex(&c->realm->realm_lock);

		/* get away from this square and this battle */
            event_ptr = (struct event_t *) Do_create_event();
            event_ptr->to = c->game;
            event_ptr->from = c->game;
            event_ptr->type = MOVE_EVENT;
	    event_ptr->arg3 = A_NEAR;
            Do_file_event(c, event_ptr);

            return;

	    /* we decided to attack, so have the opponent echo an attack */
	case IT_ECHO:

	    newMessage = IT_DEFEND;
	    break;

	    /* we are to attack */
	case IT_ATTACK:

	        /* give the character fatigue */
	    ++c->battle.rounds;

	    Do_speed(c, c->player.max_quickness, c->player.quicksilver,
	            c->battle.speedSpell, FALSE);

	    myStats->speed = c->player.quickness;

                /* if the player has a ring in use */
            if (c->battle.ring_in_use) {

                    /* age ring */
                if (c->player.ring_type != R_DLREG) {
                    --c->player.ring_duration;
                }

                    /* regardless of ring type, heal the character */
                Do_energy(c, c->player.max_energy + c->player.shield,
                        c->player.max_energy, c->player.shield,
                        c->battle.force_field, FALSE);
            }

	    Do_send_line(c,
		    "You attack this round.  Please choose your attack:\n");

	    Do_playerhits(c, &newMessage, &newArg);

		/* if the player put up a shield, copy the info over */
	    if (newMessage == IT_SHIELD) {
		myStats->shield = c->battle.force_field;
	    }

	    break;

	    /* we are to defend */
	case IT_DEFEND:

	    sprintf(string_buffer,
		   "%s attacks for this round.  Please wait for a decision.\n",
		   c->battle.opponent->name);

	    newMessage = IT_ATTACK;
	    break;

	    /* the other thread is declaring victory */
	case IT_VICTORY:

	    newMessage = IT_DEFEAT;

		/* clear any spells */
	    Do_energy(c, c->player.energy, c->player.max_energy,
		    c->player.shield, 0.0, FALSE);

	    Do_strength(c, c->player.max_strength, c->player.sword, 0, FALSE);

	    Do_speed(c, c->player.max_quickness, c->player.quicksilver, 0,
		    FALSE);

	    c->battle.force_field = 0;
	    c->battle.strengthSpell = 0;
	    c->battle.speedSpell = 0;

	    break;

	    /* the other thread stands defeated - we clean up */
	case IT_DEFEAT:

		/* award experience if there is any to be had */
	    if (c->battle.opponent->experience) {
		Do_experience(c, c->battle.opponent->experience, FALSE);

		    /* for killing another player, pick up sin */
		Do_sin(c, sqrt(c->battle.opponent->size) / 3.1);

sprintf(error_msg, "%s KILLED %s in IT combat.\n", c->player.lcname,
c->battle.opponent->name);
Do_log(BATTLE_LOG, error_msg);

	    }

		/* clear any spells */
	    Do_energy(c, c->player.energy, c->player.max_energy,
		    c->player.shield, 0.0, FALSE);

	    Do_strength(c, c->player.max_strength, c->player.sword, 0, FALSE);

	    Do_speed(c, c->player.max_quickness, c->player.quicksilver, 0,
		    FALSE);

	    c->battle.force_field = 0;
	    c->battle.strengthSpell = 0;
	    c->battle.speedSpell = 0;

		/* if we're in the throne room and are not king */
		/* become king NOW (another battle may be pending) */
	    if (c->player.location == PL_THRONE && c->player.special_type !=
		    SC_KING && c->player.special_type != SC_STEWARD) {

		    /* become steward */
        	if (c->player.level >= 10 && c->player.level < 200) {
		    Do_steward(c);
		}
        	else if (c->player.level >= 1000 && c->player.level < 2000) {
		    Do_king(c);
		}
	    }

	    Do_lock_mutex(&c->realm->realm_lock);

		/* if there are no more battles to fight */
	    if (theCombat->next_opponent == NULL) {

		    /* pull ourselves out of combat */
	        c->game->it_combat = NULL;
	        Do_unlock_mutex(&c->realm->realm_lock);

		    /* create a body event for ourselves */
		    /* opponent put player struct here if he will die */
	        if (theCombat->player_ptr != NULL) {
		    event_ptr = (struct event_t *) Do_create_event();
		    event_ptr->to = c->game;
		    event_ptr->from = c->game;
		    event_ptr->type = CORPSE_EVENT;

		        /* the body can be cursed */
		    event_ptr->arg1 = TRUE;
		    event_ptr->arg4 = theCombat->player_ptr;
		    Do_file_event(c, event_ptr);
	        }
	    }

		/* if there are more opponents to fight */
	    else {
	        event_ptr = (struct event_t *) Do_create_event();
                event_ptr->to = c->game;
                event_ptr->from = c->game;
	       	event_ptr->type = IT_COMBAT_EVENT;
	        event_ptr->arg4 = (void *)theCombat->next_opponent;
	        Do_file_event(c, event_ptr);

		    /* put ourselves back in combat */
		c->game->it_combat = theCombat->next_opponent;

		    /* create a realm object for this kill, if necessary */
		if (theCombat->player_ptr != NULL) {

		    object_ptr = (struct realm_object_t *)
			    Do_malloc(SZ_REALM_OBJECT);

		    object_ptr->x = c->player.x;
		    object_ptr->y = c->player.y;
		    object_ptr->type = CORPSE;
		    object_ptr->arg1 = theCombat->player_ptr;
		    object_ptr->next_object = c->realm->objects;
		    c->realm->objects = object_ptr;
		}

	        Do_unlock_mutex(&c->realm->realm_lock);
	    }

	    Do_unlock_mutex(&theCombat->theLock);
	    Do_destroy_mutex(&theCombat->theLock);
	    free((void *)theCombat);

	    Do_send_line(c, "You are victorious!\n");
	    Do_more(c);
	    Do_send_clear(c);

	    return;

	case IT_MELEE:

	    sprintf(string_buffer,
		    "%s melees with you hitting %.0f times!\n",
		    c->battle.opponent->name, theArg);

	    Do_energy(c, myStats->energy, c->player.max_energy,
		    c->player.shield, myStats->shield, FALSE);

	    break;

	case IT_SKIRMISH:

	    sprintf(string_buffer,
		    "%s skirmishes with you hitting %.0f times!\n",
		    c->battle.opponent->name, theArg);

	    Do_energy(c, myStats->energy, c->player.max_energy,
		    c->player.shield, myStats->shield, FALSE);

	    break;

	case IT_NICKED:

	    c->battle.opponent->experience += theArg;
	    theArg *= 30.0 * c->player.level + 1.0;

	    sprintf(string_buffer,
		    "%s nicks you taking %.0f experience!\n",
		    c->battle.opponent->name, theArg);

	    Do_energy(c, myStats->energy, c->player.max_energy,
		    c->player.shield, myStats->shield, FALSE);

	    Do_speed(c, c->player.max_quickness, c->player.quicksilver,
		    c->battle.speedSpell + 2, FALSE);

		/* only subtract positive values, characters managed to
			get negative experience */

	    if (theArg > 0) {
	        c->player.experience -= theArg;
	    }

	    break;

        case IT_EVADED:

	    sprintf(string_buffer, "%s flees the battle!\n",
		    c->battle.opponent->name);

	    c->battle.opponent->experience = 0;
	    newMessage = IT_VICTORY;
	    break;

        case IT_WIZEVADE:

	    sprintf(string_buffer, "%s laughs at your pitiful attack and teleports away!\n",
		    c->battle.opponent->name);

	    c->battle.opponent->experience = 0;
	    newMessage = IT_VICTORY;
	    break;

        case IT_NO_EVADE:

	    sprintf(string_buffer,
		    "%s tried to flee, but couldn't shake you!\n",
		    c->battle.opponent->name);

	    break;

        case IT_LUCKOUT:

	    sprintf(string_buffer, "%s successfully lucks out in the battle!\n",
		    c->battle.opponent->name);

	    Do_energy(c, myStats->energy, c->player.max_energy,
		    c->player.shield, myStats->shield, FALSE);

	    break;

        case IT_NO_LUCKOUT:

	    sprintf(string_buffer,
		    "%s failed to luckout in the battle!\n",
		    c->battle.opponent->name);

	    break;

        case IT_RING:

		sprintf(string_buffer, "%s puts on a ring!\n",
			c->battle.opponent->name);

		break;

        case IT_NO_RING:

	    sprintf(string_buffer, "%s searches in vain for a ring!\n",
		    c->battle.opponent->name);

	    break;

        case IT_ALL_OR_NOT:

	    sprintf(string_buffer,
		    "%s casts All or Nothing, blasting you for %.0f damage!\n",
		    c->battle.opponent->name, theArg);

	    Do_energy(c, myStats->energy, c->player.max_energy,
		    c->player.shield, myStats->shield, FALSE);

	    break;

        case IT_NO_ALL_OR_NOT:

	    sprintf(string_buffer,
		    "%s flubs an attempt to cast All or Nothing!\n",
		    c->battle.opponent->name);

	    break;

        case IT_BOLT:

	    sprintf(string_buffer,
		    "%s hit you with a %.0f damage Mana Bolt!\n",
		    c->battle.opponent->name, theArg);

	    Do_energy(c, myStats->energy, c->player.max_energy,
		    c->player.shield, myStats->shield, FALSE);

	    break;

        case IT_NO_BOLT:

	    sprintf(string_buffer,
		    "%s casts a Mana Bolt that peters out!\n",
		    c->battle.opponent->name);

	    break;

        case IT_SHIELD:

	    sprintf(string_buffer, "%s throws up a Force Shield!\n",
		    c->battle.opponent->name);

	    break;

        case IT_NO_SHIELD:

	    sprintf(string_buffer, "%s messes up a Force Shield spell!\n",
		    c->battle.opponent->name);

	    break;

        case IT_TRANSFORM:

	    Do_lock_mutex(&c->realm->monster_lock);

	    sprintf(string_buffer,
		    "%s casts Transform, turning you into %s!\n",
		    c->battle.opponent->name,
		    c->realm->monster[(int)theArg].name);

	    event_ptr = (struct event_t *) Do_create_event();
	    event_ptr->type = DEATH_EVENT;
	    event_ptr->arg1 = theArg;
	    event_ptr->arg3 = K_TRANSFORMED;

	    Do_unlock_mutex(&c->realm->monster_lock);
	    Do_handle_event(c, event_ptr);

	    newMessage = IT_DEFEAT;
	    break;

        case IT_NO_TRANSFORM:

	    sprintf(string_buffer, "%s fails to cast a Transform spell!\n",
		    c->battle.opponent->name);

	    break;

        case IT_TRANSFORM_BACK:

	    sprintf(string_buffer,
		    "%s's Transform spell backfires!  %s turns into %s!\n",
		    c->battle.opponent->name, c->battle.opponent->name,
		    c->realm->monster[(int)theArg].name);

	    Do_cancel_monster(c->battle.opponent);
	    newMessage = IT_VICTORY;
	    break;

        case IT_MIGHT:

	    sprintf(string_buffer,
		    "%s buffs up with an Increase Might spell!\n",
		    c->battle.opponent->name);

	    c->battle.opponent->strength += c->player.max_strength;

	    break;

        case IT_NO_MIGHT:

	    sprintf(string_buffer,
		    "%s flounders with an Increase Might spell!\n",
		    c->battle.opponent->name);

	    break;

        case IT_HASTE:

	    sprintf(string_buffer,
		    "%s casts a Haste spell and speeds up!\n",
		    c->battle.opponent->name);

	    c->battle.opponent->strength += c->player.max_quickness;

	    break;

        case IT_NO_HASTE:

	    sprintf(string_buffer,
		    "%s casts an ineffective Haste spell!\n",
		    c->battle.opponent->name);

	    break;

        case IT_TRANSPORT:

	    sprintf(string_buffer,
		    "%s sends you away with a Transport spell!\n",
		    c->battle.opponent->name);

	    event_ptr = (struct event_t *) Do_create_event();
	    event_ptr->type = MOVE_EVENT;
	    event_ptr->arg3 = A_FAR;
	    Do_file_event(c, event_ptr);

	    newMessage = IT_DEFEAT;
	    break;

        case IT_NO_TRANSPORT:

	    sprintf(string_buffer,
		    "%s misfires a Transport spell!\n",
		    c->battle.opponent->name);

	    break;

        case IT_TRANSPORT_BACK:

	    sprintf(string_buffer,
		    "%s is transported away from a bounced Transport spell!\n",
		    c->battle.opponent->name);

	    Do_cancel_monster(c->battle.opponent);
	    newMessage = IT_VICTORY;
	    break;

        case IT_PARALYZE:

	    sprintf(string_buffer,
		    "%s casts a Paralyze spell holding you in place!\n",
		    c->battle.opponent->name);

	    break;

        case IT_NO_PARALYZE:

	    sprintf(string_buffer, "%s casts a dud Paralyze spell!\n",
		    c->battle.opponent->name);

	    break;

        case IT_PASS:

	    sprintf(string_buffer, "%s whistles a happy tune.\n",
		    c->battle.opponent->name);

	    break;

	default:

	    sprintf(error_msg,
		    "[%s] Unknown combat message %d in Do_it_combat_turn.\n",
		    c->connection_id, theMessage);

	    Do_log_error(error_msg);		

            theCombat->message = IT_ABANDON;
            Do_lock_mutex(&theCombat->theLock);
            *hisFlag = TRUE;
            Do_unlock_mutex(&theCombat->theLock);
	    kill(c->battle.opponent->processID, SIGIO);

            Do_lock_mutex(&c->realm->realm_lock);
            c->game->it_combat = NULL;
	    c->game->virtual = TRUE;
            Do_unlock_mutex(&c->realm->realm_lock);
	    c->run_level = EXIT_THREAD;
	    return;
	}

	if (strlen(string_buffer))
	    Do_send_line(c, string_buffer);

	    /* if the character has died */
        if (c->player.energy <= 0.0) {

	    newMessage = IT_DEFEAT;
	    event_ptr = (struct event_t *) Do_create_event();
	    event_ptr->type = DEATH_EVENT;
	    event_ptr->arg3 = K_IT_COMBAT;

	    event_ptr->arg4 = (void *)
		    Do_malloc(strlen(c->battle.opponent->name) + 1);

	    strcpy(event_ptr->arg4, c->battle.opponent->name);

		/* if we leave a corpse */
	    if (c->player.level < 3000 && c->player.special_type != SC_KING &&
		    (!c->battle.ring_in_use || !(c->player.ring_type ==
		    R_DLREG || c->player.ring_type == R_NAZREG))) {

		    /* create a copy of ourselves for the corpse */
	        theCombat->player_ptr = (struct player_t *)
			Do_copy_record(&c->player, FALSE);
	    }

	    Do_handle_event(c, event_ptr);
	}

	    /* if we lost, remove us from battle completely */
	    /* otherwise the winner could encounter us again before we move */
	if (newMessage == IT_DEFEAT || newMessage == IT_EVADED) {

	    Do_lock_mutex(&c->realm->realm_lock);
	    c->game->it_combat = NULL;
	    c->game->virtual = TRUE;
	    Do_unlock_mutex(&c->realm->realm_lock);
	}

	    /* send off the new message */
	theCombat->message = newMessage;
	theCombat->arg1 = newArg;
	Do_lock_mutex(&theCombat->theLock);
	*hisFlag = TRUE;
	Do_unlock_mutex(&theCombat->theLock);

	kill(c->battle.opponent->processID, SIGIO);

	if (c->player.poison > 0.0) 
	    myStats->energy = c->player.energy;

	if (newMessage == IT_DEFEAT) {

	    Do_orphan_events(c);
	    Do_more(c);
	    Do_send_clear(c);
	    return;
	}
    }
}

