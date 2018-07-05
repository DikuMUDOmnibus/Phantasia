/*
 * fight.c   Phantasia monster fighting routines
 */

#include "include.h"

/************************************************************************
/
/ FUNCTION NAME: Do_monster(struct client_t *c, struct event_t the_event)
/
/ FUNCTION: monster battle routine
/
/ AUTHOR: E. A. Estes, 2/20/86
/	  Brian Kelly, 5/20/99
/
/ ARGUMENTS:
/       int particular - particular monster to fight if >= 0
/
/ RETURN VALUE: none
/
/ MODULES CALLED: monsthits(), playerhits(), readmessage(), callmonster(),
/       writerecord(), pickmonster(), displaystats(), pow(), cancelmonster(),
/       awardtreasure(), more(), death(), wmove(), setjmp(), drandom(), printw(),
/       longjmp(), wrefresh(), mvprintw(), wclrtobot()
/ / DESCRIPTION: /       Choose a monster and check against some special types.
/       Arbitrate between monster and player.  Watch for either
/       dying.
/
*************************************************************************/

Do_monster(struct client_t *c, struct event_t *the_event)
{
    struct event_t *event_ptr;
    bool firsthit = c->player.blessing;  /* set if player gets the first hit */
    bool monsthit = TRUE;
    int count;
    char string_buffer[SZ_LINE];
    char string_buffer2[SZ_LINE];
    short sTemp;
    double dTemp;

	/* set battle stats */
    c->battle.ring_in_use = FALSE;
    c->battle.tried_luckout = FALSE;
    c->battle.melee_damage = 0.0;
    c->battle.skirmish_damage = 0.0;


	/* create a player opponent */
    c->battle.opponent = (struct opponent_t *) Do_malloc(SZ_OPPONENT);
    c->battle.opponent->sin = 5;

    if (the_event->arg3 >= 0 && the_event->arg3 < NUM_MONSTERS)  {
            /* monster is specified */
        c->battle.opponent->type = the_event->arg3;
    }

    else {

	if (c->player.special_type == SC_VALAR) {

                /* even chance of any monster */
            c->battle.opponent->type = (int) ROLL(0.0, 100.0);
	}

                /* 10% of getting cerbed with big shield */
        else if ((c->player.shield / c->player.degenerated > 50000.0) &&
                 (c->player.circle >= 36) && RND() < .1) {

            c->battle.opponent->type = 96;
        }
                /* 10% of getting jabbed with big quick */
        else if ((c->player.quicksilver / c->player.degenerated > 500.0) &&
                 (c->player.circle >= 36) && RND() < .1) {

            c->battle.opponent->type = 94;

        } else if (c->player.circle >= 36) {

        	/* even chance of all non-water monsters */
            c->battle.opponent->type = (int) ROLL(14.0, 86.0);

        } else if ((c->player.circle == 27) || (c->player.circle == 28)) {

        	/* the cracks of doom - no weak monsters except modnar */
             
            c->battle.opponent->type = (int) (ROLL(50.0, 50.0));

            if (c->battle.opponent->type < 52) {
                c->battle.opponent->type = 15;
            }

        } else if ((c->player.circle < 31) && (c->player.circle > 24)) {

        	/* gorgoroth - no weak monsters except modnar, 
                   weighed towards middle  */
            c->battle.opponent->type = (int) (ROLL(50.0, 25.0) +
		    ROLL(0.0, 26.0));

            if (c->battle.opponent->type < 52) {
                c->battle.opponent->type = 15;
            }

        } else if (c->player.circle > 19) {

                /* the marshes - water monsters, idiots, and modnar */
            c->battle.opponent->type = (int) ROLL(0.0, 17.0);

        } else if (c->player.circle > 15) {

                /* chance of all non-water monsters, weighted toward middle */
            c->battle.opponent->type =
                    (int) (ROLL(0.0, 50.0) + ROLL(14.0, 37.0));

        } else if (c->player.circle > 9) {
        	/* TT 1-8 monsters, weighted toward middle */
            c->battle.opponent->type = 
                    (int) (ROLL(0.0, (-8.0 + c->player.circle * 4)) + 
                           ROLL(14.0, 26.0));
                /*  (int) (ROLL(0.0, (35.0 + c->player.circle)) + 
                           ROLL(14.0, 26.0)); */

        } else if (c->player.circle > 7) {
        	/* Hard type 3-5 monsters */
            c->battle.opponent->type = 
                    (int) ROLL(14.0, (18.0 + c->player.circle * 4));

        } else if (c->player.circle > 4) {
        	/* even chance of type 1-3 + easy type 4-5 monsters */
            c->battle.opponent->type = (int) ROLL(14.0, 46.0);

        } else if (c->player.circle == 4) {
        	/* even chance of all type 1-3 */
            c->battle.opponent->type = (int) ROLL(14.0, 38.0);

        } else {

        	/* even chance of some of the tamest non-water monsters */
            c->battle.opponent->type = 
                (int) ROLL(14.0, (17.0 + c->player.circle * 4));
	}
    }

    if (c->battle.opponent->type == 100) {
            c->battle.opponent->type = 15;
    }

	/* set the monster's size */
    if (the_event->arg2 > 0) {
            /* monster size is specified */
    	c->battle.opponent->size = the_event->arg2;
    } else if ((c->player.circle == 27) || (c->player.circle == 28)) {
            /* cracks and gorgoroth scale with player level */

        if (c->player.ring_type == R_DLREG) {
            c->battle.opponent->size = MAX(c->player.circle, 
                                       (floor((.2 + .3 * RND()) * c->player.level)));
        } else if (c->player.ring_type != R_NONE) {
            c->battle.opponent->size = MAX(c->player.circle, 
                                       (floor((.15 + .35 * RND()) * c->player.level)));
        } else {
            c->battle.opponent->size = MAX(c->player.circle, 
                                       (floor((.1 + .25 * RND()) * c->player.level)));
        }
    } else if ((c->player.circle < 31) && (c->player.circle > 24)) {

        if (c->player.ring_type == R_DLREG) {
            c->battle.opponent->size = MAX(c->player.circle, 
                                       ((.15 + .1 * RND()) * c->player.level));
        } else if (c->player.ring_type != R_NONE) {
            c->battle.opponent->size = MAX(c->player.circle, 
                                       ((.1 + .15 * RND()) * c->player.level));
        } else {
            c->battle.opponent->size = MAX(c->player.circle, 
                                       ((.05 + .1 * RND()) * c->player.level));
        }
    } else {
    	c->battle.opponent->size = c->player.circle;
    }

    Do_lock_mutex(&c->realm->monster_lock);

	/* fill structure */
    strcpy(c->battle.opponent->name,
	    c->realm->monster[c->battle.opponent->type].name);

    strcpy(c->battle.opponent->realName,
	    c->realm->monster[c->battle.opponent->type].name);

    c->battle.opponent->experience = c->battle.opponent->size *
	    c->realm->monster[c->battle.opponent->type].experience;

    c->battle.opponent->brains = c->battle.opponent->size *
	    c->realm->monster[c->battle.opponent->type].brains;

    c->battle.opponent->strength = 
    c->battle.opponent->max_strength = (1.0 + c->battle.opponent->size / 2.0)
	    * c->realm->monster[c->battle.opponent->type].strength;

    /* Randomize energy slightly */
    c->battle.opponent->energy = 
    c->battle.opponent->max_energy = floor(c->battle.opponent->size *
	    c->realm->monster[c->battle.opponent->type].energy * 
            (.9 + RND() * .2));

    c->battle.opponent->speed = 
    c->battle.opponent->max_speed = 
	    c->realm->monster[c->battle.opponent->type].speed;

    c->battle.opponent->special_type = 
	    c->realm->monster[c->battle.opponent->type].special_type;

    c->battle.opponent->treasure_type = 
	    c->realm->monster[c->battle.opponent->type].treasure_type;

    c->battle.opponent->flock_percent = 
	    c->realm->monster[c->battle.opponent->type].flock_percent;

    Do_unlock_mutex(&c->realm->monster_lock);

    c->battle.opponent->shield = 0.0;

	/* handle some special monsters */
    if (c->battle.opponent->special_type == SM_MODNAR) {

        if (c->player.special_type < SC_COUNCIL) {

                /* randomize some stats */
            c->battle.opponent->strength *= RND() + 0.5;
            c->battle.opponent->brains *= RND() + 0.5;
            c->battle.opponent->speed *= RND() + 0.5;
            c->battle.opponent->energy *= RND() + 0.5;
            c->battle.opponent->experience *= RND() + 0.5;

            c->battle.opponent->treasure_type =
		    (int) ROLL(0.0, (double) c->battle.opponent->treasure_type);
        }
        else {

                /* make Modnar into Morgoth */
            strcpy(c->battle.opponent->name, "Morgoth");
            strcpy(c->battle.opponent->realName, "Morgoth");
            c->battle.opponent->special_type = SM_MORGOTH;

            c->battle.opponent->energy = c->battle.opponent->max_energy = 
		    floor((8 + (c->player.level / 250)) * 
                          (c->player.strength * 
                           (1 + sqrt(c->player.sword) * N_SWORDPOWER)) *
                          (.75 + RND() * .5));

            c->battle.opponent->strength = c->battle.opponent->max_strength = 
		    floor((.025 + RND() * 
                          ((.02 + .05 * c->player.level/10000))) * 
                         (c->player.max_energy + c->player.shield));

	    if (c->player.special_type == SC_EXVALAR) {
              c->battle.opponent->speed = 1;
	    }
	    else if (RND() < .5) {
              c->battle.opponent->speed = 1 + 
                              RND() * ((c->player.level - 2500) / 7500);
            } else {
              c->battle.opponent->speed = 1 - 
                              RND() * ((c->player.level - 2500) / 7500);
            }

            c->battle.opponent->speed *= c->player.max_quickness +
		    sqrt(c->player.quicksilver) - c->battle.opponent->size *
		    .0005;

	    
				/* Morgie gets faster as you go on to counter stat balancing */
			if((c->player.level >= 3000) && (c->player.level < 4000)){

				c->battle.opponent->max_speed = c->battle.opponent->speed;
	
			} else if((c->player.level >= 4000) && (c->player.level < 5000)){

				c->battle.opponent->max_speed = c->battle.opponent->speed*1.05;
	
			} else if((c->player.level >= 5000) && (c->player.level < 6000)){

				c->battle.opponent->max_speed = c->battle.opponent->speed*1.1;
	
			} else if((c->player.level >= 6000) && (c->player.level < 7000)){

				c->battle.opponent->max_speed = c->battle.opponent->speed*1.15;
	
			} else if((c->player.level >= 7000) && (c->player.level < 8000)){

				c->battle.opponent->max_speed = c->battle.opponent->speed*1.2;
	
			} else if((c->player.level >= 8000) && (c->player.level < 9000)){

				c->battle.opponent->max_speed = c->battle.opponent->speed*1.25;
	
			} else if((c->player.level >= 9000) && (c->player.level < 10000)){

				c->battle.opponent->max_speed = c->battle.opponent->speed*1.3;
	
			} else if(c->player.level >= 10000){

				c->battle.opponent->max_speed = c->battle.opponent->speed*1.35;
			}

            c->battle.opponent->brains = c->player.brains * 20;
            c->battle.opponent->flock_percent = 0.0;

                /* Morgoth drops gold to annoy players */
            c->battle.opponent->treasure_type = 7;
            c->battle.opponent->experience = floor(c->battle.opponent->energy
		    / 4);

        }
    }

    else if (c->battle.opponent->special_type == SM_MIMIC) {

            /* pick another name */
        while (!strcmp(c->battle.opponent->name, "A Mimic")) {

            strcpy(c->battle.opponent->name,
	        c->realm->monster[(int) ROLL(0.0, 100.0)].name);

            if (!strcmp(c->battle.opponent->name, "A Succubus") 
                && (c->player.gender == FEMALE)) {
                strcpy(c->battle.opponent->name, "An Incubus");
            }

            firsthit = TRUE;
        }
    }

    else if ((c->battle.opponent->special_type == SM_SUCCUBUS) &&
             (c->player.gender == FEMALE)) {

            /* females should be tempted by incubi, not succubi */
        strcpy(c->battle.opponent->name, "An Incubus");
        strcpy(c->battle.opponent->realName, "An Incubus");
	        
    }

    Do_send_clear(c);

        /* cannot see monster if blind */
    if (c->player.blind) {
        strcpy(c->battle.opponent->name, "A monster");
        the_event->arg1 = (double) MONSTER_RANDOM;
    }

    if (c->battle.opponent->special_type == SM_UNICORN) {

        if (c->player.virgin) {
          if (RND() < c->player.sin - .1) {

            sprintf(string_buffer,
                    "%s glares at you and gallops away with your virgin!\n", 
                    c->battle.opponent->name);

	    Do_send_line(c, string_buffer);

            Do_virgin(c, FALSE, FALSE);
	    Do_cancel_monster(c->battle.opponent);
          } else {
           
            sprintf(string_buffer,
		    "You just subdued %s, thanks to the virgin.\n",
		    c->battle.opponent->name);

	    Do_send_line(c, string_buffer);

            Do_virgin(c, FALSE, FALSE);
	    c->battle.opponent->energy = 0.0;
          }
	}
        else if (! c->player.blind) {

            sprintf(string_buffer, "You just saw %s running away!\n",
		    c->battle.opponent->name);

	    Do_send_line(c, string_buffer);

	    Do_cancel_monster(c->battle.opponent);
        } else {
            sprintf(string_buffer, "You just heard %s running away!\n",
		    c->battle.opponent->name);

	    Do_send_line(c, string_buffer);

	    Do_cancel_monster(c->battle.opponent);
        }
    }
    else {

        if (c->battle.opponent->special_type == SM_MORGOTH) {

            sprintf(string_buffer,
		    "You've encountered %s, Bane of the Council and Enemy of the Vala.\n",
            	    c->battle.opponent->name);

	    Do_send_line(c, string_buffer);

	}
    }

       /* overpower Dark Lord with blessing and charms or if superchar */
    if (c->battle.opponent->special_type == SM_DARKLORD) {
      if (c->player.blessing) {
        if (c->player.charms >= floor(c->battle.opponent->size 
                                      * (.8 - .1 * c->player.type)) + 1) {

          sprintf(string_buffer, "You just overpowered %s!\n",
		c->battle.opponent->name);

          Do_send_line(c, string_buffer);


          /* have dark lord take a percentage of charms 
             if player is carrying way too many charms */
          c->player.charms -= MAX(floor(c->battle.opponent->size 
                                  * (.8 - .1 * c->player.type)), 
                                  floor((.35 - (.05 * c->player.type)) * c->player.charms));

          if (c->player.charms < 0) {
              c->player.charms = 0;
          }
  
	  c->battle.opponent->energy = 0.0;
  
	  sprintf(string_buffer, "%s has just defeated %s!\n",
		  c->modifiedName, c->battle.opponent->name);
  
	  Do_broadcast(c, string_buffer);
        }
      }
    }

	/* give this new monster the proper introduction */
    if (c->battle.opponent->energy > 0) {

        switch ((int)the_event->arg1) {

        case MONSTER_RANDOM:
        default:

	    sprintf(string_buffer, "You are attacked by %s.",
		    c->battle.opponent->name);
	    break;

        case MONSTER_CALL:

	    sprintf(string_buffer, "You find and attack %s.",
		    c->battle.opponent->name);
	    break;

        case MONSTER_FLOCKED:

	    sprintf(string_buffer, "%s's friend appears and attacks.",
		    c->battle.opponent->name);
	    break;

        case MONSTER_SHRIEKER:

	    sprintf(string_buffer, "%s responds to the shrieker's clamor.",
		    c->battle.opponent->name);
	    break;

        case MONSTER_JABBERWOCK:

	    sprintf(string_buffer, "The Jabberwock summons %s.",
		    c->battle.opponent->name);
	    break;

        case MONSTER_TRANSFORM:

	    sprintf(string_buffer, "%s now attacks.",
		    c->battle.opponent->name);
	    break;

        case MONSTER_SUMMONED:
        case MONSTER_SPECIFY:

	    sprintf(string_buffer, "%s appears and attacks.",
		    c->battle.opponent->name);
	    break;

	case MONSTER_PURGATORY:

	    sprintf(string_buffer, "%s appears and attacks.",
		    c->battle.opponent->name);

		/* Purgatory characters always get first attack since they
		had the turn when they disconnected */
	    firsthit = TRUE;

		/* Assume they already lucked out */
	    c->battle.tried_luckout = TRUE;

                /* prevent players from rerolling monster size */
            if ((c->player.circle < 31) && (c->player.circle > 24)) {
                Do_send_line(c, "The monster appears to have grown stronger in your absence!\n");
                c->battle.opponent->size = MAX(c->player.circle, .5 * c->player.level);
            }
	    break;
	}

            /* print header, and arbitrate between player and monster */
        sprintf(string_buffer2, " -  (Size: %.0f)\n",
		c->battle.opponent->size);

        strcat(string_buffer, string_buffer2);
        Do_send_line(c, string_buffer);
    }

	/* throw up spells for this monster if not Morgoth */
    if (c->battle.opponent->special_type != SM_MORGOTH) {
        Do_starting_spells(c);
    }

	/* allow experimentos to test the speed equation */
    if (c->battle.opponent->special_type == SM_MORON) {
        if (c->player.type == C_EXPER && c->player.level == 0) {
            c->battle.opponent->speed = 
            c->battle.opponent->max_speed = 
            c->player.quickness;

            Do_send_line(c, "Recognizing a kindred spirit, A Moron now moves as quickly as you!\n");

            Do_send_line(c, "You feel fully refreshed and healed for battle!\n");

            Do_energy(c, c->player.max_energy + c->player.shield,
                      c->player.max_energy, c->player.shield,
                      c->battle.force_field, FALSE);
            c->battle.rounds = 0;
        }
    }

        /* adjust equipment-stealing monsters to knock off unbalanced chars */
    if ((c->battle.opponent->special_type == SM_CERBERUS) &&
        (c->player.shield / c->player.degenerated > 100000.0) &&
        (c->player.degenerated < 50)) {
            c->battle.opponent->speed = c->player.max_quickness * 4;
            c->battle.opponent->max_speed = c->battle.opponent->speed;
            Do_send_line(c, "Cerberus's eyes flare brightly as he sees the immense amounts of metal you are carrying!\n");
    }

        /* adjust equipment-stealing monsters to knock off unbalanced chars */
    if ((c->battle.opponent->special_type == SM_JABBERWOCK) &&
        (c->player.quicksilver / c->player.degenerated > 500.0) &&
        (c->player.degenerated < 50)) {
         c->battle.opponent->speed = c->player.max_quickness * 4;
         c->battle.opponent->max_speed = c->battle.opponent->speed;
            Do_send_line(c, "A Jabberwock whiffles in delight as it sees your immense stash of quicksilver!\n");
    }

    if ((c->battle.opponent->special_type == SM_DARKLORD) && (c->player.blessing) && (c->battle.opponent->energy > 0) && (c->morgothCount == 0)) {
            Do_send_line(c, "Your blessing is consumed by the great evil of your opponent!\n");
            Do_blessing(c, FALSE, FALSE);
    }
    
    if ((c->battle.opponent->special_type == SM_DARKLORD) &&
    	(c->morgothCount > 0)&&(c->battle.opponent->energy > 0)) {
    		Do_send_line(c, "The Dark Lord keeps a safe distance away from you, smelling the scent of its masters blood upon your sword!\n");
    }
    		

    if ((c->player.ring_type != R_NONE) &&
        (c->battle.opponent->special_type != SM_IT_COMBAT) &&
        ((RND() * 1000 / c->battle.opponent->treasure_type) < c->player.sin)) {

            Do_send_line(c, "You feel compelled to put your ring on your finger!\n");
            c->battle.ring_in_use = TRUE;

	    sprintf(string_buffer, "%s, %s, forced_ring.\n",
		    c->player.lcname,
		    c->realm->charstats[c->player.type].class_name);

            Do_sin(c, 0.1);

	    Do_log(COMBAT_LOG, &string_buffer);
    } 
        

    while (c->battle.opponent->energy > 0.0) {

            /* allow paralyzed monster to wake up */
        c->battle.opponent->speed = MIN(c->battle.opponent->speed + 1.0,
		c->battle.opponent->max_speed);

            /* check if blessing gives first hit -- monster has to win
               initiative extra times depending on sin level -- 
               a sinless player always wins first strike */
               
        if ((firsthit) && (c->player.sin > 0)) {

            monsthit = TRUE;

            for (count = 0; count < (6 - floor((c->player.sin + 1.0) / 1.25)); count++) {
                monsthit &= (RND() * c->battle.opponent->speed > 
                             RND() * c->player.quickness);
            }

            if ((monsthit == TRUE) && (c->player.quickness > 0)) {
                firsthit = FALSE;
            }
        }
                
            /* monster is faster */
        if (RND() * c->battle.opponent->speed >
		(RND() * c->player.quickness)
	            /* not darklord */
	        && c->battle.opponent->special_type != SM_DARKLORD
            	    /* not shrieker */
                && c->battle.opponent->special_type != SM_SHRIEKER
            	    /* not mimic */
                && c->battle.opponent->special_type != SM_MIMIC
                    /* not first attack with a blessing */
                && !firsthit) {

                /* monster gets a hit */
            Do_monsthits(c);

	}
            /* player gets a hit */
        else {

            firsthit = FALSE;

              /* if the player has a good ring in use */
            if (c->battle.ring_in_use) {

                    /* age ring */
                if (c->player.ring_type != R_DLREG) {
                    --c->player.ring_duration;
                }

                    /* regardless of ring type, heal the character */
                Do_energy(c, c->player.max_energy + c->player.shield,
                        c->player.max_energy, c->player.shield,
                        c->battle.force_field, FALSE);
            }

            Do_playerhits(c, &sTemp, &dTemp);
        }

            /* player died */
        if ((c->player.energy <= 0.0) || (c->player.strength <= 0.0)) {

            event_ptr = (struct event_t *) Do_create_event();
	    event_ptr->type = DEATH_EVENT;
	    event_ptr->arg1 = (double) c->battle.ring_in_use;

		/* check if player died from greed or fatigue */
	    if (c->player.quickness == 0) {
              if (c->battle.rounds >= c->player.max_quickness * N_FATIGUE) {
                event_ptr->arg3 = K_FATIGUE;
              } else {
	        event_ptr->arg3 = K_GREED;
              }
	    }
	    else {
	        event_ptr->arg3 = K_MONSTER;
	    }

	    event_ptr->arg4 = (void *) Do_malloc(
		    strlen(c->battle.opponent->realName) + 1);

	    strcpy(event_ptr->arg4, c->battle.opponent->realName);

                /* fight ends even if the player is saved from death */
            Do_cancel_monster(c->battle.opponent);

		/* log the loss */
            sprintf(string_buffer, "%s killed_by size %.0lf %s\n",
		    c->player.lcname, c->battle.opponent->size,
		    c->battle.opponent->realName);

	    Do_log(BATTLE_LOG, &string_buffer);

	    Do_handle_event(c, event_ptr);

        }

	    /* give the character fatigue */
        ++c->battle.rounds;

        Do_speed(c, c->player.max_quickness, c->player.quicksilver,
	        c->battle.speedSpell, FALSE);
    }

        /* give player credit for killing monster */
    if (c->battle.opponent->experience) {
	Do_experience(c, c->battle.opponent->experience, FALSE);

	    /* log the victory */
        sprintf(string_buffer, "%s killed size %.0lf %s\n",
		c->player.lcname, c->battle.opponent->size,
		c->battle.opponent->realName);

	Do_log(BATTLE_LOG, &string_buffer);
    }
    else if (c->player.energy > 0.0) {

	    /* log no_victor */
        sprintf(string_buffer, "%s survived_with size %.0lf %s\n",
		c->player.lcname, c->battle.opponent->size,
		c->battle.opponent->realName);

	Do_log(BATTLE_LOG, &string_buffer);
    }

    Do_more(c);
    Do_send_clear(c);

        /* monster flocks */
    if (RND() < c->battle.opponent->flock_percent / 100.0) {

        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = MONSTER_EVENT;
	event_ptr->arg1 = MONSTER_FLOCKED;
	event_ptr->arg2 = the_event->arg2;
        event_ptr->arg3 = c->battle.opponent->type;
        Do_file_event_first(c, event_ptr);
        /* last fight in sequence, remove timeout penalty */ 
    } else {
        c->battle.timeouts = 0;
    }

        /* monster has treasure */
    if (c->battle.opponent->treasure_type > 0.0 && 
        (RND() > 0.2 + pow(0.4, (double) (c->battle.opponent->size / 3.0)) || 
         c->battle.opponent->special_type == SM_UNICORN) ) {

        event_ptr = (struct event_t *) Do_create_event();
        event_ptr->type = TREASURE_EVENT;
	event_ptr->arg1 = c->battle.opponent->size;

        /* unicorns will always drop trove or pick scrolls */
        if (c->battle.opponent->special_type == SM_UNICORN) {
            if (c->player.level <= 100 - c->player.gems) {
                event_ptr->arg2 = 1;
            } else {
                event_ptr->arg2 = 5;
            }
        }

        event_ptr->arg3 = c->battle.opponent->treasure_type;
        Do_file_event(c, event_ptr);
    }

    if (c->player.ring_duration <= 0) {
	if (c->player.ring_type == R_NAZREG) {

	    Do_ring(c, R_NONE, FALSE);

	    Do_send_clear(c);
	    Do_send_line(c, "Your ring vanishes!\n");
	    Do_more(c);
	}
	else if (c->player.ring_type == R_BAD) {

	    Do_ring(c, R_SPOILED, FALSE);
	}
	else if (c->player.ring_type == R_SPOILED) {

	    event_ptr = (struct event_t *)Do_create_event();
	    event_ptr->type = DEATH_EVENT;
	    event_ptr->arg3 = K_RING;
	    Do_file_event(c, event_ptr);
	}
    }

	/* remove player bonuses */
    Do_energy(c, c->player.energy, c->player.max_energy, c->player.shield,
	0, FALSE);

    Do_strength(c, c->player.max_strength, c->player.sword, 0, FALSE);
    Do_speed(c, c->player.max_quickness, c->player.quicksilver, 0, FALSE);

    c->battle.force_field = 0;
    c->battle.strengthSpell = 0;
    c->battle.speedSpell = 0;

	/* destroy the opponent structure */
    free((void *) c->battle.opponent);

}


/************************************************************************
/
/ FUNCTION NAME: Do_playerhits(struct client_t *c)
/
/ FUNCTION: prompt player for action in monster battle, and process
    c->battle.ring_in_use = FALSE;
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/23/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: hitmonster(), throwspell(), inputoption(), cancelmonster(),
/       floor(), wmove(), drandom(), altercoordinates(), waddstr(), mvprintw(),
/       wclrtoeol(), wclrtobot()
/
/ DESCRIPTION:
/       Process all monster battle options.
/
*************************************************************************/

Do_playerhits(struct client_t *c, short *theAttack, double *theArg)
{
    struct button_t theButtons;
    struct event_t *event_ptr;
    double  might, inflict, dtemp;        /* damage inflicted */
    char string_buffer[SZ_LINE];
    char error_msg[SZ_ERROR_MESSAGE];
    long answer;
    int rc;
    int temp;
    int wizEvade = 1;

        /* see if the game is shutting down */
    Do_shutdown_check(c);

    strcpy(theButtons.button[0], "Melee\n");
    strcpy(theButtons.button[1], "Skirmish\n");
    Do_clear_buttons(&theButtons, 2);
    if (c->player.sword != 0) {
        strcpy(theButtons.button[2], "Nick\n");
    }
    strcpy(theButtons.button[3], "Spell\n");
    strcpy(theButtons.button[4], "Rest\n");
    Do_clear_buttons(&theButtons, 5);
    strcpy(theButtons.button[6], "Evade\n");

        /* haven't tried to luckout yet */
    if (!c->battle.tried_luckout)
            /* cannot luckout against Morgoth */
        if (c->battle.opponent->special_type == SM_MORGOTH)
	    strcpy(theButtons.button[5], "Ally\n");
        else
	    strcpy(theButtons.button[5], "Luckout\n");

        /* player has a ring */
    if (c->player.ring_type != R_NONE)
	strcpy(theButtons.button[7], "Use Ring\n");

    rc = Do_buttons(c, &answer, &theButtons);

	/* if player has dropped */
    if (rc == S_ERROR) {

	    /* evade if in inter-player combat */
        if (c->battle.opponent->special_type == SM_IT_COMBAT) {
	    answer = 5;
	}
	else {

	        /* throw the character into purgatory */
	    c->player.purgatoryFlag = TRUE;
	    c->player.monsterNumber = c->battle.opponent->type;

		/* cancel the monster and save the game */
            Do_cancel_monster(c->battle.opponent);
	    c->run_level = SAVE_AND_EXIT;
	    return;
	}
    }

    else if (rc == S_TIMEOUT || rc == S_CANCEL) {
	Do_age(c);

                /* these monsters would never attack on timeouts */
	if (RND() < .25 && 
	    c->battle.opponent->special_type != SM_DARKLORD &&
            c->battle.opponent->special_type != SM_SHRIEKER) {

 		Do_monsthits(c);
	}
	*theAttack = IT_PASS;

	sprintf(string_buffer, "%s, %s, timeout.\n",
		c->player.lcname, 
		c->realm->charstats[c->player.type].class_name);

	    /* give the character fatigue from timeouts */
        ++c->battle.timeouts;
        if (c->battle.timeouts < N_FATIGUE * 4) {
            c->battle.timeouts *= 2;
        } else if ((c->battle.opponent->special_type == SM_DARKLORD) ||
                   (c->battle.opponent->special_type == SM_SHRIEKER)) {
            Do_send_line(c, "It wandered off!\n");

            Do_cancel_monster(c->battle.opponent);
	    *theAttack = IT_EVADED;

            sprintf(string_buffer, "%s, %s, evade.\n", c->player.lcname,
                    c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
        }

        c->battle.rounds += c->battle.timeouts;

	Do_log(COMBAT_LOG, &string_buffer);
	return;
    }

    switch (answer) {

    case 0:               /* melee */
            /* melee affects monster's energy and strength */
	might = c->player.strength * (1 + sqrt(c->player.sword) * N_SWORDPOWER) 
                + c->battle.strengthSpell;

	    /* use of ring doubles a player's might */
	if (c->battle.ring_in_use) {
	    might *= 2;
	}

        inflict = floor((.5 + 1.3 * RND()) * might);

        c->battle.melee_damage += inflict;

        c->battle.opponent->strength = c->battle.opponent->max_strength
                - (c->battle.melee_damage / c->battle.opponent->max_energy)
                * (c->battle.opponent->max_strength / 3.0);

        Do_hitmonster(c, inflict);
	*theAttack = IT_MELEE;
	*theArg = inflict;

	sprintf(string_buffer, "%s, %s, melee %.0lf.\n", c->player.lcname,
                c->realm->charstats[c->player.type].class_name, inflict);

	Do_log(COMBAT_LOG, &string_buffer);

        break;

    case 1:               /* skirmish */
	might = c->player.strength * (1 + sqrt(c->player.sword) * N_SWORDPOWER) 
                + c->battle.strengthSpell;

	    /* use of ring doubles a player's might */
	if (c->battle.ring_in_use) {
	    might *= 2;
	}

            /* skirmish affects monster's energy and speed */
        inflict = floor((.33 + 1.1 * RND()) * might);

        c->battle.skirmish_damage += inflict;

        c->battle.opponent->speed = c->battle.opponent->max_speed
                - (c->battle.skirmish_damage / c->battle.opponent->max_energy)
                * (c->battle.opponent->max_speed / 3.0);

        Do_hitmonster(c, inflict);
	*theAttack = IT_SKIRMISH;
	*theArg = inflict;

        sprintf(string_buffer, "%s, %s, skirmish %.0lf.\n", c->player.lcname,
                c->realm->charstats[c->player.type].class_name, inflict);

	Do_log(COMBAT_LOG, &string_buffer);

        break;

    case 6:               /* evade */

	    /* in itcombat, give the player a bonus to evade */
        if (c->battle.opponent->special_type == SM_IT_COMBAT) {

	    dtemp = 6;

            /* huge bonus to wizards and apprentices */
            if (c->wizard > 0) {
                wizEvade = 0;
            } 

	}
	else {
	    dtemp = 1;
	}

            /* use brains and speed to try to evade */
        if ((c->battle.opponent->special_type != SM_MIMIC || RND() < .05) &&
		    /* cannot run from mimic very effectively */
		((c->battle.opponent->special_type == SM_DARKLORD ||
		  c->battle.opponent->special_type == SM_SHRIEKER) ||
                    /* can always run from D. L. and shrieker */
                RND() * (c->player.quickness * 1.1 + 1) * (c->player.brains + 1) *
		dtemp > RND() * (c->battle.opponent->speed * 1.1 + 1) *
		(c->battle.opponent->brains + 1) * wizEvade )) {

            if (c->battle.opponent->special_type == SM_MORGOTH) {
                Do_send_line(c, "Morgoth cackles evilly as you run away.  You feel as if you have done something terribly wrong!\n");
                Do_sin(c, 3.0);
            } else if ((c->battle.opponent->type == 71) && 
                       (! c->player.blind))  {
                Do_send_line(c, "You shun the frumious Bandersnatch!\n");
            } else if (wizEvade == 0) {
                Do_send_line(c, "You magically whisk yourself away!\n");
            } else {
                Do_send_line(c, "You got away!\n");
            }

            Do_cancel_monster(c->battle.opponent);

	    event_ptr = (struct event_t *) Do_create_event();
	    event_ptr->type = MOVE_EVENT;
	    event_ptr->arg3 = A_NEAR;

	    Do_file_event(c, event_ptr);

            if (wizEvade == 0) {
	        *theAttack = IT_WIZEVADE;
                sprintf(string_buffer, "%s, %s, wiz_evade.\n", c->player.lcname,
                    c->realm->charstats[c->player.type].class_name);
            } else {
	        *theAttack = IT_EVADED;
                sprintf(string_buffer, "%s, %s, evade.\n", c->player.lcname,
                    c->realm->charstats[c->player.type].class_name);
            }

	    Do_log(COMBAT_LOG, &string_buffer);
        }
        else {

            sprintf(string_buffer, "%s is still after you!\n",
		    c->battle.opponent->name);

	    Do_send_line(c, string_buffer);
	    *theAttack = IT_NO_EVADE;

            sprintf(string_buffer, "%s, %s, no_evade.\n", c->player.lcname,
                    c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	}
        break;

    case 3:               /* magic spell */
        Do_throwspell(c, theAttack, theArg);
        break;

    case 2:               /* nick */

            /* Morgoth can't be nicked */
        if (c->battle.opponent->special_type == SM_MORGOTH) {
            sprintf(string_buffer, "You attempt to nick %s, but your sword bounces off him!\n",
		    c->battle.opponent->name, inflict);

	    Do_send_line(c, string_buffer);
            inflict = 0;
        } else {
                /* hit 1 plus sword; give some experience */
            *theArg = floor(c->battle.opponent->experience / 10.0);
            c->battle.opponent->experience -= *theArg;

            Do_experience(c, *theArg, FALSE);

            inflict = 1.0 + c->player.sword;

                /* monster gets meaner */
            c->battle.opponent->max_speed += 2.0;
            c->battle.opponent->speed = (c->battle.opponent->speed < 0.0) ?
                    0.0 : c->battle.opponent->speed + 2.0;

                /* Dark Lord; doesn't like to be nicked */
            if (c->battle.opponent->special_type == SM_DARKLORD) {

                sprintf(string_buffer, "You nicked %s for %.0f damage!  That wasn't a very good idea!\n",
                        c->battle.opponent->name, inflict);

                Do_send_line(c, string_buffer);

                Do_speed(c, c->player.max_quickness / 2.0, c->player.quicksilver,
                        c->battle.speedSpell, FALSE);

                Do_cancel_monster(c->battle.opponent);

                event_ptr = (struct event_t *) Do_create_event();
                event_ptr->type = MOVE_EVENT;
                event_ptr->arg3 = A_FAR;
                Do_file_event(c, event_ptr);
            }
            else {
                Do_hitmonster(c, inflict);
            }
        }

	*theAttack = IT_NICKED;

        sprintf(string_buffer, "%s, %s, nick %.0lf.\n", c->player.lcname,
                c->realm->charstats[c->player.type].class_name, inflict);

	Do_log(COMBAT_LOG, &string_buffer);

        break;

    case 5:       /* luckout */

        if (!c->battle.tried_luckout) {

            c->battle.tried_luckout = TRUE;

                /* Morgoth; ally */
            if (c->battle.opponent->special_type == SM_MORGOTH) {

                if (RND() < c->player.sin / 25.0) {

                    sprintf(string_buffer, "%s accepted!\n",
			    c->battle.opponent->name);

		    Do_send_line(c, string_buffer);
                    Do_cancel_monster(c->battle.opponent);
		    Do_sin(c, 7.5);

	            sprintf(string_buffer, "%s, %s, ally.\n",
		    c->player.lcname,
		    c->realm->charstats[c->player.type].class_name);

		    Do_log(COMBAT_LOG, &string_buffer);
                }
                else 
                    Do_send_line(c, "Nope, he's not interested.\n");
		    Do_more(c);

	            sprintf(string_buffer, "%s, %s, no_ally.\n",
		    c->player.lcname,
		    c->realm->charstats[c->player.type].class_name);

		    Do_log(COMBAT_LOG, &string_buffer);
	    }

		/* routines for inter-player combat */
            else if (c->battle.opponent->special_type == SM_IT_COMBAT) {

                    /* use brains for success with a major handicap */
                if (RND() * c->player.brains < RND() *
                        c->battle.opponent->brains * 10) {

                    sprintf(string_buffer, "You blew it, %s.\n",
			    c->modifiedName);

		    Do_send_line(c, string_buffer);
	            *theAttack = IT_NO_LUCKOUT;

	            sprintf(string_buffer, "%s, %s, no_luckout.\n",
			    c->player.lcname,
                	    c->realm->charstats[c->player.type].class_name);

		    Do_log(COMBAT_LOG, &string_buffer);
		}
                else {
                    Do_send_line(c, "You made it!\n");
    		    c->battle.opponent->energy = 0;
	            *theAttack = IT_LUCKOUT;

		    sprintf(string_buffer, "%s, %s, luckout.\n",
			    c->player.lcname,
			    c->realm->charstats[c->player.type].class_name);

		    Do_log(COMBAT_LOG, &string_buffer);
                }
	    }
            else {
                    /* normal monster; use brains for success */
                if (c->battle.opponent->special_type == SM_DARKLORD ||
			RND() * c->player.brains < RND() *
                        c->battle.opponent->brains) {

                    sprintf(string_buffer, "You blew it, %s.\n",
			    c->modifiedName);

		    Do_send_line(c, string_buffer);
	            *theAttack = IT_NO_LUCKOUT;

	            sprintf(string_buffer, "%s, %s, no_luckout.\n",
			    c->player.lcname,
                	    c->realm->charstats[c->player.type].class_name);

		    Do_log(COMBAT_LOG, &string_buffer);
		}
                else {
                    Do_send_line(c, "You made it!\n");
    		    c->battle.opponent->energy = 0;
	            *theAttack = IT_LUCKOUT;

		    sprintf(string_buffer, "%s, %s, luckout.\n",
			    c->player.lcname,
			    c->realm->charstats[c->player.type].class_name);

		    Do_log(COMBAT_LOG, &string_buffer);
                }
            }
        }
        break;

    case 7:               /* use ring */

        if (c->player.ring_type != R_NONE) {

            Do_send_line(c, "Now using ring.\n");
            c->battle.ring_in_use = TRUE;
	    *theAttack = IT_RING;

            Do_sin(c, 0.1);

	    sprintf(string_buffer, "%s, %s, ring.\n",
		    c->player.lcname,
		    c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
        }
	else {
	    *theAttack = IT_NO_RING;

	    sprintf(string_buffer, "%s, %s, no_ring.\n",
		    c->player.lcname,
		    c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
        }
        break;

    case 4:		   /* rest */

	*theAttack = IT_PASS;

	sprintf(string_buffer, "%s, %s, pass.\n",
		c->player.lcname,
		c->realm->charstats[c->player.type].class_name);

	Do_log(COMBAT_LOG, &string_buffer);

        if (c->battle.opponent->special_type == SM_IT_COMBAT) {
            sprintf(string_buffer, "You attempt to rest, but your opponent prevents you!\n");
	    Do_send_line(c, string_buffer);

	    Do_age(c);
          
        } else {
	    Do_rest(c);
	    Do_age(c);

            /* and just to keep life interesting */
	    Do_monsthits(c);

            /* extra hit for morgoth */
            if (c->battle.opponent->special_type == SM_MORGOTH) {
              c->battle.opponent->strength *= 1.1;
	      Do_monsthits(c);
              sprintf(string_buffer, "You rest to catch your breath, but Morgoth seems to grow stronger!\n");
	      Do_send_line(c, string_buffer);
            }
	}

	break;

    default: 

        sprintf(error_msg, "[%s] Returned non-option in Do_playerhits.\n",
                c->connection_id);

        Do_log_error(error_msg);
	Do_caught_hack(c, H_SYSTEM);
        break;

    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_monsthits(struct client_t *c)
/
/ FUNCTION: process a monster hitting the player
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/23/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: cancelmonster(), scramblestats(), more(), floor(), wmove(),
/       drandom(), altercoordinates(), longjmp(), waddstr(), mvprintw(),
/       getanswer()
/
/ DESCRIPTION:
/       Handle all special monsters here.  If the monster is not a special
/       one, simply roll a hit against the player.
/
*************************************************************************/

Do_monsthits(struct client_t *c)
{
    struct event_t *event_ptr;
    double  inflict = -1;                /* damage inflicted */
    char ch, string_buffer[SZ_LINE];
    long theAnswer;
    float ftemp;
    double dtemp, dtemp2;
    
    static  char    *moronmesg[] =
        /* add more messages at front, if desired */
        /* last two messages for females only */
        {
        "A Moron queries, 'what do u need to do to b a apprentice? I play for two years'\n",
        "A Moron remembers, 'in the good old days I ruled the realm, I had  a MANs sword'\n",
        "A Moron threatens, 'dont mess with me man, I a l33t haxxor you n00b!'\n",
        "A Moron complains, 'aargh the lag! can u reset the server?'\n",
        "A Moron complains, 'this game needs cheats!'\n",
        "A Moron begs, 'steward can i please have 5k?  ill pay you back.'\n",
        "A Moron complains, 'this game is too hard!'\n",
        "A Moron complains, 'this game is too easy!'\n",
        "A Moron complains, 'this game sucks!'\n",
        "A Moron snarls, 'i hate the changes.  why cant they bring back the old version?'\n",
        "A Moron grumbles, 'wizards never do anything.  why dont they add some pics?'\n",
        "A Moron queries, 'where do i buy stuff?'\n",
        "A Moron whimpers, 'how do I get rid of plague?'\n",
        "A Moron boasts, 'i have a level 8k char, just you wait!'\n",
        "A Moron wonders, 'what do i do with a virgin?\n",
        "A Moron squeals, 'ooh a smurf how cute!\n",
        "A Moron howls, 'but i don't want to read the rules!'\n",
        "A Moron asks, 'how come morons never run out?\n",
        "A Moron snivels, 'why is everything cursed?  this curse rate is too high!'\n",
        "A Moron whines, 'how come a Troll hit me 5 times in a row?  it must be a bug!'\n",
        "A Moron yells, 'HEY ALL COME CHEK OUT MY KNEW GAME!'\n",
        "A Moron slobbers, 'please make me an apprentice please please please'\n",
        "A Moron grouches, 'all the apprentices are power-hungry maniacs'\n",
        "A Moron asserts, 'I'm not a liar, honest!'\n",
        "A Moron sings, 'i love you!  you love me!'\n",
        "A Moron exclaims, 'But I didn't MEAN to kill you!'\n",
        "A Moron curses, 'smurfing smurf smurf!  why can't i swear?'\n",
		"A Moron demands, 'i want a bank to store gold so i can get a bigger shield!'\n",
        "A Moron bawls, 'Waa!  My mommy died!  Hey, will u make me apprentice?'\n",
        "A Moron warns, 'my dad is a wizard, don't mess with me!'\n",
        "A Moron leers, 'hey baby what's your sign?'\n",
        "A Moron drools, 'If I had a pick scroll, I'd pick you!  What's your number?'\n"
        };


    if (c->player.quickness <= 0) {
	/* kill a player at speed 0 with no network - saves time */
	/* also kill a player at speed 0 once monster inflicts 3 rounds 
           of fatigue so that people won't have to sit forever */
        if (!c->socket_up || c->battle.rounds > N_FATIGUE * 3) {
            inflict = c->player.energy + c->battle.force_field + 1.0;
            c->battle.opponent->special_type = SM_NONE;
        }
    }

        /* may be a special monster */
    switch (c->battle.opponent->special_type) {

    case SM_DARKLORD:

            /* hits just enough to kill player */
        inflict = c->player.energy + c->battle.force_field + 1.0;

	break;

    case SM_SHRIEKER:
	event_ptr = (struct event_t *) Do_create_event();
	event_ptr->type = MONSTER_EVENT;
	event_ptr->arg1 = MONSTER_SHRIEKER;

        if (c->player.shield / c->player.degenerated > 100000.0) {
            event_ptr->arg3 = 96;
        } else if (c->player.quicksilver / c->player.degenerated > 500.0) {
            event_ptr->arg3 = 94;
        } else {
            event_ptr->arg3 = ROLL(70.0, 30.0);
        }
        Do_file_event_first(c, event_ptr);

            /* call a big monster */
        Do_send_line(c,
            "Shrieeeek!!  You scared it, and it called one of its friends.\n");

	Do_cancel_monster(c->battle.opponent);
	return;

    case SM_BALROG:

	    /* if there is no experience to take, do damage */
        if (RND() > .33 && c->player.experience > 0 && 
            c->player.special_type < SC_KING) {

                        /* take experience away */
            inflict = (.001 + RND() * .003) * c->player.experience;
            inflict = MIN(c->player.experience, inflict);

                        /* add to strength */
            c->battle.opponent->strength += MIN(.05 * c->player.shield, floor(sqrt(c->player.experience / 1800.0)));

            sprintf(string_buffer, "%s lashes his whip and absorbs %.0f experience points from you!\n",
		        c->battle.opponent->name, inflict);
    
            Do_send_line(c, string_buffer);

            Do_experience(c, -inflict, FALSE);
            return;
        }
        break;

    case SM_FAERIES:

            /* holy water kills when monster tries to hit */
        if (c->player.holywater >= floor(sqrt(c->battle.opponent->size))) {
            Do_send_line(c, "Your holy water killed it!\n");
            c->player.holywater -= floor(sqrt(c->battle.opponent->size));
            c->battle.opponent->energy = 0.0;
            return;
        }
        break;

    case SM_TITAN:

        c->player.shield = ceil(c->player.shield * .99);
	inflict = floor(1.0 + RND() * c->battle.opponent->strength);
	inflict = MIN(inflict, c->player.energy);

	if (c->battle.force_field > 0.0) {

		/* inflict damage through force field */

	    sprintf(string_buffer,
		    "%s punches through your pitiful force field and hit you for %.0f damage!\n",
		    c->battle.opponent->name, inflict);

	    Do_send_line(c, string_buffer);

            c->battle.force_field = 0.0;

	    Do_energy(c, c->player.energy - inflict,
		c->player.max_energy, c->player.shield,
		c->battle.force_field, FALSE);

	    return;
	} else {
	    sprintf(string_buffer,
		    "%s hit you for %.0f damage and damages your shield!\n",
		    c->battle.opponent->name, inflict);

	    Do_send_line(c, string_buffer);
        }


	break;

    case SM_NONE:
	    /* normal hit */
	break;

    default:

            /* special things */
        if ((RND() < 0.2) || 
            ((c->battle.opponent->special_type == SM_CERBERUS) && 
             (c->player.shield / c->player.degenerated > 50000.0)) ||
            ((c->battle.opponent->special_type == SM_JABBERWOCK) &&
             (c->player.quicksilver / c->player.degenerated > 500.0))
           ) {
          
            /* check for magic resistance */
            if ((c->battle.opponent->special_type != SM_MODNAR) &&
                (c->battle.opponent->special_type != SM_MORGOTH) &&
                (c->battle.opponent->special_type != SM_MORON) &&
                (c->battle.opponent->special_type != SM_SMURF) &&
                (c->battle.opponent->special_type != SM_IDIOT) &&
                (c->battle.opponent->special_type != SM_MIMIC) &&
                (c->battle.opponent->special_type != SM_SMEAGOL) &&
                (c->battle.opponent->special_type != SM_TROLL) &&
                ((c->player.type == C_HALFLING) && (RND() < 0.25))) {

                sprintf(string_buffer, "%s tries to do something special, but you resist the attack!\n", c->battle.opponent->name);

	        Do_send_line(c, string_buffer);

                return;
            } 

            if ((c->battle.opponent->special_type != SM_MODNAR) &&
                (c->battle.opponent->special_type != SM_MORON) &&
                (c->battle.opponent->special_type != SM_SMURF) &&
                (c->battle.opponent->special_type != SM_TROLL) &&
                (c->battle.opponent->special_type != SM_SMEAGOL) &&
                ((c->battle.opponent->special_type != SM_SARUMAN) ||
                 (c->player.amulets >= c->player.charms)) &&
                (c->battle.opponent->special_type != SM_MIMIC)) {

                /* dwarves/halflings/expers use fewer charms */
                dtemp = c->battle.opponent->treasure_type 
                       - (floor((c->player.type + 1) * 1.5));

                if (dtemp < 1) dtemp = 1;

                dtemp2 = ceil(RND() * floor(sqrt(c->battle.opponent->size) *
                                      dtemp));
                if (dtemp2 < dtemp) {
                    dtemp2 = dtemp;
                }

                if (c->player.charms >= dtemp2) {
                    sprintf(string_buffer, 
                            "%s, %s, %.0lf charms blocked size %.0lf attack (%.0lf TT %d)\n", 
                            c->player.lcname, 
                            c->realm->charstats[c->player.type].class_name, 
                            dtemp2, c->battle.opponent->size, dtemp,
                            c->battle.opponent->treasure_type);

                    Do_log(COMBAT_LOG, string_buffer);

                    sprintf(string_buffer, "%s tries to do something special, but you used %.0lf of your charms to block it!\n", c->battle.opponent->name, dtemp2);

	            Do_send_line(c, string_buffer);

                    c->player.charms -= dtemp2;

                    return;
                }
            }

            switch (c->battle.opponent->special_type) {

            case SM_LEANAN:

                    /* takes some of the player's strength */
                inflict = ROLL(1.0, c->battle.opponent->size * 4);
                inflict = MIN(c->player.level, inflict);
                inflict = MAX(.02 * c->player.strength, inflict);

                if (inflict > c->player.strength) {

                    sprintf(string_buffer, "%s sucks all of your strength away, destroying your soul!\n",
                        c->battle.opponent->name);

                    inflict = c->player.strength;

                } else {
  
                    sprintf(string_buffer, "%s sapped %0.f of your strength!\n",
                            c->battle.opponent->name, inflict);
                }

		Do_send_line(c, string_buffer);

		Do_strength(c, c->player.max_strength - inflict,
			c->player.sword, c->battle.strengthSpell, FALSE);

                return;

            case SM_THAUMATURG:

                    /* transport player */
                sprintf(string_buffer, "%s transported you!\n",
			    c->battle.opponent->name);

		Do_send_line(c, string_buffer);

		event_ptr = (struct event_t *) Do_create_event();
		event_ptr->type = MOVE_EVENT;
		event_ptr->arg3 = A_FAR;
		Do_file_event(c, event_ptr);

                Do_cancel_monster(c->battle.opponent);

                return;

	    case SM_SARUMAN:

		if (c->player.charms > c->player.amulets) {
		    sprintf(string_buffer, 
			    "%s turns your charms into amulets and vice versa!\n",
			    c->battle.opponent->name);
		    Do_send_line(c, string_buffer);

		    dtemp = c->player.charms;
		    c->player.charms = c->player.amulets;
		    c->player.amulets = dtemp;
		} else if (c->player.palantir) {
			/* take away palantir */
		    Do_send_line(c, "Wormtongue stole your palantir!\n");
		    Do_palantir(c, FALSE, FALSE);
		} else if (RND() > 0.5) {
			/* gems turn to gold */
		    sprintf(string_buffer,
			    "%s transformed your gems into gold!\n",
			    c->battle.opponent->name);

		    Do_send_line(c, string_buffer);
		    Do_gold(c, c->player.gems, FALSE);
		    Do_gems(c, -c->player.gems, FALSE);
		} else {
			/* scramble some stats */
		    sprintf(string_buffer,
			    "%s casts a spell and you feel different!\n",
			    c->battle.opponent->name);

		    Do_send_line(c, string_buffer);
		    Do_scramble_stats(c);
		}
		return;

            case SM_VORTEX:

                    /* suck up some mana */
                inflict = ROLL(0, 50 * c->battle.opponent->size);
                inflict = MIN(c->player.mana, floor(inflict));

		sprintf(string_buffer, "%s sucked up %.0f of your mana!\n",
			c->battle.opponent->name, inflict);

		Do_send_line(c, string_buffer);

		Do_mana(c, -inflict, FALSE);
                return;

            case SM_SUCCUBUS:

                    /* take some brains */
                sprintf(string_buffer, "%s caresses you and whispers sweet nothings in your ear.  You feel foolish!\n",
			c->battle.opponent->name);

		Do_send_line(c, string_buffer);
                c->player.brains *= 0.8;
                return;

            case SM_TIAMAT:

                    /* take some gold and gems */
                sprintf(string_buffer,
			"%s took half your gold and gems and flew off.\n",
			c->battle.opponent->name);

		Do_send_line(c, string_buffer);
		Do_gold(c, -floor(c->player.gold / 2.0), FALSE);
                Do_gems(c, -floor(c->player.gems / 2.0), FALSE);
                Do_cancel_monster(c->battle.opponent);
                return;

            case SM_KOBOLD:

                    /* steal a gold piece and run */
                if (c->player.gold > 0) {
                    sprintf(string_buffer,
			"%s stole one gold piece and ran away.\n",
			c->battle.opponent->name);

		    Do_send_line(c, string_buffer);
		    Do_gold(c, -1.0, FALSE);
		    Do_check_weight(c);
                    Do_cancel_monster(c->battle.opponent);
                    return;
                } else break;

            case SM_SHELOB:

                    /* bite and (medium) poison */
                sprintf(string_buffer, "%s has bitten and poisoned you!\n",
			c->battle.opponent->name);

		Do_send_line(c, string_buffer);
		Do_adjusted_poison(c, 1.0);
                return;

            case SM_LAMPREY:

                if (RND() * 10 < (c->battle.opponent->size / 2) - 1) {
                    /*  bite and (small) poison */
                sprintf(string_buffer, "%s bit and poisoned you!\n",
			c->battle.opponent->name);

		Do_send_line(c, string_buffer);
		Do_adjusted_poison(c, 0.25);
                }
                return;

            case SM_BONNACON:

                    /* fart and run */
                sprintf(string_buffer, "%s farted and scampered off.\n",
			c->battle.opponent->name);

		Do_send_line(c, string_buffer);

		    /* damage from fumes */
		Do_energy(c, c->player.energy / 2.0, c->player.max_energy,
			c->player.shield, c->battle.force_field, FALSE);

                Do_cancel_monster(c->battle.opponent);
                return;

            case SM_SMEAGOL:

                if (c->player.ring_type != R_NONE) {

                        /* try to steal ring */
                    if (RND() > 0.1)

                        sprintf(string_buffer,
		        "%s tried to steal your ring, but was unsuccessful.\n",
			c->battle.opponent->name);

                    else {

                        sprintf(string_buffer,
		         "%s tried to steal your ring and ran away with it!\n",
			 c->battle.opponent->name);

			Do_ring(c, R_NONE, FALSE);
                        Do_cancel_monster(c->battle.opponent);
                    }
		    Do_send_line(c, string_buffer);
	   	    return;
                } else if (c->player.type == C_HALFLING) {
                  if (c->player.sin > 2.0) { 
                    sprintf(string_buffer,
		         "%s cries, 'Thief!  Baggins!  We hates it, we hates it, for ever and ever!'\n", 
                         c->battle.opponent->name);
		    Do_send_line(c, string_buffer);
    
                  } else {
                    sprintf(string_buffer,
                         "%s wonders, 'What has it got in itsss pocketsss?'\n",
                         c->battle.opponent->name);
		    Do_send_line(c, string_buffer);
                  }
                }
                break;

            case SM_CERBERUS:

                    /* take all metal treasures */
                sprintf(string_buffer, "%s took all your metal treasures and ran off!\n",
			c->battle.opponent->name);

		Do_send_line(c, string_buffer);

	        sprintf(string_buffer, "%s, %s, cerbed.\n",
	   	        c->player.lcname,
		        c->realm->charstats[c->player.type].class_name);

	        Do_log(COMBAT_LOG, &string_buffer);

		Do_energy(c, c->player.energy, c->player.max_energy, 0.0,
			c->battle.force_field, FALSE);

		Do_strength(c, c->player.max_strength, 0.0,
			c->battle.strengthSpell, FALSE);

                if (c->player.level > MIN_KING) {
		        Do_crowns(c, -c->player.crowns, FALSE);
                }

		Do_gold(c, -c->player.gold, FALSE);
		
                Do_cancel_monster(c->battle.opponent);
		return;

            case SM_UNGOLIANT:

                    /* (large) poison and take a quickness */
                sprintf(string_buffer, "%s stung you with a virulent poison.  You begin to slow down!\n",
			c->battle.opponent->name);

		Do_send_line(c, string_buffer);
		Do_adjusted_poison(c, 5.0);

		Do_speed(c, c->player.max_quickness - 1.0,
			c->player.quicksilver, c->battle.speedSpell, FALSE);

                return;

            case SM_JABBERWOCK:

              if ((RND() > .75) || (c->player.quicksilver == 0)) {
                /* fly away, and leave either a Jubjub bird or Bandersnatch */
                sprintf(string_buffer,
            "%s flew away, and left you to contend with one of its friends.\n",
			c->battle.opponent->name);

		Do_send_line(c, string_buffer);
		event_ptr = (struct event_t *) Do_create_event();
		event_ptr->type = MONSTER_EVENT;
		event_ptr->arg1 = MONSTER_JABBERWOCK;
                event_ptr->arg3 = 55 + ((RND() > 0.5) ? 16 : 0);
		Do_file_event_first(c, event_ptr);

                Do_cancel_monster(c->battle.opponent);
		return;
              } else {
                    /* burble, causing the player to lose quicksilver */
                sprintf(string_buffer,
                        "%s burbles as it drinks half of your quicksilver!\n",
			c->battle.opponent->name);

		Do_send_line(c, string_buffer);
		Do_speed(c, c->player.max_quickness,
			floor(c->player.quicksilver * .5), c->battle.speedSpell, FALSE);
		return;
              }


            case SM_TROLL:

                    /* partially regenerate monster */
                sprintf(string_buffer,
			"%s partially regenerated his energy!\n",
			c->battle.opponent->name);

		Do_send_line(c, string_buffer);

                c->battle.opponent->energy += floor(
			(c->battle.opponent->max_energy -
			c->battle.opponent->energy) / 2.0);

                c->battle.opponent->strength = c->battle.opponent->max_strength;
                c->battle.opponent->speed = c->battle.opponent->max_speed;
                c->battle.melee_damage = c->battle.skirmish_damage = 0.0;
                return;

            case SM_WRAITH:

                if (!c->player.blind) {

                        /* make blind */
                    sprintf(string_buffer, "%s blinded you!\n",
			    c->battle.opponent->name);

		    Do_send_line(c, string_buffer);
                    c->player.blind = TRUE;
                    strcpy(c->battle.opponent->name, "A monster");

		        /* update the character description */
		    Do_lock_mutex(&c->realm->realm_lock);
		    Do_player_description(c);
		    Do_unlock_mutex(&c->realm->realm_lock);


		    return;
                }
                break;

            case SM_IDIOT:

                sprintf(string_buffer, "%s drools.\n",
	                c->battle.opponent->name);

   	        Do_send_line(c, string_buffer);
                break;

            case SM_MORON:

                /* don't subject males to pickup line */
                if (c->player.gender == FEMALE) { 
	            Do_send_line(c, moronmesg[(int) ROLL(0.0,
		        (double) sizeof(moronmesg) / sizeof(char *))]);
                } else { 
	            Do_send_line(c, moronmesg[(int) ROLL(0.0,
		        ((double) (sizeof(moronmesg) / sizeof(char *)) - 2))]);
                }

                break;

            case SM_SMURF:

                if (RND() < .5) {
                  if (RND() < .5) {
                    if (c->player.gender == FEMALE) {
                      sprintf(string_buffer, "%s sneers, 'Smurfette is prettier than you!'\n", c->battle.opponent->name);
                    } else if (c->player.type == C_MAGIC) {
                      sprintf(string_buffer, "%s yells out, 'Aah!  I'm being attacked by Gargamel!'\n", c->battle.opponent->name);
                    } else if (c->player.type == C_HALFLING) {
                      sprintf(string_buffer, "%s wonders, 'Are you Angry Smurf?'\n", c->battle.opponent->name);
                    } else if (c->player.type == C_DWARF) {
                      sprintf(string_buffer, "%s howls, 'A giant!  Run!'\n", c->battle.opponent->name);
                    } else {
                      sprintf(string_buffer, "%s snarls, 'Smurf you, you smurfing smurf!'\n", c->battle.opponent->name);
                    }
                  } else {
                    sprintf(string_buffer, "%s shrieks, 'Help, Papa Smurf, Help!'\n", c->battle.opponent->name);
                  }

                } else {
                  sprintf(string_buffer, "%s sings, 'Lah lah la lah la la!'\n", c->battle.opponent->name);
                }

   	        Do_send_line(c, string_buffer);
                break;


	    case SM_NAZGUL:

                    /* try to take ring if player has one */
                if (c->player.ring_type != R_NONE) {

                        /* player has a ring */
                    sprintf(string_buffer, "%s demands your ring.  Do you hand it over?\n", c->battle.opponent->name);
		    Do_send_line(c, string_buffer);

		    if (Do_yes_no(c, &theAnswer) == S_NORM && theAnswer == 0) {

                            /* take ring away */
			Do_ring(c, R_NONE, FALSE);
                        c->battle.ring_in_use = FALSE;
                        Do_cancel_monster(c->battle.opponent);
			return;
		    } else {
                        c->battle.opponent->strength *= 1.1 + .4 * RND();

                        c->battle.opponent->max_speed++;
                        c->battle.opponent->speed = 
                             c->battle.opponent->speed + 
                             ceil((c->battle.opponent->max_speed - 
                                  c->battle.opponent->speed) / 2);

                        sprintf(string_buffer, "Angered by your refusal, %s attacks harder and faster!.\n", c->battle.opponent->name);

		        Do_send_line(c, string_buffer);
                    }
                        /* also fall through to the curse */

                }
                    /* curse the player */

                if (c->player.blessing == TRUE) {
                    sprintf(string_buffer, "%s hurls an eldritch curse at you!  But you were saved by your blessing!\n", c->battle.opponent->name);
		    Do_send_line(c, string_buffer);
	            Do_blessing(c, FALSE, FALSE);
                } else {
                    sprintf(string_buffer, "%s hurls an eldritch curse at you!  You feel weak and ill!\n", c->battle.opponent->name);
		    Do_send_line(c, string_buffer);
                  
		    Do_energy(c, 
                        (c->player.energy + c->battle.force_field) / 2,
			c->player.max_energy * .99, c->player.shield,
			c->battle.force_field, FALSE);

		    Do_adjusted_poison(c, 0.5);
                    return;
		}
		break;
	    }
        }
    }

        /* fall through to here if monster inflicts a normal hit */
    if (inflict == -1) {

	inflict = floor(1.0 + RND() * c->battle.opponent->strength);
    }

    sprintf(string_buffer, "%s hit you for %.0f damage!\n",
	    c->battle.opponent->name, inflict);

    Do_send_line(c, string_buffer);

    if (c->battle.force_field < inflict) {

	Do_energy(c, c->player.energy + c->battle.force_field - inflict,
		c->player.max_energy, c->player.shield, 0, FALSE);
    }
    else {

	Do_energy(c, c->player.energy, c->player.max_energy, c->player.shield,
		c->battle.force_field - inflict, FALSE);
    }

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_cancel_monster(struct opponent_t the_opponent)
/
/ FUNCTION: mark current monster as no longer active
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 5/23/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: none
/
/ DESCRIPTION:
/       Clear current monster's energy, experience, treasure type, and
/       flock.  This is the same as having the monster run away.
/
*************************************************************************/

Do_cancel_monster(struct opponent_t *the_opponent)
{
    the_opponent->energy = 0.0;
    the_opponent->experience = 0.0;
    the_opponent->treasure_type = 0;
    the_opponent->flock_percent = 0;
}


/**/
/************************************************************************
/
/ FUNCTION NAME: Do_hitmonster()
/
/ FUNCTION: inflict damage upon current monster
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 6/9/99
/
/ ARGUMENTS:
/       double inflict - damage to inflict upon monster
/
/ RETURN VALUE: none
/
/ MODULES CALLED: monsthits(), wmove(), strcmp(), waddstr(), mvprintw()
/
/ DESCRIPTION:
/       Hit monster specified number of times.  Handle when monster dies,
/       and a few special monsters.
/
*************************************************************************/

Do_hitmonster(struct client_t *c, double inflict)
{
    char string_buffer[SZ_LINE];

    sprintf(string_buffer, "You hit %s for %.0f damage!\n", c->battle.opponent->name,
	    inflict);

    Do_send_line(c, string_buffer);

    c->battle.opponent->shield -= inflict;

    if (c->battle.opponent->shield < 0.0) {
        c->battle.opponent->energy += c->battle.opponent->shield;
	c->battle.opponent->shield = 0.0;
    }

    if (c->battle.opponent->special_type != SM_IT_COMBAT) {

        if (c->battle.opponent->energy > 0.0) {

            if (c->battle.opponent->special_type == SM_DARKLORD ||
		    c->battle.opponent->special_type == SM_SHRIEKER) {

            	    /* special monster didn't die */
                Do_monsthits(c);
            }
        }
        else {

                /* monster died.  print message. */
            if (c->battle.opponent->special_type == SM_DARKLORD) {

	        sprintf(string_buffer, "%s has just defeated the Dark Lord!\n",
		        c->modifiedName);

	        Do_broadcast(c, string_buffer);
	    }
            else if (c->battle.opponent->special_type == SM_MORGOTH) {

	        sprintf(string_buffer, "You have overthrown Morgoth!  But beware, he will return...\n",
		        c->modifiedName);
		        c->morgothCount = 500;

	        Do_send_line(c, string_buffer);

	        sprintf(string_buffer, "All hail %s for overthrowing Morgoth, Enemy of the Vala!\n",
		        c->modifiedName);

	        Do_broadcast(c, string_buffer);

	    }
            else if (c->battle.opponent->special_type == SM_SMURF && 
                     !c->player.blind) {

	        sprintf(string_buffer, "You finally manage to squish the Smurf.  Good work, %s.\n",
		        c->modifiedName);

	        Do_send_line(c, string_buffer);

	    }
            else {

                    /* all other types of monsters */
	        sprintf(string_buffer, "You killed it.  Good work, %s.\n",
		        c->modifiedName);

	        Do_send_line(c, string_buffer);

                if (c->battle.opponent->special_type == SM_MIMIC &&
		        strcmp(c->battle.opponent->name, "A Mimic") != 0 &&
		        !c->player.blind) {

                    sprintf(string_buffer,
		       "%s's body slowly changes into the form of a mimic.\n",
		       c->battle.opponent->name);

	            Do_send_line(c, string_buffer);
		}
            }
        }
    }
}


/**/
/************************************************************************
/
/ FUNCTION NAME: Do_throwspell(struct client_t *c)
/
/ FUNCTION: throw a magic spell
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 6/9/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: hitmonster(), cancelmonster(), sqrt(), floor(), wmove(),
/       drandom(), altercoordinates(), longjmp(), infloat(), waddstr(), mvprintw(),
/       getanswer()
/
/ DESCRIPTION:
/       Prompt player and process magic spells.
/
*************************************************************************/

Do_throwspell(struct client_t *c, short *theAttack, double *theArg)
{
    struct button_t theButtons;
    double  inflict = 0.0;        /* damage inflicted */
    double  dtemp;          /* for dtemporary calculations */
    int     ch;             /* input */
    struct event_t *event_ptr;
    char string_buffer[SZ_LINE];
    long answer;
    short stemp;
    float ftemp;

    Do_clear_buttons(&theButtons, 0);

    if (c->player.magiclvl >= ML_ALLORNOTHING &&
            c->battle.opponent->special_type != SM_MORGOTH)

	strcpy(theButtons.button[0], "All or Nothing\n");

    if (c->player.magiclvl >= ML_MAGICBOLT &&
            c->battle.opponent->special_type != SM_MORGOTH)

	strcpy(theButtons.button[1], "Magic Bolt\n");

    if ((c->player.magiclvl >= ML_FORCEFIELD) &&
	(c->battle.opponent->special_type != SM_MORGOTH)) {
	strcpy(theButtons.button[4], "Force Field\n");
    }

    if (c->player.magiclvl >= ML_XFORM &&
	c->battle.opponent->special_type != SM_MORGOTH) {

	strcpy(theButtons.button[7], "Transform\n");
    }
    else {
	strcpy(theButtons.button[7], "Cancel\n");
    }

    if (c->player.magiclvl >= ML_INCRMIGHT &&
	c->battle.opponent->special_type != SM_MORGOTH) 

	strcpy(theButtons.button[2], "Increase Might\n");

    if (c->player.magiclvl >= ML_HASTE &&
	c->battle.opponent->special_type != SM_MORGOTH) 

	strcpy(theButtons.button[3], "Haste\n");

    if (c->player.magiclvl >= ML_XPORT &&
            c->battle.opponent->special_type != SM_MORGOTH)

	strcpy(theButtons.button[5], "Transport\n");

    if (c->player.magiclvl >= ML_PARALYZE &&
            c->battle.opponent->special_type != SM_MORGOTH)

	strcpy(theButtons.button[6], "Paralyze\n");

    Do_send_line(c, "Which spell do you wish to cast?\n");

    if (Do_buttons(c, &answer, &theButtons) != S_NORM) {
	*theAttack = IT_PASS;
	return;
    }

    switch(answer) {

    case 0:   /* all or nothing */

        ftemp = c->battle.opponent->sin - (c->player.level / 100);
        
            /* for debugging purposes */
        if (c->battle.opponent->special_type == SM_IT_COMBAT) {
            sprintf(string_buffer, 
                    "%s, %s, IT AON, Level %.0lf, Op Sin: %.0f, %.0f\%\n",
                    c->player.lcname, 
                    c->realm->charstats[c->player.type].class_name, 
                    c->player.level,
                    c->battle.opponent->sin,
                    ftemp);

            Do_log(COMBAT_LOG, string_buffer);
        }

        
	    /* 25% or difference in levels div 10000 in it_combat */
        if ((c->battle.opponent->special_type == SM_IT_COMBAT 
             && RND() < MAX(.2, ftemp / 100))

		    /* success is based on ML and monster exp in combat */
		|| (c->battle.opponent->special_type != SM_IT_COMBAT &&
		3000 * c->player.magiclvl / (c->battle.opponent->experience +
		10000 * c->player.magiclvl) > RND())) {

	        /* success */
            inflict = c->battle.opponent->energy +
		    c->battle.opponent->shield + 1.0;

	    *theAttack = IT_ALL_OR_NOT;
	    *theArg = inflict;

            if (c->battle.opponent->special_type == SM_DARKLORD) {

                    /* all or nothing doesn't quite work against D. L. */
                inflict *= 0.9;
	    }
        }
        else {

                /* failure -- monster gets stronger and quicker */
            c->battle.opponent->strength = c->battle.opponent->max_strength
		    *= 2.0;

            c->battle.opponent->max_speed *= 2.0;

                /* paralyzed monsters wake up a bit */
	    if (c->battle.opponent->speed * 2.0 > 1.0) {
                c->battle.opponent->speed = c->battle.opponent->speed * 2.0;
	    }
	    else {
                ++c->battle.opponent->speed;
	    }

	    *theAttack = IT_NO_ALL_OR_NOT;

	    sprintf(string_buffer, "%s, %s, no_allornothing.\n",
		    c->player.lcname,
		    c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
        }

        if (c->player.mana >= MM_ALLORNOTHING) {

                /* take a mana if player has one */
	    Do_mana(c, -MM_ALLORNOTHING, FALSE);
	}

        Do_hitmonster(c, inflict);

	sprintf(string_buffer, "%s, %s, allornothing %.0lf.\n",
		c->player.lcname,
		c->realm->charstats[c->player.type].class_name, inflict);

	Do_log(COMBAT_LOG, &string_buffer);

        break;

    case 1:   /* magic bolt */

        if (c->player.magiclvl < ML_MAGICBOLT) {
	    Do_send_line(c, "You can not cast that spell.\n"); 
	    *theAttack = IT_NO_BOLT;
	}
        else {
                /* prompt for amount to expend */
	    if (Do_double_dialog(c, &dtemp, "How much mana for bolt?\n")) {

		sprintf(string_buffer, "%s, %s, no_magicbolt.\n",
			c->player.lcname,
			c->realm->charstats[c->player.type].class_name);

		Do_log(COMBAT_LOG, &string_buffer);
	        *theAttack = IT_NO_BOLT;
          	break;
	    }
 
            if  (dtemp > 0.0 && dtemp <= c->player.mana) {

		Do_mana(c, -dtemp, FALSE);

                if (c->battle.opponent->special_type == SM_DARKLORD) {

                        /* magic bolts don't work against D. L. */
                    inflict = 0.0;

                    Do_send_line(c, "Magic Bolt fired!\n");
                    Do_hitmonster(c, inflict);
	            *theAttack = IT_BOLT;
		    *theArg = inflict;

		    sprintf(string_buffer, "%s, %s, magicbolt %.0lf.\n",
			   c->player.lcname,
			   c->realm->charstats[c->player.type].class_name,
			   inflict);

		    Do_log(COMBAT_LOG, &string_buffer);
	        }
                else {

                    if (c->battle.opponent->special_type == SM_IT_COMBAT) {
		        /* magic bolts in IT_COMBAT don't always work */
                        if (RND() > .05 + (sqrt(c->player.magiclvl) / 50)) {

                                /* the magic bolt fails */
                            Do_send_line(c, "You mess up the incantation!\n");
	                    *theAttack = IT_NO_BOLT;
		            *theArg = 0; 

			    sprintf(string_buffer, "%s, %s, no_magicbolt.\n",
			       c->player.lcname,
			       c->realm->charstats[c->player.type].class_name);

			    Do_log(COMBAT_LOG, &string_buffer);
                        } else {
                            inflict = floor(dtemp * (.7 + .6 * RND()) *
				(pow(c->player.magiclvl, 0.20) + 1));

                            Do_send_line(c, "A weak Magic Bolt is fired!\n");
                            Do_hitmonster(c, inflict);
	                    *theAttack = IT_BOLT;
		            *theArg = inflict;

			    sprintf(string_buffer, "%s, %s, magicbolt %.0lf.\n",
			       c->player.lcname,
			       c->realm->charstats[c->player.type].class_name,
			       inflict);

			    Do_log(COMBAT_LOG, &string_buffer);
                        }
                         
                    }
		    else {


				/* Bolt Damage */
				
                        inflict = floor(dtemp * (.7 + .6 * RND()) *
				(pow(c->player.magiclvl, 0.40) + 1));
				
				
				/* Low levels dont get special bolts */
				
				if (c->player.level > 10){
				
				/* Dud bolt, normal bolt, or super bolt? */
				
                        if ((RND() * (c->player.level + c->player.magiclvl)) < 
                            sqrt(c->player.level)) {

							if (RND() > .5){
							
								/* Dud Bolt 50%-100% damage*/
								Do_send_line(c, "You fumble over the incantation a bit...\n");	
                        	    inflict = inflict * ( ( RND() / 2 ) + .5 );
                        	} else {
                        	
                        		/* Super Bolt 100%-200% Damage + 25%-75% of expended Mana*/
                        		Do_send_line(c, "Magical energies surge through you!\n");
                        		inflict = inflict + (inflict * RND());
                        		Do_mana(c, dtemp * ( ( RND() / 2 ) + .25 ), FALSE);
                        	}
                        }
                        }

                        Do_send_line(c, "Magic Bolt fired!\n");
                        Do_hitmonster(c, inflict);
	                *theAttack = IT_BOLT;
		        *theArg = inflict;

			sprintf(string_buffer, "%s, %s, magicbolt %.0lf.\n",
			       c->player.lcname,
			       c->realm->charstats[c->player.type].class_name,
			       inflict);

			Do_log(COMBAT_LOG, &string_buffer);
                   }
                }
	    }
            else if  (dtemp > c->player.mana) {
	        Do_send_line(c, "You do not have enough mana.\n"); 

		sprintf(string_buffer, "%s, %s, no_magicbolt.\n",
		       c->player.lcname,
		       c->realm->charstats[c->player.type].class_name);

		Do_log(COMBAT_LOG, &string_buffer);
	        *theAttack = IT_NO_BOLT;
	    }
	    else {
		sprintf(string_buffer, "%s, %s, no_magicbolt.\n",
		       c->player.lcname,
		       c->realm->charstats[c->player.type].class_name);

		Do_log(COMBAT_LOG, &string_buffer);
	        *theAttack = IT_NO_BOLT;
	    }
	}

        break;

    case 4:   /* force field */

        if (c->player.magiclvl < ML_FORCEFIELD) {

	    Do_send_line(c, "You can not cast that spell.\n"); 

	    sprintf(string_buffer, "%s, %s, no_forcefield.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	    *theAttack = IT_NO_SHIELD;
	}
        else if (c->player.mana <= MM_FORCEFIELD) { 

	    Do_send_line(c, "You do not have enough mana.\n");

	    sprintf(string_buffer, "%s, %s, no_forcefield.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	    *theAttack = IT_NO_SHIELD;
        }
        else if (c->player.mana < MM_FORCEFIELD + (c->player.magiclvl / 2)) {

	    Do_send_line(c, "You run out of mana before you can put up a full-strength field.\n"); 

	    Do_energy(c, c->player.energy, c->player.max_energy,
		    c->player.shield, c->battle.force_field + 40 *
                    (c->player.mana - MM_FORCEFIELD), FALSE);
	    Do_mana(c, -(c->player.mana), FALSE);

	    sprintf(string_buffer, "%s, %s, weak-forcefield.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	    *theAttack = IT_SHIELD;
	}
        else {

	    Do_mana(c, -(MM_FORCEFIELD + (c->player.magiclvl / 2)), FALSE);

	    Do_energy(c, c->player.energy, c->player.max_energy,
		    c->player.shield, c->battle.force_field + 20 *
                    c->player.magiclvl, FALSE);

            Do_send_line(c, "Force Field up.\n");

	    sprintf(string_buffer, "%s, %s, forcefield.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	    *theAttack = IT_SHIELD;
        }

        break;

    case 7:   /* transform */

        if (c->player.magiclvl < ML_XFORM || c->battle.opponent->special_type
		== SM_MORGOTH) {

	    /* Drop out, this is a cancel */
	    sprintf(string_buffer, "%s, %s, cancel.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
            *theAttack = IT_PASS;
            Do_playerhits(c, theAttack, theArg);
	}
        else if (c->player.mana < MM_XFORM) {
	    Do_send_line(c, "You do not have enough mana.\n"); 

	    sprintf(string_buffer, "%s, %s, no_transform.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	    *theAttack = IT_NO_TRANSFORM;
	}
	else if (c->battle.opponent->special_type == SM_IT_COMBAT) {

            Do_mana(c, -MM_XFORM, FALSE);

		/* spell succeeds against another player */
	    if (RND() < 0.15) {
	        event_ptr = (struct event_t *) Do_create_event();
	        event_ptr->type = MONSTER_EVENT;
	        event_ptr->arg1 = MONSTER_TRANSFORM;
	        event_ptr->arg3 = (int) ROLL(0.0, 100.0);

		sprintf(string_buffer, "%s screams in agony and becomes %s.\n",
			c->battle.opponent->name, 
	    		c->realm->monster[event_ptr->arg3].name);

		Do_send_line(c, string_buffer);

		Do_file_event_first(c, event_ptr);
	        Do_cancel_monster(c->battle.opponent);
		*theAttack = IT_TRANSFORM;
		*theArg = event_ptr->arg3;

	        sprintf(string_buffer, "%s, %s, transform.\n",
		       c->player.lcname,
		       c->realm->charstats[c->player.type].class_name);

	        Do_log(COMBAT_LOG, &string_buffer);
	    }
		/* spell backfires */
	    else if (RND() < 0.02) {
		event_ptr = (struct event_t *) Do_create_event();
                event_ptr->type = DEATH_EVENT;
                event_ptr->arg1 = (double) ROLL(0.0, 90.0);
                event_ptr->arg3 = K_TRANSFORMED;
                Do_cancel_monster(c->battle.opponent);
                *theAttack = IT_TRANSFORM_BACK;
                *theArg = event_ptr->arg1;

		sprintf(string_buffer,
			"The spell backfires and you become %s.\n",
	    		c->realm->monster[event_ptr->arg3].name);

		Do_send_line(c, string_buffer);

	        sprintf(string_buffer, "%s, %s, transform_back.\n",
		       c->player.lcname,
		       c->realm->charstats[c->player.type].class_name);

	        Do_log(COMBAT_LOG, &string_buffer);
                Do_handle_event(c, event_ptr);
	    }
		/* spell fails */
	    else {
		sprintf(string_buffer, "%s resists the Transform spell.\n",
			c->battle.opponent->name); 

		Do_send_line(c, string_buffer);

	        sprintf(string_buffer, "%s, %s, no_transform.\n",
		       c->player.lcname,
		       c->realm->charstats[c->player.type].class_name);

	        Do_log(COMBAT_LOG, &string_buffer);
                *theAttack = IT_NO_TRANSFORM;
	    }
	}
        else {

	    Do_mana(c, -MM_XFORM, FALSE);

            if (8000 * c->player.magiclvl / (c->battle.opponent->experience +
                20000 * c->player.magiclvl) > RND() - .1) {

	        event_ptr = (struct event_t *) Do_create_event();
	        event_ptr->type = MONSTER_EVENT;
	        event_ptr->arg1 = MONSTER_TRANSFORM;
	        event_ptr->arg3 = (int) ROLL(0.0, 100.0);

		Do_file_event_first(c, event_ptr);

	        sprintf(string_buffer, "The monster shudders and transforms!\n");
	        Do_send_line(c, string_buffer);

	        Do_cancel_monster(c->battle.opponent);
                *theAttack = IT_TRANSFORM;

	        sprintf(string_buffer, "%s, %s, transform.\n",
		       c->player.lcname,
		       c->realm->charstats[c->player.type].class_name);

	        Do_log(COMBAT_LOG, &string_buffer);
	    }
		/* the spell backfires */

/*  Removed 12/29/00 - Insta-death is mean
	    else if (RND() < 0.01) {
		event_ptr = (struct event_t *) Do_create_event();
                event_ptr->type = DEATH_EVENT;
                event_ptr->arg1 = (double) ROLL(0.0, 90.0);
                event_ptr->arg3 = K_TRANSFORMED;
                Do_file_event(c, event_ptr);
                Do_cancel_monster(c->battle.opponent);
                *theAttack = IT_TRANSFORM_BACK;
                *theArg = event_ptr->arg1;

	        sprintf(string_buffer, "%s, %s, transform_back.\n",
		       c->player.lcname,
		       c->realm->charstats[c->player.type].class_name);

	        Do_log(COMBAT_LOG, &string_buffer);

		sprintf(string_buffer,
			"The spell backfires and you become %s.\n",
	    		c->realm->monster[event_ptr->arg3].name);

		Do_send_line(c, string_buffer);
	    }
*/
		/* spell fails */
	    else {
		sprintf(string_buffer, "%s resists the Transform spell.\n",
			c->battle.opponent->name); 

		Do_send_line(c, string_buffer);

	        sprintf(string_buffer, "%s, %s, no_transform.\n",
		       c->player.lcname,
		       c->realm->charstats[c->player.type].class_name);

	        Do_log(COMBAT_LOG, &string_buffer);
                *theAttack = IT_NO_TRANSFORM;
	    }
       }

       break;

    case 2:   /* increase might */

        if (c->player.magiclvl < ML_INCRMIGHT) {
	    Do_send_line(c, "You can not cast that spell.\n"); 
            *theAttack = IT_NO_MIGHT;

	    sprintf(string_buffer, "%s, %s, no_might.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	}
        else if (c->player.mana < MM_INCRMIGHT + (c->player.magiclvl / 2)) {
	    Do_send_line(c, "You do not have enough mana.\n"); 
            *theAttack = IT_NO_MIGHT;

	    sprintf(string_buffer, "%s, %s, no_might.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	}
        else {

	    Do_mana(c, -(MM_INCRMIGHT + (c->player.magiclvl / 2)), FALSE);

	    Do_strength(c, c->player.max_strength, c->player.sword,
		    c->battle.strengthSpell + c->player.max_strength *
		    (1 - 10 / (sqrt(c->player.magiclvl) + 10)), FALSE);

            *theAttack = IT_MIGHT;

	    sprintf(string_buffer, "%s, %s, incrmight.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
        }

        break;

    case 3:   /* haste */

        if (c->player.magiclvl < ML_HASTE) {
	    Do_send_line(c, "You can not cast that spell.\n"); 
            *theAttack = IT_NO_HASTE;

	    sprintf(string_buffer, "%s, %s, no_haste.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	}
        else if (c->player.mana < MM_HASTE + (c->player.magiclvl / 2)) {
	    Do_send_line(c, "You do not have enough mana.\n"); 
            *theAttack = IT_NO_HASTE;

	    sprintf(string_buffer, "%s, %s, no_haste.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	}
        else {

	    Do_mana(c, -(MM_HASTE + (c->player.magiclvl / 2)), FALSE);

	    Do_speed(c, c->player.max_quickness, c->player.quicksilver,
		    c->battle.speedSpell + c->player.max_quickness / 2 *
		    (1 - 10 / (sqrt(c->player.magiclvl) + 10)), FALSE);

            *theAttack = IT_HASTE;

	    sprintf(string_buffer, "%s, %s, haste.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
        }

        break;

    case 5:   /* transport */

        if (c->player.magiclvl < ML_XPORT) {
	    Do_send_line(c, "You can not cast that spell.\n"); 
            *theAttack = IT_NO_TRANSPORT;

	    sprintf(string_buffer, "%s, %s, no_transport.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	}
        else if (c->player.mana < MM_XPORT) {
	    Do_send_line(c, "You do not have enough mana.\n"); 
            *theAttack = IT_NO_TRANSPORT;

	    sprintf(string_buffer, "%s, %s, no_transport.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	}
        else {

	    Do_mana(c, -MM_XPORT, FALSE);

	    if (c->battle.opponent->special_type == SM_IT_COMBAT) {

		    /* spell succeeds aginst another player */
		if (RND() < 0.25) {
	            Do_cancel_monster(c->battle.opponent);
		    *theAttack = IT_TRANSPORT;

	    	    sprintf(string_buffer, "%s, %s, transport.\n",
		   	    c->player.lcname,
		   	    c->realm->charstats[c->player.type].class_name);

	    	    Do_log(COMBAT_LOG, &string_buffer);

		}
		else {

                    sprintf(string_buffer, "%s resists the transport.\n",
		            c->battle.opponent->name);
			
		    Do_send_line(c, string_buffer);
		    *theAttack = IT_NO_TRANSPORT;

	    	    sprintf(string_buffer, "%s, %s, no_transport.\n",
		   	    c->player.lcname,
		   	    c->realm->charstats[c->player.type].class_name);

	     	    Do_log(COMBAT_LOG, &string_buffer);
		}
	    }
            else if (1000 * c->player.magiclvl /
		    (c->battle.opponent->experience + 1000 *
		    c->player.magiclvl) > RND()) {

                sprintf(string_buffer, "%s is transported.\n",
			c->battle.opponent->name);

		Do_send_line(c, string_buffer);

		stemp = c->battle.opponent->treasure_type;
		Do_cancel_monster(c->battle.opponent);

                if (RND() > 0.5 && c->battle.opponent->special_type != SM_DARKLORD) {

                        /* monster dropped its treasure */
                    c->battle.opponent->treasure_type = stemp;
		}

		*theAttack = IT_TRANSPORT;

	    	sprintf(string_buffer, "%s, %s, transport.\n",
		   	c->player.lcname,
		   	c->realm->charstats[c->player.type].class_name);

	    	Do_log(COMBAT_LOG, &string_buffer);
	    }
	    else if (RND() < 0.2 || c->battle.opponent->special_type == SM_DARKLORD) {

                Do_send_line(c, "Transport backfired!\n");
		event_ptr = (struct event_t *) Do_create_event();
		event_ptr->type = MOVE_EVENT;
		event_ptr->arg3 = A_FAR;
		Do_file_event(c, event_ptr);

                Do_cancel_monster(c->battle.opponent);
                *theAttack = IT_TRANSPORT_BACK;

	    	sprintf(string_buffer, "%s, %s, transport_back.\n",
		   	c->player.lcname,
		   	c->realm->charstats[c->player.type].class_name);

	    	Do_log(COMBAT_LOG, &string_buffer);
	    }
	    else {

                sprintf(string_buffer, "%s resists the transport.\n",
	                c->battle.opponent->name);
			
	        Do_send_line(c, string_buffer);
		*theAttack = IT_NO_TRANSPORT;

	    	sprintf(string_buffer, "%s, %s, no_transport.\n",
		   	c->player.lcname,
		   	c->realm->charstats[c->player.type].class_name);

	    	Do_log(COMBAT_LOG, &string_buffer);
	    }
	}

        break;

    case 6:   /* paralyze */

        if (c->player.magiclvl < ML_PARALYZE) {
	    Do_send_line(c, "You can not cast that spell.\n"); 
	    *theAttack = IT_NO_PARALYZE;

	    sprintf(string_buffer, "%s, %s, no_paralyze.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	}
        else if (c->player.mana < MM_PARALYZE) {
	    Do_send_line(c, "You do not have enough mana.\n"); 
	    *theAttack = IT_NO_PARALYZE;

	    sprintf(string_buffer, "%s, %s, no_paralyze.\n",
		   c->player.lcname,
		   c->realm->charstats[c->player.type].class_name);

	    Do_log(COMBAT_LOG, &string_buffer);
	}
        else {

	    Do_mana(c, -MM_PARALYZE, FALSE);

	    if (c->battle.opponent->special_type == SM_IT_COMBAT) {

		if (RND() < 0.2) {

                    sprintf(string_buffer, "%s is held.\n",
		            c->battle.opponent->name);

		    Do_send_line(c, string_buffer);
                    c->battle.opponent->speed = -2.0;
	            *theAttack = IT_PARALYZE;

		    sprintf(string_buffer, "%s, %s, paralyze.\n",
			   c->player.lcname,
			   c->realm->charstats[c->player.type].class_name);

		    Do_log(COMBAT_LOG, &string_buffer);
                }
                else {

                    sprintf(string_buffer, "%s is unaffected.\n",
		            c->battle.opponent->name);

		    Do_send_line(c, string_buffer);
	            *theAttack = IT_NO_PARALYZE;

		    sprintf(string_buffer, "%s, %s, no_paralyze.\n",
			   c->player.lcname,
			   c->realm->charstats[c->player.type].class_name);

		    Do_log(COMBAT_LOG, &string_buffer);
		}
	    }

            else if (4000 * c->player.magiclvl /
		    (c->battle.opponent->experience + 6000 *
		    c->player.magiclvl) > RND() - .1) {

                sprintf(string_buffer, "%s is held.\n",
		        c->battle.opponent->name);

		Do_send_line(c, string_buffer);
                c->battle.opponent->speed = -2.0;
	        *theAttack = IT_PARALYZE;

		sprintf(string_buffer, "%s, %s, paralyze.\n",
			c->player.lcname,
			c->realm->charstats[c->player.type].class_name);

		Do_log(COMBAT_LOG, &string_buffer);
            }

            else {
                Do_send_line(c, "Monster unaffected.\n");
	        *theAttack = IT_NO_PARALYZE;

		sprintf(string_buffer, "%s, %s, no_paralyze.\n",
			c->player.lcname,
			c->realm->charstats[c->player.type].class_name);

		Do_log(COMBAT_LOG, &string_buffer);
	    }
        }

        break;

    case 8:   /* specify */

        if (c->player.special_type < SC_COUNCIL ||
		c->battle.opponent->special_type == SM_IT_COMBAT)

	    Do_send_line(c, "You can not cast that spell.\n"); 
        else if (c->player.mana < MM_SPECIFY)
	    Do_send_line(c, "You do not have enough mana.\n"); 
        else {

	    Do_mana(c, -MM_SPECIFY, FALSE);
	    event_ptr = (struct event_t *) Do_create_event();
	    event_ptr->type = MONSTER_EVENT;
	    event_ptr->arg1 = MONSTER_SPECIFY;

	    if (Do_long_dialog(c, &event_ptr->arg3,
		    "Which monster [0-99]?\n")) {

		free((void *)event_ptr);
		return;
	    }
	    if (event_ptr->arg3 >= NUM_MONSTERS || event_ptr->arg3 < 0) {
		event_ptr->arg3 = 14;
	    }

	    Do_handle_event(c, event_ptr);
	    Do_cancel_monster(c->battle.opponent);
        }

        break;
    }
}


/**/
/************************************************************************
/
/ FUNCTION NAME: Do_scramble_stats(struct client_t *c)
/
/ FUNCTION: scramble some selected statistics
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 6/9/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: floor(), drandom()
/
/ DESCRIPTION:
/       Swap two player statistics randomly (truncating gains).
/
*************************************************************************/

Do_scramble_stats(struct client_t *c)
{
double  dbuf[8];                 /* to put statistic in */
double  dtemp1;			 /* for swapping values */
int count = 0, swapee = 0;
float ftemp;

    /* fill buffer */
    dbuf[0] = c->player.max_strength;
    dbuf[1] = c->player.max_energy;
    dbuf[2] = c->player.mana;
    dbuf[3] = c->player.brains;
    dbuf[4] = c->player.magiclvl;
    dbuf[5] = c->player.sin;

    while (count == swapee) { 
        count = (int) ROLL(0, 6);
        swapee = (int) ROLL(0, 6);
    }

        /* swap the items */
    dtemp1 = dbuf[count];
    dbuf[count] = dbuf[swapee];
    dbuf[swapee] = dtemp1;

	/* assign a new strength */
    if (dbuf[0] > c->player.max_strength && (.95 > RND())) {
	dbuf[0] = c->player.max_strength;
    }

    Do_strength(c, dbuf[0], c->player.sword, c->battle.strengthSpell, FALSE);

	/* assign new energy */
    if (dbuf[1] > c->player.energy && (.95 > RND())) {
	dbuf[1] = c->player.energy;
    }

    Do_energy(c, c->player.energy, dbuf[1], c->player.shield,
	    c->battle.force_field, FALSE);

	/* assign new mana */
    if (dbuf[2] > c->player.mana && (.95 > RND())) {
	dbuf[2] = c->player.mana;
    }

    Do_mana(c, dbuf[2] - c->player.mana, FALSE); 

	/* assign new brains */
    if (dbuf[3] > c->player.brains && (.95 > RND())) {
	dbuf[3] = c->player.brains;
    }
    else if (dbuf[3] < 0) {
	dbuf[3] = 0.0;
    }

    c->player.brains = dbuf[3];

	/* assign new magic level */
    if (dbuf[4] > c->player.magiclvl && (.95 > RND())) {
	dbuf[4] = c->player.magiclvl;
    }
    else if (dbuf[4] < 0) {
	dbuf[4] = 0.0;
    }

    c->player.magiclvl = dbuf[4];

	/* assign new sin */
    if (dbuf[5] > c->player.sin && (.95 > RND())) {
	dbuf[5] = c->player.sin;
    }
    else if (dbuf[4] < 0) {
	dbuf[5] = 0.0;
    }

    c->player.sin = dbuf[5];

        /* force a smurf check to prevent people from resting on saruman */
    Do_sin(c, 0.0);
}


/************************************************************************
/
/ FUNCTION NAME: Do_starting_spells(struct client_t *c)
/
/ FUNCTION: scramble some selected statistics
/
/ AUTHOR: Brian Kelly, 11/9/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: floor(), drandom()
/
/ DESCRIPTION:
/       Swap a few player statistics randomly.
/
*************************************************************************/

Do_starting_spells(struct client_t *c)
{

	/* throw up spells */
    while (c->player.shield_nf) {

	Do_energy(c, c->player.energy, c->player.max_energy,
		c->player.shield, c->battle.force_field + 9 *
                c->player.max_energy, FALSE);

	Do_send_line(c, "A force field spell is cast upon you.\n");
	--c->player.shield_nf;
    }

    while (c->player.strong_nf) {

        if (c->player.sin < .5 * RND()) {
	    Do_strength(c, c->player.max_strength, c->player.sword,
		c->battle.strengthSpell + 9 * c->player.max_strength, FALSE);

	    Do_send_line(c, "Your strength is as the strength of ten, because your heart is pure.\n");
        } else {
	    Do_strength(c, c->player.max_strength, c->player.sword,
		c->battle.strengthSpell + 3 * c->player.max_strength, FALSE);

	    Do_send_line(c, "A strength spell is cast upon you.\n");
        }

	--c->player.strong_nf;
    }

    while (c->player.haste_nf) {

	Do_speed(c, c->player.max_quickness, c->player.quicksilver,
		c->battle.speedSpell + 10 + c->player.max_quickness, FALSE);

	Do_send_line(c, "A haste spell is cast upon you.\n");
	--c->player.haste_nf;
    }

    return;
}
