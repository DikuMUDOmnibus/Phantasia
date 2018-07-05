/*
 * phantdefs.h - important constants for Phantasia
 */

/* testig stuff - should be delete before production */

/* configurables */
#define PHANTASIA_PORT 9897

/* boolean identifiers */
typedef char bool;
#define TRUE			1
#define FALSE			0

#define MALE			0
#define FEMALE			1

/* server run_level constants */
#define RUN_SERVER		0	/* continue through main loop */
#define HARD_SHUTDOWN		1	/* kill server NOW */
#define FAST_SHUTDOWN		2	/* just save the realm */
#define SHUTDOWN		3	/* wait on threads, then save */
#define LEISURE_SHUTDOWN	4	/* wait until no one is logged on */

/* thread run_level constants */
#define SIGNING_IN		0	/* connection has no account */
#define CHAR_SELECTION		1	/* player needs a character */
#define PLAY_GAME		2	/* in play */
#define SAVE_AND_CONTINUE	3	/* save char and ask to continue */
#define SAVE_AND_EXIT		4	/* save char and leave game */
#define GO_AGAIN		5	/* save char and leave game */
#define EXIT_THREAD		6	/* leave the main loop */
#define LEAVING_THREAD		7	/* Final thread exit */

/* socket return values */
#define S_NORM			0	/* socket returned data */
#define S_CANCEL		1	/* player canceled selection */
#define S_TIMEOUT		2	/* player ran out of time */
#define S_ERROR			3	/* socket is closed */

/* tag types */
#define T_REJECT		0	/* hang up on address ASAP */
#define T_BAN			1	/* kick when character loaded, ban */
#define T_SUICIDE		2	/* all loaded characters hari-kari */
#define T_MUTE			3	/* player may not chat */
#define T_PREFIX		4	/* add a prefix to the player */
#define T_SUFFIX		5	/* add a suffix to the player */

/* tagged types */
#define T_MACHINE		0	/* tag of a specific machine */
#define T_ACCOUNT		1	/* tag of an account */
#define T_ADDRESS		2	/* tag of an address */
#define T_NETWORK		3	/* tag of an network */

/* hearing constants */
#define HEAR_SELF               0       /* hear the channel you're in */
#define HEAR_ONE                1       /* hear channel 1 -- apprentice */
#define HEAR_ALL                11      /* hear all channels */

/* realm objects */
#define ENERGY_VOID		0	/* object is an energy void */
#define CORPSE			1	/* object is a corpse */
#define TREASURE_TROVE		2	/* opject is the treasure trove */
#define HOLY_GRAIL		3	/* object is the holy grail */

/* total number of things */
#define NUM_MONSTERS		100	/* monsters in the game */
#define NUM_CHARS		6	/* number of character types */
#define NUM_ITEMS		7	/* items in the shops */

/* event types */
#define NULL_EVENT		0	/* blank event */
	/* immediate events */
#define KICK_EVENT		1	/* get off my game */
#define TAG_EVENT		2	/* tag yourself */
#define REQUEST_DETAIL_EVENT	3	/* get player info */
#define CONNECTION_DETAIL_EVENT	4	/* another player's info */
#define REQUEST_RECORD_EVENT	5	/* get player info */
#define PLAYER_RECORD_EVENT	6	/* another player's info */
#define ADD_PLAYER_EVENT	7	/* add player info */
#define REMOVE_PLAYER_EVENT	8	/* remove player info */
#define CHANGE_PLAYER_EVENT	9	/* change player info */
#define CHAT_EVENT		10	/* chat message */
#define REPRIMAND_EVENT		11	/* Apprentices reprimand */
#define UNTAG_EVENT		12	/* remove a prefix or suffix */
#define UNSUSPEND_EVENT		13	/* resume player's game */
#define GAME_MARKER		14	/* only game events follow */
	/* ASAP events */
#define DEATH_EVENT		20	/* player died */
#define IT_COMBAT_EVENT		21	/* enter interterminal-combat */
#define EXPERIENCE_EVENT	23	/* award the player experience */
	/* tampering events */
#define SUSPEND_EVENT		25	/* stop player's game */
#define CANTRIP_EVENT           26	/* apprentice options */
#define MODERATE_EVENT		27	/* wizard options */
#define ADMINISTRATE_EVENT	28	/* administrative options */
#define VALAR_EVENT		29	/* become/lose valar */
#define KING_EVENT		30	/* become king */
#define STEWARD_EVENT		31	/* become steward */
#define DETHRONE_EVENT		32	/* lose king or steward */
#define SEX_CHANGE_EVENT        34      /* toggle player's sex */
#define RELOCATE_EVENT		35	/* order player to location */
#define TRANSPORT_EVENT		36	/* transport player */
#define CURSE_EVENT		37	/* curse player */
#define SLAP_EVENT		38	/* slap player */
#define BLIND_EVENT		39	/* blind player */
#define BESTOW_EVENT		40	/* king bestowed gold */
#define SUMMON_EVENT		41	/* summon monster for player */
#define BLESS_EVENT		42
#define HEAL_EVENT		43
#define STRONG_NF_EVENT		44	/* set the player's strength_nf flag */
#define KNIGHT_EVENT		45	/* this player has been knighted */
#define DEGENERATE_EVENT	46
#define HELP_EVENT		47	/* player is asking for information */

	/* command events */
#define COMMAND_EVENT		48	/* the valar uses a power */
#define SAVE_EVENT		49	/* save the game and quit */
#define MOVE_EVENT		50	/* move the character */
#define EXAMINE_EVENT		51	/* examine the stats on a character */
#define DECREE_EVENT		52	/* make a decree */
#define ENACT_EVENT		53	/* the steward enacts something */
#define LIST_PLAYER_EVENT	54	/* list the players in the game */
#define CLOAK_EVENT		55	/* cloak/uncloak */
#define TELEPORT_EVENT		56	/* teleport player */
#define INTERVENE_EVENT		57	/* a council uses a power */
#define REST_EVENT		58	/* rest player */
#define INFORMATION_EVENT	59	/* go to information screen */
#define FORCEAGE_EVENT      60  /* forcefully age a player */

	/* normal events */
    /* events after this are destroyed on orphan */
#define DESTROY_MARKER		69
#define ENERGY_VOID_EVENT	70	/* create/hit energy void */
#define TROVE_EVENT		71	/* find the treasure trove */
#define MONSTER_EVENT		72	/* encounter monster */
#define PLAGUE_EVENT		73	/* hit with plague */
#define MEDIC_EVENT		74	/* encounter medic */
#define GURU_EVENT		75	/* encounter guru */
#define TRADING_EVENT		76	/* find a trading post */
#define TREASURE_EVENT		77	/* find treasure */
#define VILLAGE_EVENT		78	/* found a village or a volcano */
#define TAX_EVENT		79	/* encounter tax collector */

	/* realm objects */
    /* events after this are made realm objects on orphan */
#define REALM_MARKER		90
#define CORPSE_EVENT		91	/* find a corpse */
#define GRAIL_EVENT		92	/* find the holy grail */
#define LAST_EVENT		93	/* used to find bad events */

/* combat messages */
#define IT_OPPONENT_BUSY	1	/* currently on another combat */
#define IT_REPORT		2	/* report in */
#define IT_JUST_DIED		3	/* player has just been killed */
#define IT_JUST_LEFT		42	/* player just left the game */
#define IT_ATTACK		4	/* player is attacker */
#define IT_DEFEND		5	/* player is defender */
#define IT_MELEE		6	/* attacker meleed */
#define IT_SKIRMISH		7	/* attacker skirmished */
#define IT_NICKED		8	/* attacker nicked */
#define IT_EVADED		9	/* attacker evaded */
#define IT_NO_EVADE		10	/* attacker failed to evade */
#define IT_LUCKOUT		11	/* attacker lucked-out (luckouted?) */
#define IT_NO_LUCKOUT		12	/* attacker failed to luckout */
#define IT_RING			13	/* attacker put on a ring */
#define IT_NO_RING		14	/* attacker failed to put on a ring */
#define IT_ALL_OR_NOT		15	/* attacker cast all or nothing */
#define IT_NO_ALL_OR_NOT	16	/* attacker blew all or nothing */
#define IT_BOLT			17	/* attacker cast magic bolt */
#define IT_NO_BOLT		18	/* attacker blew magic bolt */
#define IT_SHIELD		19	/* attacker cast force field */
#define IT_NO_SHIELD		20	/* attacker blew force field */
#define IT_TRANSFORM		21	/* attacker transformed defender */
#define IT_NO_TRANSFORM		22	/* attacker blew transform */
#define IT_TRANSFORM_BACK	23	/* attacker's transform backfired */
#define IT_MIGHT		24	/* attacker cast increase might */
#define IT_NO_MIGHT		25	/* attacker blew increase might */
#define IT_HASTE		26	/* attacker cast haste */
#define IT_NO_HASTE		27	/* attacker blew haste */
#define IT_TRANSPORT		28	/* attacker cast transport */
#define IT_NO_TRANSPORT		29	/* attacker blew transport */
#define IT_TRANSPORT_BACK	30	/* attacker's transport backfired */
#define IT_PARALYZE		31	/* attaker cast paralyze */
#define IT_NO_PARALYZE		32	/* attaker blew paralyze */
#define IT_PASS			33	/* attacker passed the turn */
#define IT_CONTINUE		34	/* defender continues the battle */
#define IT_CONCEDE		35	/* defender surrenders */
#define IT_DEFEAT		36	/* the sender stands defeated */
#define IT_VICTORY		37	/* the sender claims victory */
#define IT_DONE			38	/* This is Kang, cease hostilities */
#define IT_ECHO			39	/* tell me to attack */
#define IT_ABANDON		40	/* player quiting and saving self  */
#define IT_BORED		41	/* player quit after timeout */
#define IT_WIZEVADE             43	/* player used wiz powers to evade */

/* client->player packet headers */
#define HANDSHAKE_PACKET	2	/* used when connecting */
#define CLOSE_CONNECTION_PACKET	3	/* last message before close */
#define PING_PACKET		4	/* used for timeouts */
#define ADD_PLAYER_PACKET	5	/* add a player to the list */
#define REMOVE_PLAYER_PACKET	6	/* remove a player from the list */
#define SHUTDOWN_PACKET		7	/* the server is going down */
#define ERROR_PACKET		8	/* server has encountered an error */

#define CLEAR_PACKET		10	/* clears the message screen */
#define WRITE_LINE_PACKET	11	/* write a line on message screen */

#define BUTTONS_PACKET		20	/* use the interfaces buttons */
#define FULL_BUTTONS_PACKET	21	/* use buttons and compass */
#define STRING_DIALOG_PACKET	22	/* request a message response */
#define COORDINATES_DIALOG_PACKET 23	/* request player coordinates */
#define PLAYER_DIALOG_PACKET	24	/* request a player name */
#define PASSWORD_DIALOG_PACKET	25	/* string dialog with hidden text */
#define SCOREBOARD_DIALOG_PACKET 26	/* pull up the scoreboard */
#define DIALOG_PACKET		 27	/* okay dialog with next line */

#define CHAT_PACKET		30	/* chat message */
#define ACTIVATE_CHAT_PACKET	31	/* turn on the chat window */
#define DEACTIVATE_CHAT_PACKET	32	/* turn off the chat window */
#define PLAYER_INFO_PACKET	33	/* display a player's info */
#define CONNECTION_DETAIL_PACKET 34	/* display connection info */

#define NAME_PACKET		40	/* set the player's name */
#define LOCATION_PACKET		41	/* refresh the player's energy */
#define ENERGY_PACKET		42	/* refresh the player's energy */
#define STRENGTH_PACKET		43	/* refresh the player's strength */
#define SPEED_PACKET		44	/* refresh the player's speed */
#define SHIELD_PACKET		45	/* refresh the player's shield */
#define SWORD_PACKET		46	/* refresh the player's sword */
#define QUICKSILVER_PACKET	47	/* refresh the player's quicksilver */
#define MANA_PACKET		48	/* refresh the player's mana */
#define LEVEL_PACKET		49	/* refresh the player's level */
#define GOLD_PACKET		50	/* refresh the player's gold */
#define GEMS_PACKET		51	/* refresh the player's gems */
#define CLOAK_PACKET		52	/* refresh the player's cloak */
#define BLESSING_PACKET		53	/* refresh the player's blessing */
#define CROWN_PACKET		54	/* refresh the player's crowns */
#define PALANTIR_PACKET		55	/* refresh the player's palantir */
#define RING_PACKET		56	/* refresh the player's ring */
#define VIRGIN_PACKET		57	/* refresh the player's virgin */


/* player->client packet headers */
#define C_RESPONSE_PACKET	1	/* player feedback for game */
#define C_CANCEL_PACKET		2	/* player canceled question */
#define C_PING_PACKET		3	/* response to a ping */
#define C_CHAT_PACKET		4	/* chat message from player */
#define C_EXAMINE_PACKET	5	/* examine a player */
#define C_ERROR_PACKET		6	/* client is lost */
#define C_SCOREBOARD_PACKET	7	/* show the scoreboard */

/* locations within the realm */
#define PL_REALM		0	/* normal coordinates */
#define	PL_THRONE		1	/* In the lord's chamber */
#define PL_EDGE			2	/* On the edge of the realm */
#define PL_VALHALLA 		3	/* In Valhalla */
#define PL_PURGATORY		4	/* In purgatory fighting */

/* size of many structures */
#define SZ_PLAYER sizeof(struct player_t)	/* size of player_t */
#define SZ_GAME sizeof(struct game_t)	/* size of game_t */
#define SZ_IT_COMBAT sizeof(struct it_combat_t)	/* size of it_combat_t */
#define SZ_PLAYER_DESC sizeof(struct player_desc_t) /* size of player_desc_t */
#define SZ_PLAYER_SPEC sizeof(struct player_spec_t) /* size of player_spec_t */
#define SZ_EVENT sizeof(struct event_t)	/* size of event_t */
#define SZ_REALM_STATE sizeof(struct realm_state_t)	/* size of realm_state_t */
#define SZ_REALM_OBJECT sizeof(struct realm_object_t)	/* size of realm_object_t */
#define SZ_SCOREBOARD sizeof(struct scoreboard_t)	/* size of scoreboard_t */
#define SZ_CLIENT sizeof(struct client_t)	/* size of client_t */
#define SZ_OPPONENT sizeof(struct opponent_t)	/* size of opponent_t */
#define SZ_BUTTON sizeof(struct button_t)	/* size of button_t */
#define SZ_ACCOUNT sizeof(struct account_t)	/* size of account_t */
#define SZ_LINKED_LIST sizeof(struct linked_list_t) /* size of linked_list_t */
#define SZ_EXAMINE sizeof(struct examine_t)
#define SZ_TAG sizeof(struct tag_t)
#define SZ_TAGGED sizeof(struct tagged_t)
#define SZ_TAGGED_LIST sizeof(struct tagged_list_t)
#define SZ_NETWORK sizeof(struct network_t)
#define SZ_CONNECTION sizeof(struct connection_t)
#define SZ_HISTORY sizeof(struct history_t)
#define SZ_HISTORY_LIST sizeof(struct history_list_t)
#define SZ_DETAIL sizeof(struct detail_t)
#define SZ_TAGGED_SORT sizeof(struct tagged_sort_t)

/* string sizes */
#define SZ_IN_BUFFER		1024	/* largest possible client message */
#define SZ_OUT_BUFFER		1024	/* largest possible server message */
#define SZ_NAME			33	/* player name field (incl. trailing null) */
#define MAX_NAME_LEN		16	/* actual player name */
#define SZ_PASSWORD		16	/* 128 bit MD5 hash of the password */
#define SZ_FROM			81	/* ip or dns login (incl. null) */
#define SZ_MONSTER_NAME		49	/* characters in monster names */
#define SZ_AREA			24	/* name of player location */
#define SZ_HOW_DIED		78	/* string describing character death */
#define SZ_CLASS_NAME		13	/* longest class name */
#define	SZ_ITEMS		12	/* longest shop item description */
#define	SZ_ERROR_MESSAGE	256	/* max length of error message */
#define SZ_LINE			256	/* length of one line on terminal */
#define SZ_LABEL		22	/* length of interface button text */
#define SZ_NUMBER		25	/* characters describing number */
#define SZ_CHAT			512	/* largest chat message */
#define SZ_PACKET_TYPE		2	/* maximum packet size */
#define SZ_SPEC			7	/* 5 chars, newline and null */

/* possible errors */
#define MALLOC_ERROR		1001
#define DATA_FILE_ERROR		1002
#define MONSTER_FILE_ERROR	1003
#define CHARACTER_FILE_ERROR	1004
#define CHARSTATS_FILE_ERROR	1004
#define SHOPITEMS_FILE_ERROR	1004
#define SCOREBOARD_FILE_ERROR	1005
#define CHAT_LOG_FILE_ERROR	1006
#define SOCKET_CREATE_ERROR	1007
#define SOCKET_BIND_ERROR	1008
#define SOCKET_LISTEN_ERROR	1009
#define SOCKET_SELECT_ERROR	1010
#define SOCKET_ACCEPT_ERROR	1011
#define MUTEX_INIT_ERROR	1012
#define MUTEX_DESTROY_ERROR	1013
#define MUTEX_LOCK_ERROR	1014
#define MUTEX_UNLOCK_ERROR	1015
#define PTHREAD_ATTR_ERROR	1016
#define PTHREAD_CREATE_ERROR	1017
#define GENERAL_EVENT_ERROR	1021
#define IMPOSSIBLE_EVENT_ERROR	1022
#define EVENT_ADDRESS_ERROR	1023
#define UNDEFINED_OBJECT_ERROR	1024
#define BATTLE_PHASE_ERROR	1025
#define DEFENDER_MESSAGE_ERROR	1026
#define BATTLE_MESSAGE_ERROR	1027
#define SEND_SOCKET_ERROR	1028
#define READ_SOCKET_ERROR	1029

/* ring constants */
#define R_NONE          0               /* no ring */
#define R_NAZREG        1               /* regular Nazgul ring (expires) */
#define R_DLREG         2               /* regular Dark Lord ring (does not expire) */
#define R_BAD           3               /* bad ring */
#define R_SPOILED       4               /* ring which has gone bad */
#define R_YES		5		/* masked ring type */

/* constants for character types */
#define C_MAGIC         0               /* magic user */
#define C_FIGHTER       1               /* fighter */
#define C_ELF           2               /* elf */
#define C_DWARF         3               /* dwarf */
#define C_HALFLING      4               /* halfling */
#define C_EXPER         5               /* experimento */

/* constants for special character types */
#define SC_NONE         0               /* not a special character */
#define SC_KNIGHT       1               /* knight */
#define SC_STEWARD      2               /* steward */
#define SC_KING         3               /* king */
#define SC_COUNCIL      4               /* council of the wise */
#define SC_EXVALAR      5               /* past valar - now council */
#define SC_VALAR        6               /* valar */

    /* means of death */
#define K_OLD_AGE	0		/* old age */
#define K_MONSTER	1		/* combat with monster */
#define K_IT_COMBAT	2		/* combat with another player */
#define K_GHOSTBUSTERS	3		/* lost connection */
#define K_VAPORIZED	4		/* vaporized by another player */
#define K_RING		5		/* killed by a cursed ring */
#define K_NO_ENERGY	6		/* player ran out of energy */
#define K_FELL_OFF	7		/* fell off the edge of the world */
#define K_TRANSFORMED   8		/* turned into another monster */
#define K_SEGMENTATION	9		/* bad internal error */
#define K_SUICIDE	10		/* character did something bad */
#define K_SQUISH	11		/* new wizard kill option */
#define K_GREED		12		/* killed when carrying too much gold */
#define K_FATIGUE	13		/* killed when speed = 0 from fatigue */
#define K_SIN    	14		/* goes to hell for too much evil */

    /* special monster constants */
#define SM_RANDOM	-1		/* pick a monster by normal means */
#define SM_NONE         0               /* nothing special */
#define SM_UNICORN      1               /* unicorn */
#define SM_MODNAR       2               /* Modnar */
#define SM_MIMIC        3               /* mimic */
#define SM_DARKLORD     4               /* Dark Lord */
#define SM_LEANAN       5               /* Leanan-Sidhe */
#define SM_SARUMAN      6               /* Saruman */
#define SM_THAUMATURG   7               /* thaumaturgist */
#define SM_BALROG       8               /* balrog */
#define SM_VORTEX       9               /* vortex */
#define SM_NAZGUL       10              /* nazgul */
#define SM_TIAMAT       11              /* Tiamat */
#define SM_KOBOLD       12              /* kobold */
#define SM_SHELOB       13              /* Shelob */
#define SM_FAERIES      14              /* assorted faeries */
#define SM_LAMPREY      15              /* lamprey */
#define SM_SHRIEKER     16              /* shrieker */
#define SM_BONNACON     17              /* bonnacon */
#define SM_SMEAGOL      18              /* Smeagol */
#define SM_SUCCUBUS     19              /* succubus */
#define SM_CERBERUS     20              /* Cerberus */
#define SM_UNGOLIANT    21              /* Ungoliant */
#define SM_JABBERWOCK   22              /* jabberwock */
#define SM_MORGOTH      23              /* Morgoth */
#define SM_TROLL        24              /* troll */
#define SM_WRAITH       25              /* wraith */
#define SM_TITAN        26              /* titan */
#define SM_IT_COMBAT    27              /* fighting another player */
#define SM_IDIOT        28              /* idiot */
#define SM_SMURF        29              /* smurf */
#define SM_MORON        30              /* moron */

    /* encounter constants */
#define MONSTER_RANDOM		0	/* monster was wandering */
#define MONSTER_CALL		1	/* monster was hunted */
#define MONSTER_FLOCKED		2	/* another monster in herd */
#define MONSTER_SHRIEKER	3	/* called by shrieker */
#define MONSTER_JABBERWOCK	4	/* called by jabberwock */
#define MONSTER_TRANSFORM	5	/* monster was polymorphed */
#define MONSTER_SUMMONED	6	/* another player threw monster */
#define MONSTER_SPECIFY		7	/* player requested monster */
#define MONSTER_PURGATORY	8	/* encounter in purgatory */

    /* scoreboard constants */
#define SB_KEEP_ABOVE		1000	/* below this level, delete chars */
#define SB_KEEP_FOR		2592000 /* seconds to keep low chars */

    /* other constants */
#define CORPSE_LIFE		2592000 /* seconds corpses stay in game */
#define KEEP_TIME		2592000 /* base secs to keep saved characters */
#define NEWBIE_KEEP_TIME	259200  /* base secs to keep saved characters */
#define ACCOUNT_KEEP_TIME	7776000 /* secs to keep accounts */

/* constants for altering coordinates */
#define A_SPECIFIC      0             /* coordinates specified, non-TP */
#define A_FORCED        1             /* coordinates specified, ignore Beyond */
#define A_NEAR          2             /* coordinates not specified, move near */
#define A_FAR           3             /* coordinates not specified, move far */
#define A_TRANSPORT	4	      /* distant teleport */
#define A_OUST		5	      /* more distant teleport */
#define A_BANISH	6	      /* move player to beyond */
#define A_TELEPORT      7             /* moved by teleport */

    /* spell constants */
#define P_HEAL		0		/* steward heals a player */
#define P_CURE		1		/* council heals with poison cure */
#define P_RESTORE	2		/* valar restores a character */
#define P_CURSE		0		/* steward curse */
#define P_EXECRATE	1		/* king's stronger curse */
#define P_SMITE		2		/* valar decimates a character */

/* constants for spells */
#define ML_ALLORNOTHING 0.0             /* magic level for 'all or nothing' */
#define MM_ALLORNOTHING 1.0             /* mana used for 'all or nothing' */
#define ML_MAGICBOLT    5.0             /* magic level for 'magic bolt' */
#define ML_INCRMIGHT    15.0            /* magic level for 'increase might' */
#define MM_INCRMIGHT    30.0            /* mana used for 'increase might' */
#define ML_HASTE        25.0            /* magic level for 'haste' */
#define MM_HASTE        35.0            /* mana used for 'haste' */
#define ML_FORCEFIELD   35.0            /* magic level for 'force field' */
#define MM_FORCEFIELD   60.0            /* mana used for 'force field' */
#define ML_XPORT        45.0            /* magic level for 'transport' */
#define MM_XPORT        100.0           /* mana used for 'transport' */
#define ML_PARALYZE     60.0            /* magic level for 'paralyze' */
#define MM_PARALYZE     125.0           /* mana used for 'paralyze' */
#define ML_XFORM        75.0            /* magic level for 'transform' */
#define MM_XFORM        150.0           /* mana used for 'transform' */
#define MM_SPECIFY      1000.0          /* mana used for 'specify' */
#define ML_CLOAK        20.0            /* magic level for 'cloak' */
#define MEL_CLOAK       7.0             /* experience level for 'cloak' */
#define MM_CLOAK        35.0            /* mana used for 'cloak' */
#define ML_TELEPORT     40.0            /* magic level for 'teleport' */
#define MEL_TELEPORT    12.0            /* experience level for 'teleport' */
#define MM_INTERVENE    3000.0          /* mana used to 'intervene' */
#define MM_COMMAND      15000.0         /* mana used to 'command' */

/* some miscellaneous constants */
#define N_DAYSOLD       30              /* number of days old for purge */
#define N_AGE           750             /* age to degenerate ratio */
#define N_GEMVALUE      (1000.0)        /* number of gold pieces to gem ratio */
#define N_FATIGUE	50		/* rounds of combat before -1 speed */
#define N_SWORDPOWER    .04             /* percentage of strength swords increase */

#define D_BEYOND        (1.0e6)         /* distance to beyond point of no return */
#define D_EXPER         (2000.0)        /* distance experimentos are allowed */
#define D_EDGE		(100000000.0)	/* edge of the world */
#define D_CIRCLE	125.0		/* distance for each circle */
#define STATELEN	256		/* random number state buffer */
#define MIN_STEWARD     10.0            /* minimum level for steward */
#define MAX_STEWARD     200.0           /* maximum level for steward */
#define MIN_KING        1000.0           /* minimum level for king */
#define MAX_KING        2000.0          /* maximum level for king */

/* hacking constants */
#define H_SYSTEM	0		/* hacking the system */
#define H_PASSWORDS	1		/* hacking passwords */
#define H_CONNECTIONS	2		/* excessive connections */
#define H_KILLING	3		/* killing rampage */
#define H_PROFANITY	4		/* using profanity */
#define H_DISRESPECTFUL 5		/* disrespectful to wizards */
#define H_FLOOD     	6		/* flooding chat */
#define H_SPAM		7		/* spamming chat */
#define H_WHIM		8		/* wizard's whim */

