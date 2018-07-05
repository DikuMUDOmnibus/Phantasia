/*
 * file.c       File manipulation routines
 */

#include "include.h"

/***************************************************************************
/ FUNCTION NAME: Do_purge_characters(void)
/
/ FUNCTION: Handles error messages
/
/ AUTHOR:  Brian Kelly, 4/22/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

void Do_purge_characters(void)
{
    FILE *character_file;
    char error_msg[SZ_ERROR_MESSAGE];

	/* open the character file */
    errno = 0;
    if ((character_file=fopen(CHARACTER_FILE, "w")) == NULL) {

        sprintf(error_msg,
		"[0.0.0.0:?] fopen of %s failed in Do_purge_characters: %s\n",
		CHARACTER_FILE, strerror(errno));

	Do_log_error(error_msg);
	exit(CHARACTER_FILE_ERROR);
    }

    fclose(character_file);

    return;
}


/***************************************************************************
/ FUNCTION NAME: Do_purge_scoreboard(void)
/
/ FUNCTION: Handles error messages
/
/ AUTHOR:  Brian Kelly, 4/22/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

void Do_purge_scoreboard(void)
{
    FILE *scoreboard_file;
    char error_msg[SZ_ERROR_MESSAGE];

	/* open the scoreboard file */
    errno = 0;
    if ((scoreboard_file=fopen(SCOREBOARD_FILE, "w")) == NULL) {

        sprintf(error_msg,
		"[0.0.0.0:?] fopen of %s failed in Do_purge_scoreboard: %s\n",
		SCOREBOARD_FILE, strerror(errno));

	Do_log_error(error_msg);
	exit(SCOREBOARD_FILE_ERROR);
    }

    fclose(scoreboard_file);

    return;
}


/***************************************************************************
/ FUNCTION NAME: Do_scoreboard_add(struct client_t *c, struct scoreboard_t *entry)
/
/ FUNCTION: Handles error messages
/
/ AUTHOR:  Brian Kelly, 4/22/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

void Do_scoreboard_add(struct client_t *c, struct scoreboard_t *entry, bool showScore)
{
    FILE    *scoreboard_file, *temp_file;       /* for updating various files */
    struct scoreboard_t sb;
    struct event_t *event_ptr;
    char error_msg[SZ_ERROR_MESSAGE];
    int location;
    bool save_flag;

    Do_lock_mutex(&c->realm->scoreboard_lock);

	/* open the scoreboard file */
    errno = 0;
    if ((scoreboard_file=fopen(SCOREBOARD_FILE, "r")) == NULL) {

	Do_unlock_mutex(&c->realm->scoreboard_lock);

        sprintf(error_msg, "[%s] fopen of %s failed in Do_scoreboard_add: %s\n",
		c->connection_id, SCOREBOARD_FILE, strerror(errno));

        Do_log_error(error_msg);
	return;
    }

	/* open a tempoary file */
    if ((temp_file=fopen(TEMP_SCOREBOARD_FILE, "w")) == NULL) {

	fclose(scoreboard_file);
	Do_unlock_mutex(&c->realm->scoreboard_lock);

        sprintf(error_msg,
		"[%s] fopen of %s failed in Do_scoreboard_add(2): %s\n",
		c->connection_id, TEMP_SCOREBOARD_FILE, strerror(errno));

        Do_log_error(error_msg);
	return;
    }

    save_flag = TRUE;
    location = 0;

	/* copy records over to the temp file */
    while (fread((void *)&sb, SZ_SCOREBOARD, 1, scoreboard_file) == 1) {

	    /* if our player is higher level than our copy, put him in */
	if (save_flag && sb.level <= entry->level) {

            if (fwrite((void *)entry, SZ_SCOREBOARD, 1, temp_file) != 1) {

	        fclose(scoreboard_file);
	        fclose(temp_file);
		remove(TEMP_CHARACTER_FILE);
	        Do_unlock_mutex(&c->realm->scoreboard_lock);

                sprintf(error_msg,
			"[%s] fwrite of %s failed in Do_scoreboard_add: %s\n",
                        c->connection_id, TEMP_SCOREBOARD_FILE,
			strerror(errno));

                Do_log_error(error_msg);
		return;
            }
	    save_flag = FALSE;
	}

            /* check that the record to save isn't too old */
        if (sb.level > SB_KEEP_ABOVE || sb.time > time(NULL) - 
                SB_KEEP_FOR) {

	        /* transfer over the record */
            if (fwrite((void *)&sb, SZ_SCOREBOARD, 1, temp_file) != 1) {

	        fclose(scoreboard_file);
	        fclose(temp_file);
	        remove(TEMP_CHARACTER_FILE);
	        Do_unlock_mutex(&c->realm->scoreboard_lock);

                sprintf(error_msg,
		      "[%s] fwrite of %s failed in Do_scoreboard_add(2): %s\n",
                      c->connection_id, TEMP_SCOREBOARD_FILE, strerror(errno));

                Do_log_error(error_msg);
		return;
            }
        }

	    /* count this record if we haven't saved our character yet */
	if (save_flag) {
	    ++location;
	}
    }

	/* if no more scoreboard records, and we haven't written ours yet */
    if (save_flag) {

	    /* write it now */
        if (fwrite((void *)entry, SZ_SCOREBOARD, 1, temp_file) != 1) {

	    fclose(scoreboard_file);
	    fclose(temp_file);
	    remove(TEMP_CHARACTER_FILE);
	    Do_unlock_mutex(&c->realm->scoreboard_lock);

            sprintf(error_msg,
		    "[%s] fwrite of %s failed in Do_scoreboard_add(3): %s\n",
                    c->connection_id, TEMP_SCOREBOARD_FILE, strerror(errno));

            Do_log_error(error_msg);
	    return;
        }
    }

	/* remove the scoreboard */
    remove(SCOREBOARD_FILE);

	/* replace the old scoreboard file with the new one */
    rename(TEMP_SCOREBOARD_FILE, SCOREBOARD_FILE);

	/* close the file handles */
    fclose(scoreboard_file);
    fclose(temp_file);

    Do_unlock_mutex(&c->realm->scoreboard_lock);

        /* display the scoreboard */
    if (showScore) {
        Do_scoreboard(c, location, TRUE);
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_log(char *filename, char *message)
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

Do_log(char *filename, char *message)
{
    FILE *log_file;
    time_t time_now;
    char error_msg[SZ_ERROR_MESSAGE];
    char string_buffer[SZ_LINE];

    time_now = time(NULL);
    ctime_r(&time_now, string_buffer);
    Do_truncstring(string_buffer);

        /* open the error log file */
    errno = 0;
    if ((log_file=fopen(filename, "a")) == NULL) {

            /* if the log file won't open, send and error */
        if (strcmp(filename, ERROR_LOG)) {

            sprintf(error_msg,
		    "[?:?] Can not open log file %s in Do_log: %s.\n",
		    filename, strerror(errno));

            Do_log_error(error_msg);
        }

            /* unless it is the error log, then go to stdout */
        else {

            printf("%s [?:?] Can not open error file %s in Do_log: %s.\n",
		    string_buffer, ERROR_LOG, strerror(errno));

	    printf("%s %s", string_buffer, message);
        }

        return;
    }

        /* print the error to the logfile */
    fprintf(log_file, "%s %s", string_buffer, message);

        /* close the logfile */
    fclose(log_file);

        /* return, regardless of error */
    return;
}


/***************************************************************************
/ FUNCTION NAME: Do_log_error(char *message)
/
/ FUNCTION: Handles error messages
/
/ AUTHOR:  Brian Kelly, 4/12/99
/
/ ARGUMENTS:
/       int error - the error code to be returned
/       char *message - the error message to be printed
/
/ RETURN VALUE: none
/
/ DESCRIPTION:
/       Process arguments, initialize program, and loop forever processing
/       player input.
/
****************************************************************************/

Do_log_error(char *message)
{
    Do_log(ERROR_LOG, message);
}


/************************************************************************
/
/ FUNCTION NAME: int Do_check_protected(struct client_t *c, char *theNetwork)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 1/3/01
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

int Do_check_protected(struct client_t *c, char *theNetwork)
{
    char error_msg[SZ_ERROR_MESSAGE];
    char string_buffer[SZ_LINE];
    FILE *theFile;

        /* open the protected file */
    if ((theFile=fopen(PROTECTED_FILE, "r")) == NULL) {

        sprintf(error_msg,
		"[0.0.0.0:?] fopen of %s failed in Do_check_protected: %s\n",
                PROTECTED_FILE, strerror(errno));

        Do_log_error(error_msg);
	return FALSE;
    }

        /* loop through the the addresses */
    while (fgets(string_buffer, SZ_FROM, theFile) != NULL) {

        Do_truncstring(string_buffer);

        if (!strcmp(string_buffer, theNetwork)) {
	    fclose(theFile);
            return TRUE;
	}
    }

    fclose(theFile);
    return FALSE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_restore_character(struct client_t *c)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/17/01
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

Do_restore_character(struct client_t *c)
{

    struct player_t readPlayer;
    struct history_t theHistory;
    FILE *character_file;
    char characterName[SZ_NAME], lcCharacterName[SZ_NAME];
    char error_msg[SZ_ERROR_MESSAGE];
    bool char_flag;


    if (Do_string_dialog(c, characterName, SZ_NAME - 1,
            "What is the name of the character to restore\n")) {

	Do_send_clear(c);
        return;
    }

    Do_send_clear(c);
    Do_lowercase(&lcCharacterName, &characterName);
    if (Do_look_character(c, &lcCharacterName, &readPlayer)) {

	sprintf(error_msg,
		"A character named %s is already in the character file.\n",
		characterName);

	Do_send_line(c, error_msg);
	Do_more(c);
	Do_send_clear(c);
	return;
    }

    errno = 0;
    if ((character_file=fopen(SECONDARY_CHAR_FILE, "r")) == NULL) {

	sprintf(error_msg, "fopen of %s failed: %s\n",
		SECONDARY_CHAR_FILE, strerror(errno));

	Do_send_line(c, error_msg);
	Do_more(c);
	Do_send_clear(c);
	return;
    }

    char_flag = FALSE;

	/* read each line of the character file */
    while (fread((void *)&readPlayer, SZ_PLAYER, 1, character_file) == 1) {

	    /* if we find our character, copy it over */
        if (strcmp(readPlayer.lcname, lcCharacterName) == 0) {
	    char_flag = TRUE;
	    break;
        }
    }

    fclose(character_file);

    if (!char_flag) {

	sprintf(error_msg, "A character named %s was not found in %s.\n",
		characterName, SECONDARY_CHAR_FILE);

	Do_send_line(c, error_msg);
	Do_more(c);
	Do_send_clear(c);
	return;
    }

	/* save the character */
    Do_save_character(c, &readPlayer);

	/* log this restore */
    sprintf(theHistory.description, "%s restored the character named %s\n",
	    c->modifiedName, readPlayer.name);

    theHistory.date = time(NULL);
    theHistory.type = T_ACCOUNT;
    strcpy(theHistory.name, readPlayer.parent_account);
    Do_save_history(c, &theHistory);

    theHistory.type = T_ADDRESS;
    strcpy(theHistory.name, readPlayer.parent_network);
    Do_save_history(c, &theHistory);

    sprintf(error_msg, "[%s] %s restored\n", c->connection_id,
            readPlayer.lcname);

    Do_log(GAME_LOG, error_msg);

    Do_send_line(c, "The character has been successfully restored.\n");
    Do_more(c);
    Do_send_clear(c);
    return;
}
