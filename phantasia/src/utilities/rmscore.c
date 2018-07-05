/*
 * main.c - conversion of the old character file to the new one
 */

#include <sys/time.h>
#include <stdio.h>
#include <errno.h>

typedef char bool;

/* string sizes */
#define SZ_NAME		33	/* player name (incl. trailing null) */
#define SZ_FROM		41	/* ip or dns login (incl. null) */
#define SZ_CLASS_NAME           13      /* longest class name */
#define SZ_HOW_DIED             78

struct scoreboard_t                     /* scoreboard entry */
{
    double      level;                  /* level of player */
    char        class[SZ_CLASS_NAME];   /* character type of player */
    char        name[SZ_NAME];          /* name of player */
    char        from[SZ_FROM];          /* ip or DNS of player */
    char        how_died[SZ_HOW_DIED];  /* description of player's fate */
    time_t      time;                   /* time of death */
};

#define SZ_SCOREBOARD sizeof(struct scoreboard_t)

main(argc, argv)
int     argc;
char    *argv[];
{
    struct scoreboard_t entry;
    FILE *scoreboard_file;
    FILE *new_scoreboard_file;
    char string_buffer[80];
   int firstFlag;

    if (argc != 3) {

	printf("usage: a.out score_file char_name\n");
	printf("This program will instances of char_name from score_file.\n");
	exit(10);
    }

    errno = 0;
    if ((scoreboard_file=fopen(argv[1], "r")) == NULL) {

	printf("fopen of %s failed: %s\n", argv[1], strerror(errno));
	exit(1);
    }

    errno = 0;
    if ((new_scoreboard_file=fopen("scoreboard.new", "w")) == NULL) {

	printf("fopen of scoreboard.new failed: %s\n", strerror(errno));
	exit(1);
    }

    firstFlag = 1;

	/* start reading the characters */
    while (fread((void *)&entry, SZ_SCOREBOARD, 1, scoreboard_file) == 1) {

	if (firstFlag && !strcmp(entry.name, argv[2])) {
            strcpy(entry.class, "Halfling");
	    firstFlag = 0;

/*
	    fwrite((void *)&entry, SZ_SCOREBOARD, 1, new_scoreboard_file);
*/
	}
fwrite((void *)&entry, SZ_SCOREBOARD, 1, new_scoreboard_file);
/*
	if (entry.level < 3000) {

	    fwrite((void *)&entry, SZ_SCOREBOARD, 1, new_scoreboard_file);
	}
*/
    }
}
