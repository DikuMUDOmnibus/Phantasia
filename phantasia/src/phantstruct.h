/*
 * phantstruct.h - structure definitions for Phantasia
 */

extern player_t;
extern opponent_t;
extern battle_t;
extern it_combat_t;
extern gladiator_t;
extern account_t;
extern server_t;
extern client_t;
extern game_t;
extern player_desc_t;
extern event_t;
extern realm_t;
extern realm_object_t;
extern scoreboard_t;
extern monster_t;
extern charstats_t;
extern shop_item_t;
extern connection_t;

struct player_t		/* player attributes */
{
	/* player information */
    char	name[SZ_NAME]; 		/* player name */
    char	lcname[SZ_NAME];	/* player name in lowercase */
    unsigned char password[16];		/* password hash */

	/* player location */
    double	x;			/* x coord */
    double	y;			/* y coord */
    char	area[SZ_AREA];		/* name of the area player is in */
    double	circle;			/* current circle player is in */
    bool	beyond;			/* is the character past no return */
    short	location;		/* codes for special all locations */

	/* player stats */
    double	experience;		/* experience */
    double	level;			/* level */

    double	strength;		/* strength */
    double	max_strength;		/* maximum strength */
    double	energy;			/* energy */
    double	max_energy;		/* maximum energy */
    float	quickness;		/* quickness */
    float	max_quickness;		/* quickness */
    double	mana;			/* mana */
    double	brains;			/* brains */
    double	magiclvl;		/* magic level */
    float	poison;			/* poison */
    float	sin;			/* sin */
    bool        gender;			/* player is male/female */
    short	lives;			/* multiple lives for council, valar */
    int		age;			/* age of player */
    int		degenerated;		/* age/3000 last degenerated */

	/* player status */
    short	type;			/* character type */
    short	special_type;		/* special character type */
    bool	cloaked;		/* is character cloaked */
    bool        blind;			/* blindness */
    short	shield_nf;		/* does player get shield next battle */
    short	haste_nf;		/* will the player be hasted */
    short	strong_nf;		/* does player get a strength bonus */

	/* player currency */
    double	gold;			/* gold */
    double	gems;			/* gems */

	/* player equipment */
    double	sword;			/* sword */
    double	shield;			/* shield */
    float	quicksilver;		/* quicksilver */
    int		holywater;		/* holy water */
    int		amulets;		/* amulets */
    int		charms;			/* charms */
    short	crowns;			/* crowns */
    bool	virgin;			/* virgin */
    bool	palantir;		/* palantir */
    bool	blessing;		/* blessing */

    short	ring_type;		/* type of ring */
    int		ring_duration;		/* duration of ring */

	/* creation information */
    char	parent_account[SZ_NAME];	/* created by this account */
    char	parent_network[SZ_FROM];	/* created from this address */
    time_t	date_created;		/* created at this time */
    bool	faithful;		/* may other accounts load? */

	/* current or previous user information */
    char	last_IP[SZ_FROM];	/* last IP or DNS address */
    char	last_account[SZ_NAME];	/* last account accessed */
    time_t	last_load;		/* time last accessed */
    time_t	last_reset;		/* time password was last reset */
    int		load_count;		/* times character has been loaded */
    time_t	time_played;		/* seconds character has been played */

	/* hack foilers */
    int		bad_passwords;		/* unsuccessful load attempts */
    int		muteCount;		/* times caught spamming */
    time_t	lastMute;		/* last instance spammed */

	/* purgatory options */
    bool	purgatoryFlag;
    short	monsterNumber;
};


struct player_mod_t	/* information to modify an account */
{
	/* new name */
    bool	newName;		/* is there a new name? */
    char name[SZ_NAME];			/* character's new name */
    char lcName[SZ_NAME];		/* character's new name lowercase */

	/* new password */
    bool	newPassword;		/* is there a new password? */
    unsigned char password[16];		/* hash of the new password */
    bool	passwordReset;		/* is the password being reset? */

	/* sharing permissions */
    bool	newPermissions;		/* are we sending a perm change? */
    bool	faithful;		/* is the character faithful */

	/* info updates */
    bool	badPassword;		/* load attempt */
};


struct opponent_t	/* opponent attributes for battle */
{
	/* opponent information */
    char	name[SZ_MONSTER_NAME];	/* opponent name */
    char	realName[SZ_MONSTER_NAME]; /* opponent's real name */
    short	type;			/* opponent type */
    int		processID;		/* IT combat opponent process ID */

	/* opponent stats */
    double	experience;		/* experience */
    double	strength;		/* strength */
    double	max_strength;		/* maximum strength */
    double	energy;			/* energy */
    double	max_energy;		/* maximum energy */
    float	speed;			/* speed */
    float	max_speed;		/* maximum speed */
    double	brains;			/* brains */
    double	size;			/* monster size */
    float       sin;                    /* sin */

	/* battle info */
    double	shield;			/* shield */
    short	special_type;		/* special monster flag */
    short	treasure_type;		/* treasure type */
    short	flock_percent;		/* percent chance of flocking */
};

struct battle_t		/* battle information */
{
	/* player information */
    double	force_field;		/* force field */
    double	strengthSpell;		/* strength bonus */
    double	speedSpell;		/* speed bonus */
    bool	ring_in_use;		/* ring flag */

	/* opponent information */
    struct opponent_t *opponent;	/* pointer to opponent info */

	/* battle information */
    double	melee_damage;		/* damage done in melee */
    double	skirmish_damage;	/* damage done in skirmish */
    bool	tried_luckout;		/* luckout flag */
    int		rounds;			/* count of player attacks */
    int		timeouts;		/* count of player timeouts */
};

struct it_combat_t
{
    pthread_mutex_t theLock;

    struct opponent_t opponent[2];	/* structures to attack */
    bool opponentFlag[2];
    short message;
    double arg1;
    struct player_t *player_ptr;

	/* other battles */
    struct it_combat_t *next_opponent;	/* pointer to next player to fight */
};

struct client_t		/* structure for client variables */
{
	/* socket variables */
    int		socket;			/* the client-player socket */
    bool	socket_up;		/* flag if socket is active */
    struct sockaddr_in address;		/* player address */
    char out_buffer[SZ_OUT_BUFFER];	/* storage for outgoing messages */
    size_t out_buffer_size;		/* number of bytes in out_buffer */
    char in_buffer[SZ_IN_BUFFER];	/* storage for outgoing messages */
    size_t in_buffer_size;		/* number of bytes in in_buffer */

	/* connection information */
    char IP[SZ_FROM];			/* IP or DNS of player */
    char network[SZ_FROM];		/* IP or DNS of player's network */
    bool addressResolved;		/* is the address a DNS entry? */
    char connection_id[SZ_FROM + 15];	/* IP or DNS : process id */
    long machineID;			/* Cookie assigned to system */
    short run_level;			/* area of game player is in */
    char modifiedName[SZ_NAME];		/* tagged name */

	/* account information */
    char account[SZ_NAME];		/* account logged in as */
    char lcaccount[SZ_NAME];		/* account in lowercase */
    char wizaccount[SZ_NAME];		/* wizard account for backdoor */
    char wizIP[SZ_FROM];		/* wizard IP for backdoor */
    char previousName[SZ_NAME];		/* saves the name when killed */
    char email[SZ_FROM];		/* player e-mail from account info */
    char parentNetwork[SZ_FROM];	/* network of the account */
    time_t date_connected;		/* time this connection started */

	/* client variables */
    int		channel;		/* what chat channel is being used */
    int		timeout;		/* current time player has to act */ 
    int		timeoutAt;		/* current time player has to act */ 
    short	timeoutFlag;		/* is a timeout response expected? */
    double	knightEnergy;		/* energy bonus for knighthood */
    float	knightQuickness;	/* quickness bonus for kinghthood */
    double  morgothCount;		/* time since the last Morgoth defeated */
    int		ageCount;			/* one ring counter */
    short	wizard;			/* administrator level */
    bool	broadcast;		/* broadcast next chat? */
    bool	stuck;			/* did the player stay for it-combat */
    bool	suspended;		/* hold game and only hear wizards */
    time_t	muteUntil;		/* time after which player can chat */
    time_t	tagUntil;		/* time the player's name tag disappears */
    bool	hearBroadcasts;		/* are server messages posted? */
    bool	accountLoaded;		/* do we have an account? */
    bool	characterLoaded;	/* do we have a character? */
    bool	characterAnnounced;	/* has the character been accounced? */

	/* hack information */
    time_t	chatTimes[10];
    int   	chatLength[10];
    short	swearCount;

	/* directory to important information */
    struct player_t player;		/* the main player structure */
    struct battle_t battle;		/* structure used for battles */
    struct event_t *events;		/* pointer to character events */
    struct realm_t *realm;		/* pointer to server realm variables */
    struct game_t *game;		/* pointer to game's game_t struct */
};

struct player_desc_t	/* common information about a player */
{
	/* Player information */
    char	name[SZ_NAME];		/* name of the character */
    char	lcname[SZ_NAME];	/* lowercase character name */
    char	parent_account[SZ_NAME]; /* character parent account */
    short	type;			/* character class */
    short	special_type;		/* special character type */
    bool	gender;			/* is the character male or female */
    double	level;			/* the character's current level */
    int		channel;		/* the chat channel being used */
    short       wizard;			/* is player game admin */
    bool	cloaked;		/* is the player cloaked */
    bool	palantir;		/* does the player have a palantir */
    bool	blind;			/* is character blind? */
};

struct game_t		/* structure for a linked list of games */
{
	/* the thread itself */
    pthread_t	the_thread;		/* the game thread */
    bool	cleanup_thread;		/* has the thread ended? */
    int		the_socket;
    pid_t	clientPid;		/* the process ID of the client */

	/* player information */
    char	IP[SZ_FROM];		/* IP being used */
    char	network[SZ_FROM];	/* network being used */
    char	account[SZ_NAME];	/* account being used */
    long	machineID;		/* machine number */

	/* character information */
    double	x;			/* x-coordinate */
    double	y;			/* y-coordinate */
    bool	virtual;		/* player not at specific coordinates */
    char	area[SZ_AREA];		/* name of area in the game */
    bool	useLocationName;	/* always show area over coords */
    bool	palantir;		/* does the player have a palantir */
    double	circle;			/* the circle the character is in */
    int	        hearAllChannels;	/* can this player hear all chat? */
    bool	chatFilter;
    bool	sendEvents;		/* include this player on broadcasts? */

    struct player_desc_t *description;	/* pointer to the player description */
    struct it_combat_t *it_combat;	/* pointer to character's it_combat */

	/* event traffic */
    struct event_t *events_in;		/* queue for events to the thread */
    pthread_mutex_t events_in_lock; /* lock on events in */

	/* server info */
    struct game_t *next_game;		/* pointer to the next game */
};

struct event_t		/* structure to describe actions */
{
	/* event information */
    short	type;			/* the type of event */
    double	arg1, arg2;		/* event arguments */
    long int	arg3;			/* another argument */
    void	*arg4;			/* and more arguments */

	/* addressing information */
    struct game_t *from;		/* who created the event */
    struct game_t *to;			/* where the event is going */
    struct event_t *next_event;		/* pointer to the next event */
};

struct scoreboard_t			/* scoreboard entry */
{
    double      level;			/* level of player */
    char        class[SZ_CLASS_NAME];	/* character type of player */
    char        name[SZ_NAME];		/* name of player */
    char        from[SZ_FROM];		/* ip or DNS of player */
    char	how_died[SZ_HOW_DIED];	/* description of player's fate */
    time_t	time;			/* time of death */
};

struct monster_t	/* base monster information */
{
    char	name[SZ_MONSTER_NAME];	/* name of the monster */
    short	special_type;			/* monster special type */
    double	experience;		/* monster experience */
    double	energy;			/* monster energy */
    double	strength;		/* monster strength */
    double	brains;			/* monster brains */
    double	speed;			/* monster speed */
    short	treasure_type;		/* monster treasure type */
    short	flock_percent;		/* percent chance of flocking */	
};

struct charstats_t			/* character type statistics */
{
    char	class_name[SZ_CLASS_NAME];/* name of the character class */
    char	short_class_name;	/* character class abriviation */
    double	max_brains;            /* max brains per level */
    double	max_mana;              /* max mana per level */
    double	weakness;             /* how strongly poison affects player */
    double	goldtote;             /* how much gold char can carry */
    int		ring_duration;         /* bad ring duration */
    struct
        {
        double	base;           /* base for roll */
        double	interval;       /* interval for roll */
        double	increase;       /* increment per level */
        } quickness,          /* quickness */
          strength,           /* strength */
          mana,               /* mana */
          energy,             /* energy level */
          brains,             /* brains */
          magiclvl;           /* magic level */
};

struct shop_item_t			/* menu item for purchase */
{
    char        item[SZ_ITEMS];		/* menu item name */
    double      cost;			/* cost of item */
};

struct realm_state_t    /* things to save upon shutdown */
{
    double      kings_gold;
    char        king_name[SZ_NAME];	/* name of old king */
    char        valar_name[SZ_NAME];	/* name of old valar */
};

struct realm_object_t	/* things to run into out there */
{
    double      x;			/* x coordinate */
    double      y;			/* y coordinate */
    short	type;			/* what the object is */
    void        *arg1;			/* item pointer argument */
    struct realm_object_t *next_object;	/* pointer to the next object */
};

struct realm_t		/* variables for the entire realm */
{
	/* server variables */
    pid_t serverPid;			/* the process ID of the server */
	
	/* the linked list of games */
    struct game_t *games;		/* begin linked list of games */
    struct game_t *knight;		/* pointer to current knight */
    struct game_t *king;		/* pointer to current king */
    struct game_t *valar;		/* pointer to current valar */
    bool king_flag;			/* true when there is a king */
    char king_name[SZ_NAME];		/* name of old king */
    char valar_name[SZ_NAME];		/* name of old valar */

	/* file mutexes */
    pthread_mutex_t backup_lock;	/* access to the backup file */
    pthread_mutex_t scoreboard_lock;	/* scoreboard file in use */
    pthread_mutex_t character_file_lock; /* locks access to character file */
    pthread_mutex_t log_file_lock;	/* locks all log files */
    pthread_mutex_t network_file_lock;	/* locks the network file */
    pthread_mutex_t tag_file_lock;	/* locks the tag file */
    pthread_mutex_t tagged_file_lock;	/* locks the tagged file */
    pthread_mutex_t history_file_lock;	/* locks the history file */

	/* variable/link list mutexes */
    pthread_mutex_t realm_lock;		/* locks the whole realm */
    pthread_mutex_t account_lock;	/* account file and email_limbo */
    pthread_mutex_t hack_lock;		/* locks hack stats */
    pthread_mutex_t monster_lock;	/* monster list lock */
    pthread_mutex_t object_lock;	/* realm object lock */
    pthread_mutex_t kings_gold_lock;	/* lock of the kings gold */
    pthread_mutex_t connections_lock;	/* lock the connection linked list */

	/* changing game variables */
    struct monster_t monster[NUM_MONSTERS]; /* list of all monsters */
    struct realm_object_t *objects;	/* list of objects in realm */
    double	kings_gold;		/* amount of gold in kings coffers */
    double	steward_gold;		/* amount of gold in stewards coffers */
    int nextTagNumber;			/* numbers for session tags */

	/* realm-wide fixed information */
    struct charstats_t charstats[NUM_CHARS]; /* character types */
    struct shop_item_t shop_item[NUM_ITEMS]; /* shop items */

	/* linked lists */
    struct connection_t *connections;	/* counts of logins and passwords */

	/* names and e-mail addresses in limbo */
    struct linked_list_t *name_limbo;
    struct linked_list_t *email_limbo;
};

struct account_t	/* structure to store account information */
{
	/* main information */
    char	name[SZ_NAME];		/* account name */
    char	lcname[SZ_NAME];	/* name in lowercase */
    unsigned char password[16];		/* password hash */
    time_t	last_reset;		/* time of last password reset */
    char	email[SZ_FROM];	/* creator e-mail */
    char	lcemail[SZ_FROM]; /* e-mail in lowercase */
    char	confirmation[SZ_PASSWORD]; /* first time confirmation */
    
	/* creation information */
    char        parent_IP[SZ_FROM];     /* created from this address */
    char        parent_network[SZ_FROM];     /* created from this address */
    time_t      date_created;          /* created at this time */

	/* previous user information */
    char        last_IP[SZ_FROM];       /* last IP or DNS address */
    char        last_network[SZ_FROM];  /* last IP or DNS address */
    time_t      last_load;		/* time last accessed */
    int		login_count;		/* number of times account used */

	/* hack foilers */
    int         bad_passwords;          /* unsuccessful load attempts */
    int		hackCount;		/* number of alleged hacks */
    int		rejectCount;		/* times given a reject tag */
    time_t	lastHack;		/* time of the last hack */
    int		muteCount;		/* times given a reject tag */
    time_t	lastMute;		/* time of the last hack */
};

struct account_mod_t	/* information to modify an account */
{
	/* new password */
    bool	newPassword;		/* is there a new password? */
    unsigned char password[16];		/* hash of the new password */
    bool	passwordReset;		/* is the password being reset? */

	/* e-mail resets - not used */ 
    bool	newEmail;
    char	email[SZ_FROM];
    char	confirmation[SZ_PASSWORD];

	/* hack information */
    bool	hack;
    int		hackCount;
    bool	mute;
    int		muteCount;

	/* info updates */
    bool	confirm;
    bool	access;
    bool	badPassword;
};

struct server_t		/* structure for top server variables */
{
	/* server variables */
    short	run_level;		/* flag set to shutdown server */
    int		the_socket;		/* socket to listen for connections */

	/* game variables */
    short	num_games;		/* current count of players */
    struct realm_t realm;		/* realm variables */
};

struct player_spec_t	/* info for java client interface */
{
	/* the data */
    char name[SZ_NAME + 1];			/* character name */
    char type[SZ_SPEC];			/* character type and special type */
};

struct player_stats_t	/* main display information */
{
	/* player location */
    unsigned int x1;			/* x coord */
    unsigned int x2;			/* x coord */
    unsigned int y1;			/* y coord */
    unsigned int y2;			/* y coord */

	/* player stats */
    unsigned int strength1;		/* strength */
    unsigned int strength2;		/* strength */
    unsigned int max_str1;		/* maximum strength */
    unsigned int max_str2;		/* maximum strength */
    unsigned int energy1;		/* energy */
    unsigned int energy2;		/* energy */
    unsigned int max_ener1;		/* maximum energy */
    unsigned int max_ener2;		/* maximum energy */
    unsigned int quickness1;		/* quickness */
    unsigned int quickness2;		/* quickness */
    unsigned int max_quick1;		/* quickness */
    unsigned int max_quick2;		/* quickness */
    unsigned int mana1;			/* mana */
    unsigned int mana2;			/* mana */
    unsigned int level1;		/* level */
    unsigned int level2;		/* level */

	/* player currency */
    unsigned int gold1;			/* gold */
    unsigned int gold2;			/* gold */

	/* player equipment */
    unsigned int sword1;		/* sword */
    unsigned int sword2;		/* sword */
    unsigned int shield1;		/* shield */
    unsigned int shield2;		/* shield */
    unsigned int quicksilver1;		/* quicksilver */
    unsigned int quicksilver2;		/* quicksilver */
    bool	crowns;			/* crowns */
    bool	virgin;			/* virgin */
    bool	palantir;		/* palantir */
    bool	blessing;		/* blessing */
    bool	ring;			/* ring */

	/* player information */
    char	name[SZ_NAME + 1]; 	/* player name */
    char	area[SZ_AREA + 1]; /* name of the area player is in */
};

struct button_t		/* client button configuration */
{
	/* compass buttons */
    char	compass;

	/* interface buttons */
    char	button[8][SZ_LABEL];
};

struct linked_list_t	/* generic linked list */
{
    char	name[SZ_FROM];
    struct linked_list_t *next;
};

struct examine_t		/* examine strcture */
{
    char	title[SZ_NAME + 30];	/* player name */
    char	location[50];	/* name of the area player is in */

	/* player information */
    char	account[SZ_NAME];	/* account player is using */
    char	network[SZ_FROM];	/* player's domain */
    short	channel;		/* channel player is on */

	/* player stats */
    double	level;			/* level */
    double	experience;		/* experience */
    double	nextLevel;		/* experience */

    double	strength;		/* strength */
    double	max_strength;		/* maximum strength */
    double	energy;			/* energy */
    double	max_energy;		/* maximum energy */
    float	quickness;		/* quickness */
    float	max_quickness;		/* quickness */
    double	mana;			/* mana */
    double	brains;			/* brains */
    double	magiclvl;		/* magic level */
    float	poison;			/* poison */
    float	sin;			/* sin */
    char        gender[7];		/* player is male/female */
    short	lives;			/* multiple lives for council, valar */

	/* player status */
    char	cloaked[5];		/* is character cloaked */
    char        blind[5];			/* blindness */

	/* player currency */
    double	gold;			/* gold */
    double	gems;			/* gems */

	/* player equipment */
    double	sword;			/* sword */
    double	shield;			/* shield */
    float	quicksilver;		/* quicksilver */
    int		holywater;		/* holy water */
    int		amulets;		/* amulets */
    int		charms;			/* charms */
    short	crowns;			/* crowns */
    char	virgin[5];			/* virgin */
    char	palantir[5];		/* palantir */
    char	blessing[5];		/* blessing */
    char	ring[5];		/* ring */

	/* creation information */
    int		age;			/* age of player */
    int		degenerated;		/* age/3000 last degenerated */
    char	date_loaded[30];		/* loaded on this date */
    char	date_created[30];		/* created at this time */
    char	time_played[12];		/* seconds character has been played */

};

struct tag_t	/* saved struct of bans, mutes, etc. */
{
    int		number;
    short 	type;
    time_t	validUntil;
    bool	affectNetwork;
    bool	contagious;
    char	description[SZ_FROM];
};

struct tagged_t	/* saved struct of bans, mutes, etc. */
{
    int		tagNumber;
    short 	type;
    char	name[SZ_FROM];
    time_t	validUntil;
};

struct network_t	/* strcture to save hack info for networks */
{
    char	address[SZ_FROM];
    int		hackCount;
    time_t	lastHack;
    int		muteCount;
    time_t	lastMute;
    time_t	expires;
};

struct connection_t	/* short term struct to watch for hacks by connection */
{
    char	theAddress[SZ_FROM];
    time_t	connections[10];
    int		connectionCount;
    time_t	badPasswords[8];
    int		badPasswordCount;
    time_t	eraseAt;
    struct connection_t *next;
};

struct tagged_list_t 	/* to create a list of tags */
{
    struct tagged_t theTagged;
    struct tagged_list_t *next;
};

struct history_t	/* list of past administrative events */
{
    time_t	date;
    short	type;
    char	name[SZ_FROM];
    char	description[SZ_LINE + 50];
};

struct history_list_t 	/* to create a list of history */
{
    struct history_t theHistory;
    struct history_list_t *next;
};

struct detail_t		/* information on player's connection */
{
    char modifiedName[SZ_NAME];
    char name[SZ_NAME];
    char faithful[5];
    char parentAccount[SZ_NAME];
    char charParentNetwork[SZ_FROM];
    int playerMutes;

    char account[SZ_NAME];
    char email[SZ_FROM];
    char accParentNetwork[SZ_FROM];
    int accountMutes;

    char IP[SZ_FROM];
    char network[SZ_FROM];
    int machineID;
    char dateConnected[30];
    int networkMutes;
};

struct tagged_sort_t	/* strcture to help tag inheritance */
{
    int tag;
    struct tagged_t *tagged[4];
    struct tagged_sort_t *next;
};

