/*
 * account.c       Routines to create and manipulate accounts
 */

#include "include.h"


/************************************************************************
/
/ FUNCTION NAME: Do_get_account(client_t *c)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 1/1/01
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

Do_get_account(struct client_t *c)
{
    int rc, answer;
    char error_msg[SZ_ERROR_MESSAGE];
    struct button_t buttons;

    Do_send_line(c, "\n");

    Do_send_line(c,
       "Do you need to sign up for an account, or are you ready to log in?\n");

    strcpy(buttons.button[0], "Sign Up\n");
    strcpy(buttons.button[1], "Log In\n");
    Do_clear_buttons(&buttons, 2);
    strcpy(buttons.button[3], "New Pass\n");
    strcpy(buttons.button[6], "Scoreboard\n");
    strcpy(buttons.button[7], "Quit\n");

    rc = Do_buttons(c, &answer, &buttons);
    Do_send_clear(c);

    if (rc != S_NORM) {
	answer = 7;
    }

	/* switch on the player's answer */
    switch (answer) {

	/* if the player needs a new account */
    case 0:

	    /* Let's make them an account */
	Do_account_signup(c);
	break;

	/* the player has an account */
    case 1:

	    /* The wish to log in */
	Do_account_login(c);
	break;

	/* the player has forgotten his account password */
    case 3:

	    /* go to account password reset */
	Do_reset_account_password(c);
	break;

	/* show the character the scoreboard */
    case 6:

	Do_scoreboard(c, 0);
	break;

	/* exit if requested */
    case 7:

	c->run_level = EXIT_THREAD;
	return;

	/* since it's a push button interface, any other answer is a hacker */
    default:

	sprintf(error_msg, "[%s] Returned non-option %d in Do_get_account.\n",
		c->connection_id, answer);

	Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_account_signup(client_t *c)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 1/1/01
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

Do_account_signup(struct client_t *c)
{
    int rc, answer, i;
    char string_buffer[SZ_LINE];
    struct account_t theAccount;
    struct button_t buttons;

    Do_send_clear(c);
    Do_send_line(c, "To get an account, you will need an account name, a password and an active e-mail address.  Are you ready to provide those things?\n");

    strcpy(buttons.button[0], "Yes\n");
    strcpy(buttons.button[1], "No\n");
    Do_clear_buttons(&buttons, 2);

    strcpy(buttons.button[7], "Quit\n");

    rc = Do_buttons(c, &answer, &buttons);
    Do_send_clear(c);

    if (rc != S_NORM) {
	answer = 7;
    }

	/* switch on the player's answer */
    switch (answer) {

	/* if the player is ready */
    case 0:

	    /* we continue on */
	break;

	/* back to main account menu */
    case 1:

	return;

	/* exit if requested */
    case 7:

	c->run_level = EXIT_THREAD;
	return;

	/* since it's a push button interface,
		any other answer is a hacker */
    default:

	sprintf(string_buffer,
		"[%s] Returned non-option %d in Do_account_signup.\n",
		c->connection_id, answer);

	Do_log_error(string_buffer);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

    Do_send_line(c, "First we'll need an account name.  This name will be visible to other players and this is important, _characters can not use account names_.  Please only use letters, numbers and underscores, and do not use the name of the character you want to play.\n");

    Do_more(c);
    Do_send_clear(c);

	/* Get an account name */
    for (;;) {

            /* name the new character */
	if (Do_string_dialog(c, &theAccount.name, SZ_NAME - 1,
		"What account name would you like?\n"))

	    return;

	    /* convert the name to lower case and store it */
        Do_lowercase(&theAccount.lcname, &theAccount.name);

	    /* Check the name for illegal characters or profanity */
        if (Do_approve_name(c, &theAccount.lcname, &theAccount.name, 
		&answer) != S_NORM) {

	    return;
	}

	    /* move on to the next stage */
	if (answer) {
	    break;
	}
    }

    Do_send_line(c, "Now think of a password.  You may use any characters you like.  Make it a good one so people can't guess it!\n");

    Do_more(c);
    Do_send_clear(c);

	/* Get a password for the account */
    if (!Do_new_password(c, theAccount.password, "account")) {

	Do_release_name(c, theAccount.lcname);
	return;
    }

    Do_send_line(c, "Finally, I need your e-mail address.  I will use it to send you a confirmation code as well as password resets you request, but nothing else.\n");

    Do_more(c);
    Do_send_clear(c);

	/* Get an account name */
    for (;;) {

            /* get an e-mail address */
	if (Do_string_dialog(c, theAccount.email, SZ_FROM - 1,
		"What is your e-mail address.\n")) {

	    Do_release_name(c, theAccount.lcname);
	    return;
	}

        if (Do_approve_email(c, &theAccount, &answer) != S_NORM) {

	    Do_release_name(c, theAccount.lcname);
	    return;
	}

	    /* continue on a positive answer */
	if (answer) {
	    break;
	}
    }

    Do_send_clear(c);
    Do_send_line(c, "Please re-check the information.\n");

    sprintf(string_buffer, "Account name: %s\n", theAccount.name);
    Do_send_line(c, string_buffer);
    sprintf(string_buffer, "E-mail address: %s\n", theAccount.email);
    Do_send_line(c, string_buffer);

    Do_send_line(c, "Do I have everything correct?\n");

    if (Do_yes_no(c, &answer) != S_NORM || answer == 1) {
	Do_release_name(c, theAccount.lcname);
	Do_release_email(c, theAccount.lcemail);
	return;
    }

    Do_send_clear(c);

	/* create a confirmation code */
    Do_create_password(&theAccount.confirmation);

	/* call the script to e-mail the confirmation code */
    sprintf(string_buffer, "%s %s %s %s\n", NEW_ACCOUNT_SCRIPT,
	    theAccount.name, theAccount.confirmation, theAccount.email);

	/* if the mail send fails */
    if (rc = system(string_buffer)) {

	sprintf(string_buffer,
    "[%s] New account e-mail failed with a code of %d in Do_account_signup.\n",
    c->connection_id, rc);

	Do_log_error(string_buffer);

	Do_send_line(c, "An error occurred while trying to send the confirmation e-mail.  Please contact the game administrator and ask about this message.\n");

	Do_more(c);
	Do_release_name(c, theAccount.lcname);
	Do_release_email(c, theAccount.lcemail);
	return;
    }

	/* start filling in the account strcture */
    strcpy(theAccount.parent_network, c->network);
    theAccount.date_created =
    theAccount.last_load = time(NULL);
    theAccount.last_reset =
    theAccount.bad_passwords =
    theAccount.login_count =
    theAccount.hackCount =
    theAccount.muteCount =
    theAccount.rejectCount = 0;

	/* save the account */
    Do_save_account(c, &theAccount);

    sprintf(string_buffer, "[%s] Created account %s with e-mail address %s.\n",
	    c->connection_id, theAccount.lcname, theAccount.email);

    Do_log(CONNECTION_LOG, string_buffer);

    Do_send_line(c, "Your account has been created and your confirmation code has been e-mailed to the address you provided.  To activate the account, log in with the information you provided, but have the confirmation code handy as you will need it.\n");

    Do_more(c);
    Do_send_clear(c);
    Do_send_line(c, "Welcome to Phantasia, and good luck in the realm.\n");
    Do_more(c);
    Do_send_clear(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_approve_email(struct client_t *c, struct account_t *theAccount, int *answer)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 1/1/01
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

int Do_approve_email(struct client_t *c, struct account_t *theAccount, int *answer)
{
    int len, i;
    bool at_flag, no_punct, good_address;
    char string_buffer[SZ_LINE];
    struct account_t readAccount;
    struct linked_list_t *list_ptr;
    FILE *account_file;

	/* see if the e-mail address looks okay */
    at_flag = FALSE;
    no_punct = TRUE;
    good_address = TRUE;
    len = strlen(theAccount->email);
    for (i = 0; i < len ; i++) {

	if (theAccount->email[i] == '@' || theAccount->email[i] ==
		'.' || theAccount->email[i] == '-') {

		/* if the previous was punctuation or the start */
	    if (no_punct) {
		good_address = FALSE;
		break;
	    }

	    no_punct = TRUE;

		/* if this is the last character */
	    if (i + 1 == len) {
		good_address = FALSE;
		break;
	    }

	    if (theAccount->email[i] == '@') {

		    /* if we've already seen an '@' */
		if (at_flag) {
		    good_address = FALSE;
		    break;
		}
		else {
		    at_flag = TRUE;
		}
	    }
	}
	else if (theAccount->email[i] == '_') {
	    /* do nothing - here because addresses may have underscores
	    at the beginning or end */
	}

	    /* approve only letters and numbers */
	else if (isalnum(theAccount->email[i])) {
	    no_punct = FALSE;
	}

	    /* reject anything other than letters, numbers, '.', '_' or '@' */
	else {
	    good_address = FALSE;
	    break;
	}
    }

	/* make sure a '@' was there someplace */
    if (!at_flag) {
	good_address = FALSE;
    }

	/* if the adress is bad, complain and return */
    if (!good_address) {

	sprintf(string_buffer, "\"%s\" does not appear to be a proper e-mail address.  Please try another.\n", theAccount->email);

	Do_send_line(c, string_buffer);
	Do_more(c);
	*answer = FALSE;
	return S_NORM;
    }

	/* it's a good address, convert it to lowercase */
    Do_lowercase(&theAccount->lcemail, theAccount->email);

	/* check the account file for a duplicate address */
    errno = 0;
    Do_lock_mutex(&c->realm->account_lock);
    if ((account_file=fopen(ACCOUNT_FILE, "r")) == NULL) {

        sprintf(string_buffer,
		"[%s] fopen of %s failed in Do_approve_email: %s\n",
                c->connection_id, ACCOUNT_FILE, strerror(errno));

        Do_log_error(string_buffer);
    }
    else {

            /* run through each entry and compare */
        while (fread((void *)&readAccount, SZ_ACCOUNT, 1, account_file) == 1) {

            if (strcmp(readAccount.lcemail, theAccount->lcemail)
		    == 0) {

                fclose(account_file);
                Do_unlock_mutex(&c->realm->account_lock);

	        sprintf(string_buffer, "The email address \"%s\" already has an account registered to it.  Please choose another.\n", theAccount->email);

		Do_send_line(c, string_buffer);
		Do_more(c);
		*answer = FALSE;
		return S_NORM;
	    }
	}

        fclose(account_file);
    }

	/* see if this address is in limbo */
    list_ptr = c->realm->email_limbo;

	/* run through all addresses in limbo */
    while (list_ptr != NULL) {

	if (strcmp(list_ptr->name, theAccount->lcemail) == 0) {
	
            Do_unlock_mutex(&c->realm->account_lock);

	    sprintf(string_buffer, "The email address \"%s\" is currently being registered by another player.  Please choose another.\n", theAccount->email);

	    Do_send_line(c, string_buffer);
	    Do_more(c);
	    *answer = FALSE;
	    return S_NORM;
	}

	list_ptr = list_ptr->next;
    }

	/* e-mail address checks out.  Put ours in limbo */
    list_ptr = (struct linked_list_t *) Do_malloc(SZ_LINKED_LIST);

    strcpy(list_ptr->name, theAccount->lcemail);
    list_ptr->next = c->realm->email_limbo;
    c->realm->email_limbo = list_ptr;
	
    Do_unlock_mutex(&c->realm->account_lock);

    *answer = TRUE;
    return S_NORM;
}


/************************************************************************
/
/ FUNCTION NAME: Do_release_email(struct client_t *c, char *address);
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 1/1/01
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

Do_release_email(struct client_t *c, char *address)
{
    char error_msg[SZ_ERROR_MESSAGE];
    struct linked_list_t *list_ptr, **list_ptr_ptr;

    Do_lock_mutex(&c->realm->account_lock);

	/* start at the first pointer */
    list_ptr_ptr = &c->realm->email_limbo;

	/* run through all addresses in limbo */
    while (*list_ptr_ptr != NULL) {

	if (strcmp((*list_ptr_ptr)->name, address) == 0) {

		/* remove this section of linked list */
	    list_ptr = *list_ptr_ptr;
	    *list_ptr_ptr = list_ptr->next;
	    free((void *)list_ptr);
	
            Do_unlock_mutex(&c->realm->account_lock);
	    return;
	}

	list_ptr_ptr = &((*list_ptr_ptr)->next);
    }

    Do_unlock_mutex(&c->realm->account_lock);

    sprintf(error_msg,
	    "[%s] Passed name not found in email limbo in Do_release_email.\n",
	    c->connection_id, address);

    Do_log_error(error_msg);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: int Do_save_account(struct client_t *c, struct account_t *theAccount)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 1/1/01
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

int Do_save_account(struct client_t *c, struct account_t *theAccount)
{
    char string_buffer[SZ_LINE];
    FILE *account_file;

	/* open the account file for writing */
    errno = 0;
    Do_lock_mutex(&c->realm->account_lock);
    if ((account_file=fopen(ACCOUNT_FILE, "a")) == NULL) {

        Do_unlock_mutex(&c->realm->account_lock);
        sprintf(string_buffer,
		"[%s] fopen of %s failed in Do_save_account: %s.\n",
                c->connection_id, ACCOUNT_FILE, strerror(errno));

        Do_log_error(string_buffer);
	return S_ERROR;
    }

	/* write the new account at the end */
    if (fwrite((void *)theAccount, SZ_ACCOUNT, 1, account_file) != 1) {

        fclose(account_file);
        Do_unlock_mutex(&c->realm->account_lock);
        sprintf(string_buffer,
		"[%s] fwrite of %s failed in Do_save_account: %s.\n",
                c->connection_id, ACCOUNT_FILE, strerror(errno));

        Do_log_error(string_buffer);
	return S_ERROR;
    }

	/* close the file */
    fclose(account_file);
    Do_unlock_mutex(&c->realm->account_lock);

	/* release the names in limbo */
    Do_release_name(c, theAccount->lcname);
    Do_release_email(c, theAccount->lcemail);

    return S_NORM;
}


/************************************************************************
/
/ FUNCTION NAME: Do_account_login(struct client_t *c)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 1/1/01
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

Do_account_login(struct client_t *c)
{
    char string_buffer[SZ_LINE], string_buffer2[SZ_LINE], accountName[SZ_NAME];
    struct account_t theAccount;
    struct account_mod_t theMod;
    int i;

        /* Inquire the account to load */
    for (;;) {

        if (Do_string_dialog(c, &accountName, SZ_NAME - 1,
		"What is the name of your account?\n")) {

	    return;
        }

        if (Do_look_account(c, &accountName, &theAccount)) {
	    break;
	}

	Do_send_line(c, "I can not find the account you named.  Please check the spelling and try again.\n");

	Do_more(c);
        Do_send_clear(c);
    }

	/* ask for a password */
    if (!Do_request_account_password(c, &theAccount.password, theAccount.name,
	    theAccount.lcname)) {

	    /* add a confirmation code reminder if applicable */
        if (theAccount.confirmation[0] != '\0') {

	    Do_send_line(c, "Perhaps you entered the confirmation code emailed to you instead of your chosen password.  If so, try to log in again and enter the PASSWORD YOU CHOSE when you created the account.\n");

	    Do_more(c);
            Do_send_clear(c);
        }

	return;
    }

	/* ask for confirmation code if one exists */
    if (theAccount.confirmation[0] != '\0') {

	for (i = 0; i < 2; i++) {

        	/* Inquire the account to load */

      	    if (Do_string_dialog(c, &string_buffer, SZ_LINE - 1,
		    "Please enter the confirmation code mailed to you.\n")) {

		return;
	    }

		/* good code */
	    if (strcmp(string_buffer, theAccount.confirmation) == 0) {

		Do_clear_account_mod(&theMod);
		theMod.confirm = TRUE;
		if (!Do_modify_account(c, &accountName, NULL, &theMod)) {	

		    Do_send_line(c, "The account change request encountered an error.  Please contact the game administrator about the problem.\n"); 

		    Do_more(c);
		    Do_send_clear(c);
		    return;
		}

	        Do_send_line(c, "Confirmation code confirmed!  You will never be asked for this code again, so feel free to forget all about it.\n");

                Do_more(c);
	        Do_send_clear(c);
		break;
	    }

	    if (i == 0) {

	        Do_send_line(c, "That is not the correct confirmation code.  Please double check the code before you hit \"ok\"\n");

                Do_more(c);
	        Do_send_clear(c);
	    }
	    else {

	        Do_send_clear(c);

	        Do_send_line(c, "That is not the confirmation code that was mailed to you.  Please wait until you have that e-mail before again attempting to log on with this account.\n");

                Do_more(c);
		return;
	    }
	}
    }

	/* mark us as logged in and get updated information */
    Do_clear_account_mod(&theMod);
    theMod.access = TRUE;
    if (!Do_modify_account(c, &accountName, &theAccount, &theMod)) {

	Do_send_line(c, "The program encountered an error while accessing your account information.  Please contact the game administrator about the problem.\n");
	Do_more(c);
	Do_send_clear(c);
    }

    sprintf(string_buffer,
	    "[%s] Logged on with account %s with e-mail address %s.\n",
	    c->connection_id, theAccount.lcname, theAccount.email);

    Do_log(CONNECTION_LOG, string_buffer);

    strcpy(c->account, theAccount.name);
    strcpy(c->lcaccount, theAccount.lcname);
    strcpy(c->email, theAccount.email);
    strcpy(c->parentNetwork, theAccount.parent_network);
    c->accountLoaded = TRUE;

	/* put the account name in the game description */
    Do_lock_mutex(&c->realm->realm_lock);
    strcpy(c->game->account, theAccount.name);
    Do_unlock_mutex(&c->realm->realm_lock);

    if (theAccount.bad_passwords) {

	sprintf(string_buffer, "There have been %d unsuccessful login(s) to this account since last successful login.\n", theAccount.bad_passwords);

	Do_send_line(c, string_buffer);
    }

    if (theAccount.login_count == 0) {

	Do_send_line(c, "Welcome new player.  To create a character, hit the \"New Char\" button just below.\n");
    }
    else {

	    /* convert the last time and remove the "\n". */
	ctime_r(&theAccount.last_load, string_buffer2);
	string_buffer2[strlen(string_buffer2) - 1] = '\0';

	sprintf(string_buffer, "Last login at %s from %s.\n", string_buffer2,
		theAccount.last_IP);

	Do_send_line(c, string_buffer);
    }

    c->run_level = CHAR_SELECTION;

	/* make sure this isn't a banned address */
    Do_check_tags(c);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: int Do_look_account(struct client_t *c, char *Accountname, struct account_t *theAccount)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 1/3/01
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

int Do_look_account(struct client_t *c, char *accountName, struct account_t *theAccount)
{
    char string_buffer[SZ_LINE];
    FILE *account_file;

	/* open the account file */
    errno = 0;
    Do_lock_mutex(&c->realm->account_lock);
    if ((account_file=fopen(ACCOUNT_FILE, "r")) == NULL) {

        Do_unlock_mutex(&c->realm->account_lock);
        sprintf(string_buffer,
		"[%s] fopen of %s failed in Do_look_account: %s.\n",
                c->connection_id, ACCOUNT_FILE, strerror(errno));

        Do_log_error(string_buffer);
	return FALSE;
    }

	/* convert the name to lower case and store it */
    Do_lowercase(accountName, accountName);

	/* run through the account entries */
    while (fread((void *)theAccount, SZ_ACCOUNT, 1, account_file) == 1) {

	    /* if this is the account */
        if (strcmp(theAccount->lcname, accountName) == 0) {

		/* return with the good news */
            fclose(account_file);
            Do_unlock_mutex(&c->realm->account_lock);

/* There's an overflow with the mute count somewhere -- EH & BK - 1/6/02 */
if (theAccount->muteCount > 6 || theAccount->muteCount < 0) {
  theAccount->muteCount = 0;
}


	    return TRUE;
	}
    }

	/* close down and return a negative */
    fclose(account_file);
    Do_unlock_mutex(&c->realm->account_lock);
    return FALSE;
}


/************************************************************************
/
/ FUNCTION NAME: Do_clear_account_mod(struct account_mod_t *theMod)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 1/3/01
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

Do_clear_account_mod(struct account_mod_t *theMod)
{
    theMod->newPassword = FALSE;
    theMod->passwordReset = FALSE;
    theMod->newEmail = FALSE;
    theMod->confirm = FALSE;
    theMod->access = FALSE;
    theMod->badPassword = FALSE;
    theMod->hack = FALSE;
    theMod->mute = FALSE;

    return;
}


/************************************************************************
/
/ FUNCTION NAME: int Do_modify_account(struct client_t *c, char *Accountname, struct account_t *theAccount, struct account_mod_t *theMod)
/
/ FUNCTION: return a char specifying player type
/
/ AUTHOR: Brian Kelly, 1/3/01
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

int Do_modify_account(struct client_t *c, char *accountName, struct account_t *theAccount, struct account_mod_t *theMod)
{
    char string_buffer[SZ_LINE];
    struct account_t readAccount;
    FILE *account_file, *temp_file;
    time_t timeNow;
    bool found_flag;

	/* open the account file */
    errno = 0;
    Do_lock_mutex(&c->realm->account_lock);
    if ((account_file=fopen(ACCOUNT_FILE, "r")) == NULL) {

        Do_unlock_mutex(&c->realm->account_lock);
        sprintf(string_buffer,
		"[%s] fopen of %s failed in Do_modify_account: %s.\n",
                c->connection_id, ACCOUNT_FILE, strerror(errno));

        Do_log_error(string_buffer);
	return FALSE;
    }

	/* open a temp file to transfer records to */
    errno = 0;
    if ((temp_file=fopen(TEMP_ACCOUNT_FILE, "w")) == NULL) {

        fclose(account_file);
        Do_unlock_mutex(&c->realm->account_lock);
        sprintf(string_buffer,
		"[%s] fopen of %s failed in Do_modify_account(2): %s.\n",
                c->connection_id, TEMP_ACCOUNT_FILE, strerror(errno));

        Do_log_error(string_buffer);
	return FALSE;
    }

	/* get the time now */
    timeNow = time(NULL);
    found_flag = FALSE;

	/* run through the account entries */
    while (fread((void *)&readAccount, SZ_ACCOUNT, 1, account_file) == 1) {

	    /* if this is the account */
        if (strcmp(readAccount.lcname, accountName) == 0) {

	    found_flag = TRUE;

		/* copy the record to the passed pointer */
	    if (theAccount != NULL) {
	        memcpy(theAccount, &readAccount, SZ_ACCOUNT);
	    }

		/* make the modifications */
	    if (theMod->newPassword) {
		memcpy(&readAccount.password, &theMod->password, SZ_PASSWORD);
	    }

	    if (theMod->passwordReset) {
		readAccount.last_reset = timeNow;
	    }

	    if (theMod->newEmail) {
		strncpy(readAccount.email, theMod->email, SZ_FROM);

		strncpy(readAccount.confirmation, theMod->confirmation,
			SZ_PASSWORD);
	    }

	    if (theMod->confirm) {
		readAccount.confirmation[0] = '\0';
	    }

	    if (theMod->access) {
		strncpy(readAccount.last_IP, c->IP, SZ_FROM);
		strncpy(readAccount.last_network, c->network, SZ_FROM);
		readAccount.last_load = timeNow;
		++readAccount.login_count;
		readAccount.bad_passwords = 0;
	    }

	    if (theMod->badPassword) {
		++readAccount.bad_passwords;
	    }

	    if (theMod->hack) {
		readAccount.hackCount = theMod->hackCount;
		readAccount.lastHack = timeNow;
	    }

	    if (theMod->mute) {
		readAccount.muteCount = theMod->muteCount;
		readAccount.lastMute = timeNow;
	    }
	}
	else {

		/* see if this account is still valid */
	    if (timeNow - readAccount.last_load > ACCOUNT_KEEP_TIME) {

		    /* log the deletion of the account */
		sprintf(string_buffer, "[%s] Deleted the account %s.\n",
			c->connection_id, readAccount.lcname);

		Do_log(CONNECTION_LOG, string_buffer);

		    /* don't save this account */
		continue;
	    }
	}

	    /* write the account to the temp file */
        if (fwrite((void *)&readAccount, SZ_ACCOUNT, 1, temp_file) != 1) {

            fclose(account_file);
            fclose(temp_file);
	    remove(TEMP_ACCOUNT_FILE);
            Do_unlock_mutex(&c->realm->account_lock);

            sprintf(string_buffer,
		    "[%s] fwrite of %s failed in Do_modify_account: %s.\n",
                    c->connection_id, TEMP_ACCOUNT_FILE, strerror(errno));

            Do_log_error(string_buffer);
	    return FALSE;
	}
    }

	/* close the two files */
    fclose(account_file);
    fclose(temp_file);

        /* delete the old character record */
    remove(ACCOUNT_FILE);

        /* replace it with the temporary file */
    rename(TEMP_ACCOUNT_FILE, ACCOUNT_FILE);

    Do_unlock_mutex(&c->realm->account_lock);

    if (found_flag) {
	return TRUE;
    }

    sprintf(string_buffer,
	 "[%s] Couldn't find account \"%s\" to modify in Do_modify_account.\n",
         c->connection_id, accountName);

    Do_log_error(string_buffer);
    return FALSE;
}


/*************************************************************************
/
/ FUNCTION NAME: Do_account_options(struct client_t *c)
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

Do_account_options(struct client_t *c)
{
    char error_msg[SZ_ERROR_MESSAGE], string_buffer[SZ_LINE];
    struct account_t theAccount;
    struct button_t buttons;
    int rc;
    long answer;

    if (!Do_look_account(c, c->lcaccount, &theAccount)) {

        sprintf(error_msg,
		"[%s] Unable to load own account %s in Do_account_options.\n",
        	c->connection_id, c->lcaccount);

        Do_log_error(error_msg);
    }

    sprintf(string_buffer, "Connecting From: %s\n", c->IP);
    Do_send_line(c, string_buffer);
    sprintf(string_buffer, "Account Name: %s\n", theAccount.name);
    Do_send_line(c, string_buffer);
    sprintf(string_buffer, "Account E-Mail: %s\n", theAccount.email);
    Do_send_line(c, string_buffer);
    sprintf(string_buffer, "Created From: %s\n", theAccount.parent_network);
    Do_send_line(c, string_buffer);
    ctime_r(&theAccount.date_created, error_msg);
    sprintf(string_buffer, "Created On: %s", error_msg);
    Do_send_line(c, string_buffer);
    sprintf(string_buffer, "Successful Logins: %d\n", theAccount.login_count);
    Do_send_line(c, string_buffer);

    strcpy(buttons.button[0], "Change Pass\n");
    Do_clear_buttons(&buttons, 1);
    strcpy(buttons.button[7], "Go Back\n");

/* I was thinking od adding a "change email" option, but I then realized that
all of a players characters could become compromized if just the account was */

    rc = Do_buttons(c, &answer, &buttons);
    Do_send_clear(c);

    if (rc != S_NORM) {
	answer = 7;
    }

	/* switch on the player's answer */
    switch (answer) {

	/* Change the account password */
    case 0:
	Do_change_account_password(c);
	break;

	/* Return to previous state */
    case 7:
	return;

    default:

        sprintf(error_msg,
		"[%s] Returned non-option %ld in Do_account_options.\n",
                c->connection_id, answer);

        Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_change_account_password(struct client_t *c)
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

Do_change_account_password(struct client_t *c)
{

    char error_msg[SZ_ERROR_MESSAGE], string_buffer[SZ_LINE];
    unsigned char newPassword[SZ_PASSWORD];
    struct button_t theButtons;
    struct account_t theAccount;
    struct account_mod_t theMod;
    int rc;
    long answer;

    Do_send_line(c, "This option allows you to change the password of your account.  Do you wish to continue?\n");

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
	       "[%s] Returned non-option %ld in Do_change_account_password.\n",
               c->connection_id, answer);

        Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

	/* load the account information */
    if (!Do_look_account(c, c->account, &theAccount)) {

        sprintf(error_msg,
	 "[%s] Unable to load own account %s in Do_change_account_password.\n",
         c->connection_id, c->lcaccount);

        Do_log_error(error_msg);

        Do_send_line(c, "I can not load your account information.  Please contact the game administrator about the problem.\n");

        Do_more(c);
        Do_send_clear(c);
	return;
    }

	/* confirm the player knows his current password */
    if (!Do_request_account_password(c, theAccount.password, c->account,
	    c->lcaccount)) {

        return;
    }

    Do_clear_account_mod(&theMod);
    theMod.newPassword = TRUE;

	/* Get the new password from the player */
    if (!Do_new_password(c, &theMod.password, "account")) {
        return;
    }

    if (!Do_modify_account(c, c->lcaccount, NULL, &theMod)) {

	    /* if false returns, the modify did not occur */
	Do_send_line(c, "The account change request encountered an error.  Please write down the timestamp at the end and contact the game administrator about the problem.  The password has NOT been changed.\n"); 

	Do_more(c);
	Do_send_clear(c);
	return;
    }

    sprintf(string_buffer, "[%s] Changed the password to account %s.\n",
	    c->connection_id, c->lcaccount);

    Do_log(CONNECTION_LOG, string_buffer);

    Do_send_line(c,
	    "The password to your account has been successfully changed.\n");

    Do_more(c);
    Do_send_clear(c);
    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_reset_account_password(struct client_t *c)
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

Do_reset_account_password(struct client_t *c)
{

    char error_msg[SZ_ERROR_MESSAGE], accountName[SZ_NAME];
    char string_buffer[SZ_LINE], lcAccountName[SZ_NAME];
    char newPassword[SZ_PASSWORD];
    struct button_t theButtons;
    struct account_t theAccount;
    struct account_mod_t theMod;
    int rc;
    long answer;
    MD5_CTX context;
    unsigned int len;

    Do_send_line(c, "With this option a random password will be created for your account and e-mailed to the account address.  You are required to be logged in at the same location at which the account was created.  This is the only way to gain access to an account whose password you've forgotten.\n");

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
		"[%s] Returned non-option %ld in Do_reset_account_password.\n",
                c->connection_id, answer);

        Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
	return;
    }

    for (;;) {

	    /* prompt for the account name */
        if (Do_string_dialog(c, accountName, SZ_NAME - 1,
	     "What is the name of the account that needs a new password?\n")) {

	    return;
        }

	    /* load the account information */
        Do_lowercase(&lcAccountName, &accountName);
	if (Do_look_account(c, lcAccountName, &theAccount)) {
	    break;
	}

	sprintf(string_buffer, "I can not find an account named \"%s\".  Please check the spelling and try again.\n", accountName);

        Do_send_line(c, string_buffer);
        Do_more(c);
	Do_send_clear(c);
    }

	/* compare network addresses */
    if (strcmp(theAccount.parent_network, c->network)) {

	Do_send_line(c, "You are in a different internet domain than the one used to create this account.  The account password reset can not continue.\n");

	Do_more(c);
	Do_send_clear(c);
	return;
    }

	/* only confirmed accounts can be reset */
    if (theAccount.confirmation[0] != '\0') {

	Do_send_line(c, "Password resets can not be done on accounts with a e-mail address that has not been confirmed.\n");

	Do_more(c);
	Do_send_clear(c);
	return;
    }

	/* allow only one reset every 24 hours */
    if (time(NULL) - theAccount.last_reset < 86400) {

	Do_send_line(c, "The password for this account was reset in the last 24 hours.  You must wait before resetting it again.\n");

	Do_more(c);
	Do_send_clear(c);
	return;
    }

    sprintf(string_buffer, "Are you certain you wish to reset the password for the account \"%s\"?\n", theAccount.name);

    Do_send_line(c, string_buffer);

    if (Do_yes_no(c, &answer) != S_NORM || answer == 1) {

	Do_send_clear(c);
	sprintf(string_buffer, "Password reset aborted.  The password to the account \"%s\" has NOT been changed.\n", theAccount.name);

	Do_send_line(c, string_buffer);
	Do_more(c);
	Do_send_clear(c);
	return;
    }

    Do_send_clear(c);

	/* create a new password */
    Do_create_password(&newPassword);

        /* call the script to e-mail this new password */
    sprintf(string_buffer, "%s %s %s %s\n", ACCOUNT_PASSWORD_RESET_SCRIPT,
            theAccount.name, newPassword, theAccount.email);

        /* if the mail send fails */
    if (rc = system(string_buffer)) {

        sprintf(string_buffer, "[%s] Account password reset e-mail failed with a code of %d in Do_reset_account_password.", c->connection_id, rc);

        Do_log_error(string_buffer);

        Do_send_line(c, "An error occured while trying to send e-mail containing the new password.  The account password has NOT been changed.  Please write down the timestamp at the end and contact the game administrator about this problem.\n");

        Do_more(c);
	Do_send_clear(c);
        return;
    }

	/* prepare the modification */
    Do_clear_account_mod(&theMod);
    theMod.newPassword = TRUE;
    theMod.passwordReset = TRUE;

        /* run the password through a MD5 hash */
    len = strlen(newPassword);
    MD5Init(&context);
    MD5Update(&context, newPassword, len);

	/* put the password hash into the change struct */
    MD5Final(theMod.password, &context);

	/* put the modification in place */
    if (!Do_modify_account(c, lcAccountName, NULL, &theMod)) {	

	Do_send_line(c, "The account change request encountered an error.  The password has NOT been changed and the e-mail sent to you contains incorrect information.  Please contact the game administrator about the problem.\n"); 

	Do_more(c);
	Do_send_clear(c);
	return;
    }

    sprintf(string_buffer, "[%s] Reset the password to account %s.\n",
	    c->connection_id, theAccount.lcname);

    Do_log(CONNECTION_LOG, string_buffer);

    sprintf(string_buffer, "The password to the account \"%s\" has been successfully changed.  An e-mail with the new password is in transit.\n", theAccount.name);

    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);
}


