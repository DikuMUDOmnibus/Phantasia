/*
 * character.c       Routines to create a character
 */

#include "include.h"

/************************************************************************
/
/ FUNCTION NAME: Do_player_special_type()
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: E. A. Estes, 12/4/85
/      Brian Kelly, 4/26/99
/
/ ARGUMENTS:
/       struct client_t c - pointer to the main client strcture
/
/ RETURN VALUE: 
/    char - character the describes the character
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

char Do_player_special_type(short special_type, bool sex)
{
    /* character descriptions */
    char results[] = { ' ', 'N', 'S', 'K', 'C', 'X', 'V' };

    if (sex == FEMALE) {
        results[SC_KING] = 'Q';
    }

    return results[special_type];
}


/************************************************************************
/
/ FUNCTION NAME: Do_character_playing(struct client_t *c, char *the_name)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: E. A. Estes, 12/4/85
/      Brian Kelly, 5/4/99
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

int Do_character_playing(struct client_t *c, char *the_name)
{
    struct game_t *game_ptr;

    Do_lock_mutex(&c->realm->realm_lock);

    /* run though all the games and check the names */
    game_ptr = c->realm->games;
    while (game_ptr != NULL) {

        /* check for a name match */
    if (game_ptr->description != NULL &&
        !strcmp(the_name, game_ptr->description->lcname)) {

        Do_unlock_mutex(&c->realm->realm_lock);
        return TRUE;
    }
    game_ptr = game_ptr->next_game;
    }

    Do_unlock_mutex(&c->realm->realm_lock);
    return FALSE;
}


/***************************************************************************
/ FUNCTION NAME: struct player_t Do_copy_record(struct player_t *orig)
/
/ FUNCTION: Makes a copy of a character record
/
/ AUTHOR: Brian Kelly 4/24/99
/
/ ARGUMENTS:
/       struct player_t *orig - the original player struct
/    bool wizard - true if the wizard requested the info
/
/ RETURN VALUE: 
/    struct player_t * - a pointer to a copy of the player structure
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

struct player_t *Do_copy_record(struct player_t *orig)
{
    struct player_t *player_ptr;

    /* create a new player record */
    player_ptr = (struct player_t *) Do_malloc(SZ_PLAYER);

    /* copy the information */
    memcpy((void *)player_ptr, (void *)orig, SZ_PLAYER);

    /* return the pointer */
    return player_ptr;
}


/***************************************************************************
/ FUNCTION NAME: struct player_desc_t *Do_make_description(struct client_t *c)
/
/ FUNCTION: Makes a character description record
/
/ AUTHOR: Brian Kelly 4/26/99
/
/ ARGUMENTS:
/       struct player_t *orig - the original player struct
/
/ RETURN VALUE: 
/    struct describe_t * - pointer to the new description
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

struct player_desc_t *Do_make_description(struct client_t *c)
{
    struct player_desc_t *desc_ptr;

    /* allocate for the new description */
    desc_ptr = (struct player_desc_t *) Do_malloc(SZ_PLAYER_DESC);

    /* copy the information */
    strcpy(desc_ptr->name, c->modifiedName);
    strcpy(desc_ptr->lcname, c->player.lcname);
    strcpy(desc_ptr->parent_account, c->player.parent_account);
    desc_ptr->type = c->player.type;
    desc_ptr->special_type = c->player.special_type;
    desc_ptr->gender = c->player.gender;

    desc_ptr->level = c->player.level;
    desc_ptr->channel = c->channel;
    desc_ptr->cloaked = c->player.cloaked;
    desc_ptr->palantir = c->player.palantir;
    desc_ptr->blind = c->player.blind;

    desc_ptr->wizard = c->wizard;
/*
    if (c->wizard > 2) {
        desc_ptr->wizard = TRUE;
    }
    else {
        desc_ptr->wizard = FALSE;
    }
*/

    return desc_ptr;
}


/***************************************************************************
/ FUNCTION NAME: Do_player_description(struct client_t *c)
/
/ FUNCTION: Makes a character description record
/
/ AUTHOR: Brian Kelly 4/26/99
/
/ ARGUMENTS:
/       struct player_t *orig - the original player struct
/
/ RETURN VALUE: 
/    struct describe_t * - pointer to the new description
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_player_description(struct client_t *c)
{
    /* WARNING: Realm should be locked before calling this function */

    /* free the old description, if necessary */
    if (c->game->description != NULL) {
        free(c->game->description);
    }

    /* create a new one */
    c->game->description = Do_make_description(c);

    return;
}


/***************************************************************************
/ FUNCTION NAME: struct player_spec_t *Do_make_spec(struct client_t *c)
/
/ FUNCTION: Makes a character spec record
/
/ AUTHOR: Brian Kelly 4/26/99
/
/ ARGUMENTS:
/       struct client_t *c - the main client strcture
/
/ RETURN VALUE: 
/    struct player_spec_t * - pointer to the new specifications
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

struct player_spec_t *Do_make_specification(struct client_t *c)
{
    struct player_spec_t *spec_ptr;

    /* allocate for the new description */
    spec_ptr = (struct player_spec_t *) Do_malloc(SZ_PLAYER_SPEC);

    /* copy the information */
    strcpy(spec_ptr->name, c->modifiedName);
    strcat(spec_ptr->name, "\n");

    spec_ptr->type[0] = c->realm->charstats[c->player.type].short_class_name;

    if (c->wizard > 2) {
        spec_ptr->type[1] = 'W';
        spec_ptr->type[2] = ' ';
        spec_ptr->type[3] = '-';
        spec_ptr->type[4] = c->channel + 48;
        spec_ptr->type[5] = '\n';
        spec_ptr->type[6] = '\0';
    }
    else if (c->player.special_type == SC_NONE) {
      if (c->wizard == 2) {
        spec_ptr->type[1] = 'A';
        spec_ptr->type[2] = ' ';
        spec_ptr->type[3] = '-';
        spec_ptr->type[4] = c->channel + 48;
        spec_ptr->type[5] = '\n';
        spec_ptr->type[6] = '\0';
      } else {
        spec_ptr->type[1] = ' ';
        spec_ptr->type[2] = '-';
        spec_ptr->type[3] = c->channel + 48;
        spec_ptr->type[4] = '\n';
        spec_ptr->type[5] = '\0';
      }
   
    }
    else {

        spec_ptr->type[1] = Do_player_special_type(c->player.special_type,
        c->player.gender);

        spec_ptr->type[2] = ' ';
        spec_ptr->type[3] = '-';
        spec_ptr->type[4] = c->channel + 48;
        spec_ptr->type[5] = '\n';
        spec_ptr->type[6] = '\0';
    }

    /* return the pointer */
    return spec_ptr;
}


/***************************************************************************
/ FUNCTION NAME: Do_send_spec(struct client_t *c)
/
/ FUNCTION: Makes a character spec record
/
/ AUTHOR: Brian Kelly 5/16/99
/
/ ARGUMENTS:
/       struct client_t *c - the main client strcture
/
/ RETURN VALUE: 
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_send_specification(struct client_t *c, long int type)
{
    struct event_t *event_ptr;

    /* create a new event */
    event_ptr = (struct event_t *) Do_create_event();

    /* fill it out */
    event_ptr->type = type;
    event_ptr->arg3 = (long) SZ_PLAYER_SPEC;
    event_ptr->arg4 = (void *) Do_make_specification(c);
    event_ptr->from = c->game;

    /* send the event to everyone */
    Do_broadcast_event(c, event_ptr);

    return;
}


/***************************************************************************
/ FUNCTION NAME: Do_starting_spec(struct client_t *c)
/
/ FUNCTION: Makes a character spec record
/
/ AUTHOR: Brian Kelly 8/15/99
/
/ ARGUMENTS:
/       struct client_t *c - the main client strcture
/
/ RETURN VALUE: 
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_starting_spec(struct client_t *c)
{
    /* WARNING: the realm should be locked before calling this function */
    char string_buffer[SZ_LINE];
    struct game_t *game_ptr;
    struct player_spec_t *spec_ptr;
    struct event_t *event_ptr;

    /* run through the list of games */
    game_ptr = c->realm->games;
    while (game_ptr != NULL) {

        /* see if the game is currently playing */
    if (game_ptr->description != NULL) {

        /* create a spec for this player */
        spec_ptr = (struct player_spec_t *) Do_malloc(SZ_PLAYER_SPEC);

            /* copy the information */
        strcpy(spec_ptr->name, game_ptr->description->name);
        strcat(spec_ptr->name, "\n");

        spec_ptr->type[0] = c->realm->charstats[game_ptr->description->type].short_class_name;

        if (game_ptr->description->wizard > 2) {
        spec_ptr->type[1] = 'W';
            spec_ptr->type[2] = ' ';
            spec_ptr->type[3] = '-';
            spec_ptr->type[4] = game_ptr->description->channel + 48;
            spec_ptr->type[5] = '\n';
            spec_ptr->type[6] = '\0';
        }
        else if (game_ptr->description->special_type) {

        spec_ptr->type[1] = Do_player_special_type(
            game_ptr->description->special_type,
            game_ptr->description->gender);

            spec_ptr->type[2] = ' ';
            spec_ptr->type[3] = '-';
            spec_ptr->type[4] = game_ptr->description->channel + 48;
            spec_ptr->type[5] = '\n';
            spec_ptr->type[6] = '\0';
        }
        else if (game_ptr->description->wizard == 2) {
        spec_ptr->type[1] = 'A';
            spec_ptr->type[2] = ' ';
            spec_ptr->type[3] = '-';
            spec_ptr->type[4] = game_ptr->description->channel + 48;
            spec_ptr->type[5] = '\n';
            spec_ptr->type[6] = '\0';
        }
        else {
            spec_ptr->type[1] = ' ';
            spec_ptr->type[2] = '-';
            spec_ptr->type[3] = game_ptr->description->channel + 48;
            spec_ptr->type[4] = '\n';
            spec_ptr->type[5] = '\0';
        }

        /* create a new event */
        event_ptr = (struct event_t *) Do_create_event();

        /* fill it out */
        event_ptr->type = ADD_PLAYER_EVENT;
        event_ptr->arg3 = (long) SZ_PLAYER_SPEC;
        event_ptr->arg4 = (void *) spec_ptr;
        event_ptr->from = game_ptr;

        /* send the event */
        Do_file_event(c, event_ptr);
    }

        /* move to the next game */
    game_ptr = game_ptr->next_game;
    }

    return;
}


/***************************************************************************
/ FUNCTION NAME: Do_get_character(struct client_t *c)
/
/ FUNCTION: initialize state, and call main process
/
/ AUTHOR: E. A. Estes, 12/4/85
/      Brian Kelly 4/23/99
/
/ ARGUMENTS:
/       struct client_t *c - the main client strcture
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_get_character(struct client_t *c)
{
    struct button_t buttons;
    char    error_msg[SZ_ERROR_MESSAGE];
    int i, rc;
    long answer;

    for (;;) {

        /* see if the player has a saved game */
    Do_send_line(c, "What do you wish to do?\n");

    strcpy(buttons.button[0], "New Char\n");
    strcpy(buttons.button[1], "Load Char\n");
    Do_clear_buttons(&buttons, 2);
    strcpy(buttons.button[3], "Characters\n");
    strcpy(buttons.button[4], "Account\n");
    strcpy(buttons.button[6], "Scoreboard\n");
    strcpy(buttons.button[7], "Quit\n");

    rc = Do_buttons(c, &answer, &buttons);
    Do_send_clear(c);

    if (rc != S_NORM) {
        answer = 7;
    }

        /* switch on the player's answer */
    switch (answer) {

        /* if the player has a character to run */
    case 1:

        /* go recall the player */
        Do_recall_player(c);
        break;

        /* if the player does not have a character to run */
    case 0:

        /* create a new character */
        Do_roll_new_player(c);
        break;

        /* character options */
    case 3:

        Do_character_options(c);
        break;

        /* account options */
    case 4:

        Do_account_options(c);
        break;

        /* show the character the scoreboard */
    case 6:

        Do_scoreboard(c, 0);
        break;

        /* exit if requested */
    case 7:

        c->run_level = EXIT_THREAD;
        break;

        /* since it's a push button interface,
            any other answer is a hacker */
    default:

        sprintf(error_msg,
            "[%s] Returned non-option in Do_get_character.\n",
            c->connection_id);

        Do_log_error(error_msg);
        Do_caught_hack(c, H_SYSTEM);
    }
    return;
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_roll_new_player(struct client_t *c)
/
/ FUNCTION: roll up a new character
/
/ AUTHOR: E. A. Estes, 12/4/85
/      Brian Kelly, 5/4/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: initplayer(), allocrecord(), truncstring(), fabs(), wmove(),
/       wclear(), sscanf(), strcmp(), genchar(), waddstr(), findname(), mvprintw(),
/       getanswer(), getstring()
/
/ DESCRIPTION:
/       Prompt player, and roll up new character.
/
*************************************************************************/

Do_roll_new_player(struct client_t *c)
{
    struct charstats_t *stat_ptr;
    struct game_t *game_ptr;
    int     i, theMask, suffixFlag;  
    double x, y;
    char answer, string_buffer[SZ_LINE], string_buffer2[SZ_LINE]; 
    struct button_t buttons;
    char    error_msg[SZ_ERROR_MESSAGE];
    long theAnswer;
    FILE *liar_file;
    struct in_addr theNetwork;

    /* muted characters would make characters with message names */
    if (c->muteUntil > time(NULL)) {
    Do_send_line(c, "Muted players may not create characters.\n");
    Do_more(c);
    Do_send_clear(c);

        return;
    }

    /* get the character type */
    Do_send_line(c, "Which type of character do you want?\n");

    for (i = 0; i < NUM_CHARS; i++) {

        /* copy over the class name */
        sprintf(buttons.button[i], "%s\n",
            c->realm->charstats[i].class_name);
    }

    Do_clear_buttons(&buttons, NUM_CHARS);
    strcpy(buttons.button[7], "Cancel\n");

    /* send the information */
    if (Do_buttons(c, &theAnswer, &buttons) != S_NORM || theAnswer == 7) {
        Do_send_clear(c);
    return;
    }

    Do_send_clear(c);
    c->player.type = theAnswer;

    /* if the information is out of range, we have a hacker */
    if (c->player.type < 0 && c->player.type >= NUM_CHARS) {

        sprintf(error_msg, "[%s] Returned non-option in Do_roll_new_player.\n",
        c->connection_id);

    Do_log_error(error_msg);
    Do_caught_hack(c, H_SYSTEM);
    return;
    }

    stat_ptr = &c->realm->charstats[c->player.type];

    for (;;) {

        /* give a random x and y */
    x = 0;
    y = 0;
    Do_move_close(&x, &y, D_CIRCLE - 1);

    if (c->player.type != C_EXPER) {
        Do_location(c, x, y, FALSE);
    }

        c->player.energy =
                ROLL(stat_ptr->energy.base, stat_ptr->energy.interval);

    Do_energy(c, c->player.energy, c->player.energy, 0.0, 0.0, FALSE);

    Do_strength(c, ROLL(stat_ptr->strength.base,
        stat_ptr->strength.interval), 0.0, 0.0, FALSE);

        Do_speed(c, ROLL(stat_ptr->quickness.base,
        stat_ptr->quickness.interval), 0.0, 0.0, FALSE);

    Do_mana(c, ROLL(stat_ptr->mana.base, stat_ptr->mana.interval) -
        c->player.mana, FALSE);

        /* set the gold manually so no taxes are taken */
    c->player.gold = ROLL(50.0, 75.0);
    Do_gold(c, 0.0, TRUE);

        c->player.brains =
            ROLL(stat_ptr->brains.base, stat_ptr->brains.interval);

        c->player.magiclvl =
            ROLL(stat_ptr->magiclvl.base, stat_ptr->magiclvl.interval);

        /* experimento's have fixed stats */
        if (c->player.type == C_EXPER)
            break;

    sprintf(string_buffer, "Brains      : %2.0f\n", c->player.brains);
        Do_send_line(c, string_buffer);

    sprintf(string_buffer, "Magic Level : %2.0f\n", c->player.magiclvl);
        Do_send_line(c, string_buffer);

        if (c->player.type == C_HALFLING) {

                /* give halfling some experience */
            c->player.experience = 0;
            Do_experience(c, ROLL(600.0, 200.0), FALSE);

        sprintf(string_buffer, "Experience  : %2.0f\n",
            c->player.experience);

            Do_send_line(c, string_buffer);
        }

        /* see if the player wants to keep the character */
        Do_send_line(c, "Do you wish to keep these stats?\n");

    strcpy(buttons.button[0], "Reroll\n");
    strcpy(buttons.button[1], "Keep\n");
    Do_clear_buttons(&buttons, 2);
    strcpy(buttons.button[7], "Cancel\n");

            /* see if the game is shutting down */
        Do_shutdown_check(c);

    if (Do_buttons(c, &theAnswer, &buttons) != S_NORM || theAnswer == 7) {
        Do_send_clear(c);
        return;
    }

    else if (theAnswer == 1) {
            Do_send_clear(c);
        break;
    }

    else if (theAnswer != 0) {
            Do_send_clear(c);

            sprintf(error_msg,
            "[%s] Returned non-option in Do_roll_new_player(2).\n",
            c->connection_id);

        Do_log_error(error_msg);
        Do_caught_hack(c, H_SYSTEM);
        return;
        }
        Do_send_clear(c);
    }

        /* get coordinates for experimento */
    if (c->player.type == C_EXPER) {

        for (;;) {

            if (Do_coords_dialog(c, &x, &y,
                "Enter the approximate X Y coordinates of your experimento?\n")) {

        return;
        }

            if (fabs(x) > D_EXPER || fabs(y) > D_EXPER) {

                Do_send_line(c, "Experimento starting coordinates must be between -2000 and 2000.  Please try again.\n");

        Do_more(c);
        Do_send_clear(c);
        }
            else {
                break;
        }
        }

            /* experimentos never quite start where they wish */
    Do_move_close(&x, &y, 5);

    if (x == 0 && y == 0) {
        Do_move_close(&x, &y, 0);
    }

    Do_location(c, x, y, FALSE);
    }

    for (;;) {

            /* name the new character */
        sprintf(string_buffer,
        "Give your character a name. [up to %d characters]\n", MAX_NAME_LEN);

    if (Do_string_dialog(c, c->player.name, SZ_NAME - 1, string_buffer))
        return;

        /* see if the name is approved */
    Do_lowercase(&c->player.lcname, &c->player.name);

    if (Do_approve_name(c, &c->player.lcname, &c->player.name,
        &theAnswer) != S_NORM) {

        return;
    }

    if (theAnswer) {
        break;
    }
    }

    strcpy(c->modifiedName, c->player.name);
    Do_name(c);

    /* determine the sex of the charcter */
    Do_send_line(c, "Is the character male or female?\n");

    strcpy(buttons.button[0], "Male\n");
    strcpy(buttons.button[1], "Female\n");
    Do_clear_buttons(&buttons, 2);
    strcpy(buttons.button[7], "Cancel\n");

    if (Do_buttons(c, &theAnswer, &buttons) != S_NORM || theAnswer == 7) {
    Do_release_name(c, c->player.lcname);
    Do_send_clear(c);
    return;
    }

    Do_send_clear(c);

    if (theAnswer == 1)
    c->player.gender = FEMALE;
    else
    c->player.gender = MALE;

        /* get a password for character */
    if (!Do_new_password(c, c->player.password, "character")) {

    Do_release_name(c, c->player.lcname);
    return;
    }

    /* creation complete */
    c->run_level = PLAY_GAME;
    c->player.date_created = time(NULL);

    /* put a description in place */
    Do_lock_mutex(&c->realm->realm_lock);
    Do_player_description(c);
    Do_unlock_mutex(&c->realm->realm_lock);

    /* remove the name in limbo now the description is in place */
    Do_release_name(c, c->player.lcname);

    /* log the creation */
    sprintf(string_buffer, "[%s] %s created by %s\n", c->connection_id,
        c->player.lcname, c->lcaccount);

    Do_log(GAME_LOG, &string_buffer);

    sprintf(string_buffer, "%s, %s, 0 age, 0 seconds, level 0\n",
        c->player.lcname, c->realm->charstats[c->player.type].class_name);

    Do_log(LEVEL_LOG, &string_buffer);

    return;
}

/************************************************************************
/
/ FUNCTION NAME: Do_recall_player()
/
/ FUNCTION: find a character on file
/
/ AUTHOR: E. A. Estes, 12/4/85
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: writerecord(), truncstring(), more(), death(), wmove(),
/       wclear(), strcmp(), printw(), cleanup(), waddstr(), findname(), mvprintw (),
/       getanswer(), getstring()
/
/ GLOBAL INPUTS: Player, *stdscr, Databuf[]
/
/ GLOBAL OUTPUTS: Echo, Player
/
/ DESCRIPTION:
/       Search for a character of a certain name, and check password.
/
*************************************************************************/

Do_recall_player(struct client_t *c)
{
    char characterName[SZ_NAME], string_buffer[SZ_LINE]; 
    char lcCharacterName[SZ_NAME];
    char error_msg[SZ_ERROR_MESSAGE];
    struct player_mod_t theMod;
    long answer;
    int exceptionFlag, wizType = 0;
    char wizNetwork[SZ_FROM], wizAccount[SZ_NAME], wizCharacter[SZ_NAME];
    FILE *wizard_file;

        /* open the wizard file to see if this person is one */
    if ((wizard_file=fopen(WIZARD_FILE, "r")) == NULL) {

        sprintf(string_buffer,
            "[%s] fopen of %s failed in Do_recall_player: %s\n",
            c->connection_id, WIZARD_FILE, strerror(errno));

        Do_log_error(string_buffer);
    }
    else {

            /* loop through the the names */
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

    for (;;) {

        /* prompt for the character */
        if (Do_string_dialog(c, characterName, SZ_NAME - 1,
            "What is the name of the character?\n")) {

            return;
        }

        /* load the character information */
        Do_lowercase(&lcCharacterName, &characterName);
        Do_send_line(c, "Looking up character information...\n");
        Do_send_buffer(c);

        if (strcmp("", lcCharacterName) == 0) {
        } else if (Do_look_character(c, lcCharacterName, &c->player)) {
            Do_send_clear(c);
            break;
        }

        Do_send_clear(c);

		/* see if the character is playing */
	if (Do_character_playing(c, lcCharacterName)) {

	    Do_send_line(c, "That character is currently in the game.  If you were just playing this character and was disconnected, wait a minute and the character will either be saved or killed.\n");

	} else {

            sprintf(string_buffer, "I can not find a character named \"%s\".  Please check the spelling and try again.\n", characterName);

            Do_send_line(c, string_buffer);
        }

        Do_more(c);
        Do_send_clear(c);
    }

    /* if character is faithful, make sure this is parent account */
    /* allow wizards to load protected characters */
    if (wizType < 3 && c->player.faithful
        && strcmp(c->player.parent_account, c->lcaccount)) {

            /* this is a possible hack attempt, so log it */
        Do_clear_character_mod(&theMod);
        theMod.badPassword = TRUE;
        Do_modify_character(c, lcCharacterName, &theMod);

    sprintf(string_buffer, "The character named \"%s\" can only be loaded from the account that created it.\n", characterName);

        Do_send_line(c, string_buffer);
        Do_more(c);
    Do_send_clear(c);
    return;
    }

    /* found character - now get the password */
    if (!Do_request_character_password(c, c->player.password, c->player.name,
        c->player.lcname, wizType)) {

        return;
    }

        /* put a description in place - now so nobody can use the name */
    Do_lock_mutex(&c->realm->realm_lock);
    Do_player_description(c);
    Do_unlock_mutex(&c->realm->realm_lock);

    /* load the character */
    Do_send_line(c, "Loading the character...\n");
    Do_send_buffer(c);
    if (!Do_load_character(c, lcCharacterName)) {

        /* if false returns, the character was not loaded */
    Do_send_clear(c);

            /* erase the description just put in */
    Do_lock_mutex(&c->realm->realm_lock);
    free(c->game->description);
    c->game->description = NULL;
    Do_unlock_mutex(&c->realm->realm_lock);

        /* inform the user */
    sprintf(string_buffer, "The character %s is no longer in the character file.  This is normally because the character was just loaded by someone else.\n", c->player.name);

    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
    return;
    }

    Do_send_clear(c);

        /* backup the character */
    Do_backup_save(c, TRUE);

    /* import the character to this account if none */
    if (c->player.parent_account[0] == '\0') {

    strcpy(c->player.parent_account, c->lcaccount);
    strcpy(c->player.parent_network, c->network);
    c->player.faithful = TRUE;

    sprintf(string_buffer, "The character %s has been imported into account %s.  Future modifications to this character must be done through this account.\n", c->player.name, c->account);

    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
    }

    c->run_level = PLAY_GAME;

    /* log the load */
    sprintf(string_buffer, "[%s] %s loaded by %s\n", c->connection_id,
        c->player.lcname, c->lcaccount);

    Do_log(GAME_LOG, &string_buffer);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_look_character(struct client_t *c, char *the_name, struct player_t *thePlayer)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 1/3/01
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

int Do_look_character(struct client_t *c, char *the_name, struct player_t *thePlayer)
{

    FILE *character_file;
    char error_msg[SZ_ERROR_MESSAGE];

    Do_lock_mutex(&c->realm->character_file_lock);

    errno = 0;
    if ((character_file=fopen(CHARACTER_FILE, "r")) == NULL) {

        Do_unlock_mutex(&c->realm->character_file_lock);
    sprintf(error_msg,
        "[%s] fopen of %s failed in Do_look_character: %s\n",
        c->connection_id, CHARACTER_FILE, strerror(errno));

    Do_log_error(error_msg);
    return FALSE;
    }

    /* loop through the the files */
    while (fread((void *)thePlayer, SZ_PLAYER, 1, character_file) == 1) {

        if (strcmp(thePlayer->lcname, the_name) == 0) {
        fclose(character_file);
            Do_unlock_mutex(&c->realm->character_file_lock);
            return TRUE;
        }
    }

    fclose(character_file);
    Do_unlock_mutex(&c->realm->character_file_lock);
    return FALSE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_load_character(struct client_t *c, char *the_name)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: E. A. Estes, 12/4/85
/      Brian Kelly, 5/4/99
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

int Do_load_character(struct client_t *c, char *the_name)
{

    struct player_t readPlayer;
    FILE *character_file, *temp_file;
    char error_msg[SZ_ERROR_MESSAGE];
    time_t timeNow;
    bool char_flag;

    Do_lock_mutex(&c->realm->character_file_lock);

    errno = 0;
    if ((character_file=fopen(CHARACTER_FILE, "r")) == NULL) {

        Do_unlock_mutex(&c->realm->character_file_lock);
    sprintf(error_msg, "[%s] fopen of %s failed in Do_load_character: %s\n",
        c->connection_id, CHARACTER_FILE, strerror(errno));

    Do_log_error(error_msg);
    return FALSE;
    }

    /* create a temporary file */
    errno = 0;
    if ((temp_file=fopen(TEMP_CHARACTER_FILE, "w")) == NULL) {

    sprintf(error_msg,
        "[%s] fopen of %s failed in Do_load_character(2): %s\n",
        c->connection_id, TEMP_CHARACTER_FILE, strerror(errno));

    Do_log_error(error_msg);
    fclose(character_file);
        Do_unlock_mutex(&c->realm->character_file_lock);
    return FALSE;
    }

    char_flag = FALSE;
    timeNow = time(NULL);

        /* read each line of the character file */
    while (fread((void *)&readPlayer, SZ_PLAYER, 1, character_file) == 1) {

            /* if we find our character, copy it over */
        if (strcmp(readPlayer.lcname, the_name) == 0) {
        memcpy(&c->player, &readPlayer, SZ_PLAYER);
        char_flag = TRUE;
        continue;
        }

        /* see if the character has expired */

    else if ((readPlayer.level < 2) && 
                 (timeNow - readPlayer.last_load > NEWBIE_KEEP_TIME)) {

                /* log the deletion of the account */
        sprintf(error_msg, "[%s] %s deleted\n", c->connection_id,
            readPlayer.lcname);

        Do_log(GAME_LOG, &error_msg);
        continue;
    }
    else if ((readPlayer.level >= 2) && 
                 (timeNow - readPlayer.last_load > 
                         KEEP_TIME + (readPlayer.degenerated * 172800))) {

                /* log the deletion of the account */
        sprintf(error_msg, "[%s] %s deleted\n", c->connection_id,
            readPlayer.lcname);

        Do_log(GAME_LOG, &error_msg);
        continue;
    }

      /* delete characters that are from the future */
    if (timeNow < readPlayer.last_load) {

                /* log the deletion of the account */
        sprintf(error_msg, "[%s] %s from the future and deleted\n",
            c->connection_id, readPlayer.lcname);

        Do_log(GAME_LOG, &error_msg);
        continue;
    }

        errno = 0;
    if (fwrite((void *)&readPlayer, SZ_PLAYER, 1, temp_file)
        != 1) {

        sprintf(error_msg,
            "[%s] fwrite of %s failed in Do_load_character: %s\n",
            c->connection_id, TEMP_CHARACTER_FILE, strerror(errno));

        Do_log_error(error_msg);
        fclose(character_file);
        fclose(temp_file);
        remove(TEMP_CHARACTER_FILE);
                Do_unlock_mutex(&c->realm->character_file_lock);
        return FALSE;
        }
    }

    /* delete the old character record */
    remove(CHARACTER_FILE);

    /* replace it with the temporary file */
    rename(TEMP_CHARACTER_FILE, CHARACTER_FILE);

    /* close the file handles */
    fclose(temp_file);
    fclose(character_file);
    Do_unlock_mutex(&c->realm->character_file_lock);

    /* if the character was found */
    if (char_flag) {

/* There's an overflow with the mute count somewhere -- EH & BK - 1/6/02 */
if (c->player.muteCount > 6 || c->player.muteCount < 0) {
  c->player.muteCount = 0;
}

        return TRUE;
    }

    return FALSE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_save_character(struct client_t *c, struct player_t *thePlayer)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: E. A. Estes, 12/4/85
/      Brian Kelly, 5/4/99
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

int Do_save_character(struct client_t *c, struct player_t *thePlayer)
{
    FILE *character_file;
    char error_msg[SZ_ERROR_MESSAGE];
    char string_buffer[SZ_LINE];

    Do_lock_mutex(&c->realm->character_file_lock);

    errno = 0;
    if ((character_file=fopen(CHARACTER_FILE, "a")) == NULL) {

        Do_unlock_mutex(&c->realm->character_file_lock);

    sprintf(error_msg,
        "[%s] fopen of %s failed in Do_save_character: %s\n",
        c->connection_id, CHARACTER_FILE, strerror(errno));

    Do_log_error(error_msg);

    Do_send_line(c, "There was an error opening the character file to save your character.  Please contact the game administrator with this problem.\n");

    Do_more(c);
    Do_send_clear(c);
    return;
    }

    /* write the player record to the end of the character file */
    errno = 0;
    if (fwrite((void *)thePlayer, SZ_PLAYER, 1, character_file) != 1) {

    fclose(character_file);
        Do_unlock_mutex(&c->realm->character_file_lock);

    sprintf(error_msg,
        "[%s] fwrite of %s failed in Do_save_character: %s\n",
        c->connection_id, CHARACTER_FILE, strerror(errno));

    Do_log_error(error_msg);

    Do_send_line(c, "There was an error opening the character file to save your character.  Please contact the game administrator with this problem.\n");

    Do_more(c);
    Do_send_clear(c);
    return;
    }

    /* close the character file */
    fclose(character_file);
    Do_unlock_mutex(&c->realm->character_file_lock);
}


/************************************************************************
/
/ FUNCTION NAME: Do_backup_save(struct client_t *c)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 11/25/99
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

Do_backup_save(struct client_t *c, int backup)
{
    FILE *backup_file;
    FILE *temp_backup_file;
    struct player_t the_player;
    char error_msg[SZ_ERROR_MESSAGE];

    Do_lock_mutex(&c->realm->backup_lock);

    errno = 0;
    if ((temp_backup_file=fopen(TEMP_BACKUP_FILE, "w")) == NULL) {

        Do_unlock_mutex(&c->realm->backup_lock);
    sprintf(error_msg, "[%s] fopen of %s failed in Do_backup_save: %s\n",
        c->connection_id, TEMP_BACKUP_FILE, strerror(errno));

    Do_log_error(error_msg);
    return;
    }

    /* if we're supposed to back up our charcter */
    if (backup) {

        /* write the player record to the end of the character file */
        errno = 0;
        if (fwrite((void *)&c->player, SZ_PLAYER, 1, temp_backup_file) != 1) {

        sprintf(error_msg,
            "[%s] fwrite of %s failed in Do_backup_save: %s\n",
            c->connection_id, TEMP_BACKUP_FILE, strerror(errno));

        Do_log_error(error_msg);
        fclose(temp_backup_file);
            Do_unlock_mutex(&c->realm->backup_lock);
        return;
        }
    }

    /* open the real backup file */
    errno = 0;
    if ((backup_file=fopen(BACKUP_FILE, "r")) == NULL) {

    sprintf(error_msg,
        "[%s] fopen of %s failed in Do_backup_save: %s\n",
        c->connection_id, BACKUP_FILE, strerror(errno));

    Do_log_error(error_msg);
    }
    else {

        /* read each line of the old backup file */
        errno = 0;
        while (fread((void *)&the_player, SZ_PLAYER, 1, backup_file) == 1) {

            /* if its anyone elses character, copy it over */
            if (strcmp(the_player.lcname, c->player.lcname)) {
        
            if (fwrite((void *)&the_player, SZ_PLAYER, 1, temp_backup_file)
            != 1) {

                sprintf(error_msg,
             "[%s] fwrite of %s failed in Do_backup_save(2): %s\n",
             c->connection_id, TEMP_BACKUP_FILE, strerror(errno));

            Do_log_error(error_msg);
            fclose(backup_file);
            fclose(temp_backup_file);
                    Do_unlock_mutex(&c->realm->backup_lock);
            return;
            }
        }
    }
        remove(BACKUP_FILE);
        fclose(backup_file);
    }

        /* remove and replace the old backup file */
    rename(TEMP_BACKUP_FILE, BACKUP_FILE);
    fclose(temp_backup_file);
    Do_unlock_mutex(&c->realm->backup_lock);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_approve_name(struct client_t *c, char *lcname, char *name, int *answer)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 1/1/01
/
/ ARGUMENTS:
/       struct client_t c - pointer to the main client strcture
/
/ RETURN VALUE:
/       char - character the describes the character
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

int Do_approve_name(struct client_t *c, char *lcname, char *name, int *answer)
{
    int len, i, underscore_count;
    bool space_flag, double_underscore;
    char string_buffer[SZ_LINE];
    struct account_t readAccount;
    struct player_t readPlayer;
    struct game_t *game_ptr;
    struct linked_list_t *list_ptr;
    FILE *the_file;

    /* just stop right now if the name is null */
    len = strlen(lcname);
    if (len == 0) {

        sprintf(string_buffer, "Please enter the name you would like into the dialog box.\n", lcname);

          Do_send_line(c, string_buffer);
        Do_more(c);
    Do_send_clear(c);
        *answer = FALSE;
        return S_NORM;
    }

        /* see if the name is too long */
    if (len > MAX_NAME_LEN)  {
         sprintf(string_buffer, "\"%s\" is too long.  Please use %d characters or less.\n", name, MAX_NAME_LEN);

         Do_send_line(c, string_buffer);
         Do_more(c);
         Do_send_clear(c);
         *answer = FALSE;
         return S_NORM;
    }


    /* see if we have this name reserved */
    if (c->previousName[0] != '\0') {

        /* see if this name is reserved for us */
        if (strcmp(c->previousName, lcname) == 0) {
            c->previousName[0] = '\0';
            *answer = TRUE;
            return S_NORM;
        }

        /* remove the reserved name from limbo */
        Do_release_name(c, c->previousName);
        c->previousName[0] = '\0';
    }

        /* see if the name looks okay */
    space_flag = FALSE;
    double_underscore = FALSE;
    underscore_count = 0;
    for (i = 0; i < len ; i++) {

    if (isalnum(lcname[i])) {
        double_underscore = FALSE;
    }
    else {
        if (lcname[i] == '_') {

        ++underscore_count;

        if (double_underscore) {

                sprintf(string_buffer, "\"%s\" uses underscores together.  Please use only one underscore to represent a space.\n", name);

                Do_send_line(c, string_buffer);
                Do_more(c);
                Do_send_clear(c);
                *answer = FALSE;
                return S_NORM;
        }
        else {
            double_underscore = TRUE;
        }
        }
        else {

            /* An error, but hold off in case of other bad chars */
            if (lcname[i] == ' ') {
                space_flag = TRUE;
            }
            else {

                sprintf(string_buffer, "\"%s\" contains invalid characters.  You may only use letters, numbers and underscores.  Please use a different name.\n", name);

                Do_send_line(c, string_buffer);
                Do_more(c);
                Do_send_clear(c);
                *answer = FALSE;
                return S_NORM;
        }
        }
    }
    }

    if (space_flag) {

        sprintf(string_buffer, "\"%s\" uses spaces which is not permitted.  Please use underscores instead of spaces.\n", name);

        Do_send_line(c, string_buffer);
        Do_more(c);
        Do_send_clear(c);
        *answer = FALSE;
        return S_NORM;
    }

    if (lcname[0] == '_' || lcname[len] == '_') {

        sprintf(string_buffer, "\"%s\" is not using underscores to separate characters.  Please remove them if they are not going to be used as a space.\n", name);

        Do_send_line(c, string_buffer);
        Do_more(c);
        Do_send_clear(c);
        *answer = FALSE;
        return S_NORM;
    }

    if ((double)underscore_count / (double)len >= .25) {

        sprintf(string_buffer, "\"%s\" contains too many underscores.  Please use underscores only to separate words.\n", name);

        Do_send_line(c, string_buffer);
        Do_more(c);
        Do_send_clear(c);
        *answer = FALSE;
        return S_NORM;
    }

    /* the name is of an acceptable format - see if it is profane */
    if (Do_profanity_check(lcname)) {

        sprintf(string_buffer, "\"%s\" does not get past the profanity checker.  Please choose a different name.\n", lcname);

        Do_send_line(c, string_buffer);
        Do_more(c);
        Do_send_clear(c);
        *answer = FALSE;
        return S_NORM;
    }

    /* start checking accounts to lock the realm as little as possible */
    errno = 0;
    Do_lock_mutex(&c->realm->account_lock);
    if ((the_file=fopen(ACCOUNT_FILE, "r")) == NULL) {

        sprintf(string_buffer, "[%s] fopen of %s failed Do_approve_name: %s\n",
                c->connection_id, ACCOUNT_FILE, strerror(errno));

        Do_log_error(string_buffer);
    }
    else {

            /* run through each entry and compare */
        while (fread((void *)&readAccount, SZ_ACCOUNT, 1, the_file) == 1) {

            if (strcmp(readAccount.lcname, lcname) == 0) {

                fclose(the_file);
                Do_unlock_mutex(&c->realm->account_lock);

                sprintf(string_buffer, "The name \"%s\" has already been taken.  Please choose another.\n", name);

                Do_send_line(c, string_buffer);
                Do_more(c);
                Do_send_clear(c);
                *answer = FALSE;
                return S_NORM;
            }
        }

        fclose(the_file);
    }

    /* We have to lock the realm now */
    Do_lock_mutex(&c->realm->realm_lock);

    /* start looking through the games in play */
    game_ptr = c->realm->games;

        /* run through all addresses in limbo */
    while (game_ptr != NULL) {

        if (game_ptr->description != NULL && strcmp(lcname,
        game_ptr->description->lcname) == 0) {

            Do_unlock_mutex(&c->realm->realm_lock);
            Do_unlock_mutex(&c->realm->account_lock);

            sprintf(string_buffer, "The name \"%s\" has already been taken.  Please choose another.\n", name);

            Do_send_line(c, string_buffer);
            Do_more(c);
            Do_send_clear(c);
            *answer = FALSE;
            return S_NORM;
        }

        game_ptr = game_ptr->next_game;
    }

    /* now look through the character file */
    errno = 0;
    Do_lock_mutex(&c->realm->character_file_lock);
    if ((the_file=fopen(CHARACTER_FILE, "r")) == NULL) {

        sprintf(string_buffer,
        "[%s] fopen of %s failed in Do_approve_name: %s\n",
                c->connection_id, CHARACTER_FILE, strerror(errno));

        Do_log_error(string_buffer);
    }
    else {

            /* run through each entry and compare */
        while (fread((void *)&readPlayer, SZ_PLAYER, 1, the_file) == 1) {

            if (strcmp(readPlayer.lcname, lcname) == 0) {

                fclose(the_file);
                Do_unlock_mutex(&c->realm->character_file_lock);
            Do_unlock_mutex(&c->realm->realm_lock);
                Do_unlock_mutex(&c->realm->account_lock);

                sprintf(string_buffer, "The name \"%s\" has already been taken.  Please choose another.\n", name);

                Do_send_line(c, string_buffer);
                Do_more(c);
        Do_send_clear(c);
                *answer = FALSE;
                return S_NORM;
            }
        }

        fclose(the_file);
    }

    /* start looking through names in limbo */
    list_ptr = c->realm->name_limbo;

        /* run through all addresses in limbo */
    while (list_ptr != NULL) {

        if (strcmp(list_ptr->name, lcname) == 0) {

            Do_unlock_mutex(&c->realm->character_file_lock);
        Do_unlock_mutex(&c->realm->realm_lock);
            Do_unlock_mutex(&c->realm->account_lock);

            sprintf(string_buffer, "The name \"%s\" is currently being registered by another player.  Please choose another.\n", name);

            Do_send_line(c, string_buffer);
            Do_more(c);
        Do_send_clear(c);
            *answer = FALSE;
            return S_NORM;
        }

    list_ptr = list_ptr->next;
    }

        /* name address checks out.  Put ours in limbo */
    list_ptr = (struct linked_list_t *) Do_malloc(SZ_LINKED_LIST);

    strcpy(list_ptr->name, lcname);
    list_ptr->next = c->realm->name_limbo;
    c->realm->name_limbo = list_ptr;

    Do_unlock_mutex(&c->realm->character_file_lock);
    Do_unlock_mutex(&c->realm->realm_lock);
    Do_unlock_mutex(&c->realm->account_lock);

    *answer = TRUE;
    return S_NORM;
}


/************************************************************************
/
/ FUNCTION NAME: Do_release_name(struct client_t *c, char *name);
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 1/1/01
/
/ ARGUMENTS:
/       struct client_t c - pointer to the main client strcture
/
/ RETURN VALUE:
/       char - character the describes the character
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

Do_release_name(struct client_t *c, char *name)
{
    char error_msg[SZ_ERROR_MESSAGE];
    struct linked_list_t *list_ptr, **list_ptr_ptr;

    Do_lock_mutex(&c->realm->character_file_lock);

        /* start at the first pointer */
    list_ptr_ptr = &c->realm->name_limbo;

        /* run through all addresses in limbo */
    while (*list_ptr_ptr != NULL) {

        if (strcmp((*list_ptr_ptr)->name, name) == 0) {

                /* remove this section of linked list */
            list_ptr = *list_ptr_ptr;
            *list_ptr_ptr = list_ptr->next;
            free((void *)list_ptr);

            Do_unlock_mutex(&c->realm->character_file_lock);
            return;
        }

        list_ptr_ptr = &((*list_ptr_ptr)->next);
    }

    Do_unlock_mutex(&c->realm->character_file_lock);

    sprintf(error_msg,
            "[%s] The name %s was not found in limbo by Do_release_name.\n",
            c->connection_id, name);

    Do_log_error(error_msg);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_clear_character_mod(struct player_mod_t *theMod)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/03/01
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

Do_clear_character_mod(struct player_mod_t *theMod)
{

    theMod->newName = FALSE;
    theMod->newPassword = FALSE;
    theMod->passwordReset = FALSE;
    theMod->newPermissions = FALSE;
    theMod->badPassword = FALSE;
    return;

}


/************************************************************************
/
/ FUNCTION NAME: Do_modify_character(struct client_t *c, char *the_name, struct player_mod_t *theMod)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/03/01
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

int Do_modify_character(struct client_t *c, char *the_name, struct player_mod_t *theMod)
{

    struct player_t readPlayer;
    FILE *character_file;
    char error_msg[SZ_ERROR_MESSAGE];
    long loc = 0;

    Do_lock_mutex(&c->realm->character_file_lock);

    errno = 0;
    if ((character_file=fopen(CHARACTER_FILE, "r+")) == NULL) {

        Do_unlock_mutex(&c->realm->character_file_lock);
    sprintf(error_msg,
        "[%s] fopen of %s failed in Do_modify_character: %s\n",
        c->connection_id, CHARACTER_FILE, strerror(errno));

    Do_log_error(error_msg);
    return FALSE;
    }

    /* read each line of the character file */
    while (fread((void *)&readPlayer, SZ_PLAYER, 1, character_file) == 1) {

        /* if we find our character, make the mods and save */
        if (strcmp(readPlayer.lcname, the_name) == 0) {

        /* are we changing the character name? */
        if (theMod->newName) {
        strcpy(readPlayer.name, theMod->name);
        strcpy(readPlayer.lcname, theMod->lcName);
        }

        /* are we putting in a new password? */
        if (theMod->newPassword) {
        memcpy(readPlayer.password, theMod->password, SZ_PASSWORD);
        }
        
        /* is this a password reset? */
        if (theMod->passwordReset) {
        readPlayer.last_reset = time(NULL);
        }

        /* change permissions? */
        if (theMod->newPermissions) {
        readPlayer.faithful = theMod->faithful;
        }

        /* if someone has typrd in a bad password */
        if (theMod->badPassword) {
            ++readPlayer.bad_passwords;
        }

        /* now, write over the previous entry */
        fseek(character_file, loc, 0);
            errno = 0;
        if (fwrite((void *)&readPlayer, SZ_PLAYER, 1, character_file)
            != 1) {

            sprintf(error_msg,
                 "[%s] fwrite over %s failed in Do_modify_character: %s\n",
                 c->connection_id, CHARACTER_FILE, strerror(errno));

        Do_log_error(error_msg);
        }

        fclose(character_file);
            Do_unlock_mutex(&c->realm->character_file_lock);
        return TRUE;
        }
    
    loc += SZ_PLAYER;
    }

    Do_unlock_mutex(&c->realm->character_file_lock);

    sprintf(error_msg,
        "[%s] The character %s was not found by Do_modify_character.\n",
        c->connection_id, the_name);

    Do_log_error(error_msg);
    return FALSE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_character_options(struct client_t *c)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 1/4/01
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

Do_character_options(struct client_t *c)
{
    struct player_t readPlayer;
    struct button_t buttons;
    struct game_t *game_ptr;
    FILE *character_file;
    char string_buffer[SZ_LINE];
    char error_msg[SZ_ERROR_MESSAGE];
    char theTitle[SZ_ERROR_MESSAGE];
    bool found_flag;
    int rc;
    long answer;

    found_flag = FALSE;

    /* start by looking at games in play */
    Do_send_line(c, "Searching for characters assigned to this account...\n");
    Do_send_buffer(c);
    Do_lock_mutex(&c->realm->realm_lock);
    game_ptr = c->realm->games;

    while (game_ptr != NULL) {

    if (game_ptr->description != NULL) {

        if (!strcmp(c->lcaccount, game_ptr->description->parent_account)) {

            /* do we need to put up a header? */
            if (!found_flag) {

            Do_send_clear(c);

            Do_send_line(c,
                "Character Name - Level - Date Last Loaded\n");

            found_flag = TRUE;
            }

                Do_make_character_title(c, game_ptr, theTitle);

                    /* put everything together */
                sprintf(string_buffer, "%s - %.0lf - Currently Playing\n", 
            theTitle, game_ptr->description->level);

            Do_send_line(c, string_buffer);
        }

        }
    game_ptr = game_ptr->next_game;
    }

    Do_lock_mutex(&c->realm->character_file_lock);

    errno = 0;
    if ((character_file=fopen(CHARACTER_FILE, "r")) == NULL) {

    sprintf(error_msg,
        "[%s] fopen of %s failed in Do_character_options: %s\n",
        c->connection_id, CHARACTER_FILE, strerror(errno));

    Do_log_error(error_msg);
    }
    else {

        /* loop through the the characters */
        while (fread((void *)&readPlayer, SZ_PLAYER, 1, character_file) == 1) {

            if (strcmp(c->lcaccount, readPlayer.parent_account) == 0) {

            /* do we need to put up a header? */
            if (!found_flag) {

            Do_send_clear(c);

            Do_send_line(c,
                "Character Name - Level - Date Last Loaded\n");

            found_flag = TRUE;
            }

            /* no carriage return, provided by ctime */
        ctime_r(&readPlayer.last_load, error_msg);

                    /* put everything together */
                sprintf(string_buffer, "%s the %s - %.0lf - %s",
            readPlayer.name,
                        c->realm->charstats[readPlayer.type].class_name,
            readPlayer.level, error_msg);

            Do_send_line(c, string_buffer);
        }
        }

    fclose(character_file);
    }

    Do_unlock_mutex(&c->realm->realm_lock);
    Do_unlock_mutex(&c->realm->character_file_lock);

    if (!found_flag) {
    Do_send_clear(c);
    Do_send_line(c, "No characters from this account were found.\n");
    }

    strcpy(buttons.button[0], "Change Pass\n");
    strcpy(buttons.button[1], "Reset Pass\n");
    Do_clear_buttons(&buttons, 2);

    if (c->wizard > 2) {
        strcpy(buttons.button[3], "Change Name\n");
    }

    strcpy(buttons.button[4], "Sharing\n");

    strcpy(buttons.button[7], "Go Back\n");

    rc = Do_buttons(c, &answer, &buttons);
    Do_send_clear(c);

    if (rc != S_NORM) {
    answer = 7;
    }

    /* switch on the player's answer */
    switch (answer) {

    /* The player wishes to change a character password */
    case 0:
    Do_change_character_password(c);
    break;

    /* The user wants a password reset */
    case 1:
    Do_reset_character_password(c);
    break;

    /* Rename the character */
/*
    case 3:
    Do_rename_character(c);
    break;
*/

    /* Change character sharing permissions */
    case 4:
    Do_character_sharing(c);
    break;

    /* Return to previous state */
    case 7:
    return;

    default:

        sprintf(error_msg,
                "[%s] Returned non-option in Do_character_options.\n",
                c->connection_id);

        Do_log_error(error_msg);
    Do_caught_hack(c, H_SYSTEM);
        return;
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_change_character_password(struct client_t *c)
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

Do_change_character_password(struct client_t *c)
{

    char error_msg[SZ_ERROR_MESSAGE], characterName[SZ_NAME];
    char string_buffer[SZ_LINE], lcCharacterName[SZ_NAME];
    struct button_t theButtons;
    struct player_mod_t theMod;
    int rc;
    long answer;

    Do_send_line(c, "This option allows you to change the password of one of your characters.  Do you wish to continue?\n");

    strcpy(theButtons.button[0], "Continue\n");
    Do_clear_buttons(&theButtons, 1);
    strcpy(theButtons.button[7], "Go Back\n");

    rc = Do_buttons(c, &answer, &theButtons);
    Do_send_clear(c);

    if (rc != S_NORM) {
        answer = 7;
    }

        /* switch on the player's answer */
    switch (answer) {

    /* Continiue */
    case 0:
        break;

        /* Go Back */
    case 7:
        return;

    default:

        sprintf(error_msg,
        "[%s] Returned non-option in Do_change_character_password.\n",
                c->connection_id);

        Do_log_error(error_msg);
    Do_caught_hack(c, H_SYSTEM);
    return;
    }

    for (;;) {

        /* prompt for the character */
        if (Do_string_dialog(c, characterName, SZ_NAME - 1,
            "Which character's password do you wish to change?\n")) {

        return;
        }

        /* load the character information */
        Do_lowercase(&lcCharacterName, &characterName);
        if (Do_look_character(c, lcCharacterName, &c->player)) {
        break;
        }

        /* see if the character is playing */
    if (Do_character_playing(c, lcCharacterName)) {

        Do_send_line(c, "That character is currently in the game.  You can not modify the passwords of characters in play.\n");

        Do_more(c);
        Do_send_clear(c);
        return;
    }
    
    sprintf(string_buffer, "I can not find a character named \"%s\".  Please check the spelling and try again.\n", characterName);

        Do_send_line(c, string_buffer);
        Do_more(c);
    Do_send_clear(c);
    }

    /* make sure we're on this character's parent account */
    if (strcmp(c->player.parent_account, c->lcaccount)) {

    Do_send_line(c, "This character was not created from this account.  You can only modify the passwords of characters you created.\n");

    Do_more(c);
    Do_send_clear(c);
    return;
    }

    /* found character - confirm player by asking for password */
    if (!Do_request_character_password(c, c->player.password, c->player.name,
        c->player.lcname, 0)) {

        return;
    }

    Do_clear_character_mod(&theMod);

    /* Get the new password from the player */
    if (!Do_new_password(c, &theMod.password, "character")) {
        return;
    }

    theMod.newPassword = TRUE;

    if (!Do_modify_character(c, lcCharacterName, &theMod)) {

        /* if false returns, the character was not modified */
    sprintf(string_buffer, "The character named \"%s\" is not in the character file.  This could be because the character was just loaded by someone else.  The password has NOT been changed.\n", c->player.name); 

    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
    return;
    }

    sprintf(string_buffer,
        "[%s] Changed the password to character %s.\n",
            c->connection_id, c->player.lcname);

    Do_log(CONNECTION_LOG, string_buffer);

    sprintf(string_buffer, "The password to the character \"%s\" has been successfully changed.\n", c->player.name);

    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_rename_character(struct client_t *c)
/
/ FUNCTION: roll up a new character
/
/ AUTHOR: Brian Kelly, 01/18/01
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

Do_rename_character(struct client_t *c)
{

    char error_msg[SZ_ERROR_MESSAGE], characterName[SZ_NAME];
    char string_buffer[SZ_LINE], lcCharacterName[SZ_NAME];
    struct button_t theButtons;
    struct player_mod_t theMod;
    int rc, theAnswer;
    long answer;

    Do_send_line(c, "You are asking to change the name of one of your characters.  Do you wish to continue?\n");

    strcpy(theButtons.button[0], "Continue\n");
    Do_clear_buttons(&theButtons, 1);
    strcpy(theButtons.button[7], "Go Back\n");

    rc = Do_buttons(c, &answer, &theButtons);
    Do_send_clear(c);

    if (rc != S_NORM) {
        answer = 7;
    }

        /* switch on the player's answer */
    switch (answer) {

    /* Continue */
    case 0:
        break;

        /* Go Back */
    case 7:
        return;

    default:

        sprintf(error_msg,
        "[%s] Returned non-option in Do_rename_character.\n",
                c->connection_id);

        Do_log_error(error_msg);
    Do_caught_hack(c, H_SYSTEM);
    return;
    }

    for (;;) {

        /* prompt for the character */
        if (Do_string_dialog(c, characterName, SZ_NAME - 1,
            "What is the name of the character you wish to change?\n")) {

        return;
        }

        /* load the character information */
        Do_lowercase(&lcCharacterName, &characterName);
        if (Do_look_character(c, lcCharacterName, &c->player)) {
        break;
        }

        /* see if the character is playing */
    if (Do_character_playing(c, lcCharacterName)) {

        Do_send_line(c, "That character is currently in the game.  You can not modifiy the passwords of characters in play.\n");

        Do_more(c);
        Do_send_clear(c);
        return;
    }
    
    sprintf(string_buffer, "I can not find a character named \"%s\".  Please check the spelling and try again.\n", characterName);

        Do_send_line(c, string_buffer);
        Do_more(c);
    Do_send_clear(c);
    }

    /* make sure we're on this character's parent account */
/*
    if (strcmp(c->player.parent_account, c->lcaccount)) {

    Do_send_line(c, "This character was not created from this account.  You can only modify the passwords of characters you created.\n");

    Do_more(c);
    Do_send_clear(c);
    return;
    }
*/

    /* found character - confirm player by asking for password */
/*
    if (!Do_request_character_password(c, c->player.password, c->player.name,
        c->player.lcname, 0)) {

        return;
    }
*/

    Do_clear_character_mod(&theMod);

    for (;;) {

        /* get the new character name */
        if (Do_string_dialog(c, &theMod.name, SZ_NAME - 1,
            "What name would you like your character to have?\n")) {

            return;
        }

        /* see if the name is approved */
    Do_lowercase(&theMod.lcName, &theMod.name);

        if (Do_approve_name(c, theMod.lcName, theMod.name,
                &theAnswer) != S_NORM) {

            return;
        }

        if (theAnswer) {
            break;
        }
    }

    theMod.newName = TRUE;

    if (!Do_modify_character(c, lcCharacterName, &theMod)) {

        /* if false returns, the character was not modified */
    sprintf(string_buffer, "The character named \"%s\" is not in the character file so the name was not changed.  This could be because the character was just loaded by someone else.\n", c->player.name); 

        Do_release_name(c, theMod.lcName);
    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
    return;
    }

    Do_release_name(c, theMod.lcName);

    sprintf(string_buffer,
        "[%s] Changed character name %s to %s.\n",
            c->connection_id, c->player.lcname, theMod.lcName);

    Do_log(CONNECTION_LOG, string_buffer);

    sprintf(string_buffer,
        "[%s] %s renamed %s.\n",
            c->connection_id, c->player.lcname, theMod.lcName);

    Do_log(GAME_LOG, string_buffer);

    sprintf(string_buffer, "The character \"%s\" is now called \"%s\".\n",
        c->player.name, theMod.name);

    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_reset_character_password(struct client_t *c)
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

Do_reset_character_password(struct client_t *c)
{

    char error_msg[SZ_ERROR_MESSAGE], characterName[SZ_NAME];
    char string_buffer[SZ_LINE], lcCharacterName[SZ_NAME];
    char newPassword[SZ_PASSWORD];
    struct player_mod_t theMod;
    struct button_t theButtons;
    int rc;
    long answer;
    MD5_CTX context;
    unsigned int len;

    Do_send_line(c, "With this option a random password will be created for one of your characters and e-mailed to your account address.  This is the only way to gain access to a character whose password you've forgotten.\n");

    strcpy(theButtons.button[0], "Continue\n");
    Do_clear_buttons(&theButtons, 1);
    strcpy(theButtons.button[7], "Go Back\n");

    rc = Do_buttons(c, &answer, &theButtons);
    Do_send_clear(c);

    if (rc != S_NORM) {
        answer = 7;
    }

        /* switch on the player's answer */
    switch (answer) {

    /* Continiue */
    case 0:
        break;

        /* Go Back */
    case 7:
        return;

    default:

        sprintf(error_msg,
        "[%s] Returned non-option in Do_reset_character_password.\n",
                c->connection_id);

        Do_log_error(error_msg);
    Do_caught_hack(c, H_SYSTEM);
    return;
    }

    for (;;) {

        /* prompt for the character */
        if (Do_string_dialog(c, characterName, SZ_NAME - 1,
            "Which character's password do you wish to reset?\n")) {

        return;
        }

        /* load the character information */
        Do_lowercase(&lcCharacterName, &characterName);
        if (Do_look_character(c, lcCharacterName, &c->player)) {
        break;
        }

        /* see if the character is playing */
    if (Do_character_playing(c, lcCharacterName)) {

        Do_send_line(c, "That character is currently in the game.  You can not modifiy the passwords of characters in play.\n");

        Do_more(c);
        Do_send_clear(c);
        return;
    }
    
    sprintf(string_buffer, "I can not find a character named \"%s\".  Please check the spelling and try again.\n", characterName);

        Do_send_line(c, string_buffer);
        Do_more(c);
    Do_send_clear(c);
    }

    /* make sure we're on this character's parent account */
    if (strcmp(c->player.parent_account, c->lcaccount)) {

    Do_send_line(c, "This character was not created from this account.  You can only modify the passwords of characters you created.\n");

    Do_more(c);
    Do_send_clear(c);
    return;
    }

    /* see if it's been longer than 24 hours since last reset */
    if (time(NULL) - c->player.last_reset < 86400) {

    sprintf(string_buffer, "The password to the character named \"%s\" has been reset within the last 24 hours.  You must wait before resetting it again.\n", c->player.name);

    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
    return;
    }

    sprintf(string_buffer, "Are you certain you wish to reset the password for the character named \"%s\"?\n", c->player.name);

    Do_send_line(c, string_buffer);

    if (Do_yes_no(c, &answer) != S_NORM || answer == 1) {

    Do_send_clear(c);
    sprintf(string_buffer, "Password reset aborted.  The password to the character named \"%s\" has NOT been changed.\n", c->player.name);

    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
    return;
    }

    Do_send_clear(c);

    /* create a new password */
    Do_create_password(&newPassword);

        /* call the script to e-mail this new password */
    sprintf(string_buffer, "%s %s %s %s\n", CHARACTER_PASSWORD_RESET_SCRIPT,
            c->player.name, newPassword, c->email);

        /* if the mail send fails */
    if (rc = system(string_buffer)) {

        sprintf(string_buffer,
              "[%s] Character password reset e-mail failed with a code of %d.",
              c->connection_id, rc);

        Do_log_error(string_buffer);

        Do_send_line(c, "An error occured while trying to send e-mail containing the new password.  The character password has NOT been changed.  Please contact the game administrator about this problem.\n");

        Do_more(c);
    Do_send_clear(c);
        return;
    }

    Do_clear_character_mod(&theMod);
    theMod.newPassword = TRUE;
    theMod.passwordReset = TRUE;

        /* run the password through a MD5 hash */
    len = strlen(newPassword);
    MD5Init(&context);
    MD5Update(&context, newPassword, len);

    /* put the password hash in place */
    MD5Final(theMod.password, &context);

    /* make the modification */
    if (!Do_modify_character(c, lcCharacterName, &theMod)) {

        /* if false returns, the character was not modified */
    sprintf(string_buffer, "The character named \"%s\" is not in the character file.  This could be because the character was just loaded by someone else.  The password has NOT been changed.\n", c->player.name); 

    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
        return;
    }

    sprintf(string_buffer,
        "[%s] Reset the password to character %s.\n",
            c->connection_id, c->player.lcname);

    Do_log(CONNECTION_LOG, string_buffer);

    sprintf(string_buffer, "The password to the character named \"%s\" has been successfully changed.  Your new password has been e-mailed to your account address.\n", c->player.name);

    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
}


/************************************************************************
/
/ FUNCTION NAME: Do_character_sharing(struct client_t *c)
/
/ FUNCTION: roll up a new character
/
/ AUTHOR: Brian Kelly, 1/5/01
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

Do_character_sharing(struct client_t *c)
{

    char error_msg[SZ_ERROR_MESSAGE], characterName[SZ_NAME];
    char string_buffer[SZ_LINE], lcCharacterName[SZ_NAME];
    struct player_mod_t theMod;
    struct button_t theButtons;
    int rc;
    long answer;

    Do_send_line(c, "By default, characters can only be loaded by the account that created them.  Here you can remove or replace that restriction.\n");

    strcpy(theButtons.button[0], "Continue\n");
    Do_clear_buttons(&theButtons, 1);
    strcpy(theButtons.button[7], "Go Back\n");

    rc = Do_buttons(c, &answer, &theButtons);
    Do_send_clear(c);

    if (rc != S_NORM) {
        answer = 7;
    }

        /* switch on the player's answer */
    switch (answer) {

    /* Continiue */
    case 0:
        break;

        /* Go Back */
    case 7:
        return;

    default:

        sprintf(error_msg,
        "[%s] Returned non-option in Do_character_sharing.\n",
                c->connection_id);

        Do_log_error(error_msg);
    Do_caught_hack(c, H_SYSTEM);
    return;
    }

    for (;;) {

        /* prompt for the character */
        if (Do_string_dialog(c, characterName, SZ_NAME - 1,
          "Which character's sharing options do you wish to modify ?\n")) {

        return;
        }

        /* load the character information */
        Do_lowercase(&lcCharacterName, &characterName);
        if (Do_look_character(c, lcCharacterName, &c->player)) {
        break;
        }

        /* see if the character is playing */
    if (Do_character_playing(c, lcCharacterName)) {

        Do_send_line(c, "That character is currently in the game.  You can not modifiy characters in play.\n");

        Do_more(c);
        Do_send_clear(c);
        return;
    }
    
    sprintf(string_buffer, "I can not find a character named \"%s\".  Please check the spelling and try again.\n", characterName);

        Do_send_line(c, string_buffer);
        Do_more(c);
    Do_send_clear(c);
    }

    /* make sure we're on this character's parent account */
    if (strcmp(c->player.parent_account, c->lcaccount)) {

    Do_send_line(c, "This character was not created from this account.  You can only modify characters you created.\n");

    Do_more(c);
    Do_send_clear(c);
    return;
    }

        /* found character - now get the password */
    if (!Do_request_character_password(c, c->player.password, c->player.name,
            c->player.lcname, 0)) {

        return;
    }

    if (c->player.faithful) {
        sprintf(string_buffer, "Currently, other accounts can not load the character named \"%s\".\n", c->player.name);
    }
    else {
        sprintf(string_buffer, "Currently, other accounts have permission to load the character named \"%s\".\n", c->player.name);
    }

    Do_send_line(c, string_buffer);
    Do_send_line(c, "\n");
    Do_send_line(c, "If another account attempts to load this character, do you wish to allow or deny their request?\n");

    Do_clear_character_mod(&theMod);

    strcpy(theButtons.button[0], "Allow\n");
    strcpy(theButtons.button[1], "Deny\n");
    Do_clear_buttons(&theButtons, 2);
    strcpy(theButtons.button[7], "Cancel\n");

    rc = Do_buttons(c, &answer, &theButtons);
    Do_send_clear(c);

    if (rc != S_NORM) {
        answer = 7;
    }

        /* switch on the player's answer */
    switch (answer) {

    /* Allow */
    case 0:
        theMod.faithful = FALSE;
    break;

    /* Deny */
    case 1:
        theMod.faithful = TRUE;
    break;

        /* Cancel */
    case 7:
        return;

    default:

        sprintf(error_msg,
        "[%s] Returned non-option in Do_character_sharing(2).\n",
                c->connection_id);

        Do_log_error(error_msg);
    Do_caught_hack(c, H_SYSTEM);
    return;
    }

    theMod.newPermissions = TRUE;
    if (!Do_modify_character(c, lcCharacterName, &theMod)) {

        /* if false returns, the character was not modified */
    sprintf(string_buffer, "The character named \"%s\" is not in the character file.  This could be because the character was just loaded by someone else.  The permissions have NOT been changed.\n", c->player.name); 

    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
        return;
    }
    
    if (answer) {

    sprintf(error_msg,
            "[%s] Faithful permissions set on character %s.\n",
                c->connection_id, c->player.lcname);

    sprintf(string_buffer, "Permissions have been set so other accounts will be denied access to the character named \"%s\".\n", c->player.name);

    }
    else {

    sprintf(error_msg,
            "[%s] Faithful permissions removed on character %s.\n",
                c->connection_id, c->player.lcname);

    sprintf(string_buffer, "Permissions have been set so other accounts may load the character named \"%s\".\n", c->player.name);

    }

    Do_log(CONNECTION_LOG, error_msg);
    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
}


/************************************************************************
/
/ FUNCTION NAME: Do_approve_entrance(struct client_t *c)
/
/ FUNCTION: roll up a new character
/
/ AUTHOR: Brian Kelly, 1/14/01
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
/
/
*************************************************************************/

Do_approve_entrance(struct client_t *c)
{
    struct event_t *event_ptr;

    /* Mark this thread as with character */
    c->characterLoaded = TRUE;

    /* assume the name is not modified */
    strcpy(c->modifiedName, c->player.name);

        /* if the character was knight after crash */
    if (c->player.special_type == SC_KNIGHT) {
        Do_dethrone(c);
    }

    if (c->player.special_type == SC_STEWARD) {
        Do_dethrone(c);
    }

        /* if the character was king after crash */
    if (c->player.special_type == SC_KING) {

        Do_lock_mutex(&c->realm->realm_lock);

            /* check to see if we're the old king */
        if (! strcmp(c->realm->king_name, c->modifiedName)) {
                 
            Do_unlock_mutex(&c->realm->realm_lock);

            if (c->player.level >= MIN_KING && c->player.level < MAX_KING) {
                Do_king(c);
            }
            else {
                Do_dethrone(c);
                c->player.special_type = SC_NONE;
            }
        }
        else {

                /* there has been a new king */
            Do_unlock_mutex(&c->realm->realm_lock);
            Do_send_line(c, "You are no longer the ruler!\n");
            c->player.special_type = SC_NONE;
        }
    }

        /* handle if the character was valar after crash */
    else if (c->player.special_type == SC_VALAR) {

        Do_lock_mutex(&c->realm->realm_lock);

            /* check for a current valar */
        if (! strcmp (c->realm->valar_name, c->modifiedName)) {
                 
                /* no valar, put ourselves */ 
            Do_unlock_mutex(&c->realm->realm_lock);
            c->realm->valar = c->game;
            strcpy(c->realm->valar_name, c->modifiedName);
        }
        else {

                /* there is a valar, bow out */
            Do_unlock_mutex(&c->realm->realm_lock);
            Do_valar(c);
        }
    }

    /* check for tags to this address, connection or character */
    Do_check_tags(c);

    /* done here */
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_entering_character(struct client_t *c)
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
/
/
*************************************************************************/

Do_entering_character(struct client_t *c)
{
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE];
    FILE *wizard_file, *motd_file;
    char theNetwork[SZ_FROM], theAccount[SZ_NAME], theCharacter[SZ_NAME];
    short wizType;
    char *char_ptr;
    int minutes;
    struct event_t *event_ptr;
    int exceptionFlag;

        /* pull death name out of limbo if necessary */
    if (c->previousName[0] != '\0') {
        Do_release_name(c, c->previousName);
        c->previousName[0] = '\0';
    }

    /* open the wizard file to see if this person is one */
    if ((wizard_file=fopen(WIZARD_FILE, "r")) == NULL) {

    sprintf(error_msg,
        "[%s] fopen of %s failed in Do_entering_character: %s\n",
        c->connection_id, WIZARD_FILE, strerror(errno));

    Do_log_error(error_msg);
    }
    else {

            /* loop through the the names */
        while (fscanf(wizard_file, "%d %s %s %s %d\n", &wizType, &theNetwork,
        &theAccount, &theCharacter, &exceptionFlag) == 5) {

        if (!strcmp(theCharacter, c->player.lcname) && !strcmp(
            theAccount, c->lcaccount) && (!strcmp(theNetwork,
            c->network) || exceptionFlag)) {

        c->wizard = wizType;
        break;
        }
    }

        fclose(wizard_file);
    }

        /* if appearing in ch 8 with a palantir, make them hear ch 1 */
    if (c->channel == 8) {

        if (!c->player.palantir) {
            c->channel = 1;
        } else {
            c->game->hearAllChannels = HEAR_ONE;
        }
    }

    /* send everyone your spec */
    Do_send_specification(c, ADD_PLAYER_EVENT);

        /* put characters in their proper area */
    if (c->wizard > 2) {
        c->player.location = PL_VALHALLA;
    }
    else if (c->player.purgatoryFlag) {
        c->player.location = PL_PURGATORY;
    }
    else {
        c->player.location = PL_REALM;
    }

        /* force the loaded values to the players status window */
    Do_update_stats(c);

    /* log the entry */
    sprintf(string_buffer, "[%s] Entering realm with character %s.\n",
        c->connection_id, c->player.lcname);

    Do_log(CONNECTION_LOG, string_buffer);

        /* set the player's timeout */
    if (c->wizard > 2) {
        c->timeout = 900;
    }
    else if (c->player.level < 80) {
        c->timeout = 60 - c->player.level / 2;
    }
    else {
        c->timeout = 20;
    }

    /* show the player last load stats and update */
    Do_last_load_info(c);

    Do_send_line(c, "\n");

    /* show the player the message of the day */
    /* try to open the MOTD file */
    errno = 0;
    if ((motd_file = fopen(MOTD_FILE, "r")) != NULL &&
        fgets(string_buffer, SZ_LINE, motd_file) != NULL) {

        if (((char_ptr = (char *) strstr(string_buffer, "TESTING ")) != NULL)
            && c->wizard < 1) {

            char_ptr += sizeof("TESTING ");

	        /* send a message to the user */
            if (sscanf(char_ptr, "%d", &minutes) != 0) {
                sprintf(string_buffer, 
		        "The game is currently down for testing.  Try back in 15 minutes.\n");
            } else {
                sprintf(string_buffer, 
                        "The game is currently down for testing.  Try back in %d minutes.\n", minutes);
            }

            Do_send_error(c, string_buffer);

            if (c->characterLoaded == TRUE) {
                c->run_level = SAVE_AND_EXIT;
            }
            else {
                c->run_level = EXIT_THREAD;
            }

            fclose(motd_file);

            return;

        } else {
            /* print message of the day */
            Do_send_line(c, string_buffer);
        }

        fclose(motd_file);
    }
    else {

        /* log an error message */
    sprintf(error_msg, 
                "[%s] fopen of %s failed in Do_entering_character: %s.\n",
        c->connection_id, MOTD_FILE, strerror(errno));

    Do_log_error(error_msg);
    }


    Do_send_buffer(c);
    if (c->wizard < 1) {
        sleep(3);
    }
    Do_more(c);
    Do_send_clear(c);

        /* announce the player entrance */
    if (c->wizard > 2) {

    sprintf(string_buffer,
            "In a brilliant flash, Wizard %s appears in the realm!\n",
            c->modifiedName);

    } else if (c->wizard == 2) {

    sprintf(string_buffer,
            "In a puff of smoke, Apprentice %s the %s appears in the realm!\n",
            c->modifiedName, c->realm->charstats[c->player.type].class_name);
        
    } else if (c->player.special_type == SC_VALAR) {

    sprintf(string_buffer,
            "Tremble, for the Valar %s has arrived!\n",
            c->modifiedName);

    } else {

        sprintf(string_buffer,
            "A new player appears in the realm, %s the %s.\n",
            c->modifiedName, c->realm->charstats[c->player.type].class_name);

    }

    c->characterAnnounced = TRUE;

    Do_broadcast(c, string_buffer);

        /* It's okay to chat now */
    Do_send_int(c, ACTIVATE_CHAT_PACKET);


    if (c->player.energy <= 0.0) {

        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = DEATH_EVENT;
        event_ptr->arg1 = (double) c->battle.ring_in_use;
        event_ptr->arg3 = K_NO_ENERGY;
        Do_handle_event(c, event_ptr);
    }



    /* handle purgatory */
    if (c->player.purgatoryFlag) {

    Do_send_line(c, "This character was in combat previously when the connection was interrupted.  You will now re-encounter that monster.\n");

    Do_more(c);
    Do_send_clear(c);
    c->player.purgatoryFlag = FALSE;

        /* Throw the monster at the player */
        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = MONSTER_EVENT;
        event_ptr->arg1 = MONSTER_PURGATORY;
        event_ptr->arg3 = (long) c->player.monsterNumber;
        Do_handle_event(c, event_ptr);

        /* return the player to the realm */
    c->player.location = PL_REALM;
    Do_location(c, c->player.x, c->player.y, TRUE);
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_handle_save(struct client_t *c)
/
/ FUNCTION: roll up a new character
/
/ AUTHOR: Brian Kelly, 01/18/01
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

Do_handle_save(struct client_t *c)
{
    char string_buffer[SZ_LINE];

    Do_save_character(c, &c->player);

        /* remove the character's backup */
    Do_backup_save(c, FALSE);
    c->characterLoaded = FALSE;

    /* log the saving */
    sprintf(string_buffer, "[%s] %s saved\n", c->connection_id,
        c->player.lcname);

    Do_log(GAME_LOG, &string_buffer);

    /* if server is shutting down or socket error */
    if (c->run_level == SAVE_AND_CONTINUE) {
    c->run_level = GO_AGAIN;
    }
    else {
    c->run_level = EXIT_THREAD;
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_leaving_character(struct client_t *c)
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

Do_leaving_character(struct client_t *c)
{
    char string_buffer[SZ_LINE];
    struct event_t *event_ptr;
    float ftemp;

    /* handle remaining events */
    Do_orphan_events(c);
    while (c->events != NULL) {

            /* remove the next event */
        event_ptr = c->events;
        c->events = event_ptr->next_event;

            /* take care of it */
        if (event_ptr->type > GAME_MARKER) {
            free((void *)event_ptr);
        }
        else {
            Do_handle_event(c, event_ptr);
        }
    }

    /* if the character is steward, knight, or king, de-throne him first */
    if (c->player.special_type == SC_STEWARD || 
        c->player.special_type == SC_KNIGHT ||
        c->player.special_type == SC_KING) {

        Do_dethrone(c);
    }

    ftemp = pow(fabs(c->player.x) / 100, .5);

    /* if the character is on a post, boot him off */
    if ((fabs(c->player.x) == fabs(c->player.y)) &&
        (floor(ftemp) == ftemp)) {

	/* if the player can not leave the chamber now, return */
        if (c->stuck) {
            Do_send_line(c, "Another player is arriving...'\n");

	    Do_more(c);
	    Do_send_clear(c);
	    return;
        }


	/* Kick live players off of posts, dead characters off of door steps */
		
		if ((c->player.energy <= 0.0) || (c->player.strength <= 0.0)) {
		
		Do_send_line(c, "The merchant scowls, and kicks your corpse off the steps of his shop!'\n");
		
		} else {
		
        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = MOVE_EVENT;
        event_ptr->arg3 = A_NEAR;
        Do_handle_event(c, event_ptr);
		
        Do_send_line(c, "The merchant scowls, says 'No loitering!, and throws you out!'\n");
        }
    }

    /* if the character is valar, remove from the realm */
    if (c->player.special_type == SC_VALAR) {
        Do_lock_mutex(&c->realm->realm_lock);
        c->realm->valar = NULL;
        Do_unlock_mutex(&c->realm->realm_lock);
    }

    /* turn off palantir */
    if (c->channel == 8) {
        c->game->hearAllChannels = HEAR_SELF;
    }

        /* announce the player's departure */
    if (c->run_level == SAVE_AND_CONTINUE || c->run_level == SAVE_AND_EXIT) {

        if (c->wizard > 2) {

        sprintf(string_buffer, "%s slowly fades from the realm!\n",
                c->modifiedName);

        } else {

            sprintf(string_buffer, "%s retires from the realm.\n",
                c->modifiedName);
        }

        Do_broadcast(c, string_buffer);
    }

    /* kill the character if leaving quickly */
    else if (c->run_level == EXIT_THREAD) {

        /* kill the player */
        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = DEATH_EVENT;
        event_ptr->arg1 = FALSE;
        event_ptr->arg3 = K_SUICIDE;
        Do_handle_event(c, event_ptr);
    }

        /* erase the player description */
    Do_lock_mutex(&c->realm->realm_lock);
    free(c->game->description);
    c->game->description = NULL;
    Do_unlock_mutex(&c->realm->realm_lock);

        /* remove the player specification */
    Do_send_specification(c, REMOVE_PLAYER_EVENT);

    c->characterAnnounced = FALSE;
    
    c->timeout = 120;

    /* chat no more */
    Do_send_int(c, DEACTIVATE_CHAT_PACKET);

    /* log the entry */
    sprintf(string_buffer, "[%s] Leaving realm.\n", c->connection_id);
    Do_log(CONNECTION_LOG, string_buffer);

    /* record time character has been playing */
    c->player.time_played += time(NULL) - c->player.last_load;

    if (c->wizaccount[0] != '\0') {
        strcpy(c->account, c->wizaccount);
        c->wizaccount[0] = '\0';
    }

    if (c->wizIP[0] != '\0') {
        strcpy(c->IP, c->wizIP);
        c->wizIP[0] = '\0';
    }
}


