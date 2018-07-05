/*
 * main.c - conversion of the old character file to the new one
 */

#include <sys/time.h>
#include <stdio.h>
#include <errno.h>

typedef char bool;

/* string sizes */
#define SZ_NAME		33	/* player name (incl. trailing null) */
#define SZ_PASSWORD		17	/* password size (incl. null) */
#define SZ_FROM		81	/* ip or dns login (incl. null) */
#define SZ_AREA			24

struct newPlayer_t		/* player attributes */
{
        /* player information */
    char        name[SZ_NAME];          /* player name */
    char        lcname[SZ_NAME];        /* player name in lowercase */
    unsigned char password[16];         /* password hash */

        /* player location */
    double      x;                      /* x coord */
    double      y;                      /* y coord */
    char        area[SZ_AREA];          /* name of the area player is in */
    double      circle;                 /* current circle player is in */
    bool        beyond;                 /* is the character past no return */
    short       location;               /* codes for special all locations */

        /* player stats */
    double      experience;             /* experience */
    double      level;                  /* level */

    double      strength;               /* strength */
    double      max_strength;           /* maximum strength */
    double      energy;                 /* energy */
    double      max_energy;             /* maximum energy */
    float       quickness;              /* quickness */
    float       max_quickness;          /* quickness */
    double      mana;                   /* mana */
    double      brains;                 /* brains */
    double      magiclvl;               /* magic level */
    float       poison;                 /* poison */
    float       sin;                    /* sin */
    bool        gender;                 /* player is male/female */
    short       lives;                  /* multiple lives for council, valar */
    int         age;                    /* age of player */
    int         degenerated;            /* age/3000 last degenerated */

        /* player status */
    short       type;                   /* character type */
    short       special_type;           /* special character type */
    bool        cloaked;                /* is character cloaked */
    bool        blind;                  /* blindness */
    short       shield_nf;              /* does player get shield next battle */
    short       invisible_nf;           /* will the player be invisible */
    short       strong_nf;              /* does player get a strength bonus */

        /* player currency */
    double      gold;                   /* gold */
    double      gems;                   /* gems */

        /* player equipment */
    double      sword;                  /* sword */
    double      shield;                 /* shield */
    float       quicksilver;            /* quicksilver */
    int         holywater;              /* holy water */
    int         amulets;                /* amulets */
    int         charms;                 /* charms */
    short       crowns;                 /* crowns */
    bool        virgin;                 /* virgin */
    bool        palantir;               /* palantir */
    bool        blessing;               /* blessing */

    short       ring_type;              /* type of ring */
    int         ring_duration;          /* duration of ring */

        /* creation information */
    char        parent_account[SZ_NAME];        /* created by this account */
    char        parent_network[SZ_FROM];        /* created from this address */
    time_t      date_created;           /* created at this time */
    bool        faithful;               /* may other accounts load? */

        /* current or previous user information */
    char        last_IP[SZ_FROM];       /* last IP or DNS address */
    char        last_account[SZ_NAME];  /* last account accessed */
    time_t      last_load;              /* time last accessed */
    time_t      last_reset;             /* time password was last reset */
    int         load_count;             /* times character has been loaded */
    time_t      time_played;            /* seconds character has been played */

        /* hack foilers */
    int         bad_passwords;          /* unsuccessful load attempts */
    int         muteCount;              /* times caught spamming */
    time_t      lastMute;               /* last instance spammed */

        /* purgatory options */
    bool        purgatoryFlag;
    short       monsterNumber;
};

#define SZ_NEW_PLAYER sizeof(struct newPlayer_t)

main(argc, argv)
int     argc;
char    *argv[];
{
    struct newPlayer_t newPlayer;
    FILE *character_file;
    char string_buffer[80];
    int i;

    if (argc != 2) {

	printf("usage: a.out filename\n");
	printf("This program will print out names, passwords and dates");
	printf("from the passed character file.\n");
	exit(10);
    }

    errno = 0;
    if ((character_file=fopen(argv[1], "r")) == NULL) {

	printf("fopen of %s failed: %s\n", argv[0], strerror(errno));
	exit(1);
    }

	/* start reading the characters */
    while (fread((void *)&newPlayer, SZ_NEW_PLAYER, 1, character_file) == 1) {
/*
	printf("Name: %s, Password %s, X: %.0lf Y: %.0lf\n",
		newPlayer.name, newPlayer.password, newPlayer.x,
		newPlayer.y);
	printf("Name: %s, From: %s, Password %s, Last Used: %s\n",
		newPlayer.name, newPlayer.from,
		newPlayer.password, ctime(&newPlayer.lastUsed));
*/

	printf("Name: %s\n", newPlayer.name);
	printf("Lowercase Name: %s\n", newPlayer.lcname);
	printf("Password: ");

	for (i = 0; i < 16; i++) {
	    printf("%x", newPlayer.password[i]);
	}
	printf("\n");

	printf("Last Used: %s", ctime(&newPlayer.last_load));
	printf("Location: ( %lf, %lf )\n", newPlayer.x, newPlayer.y);
	printf("Experience: %lf\n", newPlayer.experience);
	printf("Strength: %lf\n", newPlayer.strength);
	printf("Maximum Strength: %lf\n", newPlayer.max_strength);
	printf("Energy: %lf\n", newPlayer.energy);
	printf("Maximum Energy: %lf\n", newPlayer.max_energy);
	printf("Quickness: %lf\n", newPlayer.quickness);
	printf("Maximum Quickness: %lf\n", newPlayer.max_quickness);
	printf("Mana: %lf\n", newPlayer.mana);
	printf("Brains: %lf\n", newPlayer.brains);
	printf("Level: %lf\n", newPlayer.level);
	printf("Magic Level: %lf\n", newPlayer.magiclvl);
	printf("Poison: %f\n", newPlayer.poison);
	printf("Sin: %f\n", newPlayer.sin);
	printf("Lives: %hd\n", newPlayer.lives);
	printf("Age: %d\n", newPlayer.age);
	printf("Degenerated: %d\n", newPlayer.degenerated);
	printf("Blind: %hd\n", newPlayer.blind);
	printf("Gender: %hd\n", newPlayer.gender);
	printf("Type: %hd\n", newPlayer.type);
	printf("Special Type: %hd\n", newPlayer.special_type);
	printf("Shield Next Fight: %hd\n", newPlayer.shield_nf);
	printf("Invisible Next Fight: %hd\n", newPlayer.invisible_nf);
	printf("Strong Next Fight: %hd\n", newPlayer.strong_nf);
	printf("Gold: %lf\n", newPlayer.gold);
	printf("Gems: %lf\n", newPlayer.gems);
	printf("Sword: %lf\n", newPlayer.sword);
	printf("Shield: %lf\n", newPlayer.shield);
	printf("Quicksilver: %f\n", newPlayer.quicksilver);
	printf("Holy Water: %hd\n", newPlayer.holywater);
	printf("Amulets: %hd\n", newPlayer.amulets);
	printf("Charms: %hd\n", newPlayer.charms);
	printf("Crowns: %hd\n", newPlayer.crowns);
	printf("Virgin: %hd\n", newPlayer.virgin);
	printf("Palantir: %hd\n", newPlayer.palantir);
	printf("Blessing: %hd\n", newPlayer.blessing);
	printf("Ring Type: %hd\n", newPlayer.ring_type);
	printf("Ring Duration: %hd\n", newPlayer.ring_duration);
	printf("Parent Account: %s\n", newPlayer.parent_account);
	printf("Parent Network: %s\n", newPlayer.parent_network);
	printf("Creation Date: %s", ctime(&newPlayer.date_created));
	printf("Faithful: %hd\n", newPlayer.faithful);
	printf("Previous Account: %s\n", newPlayer.last_account);
	printf("Previous IP: %s\n", newPlayer.last_IP);
	printf("Previous Load: %s", ctime(&newPlayer.last_load));
	printf("Bad Passwords: %d\n", newPlayer.bad_passwords);
	printf("Load Count: %d\n", newPlayer.load_count);
	printf("Time Played: %d\n\n", newPlayer.time_played);

    }
}
