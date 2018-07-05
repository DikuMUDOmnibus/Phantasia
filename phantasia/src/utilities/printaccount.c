/*
 * main.c - conversion of the old character file to the new one
 */

#include <sys/time.h>
#include <stdio.h>
#include <errno.h>

typedef char bool;

/* string sizes */
#define SZ_NAME		33	/* player name (incl. trailing null) */
#define SZ_PASSWORD		16	/* password size (incl. null) */
#define SZ_FROM		81	/* ip or dns login (incl. null) */
#define SZ_AREA			24

struct account_t        /* structure to store account information */
{
        /* main information */
    char        name[SZ_NAME];          /* account name */
    char        lcname[SZ_NAME];        /* name in lowercase */
    unsigned char password[16];         /* password hash */
    time_t      last_reset;             /* time of last password reset */
    char        email[SZ_FROM]; /* creator e-mail */
    char        lcemail[SZ_FROM]; /* e-mail in lowercase */
    char        confirmation[SZ_PASSWORD]; /* first time confirmation */

        /* creation information */
    char        parent_IP[SZ_FROM];     /* created from this address */
    char        parent_network[SZ_FROM];     /* created from this address */
    time_t      date_created;          /* created at this time */

        /* previous user information */
    char        last_IP[SZ_FROM];       /* last IP or DNS address */
    char        last_network[SZ_FROM];  /* last IP or DNS address */
    time_t      last_load;              /* time last accessed */
    int         login_count;            /* number of times account used */

        /* hack foilers */
    int         bad_passwords;          /* unsuccessful load attempts */
    int         hackCount;              /* number of alleged hacks */
    int         rejectCount;            /* times given a reject tag */
    time_t      lastHack;               /* time of the last hack */
    int         muteCount;              /* times given a reject tag */
    time_t      lastMute;               /* time of the last hack */
};


#define SZ_ACCOUNT sizeof(struct account_t)

main(argc, argv)
int     argc;
char    *argv[];
{
    struct account_t account;
    FILE *account_file;
    char string_buffer[80];
    int i;

    if (argc != 2) {

	printf("usage: a.out filename\n");
	printf("This program will print out names, passwords and dates");
	printf("from the passed account file.\n");
	exit(10);
    }

    errno = 0;
    if ((account_file=fopen(argv[1], "r")) == NULL) {

	printf("fopen of %s failed: %s\n", argv[0], strerror(errno));
	exit(1);
    }

	/* start reading the characters */
    while (fread((void *)&account, SZ_ACCOUNT, 1, account_file) == 1) {

	printf("Name: %s\n", account.name);
	printf("Lowercase Name: %s\n", account.lcname);
	printf("Password: ");

	for (i = 0; i < 16; i++) {
	    printf("%x", account.password[i]);
	}
	printf("\n");

	printf("Last Reset: %s", ctime(&account.last_reset));
	printf("Email: %s\n", account.email);
	printf("Lowercase Email: %s\n", account.lcemail);
	printf("Confirmation: %s\n", account.confirmation);
	printf("Parent IP Address: %s\n", account.parent_IP);
	printf("Parent Network: %s\n", account.parent_network);
	printf("Date Created: %s", ctime(&account.date_created));
	printf("Last IP Address: %s\n", account.last_IP);
	printf("Last Network: %s\n", account.last_network);
	printf("Last Loaded: %s", ctime(&account.last_load));
	printf("Login Count: %d\n", account.login_count);
	printf("Bad Password Count: %d\n", account.bad_passwords);
	printf("Hack Attempts: %d\n", account.hackCount);
	printf("Reject Count: %d\n", account.rejectCount);
	printf("Last Hack: %s", ctime(&account.lastHack));
	printf("Mute Count: %d\n", account.muteCount);
	printf("Last Mute: %s\n", ctime(&account.lastMute));

    }
}
