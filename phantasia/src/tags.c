/*
 * tags.c       Routines to track users and punish wrongdoers
 */

#include "include.h"

    char *taggedTypes[] = {"machine #", "account ", "address ", "network "};
    char *tagTypes[] = {"reject", "ban", "suicide", "mute", "prefix", "suffix"};
    char *tagDescs[] = {"rejected", "banned", "ordered to commit suicide", "muted", "tagged", "tagged"}; 

/************************************************************************
/
/ FUNCTION NAME: Do_tag_self(struct client_t *c, struct tag_t *theTag)
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

Do_tag_self(struct client_t *c, struct tag_t *theTag)
{
    struct tagged_t theTagged;
    struct history_t theHistory;
    char error_msg[SZ_ERROR_MESSAGE], string_buffer[SZ_LINE];

	/* get the next tag number */
    Do_lock_mutex(&c->realm->tag_file_lock);
    theTag->number = c->realm->nextTagNumber++;
    Do_unlock_mutex(&c->realm->tag_file_lock);

	/* write the tag */
    Do_save_tag(c, theTag);

	/* get the time the ban expires */
    ctime_r(&theTag->validUntil, error_msg);
    error_msg[strlen(error_msg) - 1] = '\0';

	/* log the tag creation */
    sprintf(string_buffer,
	    "[%s] Created %s tag #%d for \"%s\" effective until %s.\n",
	    c->connection_id, tagTypes[theTag->type], theTag->number,
	    theTag->description, error_msg);

    Do_log(HACK_LOG, string_buffer);

	/* prepare a history entry */
    theHistory.date = time(NULL);

    sprintf(theHistory.description,
	    "New %s tag #%d for \"%s\" effective until %s.\n",
	    tagTypes[theTag->type], theTag->number, theTag->description,
	    error_msg);

	/* prepare the tagged entry */
    theTagged.tagNumber = theTag->number;
    theTagged.validUntil = theTag->validUntil;

	/* tag the machine if we have one */
    if (c->machineID != 0) {

	    /* associate the machine to the tag */
	theTagged.type = T_MACHINE;
	sprintf(theTagged.name, "%ld", c->machineID);
	Do_save_tagged(c, &theTagged);

	    /* log the tag creation */
        sprintf(string_buffer, "[%s] Attached machine #%d to tag #%d.\n",
	    c->connection_id, c->machineID, theTag->number);

	Do_log(HACK_LOG, string_buffer);

	    /* add this to the history file */
	theHistory.type = T_MACHINE;
	sprintf(theHistory.name, "%ld", c->machineID);
	Do_save_history(c, &theHistory);
    }

	/* if there is an account */
    if (c->accountLoaded) {

	    /* associate the account to the tag */
	theTagged.type = T_ACCOUNT;
	strcpy(theTagged.name, c->lcaccount);
	Do_save_tagged(c, &theTagged);

	    /* log the tag creation */
        sprintf(string_buffer, "[%s] Attached account %s to tag #%d.\n",
	    c->connection_id, c->lcaccount, theTag->number);

	Do_log(HACK_LOG, string_buffer);

	    /* add this to the history file */
	theHistory.type = T_ACCOUNT;
	strcpy(theHistory.name, c->lcaccount);
	Do_save_history(c, &theHistory);
    }

	/* see if this tag should affect network */
    if (theTag->affectNetwork) {

	    /* associate the network to the tag */
	theTagged.type = T_NETWORK;
	strcpy(theTagged.name, c->network);
	Do_save_tagged(c, &theTagged);

	    /* log the tag creation */
        sprintf(string_buffer, "[%s] Attached network %s to tag #%d.\n",
	    c->connection_id, c->network, theTag->number);

	Do_log(HACK_LOG, string_buffer);

	    /* add this to the history file */
	theHistory.type = T_NETWORK;
	strcpy(theHistory.name, c->network);
	Do_save_history(c, &theHistory);
    }

	/* associate the IP to the tag */
    theTagged.type = T_ADDRESS;
    strcpy(theTagged.name, c->IP);

	/* address can be associated for 1 hour maximum */
    if (theTagged.validUntil > theHistory.date + 3600) {
        theTagged.validUntil = theHistory.date + 3600;
    }

    Do_save_tagged(c, &theTagged);

	/* log the tag creation */
    sprintf(string_buffer, "[%s] Attached address %s to tag #%d.\n",
    	    c->connection_id, c->IP, theTag->number);

    Do_log(HACK_LOG, string_buffer);

	/* add this to the history file */
    theHistory.type = T_ADDRESS;
    strcpy(theHistory.name, c->IP);
    Do_save_history(c, &theHistory);

    	/* now implement the tag */
    Do_implement_tag(c, theTag);

    return;
}

	
/************************************************************************
/
/ FUNCTION NAME: Do_save_tag(struct client_t *c, struct tag_t *theTag)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/09/01
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

int Do_save_tag(struct client_t *c, struct tag_t *theTag)
{
    FILE *tag_file;
    bool found_flag;
    char error_msg[SZ_ERROR_MESSAGE];

    Do_lock_mutex(&c->realm->tag_file_lock);

    errno = 0;
    if ((tag_file=fopen(TAG_FILE, "a")) == NULL) {

        Do_unlock_mutex(&c->realm->tag_file_lock);
        sprintf(error_msg,
                "[%s] fopen of %s failed in Do_save_tag: %s\n",
                c->connection_id, TAG_FILE, strerror(errno));

        Do_log_error(error_msg);
        return FALSE;
    }

	/* write the tag to the tag file */
    if (fwrite((void *)theTag, SZ_TAG, 1, tag_file) != 1) {

        fclose(tag_file);
        remove(TAG_FILE);
        Do_unlock_mutex(&c->realm->tag_file_lock);

        sprintf(error_msg,
                "[%s] fwrite of %s failed in Do_save_tag: %s.\n",
                c->connection_id, TEMP_TAG_FILE, strerror(errno));

        Do_log_error(error_msg);
        return FALSE;
    }

        /* close the two files */
    fclose(tag_file);
    Do_unlock_mutex(&c->realm->tag_file_lock);

    return TRUE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_look_tag(struct client_t *c, struct tag_t *theTag, int tagNumber)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/09/01
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

int Do_look_tag(struct client_t *c, struct tag_t *theTag, int tagNumber)
{
    struct tag_t readTag;
    FILE *tag_file, *temp_file;
    time_t timeNow;
    bool found_flag;
    char error_msg[SZ_ERROR_MESSAGE];

    Do_lock_mutex(&c->realm->tag_file_lock);

	/* get the time now */
    timeNow = time(NULL);
    found_flag = FALSE;

        /* open a temp file to transfer records to */
    errno = 0;
    if ((temp_file=fopen(TEMP_TAG_FILE, "w")) == NULL) {

        Do_unlock_mutex(&c->realm->tag_file_lock);
        sprintf(error_msg,
                "[%s] fopen of %s failed in Do_look_tag: %s.\n",
                c->connection_id, TEMP_TAG_FILE, strerror(errno));

        Do_log_error(error_msg);
        return FALSE;
    }

    errno = 0;
    if ((tag_file=fopen(TAG_FILE, "r")) == NULL) {

        fclose(temp_file);
        Do_unlock_mutex(&c->realm->tag_file_lock);
        sprintf(error_msg,
                "[%s] fopen of %s failed in Do_look_tag: %s\n",
                c->connection_id, TAG_FILE, strerror(errno));

        Do_log_error(error_msg);
        return FALSE;
    }

        /* run through the tag entries */
    while (fread((void *)&readTag, SZ_TAG, 1, tag_file) == 1) {

	    /* see if this tag is outdated */
	if (readTag.validUntil < timeNow) {

		/* don't rewrite it */
	    continue;
	}

	    /* this is the tag? */
	else if (readTag.number == tagNumber) {

		/* copy over the tag information */
	    memcpy(theTag, &readTag, SZ_TAG);
	    found_flag = TRUE;
	}

	    /* write the tag to the temp file */
        if (fwrite((void *)&readTag, SZ_TAG, 1, temp_file) != 1) {

            fclose(tag_file);
            fclose(temp_file);
            remove(TEMP_TAG_FILE);
            Do_unlock_mutex(&c->realm->tag_file_lock);

            sprintf(error_msg,
                    "[%s] fwrite of %s failed in Do_look_tag: %s.\n",
                    c->connection_id, TEMP_TAG_FILE, strerror(errno));

            Do_log_error(error_msg);
            return FALSE;
	}
    }

        /* close the two files */
    fclose(temp_file);
    fclose(tag_file);

        /* delete the old character record */
    remove(TAG_FILE);

        /* replace it with the temporary file */
    rename(TEMP_TAG_FILE, TAG_FILE);

    Do_unlock_mutex(&c->realm->tag_file_lock);

    if (found_flag) {
	return TRUE;
    }

    return FALSE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_save_tagged(struct client_t *c, struct tag_t *theTagged)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/11/01
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

int Do_save_tagged(struct client_t *c, struct tagged_t *theTagged)
{
    FILE *tagged_file;
    bool found_flag;
    char error_msg[SZ_ERROR_MESSAGE];
    time_t newTime;

    Do_lock_mutex(&c->realm->tagged_file_lock);

    errno = 0;
    if ((tagged_file=fopen(TAGGED_FILE, "a")) == NULL) {

        Do_unlock_mutex(&c->realm->tagged_file_lock);
        sprintf(error_msg,
                "[%s] fopen of %s failed in Do_save_tagged: %s\n",
                c->connection_id, TAGGED_FILE, strerror(errno));

        Do_log_error(error_msg);
        return FALSE;
    }

	/* write the tagged to the tag file */
    if (fwrite((void *)theTagged, SZ_TAGGED, 1, tagged_file) != 1) {

        fclose(tagged_file);
        remove(TAGGED_FILE);
        Do_unlock_mutex(&c->realm->tagged_file_lock);

        sprintf(error_msg,
                "[%s] fwrite of %s failed in Do_save_tagged: %s.\n",
                c->connection_id, TEMP_TAGGED_FILE, strerror(errno));

        Do_log_error(error_msg);
        return FALSE;
    }

        /* close the two files */
    fclose(tagged_file);
    Do_unlock_mutex(&c->realm->tagged_file_lock);

    return TRUE;
}


/************************************************************************
/
/ FUNCTION NAME: struct tagged_list_t *Do_look_tagged(struct client_t *c)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/09/01
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

struct tagged_list_t *Do_look_tagged(struct client_t *c)
{
    struct tagged_t readTagged;
    struct tagged_list_t *taggedList, *taggedList_ptr;
    FILE *tagged_file, *temp_file;
    time_t timeNow;
    char error_msg[SZ_ERROR_MESSAGE], machineID[16];

    Do_lock_mutex(&c->realm->tagged_file_lock);

	/* get the time now */
    timeNow = time(NULL);
    taggedList = NULL;
    sprintf(machineID, "%ld", c->machineID);

        /* open a temp file to transfer records to */
    errno = 0;
    if ((temp_file=fopen(TEMP_TAGGED_FILE, "w")) == NULL) {

        Do_unlock_mutex(&c->realm->tagged_file_lock);
        sprintf(error_msg,
                "[%s] fopen of %s failed in Do_look_tagged: %s.\n",
                c->connection_id, TEMP_TAGGED_FILE, strerror(errno));

        Do_log_error(error_msg);
        return NULL;
    }

	/* open the tagged file */
    errno = 0;
    if ((tagged_file=fopen(TAGGED_FILE, "r")) == NULL) {

        fclose(temp_file);
        remove(TEMP_TAGGED_FILE);
        Do_unlock_mutex(&c->realm->tagged_file_lock);
        sprintf(error_msg,
                "[%s] fopen of %s failed in Do_look_tagged: %s\n",
                c->connection_id, TAGGED_FILE, strerror(errno));

        Do_log_error(error_msg);
        return NULL;
    }

        /* run through the tag entries */
    while (fread((void *)&readTagged, SZ_TAGGED, 1, tagged_file) == 1) {

	    /* see if this tag is outdated */
	if (readTagged.validUntil < timeNow) {

		/* don't rewrite it */
	    continue;
	}

	    /* is this tagged entry pertain to us? */
	else if (

		    /* see if this machine is us */
		(readTagged.type == T_MACHINE && c->machineID > 0 &&
		strcmp(readTagged.name, machineID) == 0) ||

		    /* see if this account is ours */
		(readTagged.type == T_ACCOUNT && c->accountLoaded &&
		strcmp(readTagged.name, c->account) == 0) ||

		    /* see if this is the character's account */
		(readTagged.type == T_ACCOUNT && c->characterLoaded &&
		strcmp(readTagged.name, c->player.parent_account) == 0) ||

		    /* see if this is our address */
		(readTagged.type == T_ADDRESS && strcmp(readTagged.name,
		c->IP) == 0) ||

		    /* see if this is our network */
		(readTagged.type == T_NETWORK && strcmp(readTagged.name,
		c->network) == 0) ) {

		/* create a new list strcture to pass the info back */
	    taggedList_ptr = (struct tagged_list_t *) Do_malloc(SZ_TAGGED_LIST);
	    memcpy(&taggedList_ptr->theTagged, &readTagged, SZ_TAGGED);

		/* put the strcture into the list */
	    taggedList_ptr->next = taggedList;
	    taggedList = taggedList_ptr;
	}

	    /* write the tag to the temp file */
        if (fwrite((void *)&readTagged, SZ_TAGGED, 1, temp_file) != 1) {

            fclose(tagged_file);
            fclose(temp_file);
            remove(TEMP_TAGGED_FILE);
            Do_unlock_mutex(&c->realm->tagged_file_lock);

            sprintf(error_msg,
                    "[%s] fwrite of %s failed in Do_look_tagged: %s.\n",
                    c->connection_id, TEMP_TAGGED_FILE, strerror(errno));

            Do_log_error(error_msg);
            return taggedList;
	}
    }

        /* close the two files */
    fclose(temp_file);
    fclose(tagged_file);

        /* delete the old character record */
    remove(TAGGED_FILE);

        /* replace it with the temporary file */
    rename(TEMP_TAGGED_FILE, TAGGED_FILE);

    Do_unlock_mutex(&c->realm->tagged_file_lock);
    return taggedList;
}


/************************************************************************
/
/ FUNCTION NAME: Do_check_tags(struct client_t *c)
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

int Do_check_tags(struct client_t *c)
{
    struct tagged_list_t *taggedList, *taggedList_ptr;
    struct tag_t theTag;
    char error_msg[SZ_ERROR_MESSAGE], string_buffer[SZ_LINE];

	/* read all the tags that pertain to us */
    taggedList = Do_look_tagged(c);

	/* see if this player inherits any tags, skip if wiz backdoor */
    if (c->wizaccount[0] == '\0') {
        Do_check_inherit(c, taggedList);
    }

	/* run through the list of tagged recalled */
    while (taggedList != NULL) {

	if (Do_look_tag(c, &theTag, taggedList->theTagged.tagNumber)) {

		/* implement the tag */
	    Do_implement_tag(c, &theTag);
	}

	taggedList_ptr = taggedList;
	taggedList = taggedList->next;
	free((void *)taggedList_ptr);
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: struct tagged_list_t *Do_check_inherit(struct client_t *c)
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

Do_check_inherit(struct client_t *c, struct tagged_list_t *list_ptr)
{
    struct tagged_sort_t *the_sort, *sort_ptr;

    the_sort = NULL;

	/* run though the tagged list */
    while (list_ptr != NULL) {

	    /* run through the sort list */
	sort_ptr = the_sort;
	while (sort_ptr != NULL) {

		/* see if this list entry is this sort structure */
	    if (sort_ptr->tag == list_ptr->theTagged.tagNumber) {

		    /* point the sort strcture to this entry */
		sort_ptr->tagged[list_ptr->theTagged.type] =
			&list_ptr->theTagged;

		break;
	    }

	    sort_ptr = sort_ptr->next;
	}

	    /* if we found no sort structure */
	if (sort_ptr == NULL) {

		/* create one */
	    sort_ptr = (struct tagged_sort_t *) Do_malloc(SZ_TAGGED_SORT);
	    sort_ptr->tag = list_ptr->theTagged.tagNumber;
	    sort_ptr->tagged[0] = NULL;
	    sort_ptr->tagged[1] = NULL;
	    sort_ptr->tagged[2] = NULL;
	    sort_ptr->tagged[3] = NULL;
	    sort_ptr->tagged[list_ptr->theTagged.type] = &list_ptr->theTagged;

		/* put it in place */
	    sort_ptr->next = the_sort;
	    the_sort = sort_ptr;
	}

	list_ptr = list_ptr->next;
    }

	/* run through the sorted lists */
    while (the_sort != NULL) {

	    /* accounts pass to IP and Machine */
	if (the_sort->tagged[T_ACCOUNT] != NULL) {

		/* pass to the address if needed */
	    if (the_sort->tagged[T_ADDRESS] == NULL) {
		Do_inherit_tag(c, the_sort->tagged[T_ACCOUNT], T_ADDRESS);
	    }

		/* pass to the machine if needed */
	    if (c->machineID && the_sort->tagged[T_MACHINE] == NULL) {
		Do_inherit_tag(c, the_sort->tagged[T_ACCOUNT], T_MACHINE);
	    }
	}

	    /* IP passes to the machine */
	else if (the_sort->tagged[T_ADDRESS] != NULL) {

		/* pass to the machine if needed */
	    if (c->machineID && the_sort->tagged[T_MACHINE] == NULL) {
		Do_inherit_tag(c, the_sort->tagged[T_ADDRESS], T_MACHINE);
	    }
	}

	sort_ptr = the_sort->next;
	free((void *)the_sort);
	the_sort = sort_ptr;
    }
}
	   

/************************************************************************
/
/ FUNCTION NAME: Do_inherit_tag(struct client_t *c, struct tagged_t *source, int targetType)
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

Do_inherit_tag(struct client_t *c, struct tagged_t *source, int targetType)
{
    struct tagged_t theTagged;
    struct tag_t theTag;
    struct history_t theHistory;
    char string_buffer[SZ_LINE], sourceName[SZ_FROM + 10];
    char targetName[SZ_FROM + 10];
    time_t maxDuration;

        /* load the tag */
    Do_look_tag(c, &theTag, source->tagNumber);

        /* create a new tagged */
    theTagged.tagNumber = source->tagNumber;
    theTagged.type = targetType;

	/* prepare the history entry */
    theHistory.date = time(NULL);
    theHistory.type = targetType;

	/* Find the name for the target machine */
    switch(targetType) {

    case T_MACHINE:

	sprintf(targetName, "Machine #%d", c->machineID);
	sprintf(theTagged.name, "%d", c->machineID);
	sprintf(theHistory.name, "%d", c->machineID);
        theTagged.validUntil = source->validUntil;
	break;

    case T_ACCOUNT:

	sprintf(targetName, "Account %s", c->lcaccount);
	strcpy(theTagged.name, c->lcaccount);
	strcpy(theHistory.name, c->lcaccount);
        theTagged.validUntil = source->validUntil;
	break;

    case T_ADDRESS:

	sprintf(targetName, "Address %s", c->IP);
	strcpy(theTagged.name, c->IP);
	strcpy(theHistory.name, c->IP);
	maxDuration = theHistory.date + 3600;

	    /* IPs can only be tagged for an hour */
	if (source->validUntil < maxDuration) {
            theTagged.validUntil = source->validUntil;
	}
	else {
            theTagged.validUntil = maxDuration;
	}
	break;

    case T_NETWORK:

	sprintf(targetName, "Network %s", c->network);
	strcpy(theTagged.name, c->network);
	strcpy(theHistory.name, c->network);
        theTagged.validUntil = source->validUntil;
	break;
    }

    Do_save_tagged(c, &theTagged);

        /* get the time the ban expires */
    ctime_r(&theTag.validUntil, string_buffer);
    string_buffer[strlen(string_buffer) - 1] = '\0';

    sprintf(theHistory.description,
            "Inherited %d tag #%d from %s%s for \"%s\" effective until %s.\n",
            tagTypes[theTag.type], theTag.number, taggedTypes[source->type],
	    source->name, theTag.description, string_buffer);

    Do_save_history(c, &theHistory);

    sprintf(string_buffer, "[%s] %s inherited %s tag #%d from %s%s.\n",
	    c->connection_id, targetName, tagTypes[theTag.type],
	    theTag.number, taggedTypes[source->type], source->name);

    Do_log(HACK_LOG, string_buffer);
}


/************************************************************************
/
/ FUNCTION NAME: Do_implement_tag(struct client_t *c, struct tag_t *theTag)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 01/14/01
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

Do_implement_tag(struct client_t *c, struct tag_t *theTag)
{
    char error_msg[SZ_ERROR_MESSAGE];
    char string_buffer[SZ_LINE];
    struct event_t *event_ptr;

	/* now implement the tag */
    switch (theTag->type) {

    case T_BAN:

	    /* add this event to the connection log */
        sprintf(error_msg, "[%s] Connection banned by tag #%d.\n",
	        c->connection_id, theTag->number);

        Do_log(CONNECTION_LOG, error_msg);

	    /* send a message to the user */
        Do_send_error(c, 
		"This location is no longer permitted to play Phantasia.\n");

        if (c->characterLoaded == TRUE) {
            c->run_level = SAVE_AND_EXIT;
        }
        else {
            c->run_level = EXIT_THREAD;
        }

        break;

    case T_SUICIDE:

	if (c->characterLoaded) {
            event_ptr = (struct event_t *) Do_create_event();
            event_ptr->type = DEATH_EVENT;
            event_ptr->to = c->game;
            event_ptr->from = c->game;
            event_ptr->arg3 = K_SUICIDE;
            Do_send_event(event_ptr);
	}
        break;
		
    case T_MUTE:

	if (c->characterLoaded) {

	        /* send a message to the user */
            sprintf(string_buffer, "Your characters are muted due to %s.\n",
		    theTag->description);

                /* send off an error dialog */
            Do_dialog(c, string_buffer);
	}

	c->muteUntil = theTag->validUntil;
        break;

    case T_PREFIX:

	if (c->characterLoaded) {
            strcpy(string_buffer, theTag->description);
            strcat(string_buffer, " ");
	    strcat(string_buffer, c->player.name);

	    if (c->characterAnnounced) {
                Do_send_specification(c, REMOVE_PLAYER_EVENT);
	    }

            strncpy(c->modifiedName, string_buffer, SZ_NAME - 1);

	    if (c->characterAnnounced) {
                Do_send_specification(c, ADD_PLAYER_EVENT);
	        Do_name(c);
	    }
	}
	c->tagUntil = theTag->validUntil;
	break;

    case T_SUFFIX:

	if (c->characterLoaded) {
	    strcpy(string_buffer, c->player.name);
            strcat(string_buffer, " ");
            strcat(string_buffer, theTag->description);

	    if (c->characterAnnounced) {
                Do_send_specification(c, REMOVE_PLAYER_EVENT);
	    }

            strncpy(c->modifiedName, string_buffer, SZ_NAME - 1);

	    if (c->characterAnnounced) {
                Do_send_specification(c, ADD_PLAYER_EVENT);
	        Do_name(c);
	    }
	}
	c->tagUntil = theTag->validUntil;
	break;
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_create_tag(struct client_t *c, int tagType)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 01/14/01
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

Do_create_tag(struct client_t *c, int tagType)
{
    struct button_t buttons;
    struct event_t *eventPtr;
    struct tag_t theTag, *tagPtr;
    struct tagged_t theTagged;
    struct history_t theHistory;
    long answer;
    char error_msg[SZ_ERROR_MESSAGE];
    char string_buffer[SZ_LINE];
    char tagee[SZ_NAME];
    int i;
    time_t timeNow;

    int tagTimes[] = {3600, 10800, 86400, 259200, 604800, 2592000, 7776000, 31536000};

	/* start filling out the tag */
    timeNow = time(NULL);
    theTag.type = tagType;
    theTag.affectNetwork = FALSE;
    theTag.contagious = FALSE;

    sprintf(string_buffer, "What life would you like this %s to have? (shorter times under cantrip)\n",
	    tagTypes[tagType]);

    Do_send_line(c, string_buffer);

    strcpy(buttons.button[0], "1 hour\n");
    strcpy(buttons.button[1], "3 hours\n");
    strcpy(buttons.button[2], "1 day\n");
    strcpy(buttons.button[3], "3 days\n");
    strcpy(buttons.button[4], "1 week\n");
    strcpy(buttons.button[5], "1 month\n");
    strcpy(buttons.button[6], "3 months\n");
    strcpy(buttons.button[7], "1 year\n");
    Do_clear_buttons(&buttons, 8);

    if (Do_buttons(c, &answer, &buttons) != S_NORM) {
        Do_send_clear(c);
        return;
    }

    Do_send_clear(c);

    if (answer > 7 || answer < 0) {

        sprintf(error_msg,
                "[%s] Returned non-option in Do_create_tags.\n",
                c->connection_id);

        Do_log_error(error_msg);
        Do_caught_hack(c, H_SYSTEM);
        return;
    }

        /* convert to a time */
    theTag.validUntil = timeNow + tagTimes[answer];

    if (tagType == T_PREFIX || tagType == T_SUFFIX) {

	sprintf(string_buffer,
	        "What %s would you like to add to this character's name?\n",
	        tagTypes[tagType]);
    }
    else {

	sprintf(string_buffer,
	    "This %s is being created for: (e.g. \"claiming N'Sync rules\".\n",
	    tagTypes[tagType]);
    }

    if (Do_string_dialog(c, &theTag.description, SZ_LINE - 1, string_buffer)) {
        return;
    }

	/* find out where it goes */
    sprintf(string_buffer, "What do you wish to %s?\n", tagTypes[tagType]);
    Do_send_line(c, string_buffer);

    strcpy(buttons.button[0], "Player\n");
    strcpy(buttons.button[1], "Machine\n");
    strcpy(buttons.button[2], "Account\n");
    strcpy(buttons.button[3], "Address\n");
    strcpy(buttons.button[4], "Network\n");
    Do_clear_buttons(&buttons, 5);
    strcpy(buttons.button[7], "Cancel\n");

    if (Do_buttons(c, &answer, &buttons) != S_NORM || answer == 7) {
	Do_send_clear(c);
        return;
    }

    Do_send_clear(c);
    if (answer > 5 || answer < 0) {

        sprintf(error_msg,
                "[%s] Returned non-option in Do_create_tags(2).\n",
                c->connection_id);

        Do_log_error(error_msg);
        Do_caught_hack(c, H_SYSTEM);
        return;
    }

    switch(answer) {

    case 0:

        sprintf(string_buffer, "Who do you wish to %s?\n", tagTypes[tagType]);

        if (Do_player_dialog(c, string_buffer, tagee) != S_NORM) {
            return;
        }

	    /* convert the tag to an object */
	tagPtr = (struct tag_t *) Do_malloc(SZ_TAG);
	memcpy(tagPtr, &theTag, SZ_TAG);

	    /* create the event */
        eventPtr = (struct event_t *) Do_create_event();
        eventPtr->type = TAG_EVENT;
        eventPtr->from = c->game;
	if(tagPtr->type == T_MUTE)
	{
		eventPtr->arg3 = T_MUTE;
	}
        eventPtr->arg4 = tagPtr;

	    /* send off the event */
        if (!Do_send_character_event(c, eventPtr, tagee)) {
            free((void *)eventPtr);
	    free((void *)tagPtr);
            Do_send_line(c, "That character just left the game.\n");
            Do_more(c);
	    Do_send_clear(c);
            return;
	}

        Do_send_line(c, "The tag has been sent.\n");
        Do_more(c);
        Do_send_clear(c);
	return;

    case 1:

        sprintf(string_buffer, "What machine number do you wish to %s?\n",
		tagTypes[tagType]);

            /* associate the machine to the tag */
        theTagged.type = T_MACHINE;
	theHistory.type = T_MACHINE;

	break;

    case 2:

        sprintf(string_buffer, "What account do you wish to %s?\n",
		tagTypes[tagType]);

            /* associate the account to the tag */
        theTagged.type = T_ACCOUNT;
	theHistory.type = T_ACCOUNT;

	break;

    case 3:

        sprintf(string_buffer, "What address do you wish to %s?\n",
		tagTypes[tagType]);

            /* associate the address to the tag */
        theTagged.type = T_ADDRESS;
	theHistory.type = T_ADDRESS;

	break;

    case 4:

        sprintf(string_buffer, "What network do you wish to %s?\n",
		tagTypes[tagType]);

            /* associate the network to the tag */
        theTagged.type = T_NETWORK;
	theHistory.type = T_NETWORK;

	break;

    default:

        sprintf(error_msg, "[%s] Returned non-option %ld in Do_create_tag.\n",
                c->connection_id, answer);

        Do_log_error(error_msg);
        Do_caught_hack(c, H_SYSTEM);
	return;
    }

	/* get the item to tag */
    if (Do_string_dialog(c, &tagee, SZ_NAME - 1, string_buffer)
	    != S_NORM) {

        return;
    }

        /* get the next tag number */
    Do_lock_mutex(&c->realm->tag_file_lock);
    theTag.number = c->realm->nextTagNumber++;
    Do_unlock_mutex(&c->realm->tag_file_lock);

        /* write the tag */
    Do_save_tag(c, &theTag);

	/* fill out the tagged and file it */
    Do_lowercase(&theTagged.name, tagee);
    theTagged.tagNumber = theTag.number;
    theTagged.validUntil = theTag.validUntil;
    Do_save_tagged(c, &theTagged);

        /* put together something for the logs */
    ctime_r(&theTagged.validUntil, error_msg);
    error_msg[strlen(error_msg) - 1] = '\0';
    theHistory.date = timeNow;
    strcpy(theHistory.name, theTagged.name);

    sprintf(theHistory.description,
	    "%s created %s tag #%d for \"%s\" effective until %s.\n",
            c->modifiedName, tagTypes[tagType], theTag.number,
	    theTag.description, error_msg);

    Do_save_history(c, &theHistory);

    sprintf(string_buffer,
	"[%s] %s created %s tag #%d for \"%s\" effective until %s.\n",
	c->connection_id, c->modifiedName, tagTypes[tagType],
	theTag.number, theTag.description, error_msg);

    Do_log(HACK_LOG, string_buffer);

    sprintf(string_buffer, "[%s] Attached %s%s to tag #%d.\n", c->connection_id,
	    taggedTypes[tagType], theTagged.name, theTag.number);

    Do_log(HACK_LOG, string_buffer);

    Do_send_line(c, "The tag has been sent.\n");
    Do_more(c);
    Do_send_clear(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_create_minitag(struct client_t *c, int tagType)
/
/ FUNCTION: allow trusted players to tag other players for short periods
/
/ AUTHOR: Brian Kelly, 01/14/01
/
/ ARGUMENTS:
/       struct client_t c - pointer to the main client strcture
/
/ RETURN VALUE: 
/
/ MODULES CALLED: strcpy()
/
/ DESCRIPTION:
/
*************************************************************************/

Do_create_minitag(struct client_t *c, int tagType)
{
    struct button_t buttons;
    struct event_t *eventPtr;
    struct tag_t theTag, *tagPtr;
    struct tagged_t theTagged;
    struct history_t theHistory;
    long answer;
    char error_msg[SZ_ERROR_MESSAGE];
    char string_buffer[SZ_LINE];
    char tagee[SZ_NAME];
    int i;
    time_t timeNow;

    int tagTimes[] = {60, 300, 900, 1800, 3600, 10800};

	/* start filling out the tag */
    timeNow = time(NULL);
    theTag.type = tagType;
    theTag.affectNetwork = FALSE;
    theTag.contagious = FALSE;

    sprintf(string_buffer, "How long would you like this %s to be?\n",
	    tagTypes[tagType]);

    Do_send_line(c, string_buffer);

    strcpy(buttons.button[0], "1 min\n");
    strcpy(buttons.button[1], "5 min\n");
    strcpy(buttons.button[2], "15 min\n");
    strcpy(buttons.button[3], "30 min\n");
    if (tagType != T_BAN) {
        strcpy(buttons.button[4], "1 hour\n");
        strcpy(buttons.button[5], "3 hours\n");
        Do_clear_buttons(&buttons, 6);
    } else {
        Do_clear_buttons(&buttons, 4);
    }
    strcpy(buttons.button[7], "Cancel\n");

    if (Do_buttons(c, &answer, &buttons) != S_NORM || answer == 7) {
        Do_send_clear(c);
        return;
    }

    Do_send_clear(c);

    if (answer > 7 || answer < 0) {

        sprintf(error_msg,
                "[%s] Returned non-option in Do_create_minitags.\n",
                c->connection_id);

        Do_log_error(error_msg);
        Do_caught_hack(c, H_SYSTEM);
        return;
    }

        /* convert to a time */
    theTag.validUntil = timeNow + tagTimes[answer];

    if (tagType == T_PREFIX || tagType == T_SUFFIX) {

	sprintf(string_buffer,
	        "What %s would you like to add to this character's name?\n",
	        tagTypes[tagType]);
    }
    else {

	sprintf(string_buffer,
	    "This %s is being created for: (e.g. \"being a moron\".\n",
	    tagTypes[tagType]);
    }

    if (Do_string_dialog(c, &theTag.description, SZ_LINE - 1, string_buffer)) {
        return;
    }

    sprintf(string_buffer, "Who do you wish to %s?\n", tagTypes[tagType]);

    if (Do_player_dialog(c, string_buffer, tagee) != S_NORM) {
        return;
    }

        /* convert the tag to an object */
    tagPtr = (struct tag_t *) Do_malloc(SZ_TAG);
    memcpy(tagPtr, &theTag, SZ_TAG);

        /* create the event */
    eventPtr = (struct event_t *) Do_create_event();
    eventPtr->type = TAG_EVENT;
    eventPtr->from = c->game;
    if(tagPtr->type == T_MUTE)
    {
	    eventPtr->arg3 = T_MUTE;
    }
    eventPtr->arg4 = tagPtr;

        /* send off the event */
    if (!Do_send_character_event(c, eventPtr, tagee)) {
        free((void *)eventPtr);
        free((void *)tagPtr);
        Do_send_line(c, "That character just left the game.\n");
        Do_more(c);
        Do_send_clear(c);
        return;
    }

        /* get the time the ban expires */
    ctime_r(&theTag.validUntil, error_msg);
    error_msg[strlen(error_msg) - 1] = '\0';

    sprintf(string_buffer,
	"[%s] %s created %s tag on %s for \"%s\" effective until %s.\n",
	c->connection_id, c->modifiedName, tagTypes[tagType], tagee,
	theTag.description, error_msg);

    Do_log(CANTRIP_LOG, string_buffer);

    Do_send_line(c, "The cantrip has been sent.\n");

    if ((tagType != T_PREFIX) && (tagType != T_SUFFIX)) {

        sprintf(string_buffer,
	    "%s was %s by %s for %s.\n", 
            tagee, tagDescs[tagType], c->modifiedName, theTag.description);
        Do_broadcast(c, string_buffer);

    }
    return;
}

/************************************************************************
/
/ FUNCTION NAME: int Do_remove_tagged(struct client_t *c, int tageeType, char *tageeName, int tagNumber)
/
/ FUNCTION: find location in player file of given name
/
/ AUTHOR: Brian Kelly, 01/19/01
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

int Do_remove_tagged(struct client_t *c, int tageeType, char *tageeName, int tagNumber)
{
    struct tagged_t readTagged;
    FILE *tagged_file, *temp_file;
    time_t timeNow;
    char error_msg[SZ_ERROR_MESSAGE];
    bool foundTagged;

    Do_lock_mutex(&c->realm->tagged_file_lock);

	/* get the time now */
    timeNow = time(NULL);

        /* open a temp file to transfer records to */
    errno = 0;
    if ((temp_file=fopen(TEMP_TAGGED_FILE, "w")) == NULL) {

        Do_unlock_mutex(&c->realm->tagged_file_lock);
        sprintf(error_msg,
                "[%s] fopen of %s failed in Do_remove_tagged: %s.\n",
                c->connection_id, TEMP_TAGGED_FILE, strerror(errno));

        Do_log_error(error_msg);
        return FALSE;
    }

    errno = 0;
    if ((tagged_file=fopen(TAGGED_FILE, "r")) == NULL) {

        fclose(temp_file);
        Do_unlock_mutex(&c->realm->tagged_file_lock);
        sprintf(error_msg,
                "[%s] fopen of %s failed in Do_remove_tagged: %s\n",
                c->connection_id, TAGGED_FILE, strerror(errno));

        Do_log_error(error_msg);
        return FALSE;
    }

    foundTagged = FALSE;

        /* run through the tag entries */
    while (fread((void *)&readTagged, SZ_TAGGED, 1, tagged_file) == 1) {

	    /* this is the a tagged strcture we want? */
	if (readTagged.tagNumber == tagNumber && readTagged.type == tageeType
		&& strcmp(readTagged.name, tageeName) == 0) {

		/* mark it as deleted */
	    foundTagged = TRUE;

		/* and don't re-write it */
	    continue;
	}

	    /* see if this tag is outdated */
	if (readTagged.validUntil < timeNow) {

		/* don't rewrite it */
	    continue;
	}

	    /* write the tag to the temp file */
        if (fwrite((void *)&readTagged, SZ_TAGGED, 1, temp_file) != 1) {

            fclose(tagged_file);
            fclose(temp_file);
            remove(TEMP_TAGGED_FILE);
            Do_unlock_mutex(&c->realm->tagged_file_lock);

            sprintf(error_msg,
                    "[%s] fwrite of %s failed in Do_remove_tagged: %s.\n",
                    c->connection_id, TEMP_TAGGED_FILE, strerror(errno));

            Do_log_error(error_msg);
            return FALSE;
	}
    }

        /* close the two files */
    fclose(temp_file);
    fclose(tagged_file);

        /* delete the old character record */
    remove(TAGGED_FILE);

        /* replace it with the temporary file */
    rename(TEMP_TAGGED_FILE, TAGGED_FILE);

    Do_unlock_mutex(&c->realm->tagged_file_lock);

    if (foundTagged) {
	return TRUE;
    }

    return FALSE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_untag(struct client_t *c)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 01/18/01
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

Do_untag(struct client_t *c)
{
    struct button_t buttons;
    struct history_t theHistory;
    long tageeType, tagNumber;
    char error_msg[SZ_ERROR_MESSAGE];
    char string_buffer[SZ_LINE];
    char tagee[SZ_NAME];
    int i;

    Do_send_line(c, "Do you wish to untag an account or an address?\n");

    strcpy(buttons.button[0], "Machine\n");
    strcpy(buttons.button[1], "Account\n");
    strcpy(buttons.button[2], "Address\n");
    strcpy(buttons.button[3], "Network\n");
    Do_clear_buttons(&buttons, 4);
    strcpy(buttons.button[7], "Cancel\n");

    if (Do_buttons(c, &tageeType, &buttons) != S_NORM) {
        Do_send_clear(c);
        return;
    }

    Do_send_clear(c);
    switch (tageeType) {

    case 0:
	tageeType = T_MACHINE;

        if (Do_string_dialog(c, tagee, SZ_NAME - 1,
	        "What machine do you wish to untag?\n") != S_NORM) {

            return;
        }
	break;

    case 1:
	tageeType = T_ACCOUNT;

        if (Do_string_dialog(c, tagee, SZ_NAME - 1,
	        "What account do you wish to untag?\n") != S_NORM) {

            return;
        }
	break;

    case 2:
	tageeType = T_ADDRESS;

        if (Do_string_dialog(c, tagee, SZ_NAME - 1,
	        "What address do you wish to untag?\n") != S_NORM) {

            return;
        }
	break;

    case 3:
	tageeType = T_NETWORK;

        if (Do_string_dialog(c, tagee, SZ_NAME - 1,
	        "What network do you wish to untag?\n") != S_NORM) {

            return;
        }
	break;

    case 7:
	return;
	break;

    default:

        sprintf(error_msg, "[%s] Returned non-option %ld in Do_untag.\n",
                c->connection_id, tageeType);

        Do_log_error(error_msg);
        Do_caught_hack(c, H_SYSTEM);
	return;
    }

    Do_lowercase(&tagee, &tagee);

    if (Do_long_dialog(c, &tagNumber, "What is the tag number to remove?\n")) {
        return;
    }

    if (Do_remove_tagged(c, (int) tageeType, tagee, (int) tagNumber)) {

        theHistory.date = time(NULL);
        theHistory.type = (int) tageeType;
        strcpy(theHistory.name, tagee);

	sprintf(theHistory.description, "%s removed connection to tag #%d.\n",
                c->modifiedName, tagNumber);

        Do_save_history(c, &theHistory);

	Do_send_line(c, "The tag was successfully removed.\n");
    }
    else {

	Do_send_line(c,
		"I could not find the tag number or the account/address.\n");
    }

    Do_more(c);
    Do_send_clear(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_remove_prefix_suffix(struct client_t *c)
/
/ FUNCTION: Remove a prefix or suffix tag from a player.
/
/ AUTHOR: Renee Gehlbach, 8/6/2002
/
/ ARGUMENTS:
/       struct client_t c - pointer to the main client strcture
/
/ RETURN VALUE: 
/
/ MODULES CALLED: strcpy(), strncpy(), Do_send_specification(), Do_name()
/
/ DESCRIPTION:
/       Remove a prefix or suffix from a player.
/
*************************************************************************/

Do_remove_prefix_suffix(struct client_t *c)
{
	char string_buffer[SZ_NAME];
	
	if(c->characterLoaded)
	{
		if(strcmp(c->player.name, c->modifiedName) != 0)
		{
			strcpy(string_buffer, c->player.name);
			if(c->characterAnnounced)
			{
				Do_send_specification(c, REMOVE_PLAYER_EVENT);
			}
			strncpy(c->modifiedName, string_buffer, SZ_NAME - 1);
			if(c->characterAnnounced)
			{
				Do_send_specification(c, ADD_PLAYER_EVENT);
				Do_name(c);
			}
		}
	}
	return;
}



/************************************************************************
/
/ FUNCTION NAME: int Do_tag_muted(struct client_t *c, stuct tag_t *theMute)
/
/ FUNCTION: Give muted players a tag of "the Silent" which lasts for the
/           duration of their mute
/
/ AUTHOR: Renee Gehlbach, 6/24/2002
/
/ ARGUMENTS: struct client_t *c - stu
/            
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


int Do_tag_muted(struct client_t *c, struct tag_t *theMute)
{
	struct event_t *eventPtr;
	struct tag_t theTag, *tagPtr;
	char string_buffer[SZ_LINE], error_msg[SZ_LINE];
	char tagee[SZ_NAME];

	sprintf(tagee, c->player.name);

	/* fill out the tag */
	theTag.type = T_SUFFIX;
	theTag.validUntil = theMute->validUntil;
	theTag.affectNetwork = theMute->affectNetwork;
	theTag.contagious = theMute->contagious;
	sprintf(theTag.description, "the Silent");

	/* convert the tag to an object */
	tagPtr = (struct tag_t *) Do_malloc(SZ_TAG);
	memcpy (tagPtr, &theTag, SZ_TAG);

	/* create the event */
	eventPtr = (struct event_t *) Do_create_event();
	eventPtr->type = TAG_EVENT;
	eventPtr->from = c->game;
	if(tagPtr->type == T_MUTE)
	{
		eventPtr->arg3 = T_MUTE;
	}
	eventPtr->arg4 = tagPtr;

	/* send off the event */
	if (!Do_send_character_event(c, eventPtr, tagee))
	{
		free((void *)eventPtr);
		free((void *)tagPtr);
		Do_more(c);
		Do_send_clear(c);
		return;
	}

	ctime_r(&theTag.validUntil, error_msg);
	error_msg[strlen(error_msg) - 1] = '\0';

	return;
}
