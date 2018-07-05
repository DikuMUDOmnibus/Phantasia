/*
 * info.c       Routines to retriving information
 */

#include "include.h"

/************************************************************************
/
/ FUNCTION NAME: struct examine_t *Do_create_examine(c)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 01/04/01
/
/ ARGUMENTS:
/       struct client_t c - pointer to the main client strcture
/
/ RETURN VALUE: 
/	char - character the describes the character
/
/ MODULES CALLED: strcpy()
/
/ DESCRIPTION:
/       Return a string describing the player type.
/       King, council, valar, supercedes other types.
/       The first character of the string is '*' if the player
/       has a crown.
/       If 'shortflag' is TRUE, return a 3 character string.
/
*************************************************************************/

struct examine_t *Do_create_examine(struct client_t *c, struct game_t *requestor)
{
    struct examine_t *examine_ptr;

	/* create the strcture */
    examine_ptr = (struct examine_t *)Do_malloc(SZ_EXAMINE);

	/* lock the realm for the next two calls */
    Do_lock_mutex(&c->realm->realm_lock);

	/* to titles and specific information */
    Do_make_character_title(c, c->game, &examine_ptr->title);
    strcat(examine_ptr->title, "\n");

	/* copy over the coords or description */
    if (Do_show_character_coords(requestor, c->game)) {

	sprintf(examine_ptr->location, "(%.0lf, %.0lf)\n", c->player.x,
		c->player.y);
    }
    else {
	strcpy(examine_ptr->location, c->player.area);
	strcat(examine_ptr->location, "\n");
    }
    Do_unlock_mutex(&c->realm->realm_lock);

	/* determine if the network is printable */
    if (!c->addressResolved || c->wizard > 2) {
        strcpy(examine_ptr->network, "<unavailable>\n");
    }
    else {
        strcpy(examine_ptr->network, c->network);
	strcat(examine_ptr->network, "\n");
    }

	/* determine when the next level will occur */
    examine_ptr->nextLevel = 1800 * (c->player.level + 1) *
	    (c->player.level + 1);

	/* format player gender */
    if (c->player.gender == MALE) {
	strcpy(examine_ptr->gender, "Male\n");
    }
    else {
	strcpy(examine_ptr->gender, "Female\n");
    }

    Do_format_time(examine_ptr->time_played, c->player.time_played +
	    time(NULL) - c->player.last_load);

    if (c->wizard > 2) {
        strcpy(examine_ptr->account, "<unavailable>\n");
    }
    else if (!strcmp(c->account,"eyhung")) {
        strcpy(examine_ptr->account, c->player.name);
        strcat(examine_ptr->account, "\n");
    }
    else {
        strcpy(examine_ptr->account, c->account);
        strcat(examine_ptr->account, "\n");
    }

    Do_true_false(&examine_ptr->cloaked, c->player.cloaked);
    Do_true_false(&examine_ptr->blind, c->player.blind);
    Do_true_false(&examine_ptr->virgin, c->player.virgin);
    Do_true_false(&examine_ptr->palantir, c->player.palantir);
    Do_true_false(&examine_ptr->blessing, c->player.blessing);
    Do_true_false(&examine_ptr->ring, c->player.ring_type);

    ctime_r(&c->player.last_load, examine_ptr->date_loaded);
    ctime_r(&c->player.date_created, examine_ptr->date_created);

    examine_ptr->channel = c->channel;
    examine_ptr->level = c->player.level;
    examine_ptr->experience = c->player.experience;
    examine_ptr->energy = c->player.energy;
    examine_ptr->max_energy = c->player.max_energy;
    examine_ptr->strength = c->player.strength * 
                            (1 + sqrt(c->player.sword) * N_SWORDPOWER);
    examine_ptr->max_strength = c->player.max_strength;
    examine_ptr->quickness = c->player.quickness;
    examine_ptr->max_quickness = c->player.max_quickness;
    examine_ptr->mana = c->player.mana;
    examine_ptr->brains = c->player.brains;
    examine_ptr->magiclvl = c->player.magiclvl;
    examine_ptr->poison = c->player.poison;
    examine_ptr->sin = c->player.sin;
    examine_ptr->lives = c->player.lives;
    examine_ptr->gold = c->player.gold;
    examine_ptr->gems = c->player.gems;
    examine_ptr->sword = c->player.sword;
    examine_ptr->shield = c->player.shield;
    examine_ptr->quicksilver = c->player.quicksilver;
    examine_ptr->holywater = c->player.holywater;
    examine_ptr->amulets = c->player.amulets;
    examine_ptr->charms = c->player.charms;
    examine_ptr->crowns = c->player.crowns;
    examine_ptr->age = c->player.age;
    examine_ptr->degenerated = c->player.degenerated;

    return examine_ptr;
}


/************************************************************************
/
/ FUNCTION NAME: Do_make_character_title(struct client_t *c, struct game_t *game_ptr, char *theTitle)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/04/01
/
/ ARGUMENTS:
/       char *name - name of character to look for
/       struct player *playerp - pointer of structure to fill
/
/ RETURN VALUE: location of player if found, -1 otherwise
/
/ MODULES CALLED: fread(), fseek(), strcmp()
/
/ GLOBAL INPUTS: Wizard, *Playersfp
/
/ GLOBAL OUTPUTS: none
/
/ DESCRIPTION:
/       Search the player file for the player of the given name.
/       If player is found, fill structure with player data.
/
*************************************************************************/

Do_make_character_title(struct client_t *c, struct game_t *game_ptr, char *theTitle)
{
    char title[20];

/* make sure the realm is locked before calling this function */

    title[0] = '\0';

    if (game_ptr->description->wizard > 2) {
        strcpy(title, "Wizard ");
    } else if (game_ptr->description->wizard == 2) {
        strcpy(title, "Apprentice ");
    }
        

    switch (game_ptr->description->special_type) {

    case SC_KNIGHT:

	if (game_ptr->description->gender == MALE) {
	    strcpy(title, "Sir ");
	}
	else {
	    strcpy(title, "Dame ");
	}
	break;

    case SC_STEWARD:

	strcpy(title, "Steward ");
	break;

    case SC_KING:

	if (game_ptr->description->gender == MALE) {
	    strcpy(title, "King ");
	}
	else {
	    strcpy(title, "Queen ");
	}
	break;

    case SC_COUNCIL:
    case SC_EXVALAR:

	if (game_ptr->description->gender == MALE) {
	    strcpy(title, "Councilman ");
	}
	else {
	    strcpy(title, "Councilwoman ");
	}
	break;

    case SC_VALAR:

	strcpy(title, "Valar ");
        break;
    }

    sprintf(theTitle, "%s%s the %s",  title, game_ptr->description->name,
	    c->realm->charstats[game_ptr->description->type].class_name);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_show_character_coords(struct game_t *requestor, struct game *requestee)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/04/01
/
/ ARGUMENTS:
/       char *name - name of character to look for
/       struct player *playerp - pointer of structure to fill
/
/ RETURN VALUE: location of player if found, -1 otherwise
/
/ MODULES CALLED: fread(), fseek(), strcmp()
/
/ GLOBAL INPUTS: Wizard, *Playersfp
/
/ GLOBAL OUTPUTS: none
/
/ DESCRIPTION:
/       Search the player file for the player of the given name.
/       If player is found, fill structure with player data.
/
*************************************************************************/

int Do_show_character_coords(struct game_t *requestor, struct game_t *requestee)
{

/* the realm should be locked before calling this procedure */

	/* Show name if in place like Valhalla */
    if (requestee->useLocationName) {
	return FALSE;
    }

	/* Show coords if requestee is requestor */
    if (requestee == requestor) {
	return TRUE;
    }

	/* Show name if requestor descriptionless */
    if (requestor->description == NULL) {
	return FALSE;
    }

	/* Show name if requestee is virtual */
    if (requestee->virtual) {
	return FALSE;
    }

	/* Show coords if the requestor is a game wizard */
    if (requestor->description->wizard > 2) {
	return TRUE;
    }

	/* Show name if requestor is blind */
    if (requestor->description->blind) {
	return FALSE;
    }

	/* Show coords if requestor has a palantir */
    if (requestor->description->palantir) {
	return TRUE;
    }

	/* Show name if requestee is not special and outside requestor */
    if (requestee->description->special_type == SC_NONE &&
	    requestee->circle > requestor->circle) {

	return FALSE;
    }

	/* Show name if requestee is special and far out */
    if (requestee->description->special_type != SC_NONE &&
	    requestee->circle > 400) {
	return FALSE;
    }

	/* Show name if requestee is cloaked */
    if (requestee->description->cloaked) {
	return FALSE;
    }

        /* Show name if requestor is experimento and low-level */
    if ((requestor->description->type == C_EXPER) && 
        (requestor->description->level < 50)) {
        return FALSE;
    }

	/* Otherwise show coords (Got all that?) */
    return TRUE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_format_time(struct client_t *c, char *theString, int theTime)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/04/01
/
/ ARGUMENTS:
/       char *name - name of character to look for
/       struct player *playerp - pointer of structure to fill
/
/ RETURN VALUE: location of player if found, -1 otherwise
/
/ MODULES CALLED: fread(), fseek(), strcmp()
/
/ GLOBAL INPUTS: Wizard, *Playersfp
/
/ GLOBAL OUTPUTS: none
/
/ DESCRIPTION:
/       Search the player file for the player of the given name.
/       If player is found, fill structure with player data.
/
*************************************************************************/

Do_format_time(char *theString, int theTime)
{
    int minutes, hours;

    hours = theTime / 3600;
    theTime -= hours * 3600;

    minutes = theTime / 60;
    theTime -= minutes * 60;

    sprintf(theString, "%2.2d:%2.2d:%2.2d\n", hours, minutes, theTime);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_true_false(char *theString, int theBool)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/04/01
/
/ ARGUMENTS:
/       char *name - name of character to look for
/       struct player *playerp - pointer of structure to fill
/
/ RETURN VALUE: location of player if found, -1 otherwise
/
/ MODULES CALLED: fread(), fseek(), strcmp()
/
/ GLOBAL INPUTS: Wizard, *Playersfp
/
/ GLOBAL OUTPUTS: none
/
/ DESCRIPTION:
/       Search the player file for the player of the given name.
/       If player is found, fill structure with player data.
/
*************************************************************************/

Do_true_false(char *theString, int theBool)
{
    if (theBool) {
	strcpy(theString, "Yes\n");
    }
    else {
	strcpy(theString, "No\n");
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_examine_character(c, struct examine_t *theInfo)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 01/04/01
/
/ ARGUMENTS:
/       struct client_t c - pointer to the main client strcture
/
/ RETURN VALUE: 
/	char - character the describes the character
/
/ MODULES CALLED: strcpy()
/
/ DESCRIPTION:
/       Return a string describing the player type.
/       King, council, valar, supercedes other types.
/       The first character of the string is '*' if the player
/       has a crown.
/       If 'shortflag' is TRUE, return a 3 character string.
/
*************************************************************************/

Do_examine_character(struct client_t *c, struct examine_t *theInfo)
{

    Do_send_int(c, PLAYER_INFO_PACKET);

    Do_send_string(c, theInfo->title);
    Do_send_string(c, theInfo->location);

    Do_send_string(c, theInfo->account);
    Do_send_string(c, theInfo->network);
    Do_send_int(c, theInfo->channel);

    Do_send_double(c, theInfo->level);
    Do_send_double(c, theInfo->experience);
    Do_send_double(c, theInfo->nextLevel);

    Do_send_double(c, theInfo->energy);
    Do_send_double(c, theInfo->max_energy);
    Do_send_double(c, theInfo->shield);
    Do_send_double(c, theInfo->strength);
    Do_send_double(c, theInfo->max_strength);
    Do_send_double(c, theInfo->sword);
    Do_send_float(c, theInfo->quickness);
    Do_send_float(c, theInfo->max_quickness);
    Do_send_float(c, theInfo->quicksilver);
    Do_send_double(c, theInfo->brains);
    Do_send_double(c, theInfo->magiclvl);
    Do_send_double(c, theInfo->mana);
    Do_send_string(c, theInfo->gender);
    Do_send_fpfloat(c, theInfo->poison);
    Do_send_fpfloat(c, theInfo->sin);
    Do_send_int(c, theInfo->lives);

    Do_send_double(c, theInfo->gold);
    Do_send_double(c, theInfo->gems);
    Do_send_int(c, theInfo->holywater);
    Do_send_int(c, theInfo->amulets);
    Do_send_int(c, theInfo->charms);
    Do_send_int(c, theInfo->crowns);
    Do_send_string(c, theInfo->virgin);
    Do_send_string(c, theInfo->blessing);
    Do_send_string(c, theInfo->palantir);
    Do_send_string(c, theInfo->ring);

    Do_send_string(c, theInfo->cloaked);
    Do_send_string(c, theInfo->blind);
    Do_send_int(c, theInfo->age);
    Do_send_int(c, theInfo->degenerated);
    Do_send_string(c, theInfo->time_played);
    Do_send_string(c, theInfo->date_loaded);
    Do_send_string(c, theInfo->date_created);

    return;
}



/************************************************************************
/
/ FUNCTION NAME: struct details_t *Do_create_detail(c)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 01/16/01
/
/ ARGUMENTS:
/       struct client_t c - pointer to the main client strcture
/
/ RETURN VALUE: 
/	char - character the describes the character
/
/ MODULES CALLED: strcpy()
/
/ DESCRIPTION:
/       Return a string describing the player type.
/       King, council, valar, supercedes other types.
/       The first character of the string is '*' if the player
/       has a crown.
/       If 'shortflag' is TRUE, return a 3 character string.
/
*************************************************************************/

struct detail_t *Do_create_detail(struct client_t *c)
{
    struct detail_t *detail_ptr;
    struct account_t theAccount;

	/* create the strcture */
    detail_ptr = (struct detail_t *)Do_malloc(SZ_DETAIL);

	/* character information */
    strcpy(detail_ptr->modifiedName, c->modifiedName);
    strcat(detail_ptr->modifiedName, "\n");
    strcpy(detail_ptr->name, c->player.name);
    strcat(detail_ptr->name, "\n");
    Do_true_false(detail_ptr->faithful, c->player.faithful);
    strcpy(detail_ptr->parentAccount, c->player.parent_account);
    strcat(detail_ptr->parentAccount, "\n");
    strcpy(detail_ptr->charParentNetwork, c->player.parent_network);
    strcat(detail_ptr->charParentNetwork, "\n");
    Do_look_account(c, &(c->account), &theAccount);
    detail_ptr->playerMutes = theAccount.muteCount;

	/* account information */
    strcpy(detail_ptr->account, c->account);
    strcat(detail_ptr->account, "\n");
    strcpy(detail_ptr->email, c->email);
    strcat(detail_ptr->email, "\n");
    strcpy(detail_ptr->accParentNetwork, c->parentNetwork);
    strcat(detail_ptr->accParentNetwork, "\n");

	/* connection information */
    strcpy(detail_ptr->IP, c->IP);
    strcat(detail_ptr->IP, "\n");
    strcpy(detail_ptr->network, c->network);
    strcat(detail_ptr->network, "\n");
    detail_ptr->machineID = c->machineID;
    ctime_r(&c->date_connected, detail_ptr->dateConnected);
    
    return detail_ptr;
}


/************************************************************************
/
/ FUNCTION NAME: Do_detail_connection(c, struct detail_t *theInfo)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 01/16/01
/
/ ARGUMENTS:
/       struct client_t c - pointer to the main client strcture
/
/ RETURN VALUE: 
/	char - character the describes the character
/
/ MODULES CALLED: strcpy()
/
/ DESCRIPTION:
/       Return a string describing the player type.
/       King, council, valar, supercedes other types.
/       The first character of the string is '*' if the player
/       has a crown.
/       If 'shortflag' is TRUE, return a 3 character string.
/
*************************************************************************/

Do_detail_connection(struct client_t *c, struct detail_t *theInfo)
{ 

    Do_send_int(c, CONNECTION_DETAIL_PACKET);

    Do_send_string(c, theInfo->modifiedName);
    Do_send_string(c, theInfo->name);
    Do_send_string(c, theInfo->faithful);
    Do_send_string(c, theInfo->parentAccount);
    Do_send_string(c, theInfo->charParentNetwork);
    Do_send_int(c, theInfo->playerMutes);

    Do_send_string(c, theInfo->account);
    Do_send_string(c, theInfo->email);
    Do_send_string(c, theInfo->accParentNetwork);

    Do_send_string(c, theInfo->IP);
    Do_send_string(c, theInfo->network);
    Do_send_int(c, theInfo->machineID);
    Do_send_string(c, theInfo->dateConnected);

    return;
}


/***************************************************************************
/ FUNCTION NAME: Do_last_load_info(struct client_t *c)
/
/ FUNCTION: Default activity when no events are pending
/
/ AUTHOR:  Brian Kelly, 01/04/01
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

Do_last_load_info(struct client_t *c)
{
    char string_buffer[SZ_LINE];
    char string_buffer2[SZ_LINE];
    long answer;
    int exceptionFlag, wizType = 0;
    char wizNetwork[SZ_FROM], wizAccount[SZ_NAME], wizCharacter[SZ_NAME];
    FILE *wizard_file;

	/* quick character report */
    if (c->player.bad_passwords) {

        sprintf(string_buffer, "There have been %d failed login attempts with this character since last successful login.\n", c->player.bad_passwords);

        Do_send_line(c, string_buffer);
    }

    if (c->player.load_count == 0) {

        sprintf(string_buffer, "Welcome %s.  If this is your first journey into the realm, there is a good quick overview under the \"Help\" button that will appear after you hit \"More\".\n", c->player.name);

    }
    else {

            /* convert the last time and remove the "\n". */
	ctime_r(&c->player.last_load, string_buffer2);
        string_buffer2[strlen(string_buffer2) - 1] = '\0';

        sprintf(string_buffer,
		"Last login from \"%s\" with account \"%s\" at %s.\n",
		c->player.last_IP, c->player.last_account, string_buffer2);
    }

    Do_send_line(c, string_buffer);

    sprintf(string_buffer, "You are currently playing from \"%s\".\n", c->IP);
    Do_send_line(c, string_buffer);

	/* old info is out, so the new comes in */

        /* open the wizard file to see if this person is one */
    if ((wizard_file=fopen(WIZARD_FILE, "r")) == NULL) {

        sprintf(string_buffer,
            "[%s] fopen of %s failed in Do_last_load_info : %s\n",
            c->connection_id, WIZARD_FILE, strerror(errno));

        Do_log_error(string_buffer);
    }
    else {

            /* loop through the wizard names */
        while (fscanf(wizard_file, "%ld %s %s %s %d\n", &answer,
                &wizNetwork, &wizAccount, &wizCharacter, &exceptionFlag) == 5) {

                /* put down the highest wizType */
            if (!strcmp(wizAccount, c->lcaccount) &&
                    !strcmp(wizNetwork, c->network)) {

		if (answer > wizType) {
		    wizType = answer;
		}
            }
        }
        fclose(wizard_file);
    }

    if ((wizType > 2) && strcmp(c->player.last_account, c->account)
        && (c->player.load_count != 0)) {

        Do_send_line(c, "Using backdoor, not copying info.\n");
        strcpy(c->wizaccount, c->account);
        strcpy(c->account, c->player.last_account);
        strcpy(c->wizIP, c->IP);
        strcpy(c->IP, c->player.last_IP);

    } else {
        strcpy(c->player.last_IP, c->IP);
        strcpy(c->player.last_account, c->account);
        c->player.last_load = time(NULL);
        c->player.bad_passwords = 0;
        ++c->player.load_count;

    }

    return;
} 

/************************************************************************
/
/ FUNCTION NAME: Do_list_characters(struct client_t *c)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: E. A. Estes, 2/2/86
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

Do_list_characters(struct client_t *c)
{
    struct game_t *game_ptr;
    char string_buffer[SZ_LINE], title[SZ_LINE];
    char location[SZ_LINE], network[SZ_LINE];
    int    numusers = 0;   /* number of users on file */

	/* print a header */
    Do_send_line(c, "Character Title, Location, Level, Network\n");

	/* lock the realm */
    Do_lock_mutex(&c->realm->realm_lock);

	/* run through all the games */
    game_ptr = c->realm->games;
    while (game_ptr != NULL) {

	    /* if the player is playing */ 
        if (game_ptr->description != NULL) {

		/* get the character's title */
	    Do_make_character_title(c, game_ptr, title);

		/* make the location */
	    if (Do_show_character_coords(c->game, game_ptr)) {
       		sprintf(location, "(%.0lf, %.0lf)", game_ptr->x, game_ptr->y);
	    }
	    else {
	        strcpy(location, game_ptr->area);
	    }

		/* choose the network display */
	    if (game_ptr->network[0] == '\0' ||
		    game_ptr->description->wizard > 2) {

	        strcpy(network, "<unavailable>");
	    }
	    else {
	        strcpy(network, game_ptr->network);
	    }

		/* put everything together */
	    sprintf(string_buffer, "%s, %s, %.0lf, %s\n", title, location,
		    game_ptr->description->level, network);

	    numusers++;
	    Do_send_line(c, string_buffer);
        }
	game_ptr = game_ptr->next_game;
    }

    Do_unlock_mutex(&c->realm->realm_lock);

    Do_send_line(c, "\n");
    sprintf(string_buffer, "There are currently %d players.\n", numusers);
    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_list_connections(struct client_t *c)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 01/16/01
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

Do_list_connections(struct client_t *c)
{
    struct game_t *game_ptr;
    char string_buffer[SZ_LINE];
    char character[SZ_NAME], account[SZ_NAME];
    int    numusers = 0;   /* number of users on file */

	/* print a header */
    Do_send_line(c, "Character, Account, Address, Machine\n");

	/* lock the realm */
    Do_lock_mutex(&c->realm->realm_lock);

	/* run through all the games */
    game_ptr = c->realm->games;
    while (game_ptr != NULL) {

	    /* if the player is playing */ 
        if (game_ptr->description == NULL) {
	    strcpy(character, "<no character>");
	}
	else {
	    strcpy(character, game_ptr->description->name);
	}

	    /* if the player has an account */
	if (game_ptr->account[0] == '\0') {
	    strcpy(account, "<no account>");
	}
	else {
	    strcpy(account, game_ptr->account);
	}

	    /* put everything together */
	sprintf(string_buffer, "%s, %s, %s, %ld\n", character,
		account, game_ptr->IP, game_ptr->machineID);

	numusers++;
	Do_send_line(c, string_buffer);

	game_ptr = game_ptr->next_game;
    }

    Do_unlock_mutex(&c->realm->realm_lock);

    Do_send_line(c, "\n");
    sprintf(string_buffer, "There are currently %d connections.\n", numusers);
    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_save_history(struct client_t *c, struct history_t *theHistory)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: E. A. Estes, 2/2/86
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

Do_save_history(struct client_t *c, struct history_t *theHistory)
{
    char string_buffer[SZ_LINE];
    FILE *history_file;

    Do_lock_mutex(&c->realm->history_file_lock);

        /* open the history file for writing */
    errno = 0;
    if ((history_file=fopen(HISTORY_FILE, "a")) == NULL) {

        Do_unlock_mutex(&c->realm->history_file_lock);
        sprintf(string_buffer,
                "[%s] fopen of %s failed in Do_save_history: %s.\n",
                c->connection_id, HISTORY_FILE, strerror(errno));

        Do_log_error(string_buffer);
        return;
    }

        /* write the new account at the end */
    if (fwrite((void *)theHistory, SZ_HISTORY, 1, history_file) != 1) {

        fclose(history_file);
        Do_unlock_mutex(&c->realm->history_file_lock);
        sprintf(string_buffer,
                "[%s] fwrite of %s failed in Do_save_history: %s.\n",
                c->connection_id, HISTORY_FILE, strerror(errno));

        Do_log_error(string_buffer);
        return;
    }

        /* close the file */
    fclose(history_file);
    Do_unlock_mutex(&c->realm->history_file_lock);
    return;
}



/************************************************************************
/
/ FUNCTION NAME: struct history_list_t *Do_look_history(struct client_t *c, int theType, char *theName)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 01/15/01
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

struct history_list_t *Do_look_history(struct client_t *c, int historyType, char *historyName)
{
    struct history_list_t *returnList, *list_ptr;
    struct history_t theHistory;
    char string_buffer[SZ_LINE];
    FILE *history_file;

    returnList = NULL;

    Do_lock_mutex(&c->realm->history_file_lock);

        /* open the history file for reading */
    errno = 0;
    if ((history_file=fopen(HISTORY_FILE, "r")) == NULL) {

        Do_unlock_mutex(&c->realm->history_file_lock);
        sprintf(string_buffer,
                "[%s] fopen of %s failed in Do_look_history: %s.\n",
                c->connection_id, HISTORY_FILE, strerror(errno));

        Do_log_error(string_buffer);
        return returnList;
    }

        /* run through the history entries */
    while (fread((void *)&theHistory, SZ_HISTORY, 1, history_file) == 1) {

            /* if this this is one of what we want */
        if (theHistory.type == historyType && 
		strcmp(theHistory.name, historyName) == 0) {

		/* copy the information over */
	    list_ptr = (struct history_list_t *) Do_malloc(SZ_HISTORY_LIST);
	    memcpy(&list_ptr->theHistory, &theHistory, SZ_HISTORY);

		/* put the element into the list */
	    list_ptr->next = returnList;
	    returnList = list_ptr;
        }
    }

        /* close the file */
    fclose(history_file);
    Do_unlock_mutex(&c->realm->history_file_lock);
    return returnList;
}


/************************************************************************
/
/ FUNCTION NAME: Do_show_history(struct client_t *c, struct history_list_t *theList)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 01/15/01
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

Do_show_history(struct client_t *c, struct history_list_t *theList)
{
    struct history_list_t *list_ptr;
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE];

    char *taggedTypes[] = {"address", "account"};

    if (theList == NULL) {
	Do_send_line(c, "No history found.\n");
    }

    while (theList != NULL) {

	    /* get the time of the event */
	ctime_r(&theList->theHistory.date, error_msg);
	error_msg[strlen(error_msg) - 1] = '\0';

	    /* format the message */
	sprintf(string_buffer, "%s - %s %s: %s", error_msg,
		taggedTypes[theList->theHistory.type],
		theList->theHistory.name, theList->theHistory.description);

	    /* print the message */
	Do_send_line(c, string_buffer);

	    /* increment to the next in list */
	list_ptr = theList->next;
	free((void *)theList);
	theList = list_ptr;
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_wizard_information(struct client_t *c)
/
/ FUNCTION: do random stuff
/
/ AUTHOR: Brian Kelly, 01/16/01
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

Do_wizard_information(struct client_t *c)
{
    struct event_t *event_ptr;
    struct button_t buttons;
    struct realm_object_t *object_ptr, *grail_ptr, *trove_ptr;
    long answer;
    char error_msg[SZ_ERROR_MESSAGE];
    char string_buffer[SZ_LINE], playerName[SZ_NAME];

    strcpy(buttons.button[0], "Details\n");
    strcpy(buttons.button[1], "Connections\n");
    Do_clear_buttons(&buttons, 2);
    strcpy(buttons.button[3], "Channel 9\n");
    strcpy(buttons.button[5], "Secrets\n");
    strcpy(buttons.button[7], "Cancel\n");

    Do_lock_mutex(&c->realm->realm_lock);
    if (c->game->hearAllChannels == HEAR_ALL) {
        strcpy(buttons.button[4], "Tune Out\n");
    }
    else {
        strcpy(buttons.button[4], "Hear All\n");
    }
    Do_unlock_mutex(&c->realm->realm_lock);

    if (Do_buttons(c, &answer, &buttons) != S_NORM || answer == 7) {
	return;
    }

    switch (answer) {

	/* see this player's backstage information */
    case 0:
        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->to = c->game;
        event_ptr->from = c->game;
	event_ptr->type = REQUEST_DETAIL_EVENT;

            /* get the name of the player to look at */
        if (Do_player_dialog(c, "Which character do you want details on?\n",
                playerName) != S_NORM) {

            free((void *)event_ptr);
            return;
        }

        if (!Do_send_character_event(c, event_ptr, playerName)) {
            free((void *)event_ptr);
            Do_send_line(c, "That character just left the game.\n");
            Do_more(c);
	    Do_send_clear(c);
            return;
        }

        return;

    case 1:
	Do_list_connections(c);
	return;

    case 3:

        c->channel = 9;
        Do_lock_mutex(&c->realm->realm_lock);
        Do_player_description(c);
        Do_unlock_mutex(&c->realm->realm_lock);
        Do_send_specification(c, CHANGE_PLAYER_EVENT);
	return;

    case 4:
        Do_lock_mutex(&c->realm->realm_lock);
        if (c->game->hearAllChannels == HEAR_ALL) {
            c->game->hearAllChannels = HEAR_SELF;
        } else {
            c->game->hearAllChannels = HEAR_ALL;
        }
        Do_unlock_mutex(&c->realm->realm_lock);
	return;

    case 5:
	grail_ptr = NULL;
	trove_ptr = NULL;
        Do_lock_mutex(&c->realm->realm_lock);
	object_ptr = c->realm->objects;

	while (object_ptr != NULL) {

	    if (object_ptr->type == HOLY_GRAIL) {

		if (grail_ptr != NULL) {

	            sprintf(error_msg,
    "[%s] Duplicate grails found in realm objects in Do_wizard_information.\n",
    c->connection_id);

		    Do_log_error(error_msg);
		    Do_send_line(c, "Extra grail found in the realm!\n");
		}
		else {
		    grail_ptr = object_ptr;
		}
	    }
	    else if (object_ptr->type == TREASURE_TROVE) {

		if (trove_ptr != NULL) {

	            sprintf(error_msg,
    "[%s] Duplicate troves found in realm objects in Do_wizard_information.\n",
    c->connection_id);

		    Do_log_error(error_msg);
		    Do_send_line(c, "Extra trove found in the realm!\n");
		}
		else {
		    trove_ptr = object_ptr;
		}
	    }

	    object_ptr = object_ptr->next_object;
	}

	    /* indicate the grail location */
	if (grail_ptr == NULL) {

	    sprintf(error_msg,
	    "[%s] No grail found in realm objects in Do_wizard_information.\n",
	    c->connection_id);

	    Do_log_error(error_msg);
	    Do_send_line(c, "Holy Grail: ** NOT FOUND **\n");
	}
	else {

	    sprintf(string_buffer, "Holy Grail: %.0lf, %.0lf\n", grail_ptr->x,
		    grail_ptr->y);

	    Do_send_line(c, string_buffer);
	}

	    /* indicate the trove location */
	if (trove_ptr == NULL) {

	    sprintf(error_msg,
	    "[%s] No trove found in realm objects in Do_wizard_information.\n",
	    c->connection_id);

	    Do_log_error(error_msg);
	    Do_send_line(c, "Treasure Trove: ** NOT FOUND **\n");
	}
	else {

	    sprintf(string_buffer, "Treasure Trove: %.0lf, %.0lf\n",
		    trove_ptr->x, trove_ptr->y);

	    Do_send_line(c, string_buffer);
	}

        Do_unlock_mutex(&c->realm->realm_lock);

	    /* show the king and steward coffers */
	Do_lock_mutex(&c->realm->kings_gold_lock);

	sprintf(string_buffer, "King Treasury: %.0lf\n", c->realm->kings_gold);
	Do_send_line(c, string_buffer);

	sprintf(string_buffer, "Steward Treasury: %.0lf\n",
		c->realm->steward_gold);
	Do_send_line(c, string_buffer);

	Do_unlock_mutex(&c->realm->kings_gold_lock);

        Do_lock_mutex(&c->realm->realm_lock);
        if (c->realm->king != NULL) {
            if (c->realm->king->description != NULL) {
                sprintf(string_buffer, "Ruler : %s\n",
                    c->realm->king->description->name);
	        Do_send_line(c, string_buffer);
            } else {
                Do_send_line(c, "Ruler, but no description.\n");
            }
        }


	sprintf(string_buffer, "King : %s    Valar : %s \n",
		c->realm->king_name, c->realm->valar_name);
	Do_send_line(c, string_buffer);

        Do_unlock_mutex(&c->realm->realm_lock);
	Do_more(c);
	Do_send_clear(c);

	break;

    default:

	sprintf(error_msg,
		"[%s] Returned non-option %ld in Do_wizard_information.\n",
		c->connection_id, answer);
		
	Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_history(struct client_t *c)
/
/ FUNCTION: do random stuff
/
/ AUTHOR: Brian Kelly, 01/17/01
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

Do_history(struct client_t *c)
{
    struct button_t buttons;
    struct player_t thePlayer;
    long answer;
    char error_msg[SZ_ERROR_MESSAGE];
    char string_buffer[SZ_LINE], playerName[SZ_NAME];

    Do_send_line(c, "What do you wish history on?\n");

    strcpy(buttons.button[0], "Char Online\n");
    strcpy(buttons.button[1], "Saved Char\n");
    strcpy(buttons.button[2], "Account\n");
    strcpy(buttons.button[3], "Address\n");
    Do_clear_buttons(&buttons, 4);
    strcpy(buttons.button[5], "Enter New\n");
    strcpy(buttons.button[7], "Cancel\n");

    if (Do_buttons(c, &answer, &buttons) != S_NORM || answer == 7) {
	return;
	Do_send_clear(c);
    }

    Do_send_clear(c);
    switch (answer) {

    case 0:
	Do_player_history(c);
	return;

    case 1:
	Do_saved_player_history(c);
	return;

    case 2:
	Do_account_history(c);
	return;

    case 3:
	Do_address_history(c);
	return;

    case 5:
	Do_make_history(c);
	return;

    default:

	sprintf(error_msg,
		"[%s] Returned non-option %ld in Do_wizard_information.\n",
		c->connection_id, answer);
		
	Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_account_history(struct client_t *c)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 01/15/01
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

Do_account_history(struct client_t *c)
{
    struct history_list_t *list_ptr;
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE];
    char theAccount[SZ_NAME];

    if (Do_string_dialog(c, theAccount, SZ_NAME - 1,
	    "Retrive the history on which account?\n")) {

	return;
    }

    list_ptr = Do_look_history(c, T_ACCOUNT, theAccount);
    Do_show_history(c, list_ptr);

    Do_more(c);
    Do_send_clear(c);
}


/************************************************************************
/
/ FUNCTION NAME: Do_address_history(struct client_t *c)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 01/15/01
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

Do_address_history(struct client_t *c)
{
    struct history_list_t *list_ptr;
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE];
    char theAddress[SZ_NAME];

    if (Do_string_dialog(c, theAddress, SZ_NAME - 1,
	    "Retrive the history on which address?\n")) {

	return;
    }

    list_ptr = Do_look_history(c, T_ADDRESS, theAddress);
    Do_show_history(c, list_ptr);

    Do_more(c);
    Do_send_clear(c);
}


/************************************************************************
/
/ FUNCTION NAME: Do_player_history(struct client_t *c)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 01/15/01
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

Do_player_history(struct client_t *c)
{
    struct history_list_t *list_ptr, *list_ptr_two;
    struct game_t *game_ptr;
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE];
    char characterName[SZ_NAME], account[SZ_NAME], machineID[SZ_NAME];
    bool characterFound;

    if (Do_player_dialog(c, "Retrive the history on which player?\n",
	    characterName) != S_NORM) {

	return;
    }

    characterFound = FALSE;
    Do_lock_mutex(&c->realm->realm_lock);

        /* run though all the games and check the names */
    game_ptr = c->realm->games;
    while (game_ptr != NULL) {

            /* check for a name match */
        if (game_ptr->description != NULL &&
                !strcmp(characterName, game_ptr->description->name)) {

	    strcpy(account, game_ptr->account);
	    characterFound = TRUE;
	    break;
        }

        game_ptr = game_ptr->next_game;
    }

        /* the character was not found */
    Do_unlock_mutex(&c->realm->realm_lock);

    if (!characterFound) {

	Do_send_line(c, "That character just left the game.\n");
	Do_more(c);
	Do_send_clear(c);
	return;
    }

        /* find the machine ID */
    if (c->machineID) {
        sprintf(machineID, "%ld", c->machineID);
        list_ptr = Do_look_history(c, T_MACHINE, machineID);
    }
    else {
        list_ptr = NULL;
    }

	/* get the player information */
    list_ptr_two = Do_look_history(c, T_ACCOUNT, account);

    Do_show_two_history_lists(c, list_ptr, list_ptr_two);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_saved_player_history(struct client_t *c)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 01/16/01
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

Do_saved_player_history(struct client_t *c)
{
    struct player_t thePlayer;
    struct history_list_t *list_ptr, *list_ptr_two;
    struct game_t *game_ptr;
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE];
    char characterName[SZ_NAME], account[SZ_NAME], network[SZ_FROM];
    char lcCharacterName[SZ_NAME];
    bool characterFound;
    FILE *character_file;

    if (Do_string_dialog(c, characterName, SZ_NAME - 1,
	    "What is the name of the character to query?\n") != S_NORM) {

	return;
    }

    characterFound = FALSE;

        /* load the character information */
    Do_lowercase(&lcCharacterName, &characterName);

    if (Do_look_character(c, &lcCharacterName, &thePlayer)) {

	strcpy(account, thePlayer.parent_account);
	strcpy(network, thePlayer.parent_network);
       	characterFound = TRUE;
    }

    else {

        Do_lock_mutex(&c->realm->realm_lock);

            /* run though all the games and check the names */
        game_ptr = c->realm->games;
        while (game_ptr != NULL) {

                /* check for a name match */
            if (game_ptr->description != NULL &&
                    !strcmp(lcCharacterName, game_ptr->description->lcname)) {

	        strcpy(account, game_ptr->account);
	        strcpy(network, game_ptr->network);
	        characterFound = TRUE;
	        break;
            }

            game_ptr = game_ptr->next_game;
        }
    }

    Do_unlock_mutex(&c->realm->realm_lock);

    if (!characterFound) {

	sprintf(string_buffer,
		"A character named \"%s\" could not be found.\n",
		characterName);

	Do_send_line(c, string_buffer);
	Do_more(c);
	Do_send_clear(c);
	return;
    }

    list_ptr = Do_look_history(c, T_ACCOUNT, account);

	/* if the network address is protected, only use the account */
    if (Do_check_protected(c, network)) {
	Do_show_history(c, list_ptr);
        Do_more(c);
        Do_send_clear(c);
    }
    else {
	    /* get the player information */
        list_ptr_two = Do_look_history(c, T_ADDRESS, network);
        Do_show_two_history_lists(c, list_ptr_two, list_ptr);
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_show_two_history_lists(struct client_t *c, struct history_list_t *listOne, struct history_list_t *listTwo)
/
/ FUNCTION: do trading post stuff
/
/ AUTHOR: Brian Kelly, 01/16/01
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

Do_show_two_history_lists(struct client_t *c, struct history_list_t *listOne, struct history_list_t *listTwo)
{
    struct history_list_t *temp_ptr, **list_ptr_ptr;

	/* put list_ptr_two into list_ptr in proper order */
    list_ptr_ptr = &listOne;

	/* run theough each element of the second list */
    while(listTwo != NULL) {

	    /* increment temp_ptr until at right space or NULL */
	while (*list_ptr_ptr != NULL && (*list_ptr_ptr)->theHistory.date >
		listTwo->theHistory.date) {

		/* move to the next instance */
	    list_ptr_ptr = &(*list_ptr_ptr)->next;
	}

	    /* put the account element here */
	temp_ptr = listTwo;
	listTwo = listTwo->next;
	temp_ptr->next = *list_ptr_ptr;
	*list_ptr_ptr = temp_ptr;
    }
	
    Do_show_history(c, listOne);
    Do_more(c);
    Do_send_clear(c);
}


/************************************************************************
/
/ FUNCTION NAME: Do_make_history(struct client_t *c)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 01/15/01
/
/ ARGUMENTS:
/       struct client_t c - pointer to the main client strcture
/
/ RETURN VALUE: 
/	char - character the describes the character
/
/ MODULES CALLED: strcpy()
/
/ DESCRIPTION:
/       Return a string describing the player type.
/       King, council, valar, supercedes other types.
/       The first character of the string is '*' if the player
/       has a crown.
/       If 'shortflag' is TRUE, return a 3 character string.
/
*************************************************************************/

Do_make_history(struct client_t *c)
{
    struct button_t buttons;
    struct history_t theHistory;
    struct game_t *game_ptr;
    long answer;
    char error_msg[SZ_ERROR_MESSAGE];
    char string_buffer[SZ_LINE];
    char name[SZ_FROM], tagAddress[SZ_FROM];
    bool characterFound;

    characterFound = FALSE;

	/* what does the wizard wish to note */
    Do_send_line(c,
	  "Do you wish to make note of a player, an account or an address?\n");

    strcpy(buttons.button[0], "Player\n");
    strcpy(buttons.button[1], "Account\n");
    strcpy(buttons.button[2], "Address\n");
    Do_clear_buttons(&buttons, 3);
    strcpy(buttons.button[7], "Cancel\n");

    if (Do_buttons(c, &answer, &buttons) != S_NORM || answer == 7) {
	Do_send_clear(c);
        return;
    }

    Do_send_clear(c);
    if (answer > 2 || answer < 0) {

        sprintf(error_msg,
                "[%s] Returned non-option in Do_make_history(3).\n",
                c->connection_id);

        Do_log_error(error_msg);
        Do_caught_hack(c, H_SYSTEM);
        return;
    }

    switch(answer) {

    case 0:

        if (Do_player_dialog(c, "Which player do you wish to note?\n",
		name) != S_NORM) {

            return;
        }

	characterFound = FALSE;
        Do_lock_mutex(&c->realm->realm_lock);

            /* run though all the games and check the names */
        game_ptr = c->realm->games;
        while (game_ptr != NULL) {

                /* check for a name match */
            if (game_ptr->description != NULL &&
                    !strcmp(name, game_ptr->description->name)) {

                strcpy(name, game_ptr->account);
                characterFound = TRUE;
                break;
	    }
            game_ptr = game_ptr->next_game;
        }

        Do_unlock_mutex(&c->realm->realm_lock);

            /* the character was not found */
        if (!characterFound) {

            Do_send_line(c, "That character just left the game.\n");
            Do_more(c);
            Do_send_clear(c);
            return;
        }

        theHistory.type = T_ACCOUNT;
	break;

    case 1:

        if (Do_string_dialog(c, name, SZ_NAME - 1, 
		"What is the name of the account?\n") != S_NORM) {

            return;
        }

            /* prepare the history strcture */
        theHistory.type = T_ACCOUNT;
	break;

    case 2:

        if (Do_string_dialog(c, name, SZ_NAME - 1, 
		"What is the the network?\n") != S_NORM) {

            return;
        }

            /* prepare the history strcture */
        theHistory.type = T_ADDRESS;
	break;
    }

    if (Do_string_dialog(c, string_buffer, SZ_LINE - 1, 
	    "Please enter the note.\n") != S_NORM) {

        return;
    }

    sprintf(theHistory.description, "%s wrote, \"%s\"\n", c->modifiedName,
	    string_buffer);

    theHistory.date = time(NULL);
    Do_lowercase(theHistory.name, name);
    Do_save_history(c, &theHistory);

    if (characterFound) {
        Do_lowercase(theHistory.name, tagAddress);
        theHistory.type = T_ADDRESS;
        Do_save_history(c, &theHistory);
    }

    Do_send_line(c, "The note has been entered.\n");
    Do_more(c);
    Do_send_clear(c);
    return;
}

