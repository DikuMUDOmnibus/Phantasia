/*
 * commands.c  Phantasia routines for events
 */

#include "include.h"

extern int server_hook;

/************************************************************************
/
/ FUNCTION NAME: Do_death(struct client_t *c, struct event_t the_event)
/
/ FUNCTION: death routine
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/9/99
/
/ ARGUMENTS:
/       char *how - pointer to string describing cause of death
/
/ RETURN VALUE: none
/
/ MODULES CALLED: freerecord(), enterscore(), more(), exit(), fread(),
/       fseek(), execl(), fopen(), floor(), wmove(), drandom(), wclear(), strcmp(),
/       fwrite(), fflush(), printw(), strcpy(), fclose(), waddstr(), cleanup(),
/       fprintf(), wrefresh(), getanswer(), descrtype()
/
/ GLOBAL INPUTS: Curmonster, Wizard, Player, *stdscr, Fileloc, *Monstfp
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Kill off current player.
/       Handle rings, and multiple lives.
/       Print an appropriate message.
/       Update scoreboard, lastdead, and let other players know about
/       the demise of their comrade.
/
*************************************************************************/

Do_death(struct client_t *c, struct event_t *the_event)
{
    struct scoreboard_t sb;
    struct event_t *event_ptr;
    struct realm_object_t *object_ptr;
    struct linked_list_t *list_ptr;
    struct tag_t newTag;
    bool save_flag, corpse_flag, score_flag;	    /* do extra lives help? */
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE];
    static  char    *deathmesg[] =
        /* add more messages here, if desired */
        {
        "You have been wounded beyond repair.\n",
        "You have been disemboweled.\n",
        "You've been mashed, mauled, and spit upon.  (You're dead.)\n",
        "You died!\n",
        "You're a complete failure -- you've died!!\n",
        "You have been dealt a fatal blow!\n"
        };

    save_flag = TRUE;
    corpse_flag = FALSE;
    score_flag = TRUE;

    /* reset fatigue on death */
    c->battle.rounds = 0;
    c->battle.timeouts = 0;

    c->events = NULL;   

    switch(the_event->arg3) {

	case K_TRANSFORMED:

	    Do_lock_mutex(&c->realm->monster_lock);
	    
	    sprintf(string_buffer, "You were turned into a %s.\n",
		    c->realm->monster[(int)the_event->arg1].name);

	    Do_unlock_mutex(&c->realm->monster_lock);

	    Do_send_line(c, string_buffer);

	    sprintf(sb.how_died, "was turned into a %s",
		    c->realm->monster[(int)the_event->arg1].name);

	    break;

	case K_OLD_AGE:

            Do_send_line(c, "Your character dies of old age.\n");

	    strcpy(sb.how_died, "died of old age");
	    save_flag = FALSE;
	    break;

	case K_MONSTER:

            if (!strcmp(the_event->arg4, c->realm->monster[40].name)) {
	        sprintf(string_buffer, "%s turned you into a Smurf!\n",
		    the_event->arg4);

	        Do_send_line(c, string_buffer);

                Do_send_line(c, "How smurfy!\n");

                /* name smurf after player */
	        Do_lock_mutex(&c->realm->monster_lock);
                sprintf(string_buffer, "%s Smurf", c->modifiedName);
	        strcpy(c->realm->monster[40].name, string_buffer);
	        Do_unlock_mutex(&c->realm->monster_lock);

	        sprintf(sb.how_died, "was smurfed by %s",
	  	    the_event->arg4);
            } else if (!strcmp(the_event->arg4, "Morgoth")) {
                sprintf(string_buffer, "Morgoth roars in triumph as he bashes %s's skull in with Grond, Hammer of the Underworld!\n", c->modifiedName);
                Do_broadcast(c, string_buffer);

	        sprintf(sb.how_died, "was killed by %s",
		    the_event->arg4);
            } else {
	        sprintf(string_buffer, "You were killed by %s.\n",
		    the_event->arg4);

	        Do_send_line(c, string_buffer);

	        sprintf(sb.how_died, "was killed by %s",
		    the_event->arg4);

            }



	    free(the_event->arg4);
            c->battle.ring_in_use = (bool) the_event->arg1;
	    corpse_flag = TRUE;

	    break;

	case K_FATIGUE:

	    sprintf(string_buffer,
		    "Because of your fatigue, %s found you easy prey.\n",
		    the_event->arg4);

	    Do_send_line(c, string_buffer);

	    sprintf(sb.how_died, "was killed by fatigue");
	    free(the_event->arg4);
            c->battle.ring_in_use = (bool) the_event->arg1;
	    corpse_flag = TRUE;

	    break;

	case K_GREED:

	    sprintf(string_buffer,
		    "Because of your greed, %s found you easy prey.\n",
		    the_event->arg4);

	    Do_send_line(c, string_buffer);

	    sprintf(sb.how_died, "was killed by greed");
	    free(the_event->arg4);
            c->battle.ring_in_use = (bool) the_event->arg1;
	    corpse_flag = TRUE;

	    break;

	case K_IT_COMBAT:

	    Do_lock_mutex(&c->realm->realm_lock);

	    sprintf(string_buffer, "You died in glorious battle against %s.\n",
		    the_event->arg4);

	    Do_send_line(c, string_buffer);

	    sprintf(sb.how_died, "died in glorious battle against %s",
		    the_event->arg4);

	    Do_unlock_mutex(&c->realm->realm_lock);
	    free((void *)the_event->arg4);
	    break;

	case K_GHOSTBUSTERS:

	    strcpy(sb.how_died, "vanished from the internet");
	    save_flag = FALSE;
	    break;

	case K_SEGMENTATION:

	    strcpy(sb.how_died, "fell into a segmentation fault");
	    save_flag = FALSE;
	    break;

	case K_VAPORIZED:

	    Do_lock_mutex(&c->realm->realm_lock);

	    sprintf(string_buffer, "You were vaporized by %s!\n",
		    the_event->arg4);

	    Do_send_line(c, string_buffer);
	    Do_send_line(c, "Next time, try a disintegration proof vest.\n");

	    sprintf(sb.how_died, "was vaporized by %s", the_event->arg4);

	    Do_unlock_mutex(&c->realm->realm_lock);
	    free((void *)the_event->arg4);

	    score_flag = FALSE;
	    save_flag = FALSE;
	    break;

	case K_RING:

            Do_send_line(c,
      "Your ring has taken control of you and turned you into a monster!\n");

                /* bad ring in possession; name idiot after player */
	    Do_lock_mutex(&c->realm->monster_lock);
            sprintf(string_buffer, "%s the Idiot", c->modifiedName);
	    strcpy(c->realm->monster[14].name, string_buffer);
	    Do_unlock_mutex(&c->realm->monster_lock);

	    strcpy(sb.how_died, "was consumed by a cursed ring");
	    save_flag = FALSE;
	    break;
	    
	case K_NO_ENERGY:

            Do_send_line(c, "You died from your earlier wounds.\n");

	    strcpy(sb.how_died,
		    "died from massive internal bleeding");

	    corpse_flag = TRUE;
	    break;

	case K_FELL_OFF:

            Do_send_line(c,
	     "You stepped over the edge of the realm and died in the void.\n");

	    strcpy(sb.how_died,
		    "leisurely stepped over the edge of the realm");

	    save_flag = FALSE;
	    break;

	case K_SUICIDE:

            Do_send_line(c, "At the behest of the game administrators, you thrust your own weapon through your heart.\n");

	    strcpy(sb.how_died, "committed suicide");

	    save_flag = FALSE;
	    corpse_flag = FALSE;
	    score_flag = FALSE;

	        /* ban the character for 5-30 minutes to prevent him from 
                   coming back so quickly */
            newTag.type = T_BAN;
            newTag.validUntil = time(NULL) + 300 + RND() * 1500;
            newTag.affectNetwork = FALSE;
            strcpy(newTag.description, "suicide tag");

	        /* send it */
            Do_tag_self(c, &newTag);
	    break;

	case K_SQUISH:

            Do_send_line(c, "You come across an old bridge keeper who asks you your name, quest, and favorite color...\n");

			Do_more(c);
		    Do_send_clear(c);
		    
		    Do_send_line(c, "Unfortuantely, in a momentary lapse of concentration, you say Blue instead of Green and are hurled into the gorge of eternal peril!\n");
			Do_more(c);
			Do_send_clear(c);

	    sprintf(sb.how_died,
		    "was hurled off of the bridge of death into the gorge of eternal peril",
		    the_event->arg4);

	    save_flag = FALSE;
	    corpse_flag = TRUE;
	    score_flag = FALSE;
	    break;

	case K_SIN:

            if (c->player.level >= MIN_KING && 
                c->player.special_type != SC_EXVALAR) {
                Do_send_line(c,
       "Because of your great evil, you have become the new Dark Lord!\n");

                   /* name dark lord after player */
	        Do_lock_mutex(&c->realm->monster_lock);
                if (c->player.gender == MALE) {
                   sprintf(string_buffer, "The Dark Lord %s", c->modifiedName);
                } else {
                   sprintf(string_buffer, "The Dark Lady %s", c->modifiedName);
                }
	        strcpy(c->realm->monster[99].name, string_buffer);
	        Do_unlock_mutex(&c->realm->monster_lock);

	        strcpy(sb.how_died, "succumbed to the lure of the Dark Side");

                   /* don't put dark lords on the scoreboard */
                score_flag = FALSE;

                c->player.lives = 0;
                if (c->player.special_type == SC_VALAR) {
                    Do_send_self_event(c, VALAR_EVENT);
                }
                    
                  
            } else {
                Do_send_line(c,
       "Your pathetic attempts to abuse others has turned you into a Smurf!  Lah lah la lah la la!\n");

                   /* name smurf after player */
	        Do_lock_mutex(&c->realm->monster_lock);
                if (c->player.gender == MALE) {
                    sprintf(string_buffer, "Papa Smurf %s", c->modifiedName);
                } else {
                    sprintf(string_buffer, "Mama Smurf %s", c->modifiedName);
                }
	        strcpy(sb.how_died, "was transformed into a Smurf");
	        strcpy(c->realm->monster[40].name, string_buffer);
	        Do_unlock_mutex(&c->realm->monster_lock);

            }


	    save_flag = FALSE;
	    corpse_flag = FALSE;
	    break;

	default:

	    Do_send_line(c, deathmesg[(int) ROLL(0.0,
		    (double) sizeof(deathmesg) / sizeof(char *))]);

	    strcpy(sb.how_died, "died in a bizarre gardening accident");
	    break;
    }

    if (c->player.type == C_EXPER) {
        corpse_flag = FALSE;
        Do_send_line(c, "A mist rises from your body . . .\n");
        Do_send_buffer(c);
        sleep(10);
        if (RND() < .05) {
           Do_send_line(c, "and it reforms into your old body!\n");
           if (c->player.lives == 0) 
               c->player.lives++;
        } else {
           Do_send_line(c, "But it disperses!\n");
        }
    }


	/* check if the person can be saved from this death */
    if (save_flag) {

	    /* if the player has a good ring in use */
	if (c->battle.ring_in_use && (c->player.ring_type == R_DLREG ||
		c->player.ring_type == R_NAZREG)) {

            Do_send_line(c, "But your ring saved you from death!\n");
	    Do_more(c);
	    Do_send_clear(c);
	    Do_ring(c, R_NONE, FALSE);

	    Do_energy(c, c->player.max_energy / 12.0 + 1.0,
		    c->player.max_energy, c->player.shield, 0, FALSE);

            return;
        }

             /* extra lives */
	else if (c->player.lives > 0) {

	    Do_more(c);
	    Do_send_clear(c);

            Do_send_line(c,
		    "You should be more cautious.  You've been killed.\n");

	    sprintf(string_buffer, "You only have %d more chance(s).\n",
		    --c->player.lives);

	    Do_send_line(c, string_buffer);
	    Do_more(c);
	    Do_send_clear(c);

	    Do_energy(c, c->player.max_energy + c->player.shield,
		    c->player.max_energy, c->player.shield, 0, FALSE);

            return;
	}

	    /* a king who is killed only loses his post */
	else if (c->player.special_type == SC_KING) {

	    Do_more(c);
	    Do_send_clear(c);

            Do_send_line(c, "The head page rushes to your aid.  'Rest, your Majesty.', he says, 'You have served your people long enough'.\n");

	    Do_more(c);
	    Do_send_clear(c);

	    Do_energy(c, c->player.max_energy / 5.0, c->player.max_energy,
		    c->player.shield, 0, FALSE);

	    Do_send_self_event(c, DETHRONE_EVENT);

            Do_lock_mutex(&c->realm->realm_lock);
            c->realm->king_name[0] = '\0';
            c->realm->king = NULL;
            c->realm->king_flag = FALSE;
            Do_unlock_mutex(&c->realm->realm_lock);

            c->player.special_type = SC_NONE;

            return;
	}

	    /* if a valar lost all his lives */
	else if (c->player.special_type == SC_VALAR) {

	    Do_more(c);
	    Do_send_clear(c);

            Do_send_line(c, "You had your chances, but Valar aren't totally immortal.  You are now an ex-valar! . . .\n");

	    Do_more(c);
	    Do_send_clear(c);
	    	
	    		/* heal the player completely and remove fatigue */
		    Do_energy(c, c->player.max_energy + c->player.shield,
			    c->player.max_energy, c->player.shield, 0, FALSE);

                    c->battle.rounds = 0;
		    Do_speed(c, c->player.max_quickness, c->player.quicksilver, 0, FALSE);
                

/*
	    Do_energy(c, c->player.max_energy / 5.0, c->player.max_energy,
		    c->player.shield, 0, TRUE);

	    Do_strength(c, c->player.max_strength, 0, 0, FALSE);
	    Do_speed(c, c->player.max_quickness, 0, 0, FALSE);
            c->player.brains = c->player.level / 25.0;
*/

            Do_send_self_event(c, VALAR_EVENT);
            return;
        }
             
    }

	/* chat no more */
    Do_send_int(c, DEACTIVATE_CHAT_PACKET);

    Do_more(c);
    Do_send_clear(c);

	/* create a corpse, if necessary */
    if (corpse_flag && c->player.level < 3000) {
	object_ptr = (struct realm_object_t *) Do_malloc(SZ_REALM_OBJECT);
	object_ptr->type = CORPSE;
	object_ptr->x = c->player.x;
	object_ptr->y = c->player.y;
	object_ptr->arg1 = (void *) Do_copy_record(&c->player, FALSE);

	    /* put the new event in the realm object list */
	Do_lock_mutex(&c->realm->realm_lock);
        object_ptr->next_object = c->realm->objects;
        c->realm->objects = object_ptr;
	Do_unlock_mutex(&c->realm->realm_lock);
    }

        /* if the character is special, de-throne him */
    if (c->player.special_type == SC_KING || c->player.special_type ==
	    SC_STEWARD || c->player.special_type == SC_KNIGHT) {

        Do_dethrone(c);
    }


	/* put the character on the scoreboard if: */
	/* the character is between 50 and not an ex-valar/wizard */

    if (score_flag && (c->wizard == 0 || c->wizard == 2) && 
        c->player.level >= 100 && 
        !c->player.beyond &&
        c->player.special_type < SC_EXVALAR) {

        strcpy(sb.class, c->realm->charstats[c->player.type].class_name);
        strcpy(sb.name, c->modifiedName);
        strcpy(sb.from, c->network);
        sb.level = c->player.level;
        sb.time = time(NULL);

	Do_scoreboard_add(c, &sb, TRUE);
    }

	/* save the character name */
    strcpy(c->previousName, c->player.lcname);
    list_ptr = (struct linked_list_t *) Do_malloc(SZ_LINKED_LIST);
    strcpy(list_ptr->name, c->player.lcname);

    Do_lock_mutex(&c->realm->character_file_lock);
    list_ptr->next = c->realm->name_limbo;
    c->realm->name_limbo = list_ptr;
    Do_unlock_mutex(&c->realm->character_file_lock);

	/* broadcast the death */
    sprintf(string_buffer, "%s %s.\n", c->modifiedName, sb.how_died);
    Do_broadcast(c, string_buffer);

	/* log the death */
    sprintf(string_buffer, "[%s] %s died\n", c->connection_id,
	    c->player.lcname);

    Do_log(GAME_LOG, &string_buffer);

    sprintf(string_buffer, "%s, %s, %d age, %d seconds, dead\n",
	    c->player.lcname, c->realm->charstats[c->player.type].class_name,
	    c->player.age, c->player.time_played + time(NULL) -
	    c->player.last_load);

    Do_log(LEVEL_LOG, &string_buffer);

    c->run_level = GO_AGAIN;

}


/************************************************************************
/
/ FUNCTION NAME: Do_valar(struct client_t *c)
/
/ FUNCTION: toggle the player's valar status
/
/ AUTHOR: E. A. Estes, 12/4/85
/ 	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_valar(struct client_t *c) 
{
    struct event_t *event_ptr;
    struct scoreboard_t sb;
    char string_buffer[SZ_LINE];
    float ftemp;

	/* if the character is already valar */
    if (c->player.special_type == SC_VALAR) {

        Do_lock_mutex(&c->realm->realm_lock);

	if (c->realm->valar == c->game) {
	    c->realm->valar = NULL;
	    c->realm->valar_name[0] = '\0';
        }
        Do_unlock_mutex(&c->realm->realm_lock);

        Do_send_line(c, "You are no longer a Valar!\n");
        Do_more(c);
	Do_send_clear(c);
        c->player.special_type = SC_EXVALAR;
	c->broadcast = FALSE;
    }
    else {
        Do_send_line(c, "You have ascended to the position of Valar!\n");
        Do_more(c);
	Do_send_clear(c);

        Do_lock_mutex(&c->realm->realm_lock);

            /* remove the current valar if there is one */
        if ((c->realm->valar != NULL) && (c->realm->valar != c->game)) {
            event_ptr = (struct event_t *) Do_create_event();
            event_ptr->type = DETHRONE_EVENT;
	    event_ptr->to = c->realm->valar;
	    event_ptr->from = c->game;
	    Do_send_event(event_ptr);
        }
    
        c->realm->valar = c->game;
	strcpy(c->realm->valar_name, c->modifiedName);
        Do_unlock_mutex(&c->realm->realm_lock);

	if (c->player.special_type == SC_COUNCIL) {
            strcpy(sb.class, c->realm->charstats[c->player.type].class_name);
            strcpy(sb.name, c->modifiedName);
            strcpy(sb.from, c->network);
            sb.level = 9999;
            sb.time = time(NULL);

	    Do_scoreboard_add(c, &sb, FALSE);	
            c->player.lives = 3;
        } else if (c->player.lives < 3) {
            c->player.lives = 3;  
        }
       

        c->player.special_type = SC_VALAR;


	    /* send a chat message notifing everyone */
        sprintf(string_buffer, "Behold the new Valar %s!\n",
		c->modifiedName);

	Do_broadcast(c, string_buffer);
    }

	/* modify player description */
    Do_lock_mutex(&c->realm->realm_lock);
    Do_player_description(c);
    Do_unlock_mutex(&c->realm->realm_lock);

	/* send everyone new specs */
    Do_send_specification(c, CHANGE_PLAYER_EVENT);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_knight(struct client_t *c)
/
/ FUNCTION: make the player a knight
/
/ AUTHOR: E. A. Estes, 12/4/85
/ 	  Brian Kelly, 5/10/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_knight(struct client_t *c, struct event_t *the_event) 
{
    struct event_t *event_ptr;
    char string_buffer[SZ_LINE];

	/* don't do anything if we're already special */
    if (c->player.special_type) {
	return;
    }

	/* if there is currently another knight, dethrone him */
    Do_lock_mutex(&c->realm->realm_lock);
    if (c->realm->knight != NULL) {
        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = DETHRONE_EVENT;
	event_ptr->to = c->realm->knight;
	event_ptr->from = c->game;
	Do_send_event(event_ptr);
    }

    c->realm->knight = c->game;
    Do_unlock_mutex(&c->realm->realm_lock);
    c->player.special_type = SC_KNIGHT;

    sprintf(string_buffer, "You have been knighted by %s!\n", the_event->arg4);
    Do_send_line(c, string_buffer);
    free((void *)the_event->arg4);
    Do_more(c);
    Do_send_clear(c);

    c->knightEnergy = floor(c->player.max_energy / 4);
    c->knightQuickness = floor(c->player.max_quickness / 10);

    Do_energy(c, c->player.energy + c->knightEnergy, c->player.max_energy,
	    c->player.shield, 0, FALSE);

    Do_speed(c, c->player.max_quickness, c->player.quicksilver, 0 , FALSE);

	/* send everyone new specs */
    Do_send_specification(c, CHANGE_PLAYER_EVENT);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_steward(struct client_t *c)
/
/ FUNCTION: make the player a steward
/
/ AUTHOR: E. A. Estes, 12/4/85
/ 	  Brian Kelly, 5/9/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_steward(struct client_t *c) 
{
    struct event_t *event_ptr;
    struct realm_object_t *object_ptr, **object_ptr_ptr;
    char string_buffer[SZ_LINE];

    if (c->player.special_type == SC_KNIGHT) {
        Do_dethrone(c);
    }

	/* if there is currently a steward, dethrone him */
    Do_lock_mutex(&c->realm->realm_lock);
    if (c->realm->king != NULL) {
        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = DETHRONE_EVENT;
	event_ptr->to = c->realm->king;
	event_ptr->from = c->game;
	Do_send_event(event_ptr);
    }

	/* erase all energy voids */
    Do_lock_mutex(&c->realm->object_lock);
    object_ptr_ptr = &c->realm->objects;
    object_ptr = *object_ptr_ptr;
    while (object_ptr != NULL) {
	if (object_ptr->type == ENERGY_VOID) {
	    *object_ptr_ptr = object_ptr->next_object;
	    free((void *)object_ptr);
	}
	else {
	    object_ptr_ptr = &object_ptr->next_object;
	}
	object_ptr = *object_ptr_ptr;
    }
    Do_unlock_mutex(&c->realm->object_lock);

    c->realm->king = c->game;
    c->realm->king_flag = FALSE;
    Do_unlock_mutex(&c->realm->realm_lock);
    c->player.special_type = SC_STEWARD;

    Do_send_line(c, "You have become the steward!\n");
    Do_more(c);

    Do_crowns(c, (-1), FALSE);

	/* send a chat message notifing everyone */
    sprintf(string_buffer, "%s has become the new steward.\n",
		c->modifiedName);

    Do_broadcast(c, string_buffer);

	/* modify player description */
    Do_lock_mutex(&c->realm->realm_lock);
    Do_player_description(c);
    Do_unlock_mutex(&c->realm->realm_lock);
	/* send everyone new specs */
    Do_send_specification(c, CHANGE_PLAYER_EVENT);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_king(struct client_t *c)
/
/ FUNCTION: do king stuff
/
/ AUTHOR: E. A. Estes, 12/4/85
/ 	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_king(struct client_t *c) 
{
    struct event_t *event_ptr;
    char string_buffer[SZ_LINE];

    if (c->player.special_type == SC_KNIGHT) {
        Do_dethrone(c);
    }

	/* if there is currently a king or steward, dethrone him */
    Do_lock_mutex(&c->realm->realm_lock);
    if (c->realm->king != NULL) {
        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = DETHRONE_EVENT;
	event_ptr->to = c->realm->king;
	event_ptr->from = c->game;
	Do_send_event(event_ptr);
    }

    c->realm->king = c->game;
    c->realm->king_flag = TRUE;
    strcpy(c->realm->king_name, c->modifiedName);
    Do_unlock_mutex(&c->realm->realm_lock);
    c->player.special_type = SC_KING;

    if (c->player.gender == MALE) 
        Do_send_line(c, "You have become king!\n");
    else
        Do_send_line(c, "You have become queen!\n");

    Do_more(c);
    Do_crowns(c, (-1), FALSE);

	/* send a chat message notifing everyone */
    if (c->player.gender == MALE) {

        sprintf(string_buffer, "Long live the king.  Hail King %s!\n",
		c->modifiedName);
    }
    else {

        sprintf(string_buffer, "Long live the queen.  Hail Queen %s!\n",
		c->modifiedName);
    }

    Do_broadcast(c, string_buffer);

	/* modify player description */
    Do_lock_mutex(&c->realm->realm_lock);
    Do_player_description(c);
    Do_unlock_mutex(&c->realm->realm_lock);
	/* send everyone new specs */
    Do_send_specification(c, CHANGE_PLAYER_EVENT);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_dethrone(struct client_t *c)
/
/ FUNCTION: remove the player from the throne
/
/ AUTHOR: Brian Kelly, 7/06/00
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_dethrone(struct client_t *c) 
{
    struct event_t *event_ptr;
    char string_buffer[SZ_LINE];

	/* king or steward handled differently from knight */
    if (c->player.special_type == SC_KING || c->player.special_type ==
	    SC_STEWARD) {

	    /* if the player is on the throne, kick him off */
	if (c->player.location == PL_THRONE) {

		/* if the player can not leave the chamber now, return */
	    if (c->stuck) {

		Do_send_line(c,
	      "You are interrupted by another player entering the chamber.\n");

		Do_more(c);
		Do_send_clear(c);
		return;
	    }

            event_ptr = (struct event_t *) Do_create_event();
            event_ptr->type = MOVE_EVENT;
            event_ptr->arg3 = A_NEAR;
		/* we must handle it now in case we're saving the game */
            Do_handle_event(c, event_ptr);

	    Do_send_line(c,
		    "Your pages say, 'Your Majesty, the throne is no place to sleep!'\n");
	}

	if (c->player.special_type == SC_KING) {

            Do_send_line(c, "You vacate the throne.\n");
            sprintf(string_buffer, "%s has vacated the throne!\n",
		c->modifiedName);

            Do_lock_mutex(&c->realm->realm_lock);
	    c->realm->king_flag = FALSE;

                /* check if player should get it back later*/
            if ((c->realm->king != c->game) || c->player.level >= MAX_KING) {
                c->player.special_type = SC_NONE;
            } else {
	        c->realm->king = NULL;
            }
            Do_unlock_mutex(&c->realm->realm_lock);

	} else if (c->player.special_type == SC_STEWARD) {

            Do_send_line(c, "You are no longer steward!\n");
            sprintf(string_buffer, "%s is no longer steward!\n",
		c->modifiedName);

		/* dump the steward coffers into the king's coffers */

	    Do_lock_mutex(&c->realm->kings_gold_lock);
	    c->realm->kings_gold += c->realm->steward_gold;
	    c->realm->steward_gold = 0;
	    Do_unlock_mutex(&c->realm->kings_gold_lock);

            c->player.special_type = SC_NONE;

            Do_lock_mutex(&c->realm->realm_lock);
            if (c->realm->king == c->game) {
	        c->realm->king = NULL;
            }
            Do_unlock_mutex(&c->realm->realm_lock);
        }

	Do_more(c);

	/* send a chat message notifing everyone of the dethrone */
        Do_broadcast(c, string_buffer);

	if (c->player.special_type == SC_KING && c->realm->knight != NULL) {
            event_ptr = (struct event_t *) Do_create_event();
            event_ptr->type = DETHRONE_EVENT;
            event_ptr->to = c->realm->knight;
            Do_send_event(event_ptr);
	}

        Do_lock_mutex(&c->realm->realm_lock);

	    /* modify player description */
        Do_player_description(c);

        Do_unlock_mutex(&c->realm->realm_lock);

	    /* send everyone new specs */
        Do_send_specification(c, CHANGE_PLAYER_EVENT);
    }
    else if (c->player.special_type == SC_KNIGHT) {

        Do_lock_mutex(&c->realm->realm_lock);
	if (c->realm->knight == c->game) {
	    c->realm->knight = NULL;
	}

	    /* modify player description */
        Do_unlock_mutex(&c->realm->realm_lock);

        Do_send_line(c, "You are no longer a knight!\n");

	Do_more(c);
        c->player.special_type = SC_NONE;
	c->knightEnergy = 0;
	c->knightQuickness = 0;

	    /* modify player description */
        Do_lock_mutex(&c->realm->realm_lock);
        Do_player_description(c);
        Do_unlock_mutex(&c->realm->realm_lock);

	Do_energy(c, c->player.energy, c->player.max_energy, c->player.shield,
		0, TRUE);

	Do_speed(c, c->player.max_quickness, c->player.quicksilver, 0 , FALSE);

	    /* send everyone new specs */
        Do_send_specification(c, CHANGE_PLAYER_EVENT);
    }

    Do_send_clear(c);
    return;
}



/************************************************************************
/
/ FUNCTION NAME: Do_degenerate(struct client_t *c)
/
/ FUNCTION: degenerate the player
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_degenerate(struct client_t *c, int counts)
{
    struct event_t *event_ptr;

        /* age player slightly */
    if (counts) {
        ++c->player.degenerated;
    }

	/* degenerate speed and quicksilver */
    Do_speed(c, c->player.max_quickness * 0.975,
	    c->player.quicksilver * 0.93, c->battle.speedSpell, FALSE);

    Do_energy(c, c->player.energy, c->player.max_energy * 0.97,
	    c->player.shield * 0.93, c->battle.force_field, FALSE);

    Do_strength(c, c->player.max_strength * 0.97,
	    c->player.sword * 0.93, c->battle.strengthSpell, FALSE);

    c->player.brains *= 0.95;
    c->player.magiclvl *= 0.97;
}


/************************************************************************
/
/ FUNCTION NAME: Do_caused_degenerate(struct client_t *c, event_t *the_event)
/
/ FUNCTION: do degenerate power
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 11/10/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_caused_degenerate(struct client_t *c, struct event_t *the_event)
{
    char string_buffer[SZ_LINE];

    sprintf(string_buffer, "%s causes you and your equipment to degenerate!\n",
	    the_event->arg4);

    Do_send_line(c, string_buffer);
    free((void *)the_event->arg4);
    Do_more(c);
    Do_send_clear(c);

    Do_degenerate(c, FALSE);
}


/************************************************************************
/
/ FUNCTION NAME: Do_blind(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: toggle blindness
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_blind(struct client_t *c, struct event_t *the_event)
{
    char string_buffer[SZ_LINE];

	/* if the character is blind */
    if (! c->player.blind) {
        sprintf(string_buffer,
	    "You hear %s laugh as your sight dims and the world goes black!\n",
	    the_event->arg4);

	free((void *)the_event->arg4);
        Do_send_line(c, string_buffer);
        Do_more(c);
	Do_send_clear(c);
        c->player.blind = TRUE;
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_relocate(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: do relocation command
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_relocate(struct client_t *c, struct event_t *the_event)
{
    struct event_t *event_ptr;
    char string_buffer[SZ_LINE];

    if (fabs(c->player.x) >= D_BEYOND || fabs(c->player.y) >= D_BEYOND) {
        free((void *)the_event->arg4);
	return;
    }

    Do_lock_mutex(&c->realm->realm_lock);
    sprintf(string_buffer, "You have been relocated by %s!\n",
	    the_event->arg4);
    Do_unlock_mutex(&c->realm->realm_lock);
    Do_send_line(c, string_buffer);
    free((void *)the_event->arg4);

    Do_more(c);
    Do_send_clear(c);

    event_ptr = (struct event_t *) Do_create_event();
    event_ptr->type = MOVE_EVENT;
    event_ptr->arg1 = the_event->arg1;
    event_ptr->arg2 = the_event->arg2;
    event_ptr->arg3 = A_TELEPORT;
    Do_handle_event(c, event_ptr);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_transport(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: do transport and oust command
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_transport(struct client_t *c, struct event_t *the_event)
{
    struct event_t *event_ptr;
    char string_buffer[SZ_LINE];

    if (the_event->arg1) {

	sprintf(string_buffer, "%s ousts you from this location!\n",
	        the_event->arg4);
    }
    else {

	sprintf(string_buffer, "You have been transported by %s!\n",
	        the_event->arg4);
    }

    Do_send_line(c, string_buffer);
    free((void *)the_event->arg4);

    if (!the_event->arg1 && c->player.charms > 0) {
	Do_send_line(c, "But your charm saved you. . .\n");
        Do_more(c);
	Do_send_clear(c);
	return;
    }

    Do_more(c);
    Do_send_clear(c);

    event_ptr = (struct event_t *) Do_create_event();
    event_ptr->type = MOVE_EVENT;

	/* is this the more powerful version? */ 
    if (the_event->arg1) {
	event_ptr->arg3 = A_OUST;
    }
    else {
        event_ptr->arg3 = A_TRANSPORT;
    }

    Do_handle_event(c, event_ptr);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_curse(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: do curse, smite, and execrate
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_curse(struct client_t *c, struct event_t *the_event)
{
    char string_buffer[SZ_LINE];
    double dtemp;

    if (the_event->arg1 == P_SMITE) {

        sprintf(string_buffer,
		"%s smites you down for your insolence!\n", 
	        the_event->arg4);

    }
    else if (the_event->arg1 == P_EXECRATE) {

        sprintf(string_buffer, "You've been execrated by %s!\n", 
	        the_event->arg4);
    }
    else {

        sprintf(string_buffer, "%s lays a heavy curse on you!\n", 
	        the_event->arg4);
    }

    Do_send_line(c, string_buffer);

    if (the_event->arg1 == P_CURSE && c->player.blessing) {
	Do_send_line(c, "But your blessing saved you. . .\n");
        Do_more(c);
	Do_send_clear(c);

	return;
    }

    Do_more(c);
    Do_send_clear(c);

    if (the_event->arg1 == P_SMITE) {

        Do_energy(c, 1.0, c->player.max_energy * 0.95, c->player.shield,
		    0, FALSE);

            /* no longer cloaked, blessed or sighted */
        Do_cloak(c, FALSE, FALSE);
        Do_blessing(c, FALSE, FALSE);
	c->player.blind = TRUE;
	dtemp = 5.0;

        if (c->player.gender == MALE) {
            sprintf(string_buffer, "%s smites %s for his insolence!\n", the_event->arg4, c->player.name);
        } else {
            sprintf(string_buffer, "%s smites %s for her insolence!\n", the_event->arg4, c->player.name);
        }

        Do_broadcast(c, string_buffer);
    }
    else if (the_event->arg1 == P_EXECRATE) {

	dtemp = floor(c->player.energy / 10.0);

	if (dtemp < 1.0) {
	    dtemp = 1.0;
	}

        Do_energy(c, dtemp, c->player.max_energy * 0.98, c->player.shield,
	        0, FALSE);

        sprintf(string_buffer, "%s pronounces an execration on %s!\n", the_event->arg4, c->player.name);

            /* no longer cloaked */
        Do_cloak(c, FALSE, FALSE);
	dtemp = 2.0;
        Do_broadcast(c, string_buffer);
    }
    else {

	dtemp = floor(4.0 * c->player.energy / 5.0);

	if (dtemp < 1.0) {
	    dtemp = 1.0;
	}

	Do_energy(c, dtemp, c->player.max_energy, c->player.shield, 0, FALSE);
	dtemp = 0.5;

        sprintf(string_buffer, "%s has received the curse of %s!\n", c->player.name, the_event->arg4);
        Do_broadcast(c, string_buffer);
    }

    Do_poison(c, dtemp);
    free((void *)the_event->arg4);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_bestow(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: do bestowing
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_bestow(struct client_t *c, struct event_t *the_event)
{
    char string_buffer[SZ_LINE];
    double dtemp;

      /* can't bestow to a cloaked player */
    if (c->player.cloaked) {
	sprintf(string_buffer, "You see %s's head page walk by.  He seems to be looking for something.\n", the_event->arg4);

        free((void *)the_event->arg4);
	Do_send_line(c, string_buffer);
	Do_more(c);
	Do_send_clear(c);
        return;
    }

    if (fabs(c->player.x) == fabs(c->player.y) && c->player.location !=
                PL_THRONE) {

        dtemp = sqrt(fabs(c->player.x)/100.0);

        if (floor(dtemp) == dtemp) {
            sprintf(string_buffer, "You see %s's head page approach.  The post owner leaves the shop, says \"Oh great!  My tax refund is finally here!\", grabs the gold from the page and disappears back into the post.\n", the_event->arg4);
            Do_send_line(c, string_buffer);

            free((void *)the_event->arg4);
	    Do_more(c);
	    Do_send_clear(c);
	    return;
        }
    }

    sprintf(string_buffer, "%s's head page brings you %.0f gold pieces!\n", 
	    the_event->arg4, the_event->arg1 * N_GEMVALUE);

    Do_send_line(c, string_buffer);

    sprintf(string_buffer, "%s bestowed %lf gems to %s.\n",
                           the_event->arg4,
                           the_event->arg1,
        		   c->player.lcname);
    Do_log(PURCHASE_LOG, &string_buffer);

    free((void *)the_event->arg4);

    Do_more(c);
    Do_send_clear(c);

    if (the_event->arg1 > 0) {
        c->player.gold += the_event->arg1 * N_GEMVALUE;
        Do_gold(c, 0, TRUE);
        Do_check_weight(c);
    }


      /* bestowing may attract attention from monsters */
    Do_random_events(c);
    Do_random_events(c);


    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_summon(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: do summoning of monsters
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_summon(struct client_t *c, struct event_t *the_event)
{
    struct event_t *event_ptr;
    char string_buffer[SZ_LINE];

    if (c->player.cloaked) {
        sprintf(string_buffer, "%s hurls %s at you, but it goes whizzing by your cloaked head!\n", 
            the_event->arg4, 
            c->realm->monster[(long)the_event->arg1].name);
    } else if (c->player.special_type > SC_KING) {
        sprintf(string_buffer, "%s hurls %s at you, but you contemptuously flick it aside!\n", 
            the_event->arg4, 
            c->realm->monster[(long)the_event->arg1].name);
    } else {
        sprintf(string_buffer, "%s hurls %s at you!\n", 
	    the_event->arg4,
            c->realm->monster[(long)the_event->arg1].name);
    }

    Do_send_line(c, string_buffer);
    free((void *)the_event->arg4);

    Do_more(c);
    Do_send_clear(c);

    if (c->player.cloaked || c->player.special_type > SC_KING) {
        return; 
    }

    event_ptr = (struct event_t *) Do_create_event();
    event_ptr->type = MONSTER_EVENT;
    event_ptr->arg1 = MONSTER_SUMMONED;
    event_ptr->arg3 = (long) the_event->arg1;

    Do_handle_event(c, event_ptr);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_slap(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: slapping
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/22/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_slap(struct client_t *c, struct event_t *the_event)
{
    char string_buffer[SZ_LINE];
    struct event_t *event_ptr;
    short newChannel;
    double dtemp;

    if (c->wizard >= 2 || c->player.special_type >= SC_COUNCIL || 
        c->channel == 8) {
        sprintf(string_buffer, "%s tried to slap you, but failed!\n", the_event->arg4);

        Do_send_line(c, string_buffer);

        free((void *)the_event->arg4);

        Do_more(c);
        Do_send_clear(c);

        return;
    }

    newChannel = c->channel;

	/* find a new channel to send the player */
    while (newChannel == c->channel) {
       newChannel = floor(RND() * 6 + 1);
    }

	/* knock the player into another channel */
    c->channel = newChannel;
    Do_lock_mutex(&c->realm->realm_lock);
    Do_player_description(c);
    Do_unlock_mutex(&c->realm->realm_lock);
    Do_send_specification(c, CHANGE_PLAYER_EVENT);

	/* move the player */
    event_ptr = (struct event_t *) Do_create_event();
    event_ptr->type = MOVE_EVENT;
    event_ptr->arg3 = A_NEAR;
    Do_handle_event(c, event_ptr);

    sprintf(string_buffer,
	    "You are firmly slapped by %s and are left disoriented!\n",
	    the_event->arg4);

    Do_send_line(c, string_buffer);

    sprintf(string_buffer, "%s reels from a firm, harsh slap from %s!\n", 
	    c->player.name, the_event->arg4);

    Do_broadcast(c, string_buffer);

    free((void *)the_event->arg4);
    Do_more(c);
    Do_send_clear(c);

	/* injure the player slightly */	
    dtemp = 9.0 * c->player.energy / 10.0;
    if (dtemp < 1.0) {
	dtemp = 1.0;
    }

    Do_energy(c, dtemp, c->player.max_energy, c->player.shield, 0, FALSE);

	/* 10% of the time the player is knocked out of the game */
    if (RND() < .1) {
        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = SAVE_EVENT;
        Do_handle_event(c, event_ptr);
    }

    return;
}

/************************************************************************
/
/ FUNCTION NAME: Do_reprimand(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: apprentice reprimand
/
/ AUTHOR: Arella Kirstar, 6/2/02
	  Renee Gehlbach, 7/31/02
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION: Throws a player to another channel, setting off a server announcement.
/
*************************************************************************/

Do_reprimand(struct client_t *c, struct event_t *the_event)
{
	char string_buffer[SZ_LINE], string_buffer2[SZ_LINE];
	short newChannel = c->channel;

	/* find another random channel */
	while(newChannel == c->channel)
	{
		newChannel = floor(RND() * 6 + 1);
	}

	/* knock the player into the above channel */
	c->channel = newChannel;
	Do_lock_mutex(&c->realm->realm_lock);
	Do_player_description(c);
	Do_unlock_mutex(&c->realm->realm_lock);
	Do_send_specification(c, CHANGE_PLAYER_EVENT);

	/* announce the reprimand to the player and the chat */
	sprintf(string_buffer, "You are suddenly knocked back by the force of a reprimand from %s!  After stumbling about for a few moments you realize it would probably be wise not to continue in the way that you have been behaving....\n", the_event->arg4);
	Do_send_line(c, string_buffer);
	
	sprintf(string_buffer, "%s staggers under the brutal force of a reprimand from %s!\n", c->player.name, the_event->arg4);
	Do_broadcast(c, string_buffer);

	Do_more(c);
	Do_send_clear(c);

	/* free up the memory from the event...I think this needs to be done here...I don't see it being done anywhere else */
	free((void *)the_event->arg4);
	/* free((void *)the_event); */

	return;
}

/************************************************************************
/
/ FUNCTION NAME: Do_strong_nf(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: strength spell
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/22/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_strong_nf(struct client_t *c, struct event_t *the_event)
{
    char string_buffer[SZ_LINE];

    if (c->player.strong_nf == 0) {

        sprintf(string_buffer, "%s has transferred some of his might to you!\n", 
	        the_event->arg4);

        Do_send_line(c, string_buffer);

        Do_more(c);
        Do_send_clear(c);

	c->player.strong_nf = 1;
    }
	
    free((void *)the_event->arg4);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_bless(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: blessing
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 9/6/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_bless(struct client_t *c, struct event_t *the_event)
{
    char string_buffer[SZ_LINE];

    sprintf(string_buffer, "You are blessed by %s!\n", 
	    the_event->arg4);

    Do_send_line(c, string_buffer);

    Do_more(c);
    Do_send_clear(c);

    Do_award_blessing(c);

    sprintf(string_buffer, "%s has received the blessing of %s!\n", 
	    c->player.name, the_event->arg4);
    Do_broadcast(c, string_buffer);

    free((void *)the_event->arg4);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_heal(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: healing and curing
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 9/6/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_heal(struct client_t *c, struct event_t *the_event)
{
    struct event_t *event_ptr;
    char string_buffer[SZ_LINE];
    float ftemp;

    if (the_event->arg1 == P_RESTORE) {

        Do_energy(c, c->player.max_energy + c->player.shield + c->knightEnergy,
		c->player.max_energy, c->player.shield, 0, FALSE);

        if (c->player.poison > 0) {
            Do_poison(c, (double) -c->player.poison);
        }

	    /* remove all combat fatigue */
	c->battle.rounds = 0;

	Do_speed(c, c->player.max_quickness, c->player.quicksilver,
		c->battle.speedSpell, FALSE);
    }
    else if (the_event->arg1 == P_CURE) {
          
        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = PLAGUE_EVENT;

        /* curing adds one hundred poison, make sure to fix it so people will
           stick around to wait for it */
        event_ptr->arg3 = floor((c->player.poison * 1000) / 2) - 100000.0;

            /* don't cure if the king ran away */
        if (!Do_send_character_event(c, event_ptr, the_event->arg4)) {
            free ((void *) the_event->arg4);
            free ((void *) event_ptr);
            return;
        }

	Do_energy(c, c->player.energy + (c->player.max_energy +
		c->player.shield - c->player.energy) / 3,
		c->player.max_energy, c->player.shield, 0, FALSE);

        if (c->player.poison > 0) {
	    Do_poison(c, (double) -(c->player.poison / 2));
        }

	    /* remove some combat fatigue */
	c->battle.rounds -= 90;
	if (c->battle.rounds < 0) {
	    c->battle.rounds = 0;
	}

	Do_speed(c, c->player.max_quickness, c->player.quicksilver,
		c->battle.speedSpell, FALSE);

    }
    else {

	Do_energy(c, c->player.energy + (c->player.max_energy +
		c->player.shield - c->player.energy) / 10,
		c->player.max_energy, c->player.shield, 0, FALSE);

	    /* remove some combat fatigue */
	c->battle.rounds -= 30;
	if (c->battle.rounds < 0) {
	    c->battle.rounds = 0;
	}

	Do_speed(c, c->player.max_quickness, c->player.quicksilver,
		c->battle.speedSpell, FALSE);

    }

    if (the_event->arg1 == P_RESTORE) {

        sprintf(string_buffer, "%s restores you to full health!\n", 
	        the_event->arg4);

        Do_send_line(c, string_buffer);

	if (c->player.blind) {
            Do_send_line(c, "The veil of darkness lifts!\n");
            c->player.blind = FALSE;
	}
    }
    else if (the_event->arg1 == P_CURE) {

        sprintf(string_buffer, "You have been cured by %s!\n", 
	        the_event->arg4);

        Do_send_line(c, string_buffer);
    }
    else {

        sprintf(string_buffer, "Your wounds have been healed by %s!\n", 
	        the_event->arg4);

        Do_send_line(c, string_buffer);
    }

    Do_more(c);
    Do_send_clear(c);

    free ((void *) the_event->arg4);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_examine(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: examining a player
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/18/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_examine(struct client_t *c, struct event_t *the_event)
{
    struct game_t *game_ptr;
    struct event_t *event_ptr;
    char playerName[SZ_LINE];

	/* prepare an event */
    event_ptr = (struct event_t *) Do_create_event();
    event_ptr->type = REQUEST_RECORD_EVENT;
    event_ptr->from = c->game;
    event_ptr->arg3 = c->wizard;

	/* get the name of the player to look at */
    if (the_event->arg3 == TRUE) {

        if (Do_player_dialog(c, "Which character do you want to look at?\n",
		playerName) != S_NORM) {

	    return;
	}

	if (!Do_send_character_event(c, event_ptr, playerName)) {
	    free((void *)event_ptr);
            Do_send_line(c, "That character just left the game.\n");
	    Do_more(c);
	    return;
	}
	
	return;
    }

    event_ptr->to = c->game;
    Do_file_event(c, event_ptr);

    return;
}



/************************************************************************
/
/ FUNCTION NAME: Do_enact(struct client_t *c)
/
/ FUNCTION: steward commands
/
/ AUTHOR: E. A. Estes, 2/28/86
/	  Brian Kelly, 5/11/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_enact(struct client_t *c)
{
struct event_t *event_ptr;
struct realm_object_t *object_ptr;
struct button_t buttons;
char string_buffer[SZ_LINE], string_buffer2[SZ_LINE];
char error_msg[SZ_ERROR_MESSAGE];
long answer;
double dtemp;
float ftemp;

	/* determine what the player wants to do */
    strcpy(buttons.button[0], "Transport\n");
    strcpy(buttons.button[1], "Curse\n");
    strcpy(buttons.button[2], "Energy Void\n");
    strcpy(buttons.button[3], "Bestow\n");
    strcpy(buttons.button[4], "Collect Taxes\n");
    strcpy(buttons.button[5], "Throw Smurf\n");
    Do_clear_buttons(&buttons, 6);
    strcpy(buttons.button[7], "Cancel\n");

    if (c->player.gender == MALE) 
        Do_send_line(c, "What would you like done, sir?\n");
    else
        Do_send_line(c, "What would you like done, ma'am?\n");

    if (Do_buttons(c, &answer, &buttons) != S_NORM || answer == 7) {
	Do_send_clear(c);
	return;
    }

    Do_send_clear(c);
    switch(answer) {

    case 0:   /* transport someone */
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = TRANSPORT_EVENT;
	event_ptr->arg3 = FALSE;
        strcpy(string_buffer2, "transport");
        break;

    case 1:   /* curse another */
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = CURSE_EVENT;
        event_ptr->arg1 = P_CURSE;
        strcpy(string_buffer2, "curse");
        break;

    case 2:   /* create energy void */

	    /* create a realm object */
	object_ptr = (struct realm_object_t *) Do_malloc(SZ_REALM_OBJECT);
	object_ptr->type = ENERGY_VOID;

	if (Do_coords_dialog(c, &object_ptr->x, &object_ptr->y,
		"What should the coordinates of the void be?\n")) {

	    free((void *)object_ptr);
	    return;
	}
	
	    /* put the object in place */
	Do_lock_mutex(&c->realm->object_lock);
	object_ptr->next_object = c->realm->objects;
	c->realm->objects = object_ptr;
	Do_unlock_mutex(&c->realm->object_lock);

        if (c->player.gender == MALE)
            Do_send_line(c, "It shall be done as you have ordered, sir.\n");
        else 
            Do_send_line(c, "It shall be done as you have ordered, ma'am.\n");

	Do_more(c);
	Do_send_clear(c);

	if (c->wizard < 3)
	    Do_sin(c, .5);

        return;

    case 3:   /* bestow gems to subject */
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = BESTOW_EVENT;
        strcpy(string_buffer2, "give gems to");

	if (Do_double_dialog(c, &event_ptr->arg1,
		"How many gems to bestow(0-5) ?\n")) {

	    free((void *)event_ptr);
	    return;
	}

	event_ptr->arg1 = floor(event_ptr->arg1);

        if (c->wizard < 3) {
            if (event_ptr->arg1 > c->player.gems) {
                Do_send_line(c, "You don't have that many *gems*!\n");
	        free((void *)event_ptr);
	        Do_more(c);
	        Do_send_clear(c);
                return;
            }

            if (event_ptr->arg1 <= 0) {
	        free((void *)event_ptr);
	        Do_send_clear(c);
                return;
            }

                /* cap the maximum bestow */
	    if (event_ptr->arg1 > 5) {
                Do_send_line(c,
		    "Your head page ran off with the gems!\n");
	        Do_gems(c, -event_ptr->arg1, FALSE);

	        free((void *)event_ptr);
	        Do_more(c);
	        Do_send_clear(c);
                return;
	    }


            /* adjust gold after we are sure it will be given to someone */
	}
        break;

    case 4:   /* collect accumulated taxes */

	    /* lock up the stewards gold */
	Do_lock_mutex(&c->realm->kings_gold_lock);

	dtemp = c->realm->steward_gold;
	c->realm->steward_gold = 0;

	    /* unlock the gold */
	Do_unlock_mutex(&c->realm->kings_gold_lock);

        sprintf(string_buffer, "You have collected %.0lf in gold.\n", dtemp);

	Do_send_line(c, string_buffer);
	Do_more(c);
	Do_send_clear(c);

        if (dtemp > 0) {
            c->player.gold += dtemp;
            Do_gold(c, 0, TRUE);
            Do_check_weight(c);
        }

	if (c->wizard < 3)
	    Do_sin(c, .25);

        return;

    case 5:
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = SUMMON_EVENT;
        event_ptr->arg1 = 40;
        strcpy(string_buffer2, "smurf");

        break;
        

    default:
        sprintf(error_msg, "[%s] Returned non-option in Do_enact.\n",
                c->connection_id);

        Do_log_error(error_msg);
	free((void *)event_ptr);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

	/* if we're here, we have an event to send to another player */
        /* prompt for player to affect */
    sprintf(string_buffer, "Who do you want to %s?\n", string_buffer2);
    if (Do_player_dialog(c, string_buffer, string_buffer2) != S_NORM) {
        free((void *)event_ptr);
	return;
    }

        /* if the player named himself */
    if (!strcmp(string_buffer2, c->modifiedName) && c->wizard < 3) {
	free((void *)event_ptr);
        Do_send_line(c, "You may not do it to yourself!\n");
	Do_more(c);
	Do_send_clear(c);
	return;
    }

    event_ptr->arg3 = strlen(c->modifiedName) + 1;
    event_ptr->arg4 = (void *) Do_malloc(event_ptr->arg3);
    strcpy(event_ptr->arg4, c->modifiedName);
    event_ptr->from = c->game;

    if (!Do_send_character_event(c, event_ptr, string_buffer2)) {
        free((void *)event_ptr->arg4);
        free((void *)event_ptr);
        Do_send_line(c, "That character just left the game.\n");
        Do_more(c);
	Do_send_clear(c);
        return;
    }

    if (c->wizard < 3) {

            /* Get rid of gold, if we're giving it away */
        if (event_ptr->type == BESTOW_EVENT) {
	    Do_gems(c, -event_ptr->arg1, FALSE);

                /* check for impeachment */
            if (RND() * 40 < (event_ptr->arg1 + 1) * event_ptr->arg1) {
                Do_send_line(c, "The Tax Collectors discovered your action and impeached you for your excessive generosity!\n");
                Do_dethrone(c);
                Do_more(c);
	        Do_send_clear(c);
                return;
            }

	}
	    /* add some sin for the more evil powers */
        if (event_ptr->type == TRANSPORT_EVENT) {
	    Do_sin(c, 1.0);
        }
        else if (event_ptr->type == CURSE_EVENT) {
	    Do_sin(c, .5);
        }
        else if (event_ptr->type == MONSTER_EVENT) {
	    Do_sin(c, .1);
        }
    }

    if (c->player.gender == MALE)
        Do_send_line(c, "It has been done as you have ordered, sir.\n");
    else 
        Do_send_line(c, "It has been done as you have ordered, ma'am.\n");

    Do_more(c);
    Do_send_clear(c);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_decree(struct client_t *c)
/
/ FUNCTION: king commands
/
/ AUTHOR: E. A. Estes, 2/28/86
/	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_decree(struct client_t *c)
{
struct event_t *event_ptr;
struct button_t buttons;
char string_buffer[SZ_LINE], string_buffer2[SZ_LINE];
char error_msg[SZ_ERROR_MESSAGE];
long answer;
double dtemp;
float ftemp;

	/* determine what the player wants to do */
    strcpy(buttons.button[0], "Oust\n");
    strcpy(buttons.button[1], "Execrate\n");
    strcpy(buttons.button[2], "Cure\n");
    
    strcpy(buttons.button[3], "Collect Taxes\n");
    strcpy(buttons.button[4], "Knight\n");
    strcpy(buttons.button[5], "Tax\n");
    Do_clear_buttons(&buttons, 6);
    strcpy(buttons.button[7], "Cancel\n");

    Do_send_line(c, "What would you like done, your Majesty?\n");
    if (Do_buttons(c, &answer, &buttons) != S_NORM || answer == 7) {
	Do_send_clear(c);
	return;
    }

    Do_send_clear(c);
    switch(answer) {

    case 0:   /* oust someone */
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = TRANSPORT_EVENT;
	event_ptr->arg1 = TRUE;
        strcpy(string_buffer2, "oust");
        break;

    case 1:   /* execrate another */
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = CURSE_EVENT;
        event_ptr->arg1 = P_EXECRATE;
        strcpy(string_buffer2, "execrate");
        break;

    case 2:   /* cure the character */

	    /* create a cure event */
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = HEAL_EVENT;
        event_ptr->arg1 = P_CURE;
        strcpy(string_buffer2, "cure");

        /* add a lot of poison that will be cured when the poison from
           curing is received */
        if (c->wizard < 3) {
            Do_poison(c, 100.0);
        }
        break;


    case 3:   /* collect accumulated taxes */

	    /* lock up the kings gold */
	Do_lock_mutex(&c->realm->kings_gold_lock);

	dtemp = c->realm->kings_gold;
	c->realm->kings_gold = 0;
	dtemp += c->realm->steward_gold;
	c->realm->steward_gold = 0;

	    /* lock up the kings gold */
	Do_unlock_mutex(&c->realm->kings_gold_lock);

        sprintf(string_buffer, "You have collected %.0lf in gold.\n", dtemp);

	Do_send_line(c, string_buffer);
	Do_more(c);
	Do_send_clear(c);

        if (dtemp > 0) {
            c->player.gold += dtemp;
            Do_gold(c, 0, TRUE);
            Do_check_weight(c);
        }

	if (c->wizard < 3)
	    Do_sin(c, .25);

        return;

    case 4:   /* knight the character */

	    /* create a knight event */
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = KNIGHT_EVENT;
        strcpy(string_buffer2, "knight");
        break;

    case 5:   /* send tax collector */

	    /* create a tax collector event */
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = TAX_EVENT;
        strcpy(string_buffer2, "tax");
        break;

    default:
        sprintf(error_msg, "[%s] Returned non-option in Do_decree.\n",
                c->connection_id);

        Do_log_error(error_msg);
	free((void *)event_ptr);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

	/* if we're here, we have an event to send to another player */
        /* prompt for player to affect */
    sprintf(string_buffer, "Who do you want to %s?\n", string_buffer2);
    if (Do_player_dialog(c, string_buffer, string_buffer2) != S_NORM) {
        free((void *)event_ptr);
	return;
    }

        /* if the non-wizard player named himself */
    if (!strcmp(string_buffer2, c->modifiedName) && c->wizard < 3) {
	free((void *)event_ptr);
        Do_send_line(c, "You may not do it to yourself!\n");
	Do_more(c);
	Do_send_clear(c);
	return;
    }

    event_ptr->arg3 = strlen(c->modifiedName) + 1;
    event_ptr->arg4 = (void *) Do_malloc(event_ptr->arg3);
    strcpy(event_ptr->arg4, c->modifiedName);
    event_ptr->from = c->game;

    if (!Do_send_character_event(c, event_ptr, string_buffer2)) {
        free((void *)event_ptr->arg4);
        free((void *)event_ptr);
        Do_send_line(c, "That character just left the game.\n");
        Do_more(c);
	Do_send_clear(c);
        return;
    }

    if (c->wizard < 3) {

            /* Get rid of gold, if we're giving it away */
        if (event_ptr->type == BESTOW_EVENT)
	    Do_gold(c, -event_ptr->arg1, FALSE);

	    /* Add sin for some of the nastier powers */
        if (event_ptr->type == TRANSPORT_EVENT) {
	    Do_sin(c, 1.5);
        }
        else if (event_ptr->type == CURSE_EVENT) {
	    Do_sin(c, .75);
        } else if (event_ptr->type == TAX_EVENT) {
	    Do_sin(c, .5);
        }
    }

    Do_send_line(c, "It shall be done as you have ordered, your Majesty.\n");
    Do_more(c);
    Do_send_clear(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_intervene(struct client_t *c)
/
/ FUNCTION: council commands
/
/ AUTHOR: E. A. Estes, 2/28/86
/	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_intervene(struct client_t *c)
{
struct event_t *event_ptr;
struct realm_object_t *object_ptr, *theObject;
struct button_t theButtons;
char string_buffer[SZ_LINE], string_buffer2[SZ_LINE];
char error_msg[SZ_ERROR_MESSAGE];
long answer;              /* pointer to option description */
double distance, theDistance;
float ftemp;
double manaCost;

	/* determine what the player wants to do */

	/* one can only seek the grail with a palantir */
    strcpy(theButtons.button[0], "Slap\n");
    strcpy(theButtons.button[1], "Blind\n");
    strcpy(theButtons.button[2], "Heal\n");
    Do_clear_buttons(&theButtons, 3);

    if (c->player.palantir || (c->wizard > 2)) {
        strcpy(theButtons.button[3], "Seek Grail\n");
        strcpy(theButtons.button[4], "Seek Corpse\n");
    }

    strcpy(theButtons.button[7], "Cancel\n");

    if (c->player.gender == MALE) 
        Do_send_line(c, "What would you like done, m'lord?\n");
    else
        Do_send_line(c, "What would you like done, m'lady?\n");

    if (Do_buttons(c, &answer, &theButtons) != S_NORM || answer == 7 ) {
	Do_send_clear(c);
	return;
    }

    Do_send_clear(c);

    manaCost = floor(c->player.level * .000005 * (1000 + c->player.level * 
                c->realm->charstats[c->player.type].max_mana));

    if (c->player.mana < manaCost && (c->wizard < 3)) {
	Do_send_line(c, "Not enough mana left.\n");
	Do_more(c);
        Do_send_clear(c);
	return;
    }

    switch(answer) {

    case 0:   /* slap */

	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = SLAP_EVENT;
        strcpy(string_buffer2, "slap");
	break;

    case 1:   /* blind */

	    /* create a monster event */
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = BLIND_EVENT;
	event_ptr->arg1 = TRUE;
        strcpy(string_buffer2, "blind");
	break;

    case 2:   /* heal */

	    /* create a heal event */
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = HEAL_EVENT;
        event_ptr->arg1 = P_HEAL;
        strcpy(string_buffer2, "heal");
        break;

    case 3:   /* seek grail */

	    /* find the grail */
	Do_lock_mutex(&c->realm->realm_lock);
	object_ptr = c->realm->objects;

	    /* run through the realm objects */
	while (object_ptr != NULL) {

	    if (object_ptr->type == HOLY_GRAIL) {
		break;
	    }

	    object_ptr = object_ptr->next_object;
	}

	if (object_ptr == NULL) {

	    Do_unlock_mutex(&c->realm->realm_lock);

	    sprintf(error_msg,
		    "[%s] No grail found in realm objects in Do_intervene.\n",
		    c->connection_id);

	    Do_log_error(error_msg);

	    Do_send_line(c,
       "The palantir fills with black clouds.  It is of no use to you now.\n");

	    Do_more(c);
	    Do_send_clear(c);
	    return;
	}

	    /* determine the current distance to the grail */
	Do_distance(c->player.x, object_ptr->x, c->player.y, object_ptr->y,
		&distance);

	Do_unlock_mutex(&c->realm->realm_lock);

	    /* throw in a fudge factor of up to 25% */
        if (c->player.special_type != SC_EXVALAR) {
	    distance = floor(distance * (.75 + RND() / 2) + .25);
	    /* throw in a fudge factor of up to 100% for exvalars */
        } else {

	    /* also summon Morgoth in randomly when player gets close */
            if ((distance < c->player.level / 1000) && 
                (RND() < (c->player.level * (10 - distance) / 100000))
               ) {

	        event_ptr = (struct event_t *) Do_create_event();
                event_ptr->type = MONSTER_EVENT;
                event_ptr->arg1 = MONSTER_SUMMONED;
                event_ptr->arg3 = 15;
                Do_file_event(c, event_ptr);
            }
                
	    distance = floor(distance * (.5 + RND()) + .25);
        }

/*
        if ((strcmp(c->player.parent_account, c->lcaccount) &&
            RND() < .5)) {
            distance = distance * 3 * RND();
        }
*/

        if (distance == 0) {
            distance = 1;
        }


	sprintf(string_buffer,
		"The palantir says the grail is %0.lf distance from here.\n", 
		distance);

	Do_send_line(c, string_buffer);

        Do_more(c);


	if (c->wizard < 3)
            Do_mana(c, -manaCost, FALSE);

	return;

    case 4:   /* find the nearest corpse */

	    /* find the grail */
	Do_lock_mutex(&c->realm->realm_lock);
	object_ptr = c->realm->objects;
        theObject = NULL;
	theDistance = 1000;

	    /* run through the realm objects */
	while (object_ptr != NULL) {

	    if (object_ptr->type == CORPSE) {

		Do_distance(c->player.x, object_ptr->x, c->player.y,
			object_ptr->y, &distance);

		if (distance < theDistance) {
		    theDistance = distance;
		    theObject = object_ptr;
		}
	    }

	    object_ptr = object_ptr->next_object;
	}

	if (theObject == NULL) {

	    Do_unlock_mutex(&c->realm->realm_lock);

	    Do_send_line(c,
       "The palantir fills with black clouds.  It is of no use to you now.\n");

	    Do_more(c);
	    Do_send_clear(c);
	    return;
	}

	    /* determine the direction to the corpse */
	error_msg[0] = '\0';
	Do_direction(c, &theObject->x, &theObject->y, error_msg);
	Do_unlock_mutex(&c->realm->realm_lock);

	sprintf(string_buffer,
		"The palantir says there's a corpse to the %s.\n", error_msg);

	Do_send_line(c, string_buffer);

	Do_more(c);
	Do_send_clear(c);

	if (c->wizard < 3)
            Do_mana(c, -manaCost, FALSE);

	return;

    default:
        sprintf(error_msg, "[%s] Returned non-option in Do_intervene.\n",
                c->connection_id);
 
        Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

	/* if we're here, we have an event to send to another player */
        /* prompt for player to affect */
    sprintf(string_buffer, "Who do you want to %s?\n", string_buffer2);
    if (Do_player_dialog(c, string_buffer, string_buffer2) != S_NORM) {
        free((void *)event_ptr);
        return;
    }

        /* if the player named himself and isn't the wizard */
    if (!strcmp(string_buffer2, c->modifiedName) && (c->wizard < 3)) {
        free((void *)event_ptr);
        Do_send_line(c, "You may not do it to yourself!\n");
        Do_more(c);
        Do_send_clear(c);
	return;
    }

    event_ptr->arg3 = strlen(c->modifiedName) + 1;
    event_ptr->arg4 = (void *) Do_malloc(event_ptr->arg3);
    strcpy(event_ptr->arg4, c->modifiedName);
    event_ptr->from = c->game;

    if (!Do_send_character_event(c, event_ptr, string_buffer2)) {
        free((void *)event_ptr->arg4);
        free((void *)event_ptr);
        Do_send_line(c, "That character just left the game.\n");
        Do_more(c);
        Do_send_clear(c);
        return;
    }

    if (c->wizard < 3) {

        Do_mana(c, -manaCost);

	if (event_ptr->type == SLAP_EVENT) {
	    Do_sin(c, .25);
	}
	else if (event_ptr->type == BLIND_EVENT) {
	    Do_sin(c, .5);
	}
    }

    if (c->player.gender == MALE)
        Do_send_line(c, "It shall be done, m'lord.\n");
    else
        Do_send_line(c, "It shall be done, m'lady.\n");

    Do_more(c);
    Do_send_clear(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_command(struct client_t *c)
/
/ FUNCTION: valar commands
/
/ AUTHOR: E. A. Estes, 2/28/86
/	  Brian Kelly, 11/10/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_command(struct client_t *c)
{
struct event_t *event_ptr;
struct realm_object_t *object_ptr;
struct button_t theButtons;
char string_buffer[SZ_LINE], string_buffer2[SZ_LINE];
char error_msg[SZ_ERROR_MESSAGE];
long answer;              /* pointer to option description */
float ftemp, manaCost;
double dtemp;

	/* determine what the player wants to do */
    strcpy(theButtons.button[0], "Smite\n");
    strcpy(theButtons.button[1], "Degenerate\n");
    strcpy(theButtons.button[2], "Relocate\n");
    strcpy(theButtons.button[3], "Restore\n");
    strcpy(theButtons.button[4], "Bless\n");
    strcpy(theButtons.button[5], "Proclaim\n");
    strcpy(theButtons.button[6], "Slap\n");
    strcpy(theButtons.button[7], "Cancel\n");


    Do_send_line(c, "What kind of mayhem did you have in mind, eminence?\n");
    if (Do_buttons(c, &answer, &theButtons) != S_NORM || answer == 7 ) {
	Do_send_clear(c);
	return;
    }

    Do_send_clear(c);

    manaCost = floor(.33 * (1000 + c->player.level * 
                c->realm->charstats[c->player.type].max_mana));


    if (c->player.mana < manaCost && (c->wizard < 3)) {
	Do_send_line(c, "Not enough mana left.\n");
	Do_more(c);
	Do_send_clear(c);
	return;
    }

    event_ptr = (struct event_t *) Do_create_event();

    switch(answer) {

    case 0:	/* smite */
        event_ptr->type = CURSE_EVENT;
	event_ptr->arg1 = P_SMITE;
        strcpy(string_buffer2, "smite");
        break;

    case 1:	/* degenerate */
        event_ptr->type = DEGENERATE_EVENT;
        strcpy(string_buffer2, "degenerate");
        break;

    case 2:   /* relocate someone */
        event_ptr->type = RELOCATE_EVENT;
        strcpy(string_buffer2, "relocate");

        if (Do_coords_dialog(c, &event_ptr->arg1, &event_ptr->arg2,
		    "Relocate to where?\n")) {

	    free((void *)event_ptr);
	    return;
	}

	if (fabs(event_ptr->arg1) >= D_BEYOND || fabs(event_ptr->arg2) >=
		D_BEYOND) {

	    Do_send_line(c,
		   "Relocate will only work inside the point of no return.\n");

	    free((void *)event_ptr);
	    Do_more(c);
	    Do_send_clear(c);
	    return;
	}

	break;

    case 3:   /* restore someone */
        event_ptr->type = HEAL_EVENT;
        event_ptr->arg1 = P_RESTORE;
        strcpy(string_buffer2, "restore");
        break;

    case 4:   /* bless another */
        if (c->wizard < 3) {

	      /* with blessings, pull lives */
	    if (c->player.lives) {

	        Do_send_line(c,
	       "This bless will cost you a life.  Do you wish to continue?\n");
	    }
	    else {

	        Do_send_line(c, 
                "You will lose the position of Valar.  Do you wish to continue?\n");
	    }

            if (Do_yes_no(c, &answer) != S_NORM || answer == 1) {
                free((void *)event_ptr);
	        Do_send_clear(c);
                return;
            }

	    if (c->player.lives) {
	        --c->player.lives;
	    }
	    else {
	        Do_valar(c);
	    }
	}

        event_ptr->type = BLESS_EVENT;
        strcpy(string_buffer2, "bless");
        break;

    case 5:   /* proclaim */

	free((void *)event_ptr);

        if (c->wizard < 3) {
	    Do_mana(c, -manaCost, FALSE);
        }

	    /* set the broadcast flag */
	c->broadcast = TRUE;

	Do_send_line(c,
	    "Your next chat message will be proclaimed on all channels.\n");

	Do_more(c);
	Do_send_clear(c);
	return;

   case 6:
        event_ptr->type = SLAP_EVENT;
        strcpy(string_buffer2, "slap");
	break;


    default:
	free((void *)event_ptr);
        sprintf(error_msg, "[%s] Returned non-option in Do_command.\n",
                c->connection_id);
 
        Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

	/* if we're here, we have an event to send to another player */
        /* prompt for player to affect */
    sprintf(string_buffer, "Who do you want to %s?\n", string_buffer2);
    if (Do_player_dialog(c, string_buffer, string_buffer2) != S_NORM) {
        free((void *)event_ptr);
        return;
    }

        /* if the player named himself */
    if (!strcmp(string_buffer2, c->modifiedName) && c->wizard < 3) {
        free((void *)event_ptr);
        Do_send_line(c, "You may not do it to yourself!\n");
        Do_send_line(c, "(You should know the routine by now!)\n");
        Do_more(c);
	Do_send_clear(c);
	return;
    }

    event_ptr->arg3 = strlen(c->modifiedName) + 1;
    event_ptr->arg4 = (void *) Do_malloc(event_ptr->arg3);
    strcpy(event_ptr->arg4, c->modifiedName);
    event_ptr->from = c->game;

    if (!Do_send_character_event(c, event_ptr, string_buffer2)) {
        free((void *)event_ptr->arg4);
        free((void *)event_ptr);
        Do_send_line(c, "That character just left the game.\n");
        Do_more(c);
	Do_send_clear(c);
        return;
    }

    if (c->wizard < 3) {

	    /* remove the mana */
	Do_mana(c, -manaCost, FALSE);

	    /* add sin for the nastier abilities */
	if (event_ptr->type == CURSE_EVENT) {
	    Do_sin(c, 0.5);
	}
	else if (event_ptr->type == DEGENERATE_EVENT) {
	    Do_sin(c, 0.5);
	}
	else if (event_ptr->type == SLAP_EVENT) {
	    Do_sin(c, 0.1);
	}
	else if (event_ptr->type == RELOCATE_EVENT) {
            dtemp = floor((sqrt(pow(event_ptr->arg1, 2) 
                              + pow(event_ptr->arg2, 2)) / D_CIRCLE) + 1);
	    Do_sin(c, MIN(5.0, (dtemp / 100.0)));
	}
    }

    Do_send_line(c, "What you ask shall be done, eminence.\n");
    Do_more(c);
    Do_send_clear(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_administrate(struct client_t *c)
/
/ FUNCTION: wizard commands
/
/ AUTHOR: E. A. Estes, 2/28/86
/	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_administrate(struct client_t *c)
{
struct event_t *event_ptr;
struct realm_object_t *object_ptr;
struct button_t theButtons;
char string_buffer[SZ_LINE], string_buffer2[SZ_LINE];
char error_msg[SZ_ERROR_MESSAGE];
long answer;              /* pointer to option description */
int itemp;

	/* determine what the player wants to do */
    strcpy(theButtons.button[0], "Enact\n");
    strcpy(theButtons.button[1], "Decree\n");
    strcpy(theButtons.button[2], "Intervene\n");
    strcpy(theButtons.button[3], "Command\n");
    strcpy(theButtons.button[4], "Moderate\n");
    strcpy(theButtons.button[5], "Send Event\n");
    strcpy(theButtons.button[6], "Shutdown\n");
    strcpy(theButtons.button[7], "Cancel\n");
    Do_clear_buttons(&theButtons, 8);

    Do_send_line(c, "What would you like to do?\n");
    if (Do_buttons(c, &answer, &theButtons) != S_NORM || answer == 7) {	
	Do_send_clear(c);
	return;
    }

    Do_send_clear(c);
    switch(answer) {

	/* order a shutdown */
    case 6:

        strcpy(theButtons.button[0], "Leisure\n");
        strcpy(theButtons.button[1], "Normal\n");
        strcpy(theButtons.button[2], "Fast\n");
        strcpy(theButtons.button[3], "Hard\n");
        Do_clear_buttons(&theButtons, 4);
        strcpy(theButtons.button[7], "Cancel\n");

	Do_send_line(c, "How do you wish to shutdown the server?\n");
	if (Do_buttons(c, &answer, &theButtons) != S_NORM || answer == 7) {
	    Do_send_clear(c);
	    return;
	}

	Do_send_clear(c);
	switch(answer) {

	    /* shutdown game when no players are logged on */
	case 0:
	    itemp = LEISURE_SHUTDOWN;
	    break;

	    /* wait for all threads to exit */
	case 1:
	    itemp = SHUTDOWN;
	    break;

	    /* forget the threads, start saving the realm */
	case 2:
	    itemp = FAST_SHUTDOWN;
	    break;

	    /* order the server down now */
	case 3:
	    itemp = HARD_SHUTDOWN;
	    break;

	default:
            sprintf(error_msg, "[%s] Returned non-option in Do_administrate.\n",
                    c->connection_id);
 
            Do_log_error(error_msg);
	    Do_caught_hack(c, H_SYSTEM);
	    return;
	}

	    /* double check */
	Do_send_line(c, "Are you sure you wish to shutdown the server?\n");
        if (Do_yes_no(c, &answer) != S_NORM || answer == 1) {
	    Do_send_clear(c);
            return;
        }

	Do_send_clear(c);
	server_hook = itemp;
	return;

        /* build your own event */
    case 5:

	event_ptr = (struct event_t *) Do_create_event();

        if (Do_long_dialog(c, &event_ptr->arg3, "Event number?\n")) {
	    free((void *)event_ptr);
	    return;
	}

	event_ptr->type = (short) event_ptr->arg3;

 	if (Do_double_dialog(c, &event_ptr->arg1, "Argument 1?\n")) {
	    free((void *)event_ptr);
	    return;
	}

 	if (Do_double_dialog(c, &event_ptr->arg2, "Argument 2?\n")) {
	    free((void *)event_ptr);
	    return;
	}

            
	if (Do_long_dialog(c, &event_ptr->arg3, "Argument 3?\n")) {
	    free((void *)event_ptr);
	    return;
	}

        strcpy(string_buffer2, "send the event to");
        break;

    case 0:

	Do_enact(c);
	return;

    case 1:

	Do_decree(c);
	return;

    case 2:

	Do_intervene(c);
	return;

    case 3:

	Do_command(c);
	return;

    case 4:

	Do_moderate(c);
	return;

    default:
        sprintf(error_msg, "[%s] Returned non-option in Do_administrate(2).\n",
                c->connection_id);
 
        Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

	/* if we're here, we have an event to send to another player */
        /* prompt for player to affect */
    sprintf(string_buffer, "Who do you want to %s?\n", string_buffer2);
    if (Do_player_dialog(c, string_buffer, string_buffer2) != S_NORM) {
        free((void *)event_ptr);
        return;
    }

    event_ptr->from = c->game;

    if (!Do_send_character_event(c, event_ptr, string_buffer2)) {
        free((void *)event_ptr);
        Do_send_line(c, "That character just left the game.\n");
        Do_more(c);
        return;
    }

    Do_send_line(c, "Game modification complete.\n");
    Do_more(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_moderate(struct client_t *c)
/
/ FUNCTION: wizard administrative powers
/
/ AUTHOR: Brian Kelly, 11/8/00
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_moderate(struct client_t *c)
{
char error_msg[SZ_ERROR_MESSAGE];
struct button_t theButtons;
long answer;              /* pointer to option description */

	/* determine what the player wants to do */
    strcpy(theButtons.button[0], "History\n");
    strcpy(theButtons.button[1], "Info\n");
    strcpy(theButtons.button[2], "Cantrip\n");
    strcpy(theButtons.button[3], "Flog\n");
    strcpy(theButtons.button[4], "Modify\n");
    strcpy(theButtons.button[5], "Test\n");
    Do_clear_buttons(&theButtons, 6);
    strcpy(theButtons.button[7], "Cancel\n");

    Do_send_line(c, "How do you wish to moderate the game?\n");
    if (Do_buttons(c, &answer, &theButtons) != S_NORM || answer == 7) {
        Do_send_clear(c);
	return;
    }

    Do_send_clear(c);
    switch(answer) {

        /* Get game information */
    case 0:
	Do_history(c);
        return;

    case 1:
	Do_wizard_information(c);
	return;

    case 2:
        Do_cantrip(c);
        return;

    case 3:
	Do_flog(c);
	return;

    case 4:
	Do_modify(c);
	return;
	
	case 5:
	c->player.sin += 1;
	Do_send_line(c, "Dont ask Brian...Just dont ask/n");
	break;
	
    default:
        sprintf(error_msg, "[%s] Returned non-option in Do_moderate.\n",
                c->connection_id);
 
        Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_flog(struct client_t *c)
/
/ FUNCTION: discipline players
/
/ AUTHOR: Brian Kelly, 01/17/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_flog(struct client_t *c)
{
struct event_t *event_ptr;
struct button_t theButtons;
char string_buffer[SZ_LINE], string_buffer2[SZ_LINE];
char error_msg[SZ_ERROR_MESSAGE];
long answer;              /* pointer to option description */

	/* determine what the player wants to do */
    strcpy(theButtons.button[0], "Suspend\n");
    strcpy(theButtons.button[1], "Unsuspend\n");
    strcpy(theButtons.button[2], "Mute\n");
    strcpy(theButtons.button[3], "Kick\n");
    strcpy(theButtons.button[4], "Ban\n");
    strcpy(theButtons.button[5], "BoD\n");
    strcpy(theButtons.button[6], "Vaporize\n");
    strcpy(theButtons.button[7], "Cancel\n");

    Do_send_line(c, "How do you wish to flog the players?\n");
    if (Do_buttons(c, &answer, &theButtons) != S_NORM || answer == 7) {
	Do_send_clear(c);
	return;
    }
    Do_send_clear(c);
    switch(answer) {

        /* Suspend player */
    case 0:
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = SUSPEND_EVENT;
        strcpy(string_buffer2, "lock");
        break;

        /* Unsuspend player */
    case 1:
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = UNSUSPEND_EVENT;
        strcpy(string_buffer2, "unlock");
        break;

	/* mute player */
    case 2:
	Do_create_tag(c, T_MUTE);
	return;

        /* Kick player */
    case 3:
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = KICK_EVENT;
        strcpy(string_buffer2, "kick from the game");
        break;

	/* ban player */
    case 4:
	Do_create_tag(c, T_BAN);
	return;

        /* Squish player */
    case 5:
	event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = DEATH_EVENT;
        event_ptr->arg3 = K_SQUISH;
        strcpy(string_buffer2, "squish");
        break;

    case 6:	/* vaporize */
        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = DEATH_EVENT;
        event_ptr->arg3 = K_VAPORIZED;
        strcpy(string_buffer2, "vaporize");
        break;

    default:
        sprintf(error_msg, "[%s] Returned non-option in Do_flog.\n",
                c->connection_id);
 
        Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

	/* if we're here, we have an event to send to another player */
        /* prompt for player to affect */
    sprintf(string_buffer, "Who do you want to %s?\n", string_buffer2);
    if (Do_player_dialog(c, string_buffer, string_buffer2) != S_NORM) {
        free((void *)event_ptr);
        return;
    }

    event_ptr->arg4 = (void *) Do_malloc(strlen(c->modifiedName) + 1);
    strcpy(event_ptr->arg4, c->modifiedName);
    event_ptr->from = c->game;

    if (!Do_send_character_event(c, event_ptr, string_buffer2)) {
        free((void *)event_ptr->arg4);
        free((void *)event_ptr);
        Do_send_line(c, "That character just left the game.\n");
        Do_more(c);
        return;
    }

    Do_send_line(c, "Flog complete.\n");
    Do_more(c);
    Do_send_clear(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_cantrip(struct client_t *c)
/
/ FUNCTION: apprentice functions
/
/ AUTHOR: Eugene Hung, 08/03/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_cantrip(struct client_t *c)
{
struct event_t *event_ptr;
struct button_t theButtons;
char string_buffer[SZ_LINE], string_buffer2[SZ_LINE];
char error_msg[SZ_ERROR_MESSAGE];
char victim[SZ_NAME];
char warning[SZ_LINE];
double dtemp;
long answer;              /* pointer to option description */

	/* determine what the player wants to do */
    Do_clear_buttons(&theButtons, 0);
    strcpy(theButtons.button[0], "Mute\n");
    strcpy(theButtons.button[1], "Ban\n");
    strcpy(theButtons.button[2], "Tag\n");
    strcpy(theButtons.button[3], "Reprimand\n");
    strcpy(theButtons.button[4], "Clear Smurf\n");
    if (c->game->hearAllChannels == HEAR_SELF) {
        strcpy(theButtons.button[5], "Hear Ch 1\n");
    } else {
        strcpy(theButtons.button[5], "Tune Out\n");

    }


    if (c->player.location == PL_THRONE && c->player.special_type
	    == SC_STEWARD && !c->player.cloaked) {

        strcpy(theButtons.button[6], "Enact\n");
    }
    else if (c->player.location == PL_THRONE && c->player.special_type
	    == SC_KING && !c->player.cloaked) {

        strcpy(theButtons.button[6], "Decree\n");
    }
    else if ((c->player.special_type == SC_COUNCIL || c->player.special_type
	== SC_EXVALAR) && !c->player.cloaked) {

        strcpy(theButtons.button[6], "Intervene\n");
    }
    else if (c->player.special_type == SC_VALAR && !c->player.cloaked) {
        strcpy(theButtons.button[6], "Command\n");
    }
    else if (c->player.level < 10) {
        strcpy(theButtons.button[6], "Help\n");
    }
    strcpy(theButtons.button[7], "Cancel\n");

	/* create an event to handle the player action */
    event_ptr = (struct event_t *) Do_create_event();
    event_ptr->from = c->game;
    event_ptr->type = NULL_EVENT;

    Do_send_line(c, "What cantrip do you wish to cast?\n");
    sprintf(string_buffer, "Current Smurf : %s\n", c->realm->monster[40].name);
    Do_send_line(c, string_buffer);
    if (Do_buttons(c, &answer, &theButtons) != S_NORM || answer == 7) {
	Do_send_clear(c);
	return;
    }
    Do_send_clear(c);
    switch(answer) {

	/* mute player */
    case 0:
	Do_create_minitag(c, T_MUTE);
	break;


	/* ban player */
    case 1:
	Do_create_minitag(c, T_BAN);
	break;


        /* tag player */
    case 2:
        strcpy(theButtons.button[0], "Prefix\n");
        strcpy(theButtons.button[1], "Suffix\n");
	strcpy(theButtons.button[2], "Untag\n");
        Do_clear_buttons(&theButtons, 3);
        strcpy(theButtons.button[7], "Cancel\n");
        Do_send_line(c, "What type of tag?\n");
        if (Do_buttons(c, &answer, &theButtons) != S_NORM || answer == 7) {
	    Do_send_clear(c);
	    return;
        }

        Do_send_clear(c);
        if (answer == 0) {
            Do_create_minitag(c, T_PREFIX);
        } else if (answer == 1) {
            Do_create_minitag(c, T_SUFFIX);
        } else {
		struct event_t *eventPtr;
		char untagee[SZ_NAME];

		Do_log(DEBUG_LOG, "Untag code reached\n");

		/* whose tag will we remove? */
		sprintf(string_buffer, "Whose tag do you wish to remove?\n");
		if(Do_player_dialog(c, string_buffer, untagee) != S_NORM)
		{
			return;
		}
		
		Do_log(DEBUG_LOG, "Untagee selected\n");

		/* create the event */
		eventPtr = (struct event_t *) Do_create_event();
		
		/* only the event type should be needed to throw this event */
		eventPtr->type = UNTAG_EVENT;

		Do_log(DEBUG_LOG, "Untag event created\n");
		Do_send_character_event(c, eventPtr, untagee);
	}
        break;

		/* Reprimand player */
    case 3:
	/* tell the event it's a reprimand */
        event_ptr->type = REPRIMAND_EVENT;
	
	/* target the reprimand */
	sprintf(string_buffer, "Who do you want to reprimand?\n");
	if(Do_player_dialog(c, string_buffer, string_buffer2) != S_NORM)
	{
		free((void *)event_ptr);
		return;
	}
	event_ptr->arg3 = strlen(c->modifiedName) + 1;
	event_ptr->arg4 = (void *) Do_malloc(event_ptr->arg3);
	strcpy(event_ptr->arg4, c->modifiedName);
	
	/* send off the event */
	if(!Do_send_character_event(c, event_ptr, string_buffer2))
	{
		free((void *)event_ptr->arg4);
		free((void *)event_ptr);
		Do_send_line(c, "That character just left the game.\n");
		Do_more(c);
		Do_send_clear(c);
	}
	break;


        /* clear smurf name */
    case 4:
	Do_lock_mutex(&c->realm->monster_lock);
	strcpy(c->realm->monster[40].name, "A Smurf");
	Do_unlock_mutex(&c->realm->monster_lock);
        break;
         
        /* let player always hear channel 1 */
    case 5:
        Do_lock_mutex(&c->realm->realm_lock);
        if (c->game->hearAllChannels == HEAR_SELF) {
             c->game->hearAllChannels = HEAR_ONE;
        } else {
             c->game->hearAllChannels = HEAR_SELF;
        }
        Do_unlock_mutex(&c->realm->realm_lock);
        break;

        /* use special position powers */
    case 6:

        event_ptr->to = c->game;

        switch (c->player.special_type) {
        
            case SC_STEWARD:
                Do_enact(c);	
                break;

            case SC_KING:
                Do_decree(c);	
                break;

            case SC_COUNCIL:
            case SC_EXVALAR:
                Do_intervene(c);	
                break;

            case SC_VALAR:
                Do_command(c);	
                break;

            default:
                Do_help(c);
                break;
            break;
        }

        break;
        

    default:
        sprintf(error_msg, "[%s] Returned non-option in Do_cantrip.\n",
                c->connection_id);
 
        Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

    if (event_ptr->type == NULL_EVENT) {
	free((void *)event_ptr);
    }


    return;
}

/************************************************************************
/
/ FUNCTION NAME: Do_modify(struct client_t *c)
/
/ FUNCTION: more wizard commands
/
/ AUTHOR: Brian Kelly, 01/17/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_modify(struct client_t *c)
{
struct button_t theButtons;
char string_buffer[SZ_LINE], string_buffer2[SZ_LINE];
char characterName[SZ_NAME];
char error_msg[SZ_ERROR_MESSAGE];
long answer;              /* pointer to option description */

	/* determine what the player wants to do */
    strcpy(theButtons.button[0], "Prefix\n");
    strcpy(theButtons.button[1], "Suffix\n");
    strcpy(theButtons.button[2], "Suicide\n");
    strcpy(theButtons.button[3], "Special Clear\n");
    strcpy(theButtons.button[4], "Untag\n");
    strcpy(theButtons.button[5], "Restore Char\n");
    Do_clear_buttons(&theButtons, 6);
    strcpy(theButtons.button[7], "Cancel\n");

    Do_send_line(c, "Please choose the modification you wish.\n");
    if (Do_buttons(c, &answer, &theButtons) != S_NORM || answer == 7) {
	Do_send_clear(c);
	return;
    }
    Do_send_clear(c);
    switch(answer) {

        /* Prefix player */
    case 0:
	Do_create_tag(c, T_PREFIX);
	return;

        /* Suffix player */
    case 1:
	Do_create_tag(c, T_SUFFIX);
	return;

	/* kill player */
    case 2:
	Do_create_tag(c, T_SUICIDE);
	return;

	/*Clear or Add King or Valar information*/
    case 3:
    /* determine what the wizard wants to do */
    strcpy(theButtons.button[0], "Add King\n");
    strcpy(theButtons.button[1], "Add Valar\n");
	strcpy(theButtons.button[2], "Remove King\n");
    strcpy(theButtons.button[3], "Remove Valar\n");
    Do_clear_buttons(&theButtons, 4);
    Do_clear_buttons(&theButtons, 5);
    Do_clear_buttons(&theButtons, 6);
    strcpy(theButtons.button[7], "Cancel\n");

    Do_send_line(c, "Which would you like to clear?\n");
    if (Do_buttons(c, &answer, &theButtons) != S_NORM || answer == 7) {
	Do_send_clear(c);
	return;
    }
    Do_send_clear(c);
    switch(answer) {


	/*shall we add a king*/
	case 0:
	
		Do_send_line(c, "You never did this one Arella, silly girl\n");
		
	break;   


	/*or shall we add a Valar*/
	case 1:
	
    if (Do_string_dialog(c, characterName, SZ_NAME - 1,
	    "What is the name of the character to be named Valar?\n") != S_NORM) {

	return;
    }
        
        
		Do_lock_mutex(&c->realm->realm_lock);

			c->realm->valar = c->game;
			strcpy(c->realm->valar_name, characterName);

        Do_unlock_mutex(&c->realm->realm_lock);

        Do_send_line(c, "In the words of Jean Luc Picard, it is so.\n");
        return;

break;   

        /* Shall we remove the king?*/
    case 2:

        Do_lock_mutex(&c->realm->realm_lock);


	c->realm->king = NULL;
	c->realm->king_flag = FALSE;
	c->realm->king_name[0] = '\0';

        Do_unlock_mutex(&c->realm->realm_lock);

        Do_send_line(c, "The king has now been removed from office...No there was no intern involved.\n");
        return;
        
break;   
        
     /*Or shall we remove the Valar?*/
     case 3:

        Do_lock_mutex(&c->realm->realm_lock);


	c->realm->valar = NULL;
	c->realm->valar_name[0] = '\0';

        Do_unlock_mutex(&c->realm->realm_lock);

        Do_send_line(c, "Bye Bye Valar... We're going to miss you so.\n");
        return;
        
break;
  
  
        
}      
           

	/* remove a tag */
    case 4:
	Do_untag(c);
	return;

	/* restore a character */
    case 5:

	Do_restore_character(c);
	return;

    default:
        sprintf(error_msg, "[%s] Returned non-option in Do_modify.\n",
                c->connection_id);
 
        Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_rest(struct client_t *c)
/
/ FUNCTION: rest
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/19/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_rest(struct client_t *c)
{
    float addMana;

    if (c->player.type == C_DWARF) { 
    Do_energy(c, 
            c->player.energy + (c->player.max_energy + c->player.shield)
	    / 12.0 + c->player.level / 3.0 + 2.0, c->player.max_energy,
	    c->player.shield, c->battle.force_field, FALSE);
    } else {
    Do_energy(c, 
            c->player.energy + (c->player.max_energy + c->player.shield)
	    / 15.0 + c->player.level / 3.0 + 2.0, c->player.max_energy,
	    c->player.shield, c->battle.force_field, FALSE);
    }

        /* cannot find mana if cloaked */
    if (!c->player.cloaked) {

	addMana = floor((c->player.circle + c->player.level) / 5.0);

	if (addMana < 1.0) {
	    addMana = 1.0;
	}

	Do_mana(c, addMana, FALSE);
    }

	/* remove combat fatigue */
    if (c->battle.rounds > 0) {

	c->battle.rounds -= 30;
	if (c->battle.rounds < 0) {
	    c->battle.rounds = 0;
	}

	Do_speed(c, c->player.max_quickness, c->player.quicksilver,
		c->battle.speedSpell, FALSE);
    }

}


/************************************************************************
/
/ FUNCTION NAME: Do_energy_void(struct client_t *c)
/
/ FUNCTION: energy void
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/19/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_energy_void(struct client_t *c)
{
    struct event_t *event_ptr;

    Do_send_line(c, "You've hit an energy void!\n");

    Do_more(c);

    Do_mana(c, -2.0 * c->player.mana / 3.0, FALSE);
    Do_gold(c, -c->player.gold / 5, FALSE);
    Do_energy(c, c->player.energy / 2.0 + 1.0, c->player.max_energy * .99,
	    c->player.shield, 0, FALSE);

    Do_check_weight(c);

    event_ptr = (struct event_t *) Do_create_event();
    event_ptr->type = MOVE_EVENT;
    event_ptr->arg3 = A_NEAR;
    Do_handle_event(c, event_ptr);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_trading_post(struct client_t *c)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 6/17/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/       Different trading posts have different items.
/       Merchants cannot be cheated, but they can be dishonest
/       themselves.
/
/       Shields, swords, and quicksilver are not cumulative.  This is
/       one major area of complaint, but there are two reasons for this:
/               1) It becomes MUCH too easy to make very large versions
/                  of these items.
/               2) In the real world, one cannot simply weld two swords
/                  together to make a bigger one.
/
/       At one time, it was possible to sell old weapons at half the purchase
/       price.  This resulted in huge amounts of gold floating around,
/       and the game lost much of its challenge.
/
/       Also, purchasing gems defeats the whole purpose of gold.  Gold
/       is small change for lower level players.  They really shouldn't
/       be able to accumulate more than enough gold for a small sword or
/       a few books.  Higher level players shouldn't even bother to pick
/       up gold, except maybe to buy mana once in a while.
/
*************************************************************************/

Do_trading_post(struct client_t *c)
{
    struct button_t theButtons;
    struct button_t theItems;
    double  numitems;       /* number of items to purchase */
    double  cost;           /* cost of purchase */
    double  blessingcost;   /* cost of blessing */
    double  amuletcost;     /* cost of amulet */
    char    ch;             /* input */
    int     size;   /* size of the trading post */
    int     loop;   /* loop counter */
    long itemp;
    float ftemp;
    int     cheat = 0;      /* number of times player has tried to cheat */
    bool    dishonest = FALSE;   /* set when merchant is dishonest */
    bool    thief = FALSE;   /* set when player is too rich for his level */
    int     spent = 0;       /* total amount of gold spent on this visit */
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE];
    struct event_t *event_ptr;
    long answer;              /* pointer to option description */

    size = sqrt(fabs(c->player.x) / 100) + 1;

        /* only give blessings in the Cracks */
    if (size != 6) {
      size = MIN(6, size);
    } else {
      size = 7;
    }

        /* set up cost of variable items */
    blessingcost = 500.0 * (c->player.level + 5.0) + 10000;
    amuletcost = 250.0 + floor(c->player.level / 5);

    Do_clear_buttons(&theItems, 0);

    for (loop = 0; loop < size; ++loop) {

        sprintf(string_buffer, "%s\n", c->realm->shop_item[loop].item);
	strcpy(theItems.button[loop], string_buffer);
    }

    strcpy(theItems.button[7], "Cancel\n");

    for (;;) {

	    /* print Menu */
        for (loop = 0; loop < size; ++loop) {

                /* print Menu */
	    if (loop == 6)
                cost = blessingcost;
	    else if (loop == 3) 
                cost = amuletcost;
            else
                cost = c->realm->shop_item[loop].cost;

	    sprintf(string_buffer, "%-12s: %6.0f\n",
	            c->realm->shop_item[loop].item, cost);

	    Do_send_line(c, string_buffer);
        }

        Do_send_line(c,
	 "You are at a trading post. All purchases must be made with gold.\n");
/*
	Do_send_line(c, "\n");
*/
            /* print some important statistics */
/*
        sprintf(string_buffer,
		"Gold:   %9.0f  Gems:  %9.0f  Level:   %6.0f  Charms: %6d\n",
                c->player.gold, c->player.gems, c->player.level,
		c->player.charms);

	Do_send_line(c, string_buffer);

        sprintf(string_buffer,
	       "Shield: %9.0f  Sword: %9.0f  Quicksilver:%3.0f  Blessed: %s\n",
               c->player.shield, c->player.sword, c->player.quicksilver,
               (c->player.blessing ? " True" : "False"));

	Do_send_line(c, string_buffer);

        sprintf(string_buffer, "Brains: %9.0f  Mana:  %9.0f", c->player.brains,
		c->player.mana);

	Do_send_line(c, string_buffer);
*/
	strcpy(theButtons.button[0], "Purchase\n");
	strcpy(theButtons.button[1], "Sell Gems\n");
	strcpy(theButtons.button[2], "Leave\n");
	Do_clear_buttons(&theButtons, 3);

	if (Do_buttons(c, &itemp, &theButtons) != S_NORM) {
	    Do_send_clear(c);
	    return;
	}

	Do_send_clear(c);
        switch(itemp) {

        case 2:           /* leave */
            return;

        case 0:           /* make purchase */


            Do_send_line(c, "What would you like to buy?\n");
            if (Do_buttons(c, &itemp, &theItems) != S_NORM) {
	        Do_send_clear(c);
		return;
	    }

	    Do_send_clear(c);
            if ((itemp > size) && (itemp != 7) || itemp < 0) {

		sprintf(error_msg,
			"[%s] Returned non-option in Do_trading_post.\n",
			c->connection_id);

		Do_log_error(error_msg);
	        Do_caught_hack(c, H_SYSTEM);
		return;
	    }
            else {
                switch (itemp) {

                case 0:
                    sprintf(string_buffer,
	"Mana is one per %.0f gold piece.  How many do you want (%.0f max)?\n",
                    	    c->realm->shop_item[0].cost,
			    floor(c->player.gold /
			    c->realm->shop_item[0].cost));

		    if (Do_double_dialog(c, &numitems, string_buffer)) {
		        Do_send_clear(c);
			break;
		    }

                    cost = numitems * c->realm->shop_item[0].cost;

                    if (cost > c->player.gold || numitems < 0)
                        ++cheat;

                    else {
                        cheat = 0;
			Do_gold(c, -cost, FALSE);
                        spent += cost;

			sprintf(string_buffer, "%s, %s, purchased %lf mana\n",
            		        c->player.lcname,
				c->realm->charstats[c->player.type].class_name,
				numitems);

			Do_log(PURCHASE_LOG, &string_buffer);

                        if (RND() < 0.02)
                            dishonest = TRUE;
                        else
			    Do_mana(c, numitems, FALSE);
                    }
                    break;

                case 1:

                    sprintf(string_buffer, "Shields are %.0f gold per +1.\n",
			    c->realm->shop_item[1].cost);

		    Do_send_line(c, "Each plus will add to your energy.\n");

                    sprintf(string_buffer,
			    "How strong a shield do you want (%.0f max)?\n",
			    floor(c->player.gold /
			    c->realm->shop_item[1].cost));

		    if (Do_double_dialog(c, &numitems, string_buffer)) {
			Do_send_clear(c);
			break;
		    }

		    Do_send_clear(c);
                    cost = numitems * c->realm->shop_item[1].cost;

                    if (numitems == 0.0)
                        break;
                    else if (cost > c->player.gold || numitems < 0)
                        ++cheat;
                    else if (numitems < c->player.shield) {

                        Do_send_line(c,
			     "That's no better than what you already have.\n");

			Do_more(c);
			Do_send_clear(c);
		    }
                    else {
                        cheat = 0;
			Do_gold(c, -cost, FALSE);
                        spent += cost;

                        sprintf(string_buffer, "%s, %s, purchased %lf shield\n",
                                c->player.lcname,
                                c->realm->charstats[c->player.type].class_name,
                                numitems);

                        Do_log(PURCHASE_LOG, &string_buffer);

                        if ((c->player.level < 20) && 
                             (RND() < 1 -
                                      (c->player.level * 350 + 150) / spent) )
                            thief = TRUE;
                        else if (RND() < 0.02) 
                            dishonest = TRUE;
                        else
			    Do_energy(c, c->player.energy - c->player.shield
				    + numitems, c->player.max_energy, numitems,
				    0, FALSE);
                    }
                    break;

                case 2:

                    if (c->player.blind) {
                        Do_send_line(c, "You can't read books while blind!\n");
                        Do_more(c);
                        Do_send_clear(c);
                        break;
                    }

                    sprintf(string_buffer,
	  	   "A book costs %.0f gp.  How many do you want (%.0f max)?\n",
                   c->realm->shop_item[2].cost, floor(c->player.gold /
		   c->realm->shop_item[2].cost));

		    if (Do_double_dialog(c, &numitems, string_buffer)) {
		        Do_send_clear(c);
			break;
		    }

		    Do_send_clear(c);
                    cost = numitems * c->realm->shop_item[2].cost;

                    if (cost > c->player.gold || numitems < 0)
                        ++cheat;

                    else {
                        cheat = 0;
			Do_gold(c, -cost, FALSE);
                        spent += cost;

                        sprintf(string_buffer, "%s, %s, purchased %lf books\n",
                                c->player.lcname,
                                c->realm->charstats[c->player.type].class_name,
                                numitems);

                        Do_log(PURCHASE_LOG, &string_buffer);

                        if ((c->player.level < 20) && 
                             (RND() < 1 -
                                      (c->player.level * 350 + 150) / spent) )
                            thief = TRUE;
                        else if (RND() < 0.02) 
                            dishonest = TRUE;
                        else {
			    Do_book(c, (int) numitems);
			}
                    }
                    break;

                case 3:

                    sprintf(string_buffer, 
		"An amulet costs %.0f gp.  How many do you want (%.0f max)?\n",
                amuletcost, floor(c->player.gold /
		amuletcost));

		    if (Do_double_dialog(c, &numitems, string_buffer)) {
			Do_send_clear(c);
			break;
		    }

		    Do_send_clear(c);
                    cost = numitems * amuletcost;

                    if (cost > c->player.gold || numitems < 0)
                        ++cheat;
                    else {
                        cheat = 0;
                        Do_gold(c, -cost, FALSE);
                        spent += cost;

                        sprintf(string_buffer,
				"%s, %s, purchased %lf amulets\n",
                                c->player.lcname,
                                c->realm->charstats[c->player.type].class_name,
                                numitems);

                        Do_log(PURCHASE_LOG, &string_buffer);

                        if (RND() < 0.02)
                            dishonest = TRUE;
                        else {
                            c->player.amulets += numitems;
			}
                    }
                    break;

                case 4:

		    sprintf(string_buffer, "Swords are %.0f gold per +1.\n",
			    c->realm->shop_item[3].cost);

		    Do_send_line(c, "Each plus will improve your fighting ability.\n");
		    
                    sprintf(string_buffer,
			    "How strong a sword do you want (%.0f max)?\n",
            		    floor(c->player.gold /
                            c->realm->shop_item[4].cost));

		    if (Do_double_dialog(c, &numitems, string_buffer)) {
			Do_send_clear(c);
			break;
		    }

		    Do_send_clear(c);
                    cost = numitems * c->realm->shop_item[4].cost;

                    if (numitems == 0.0)
                        break;
                    else if (cost > c->player.gold || numitems < 0)
                        ++cheat;
                    else if (numitems < c->player.sword) {

                        Do_send_line(c,
			     "That's no better than what you already have.\n");

			Do_more(c);
			Do_send_clear(c);
		    }
                    else {
                        cheat = 0;
                        Do_gold(c, -cost, FALSE);
                        spent += cost;

                        sprintf(string_buffer, "%s, %s, purchased %lf sword\n",
                                c->player.lcname,
                                c->realm->charstats[c->player.type].class_name,
                                numitems);

                        Do_log(PURCHASE_LOG, &string_buffer);

                        if ((c->player.level < 20) && 
                             (RND() < 1 -
                                      (c->player.level * 350 + 150) / spent) )
                            thief = TRUE;
                        else if (RND() < 0.02) 
                            dishonest = TRUE;
                        else
			    Do_strength(c, c->player.max_strength, numitems, 0,
				    FALSE);
                    }
                    break;

                case 5:
                    sprintf(string_buffer,
	"Quicksilver is %.0f gp per +1.  How many + do you want (%.0f max)?\n",
        c->realm->shop_item[5].cost, floor(c->player.gold /
	c->realm->shop_item[5].cost));

		    if (Do_double_dialog(c, &numitems, string_buffer)) {
		        Do_send_clear(c);
			break;
		    }

		    Do_send_clear(c);
                    cost = numitems * c->realm->shop_item[5].cost;

                    if (numitems == 0.0)
                        break;
                    else if (cost > c->player.gold || numitems < 0)
                        ++cheat;
                    else if (numitems < c->player.quicksilver) {

                        Do_send_line(c,
			     "That's no better than what you already have.\n");

			Do_more(c);
		        Do_send_clear(c);
		    }
                    else {
                        cheat = 0;
                        Do_gold(c, -cost, FALSE);
                        spent += cost;

                        sprintf(string_buffer,
				"%s, %s, purchased %lf quicksilver\n",
                                c->player.lcname,
                                c->realm->charstats[c->player.type].class_name,
                                numitems);

                        Do_log(PURCHASE_LOG, &string_buffer);

                        if ((c->player.level < 20) && 
                             (RND() < 1 -
                                      (c->player.level * 350 + 150) / spent) )
                            thief = TRUE;
                        else if (RND() < 0.02) 
                            dishonest = TRUE;
                        else
			    Do_speed(c, c->player.max_quickness, numitems, 0,
				    FALSE);
                    }
                    break;

                case 6:
                    if (c->player.blessing) {
                        Do_send_line(c, "You already have a blessing.\n");
			Do_more(c);
		        Do_send_clear(c);
                        break;
                    }

                    sprintf(string_buffer,
	          "A blessing requires a %.0f gp donation.  Still want one?\n",
		  blessingcost);

		    Do_send_line(c, string_buffer);
		    
	            if (Do_yes_no(c, &answer) == S_NORM && answer == 0) {

                        if (c->player.gold < blessingcost)
                            ++cheat;
                        else {
                            cheat = 0;
                            Do_gold(c, -blessingcost, FALSE);
                            spent += cost;

                            sprintf(string_buffer,
				"%s, %s, purchased 1 blessing, %lf\n",
                                c->player.lcname,
                                c->realm->charstats[c->player.type].class_name,
                                blessingcost);

                            Do_log(PURCHASE_LOG, &string_buffer);

                            if (RND() < 0.02)
                                dishonest = TRUE;
                            else
				Do_blessing(c, TRUE, FALSE);
                        }
		    }
		    Do_send_clear(c);
                    break;
                }

 		case 7:
                    break;

	    }
            break;

            case 1:           /* sell gems */


                sprintf(string_buffer,
	 "A gem is worth %.0f gp.  How many do you want to sell (%.0f max)?\n",
         (double) N_GEMVALUE, c->player.gems);

		if (Do_double_dialog(c, &numitems, string_buffer)) {
		    Do_send_clear(c);
		    break;
		}

		Do_send_clear(c);
                if (numitems > c->player.gems || numitems < 0) {
                    ++cheat;
                } else if (numitems > c->player.level * c->player.level + 1) {
                    dishonest = TRUE;
                } else {
                    cheat = 0;

			/* add gold manually to prevent taxes */
                    if (numitems > 0) {
		        Do_gems(c, -numitems, FALSE);
                        c->player.gold += numitems * N_GEMVALUE;
		        Do_gold(c, 0, TRUE);
		        Do_check_weight(c);
		    }
                }
	        break;
            }

        if (cheat == 1) {

		/* give the player some sin */
	    Do_sin(c, .25);

            Do_send_line(c,
		    "Come on, merchants aren't stupid.  Stop cheating.\n");

	    Do_more(c);
	    Do_send_clear(c);
	}
        else if (cheat == 2) {

		/* give the player more sin */
	    Do_sin(c, .5);

            Do_send_line(c,
		    "You had your chance.  This merchant happens to be\n");

            sprintf(string_buffer,
		    "a %.0f level magic user, and you made %s mad!\n",
                    ROLL(c->player.circle * 20.0, 40.0), (RND() <
		    0.5) ? "him" : "her");

	    Do_send_line(c, string_buffer);
	    Do_more(c);
	    Do_send_clear(c);

            /* prevent experimentos from using a post 
               to bounce really far out */
	    event_ptr = (struct event_t *) Do_create_event();

            if (c->player.type != C_EXPER) {
	        event_ptr->arg3 = A_FAR;
            } else {
	        event_ptr->arg3 = A_NEAR;
            }
	    event_ptr->type = MOVE_EVENT;
	    Do_handle_event(c, event_ptr);

	    Do_energy(c, c->player.energy / 2.0 + 1.0, c->player.max_energy,
		    c->player.shield, 0, FALSE);

            return;
        }
        else if (thief) {
	    event_ptr = (struct event_t *) Do_create_event();
	    event_ptr->type = MOVE_EVENT;
	    event_ptr->arg1 = floor(2350 + RND() * 100);
	    event_ptr->arg2 = floor(2350 + RND() * 100);
	    event_ptr->arg3 = A_SPECIFIC;

            Do_send_line(c, "The merchant wonders, 'How come a pipsqueak like you has so much gold?\n");
            Do_send_line(c, "The merchant cries, 'A thief!  Guards!  Throw this miscreant in the Cracks of Doom!\n");
            sprintf(string_buffer, "%s, %s, was called a thief.\n",
                                c->player.lcname,
                                c->realm->charstats[c->player.type].class_name);

            Do_log(PURCHASE_LOG, &string_buffer);

	    Do_handle_event(c, event_ptr);

            return;
        }
        else if (dishonest) {
            Do_send_line(c, "The merchant stole your money and teleported you!\n");
            sprintf(string_buffer, "%s, %s, was cheated.\n",
                                c->player.lcname,
                                c->realm->charstats[c->player.type].class_name);

            Do_log(PURCHASE_LOG, &string_buffer);

	    Do_more(c);
	    Do_send_clear(c);
	    event_ptr = (struct event_t *) Do_create_event();
	    event_ptr->type = MOVE_EVENT;
	    event_ptr->arg1 = floor(1.25 * c->player.x);
	    event_ptr->arg2 = floor(1.25 * c->player.y);
	    event_ptr->arg3 = A_SPECIFIC;
	    Do_handle_event(c, event_ptr);
            return;
        }
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_teleport(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getanswer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_teleport(struct client_t *c, struct event_t *the_event)
{
    struct event_t *event_ptr;
    struct event_t *event2_ptr;
    double distance, mana = 0;
    double dtemp;
    int tempcircle;
    char string_buffer[SZ_LINE];
    long answer;

    event_ptr = (struct event_t *) Do_create_event();
    event_ptr->type = MOVE_EVENT;
    event_ptr->arg3 = A_FORCED;

    if (Do_coords_dialog(c, &event_ptr->arg1, &event_ptr->arg2,
	    "Where do you wish to teleport to?\n")) {

	free((void *)event_ptr);
	return;
    }

    Do_distance(event_ptr->arg1, 0.0, event_ptr->arg2, 0.0, &distance);
    tempcircle = floor(distance / D_CIRCLE + 1);

        /* block non-Gwaihir teleports into the Marshes/Cracks */
    if (the_event->arg2 == FALSE &&
        (tempcircle > 19 && tempcircle < 36)) {
        Do_send_line(c, "You try to teleport into the Marshes, but a mysterious force prevents you!\n");
	Do_more(c);
	Do_send_clear(c);
        return;
    }
    
	/* did the player teleport himself? */
    if (the_event->arg3 == TRUE) {

	Do_distance(c->player.x, event_ptr->arg1, c->player.y,
		event_ptr->arg2, &distance);

	mana = ceil(distance * distance / (30 * c->player.magiclvl));

	    /* make sure player has enough mana */
	if (mana > c->player.mana) {

	    free((void *) event_ptr);

	    sprintf(string_buffer,
	       "You do not have the %.0lf mana that teleport would require.\n",
	       mana);

	    Do_send_line(c, string_buffer);
	    Do_more(c);
	    Do_send_clear(c);
	    return;
	}
	else {
	    
	    sprintf(string_buffer,
	      "That teleport will cost %.0lf mana.  Do you wish to cast it?\n",
	      mana);

	    Do_send_line(c, string_buffer);

            if (Do_yes_no(c, &answer) != S_NORM || answer == 1) {
	        free((void *) event_ptr);
	        Do_send_clear(c);
                return;
            }
	}

	Do_mana(c, -mana, FALSE);
	event_ptr->arg3 = A_TELEPORT;
    }

 

    Do_handle_event(c, event_ptr);

        /* did the player use Gwaihir? */
    if (the_event->arg2 == TRUE) {

           /* is the destination a post? */
        if (event_ptr->arg1 == event_ptr->arg2) {
           
            dtemp = sqrt(fabs(event_ptr->arg1)/100.0);

            if (floor(dtemp) == dtemp) {

                   /* bypass the monster check */
                event2_ptr = (struct event_t *) Do_create_event();
                event2_ptr->type = TRADING_EVENT;
                Do_handle_event(c, event2_ptr);
            }
        }
    }
        
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_holy_grail(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: check if current player has been tampered with
/
/ AUTHOR: E. A. Estes, 12/4/85
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: readrecord(), fread(), fseek(), tampered(), writevoid()
/
/ GLOBAL INPUTS: *Energyvoidfp, Other, Player, Fileloc, Enrgyvoid
/
/ GLOBAL OUTPUTS: Enrgyvoid
/
/ DESCRIPTION:
/       Check for energy voids, holy grail, and tampering by other
/       players.
/
*************************************************************************/

Do_holy_grail(struct client_t *c, struct event_t *the_event)
{
    struct event_t *event_ptr;
    float ftemp;

    Do_send_line(c, "You have found The Holy Grail!!\n");

        /* must be council of wise to behold grail */
    if (c->player.special_type < SC_COUNCIL) {

        Do_send_line(c,
		"However, you are not experienced enough to behold it.\n");

	Do_more(c);
	Do_sin(c, (double) c->player.sin);
	Do_mana(c, 1000, FALSE);
    }
    else if (c->player.special_type == SC_VALAR) {

        Do_send_line(c,
		"You have already made it to the position of Valar.\n");

	Do_send_line(c, "The Grail is of no use to you now.\n");
	Do_more(c);
    }
    else {

        Do_send_line(c,
	       "It is now time to see if you are worthy to behold it. . .\n");

	Do_send_buffer(c);
        sleep(4);

        if ((RND() / 4.0 < c->player.sin) || 
            (strcmp(c->player.parent_account, c->lcaccount))){

            Do_send_line(c, "You are unworthy!\n");
	    Do_more(c);

	    Do_energy(c, 1.0, 1.0 + c->player.shield, c->player.shield, 0.0,
		    FALSE);

	    Do_strength(c, 1.0, c->player.sword, 0.0, FALSE);
	    Do_speed(c, 1.0, c->player.quicksilver, 0.0, FALSE);
	    Do_mana(c, -(c->player.mana - 1), FALSE);
	    Do_experience(c, -(c->player.experience), FALSE);
	    
            c->player.magiclvl =
            c->player.brains = 0;

            event_ptr = (struct event_t *) Do_create_event();
            event_ptr->type = MOVE_EVENT;
	    event_ptr->arg1 = 1.0;
	    event_ptr->arg2 = 1.0;
            event_ptr->arg3 = A_FORCED;
            Do_file_event(c, event_ptr);
        }
        else {

		/* send yourself a valar event */
            event_ptr = (struct event_t *) Do_create_event();
            event_ptr->type = VALAR_EVENT;
            Do_file_event(c, event_ptr);

	    Do_lock_mutex(&c->realm->realm_lock);
	    if (c->realm->valar != NULL) {
                event_ptr = (struct event_t *) Do_create_event();
                event_ptr->type = VALAR_EVENT;
                event_ptr->to = c->realm->valar;
                Do_send_event(event_ptr);
	    }
	    Do_unlock_mutex(&c->realm->realm_lock);
        }
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_help(struct client_t *c)
/
/ FUNCTION: check if current player has been tampered with
/
/ AUTHOR: E. A. Estes, 12/4/85
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: readrecord(), fread(), fseek(), tampered(), writevoid()
/
/ GLOBAL INPUTS: *Energyvoidfp, Other, Player, Fileloc, Enrgyvoid
/
/ GLOBAL OUTPUTS: Enrgyvoid
/
/ DESCRIPTION:
/       Check for energy voids, holy grail, and tampering by other
/       players.
/
*************************************************************************/

Do_help(struct client_t *c)
{

    Do_send_line(c, "You move about in the realm, fighting monsters, and gaining treasure in order to gain levels.  Your first priority at this time is to stay alive.  Later goals may be to become king, and eventually to become Council of the Wise, and seek the holy grail.  When you leave, your game is automatically saved, but you cannot restore, once you die, you are dead and must re-roll a new char.\n");

    Do_more(c);
    Do_send_clear(c);

    Do_send_line(c, "Fight by using melee or skirmish, or use your magic by choosing spells (although spells use mana).  Mana, your magical energy, can be restored by resting.  Plague and curses lower your energy and strength but can be cured by gurus and medics.  The medic will want about half of your gold, to cure you.  Amulets and charms protect from curses.  Players will encounter each other whenever they are at the same coordinates.\n");

    Do_more(c);
    Do_send_clear(c);

    Do_send_line(c, "The first set of posts can be found at (100,100), (-100,100), (100,-100) and (-100,-100).  The second set are out at (+/- 400, +/- 400).  Posts sell things, and you get close by using the 'compass' keys on the bottom right side of the screen and get in to them by using the 'move to' key on the left side.  There, you can buy shields, swords, and additional sundries which make life more pleasant in the realm.  Higher the post, more it sells.\n");

    Do_more(c);
    Do_send_clear(c);

    Do_send_line(c, "Some players are Game Wizards, with a W after their class.  Treat them with respect, for they administer the game and make sure it runs properly.  Do not pester Wizards to help you in your quest unless the game has performed incorrectly, as they tend to get extremely angry and tend to do nasty things to players that annoy them.  Above all, enjoy the game!\n"); 
    Do_more(c);
    Do_send_clear(c);

    /* don't penalize the player for asking for help */
    c->player.age--;
}

