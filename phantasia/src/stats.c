/*
 * stats.c  Phantasia routines player stat manipulation
 */

#include "include.h"

/************************************************************************
/
/ FUNCTION NAME: Do_init_player(struct client_t *c)
/
/ FUNCTION: initialize a character
/
/ AUTHOR: E. A. Estes, 12/4/85
/         Brian Kelly, 5/4/99
/
/ ARGUMENTS:
/       struct player *playerp - pointer to structure to init
/
/ RETURN VALUE: none
/
/ MODULES CALLED: floor(), drandom()
/
/ DESCRIPTION:
/       Put a bunch of default values in the given structure.
/
*************************************************************************/

Do_init_player(struct client_t *c)
{
    c->player.name[0] =
    c->player.lcname[0] =
    c->modifiedName[0] =
    c->player.password[0] =
    c->player.area[0] = '\0';
    c->player.last_load = 0;

        /* initialize doubles */
    c->player.x =
    c->player.y =
    c->player.circle =
    c->player.brains =
    c->player.magiclvl =

        /* initialize floats */
    c->player.poison =
    c->player.sin =

        /* initialize ints and shorts */
    c->player.location =
    c->player.age =
    c->player.lives =
    c->player.holywater =
    c->player.amulets =
    c->player.charms = 0;

        /* initialize booleans */
    c->player.beyond =
    c->player.cloaked = 
    c->player.blind =
    c->player.purgatoryFlag = FALSE;
    c->player.gender = MALE;

        /* specifics */
    c->player.degenerated = 1;                 /* don't degenerate initially */

    c->player.type = C_FIGHTER;                /* default */
    c->player.special_type = SC_NONE;

    c->player.shield_nf =
    c->player.haste_nf =
    c->player.strong_nf = FALSE;

        /* reset the client stat panel */
    Do_name(c);

    c->player.level = 0.0;
    c->player.experience = 0.0;
    Do_experience(c, 0.0, TRUE);

    Do_energy(c, 0.0, 0.0, 0.0, 0.0, FALSE);
    Do_strength(c, 0.0, 0.0, 0.0, FALSE);
    Do_speed(c, 0.0, 0.0, 0.0, FALSE);

    c->player.mana = 0.0;
    Do_mana(c, 0.0, TRUE);

    c->player.gold = 0.0;
    Do_gold(c, 0.0, TRUE);

    c->player.gems = 0.0;
    Do_gems(c, 0.0, TRUE);

    Do_cloak(c, FALSE, TRUE);

    c->player.crowns = 0.0;
    Do_crowns(c, 0, TRUE);

    Do_blessing(c, FALSE, FALSE);
    Do_palantir(c, FALSE, FALSE);
    Do_virgin(c, FALSE, FALSE);
    Do_ring(c, R_NONE, FALSE);

	/* set user specific variables */
    strcpy(c->player.parent_account, c->lcaccount);
    strcpy(c->player.parent_network, c->network);
    c->player.date_created = 0;
    c->player.faithful = TRUE;

	/* historic information */
    c->player.bad_passwords =
    c->player.muteCount =
    c->player.load_count =
    c->player.last_reset = 
    c->player.time_played = 0;

        /* set a few client variables to false */
    c->wizard =
    c->stuck = FALSE;

        /* specific client variables */
    c->timeout = 120;
}


/************************************************************************
/
/ FUNCTION NAME: Do_update_stats(struct client_t *c)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 11/1/99
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

Do_update_stats(struct client_t *c) 
{
	/* send the player's name */
    Do_name(c);
    Do_experience(c, 0.0, TRUE);

    Do_energy(c, c->player.energy, c->player.max_energy, c->player.shield,
	    c->battle.force_field, TRUE);

    Do_strength(c, c->player.max_strength, c->player.sword,
	    c->battle.strengthSpell, TRUE);

    Do_speed(c, c->player.max_quickness, c->player.quicksilver,
	    c->battle.speedSpell, TRUE);

    Do_mana(c, 0.0, TRUE);
    Do_gold(c, 0.0, TRUE);
    Do_gems(c, 0.0, TRUE);

    Do_cloak(c, c->player.cloaked, TRUE);
    Do_blessing(c, c->player.blessing, TRUE);
    Do_crowns(c, 0, TRUE);
    Do_palantir(c, c->player.palantir, TRUE);
    Do_ring(c, c->player.ring_type, TRUE);
    Do_virgin(c, c->player.virgin, TRUE);
}


/************************************************************************
/
/ FUNCTION NAME: Do_name(struct client_t *c)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/2/99
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

Do_name(struct client_t *c) 
{
	/* send the name */
    Do_send_int(c, NAME_PACKET);
    Do_send_string(c, c->modifiedName);
    Do_send_string(c, "\n");

    Do_lock_mutex(&c->realm->realm_lock);
    Do_location(c, c->player.x, c->player.y, TRUE);
    Do_unlock_mutex(&c->realm->realm_lock);
}


/************************************************************************
/
/ FUNCTION NAME: Do_location(struct client_t *c, double x, double y)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/2/99
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

Do_location(struct client_t *c, double x, double y, int force) 
{
    double distance;
/* Call this function with the realm locked if playing */

    x = floor(x);
    y = floor(y);

	/* check for changes */
    if (c->player.x != x || c->player.y != y || force) {

	Do_distance(x, 0.0, y, 0.0, &distance);
	c->player.circle = floor(distance / D_CIRCLE + 1);
	c->player.x = x;
	c->player.y = y;
	Do_name_location(c);

	Do_send_int(c, LOCATION_PACKET);
	Do_send_double(c, x);
	Do_send_double(c, y);
	Do_send_string(c, c->player.area);
	Do_send_string(c, "\n");

	if (c->run_level == PLAY_GAME) {
	    c->game->x = x;
	    c->game->y = y;
	    c->game->circle = c->player.circle;
	    strncpy(c->game->area, c->player.area, SZ_AREA);

	    if (c->player.location == PL_REALM) {
		c->game->useLocationName = FALSE;
	    }
	    else {
		c->game->useLocationName = TRUE;
	    }

	    if (c->wizard > 2) {
	        c->game->virtual = TRUE;
	    }
	    else {
	        c->game->virtual = FALSE;
	    }

	    Do_player_description(c);
	}
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_experience(struct client_t t, double theExpierence)
/
/ FUNCTION: move player to new level
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: death(), floor(), wmove(), drandom(), waddstr(), explevel()
/
/ GLOBAL INPUTS: Player, *stdscr, *Statptr, Stattable[]
/
/ GLOBAL OUTPUTS: Player, Changed
/
/ DESCRIPTION:
/       Use lookup table to increment important statistics when
/       progressing to new experience level.
/       Players are rested to maximum as a bonus for making a new
/       level.
/       Check for council of wise, and being too big to be king.
/
*************************************************************************/

Do_experience(struct client_t *c, double theExperience, int force)
{
    struct charstats_t *statptr;       /* for pointing into Stattable */
    struct event_t *event_ptr;
    double new;                    /* new level */
    double  inc;                    /* increment between new and old levels */
    char string_buffer[SZ_LINE];

	/* add the experience */
    c->player.experience += theExperience;

	/* determine the new level */
    new = CALCLEVEL(c->player.experience);
/*
    if (c->player.experience < 1.1e7)
        new = floor(pow((c->player.experience / 1000.0), 0.4748));
    else
        new = floor(pow((c->player.experience / 1250.0), 0.4865));
*/

    inc = new - c->player.level;

	/* if we've gone up any levels */
    if (inc > 0) {

	    /* make sure we send the level information */
	force = TRUE;
        c->player.level = new;

        if (c->player.type == C_EXPER)
            /* roll a type to use for increment */
            statptr = &c->realm->charstats[(int) ROLL(C_MAGIC,
		    (C_HALFLING - C_MAGIC + 1))];
        else
            statptr = &c->realm->charstats[c->player.type];

	    /* add increments to statistics */
	Do_energy(c, c->player.energy + statptr->energy.increase * inc,
		c->player.max_energy + statptr->energy.increase * inc,
		c->player.shield, c->battle.force_field, FALSE);

	Do_strength(c, c->player.max_strength + statptr->strength.increase
		* inc, c->player.sword, c->battle.strengthSpell, FALSE);

	Do_mana(c, statptr->mana.increase * inc, FALSE);

	c->player.brains += statptr->brains.increase * inc;

	c->player.magiclvl = c->player.magiclvl +
		statptr->magiclvl.increase * inc;

	    /* knights may get more energy */
	if (c->player.special_type == SC_KNIGHT) {

	    Do_energy(c, c->player.energy - c->knightEnergy +
		    floor(c->player.max_energy / 4), c->player.max_energy,
		    c->player.shield, c->battle.force_field, FALSE);

	    c->knightEnergy = floor(c->player.max_energy / 4);
	}

        if (c->player.level == 1000.0) {
            /* send congratulations message */
            Do_send_line(c, "Congratulations on reaching level 1000!  The throne awaits...\n");
	    Do_more(c);
	    Do_send_clear(c);
        }

        if (c->player.level >= MAX_STEWARD && 
            c->player.special_type == SC_STEWARD) {

		    /* no longer able to be steward -- dethrone */
	        Do_send_line(c,
		       "After level 200, you can no longer be steward.\n");

            Do_dethrone(c);
        }

        if (c->player.level >= MAX_KING && 
            c->player.special_type == SC_KING) {
		    /* no longer able to be king -- dethrone */
	        Do_send_line(c,
		        "After level 2000, you can no longer be king or queen.\n");
            Do_dethrone(c);
            c->player.special_type = SC_NONE;
        }

	    /* see if staves/crowns should be cashed in */
        if (c->player.crowns > 0) {

	    if (c->player.level >= MAX_KING) {


	        Do_send_line(c, "Your crowns were cashed in.\n");

                Do_gold(c, (c->player.crowns) * 5000.0, FALSE);
	        Do_crowns(c, -(c->player.crowns), FALSE);

                if (c->player.special_type == SC_KING) {
		    Do_dethrone(c);
                }

	        Do_more(c);
	        Do_send_clear(c);
	    }
	    else if (c->player.level >= MAX_STEWARD && c->player.level < 1000.0) {

	        Do_send_line(c, "Your staves were cashed in.\n");

                Do_gold(c, (c->player.crowns) * 1000.0, FALSE);
	        Do_crowns(c, -(c->player.crowns), FALSE);

                if (c->player.special_type == SC_STEWARD) {
		    Do_dethrone(c);
                }

	        Do_more(c);
	        Do_send_clear(c);
	    }
        }

        if (c->player.level >= 3000.0 && c->player.special_type < SC_COUNCIL) {

	        /* if by some chance this person is king or knight, dethrone him */
	    if (c->player.special_type == SC_KING || c->player.special_type == SC_KNIGHT) {
		Do_dethrone(c);
	    }

		/* announce the new member */
	    sprintf(string_buffer,
		  "The Council of the Wise announces its newest member, %s.\n",
		  c->modifiedName);

	    Do_broadcast(c, string_buffer);

        	/* make a member of the council */
	    Do_send_clear(c);
            Do_send_line(c, "You have made it to the Council of the Wise.\n");
            Do_send_line(c, "Good Luck on your search for the Holy Grail.\n");

            c->player.special_type = SC_COUNCIL;

        	/* no rings for council and above */
	    Do_ring(c, R_NONE, FALSE);

            c->player.lives = 2;             /* two extra lives */

	    Do_send_specification(c, CHANGE_PLAYER_EVENT);

	    Do_more(c);
        }

	    /* death from old age */
        if ((c->player.level > 9999.0 - (10 * c->player.degenerated)) && 
            (c->player.special_type != SC_VALAR)) {

            event_ptr = (struct event_t *) Do_create_event();
	    event_ptr->type = DEATH_EVENT;
	    event_ptr->arg3 = K_OLD_AGE;
	    event_ptr->to = c->game;
	    Do_file_event(c, event_ptr);
	}

	    /* player might gain some quickness */
	Do_check_weight(c);

	    /* update player description */
	Do_lock_mutex(&c->realm->realm_lock);
	Do_player_description(c);
	Do_unlock_mutex(&c->realm->realm_lock);

	    /* recalculate the player's timeout */
        if (c->wizard > 2) {
            c->timeout = 900;
        }
        else if (c->player.level < 80) {
	    c->timeout = 60 - c->player.level / 2;
	}
	else {
	    c->timeout = 20;
	}

	    /* log the increase */
	sprintf(string_buffer, "%s, %s, %d age, %d seconds, level %0.0lf\n",
		c->player.lcname,
		c->realm->charstats[c->player.type].class_name,
		c->player.age, c->player.time_played + time(NULL) -
		c->player.last_load, c->player.level);

        Do_log(LEVEL_LOG, &string_buffer);

            /* backup up this character in case of crash */
        Do_backup_save(c, TRUE);

    }

    if (force) {

	    /* send the level increase */
        Do_send_int(c, LEVEL_PACKET);
        Do_send_double(c, c->player.level);
    }
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_energy(struct client_t *c, double energy, double maxEnergy, double shield)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/2/99
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

Do_energy(struct client_t *c, double energy, double maxEnergy, double shield, double forceShield, int force) 
{
    maxEnergy = floor(maxEnergy);
    shield = floor(shield);
    energy = floor(energy);
    forceShield = floor(forceShield);

    if (maxEnergy < 0) {
	maxEnergy = 0;
    }

    if (energy > maxEnergy + shield + c->knightEnergy) {
        energy = maxEnergy + shield + c->knightEnergy;
    }

	/* check for changes */
    if (c->player.energy != energy || c->player.max_energy != maxEnergy ||
	    c->player.shield != shield || c->battle.force_field !=
	    forceShield || force) {

	Do_send_int(c, ENERGY_PACKET);
	Do_send_double(c, energy);
	Do_send_double(c, maxEnergy + shield + c->knightEnergy);
	Do_send_double(c, forceShield);

	c->player.energy = energy;
	c->player.max_energy = maxEnergy;
	c->battle.force_field = forceShield;

	if (c->player.shield != shield || force) {

	    Do_send_int(c, SHIELD_PACKET);
	    Do_send_double(c, shield);

	    c->player.shield = shield;
	}
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_strength(struct client_t *c, double strength, double maxStrength, double sword, double strengthSpell)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/2/99
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

Do_strength(struct client_t *c, double maxStrength, double sword, double strengthSpell, int force)
{
    double strength;

        /* a player can have a minimum of 0 strength */
    if (maxStrength < 0) {
	maxStrength = 0;
    }

        /* calculate strength based on poison */
    strength = 1.0 - c->player.poison *
	    c->realm->charstats[c->player.type].weakness / 800.0;

    if (strength > 1.0) {
	strength = 1.0;
    }

    if (strength < .1) {
	strength = .1;
    }

    strength = maxStrength * strength;

	/* check for changes */
    if (c->player.strength != strength || c->player.max_strength !=
	    maxStrength || c->player.sword != sword || c->battle.strengthSpell
	    != strengthSpell || force) {

	Do_send_int(c, STRENGTH_PACKET);
	Do_send_double(c, strength * (1 + sqrt(sword) * N_SWORDPOWER) + strengthSpell);
	Do_send_double(c, maxStrength * (1 + sqrt(sword) * N_SWORDPOWER) + strengthSpell);

	c->player.strength = strength;
	c->player.max_strength = maxStrength;
	c->battle.strengthSpell = strengthSpell;

	if (c->player.sword != sword || force) {

	    Do_send_int(c, SWORD_PACKET);
	    Do_send_double(c, sword);

	    c->player.sword = sword;
	}
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_speed(struct client_t *c, double maxQuickness, double quicksilver, double speedSpell, int force)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/2/99
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

Do_speed(struct client_t *c, double maxQuickness, double quicksilver, double speedSpell, int force)
{
    double dtemp;
    double quickness;

        /* a player can have a minimum of 0 quickness */
    if (maxQuickness < 0.0) {
        maxQuickness = 0.0;
    }

	/* see if the player is carrying too much treasure */
    if (c->wizard < 3) {
        dtemp = ((c->player.gold + c->player.gems / 2.0) - 1000.0) /
            c->realm->charstats[c->player.type].goldtote - c->player.level;
    } else {
        dtemp = 0.0;
    }

        /* gold can only slow a player down */
    if (dtemp < 0.0) {
	dtemp = 0.0;
    }

	/* subtract speed for excessive combat */
    dtemp += c->battle.rounds / N_FATIGUE;

    quickness = maxQuickness + sqrt(floor(quicksilver)) + speedSpell +
	    c->knightQuickness - dtemp;

    if (quickness < 0.0) {
	quickness = 0.0;
    }

	/* check for changes */
    if (c->player.quickness != quickness || c->player.max_quickness !=
	    maxQuickness || c->player.quicksilver != quicksilver || 
	    c->battle.speedSpell != speedSpell || force) {

	Do_send_int(c, SPEED_PACKET);
	Do_send_double(c, quickness);

	Do_send_double(c, maxQuickness + sqrt(floor(quicksilver)) + speedSpell +
		c->knightQuickness);

	c->player.quickness = quickness;
	c->player.max_quickness = maxQuickness;
	c->battle.speedSpell = speedSpell;

	if (c->player.quicksilver != quicksilver || force) {
	    
	    Do_send_int(c, QUICKSILVER_PACKET);
	    Do_send_float(c, quicksilver);

	    c->player.quicksilver = quicksilver;
	}
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_mana(struct client_t *c, double mana)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/2/99
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

Do_mana(struct client_t *c, double mana, int force)
{
    double newMana;
    double maxMana;
    char error_msg[SZ_ERROR_MESSAGE];

	/* make sure we're still under the maximum mana */
    newMana = floor(c->player.mana + mana);

    maxMana = 1000.0 + c->player.level *
	    c->realm->charstats[c->player.type].max_mana;

    if (newMana > maxMana) {
	newMana = maxMana;
    }

    if (newMana != c->player.mana || force) {

	c->player.mana = newMana;

        Do_send_int(c, MANA_PACKET);
        Do_send_double(c, c->player.mana);
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_gold(struct client_t *c, double gold)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/2/99
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

Do_gold(struct client_t *c, double gold, int force)
{
    double dtemp;

    if (gold != 0) {

	force = TRUE;

                /* all gold is taxed to drain the economy */
/*
	if (gold > 0) {
            dtemp = floor(gold / (15 + c->player.level));
            gold -= dtemp;

	    if (dtemp > 0) {
	        Do_lock_mutex(&c->realm->kings_gold_lock);
                c->realm->kings_gold += dtemp;
                Do_unlock_mutex(&c->realm->kings_gold_lock);
	    }
	}
*/

	    /* make sure we never end up with negative gold */
	if (c->player.gold + gold > 0)
            c->player.gold = floor(c->player.gold + gold);
	else
	    c->player.gold = 0;

        Do_check_weight(c);
    }

    if (force) {
        Do_send_int(c, GOLD_PACKET);
        Do_send_double(c, c->player.gold);
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_gems(struct client_t *c, double gems)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/3/99
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

Do_gems(struct client_t *c, double gems, int force)
{
    double dtemp;

    if (gems != 0) {

	force = TRUE;

/*
	if (gems > 0) {
            dtemp = floor(gems / (15 + c->player.level));
            gems -= dtemp;

	    if (dtemp > 0) {
	        Do_lock_mutex(&c->realm->kings_gold_lock);
                c->realm->kings_gold += dtemp * N_GEMVALUE;
                Do_unlock_mutex(&c->realm->kings_gold_lock);
	    }
	}
*/

	    /* make sure we never end up with negative gems */
	if (c->player.gems + gems > 0)
            c->player.gems = floor(c->player.gems + gems);
	else
	    c->player.gems = 0;

	Do_check_weight(c);
    }

    if (force) {
        Do_send_int(c, GEMS_PACKET);
        Do_send_double(c, c->player.gems);
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_cloak(struct client_t *c)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: E. A. Estes, 12/4/85
/         Brian Kelly, 5/16/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), adjuststats(), fabs(), more(), sqrt(),
/       sleep(), floor(), wmove(), drandom(), wclear(), printw(),
/       altercoordinates(), infloat(), waddstr(), wrefresh(), mvprintw(), getans
wer(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_cloak(struct client_t *c, int state, int force)
{
    struct realm_object_t *object_ptr, **object_ptr_ptr;
    struct event_t *event_ptr;
    char error_msg[SZ_ERROR_MESSAGE];

	/* if this state is a change */
    if (state != c->player.cloaked) {
	force = TRUE;

	    /* if we're cloaking */
	if (state) {

            if (c->player.mana < MM_CLOAK) {
                Do_send_line(c, "No mana left.\n");
                Do_more(c);
                return;
            }

            else {
                c->player.cloaked = TRUE;
                c->player.mana -= MM_CLOAK;
            }
  
            Do_lock_mutex(&c->realm->realm_lock);
            Do_player_description(c);
            Do_unlock_mutex(&c->realm->realm_lock);

        } else {

            Do_lock_mutex(&c->realm->realm_lock);
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

		        event_ptr->type = NULL_EVENT;

                        /* don't give the grail to 
                           a player who uncloaks on it */
                        Do_send_line(c, "You found the Grail!  But you were cloaked...\n");
                        Do_more(c);
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
		      "[%s] encountered realm object of type %d in Do_cloak.\n",
	    	      c->connection_id, object_ptr->type);

		        Do_log_error(error_msg);	
	            }

                    if (event_ptr->type != NULL_EVENT) {
	                Do_file_event(c, event_ptr);
                    } else {
	                free((void *)event_ptr);
                    }

	            free((void *)object_ptr);
		    } else {
	            object_ptr_ptr = &((*object_ptr_ptr)->next_object);
	        }
		    }
            Do_unlock_mutex(&c->realm->realm_lock);
        }
    }

	c->player.cloaked = state;

	if (force) {
		Do_send_int(c, CLOAK_PACKET);
		Do_send_bool(c, c->player.cloaked);
	}

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_crowns(struct client_t *c, int crowns)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/3/99
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

Do_crowns(struct client_t *c, int crowns, int force)
{
    bool crownFlag;

    crownFlag = ANY(c->player.crowns);

    if (crowns != 0) {

	c->player.crowns = c->player.crowns + crowns;
	if (c->player.crowns < 0) {
	    c->player.crowns = 0.0;
	}

	if (crownFlag != ANY(c->player.crowns)) {

	    crownFlag = !crownFlag;
	    force = TRUE;

            Do_lock_mutex(&c->realm->realm_lock);
            Do_player_description(c);
            Do_unlock_mutex(&c->realm->realm_lock);
	}
    }
	
    if (force) {
	Do_send_int(c, CROWN_PACKET);
	Do_send_bool(c, crownFlag);
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_blessing(struct client_t *c, bool blessing)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/3/99
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

Do_blessing(struct client_t *c, int blessing, int force)
{
    if (blessing != c->player.blessing || force) {

	Do_send_int(c, BLESSING_PACKET);
	Do_send_bool(c, blessing);

	c->player.blessing = blessing;
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_palantir(struct client_t *c, bool palantir, int force)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/3/99
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

Do_palantir(struct client_t *c, int palantir, int force)
{
    if (palantir != c->player.palantir || force) {

	Do_send_int(c, PALANTIR_PACKET);
	Do_send_bool(c, palantir);

	c->player.palantir = palantir;
    }

        /* if no palantir, kick player out of the palantir channel */
    if ((c->player.palantir == FALSE) &&
        (c->channel == 8)) {
        
        c->channel = 1;

        c->game->hearAllChannels = HEAR_SELF;

        Do_lock_mutex(&c->realm->realm_lock);
        Do_player_description(c);
        Do_unlock_mutex(&c->realm->realm_lock);

        Do_send_specification(c, CHANGE_PLAYER_EVENT);
    }

}


/************************************************************************
/
/ FUNCTION NAME: Do_virgin(struct client_t *c, bool virgin)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/3/99
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

Do_virgin(struct client_t *c, int virgin, int force)
{
    if (virgin != c->player.virgin || force) {

	Do_send_int(c, VIRGIN_PACKET);
	Do_send_bool(c, virgin);

	c->player.virgin = virgin;
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_ring(struct client_t *c, int ring, int force)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/3/99
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

Do_ring(struct client_t *c, int ring, int force)
{
    bool ringFlag;

    switch (ring) {

    case R_NONE:

	if (c->player.ring_type != R_NONE) {
	    c->player.ring_type = R_NONE;
	    force = TRUE;
	}
	ringFlag = FALSE;

	break;
 
    case R_NAZREG:

	if (c->player.ring_type == R_NONE) {
	    c->player.ring_type = R_NAZREG;
	    c->player.ring_duration = RND() * RND() * 150 + 1;
	    force = TRUE;
	}
	ringFlag = TRUE;

	break;

    case R_DLREG:

	if (c->player.ring_type == R_NONE) {
            c->player.ring_type = R_DLREG;
            c->player.ring_duration = 0;
            force = TRUE;
        }
	ringFlag = TRUE;

	break;

    case R_BAD:

	if (c->player.ring_type == R_NONE) {
            c->player.ring_type = R_BAD;

            c->player.ring_duration = 15 +
                        c->realm->charstats[c->player.type].ring_duration +
                        (int) ROLL (0,5);

            force = TRUE;
        }
	ringFlag = TRUE;

	break;

    case R_SPOILED:

	if (c->player.ring_type == R_BAD) {
	    c->player.ring_type = R_SPOILED;

	    c->player.ring_duration = ROLL(10.0, 25.0);
	}
	ringFlag = TRUE;

	break;
    }

    if (force) {
	Do_send_int(c, RING_PACKET);
	Do_send_bool(c, ringFlag);
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_charm(struct client_t *c, int charms)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 10/3/99
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

Do_charm(struct client_t *c, int charms)
{
	/* make sure we're still under the maximum charms */
    c->player.charms = MIN(c->player.charms + charms, c->player.level + 10.0);
}


/***************************************************************************
/ FUNCTION NAME: Do_age(struct client_t *c)
/
/ FUNCTION: Default activity when no events are pending
/
/ AUTHOR:  Brian Kelly, 6/17/99
/
/ ARGUMENTS:
/       struct client_t *c - structure containing all thread info
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_age(struct client_t *c)
{
    struct event_t *event_ptr;
    double temp;

        /* take hit points if player has poison */
    if (c->player.poison > 0.0) {

        temp = c->player.poison * c->realm->charstats[c->player.type].weakness
                * c->player.max_energy / 600.0;

/* Removed bit that prevents energy loss for large poison values
        if (c->player.energy > c->player.max_energy / 10.0 && temp + 5.0 <
                c->player.energy) {
*/
        if (c->player.energy > (c->player.max_energy + c->player.shield)
		/ 10.0) {

		/* Thrown in so sick energy can't drop below 10% */
	    temp = MIN(temp, c->player.energy - (c->player.max_energy +
		    c->player.shield) / 10.0);

	    Do_energy(c, c->player.energy - temp, c->player.max_energy,
		    c->player.shield, c->battle.force_field, FALSE);
	}
    }

        /* if cloaked, remove mana */
    if (c->player.cloaked) {
        
        if ((c->player.circle == 27) || (c->player.circle == 28)) {
            temp = floor(.02 * (1000 + c->player.level * 
                c->realm->charstats[c->player.type].max_mana));
        } else if ((c->player.circle > 24) && (c->player.circle < 31)) {
            temp = floor(.01 * (1000 + c->player.level * 
                c->realm->charstats[c->player.type].max_mana));
        } else if ((c->player.circle > 19) && (c->player.circle < 36)) {
            temp = floor(.005 * (1000 + c->player.level * 
                c->realm->charstats[c->player.type].max_mana));
        } else {
            temp = c->player.circle;
        }

        if (c->player.mana >= temp)
	    Do_mana(c, -temp, FALSE);
        else {
	    Do_send_line(c,
		    "You do not have enough mana to sustain your cloak.\n");

                /* ran out of mana, uncloak */
            event_ptr = (struct event_t *) Do_create_event();
            event_ptr->type = CLOAK_EVENT;
            Do_handle_event(c, event_ptr);
        }
    }


		/* Check if the Knight should stick around */
	if(c->player.special_type == SC_KNIGHT && c->realm->king == NULL){
		Do_dethrone(c);
	}


   /* Count out aging every other turn if you are holding the One Ring */
	if (c->player.ring_type == R_DLREG){
	
	c->ageCount++;

	if (c->ageCount == 2){
		c->player.age++;
		c->ageCount = 0;
	
	} else {
	
	c->player.age++;
	
	}
	} else {
	c->player.age++;
	c->morgothCount--;
	if(c->morgothCount < 0){
		c->morgothCount = 0;
		}
	}

    while (c->player.age / N_AGE > c->player.degenerated && (c->wizard < 3)) {
	Do_degenerate(c, TRUE);
    }
}


/***************************************************************************
/ FUNCTION NAME: Do_poison_modifier(struct client_t *c, float poison)
/
/ FUNCTION: Default activity when no events are pending
/
/ AUTHOR:  Brian Kelly, 5/6/01
/
/ ARGUMENTS:
/       struct client_t *c - structure containing all thread info
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_adjusted_poison(struct client_t *c, double poison)
{
    double dtemp;

    dtemp = c->player.circle - 17;
  
    poison *= (SGN(dtemp) * pow(fabs(dtemp), .33) + 5.52) / 3;

    Do_poison(c, poison);
}


/***************************************************************************
/ FUNCTION NAME: Do_poison(struct client_t *c, double poison)
/
/ FUNCTION: Default activity when no events are pending
/
/ AUTHOR:  Brian Kelly, 10/10/99
/
/ ARGUMENTS:
/       struct client_t *c - structure containing all thread info
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_poison(struct client_t *c, double poison)
{
    if (poison != 0) {
	c->player.poison += poison;

        if (c->player.poison < 1e-4) {
            c->player.poison = 0.0;
        }

	Do_strength(c, c->player.max_strength, c->player.sword,
		c->battle.strengthSpell, FALSE);
	
    }
}


/***************************************************************************
/ FUNCTION NAME: Do_sin(struct client_t *c, float *sin)
/
/ FUNCTION: Default activity when no events are pending
/
/ AUTHOR:  Brian Kelly, 06/15/01
/
/ ARGUMENTS:
/       struct client_t *c - structure containing all thread info
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_sin(struct client_t *c, double sin)
{
    struct event_t *event_ptr;

    if (isnan(c->player.sin)) {
        c->player.sin = 0.0;
    }

   	/* Doubles any ammount of sin you take in while carrying the one ring */
	if (c->player.ring_type == R_DLREG){
	c->player.sin += sin*2;

		} else {
	c->player.sin += sin;
	}

    if (c->player.sin < 0.0) {
        c->player.sin = 0.0;
    }

    if (c->player.blessing && c->player.sin > (1500.0 + c->player.level) /
        (c->player.level + 30.0)) {

        Do_send_line(c,
           "Your blessing is consumed by the evil of your actions!\n");

        Do_more(c);
        Do_send_clear(c);
    
        Do_blessing(c, FALSE, FALSE);
    }


    if (c->player.sin > 25.0 + RND() * 25.0) {
        event_ptr = (struct event_t *)Do_create_event();
        event_ptr->type = DEATH_EVENT;
        event_ptr->arg3 = K_SIN;
        Do_file_event(c, event_ptr);
    } else if ((c->player.sin > 20.0) && (RND() < .2)) {
        Do_send_line(c,
           "You cackle gleefully at the chaos you are causing!\n");

        Do_more(c);
        Do_send_clear(c);
    }

}


/************************************************************************
/
/ FUNCTION NAME: Do_book(struct client_t *c, int bookCount)
/
/ FUNCTION: initialize a character
/
/ AUTHOR: E. A. Estes, 12/4/85
/         Brian Kelly, 5/4/99
/
/ ARGUMENTS:
/       struct player *playerp - pointer to structure to init
/
/ RETURN VALUE: none
/
/ MODULES CALLED: floor(), drandom()
/
/ DESCRIPTION:
/       Put a bunch of default values in the given structure.
/
*************************************************************************/

Do_book(struct client_t *c, int bookCount)
{
    int i, multiple;
    float ftemp;
    double dtemp1, dtemp2;

    for (i = 0; i < bookCount; i++) {

        ftemp = c->player.brains - c->player.level *
                c->realm->charstats[c->player.type].max_brains;

        multiple = ceil(ftemp / (c->player.level * 
                        c->realm->charstats[c->player.type].max_brains));

	dtemp1 = c->realm->charstats[c->player.type].brains.increase;

        /* make it really hard to get more than max brains */
        if (ftemp > 0) {

            if (multiple > 0) {
	        dtemp2 = dtemp1 * (dtemp1 - 1) / multiple;
	    } else {
	        dtemp2 = dtemp1 * (dtemp1 - 1);
            }

            c->player.brains += dtemp2 / sqrt(ftemp + dtemp2);
        } else {
	    c->player.brains += dtemp1 / 2;
        }
    }
}
/************************************************************************
/
/ FUNCTION NAME: Do_forceage(struct client_t t, double Degens)
/
/ FUNCTION: forcefully age a player
/
/ AUTHOR: Arella Kirstar 7/4/02
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: death(), floor(), wmove(), drandom(), waddstr(), explevel()
/
/ GLOBAL INPUTS: Player, *stdscr, *Statptr, Stattable[]
/
/ GLOBAL OUTPUTS: Player, Changed
/
/ DESCRIPTION:
/       Used to forcefully add degenerations and age to a player
/
*************************************************************************/

Do_forceage(struct client_t *c, double Degens)
{
    struct event_t *event_ptr;
    char string_buffer[SZ_LINE];

	/* add the degens and age */
    c->player.degenerated = Degens;
    c->player.age = Degens * 750;
	
}



