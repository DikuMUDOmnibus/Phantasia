/*
 * init.c - startup/shutdown routines for Phantasia
 */

#include "include.h"

extern int server_hook;
extern randomStateBuffer;
extern randData;

/************************************************************************
/
/ FUNCTION NAME: Do_initialize(struct server_t *server)
/
/ FUNCTION: To initialize the program's variables
/
/ AUTHOR: Brian Kelly, 4/6/99
/
/ ARGUMENTS:
/       struct server_t *s - address of the server's main data strcture
/
/ RETURN VALUE: short error
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

Init_server(struct server_t *s)
{
    char error_msg[SZ_ERROR_MESSAGE];
    int error;
    struct realm_object_t *object_ptr;

	/* seed the random number generator */
    initstate_r (time(NULL), (char *)&randomStateBuffer, STATELEN,
	    (struct random_data *)&randData);

	/* initialize realm variables */
    server_hook = s->run_level;
    s->realm.serverPid = getpid();
    s->realm.objects = NULL;
    s->realm.games = NULL;
    s->realm.king = NULL;
    s->realm.valar = NULL;
    s->realm.king_flag = FALSE;
    s->realm.king_name[0] = '\0';
    s->realm.valar_name[0] = '\0';
    s->realm.name_limbo = NULL;
    s->realm.email_limbo = NULL;
    s->realm.connections = NULL;
    s->realm.steward_gold = 0;

	/* initialize all the realm mutexes */
    Do_init_mutex(&s->realm.realm_lock);

    Do_init_mutex(&s->realm.backup_lock);
    Do_init_mutex(&s->realm.scoreboard_lock);
    Do_init_mutex(&s->realm.account_lock);
    Do_init_mutex(&s->realm.character_file_lock);
    Do_init_mutex(&s->realm.log_file_lock);
    Do_init_mutex(&s->realm.network_file_lock);
    Do_init_mutex(&s->realm.tag_file_lock);
    Do_init_mutex(&s->realm.tagged_file_lock);
    Do_init_mutex(&s->realm.history_file_lock);

    Do_init_mutex(&s->realm.monster_lock);
    Do_init_mutex(&s->realm.object_lock);
    Do_init_mutex(&s->realm.kings_gold_lock);
    Do_init_mutex(&s->realm.hack_lock);
    Do_init_mutex(&s->realm.connections_lock);

	/* set the number to tags to the time */
    s->realm.nextTagNumber = Do_get_next_tag();

	/* load the realm objects and the king's gold */
    s->realm.objects = NULL;
    error = Do_load_data_file(&s->realm);

	/* if there's an error, we can continue, just with no objects */
    if (error) {

        sprintf(error_msg, "[0.0.0.0:%d] Phantasia will continue with no objects from previous games.\n", s->realm.serverPid);

        Do_log_error(error_msg);

	    /* set kings and stewards gold to zero */
        s->realm.kings_gold = 0;

	    /* only the grail should exist */
	while (s->realm.objects != NULL) {
	    object_ptr = s->realm.objects;
	    s->realm.objects = object_ptr->next_object;
	    free((void *)object_ptr);
	}

        Do_hide_grail(&s->realm, 5000);
        Do_hide_trove(&s->realm);
    }

	/* load the monsters into the realm array */
    Do_load_monster_file(&s->realm);

	/* load the charstats into the realm array */
    Do_load_charstats_file(&s->realm);
	
	/* load the shop items into the realm array */
    Do_load_shopitems_file(&s->realm);

	/* restore any characters in the backup file */
    Do_backup_restore();
	
	/* initialize the server variables */
    s->num_games = 0;

	/* start up the socket connection */
    s->the_socket = Do_init_server_socket();

	/* no problems */
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_load_data_file(struct *realm_t)
/
/ FUNCTION: To load saved configuration information
/
/ AUTHOR: Brian Kelly, 4/8/99
/
/ ARGUMENTS:
/       struct realm_t *the_realm - pointer to the realm
/
/ RETURN VALUE: short error
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

int Do_load_data_file(the_realm)

    struct realm_t *the_realm;

{
    FILE *data_file;
    struct realm_object_t *object_ptr;
    struct realm_state_t *state_ptr;
    char error_msg[SZ_ERROR_MESSAGE];
    int error;
    int grailCount = 0, troveCount = 0;

    state_ptr = (struct realm_state_t *) Do_malloc(SZ_REALM_STATE);

	/* open the data file - for king's gold and corpses */
    errno = 0;
    if ((data_file = fopen(DATA_FILE, "r")) == NULL) {

        sprintf(error_msg,
		"[0.0.0.0:%d] fopen of %s failed in Do_load_data_file: %s\n",
		the_realm->serverPid, DATA_FILE, strerror(errno));

        Do_log_error(error_msg);
        return DATA_FILE_ERROR;
    }

    if (fread((void *) state_ptr, SZ_REALM_STATE, 1, data_file) != 1) {

        sprintf(error_msg,
		"[0.0.0.0:%d] fread of %s failed in Do_load_data_file: %d\n",
		the_realm->serverPid, DATA_FILE, ferror(data_file));

        Do_log_error(error_msg);
        fclose(data_file);
        return DATA_FILE_ERROR;
    }

    the_realm->kings_gold = state_ptr->kings_gold;
    strcpy(the_realm->king_name, state_ptr->king_name);
    strcpy(the_realm->valar_name, state_ptr->valar_name);

	/* destroy the temporary realm state object */
    free((void *)state_ptr);

	/* first entry is king's gold as double float */
/*
    if (fread((void *) &the_realm->kings_gold, sizeof(the_realm->kings_gold),
	    1, data_file) != 1) {

        sprintf(error_msg,
		"[0.0.0.0:%d] fread of %s failed in Do_load_data_file: %d\n",
		the_realm->serverPid, DATA_FILE, ferror(data_file));

        Do_log_error(error_msg);
        fclose(data_file);
        return DATA_FILE_ERROR;
    }
*/

	/* all remaining items are realm objects */
	/* create a structure to read into */
    object_ptr = (struct realm_object_t *) Do_malloc(SZ_REALM_OBJECT);

	/* read next object while not at EOF */
    while (fread((void *) object_ptr, SZ_REALM_OBJECT, 1, data_file) == 1) {

        switch (object_ptr->type) {

        case CORPSE:

                /* create an strcture to hold the character information */
            object_ptr->arg1 = (void *) malloc(SZ_PLAYER);

	        /* and read in the dead player's information */
            if (fread(object_ptr->arg1, SZ_PLAYER, 1, data_file) == 0) {

                sprintf(error_msg,
	       "[0.0.0.0:%d] fread of %s failed in Do_load_data_file(2): %d\n",
	       the_realm->serverPid, DATA_FILE, ferror(data_file));

                Do_log_error(error_msg);
                fclose(data_file);
		free((void *)object_ptr->arg1);
		free((void *)object_ptr);
                return DATA_FILE_ERROR;
	    }

                /* used to be CORPSE_LIFE */
	    if (86400 * sqrt(((struct player_t *) object_ptr->arg1)->level) 
                < time(NULL) - ((struct player_t *)object_ptr->arg1)->last_load) {

		    /* delete the object */
		free((void *)object_ptr->arg1);
		free((void *)object_ptr);
            }
	    else {
	            /* put the new corpse in the realm object list */
        	object_ptr->next_object = the_realm->objects;
        	the_realm->objects = object_ptr;
	    }

            break;

            /* if the object is the holy grail, we do nothing special */
        case HOLY_GRAIL:

	        /* put the item in the realm object list */
            object_ptr->next_object = the_realm->objects;
            the_realm->objects = object_ptr;
	    ++grailCount;
            break;

            /* if the object is the treasure trove, we do nothing special */
        case TREASURE_TROVE:

	        /* put the item in the realm object list */
            object_ptr->next_object = the_realm->objects;
            the_realm->objects = object_ptr;
	    ++troveCount;
            break;

            /* anything else is an error */
        default:

            sprintf(error_msg,
      "[0.0.0.0:%d] bad realm object of type %hd read in Do_load_data_file.\n",
      the_realm->serverPid, object_ptr->type);

            Do_log_error(error_msg);
            fclose(data_file);
	    free((void *)object_ptr);
            return DATA_FILE_ERROR;
        }

	    /* create a new temporary realm object */        
        object_ptr = (struct realm_object_t *)malloc(SZ_REALM_OBJECT);
    }

	/* destroy the remaining temporary realm object */
    free((void *)object_ptr);

	/* if we found no grail, hide a new one */
    if (grailCount == 0) {

	sprintf(error_msg,
     "[0.0.0.0:%d] No grail found in Do_load_data_file. Creating a new one.\n",
     the_realm->serverPid);

	Do_log_error(error_msg);
        Do_hide_grail(the_realm, 5000);
    }
    else if (grailCount != 1) {

	sprintf(error_msg,
	   "[0.0.0.0:%d] Read in %d grails in Do_load_data_file.\n",
	   the_realm->serverPid, grailCount);

	Do_log_error(error_msg);
	return DATA_FILE_ERROR;
    }
    
	/* if we found no treasure trove, hide a new one */
    if (troveCount == 0) {

	sprintf(error_msg, "[0.0.0.0:%d] No treasure trove found in Do_load_data_file. Creating a new one.\n", the_realm->serverPid);

	Do_log_error(error_msg);
        Do_hide_trove(the_realm);
    }
    else if (troveCount != 1) {

	sprintf(error_msg,
	   "[0.0.0.0:%d] Read in %d treasure troves in Do_load_data_file.\n",
	   the_realm->serverPid, troveCount);

	Do_log_error(error_msg);
	return DATA_FILE_ERROR;
    }

    return 0;	/* no problems */
}


/************************************************************************
/
/ FUNCTION NAME: Do_load_monster_file(struct realm_t *the_realm)
/
/ FUNCTION: To load saved configuration information
/
/ AUTHOR: Brian Kelly, 4/8/99
/
/ ARGUMENTS:
/       struct realm_t *the_realm - pointer to the realm strcture
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

Do_load_monster_file(struct realm_t *the_realm)
{
    FILE *monster_file;
    char error_msg[SZ_ERROR_MESSAGE];
    int i;

	/* load monster information */
    errno = 0;

	/* open the monster file */
    if ((monster_file = fopen(MONSTER_FILE, "r")) == NULL) {

        sprintf(error_msg,
	       "[0.0.0.0:%d] fopen of %s failed in Do_load_monster_file: %s\n",
	       the_realm->serverPid, MONSTER_FILE, strerror(errno));

        Do_log_error(error_msg);
        exit(MONSTER_FILE_ERROR);
    }

	/* read each line of the monster file */
    for (i = 0; i < NUM_MONSTERS; i++) {

	    /* read the monster name which is on its own line */
        if (fgets(the_realm->monster[i].name, SZ_MONSTER_NAME, monster_file)
		== NULL) {

            sprintf(error_msg, "[0.0.0.0:%d] fgets of %s failed on call number %d in Do_load_monster_file.\n", the_realm->serverPid, MONSTER_FILE, i);

            Do_log_error(error_msg);
            exit(MONSTER_FILE_ERROR);
        }

             /* reEmove trailing blanks */
        Do_truncstring(the_realm->monster[i].name);

	    /* read the stat line for each monster */
        if (fscanf(monster_file, "%lf %lf %lf %lf %lf %hd %hd %hd\n",
		&the_realm->monster[i].strength, &the_realm->monster[i].brains,
		&the_realm->monster[i].speed, &the_realm->monster[i].energy,
		&the_realm->monster[i].experience,
		&the_realm->monster[i].treasure_type,
		&the_realm->monster[i].special_type,
		&the_realm->monster[i].flock_percent) != 8) {

		/* exit if we didn't read 8 items */
            sprintf(error_msg, "[0.0.0.0:%d] fscanf of %s failed on call number %d in Do_load_monster_file.\n", the_realm->serverPid, MONSTER_FILE, i);

            Do_log_error(error_msg);
            exit(MONSTER_FILE_ERROR);
        }
    }

	/* close the monster file */
    fclose(monster_file);

	/* all done here */
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_load_charstats_file(struct realm_t *the_realm)
/
/ FUNCTION: To load saved configuration information
/
/ AUTHOR: Brian Kelly, 5/7/99
/
/ ARGUMENTS:
/       struct realm_t *the_realm - pointer to the realm strcture
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

Do_load_charstats_file(struct realm_t *the_realm)
{
    FILE *charstats_file;
    char error_msg[SZ_ERROR_MESSAGE];
    int i;

	/* load charstats information */
    errno = 0;

	/* open the charstats file */
    if ((charstats_file = fopen(CHARSTATS_FILE, "r")) == NULL) {

        sprintf(error_msg,
		"[0.0.0.0:%d] fopen of %s failed Do_load_charstats_file: %s\n",
		the_realm->serverPid, CHARSTATS_FILE, strerror(errno));

        Do_log_error(error_msg);
        exit(CHARSTATS_FILE_ERROR);
    }

	/* read each line of the monster file */
    for (i = 0; i < NUM_CHARS; i++) {

	    /* read the character name which is on its own line */
        if (fgets(the_realm->charstats[i].class_name, SZ_CLASS_NAME,
		charstats_file) == NULL) {

            sprintf(error_msg, "[0.0.0.0:%d] fgets of %s failed on call number %d in Do_load_charstats_file.\n", the_realm->serverPid, CHARSTATS_FILE, i);

            Do_log_error(error_msg);
            exit(CHARSTATS_FILE_ERROR);
        }

             /* reEmove trailing blanks */
        Do_truncstring(the_realm->charstats[i].class_name);

	    /* read the stat line for each monster */
        if (fscanf(charstats_file, "%c %lf %lf %lf %lf %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
		&the_realm->charstats[i].short_class_name,
		&the_realm->charstats[i].max_brains,
		&the_realm->charstats[i].max_mana,
		&the_realm->charstats[i].weakness,
		&the_realm->charstats[i].goldtote,
		&the_realm->charstats[i].ring_duration,
		&the_realm->charstats[i].quickness.base,
		&the_realm->charstats[i].quickness.interval,
		&the_realm->charstats[i].quickness.increase,
		&the_realm->charstats[i].strength.base,
		&the_realm->charstats[i].strength.interval,
		&the_realm->charstats[i].strength.increase,
		&the_realm->charstats[i].mana.base,
		&the_realm->charstats[i].mana.interval,
		&the_realm->charstats[i].mana.increase,
		&the_realm->charstats[i].energy.base,
		&the_realm->charstats[i].energy.interval,
		&the_realm->charstats[i].energy.increase,
		&the_realm->charstats[i].brains.base,
		&the_realm->charstats[i].brains.interval,
		&the_realm->charstats[i].brains.increase,
		&the_realm->charstats[i].magiclvl.base,
		&the_realm->charstats[i].magiclvl.interval,
		&the_realm->charstats[i].magiclvl.increase) != 24) {

		/* exit if we didn't read 24 items */
            sprintf(error_msg, "[0.0.0.0:%d] fscanf of %s failed on call number %d in Do_load_charstats_file.\n", the_realm->serverPid, CHARSTATS_FILE, i);

            Do_log_error(error_msg);
            exit(CHARSTATS_FILE_ERROR);
        }
    }

	/* close the charstats file */
    fclose(charstats_file);

	/* all done here */
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_load_shopitems_file(struct realm_t *the_realm)
/
/ FUNCTION: To load saved configuration information
/
/ AUTHOR: Brian Kelly, 6/29/99
/
/ ARGUMENTS:
/       struct realm_t *the_realm - pointer to the realm strcture
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

Do_load_shopitems_file(struct realm_t *the_realm)
{
    FILE *shopitems_file;
    char error_msg[SZ_ERROR_MESSAGE];
    int i;

	/* load shopitems information */
    errno = 0;

	/* open the shopitems file */
    if ((shopitems_file = fopen(SHOPITEMS_FILE, "r")) == NULL) {

        sprintf(error_msg,
	     "[0.0.0.0:%d] fopen of %s failed in Do_load_shopitems_file: %s\n",
	     the_realm->serverPid, SHOPITEMS_FILE, strerror(errno));

        Do_log_error(error_msg);
        exit(SHOPITEMS_FILE_ERROR);
    }

	/* read each line of the monster file */
    for (i = 0; i < NUM_ITEMS; i++) {

	    /* read the character name which is on its own line */
        if (fgets(the_realm->shop_item[i].item, SZ_ITEMS, shopitems_file)
		== NULL) {

            sprintf(error_msg, "[0.0.0.0:%d] fgets of %s failed on call number %d in Do_load_shopitems_file.\n", the_realm->serverPid, SHOPITEMS_FILE, i);

            Do_log_error(error_msg);
            exit(SHOPITEMS_FILE_ERROR);
        }

             /* reEmove trailing blanks */
        Do_truncstring(the_realm->shop_item[i].item);

	    /* read the stat line for each item */
        if (fscanf(shopitems_file, "%lf\n", &the_realm->shop_item[i].cost)
		!= 1) {

		/* exit if we didn't read 24 items */
            sprintf(error_msg, "[0.0.0.0:%d] fscanf of %s failed on call number %d in Do_load_shopitems_file.\n", the_realm->serverPid, SHOPITEMS_FILE, i);

            Do_log_error(error_msg);
            exit(SHOPITEMS_FILE_ERROR);
        }
    }

	/* close the charstats file */
    fclose(shopitems_file);

	/* all done here */
    return;
}


/************************************************************************
/
/ FUNCTION NAME: void Do_close(struct server_t *server)
/
/ FUNCTION: To close the program's data files 
/
/ AUTHOR: Brian Kelly, 4/22/99
/
/ ARGUMENTS:
/       struct server_t *s - address of the server's main data strcture
/
/ RETURN VALUE: none
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

void Do_close(struct server_t *s)
{
    FILE *data_file;
    struct realm_object_t *object_ptr, *object_ptr2;
    struct realm_state_t *state_ptr;
    char error_msg[SZ_ERROR_MESSAGE];
    int error;

    state_ptr = (struct realm_state_t *) Do_malloc(SZ_REALM_STATE);

    state_ptr->kings_gold = s->realm.kings_gold;
    strcpy(state_ptr->king_name, s->realm.king_name);
    strcpy(state_ptr->valar_name, s->realm.valar_name);
    
	/* open the data file - for saving kings, valars, and corpses */
    errno = 0;
    if ((data_file = fopen(DATA_FILE, "w")) == NULL) {

        sprintf(error_msg, "[0.0.0.0:%d] fopen of %s failed in Do_close: %s\n",
		s->realm.serverPid, DATA_FILE, strerror(errno));

        Do_log_error(error_msg);
        exit(DATA_FILE_ERROR);
    }

    if (fwrite((void *) state_ptr, SZ_REALM_STATE, 1, data_file)
            != 1) {

        sprintf(error_msg,
		"[0.0.0.0:%d] fwrite of %s's realm state failed in Do_close: %d\n",
		s->realm.serverPid, DATA_FILE, ferror(data_file));

        Do_log_error(error_msg);
        fclose(data_file);
        exit(DATA_FILE_ERROR);
    }

	/* destroy the temporary realm state object */
    free((void *)state_ptr);

	/* first entry is king's gold as double float */
/*
    if (fwrite((void *)&s->realm.kings_gold, sizeof(double), 1, data_file)
            != 1) {

        sprintf(error_msg,
		"[0.0.0.0:%d] fwrite of %s failed in Do_close: %d\n",
		s->realm.serverPid, DATA_FILE, ferror(data_file));

        Do_log_error(error_msg);
        fclose(data_file);
        exit(DATA_FILE_ERROR);
    }
*/

	/* all remaining items are realm objects */
    object_ptr = s->realm.objects;

	/* while there is an item in the list of realm objects */
    while (object_ptr != NULL) {

	    /* write the object if it is a corpse, holy grail or treasure */
	if (object_ptr->type == HOLY_GRAIL || object_ptr->type == CORPSE ||
		object_ptr->type == TREASURE_TROVE) {	

		/* write the object to the data file */
            if (fwrite((void *)object_ptr, SZ_REALM_OBJECT, 1, data_file) !=
		    1) {

                sprintf(error_msg,
		       "[0.0.0.0:%d] fwrite of %s failed in Do_close(2): %d\n",
		       s->realm.serverPid, DATA_FILE, ferror(data_file));

                Do_log_error(error_msg);
                fclose(data_file);
                exit(DATA_FILE_ERROR);
	    }

	        /* if the object is a corpse object */
	    if (object_ptr->type == CORPSE) {

		    /* write the character record with it */
                if (fwrite((void *)object_ptr->arg1, SZ_PLAYER, 1, data_file)
			!= 1) {

                    sprintf(error_msg,
		       "[0.0.0.0:%d] fwrite of %s failed in Do_close(3): %d\n",
		       s->realm.serverPid, DATA_FILE, ferror(data_file));

                    Do_log_error(error_msg);
                    fclose(data_file);
                    exit(DATA_FILE_ERROR);
		}

		free((void *)object_ptr->arg1);
	    }
	}

	    /* point to the next realm object */
	object_ptr2 = object_ptr;
	object_ptr = object_ptr->next_object;
	free((void*)object_ptr2);
    }

	/* close the data file */
    fclose(data_file);

    return; /* no problems */
}


/************************************************************************
/
/ FUNCTION NAME: Do_hide_grail(struct *realm_t, int level)
/
/ FUNCTION: To load saved configuration information
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 8/17/99
/
/ ARGUMENTS:
/       struct realm_t *the_realm - pointer to the realm
/
/ RETURN VALUE: short error
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

int Do_hide_grail(the_realm, level)

    struct realm_t *the_realm;
    int level;

{
    double distance;
    char error_msg[SZ_ERROR_MESSAGE];

/* WARNING: Realm should already be locked when function is called */
/* or it shouldn't matter (startup) */

    struct realm_object_t *object_ptr;

    object_ptr = the_realm->objects;

    while (object_ptr != NULL) {

	if (object_ptr->type == HOLY_GRAIL) {

	    sprintf(error_msg,
	 "[0.0.0.0:%d] Do_hide_grail found a holy grail already in the realm.",
	 the_realm->serverPid);

	    Do_log_error(error_msg);
	    return;
	}

        object_ptr = object_ptr->next_object;
    }

    if (level < 3000) {
        level = 3000;
    }

	/* create a holy grail realm object */
    object_ptr = (struct realm_object_t *) Do_malloc(SZ_REALM_OBJECT);
    object_ptr->type = HOLY_GRAIL;

    /* place grail within point of no return */
    for (;;) {
        object_ptr->x = 0.0;
        object_ptr->y = 0.0;
        Do_move_close(&object_ptr->x, &object_ptr->y, 1000000.0);

	Do_distance(0.0, object_ptr->x, 0.0, object_ptr->y, &distance);

	if (distance >= level * 100) {
	    break;
	}
    }

    object_ptr->next_object = the_realm->objects;
    the_realm->objects = object_ptr;
}


/************************************************************************
/
/ FUNCTION NAME: Do_hide_trove(struct *realm_t)
/
/ FUNCTION: To load saved configuration information
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/9/01
/
/ ARGUMENTS:
/       struct realm_t *the_realm - pointer to the realm
/
/ RETURN VALUE: short error
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

int Do_hide_trove(struct realm_t *the_realm)
{
    double distance;
    char error_msg[SZ_ERROR_MESSAGE];
    struct realm_object_t *object_ptr;

/* WARNING: Realm should already be locked when function is called */
/* or it shouldn't matter (startup) */

    object_ptr = the_realm->objects;

    while(object_ptr != NULL) {

	if (object_ptr->type == TREASURE_TROVE) {

	    sprintf(error_msg,
	 "[0.0.0.0:%d] Do_hide_trove found a trove already in the realm.\n",
	 the_realm->serverPid);

	    Do_log_error(error_msg);
	    return;
	}

        object_ptr = object_ptr->next_object;
    }

	/* create a treasure_trove realm object */
    object_ptr = (struct realm_object_t *) Do_malloc(SZ_REALM_OBJECT);
    object_ptr->type = TREASURE_TROVE;

    for (;;) {

        object_ptr->x = 0.0;
        object_ptr->y = 0.0;
        Do_move_close(&object_ptr->x, &object_ptr->y, 1400.0);

	Do_distance(0.0, object_ptr->x, 0.0, object_ptr->y, &distance);

	if (distance >= 600) {
	    break;
	}
    }

    object_ptr->next_object = the_realm->objects;
    the_realm->objects = object_ptr;

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_backup_restore()
/
/ FUNCTION: To load saved configuration information
/
/ AUTHOR: Brian Kelly, 11/26/99
/
/ ARGUMENTS:
/       struct realm_t *the_realm - pointer to the realm
/
/ RETURN VALUE: short error
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

Do_backup_restore()
{
    FILE *backup_file;
    FILE *character_file;
    struct player_t the_player;
    char error_msg[SZ_ERROR_MESSAGE];
    int restoreCount = 0;

	/* open the backup file */
    errno = 0;
    if ((backup_file=fopen(BACKUP_FILE, "r")) == NULL) {

	   /* No backup file?  Cool! */
        return;
    }

        /* open the character file */
    errno = 0;
    if ((character_file=fopen(CHARACTER_FILE, "a")) == NULL) {

        sprintf(error_msg,
		"[0.0.0.0:?] fopen of %s failed in Do_backup_restore: %s\n",
		CHARACTER_FILE, strerror(errno));

        Do_log_error(error_msg);
	fclose(backup_file);
        return;
    }

       /* read each line of the backup file */
    errno = 0;
    while (fread((void *)&the_player, SZ_PLAYER, 1, backup_file) == 1) {

	    /* write the character to the charcter file */
        if (fwrite((void *)&the_player, SZ_PLAYER, 1, character_file) != 1) {

            sprintf(error_msg,
                  "[0.0.0.0:?] fwrite of %s failed in Do_backup_restore: %s\n",
                  CHARACTER_FILE, strerror(errno));

            Do_log_error(error_msg);
            fclose(backup_file);
            fclose(character_file);
            return;
        }
	else {

            sprintf(error_msg, "[0.0.0.0:?] %s restored\n", the_player.lcname);
            Do_log(SERVER_LOG, error_msg);
	    Do_log(GAME_LOG, error_msg);

	    ++restoreCount;
	}
    }

        /* delete the old backup file */
    remove(BACKUP_FILE);
    fclose(backup_file);
    fclose(character_file);

    if (restoreCount) {
        sprintf(error_msg, "Server restored %d characters.\n", restoreCount);
        Do_log(SERVER_LOG, error_msg);
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_get_next_tag()
/
/ FUNCTION: To load saved configuration information
/
/ AUTHOR: Brian Kelly, 01/18/01
/
/ ARGUMENTS:
/       struct realm_t *the_realm - pointer to the realm
/
/ RETURN VALUE: short error
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

int Do_get_next_tag()
{
    FILE *the_file;
    char error_msg[SZ_ERROR_MESSAGE];
    struct tag_t readTag;
    struct tagged_t readTagged;
    int lastTagNumber = 0;

    if ((the_file=fopen(TAG_FILE, "r")) != NULL) {

            /* run through the tag entries */
        while (fread((void *)&readTag, SZ_TAG, 1, the_file) == 1) {

	        /* see if this is the highest numbered tag */
	    if (readTag.number > lastTagNumber) {
	        lastTagNumber = readTag.number;
	    }
	}

	fclose(the_file);
    }

    if ((the_file=fopen(TAGGED_FILE, "r")) != NULL) {

            /* run through the tag entries */
        while (fread((void *)&readTagged, SZ_TAGGED, 1, the_file) == 1) {

	        /* see if this is the highest numbered tag */
	    if (readTagged.tagNumber > lastTagNumber) {
	        lastTagNumber = readTagged.tagNumber;
	    }
	}

	fclose(the_file);
    }

    return lastTagNumber + 1;
}
