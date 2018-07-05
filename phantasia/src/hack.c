/*
 * hack.c       Routines to detect/prevent hacks
 */

#include "include.h"

/************************************************************************
/
/ FUNCTION NAME: Do_caught_hack(c)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 01/06/01
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

Do_caught_hack(struct client_t *c, int theReason)
{
    int hackCount, expiredAttempts;
    struct account_t theAccount;
    struct account_mod_t theAccountMod;
    struct network_t theNetwork;
    struct tag_t theTag;
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE];
    time_t timeNow;
    bool rejectFlag;

	/* ban times in seconds- one hour, one day, one week, one month,
	four months, one year */
    int tagTimes[7] = {0, 3600, 86400, 604800, 2592000, 9676800, 31536000};
    timeNow = time(NULL);
    rejectFlag = FALSE;

	/* lock the hack lock to prevent these stats from updating by others */
    Do_lock_mutex(&c->realm->hack_lock);

    theAccount.hackCount = 0;

	/* find past triggers, start with the character, if it exists */
	/* skip the character, if it exists, it's going to be killed */
    if (c->accountLoaded) {

	    /* load the account - there may be a hack since last load */
        if (Do_look_account(c, c->lcaccount, &theAccount)) {

		/* if previous offenses, lower count by 1 per month since */
	    if (theAccount.hackCount) {

	        theAccount.hackCount -= (timeNow - theAccount.lastHack) /
			2592000;

		if (theAccount.hackCount < 0) {
		    theAccount.hackCount = 0;
		}
	    }
        }
    }

	/* now get the information on the connection - it may not exist! */
    if (Do_look_network(c, c->network, &theNetwork)) {

	    /* if previous offenses, lower count by 1 per month since */
	expiredAttempts = (timeNow - theNetwork.lastHack) / 2592000;

	if (theNetwork.hackCount) {
	    theNetwork.hackCount -= expiredAttempts;

	    if (theNetwork.hackCount < 0) {
	        theNetwork.hackCount = 0;
	    }
	}
    }
    else {
        strcpy(theNetwork.address, c->network);
        theNetwork.hackCount = 0;
    }

	/* find the highest number of hack and rejections attempts */
    if (theAccount.hackCount > theNetwork.hackCount) {
	hackCount = theAccount.hackCount;
    }
    else {
	hackCount = theNetwork.hackCount;
    }

	/* increment the hack count */
    ++hackCount;

	/* max ban is a year */
    if (hackCount > 6) {
	hackCount = 6;
    }

	/* save the account info */
    if (c->accountLoaded) {
        Do_clear_account_mod(&theAccountMod);
        theAccountMod.hack = TRUE;
        theAccountMod.hackCount = hackCount;
        Do_modify_account(c, c->lcaccount, NULL, &theAccountMod);
    }

	/* update the address info */
    theNetwork.hackCount = hackCount;
    theNetwork.lastHack = timeNow;
    
    if (theNetwork.expires < timeNow + hackCount * 2592000) {
	theNetwork.expires = timeNow + hackCount * 2592000;
    }

    Do_update_network(c, &theNetwork);

	/* prepare a tag for the account and address */
    theTag.type = T_BAN;
    theTag.validUntil = timeNow + tagTimes[hackCount];
    theTag.affectNetwork = FALSE;
    Do_get_hack_string(theReason, theTag.description);

	/* send it */
    Do_tag_self(c, &theTag);
    Do_unlock_mutex(&c->realm->hack_lock);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: int Do_look_network(struct client_t *c, char *networkName, struct network_t *theNetwork)
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

int Do_look_network(struct client_t *c, char *networkName, struct network_t *theNetwork)
{
    char string_buffer[SZ_LINE];
    FILE *network_file;

        /* open the network file */
    errno = 0;
    Do_lock_mutex(&c->realm->network_file_lock);
    if ((network_file=fopen(NETWORK_FILE, "r")) == NULL) {

        Do_unlock_mutex(&c->realm->network_file_lock);
        sprintf(string_buffer,
                "[%s] fopen of %s failed in Do_look_network: %s.\n",
                c->connection_id, NETWORK_FILE, strerror(errno));

        Do_log_error(string_buffer);
        return FALSE;
    }

        /* run through the network entries */
    while (fread((void *)theNetwork, SZ_NETWORK, 1, network_file) == 1) {

            /* if this is the network */
        if (strcmp(networkName, theNetwork->address) == 0) {

                /* return with the good news */
            fclose(network_file);
            Do_unlock_mutex(&c->realm->network_file_lock);
            return TRUE;
        }
    }

        /* close down and return a negative */
    fclose(network_file);
    Do_unlock_mutex(&c->realm->network_file_lock);

    theNetwork->hackCount = 0;
    theNetwork->muteCount = 0;

    return FALSE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_update_network(struct client_t *c, struct network_t *theNetwork)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/06/01
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

Do_update_network(struct client_t *c, struct network_t *theNetwork)
{
    struct network_t readNetwork;
    FILE *network_file, *temp_file;
    char error_msg[SZ_ERROR_MESSAGE];
    time_t timeNow;
    bool found_flag;

    Do_lock_mutex(&c->realm->network_file_lock);

        /* get the time now */
    timeNow = time(NULL);
    found_flag = FALSE;

        /* open a temp file to transfer records to */
    errno = 0;
    if ((temp_file=fopen(TEMP_NETWORK_FILE, "w")) == NULL) {

        Do_unlock_mutex(&c->realm->network_file_lock);
        sprintf(error_msg,
                "[%s] fopen of %s failed in Do_update_network: %s.\n",
                c->connection_id, TEMP_NETWORK_FILE, strerror(errno));

        Do_log_error(error_msg);
        return FALSE;
    }

    errno = 0;
    if ((network_file=fopen(NETWORK_FILE, "r")) == NULL) {

        Do_unlock_mutex(&c->realm->network_file_lock);
        sprintf(error_msg,
                "[%s] fopen of %s failed in Do_update_network: %s\n",
                c->connection_id, NETWORK_FILE, strerror(errno));

        Do_log_error(error_msg);
    }
    else {

            /* run through the network entries */
        while (fread((void *)&readNetwork, SZ_NETWORK, 1, network_file) == 1) {

                /* if this is the network */
            if (strcmp(readNetwork.address, theNetwork->address) == 0) {

	        found_flag = TRUE;

		        /* write the passed strcture */
                if (fwrite((void *)theNetwork, SZ_NETWORK, 1, temp_file) != 1) {

                    fclose(network_file);
                    fclose(temp_file);
                    remove(TEMP_NETWORK_FILE);
                    Do_unlock_mutex(&c->realm->network_file_lock);

                    sprintf(error_msg,
                        "[%s] fwrite of %s failed in Do_update_network: %s.\n",
                        c->connection_id, TEMP_NETWORK_FILE, strerror(errno));

                    Do_log_error(error_msg);
                    return FALSE;
	        }

	        continue;
            }
            else {

                    /* see if this network is still valid */
                if (timeNow > readNetwork.expires) {

                        /* log the deletion of the network */
                    sprintf(error_msg,
			    "[%s] Deleted hack info of the network %s.\n",
                            c->connection_id, readNetwork.address);

                    Do_log(CONNECTION_LOG, error_msg);

                        /* don't save this network */
                    continue;
                }
            }

                /* write the network to the temp file */
            if (fwrite((void *)&readNetwork, SZ_NETWORK, 1, temp_file) != 1) {

                fclose(network_file);
                fclose(temp_file);
                remove(TEMP_NETWORK_FILE);
                Do_unlock_mutex(&c->realm->network_file_lock);

                sprintf(error_msg,
                     "[%s] fwrite of %s failed in Do_update_network(2): %s.\n",
                     c->connection_id, TEMP_NETWORK_FILE, strerror(errno));

                Do_log_error(error_msg);
                return FALSE;
            }
	}

        fclose(network_file);
    }

	/* if we've not written in our network, we make a new entry */
    if (!found_flag) {

            /* write the new network to the temp file */
        if (fwrite((void *)theNetwork, SZ_NETWORK, 1, temp_file) != 1) {

            fclose(temp_file);
            remove(TEMP_NETWORK_FILE);
            Do_unlock_mutex(&c->realm->network_file_lock);

            sprintf(error_msg,
                    "[%s] fwrite of %s failed in Do_update_network(3): %s.\n",
                    c->connection_id, TEMP_NETWORK_FILE, strerror(errno));

            Do_log_error(error_msg);
            return FALSE;
        }
    }

        /* close the two files */
    fclose(temp_file);

        /* delete the old character record */
    remove(NETWORK_FILE);

        /* replace it with the temporary file */
    rename(TEMP_NETWORK_FILE, NETWORK_FILE);

    Do_unlock_mutex(&c->realm->network_file_lock);
    return TRUE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_tally_ip(struct client_t *c, bool connection, short badPasswords);
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/06/01
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

Do_tally_ip(struct client_t *c, bool connection, short badPasswords)
{
    struct connection_t *connection_ptr, **connection_ptr_ptr;
    time_t timeNow;

    Do_lock_mutex(&c->realm->connections_lock);
    timeNow = time(NULL);

    connection_ptr_ptr = &c->realm->connections;

	/* run through the list looking for previous entries */
    while (*connection_ptr_ptr != NULL) {

	connection_ptr = *connection_ptr_ptr;

	    /* is this the network we're talking about? */
	if (strcmp(connection_ptr->theAddress, c->network) == 0) {

		/* if we're adding a connection */
	    if (connection) {

		    /* drop out expired connections */
		Do_drop_expired(connection_ptr->connections,
			&connection_ptr->connectionCount, timeNow - 900);

		    /* see if this connection will be too many */
		if (connection_ptr->connectionCount > 19) {
		    connection_ptr->connectionCount = 0;
		    Do_unlock_mutex(&c->realm->connections_lock);
		    Do_caught_hack(c, H_CONNECTIONS);
		    return;
		}

		connection_ptr->connections[connection_ptr->connectionCount++]
			= timeNow;

		if (connection_ptr->eraseAt < timeNow + 600) {
		    connection_ptr->eraseAt = timeNow + 600;
		}
	    }

	    if (badPasswords) {

		    /* drop out expired missed passwords */
		Do_drop_expired(connection_ptr->badPasswords,
			&connection_ptr->badPasswordCount, timeNow - 1800);

		    /* see if these additions will be too many */
		if (connection_ptr->badPasswordCount + badPasswords > 9) {
		    connection_ptr->badPasswordCount = 0;
		    Do_unlock_mutex(&c->realm->connections_lock);
		    Do_caught_hack(c, H_PASSWORDS);
		    return;
		}

		while (badPasswords > 0) {

		    connection_ptr->badPasswords[
			    connection_ptr->badPasswordCount++] = timeNow;

		    --badPasswords;
		}

		if (connection_ptr->eraseAt < timeNow + 1800) {
		    connection_ptr->eraseAt = timeNow + 1800;
		}
	    }

	    Do_unlock_mutex(&c->realm->connections_lock);
	    return;
	}
	else {

		/* see if this address should be deleted */
	    if (connection_ptr->eraseAt < timeNow) {

		    /* delete the current entry and point to the next */
		*connection_ptr_ptr = connection_ptr->next;
		free((void *)connection_ptr);
	    }
	    else {
	    	    /* increment to the next pointer */
		connection_ptr_ptr = &(*connection_ptr_ptr)->next;
	    }
	}

    }

	/* no connection record for us, so make our own */
    connection_ptr = (struct connection_t *) Do_malloc(SZ_CONNECTION);
    strcpy(connection_ptr->theAddress, c->network);
    connection_ptr->connectionCount = 0;
    connection_ptr->badPasswordCount = 0;

	/* if we're adding a connection */
    if (connection) {

	connection_ptr->connections[0] = timeNow;
	connection_ptr->connectionCount = 1;
	connection_ptr->eraseAt = timeNow + 600;
    }

    if (badPasswords) {

	while (badPasswords > 0) {

	    connection_ptr->badPasswords[
		    connection_ptr->badPasswordCount++] = timeNow;

	    --badPasswords;
	}

	connection_ptr->eraseAt = timeNow + 1800;
    }

	/* put the new connection record in place */
    connection_ptr->next = c->realm->connections;
    c->realm->connections = connection_ptr;

    Do_unlock_mutex(&c->realm->connections_lock);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_drop_expired(int theArray[], int *elements, int eraseTime)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/06/01
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

Do_drop_expired(int theArray[], int *elements, int dropBefore)
{
    int i, j;

	/* the lowest elements will be bad */
    if (theArray[0] > dropBefore) {
	return;
    }

	/* if they're all bad, we don't need to move anything */
    if (theArray[*elements - 1] < dropBefore) {
	*elements = 0;
	return;
    }

	/* run through the elements to find non-expired time */
    for (i = 1; i < *elements; i++) {
	if (theArray[i] > dropBefore) {
	    break;
	}
    }

	/* move the times down */
    for (j = i; j < *elements; j++) {
	theArray[j - i] = theArray[j];
    }

    *elements -= i;

    return;
}

	
/************************************************************************
/
/ FUNCTION NAME: Do_get_hack_string(int number, char *theString);
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 01/09/01
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

Do_get_hack_string(int number, char *theString)
{
    switch(number) {

    case H_SYSTEM:
	strcpy(theString, "hacking the system");
	break;

    case H_PASSWORDS:
	strcpy(theString, "hacking passwords");
	break;

    case H_CONNECTIONS:
	strcpy(theString, "making excessive connections");
	break;

    case H_KILLING:
	strcpy(theString, "killing characters");
	break;

    case H_PROFANITY:
	strcpy(theString, "using profanity on channel 1");
	break;

    case H_DISRESPECTFUL:
	strcpy(theString, "disrespecting the wizards");
	break;

    case H_FLOOD:
	strcpy(theString, "sending too many long messages");
	break;

    case H_SPAM:
	strcpy(theString, "sending too many messages at once");
	break;

    case H_WHIM:
	strcpy(theString, "the wizard's whim");
	break;

    default:
	strcpy(theString, "listening to Neil Diamond");
	break;
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_caught_spam(c, int theReason)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 01/06/01
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

Do_caught_spam(struct client_t *c, int theReason)
{
    struct account_t theAccount;
    struct account_mod_t theAccountMod;
    struct tag_t theTag;
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE];
    time_t timeNow;
    bool banFlag;

	/* mute times; 1 min, 5 min, 20 min, 1 hour, 4 hours, 1 day, 3 days, 1 week */
    int muteTimes[8] = {60, 300, 1200, 7200, 28800, 86400, 259200, 604800};

    timeNow = time(NULL);
    banFlag = FALSE;

	/* lock the hack lock to prevent these stats from updating by others */
    Do_lock_mutex(&c->realm->hack_lock);

	/* find past triggers, start with the character, if it exists */
	/* skip the character, if it exists, it's going to be killed */
    if (Do_look_account(c, c->lcaccount, &theAccount)) {

	    /* if previous offenses, lower count by 1 per two weeks since */
	if (theAccount.muteCount) {

            theAccount.muteCount -= (timeNow - theAccount.lastMute) /
		    1296000;

	    if (theAccount.muteCount < 0) {
		theAccount.muteCount = 0;
	    }
        }
    }

	/* find the highest number of mutes */
    if (theAccount.muteCount > c->player.muteCount) {
	c->player.muteCount = theAccount.muteCount;
    }

 /* There's an overflow with the mute count somewhere -- EH */
if (c->player.muteCount > 10 || c->player.muteCount < 0) {
  c->player.muteCount = 1;
}

	/* increment the mute count */
    ++c->player.muteCount;

	/* max mute punishment is one week */
    if (c->player.muteCount > 7) {
	c->player.muteCount = 7;
	banFlag = TRUE;
    }

	/* save the account info */
    if (c->accountLoaded) {
        Do_clear_account_mod(&theAccountMod);
        theAccountMod.mute = TRUE;
        theAccountMod.muteCount = c->player.muteCount;
        Do_modify_account(c, c->lcaccount, NULL, &theAccountMod);
    }

	/* prepare a tag */
    theTag.type = T_MUTE;
    theTag.validUntil = timeNow + muteTimes[c->player.muteCount];
    theTag.affectNetwork = FALSE;
    theTag.contagious = FALSE;
    Do_get_hack_string(theReason, theTag.description);

	/* send it */
    Do_tag_self(c, &theTag);
    Do_unlock_mutex(&c->realm->hack_lock);

    sprintf(string_buffer, "%s has been muted by the server for %s.\n", c->modifiedName, theTag.description);
    Do_broadcast(c, string_buffer);

    if (banFlag) {
	Do_caught_hack(c, theReason);
    }

    return;
}
