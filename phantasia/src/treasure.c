/*
 * treasure.c   Phantasia monster fighting routines
 */

#include "include.h"

/************************************************************************
/
/ FUNCTION NAME: Do_treasure(struct client_c *c)
/
/ FUNCTION: select a treasure
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 6/17/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: pickmonster(), collecttaxes(), more(), cursedtreasure(),
/       floor(), wmove(), drandom(), sscanf(), printw(), altercoordinates(),
/       longjmp(), infloat(), waddstr(), getanswer(), getstring(), wclrtobot()
/
/ GLOBAL INPUTS: Somebetter[], Curmonster, Whichmonster, Circle, Player,
/       *stdscr, Databuf[], *Statptr, Fightenv[]
/
/ GLOBAL OUTPUTS: Whichmonster, Shield, Player
/
/ DESCRIPTION:
/       Roll up a treasure based upon monster type and size, and
/       certain player statistics.
/       Handle cursed treasure.
/
*************************************************************************/

Do_treasure(struct client_t *c, struct event_t *the_event)
{
    struct event_t *event_ptr;
    int    whichtreasure;          /* calculated treasure to grant */
    long theAnswer;
    int itemp;
    char ch;                                /* input */
    double  gold = 0.0;                     /* gold awarded */
    double  gems = 0.0;                     /* gems awarded */
    double  dtemp, x_loc, y_loc;            /* for temporary calculations */
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE];
    float ftemp;
    struct realm_object_t *object_ptr;
    struct button_t buttons;
    static  char    *druidmesg[] =
        {
        "\tA blessing works perfectly if the bearer is free of any sin.\n",
        "\tA palantir can even pierce a cloak.\n",
        "\tA smurf berry a day keeps the medic away.\n",
        "\tA thaumaturgist can really put you in hot water!\n",
        "\tAll or nothing is your best friend against higher-level players.\n",
        "\tAmulets protect you from cursed treasure.\n",
        "\tBe careful to keep your sin low, or you may find yourself having a smurfy time.\n",
        "\tBe sure to rest if your speed drops from fatigue.\n",
        "\tBeware the Cracks of Doom!\n",
        "\tBeware the Jabberwock, my friend!  The jaws that bite, the claws that catch!\n",
        "\tBlindness wears off eventually... eventually.\n",
        "\tBuy amulets to protect your charms from being used on treasure.\n",
        "\tCatching a unicorn requires the virtue and control of a saint.\n",
        "\tDo not meddle in the affairs of the Game Wizards, for they are subtle and quick to anger.\n",
        "\tDo not ask to speak to a Game Wizard without giving your reason up front, or he will ignore you.\n",
        "\tDon't beg the Game Wizards for help with the game.\n",
        "\tDon't swear on channel 1.  Acronyms count, too.\n",
        "\tDwarves regenerate energy faster than other characters.\n",
        "\tElves are immune to the plague!\n",
        "\tExperimentos have been known to come back from the dead!\n",
        "\tFighting the Dark Lord leads to certain death...unless it's a Mimic!\n", "\tHalflings are said to be extremely lucky.\n",
        "\tIf the game isn't acting the way it should, report it to an Apprentice.\n",
        "\tIf your speed drops a lot, get rid of some of the heavy gold you are carrying.\n",
        "\tIt doesn't matter if you buy books one at a time or all at once.\n",
        "\tIt is impossible to teleport into the Dead Marshes except via the eagle Gwaihir.\n",
        "\tIt is very hard to walk through the Dead Marshes.\n",
        "\tKeep moving farther out, or you'll die from old age.\n",
        "\tListen to the Apprentices, they speak with the voice of the Game Wizards.\n",
        "\tMedics don't like liars, and punish accordingly.\n",
        "\tMedics don't work for charity.\n",
        "\tMerchants don't like players sleeping on their posts.\n",
        "\tOnly a moron would fight morons for experience.\n",
        "\tOnly a fool would dally with a Succubus or Incubus.\n",
        "\tOnly the Steward can give gold, but beware their smurfs if you ask too many times!\n",
        "\tParalyze can be a slow character's best friend.\n",
        "\tPicking up treasure while poisoned is bad for your health.\n",
        "\tReading the rules is a very good idea, and is often rewarded.\n",
        "\tRings of power contain much power, but much peril as well.\n",
        "\tSaintly adventurers may not have as much fun, but they tend to live longer.\n",
        "\tSmurfs may look silly, but they are actually quite deadly.\n",
        "\tStockpile amulets to protect your charms.\n",
        "\tTeleports are more efficient over short distances.\n",
        "\tThe corpse of a councilmember or a valar has never been found.\n",
        "\tThe One Ring is most likely to be found in the Cracks of Doom.\n",
        "\tThere are only three certainties in Phantasia : death, taxes, and morons.\n",
        "\tThe Game Wizards are always right.\n",
        "\tThe gods will revoke their blessing if you sin too much.\n",
        "\tThe nastier your poison, the more gold a medic will want to cure it.\n",
        "\tThere is a post in the Plateau of Gorgoroth that sells blessings.\n",
        "\tWant to live dangerously?  Nick a shrieker.\n",
        "\tWhen all else fails ... use all or nothing.\n",
        "\tWhen starting, do not spend too much money at once, or you may be branded a thief.\n",
        "\tWizards have been known to be nice if you are polite without being obsequious.\n",
        };

	/* Gold and gems only in circles that aren't fully beyond */
    if ((the_event->arg2 == 10) ||
        ((c->player.circle <= D_BEYOND / 88.388 && RND() > 0.65) && 
         !(the_event->arg2 > 0))) {

            /* gold and gems */
        if (the_event->arg3 > 7) {

                /* gems */
            gems = floor(ROLL(1.0, pow(the_event->arg3 - 7.0, 1.8) *
		    (the_event->arg1 - 1.0) / 4.0));

            if (gems == 1) {
                sprintf(string_buffer, "You have discovered a gem!\n");
	        Do_send_line(c, string_buffer);
                Do_send_line(c, "  Do you want to pick it up?\n");
            } else {
                sprintf(string_buffer, "You have discovered %.0lf gems!\n", gems);
	        Do_send_line(c, string_buffer);
                Do_send_line(c, "  Do you want to pick them up?\n");
            }
        }
        else {

                /* gold */
            gold = floor(ROLL(the_event->arg3 * 10.0, the_event->arg3 *
		    the_event->arg3 * 8.0 * (the_event->arg1 - 1.0)));

                /* avoid single gold pieces */
            if (gold == 1.0) {
                gold = 2.0;
            }

            sprintf(string_buffer, "You have found %.0lf gold pieces.\n", gold);
	    Do_send_line(c, string_buffer);
            Do_send_line(c, "  Do you want to pick them up?\n");

        }

        Do_clear_buttons(&buttons, 0);
        strcpy(buttons.button[5], "Yes\n");
        strcpy(buttons.button[6], "No\n");
        strcpy(buttons.button[7], "No to All\n");

        itemp = Do_buttons(c, &theAnswer, &buttons);

	if (itemp != S_NORM || theAnswer == 7) {

	    Do_orphan_events(c);
            Do_send_clear(c);
	    return;
	}
	else if (theAnswer == 6) {
            Do_send_clear(c);
	    return;
	}
        else if (theAnswer != 5) {

            sprintf(error_msg,
                    "[%s] Returned non-option %ld in Do_treasure.\n",
                    c->connection_id, theAnswer);

            Do_log_error(error_msg);
/*
            Do_caught_hack(c, H_SYSTEM);
*/
            return;
        }

        if (RND() < the_event->arg3 / 35.0 + 0.04) {

                /* cursed */
            Do_send_line(c, "They were cursed!\n");
            Do_cursed_treasure(c);
        }
        else {
	    Do_gold(c, gold, FALSE);
	    Do_gems(c, gems, FALSE);
	}
    }
    else {
            /* other treasures */
        Do_send_line(c,
		"You have found some treasure.  Do you want to inspect it?\n");

        Do_clear_buttons(&buttons, 0);
        strcpy(buttons.button[5], "Yes\n");
        strcpy(buttons.button[6], "No\n");
        strcpy(buttons.button[7], "No to All\n");

        itemp = Do_buttons(c, &theAnswer, &buttons);

	if (itemp != S_NORM || theAnswer == 7) {

	    Do_orphan_events(c);
            Do_send_clear(c);
	    return;
	}
	else if (theAnswer == 6) {
            Do_send_clear(c);
	    return;
	}
        else if (theAnswer != 5) {

            sprintf(error_msg,
                    "[%s] Returned non-option %ld in Do_treasure.\n",
                    c->connection_id, theAnswer);

            Do_log_error(error_msg);
/*
            Do_caught_hack(c, H_SYSTEM);
*/
            return;
        }

        if (RND() < 0.08 && the_event->arg3 != 4) {
            Do_send_line(c, "It was cursed!\n");
            Do_cursed_treasure(c);
        }
        else {

            if ((the_event->arg2 > 0) && (the_event->arg2 < 4)) {
            	whichtreasure = the_event->arg2;
            } else {
    	    	whichtreasure = (int) ROLL(1.0, 3.0);     /* pick a treasure */
            }

            switch (the_event->arg3) {

            case 1:     /* treasure type 1 */

                switch (whichtreasure) {

                case 1:
                    Do_send_line(c, "You've found a vial of holy water!\n");
		    Do_more(c);
                    ++c->player.holywater;
                    break;

                case 2:
		    dtemp = floor(sqrt(the_event->arg1));

                    if (dtemp < 1) {
                        dtemp = 1;
                    }
               
                    if (dtemp == 1) {
                        sprintf(string_buffer,
                                "You've found an amulet of protection.\n");
                    } else {
                        sprintf(string_buffer,
                                "You've found %.0lf amulets of protection.\n",
                                dtemp);
                    }

                    Do_send_line(c, string_buffer);
		    Do_more(c);
                    c->player.amulets += dtemp;

                    break;

                case 3:
                    Do_send_line(c,
			 "You have found a holy orb. You feel less sinful.\n");

		    Do_more(c);
		    Do_sin (c, -0.1);
                    break;
                }

                    /* end treasure type 1 */
                break;

            case 2:     /* treasure type 2 */

                switch (whichtreasure) {

                case 1:
		    if (c->player.sin < 9.5 * RND() + 0.5) {

                        Do_send_line(c,
	      "You have encountered a druid who teaches you the following words of wisdom:\n");

	                Do_send_line(c, druidmesg[(int) ROLL(0.0, (double) sizeof(druidmesg) / sizeof(char *))]);

		        Do_more(c);

		        Do_experience(c, ROLL(0.0, 2000.0 + the_event->arg1 *
			        750.0), FALSE);
		    }
		    else {

                         Do_send_line(c,
	      "You have encountered a druid.  He runs in fear for his life!\n");

		         Do_more(c);
		    }

                    break;

                case 2:
                    dtemp = floor((.5 + RND()) * 15 * the_event->arg1);

                    sprintf(string_buffer, "You've found a +%.0lf buckler.\n",
			    dtemp);

		    Do_send_line(c, string_buffer);

                    if (dtemp >= c->player.shield) {

		        Do_more(c);

			Do_energy(c, c->player.energy - c->player.shield +
				dtemp, c->player.max_energy, dtemp, 0, FALSE);
		    }
                    else {

                        Do_send_line(c,
			       "But you already have something better.\n");

		        Do_more(c);
		    }
                    break;

                case 3:

		    if (c->player.poison > 0.0) {

		        Do_adjusted_poison(c, -0.25);
                        if (c->player.poison < 0.0) {
                            c->player.poison = 0.0;
                            Do_send_line(c,
		     "You've found some smurf berries!  You feel cured!\n");

                        } else {
                            Do_send_line(c,
		     "You've found some smurf berries!  You feel slightly better.\n");
                        }
		    }
		    else {

                        Do_send_line(c,
		     "You've found some smurf berries!  You feel smurfy!\n");

                        c->battle.rounds /= 2;
		        Do_speed(c, c->player.max_quickness, c->player.quicksilver, 0, FALSE);
                        Do_energy(c, 
                        c->player.energy + 
                         (c->player.max_energy + c->player.shield) / 15.0 + 
                         c->player.level / 3.0 + 2.0, 
                        c->player.max_energy,
	                c->player.shield, c->battle.force_field, FALSE);
		    }

		    Do_more(c);

                    break;
                }

                    /* end treasure type 2 */
                break;

            case 3:     /* treasure type 3 */

                switch (whichtreasure) {

                case 1:

                    Do_send_line(c,
		    "You've met a hermit!  You heal, gain mana, and lose sin.\n");

		    Do_more(c);

                    if (c->player.sin > 6.66) { 
                        Do_sin(c, -1.0);
                    } else {
		        Do_sin(c, -0.15 * c->player.sin);
                    }

                    Do_mana(c, 
                            c->realm->charstats[c->player.type].mana.increase
                            / 2 * the_event->arg1, FALSE);
                    Do_energy(c, 
                        c->player.energy + 
                         (c->player.max_energy + c->player.shield) / 7.0 + 
                         c->player.level / 3.0 + 2.0, 
                        c->player.max_energy,
	                c->player.shield, c->battle.force_field, FALSE);
                    break;

                case 2:

		    Do_award_virgin(c);
                    break;

                case 3:
                    dtemp = floor((.5 + RND()) * the_event->arg1);

                    if (dtemp < 1) { 
                        dtemp = 1;
                    }

                    sprintf(string_buffer,
			    "You've found a +%.0lf short sword!\n", dtemp);

		    Do_send_line(c, string_buffer);
                    if (dtemp >= c->player.sword) {

		        Do_more(c);

			Do_strength(c, c->player.max_strength, dtemp, 0,
				FALSE);
		    }
                    else {

                        Do_send_line(c,
				"But you already have something better.\n");

		        Do_more(c);
		    }
                }

                    /* end treasure type 3 */
                break;

            case 4:     /* treasure type 4 */

                if (c->player.blind) {
                    Do_send_line(c, "You've found a scroll.  Too bad you are blind!\n");
		    Do_more(c);
                    break;
                }
                       

                Do_send_line(c, "You've found a scroll.  Will you read it?\n");

                if (Do_yes_no(c, &theAnswer) != S_NORM || theAnswer == 1) {
            	    Do_send_clear(c);
		    return;
		}
 
                if ((the_event->arg2 > 0) && (the_event->arg2 < 7)) {
                    dtemp = the_event->arg2;
                } else {
    	     	    dtemp = (int) ROLL(1, 6);     /* pick a treasure */
                }

                if (dtemp == 1) {
                  if (c->player.level <= 100 - c->player.gems) {
                    Do_treasure_map(c);
                  } else {
                    /* character is too high level, pick another scroll */
	  	    dtemp = (int) ROLL(2, 5);
 		  }
		}

		
                switch ((int) dtemp) {

                case 2:

                    Do_send_line(c,
			      "It throws up a shield for your next monster.\n");

		    Do_more(c);
		    ++c->player.shield_nf;
		    break;

                case 3:

                    Do_send_line(c, 
			     "It makes you faster for your next monster.\n");

		    Do_more(c);
		    ++c->player.haste_nf;
		    break;

                case 4:

                    Do_send_line(c,
			"It increases your strength for your next monster.\n");

		    Do_more(c);
		    ++c->player.strong_nf;
		    break;


                case 5:

                    Do_send_line(c,
			     "It tells you how to pick your next monster.\n");

		    Do_more(c);
		    event_ptr = (struct event_t *) Do_create_event();
		    event_ptr->type = MONSTER_EVENT;
		    event_ptr->arg1 = MONSTER_SPECIFY;
                    if (Do_long_dialog(c, &event_ptr->arg3,
			    "Which monster do you want [0-99]?\n")) {

			free((void *)event_ptr);
			break;
		    }

		    if (event_ptr->arg3 >= 0 && event_ptr->arg3 <
			    NUM_MONSTERS) {

			Do_file_event(c, event_ptr);
		    }

		    break;

                case 6:
                    Do_send_line(c, "It was cursed!\n");
                    Do_cursed_treasure(c);
                    break;

                }
                    /* end treasure type 4 */
                break;

            case 5:     /* treasure type 5 */

                switch (whichtreasure) {

                case 1:

                    Do_send_line(c,
		  "You've discovered a power booster!  Your mana increases.\n");

		    Do_more(c);

                    Do_mana(c, 
                            c->realm->charstats[c->player.type].mana.increase
		              * 2 * the_event->arg1, FALSE);
                    break;

                case 2:

                    dtemp = floor((.5 + RND()) * 50.0 * the_event->arg1);

                    sprintf(string_buffer, "You've found a +%.0lf shield!\n",
			    dtemp);

		    Do_send_line(c, string_buffer);
                    if (dtemp >= c->player.shield) {

		        Do_more(c);

			Do_energy(c, c->player.energy - c->player.shield +
				dtemp, c->player.max_energy, dtemp, 0, FALSE);
		    }
                    else {

                        Do_send_line(c,
			        "But you already have something better.\n");

		        Do_more(c);
		    }
                    break;

                case 3:

		    if (c->player.poison > 0.0) {
		        Do_adjusted_poison(c, -1.0);
                        Do_send_line(c, "You've discovered some lembas!  You feel much better!\n");
                        if (c->player.poison < 0.0) {
                            c->player.poison = 0.0;
                        }

                        c->battle.rounds = 0;
		        Do_speed(c, c->player.max_quickness, c->player.quicksilver, 0, FALSE);
                    } else {
                        Do_send_line(c, "You've discovered some lembas!  You feel energetic!\n");
                        c->battle.rounds = 0;
		        Do_speed(c, c->player.max_quickness, c->player.quicksilver, 0, FALSE);
                    }

                    Do_energy(c, 
                        c->player.energy + 
                         (c->player.max_energy + c->player.shield) / 3.0 + 
                         c->player.level / 3.0 + 2.0, 
                        c->player.max_energy,
	                c->player.shield, c->battle.force_field, FALSE);

		    Do_more(c);

                    break;
                }

                    /* end treasure type 5 */
                break;

            case 6:     /* treasure type 6 */

                switch (whichtreasure) {

                case 1:

                    if (c->player.blind) { 
                        Do_send_line(c, "You've discovered a tablet!  But you can't read it while blind!\n");
                        Do_more(c);
                    } else {
     
                        Do_send_line(c, "You've discovered a tablet!  You feel smarter.\n");

                        Do_more(c);

  	                c->player.brains +=
			   c->realm->charstats[c->player.type].brains.increase
			   * (.75 + RND() / 2) * (1 + the_event->arg1 / 10);
                    }

                    break;

                case 2:

		    if (c->player.sin * RND() < 2.0) {

                        Do_send_line(c,
	        "You have come upon Treebeard!  He gives you some miruvor and you feel tougher!\n");

		        Do_more(c);

		        dtemp =
			    c->realm->charstats[c->player.type].energy.increase
			    * (.75 + RND() / 2) * (1 + the_event->arg1 / 5);

		        Do_energy(c, 
                                c->player.max_energy + dtemp + c->player.shield,
			        c->player.max_energy + dtemp, c->player.shield,
				0, FALSE);
		    }
		    else {

                        Do_send_line(c,
	 "You have come upon Treebeard! 'Hoom, hom!' he growls, and leaves.\n");

		        Do_more(c);
		    }

                    break;

                case 3:

                    dtemp = floor((.5 + RND()) * 2 * the_event->arg1);

                    sprintf(string_buffer,
			    "You've found a +%.0lf long sword!\n", dtemp);

		    Do_send_line(c, string_buffer);
                    if (dtemp >= c->player.sword) {

		        Do_more(c);

			Do_strength(c, c->player.max_strength, dtemp, 0,
				FALSE);
		    }
                    else {

                        Do_send_line(c,
				"But you already have something better.\n");

		        Do_more(c);
		    }
                    break;
		}

                    /* end treasure type 6 */
                break;

            case 7:     /* treasure type 7 */

                switch (whichtreasure) {

                case 1:

		    if (c->player.sin < 3 * RND() / 5) {
                        Do_send_line(c, 
                    "You've found an Aes Sedai.  She bows and completely cleanses you of the taint.\n");

		        Do_more(c);

		        Do_sin(c, -(c->player.sin));

                    } else {
                        Do_send_line(c, 
                    "You've found an Aes Sedai.  She sniffs loudly and cleanses some of your taint.\n");
		        Do_more(c);

                        if (c->player.sin > 4.5) {
                             Do_sin(c, -1.5);
                        } else {
		             Do_sin(c, -0.33 * c->player.sin);
                        }
                    }


                    break;

                case 2:

		    if (c->player.sin * RND() < 2.5) {

                        Do_send_line(c,
   "You release Gwaihir and he offers you a ride.  Do you wish to go anywhere?\n");

                        if (Do_yes_no(c, &theAnswer) != S_NORM || theAnswer ==
				1) {

			    break;
		        }


		        event_ptr = (struct event_t *) Do_create_event();
		        event_ptr->type = TELEPORT_EVENT;
                        event_ptr->arg2 = TRUE;
                        event_ptr->arg3 = FALSE;
		        Do_handle_event(c, event_ptr);
                        break;
		    }
		    else {

                        Do_send_line(c,
       "You release Gwaihir!  He thanks you for his freedom and flies off.\n");

			Do_more(c);
		    }
                    break;

                case 3:

		    if (c->player.sin * RND() < 2.0) {

                        Do_send_line(c, 
                "You have come upon Hercules!  He improves your strength and experience!\n");
		        Do_more(c);

		        Do_strength(c, c->player.max_strength +
			  c->realm->charstats[c->player.type].strength.increase
			  * (.75 + RND() / 2) * (1 + the_event->arg1 / 5),
			  c->player.sword, 0, FALSE);

		        Do_experience(c, 10000 * c->player.circle);
		    }
		    else {

			Do_send_line(c,
  "You have come upon Hercules!  He kicks sand in your face and walks off.\n");

			Do_more(c);
		    }

                    break;
                }

                    /* end treasure type 7 */
                break;

            case 8:     /* treasure type 8 */

                switch (whichtreasure) {

                case 1:
                    Do_send_line(c,
 "You've discovered some athelas!  You inhale its fragrance and feel wonderful!\n");

		    Do_more(c);


                    /* remove blindness */
                    if (c->player.blind) {
                        c->player.blind = FALSE;
                    }

                    /* zero out poison */
		    if (c->player.poison > 0.0) {
		        Do_poison(c, (double) -c->player.poison);
		    }

                    /* make player immune to next plague at this circle */
		    Do_adjusted_poison(c, -1.0);

                           
                    /* heal the player completely and remove fatigue */
		    Do_energy(c, c->player.max_energy + c->player.shield,
			    c->player.max_energy, c->player.shield, 0, FALSE);

                    c->battle.rounds = 0;
		    Do_speed(c, c->player.max_quickness, c->player.quicksilver, 0, FALSE);
                    break;

                case 2:

		    if (c->player.sin * RND() < 1.0) {

                        Do_send_line(c, "You have encountered Merlyn!  He teaches you magic.\n");
		        Do_more(c);

                        c->player.magiclvl +=
			  c->realm->charstats[c->player.type].magiclvl.increase
			  * (.75 + RND() / 2) * (1 + the_event->arg1 / 10);

                        Do_mana(c, 
                            c->realm->charstats[c->player.type].mana.increase 
                            * the_event->arg1, FALSE);
		    }
		    else {

			Do_send_line(c,
	        "You have encountered Merlyn! He frowns and teleports off.\n");

			Do_more(c);
		    }

                    break;


                case 3:

                    dtemp = (.75 + RND() / 2) * .8 * (the_event->arg1 - 9);

		    if (dtemp < 1) {
		        dtemp = 1;
		    }

                    sprintf(string_buffer,
		      "You have discovered some +%.0lf quicksilver!\n", dtemp);

		    Do_send_line(c, string_buffer);
                    if (dtemp >= c->player.quicksilver) {
		        Do_more(c);
			Do_speed(c, c->player.max_quickness, dtemp, 0, FALSE);
		    }
                    else {

                        Do_send_line(c,
			           "But you already have something better.\n");

		        Do_more(c);
		    }
		    break;
		}

		/* end treasure type 8 */
            break;

            case 10:
            case 11:
            case 12:
            case 13:
            case 14:    /* treasure types 10 - 14 */
                if (RND() < 0.33 || the_event->arg2 == 4) {

			/* Ungoliant treasure */
                    if (the_event->arg3 == 12) {

                        Do_send_line(c, 
                  "You've found a Silmaril!  Its light quickens your step!\n");

			Do_more(c);

                        if (c->player.max_quickness < 
                            c->realm->charstats[c->player.type].quickness.base +
                            c->realm->charstats[c->player.type].quickness.interval) {
                            Do_speed(c, c->player.max_quickness + 2.0,
				c->player.quicksilver, 0, FALSE);
                        } else {
                            Do_speed(c, c->player.max_quickness + 1.0,
				c->player.quicksilver, 0, FALSE);
                        }

                        break;
                    } 

			/* Saruman treasure */
                    else if (the_event->arg3 == 11) {
                      if (!c->player.palantir) {

                        Do_send_line(c,
				"You've acquired Saruman's palantir.\n");

			Do_more(c);
                        Do_palantir(c, TRUE, FALSE);
                        break;
                      } else {

                        Do_send_line(c,
                                "You've rescued Gandalf!  He heals you, and casts some spells on you for your next combat!\n");
		        Do_more(c);

		        Do_energy(c, c->player.max_energy + c->player.shield,
			    c->player.max_energy, c->player.shield, 0, FALSE);

                        c->player.blind = FALSE;

                        c->battle.rounds = 0;
		        Do_speed(c, c->player.max_quickness, c->player.quicksilver, 0, FALSE);

                        if (c->player.sin < RND()) {
		            ++c->player.strong_nf;
                        }

                        if (c->player.sin < RND()) {
		            ++c->player.haste_nf;
                        }

                        if (c->player.sin < RND()) {
		            ++c->player.shield_nf;
                        }

                        if (c->player.sin * 2 < RND()) {

		            event_ptr = (struct event_t *) Do_create_event();
		            event_ptr->type = MONSTER_EVENT;
		            event_ptr->arg1 = MONSTER_SPECIFY;
                            if (Do_long_dialog(c, &event_ptr->arg3,
			        "Which monster do you want [0-99]?\n")) {

			        free((void *)event_ptr);
			        break;
		            }

		            if (event_ptr->arg3 > 0 && event_ptr->arg3 <
			            NUM_MONSTERS) {

			        Do_file_event(c, event_ptr);
		            }
                        }

                        break;
                      }
                    }

			/* Nazgul treasure */
                    else if (c->player.ring_type == R_NONE
                            && c->player.special_type < SC_COUNCIL
                            && the_event->arg3 == 10) {

                        Do_send_line(c,
			  "You've discovered a ring of power.  Will you pick it up?\n");

                        if (Do_yes_no(c, &theAnswer) != S_NORM || theAnswer ==
				1) {

			    break;
			}

                            /* roll up a ring */
                        if (RND() < 0.8) {

                                /* regular ring */
			    Do_ring(c, R_NAZREG, FALSE);
			}
                        else {

                                /* bad ring */
			    Do_ring(c, R_BAD, FALSE);
		        }
			break;
		    }

			/* Dark Lord treasure */
		    else if (the_event->arg3 == 14) {

			    /* drop a ring */
                        if (c->player.special_type < SC_KING
                            && the_event->arg1 > 200 
                            && c->player.ring_type != R_DLREG) {
							Do_sin(c, RND());
                            Do_send_line(c,
			  "You've discovered The One Ring, the Ring to Rule Them All.  Will you pick it up?\n");

                            if (Do_yes_no(c, &theAnswer) != S_NORM || theAnswer
				== 1) {

			        break;
			    }

                            if (c->player.ring_type != R_NONE) {
			        Do_send_line(c, "Your old ring no longer feels very precious to you and you throw it away.\n");
                                Do_more(c);
                            }
			    Do_ring(c, R_NONE, FALSE);
                                   

                                /* roll up a ring */
                            if (RND() < 0.8) {

                                    /* regular ring */
			        Do_ring(c, R_DLREG, FALSE);
                            }
                            else {

                                    /* bad ring */
			        Do_ring(c, R_BAD, FALSE);
			    }
			    break;
		        } else if (!c->player.palantir) {
                           
                            Do_send_line(c,
				"You've acquired The Dark Lord's palantir.\n");

			    Do_more(c);
                            Do_palantir(c, TRUE, FALSE);
                            break;
                        } else if (c->player.special_type < SC_KING) {
                            Do_send_line(c,
                              "You've discovered a ring of power.  Will you pick it up?\n");

                            if (Do_yes_no(c, &theAnswer) != S_NORM || theAnswer ==
                                    1) {

                                break;
                            }

                                /* roll up a ring */
                            if (RND() < 0.95) {

                                    /* regular ring */
                                Do_ring(c, R_NAZREG, FALSE);
                            }
                            else {

                                    /* bad ring */
                                Do_ring(c, R_BAD, FALSE);
                            }
                            break;
                        }
                            
		    }
		}

                    /* end treasure types 10 - 13 */
                /* fall through to treasure type 9 if no treasure from above */

            case 9: /* treasure type 9 */

                switch (whichtreasure) {

                    /* staff, volcano, smith */
                case 1:

                    if ((c->player.level < MAX_STEWARD) &&
                        (c->player.level > 10) &&
                        (c->player.crowns < 1) &&
                        (c->player.special_type != SC_STEWARD) &&
                        (CALCLEVEL(c->player.experience) == c->player.level) 
                       ) {

                        Do_send_line(c, "You have found a staff!\n");

			Do_more(c);
			Do_crowns(c, 1, FALSE);
                    } else if ((c->player.circle > 26) &&
                               (c->player.circle < 29)) {
                        Do_volcano(c);
                    } else {
		        Do_smith(c, the_event);
                    }

                    break;

                case 2:
                    Do_send_line(c, "You've received the blessing of the Valar for your heroism!\n");
                    Do_more(c);
                    Do_award_blessing(c);
                    break;

                    /* fall through otherwise */

                case 3:
		    dtemp = floor(ROLL(floor(2 * sqrt(the_event->arg1)), 
                                       the_event->arg3 * .5 * 
                                       (sqrt(the_event->arg1))));

                    if (dtemp < 1) {
                        dtemp = 1;
                    }

                    if (dtemp == 1) {
                        sprintf(string_buffer,
                                "You've discovered a charm!\n");
                    } else {
                        sprintf(string_buffer,
			        "You've discovered %.0lf charms!\n",
			        dtemp);
                    }

                    Do_send_line(c, string_buffer);
		    Do_more(c);
                    c->player.charms += dtemp;

                    break;
                }
                    /* end treasure type 9 */
                break;
	    }

        }
    }

    Do_send_clear(c);
}


/************************************************************************
/
/ FUNCTION NAME: Do_cursed_treasure(struct client_t *c)
/
/ FUNCTION: take care of cursed treasure
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 6/17/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: waddstr()
/
/ GLOBAL INPUTS: Player, *stdscr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle cursed treasure.  Look for amulets and charms to save
/       the player from the curse.
/
*************************************************************************/

Do_cursed_treasure(struct client_t *c)
{
    float ftemp;
    double dtemp;

    if (c->player.amulets >= floor(sqrt(c->player.circle))) {
        Do_send_line(c, "But your amulets saved you!\n");
	Do_more(c);
        c->player.amulets -= floor(sqrt(c->player.circle));
    }
    else if (c->player.charms > 0) {
        Do_send_line(c, "But your charm saved you!\n");
	Do_more(c);
        --c->player.charms;
    }
    else {
	Do_more(c);
/*
	Do_energy(c, MIN(c->player.energy, (c->player.max_energy +
		c->player.shield) / 10.0), c->player.max_energy,
		c->player.shield, 0, FALSE);
*/
	dtemp = c->player.energy - RND() * (c->player.max_energy +
		c->player.shield) / 3.0;

	if (dtemp < c->player.max_energy / 10.0) {
	    dtemp =  MIN(c->player.energy, c->player.max_energy / 10.0);
	}

	Do_energy(c, dtemp, c->player.max_energy, c->player.shield, 0, FALSE);

	Do_adjusted_poison(c, 0.25);
    }

    Do_send_clear(c);
}


/************************************************************************
/
/ FUNCTION NAME: Do_corpse(struct client_t *c, struct event_t *the_event)
/
/ FUNCTION: take care of cursed treasure
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 6/17/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: waddstr()
/
/ GLOBAL INPUTS: Player, *stdscr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle cursed treasure.  Look for amulets and charms to save
/       the player from the curse.
/
*************************************************************************/

Do_corpse(struct client_t *c, struct event_t *the_event)
{
    struct player_t *the_player;
    struct event_t *event_ptr;
    char string_buffer[SZ_LINE];
    int theAnswer;
    double dtemp;

    the_player = (struct player_t *)the_event->arg4;

	/* check the cursed flag */
    if (the_event->arg1) {
        sprintf(string_buffer, "You have discovered the corpse of %s.\n",
	       the_player->name);

        Do_send_line(c, string_buffer);
        Do_send_line(c, "Do you wish to frisk the body?\n");

        if (Do_yes_no(c, &theAnswer) != S_NORM || theAnswer == 1) {

            Do_send_clear(c);
            Do_send_line(c, "The body crumbles to ashes.\n");
            Do_more(c);
            Do_send_clear(c);
	    return;
	}

        Do_send_clear(c);

	    /* Fresh discovered corpses have an 75% chance of being cursed */
	    /* This probability drops to nothing over CORPSE_LIFE days */
        if (RND() < 0.75 - 0.75 * (time(NULL) - c->player.last_load) /
		CORPSE_LIFE) {

            free((void *)the_player);
	    Do_send_line(c, "The body was cursed!\n");
	    Do_cursed_treasure(c);
	    return;
        }
    }
    else {
        sprintf(string_buffer, "You search the defeated carcass of %s.\n",
	       the_player->name);

        Do_send_line(c, string_buffer);
    }

    if (the_player->gold > 0.0) {
	sprintf(string_buffer, "You find %.0lf gold.\n", the_player->gold);
	Do_send_line(c, string_buffer);
	Do_gold(c, the_player->gold, FALSE);
    }

    if (the_player->gems > 1.0) {
	sprintf(string_buffer, "You find %.0lf gems.\n", the_player->gems);
	Do_send_line(c, string_buffer);
	Do_gems(c, the_player->gems, FALSE);
    } else if (the_player->gems > 0.0) {
	sprintf(string_buffer, "You find %.0lf gem.\n", the_player->gems);
	Do_send_line(c, string_buffer);
	Do_gems(c, the_player->gems, FALSE);
    }

    the_player->sword = floor(the_player->sword / 2);
    if (the_player->sword > c->player.sword) {

	sprintf(string_buffer, "You find a %.0lf strength sword.\n",
		the_player->sword);

	Do_send_line(c, string_buffer);
	Do_strength(c, c->player.max_strength, the_player->sword, 0, FALSE);
    }

    the_player->shield = floor(the_player->shield / 2);
    if (the_player->shield > c->player.shield) {

	sprintf(string_buffer, "You find a %.0lf strength shield.\n",
		the_player->shield);

	Do_send_line(c, string_buffer);

	Do_energy(c, c->player.energy - c->player.shield +
		the_player->shield, c->player.max_energy,
		the_player->shield, 0, FALSE);
    }

    the_player->quicksilver = floor(the_player->quicksilver / 4);
    if (the_player->quicksilver > c->player.quicksilver) {

	sprintf(string_buffer, "You find %.0f quicksilver.\n",
		the_player->quicksilver);

	Do_send_line(c, string_buffer);
	Do_speed(c, c->player.max_quickness, the_player->quicksilver, 0,
		FALSE);
     }

     if (the_player->holywater > 0) {

	sprintf(string_buffer, "You find %hd holy water.\n",
		the_player->holywater);

	Do_send_line(c, string_buffer);
	c->player.holywater += the_player->holywater;
    }

    if (the_player->amulets > 0) {

	sprintf(string_buffer, "You find %hd amulets.\n",
		the_player->amulets);

	Do_send_line(c, string_buffer);
	c->player.amulets += the_player->amulets;
    }

    if (the_player->charms > 0) {

	sprintf(string_buffer, "You find %hd charms.\n",
		the_player->charms);

	Do_send_line(c, string_buffer);
	c->player.charms += the_player->charms;
    }

    if (the_player->virgin) {
	Do_award_virgin(c);
    }

    free((void *)the_player);

    Do_more(c);
    Do_send_clear(c);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_award_blessing(struct client_t *c)
/
/ FUNCTION: take care of cursed treasure
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 9/6/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: waddstr()
/
/ GLOBAL INPUTS: Player, *stdscr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle cursed treasure.  Look for amulets and charms to save
/       the player from the curse.
/
*************************************************************************/

Do_award_blessing(struct client_t *c)
{
    float ftemp;

    Do_blessing(c, TRUE, FALSE);

    Do_sin(c, -0.25 * c->player.sin);

    Do_energy(c, c->player.max_energy + c->player.shield,
	    c->player.max_energy, c->player.shield, 0.0, FALSE);

    Do_mana(c, 100.0 * c->player.circle, FALSE);
}


/************************************************************************
/
/ FUNCTION NAME: Do_award_virgin(struct client_t *c)
/
/ FUNCTION: take care of cursed treasure
/
/ AUTHOR: E. A. Estes, 12/4/85
/	  Brian Kelly, 9/6/99
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: waddstr()
/
/ GLOBAL INPUTS: Player, *stdscr
/
/ GLOBAL OUTPUTS: Player
/
/ DESCRIPTION:
/       Handle cursed treasure.  Look for amulets and charms to save
/       the player from the curse.
/
*************************************************************************/

Do_award_virgin(struct client_t *c)
{
    int theAnswer;
    float ftemp;

    if (c->player.gender == MALE) {

        Do_send_line(c,
     "You have rescued a virgin. Do you wish you succumb to your carnal desires?\n");

        if (Do_yes_no(c, &theAnswer) != S_NORM) {
	    return;
	}

	if (theAnswer == 1) {
	    Do_virgin(c, TRUE, FALSE);
	}
        else {
	    Do_experience(c, 2500.0 * c->player.circle, FALSE);
	    Do_sin(c, 1.0);
        }
    }
    else {

        Do_send_line(c,
           "You have rescued a virgin.  Do you wish to sacrifice her now?\n");

        if (Do_yes_no(c, &theAnswer) != S_NORM) {
	    return;
	}

	if (theAnswer == 1) {
	    Do_virgin(c, TRUE, FALSE);
	}
        else {
	    Do_experience(c, 2500.0 * c->player.circle, FALSE);
	    Do_sin(c, 1.0);
        }
    }
}


/************************************************************************
/
/ FUNCTION NAME: Do_treasure_trove(struct client_c *c)
/
/ FUNCTION: select a treasure
/
/ AUTHOR: Brian Kelly, 5/9/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: pickmonster(), collecttaxes(), more(), cursedtreasure(),
/       floor(), wmove(), drandom(), sscanf(), printw(), altercoordinates(),
/       longjmp(), infloat(), waddstr(), getanswer(), getstring(), wclrtobot()
/
/ GLOBAL INPUTS: Somebetter[], Curmonster, Whichmonster, Circle, Player,
/       *stdscr, Databuf[], *Statptr, Fightenv[]
/
/ GLOBAL OUTPUTS: Whichmonster, Shield, Player
/
/ DESCRIPTION:
/       Roll up a treasure based upon monster type and size, and
/       certain player statistics.
/       Handle cursed treasure.
/
*************************************************************************/

Do_treasure_trove(struct client_t *c)
{
    struct event_t *event_ptr;

    double gems = 0.0;                     /* gems awarded */
    double charms = 0.0;                   /* charms awarded */
    char string_buffer[SZ_LINE];

    Do_send_line(c, "You have found a treasure trove!\n");

    gems = 1 + floor(c->player.circle * (RND() * .33));

    if (!c->player.blessing) {
        sprintf(string_buffer, "You find %.0lf gems and a blessing!\n", gems);
        Do_award_blessing(c);
    } else if (c->player.charms < 32) {
        charms = 1 + floor(10 * RND() * sqrt(c->player.circle));

        sprintf(string_buffer, "You find %.0lf gems and %.0lf charms!\n", 
                gems, charms);

        c->player.charms += charms;
    } else {
        gems += floor(2 + RND() * 3);
        sprintf(string_buffer, "You find %.0lf gems!\n", gems);
    }

    Do_send_line(c, string_buffer);
    Do_more(c);
    Do_send_clear(c);

    Do_gems(c, gems, FALSE);

    sprintf(string_buffer,
	"%s finds and unearths the treasure trove.  The hunt begins anew...\n",
        c->modifiedName);

    Do_broadcast(c, string_buffer);

    return;
}

/************************************************************************
/
/ FUNCTION NAME: Do_treasure_map(struct client_c *c)
/
/ FUNCTION: output a treasure map
/
/ AUTHOR: Brian Kelly and Eugene Hung, 7/26/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: 
/
/ GLOBAL INPUTS: 
/
/ GLOBAL OUTPUTS: 
/
/ DESCRIPTION:
/       Generate a treasure map.
/
*************************************************************************/

Do_treasure_map(struct client_t *c) {

    double  dtemp, x_loc, y_loc;           /* for temporary calculations */
    struct realm_object_t *object_ptr;
    char string_buffer[SZ_LINE], error_msg[SZ_ERROR_MESSAGE];

        /* find the treasure trove */
    Do_lock_mutex(&c->realm->realm_lock);
    object_ptr = c->realm->objects;

        /* run through the realm objects */
    while (object_ptr != NULL) {

        if (object_ptr->type == TREASURE_TROVE) {
            break;
        }

        object_ptr = object_ptr->next_object;
    }

    if (object_ptr == NULL) {

	Do_unlock_mutex(&c->realm->realm_lock);

        sprintf(error_msg,
             "[%s] No trove found in realm objects in Do_treasure.\n",
             c->connection_id);

        Do_log_error(error_msg);

        Do_send_line(c, "It was cursed!\n");
        Do_cursed_treasure(c);
        return;
    }

    x_loc = object_ptr->x;
    y_loc = object_ptr->y;

    Do_unlock_mutex(&c->realm->realm_lock);

          /* determine the distance to the trove */
    Do_distance(c->player.x, x_loc, c->player.y, y_loc, &dtemp);

    

    if (dtemp > 1.0) {
          /* throw in a fudge factor of up to 12.5% if not near trove */
        dtemp = floor(dtemp * (.875 + RND() * .25) + .01);

        sprintf(string_buffer,
     "It says, 'To find me treasure trove, ye must move %.0lf squares to the ",
     dtemp);

        Do_direction(c, &x_loc, &y_loc, string_buffer);

	strcat(string_buffer, " and then look for me next map.\n");
    } else if (dtemp == 1.0) {

        strcpy(string_buffer,
		  "Arr, you're almost there.  The booty is 1 square ");

	        Do_direction(c, &x_loc, &y_loc, string_buffer);
		strcat(string_buffer, ".\n");

    } else {
        strcpy(string_buffer, "You've found the treasure!  Dig matey, dig!\n");
    }

    Do_send_line(c, string_buffer);
    Do_more(c);

    return;
}


/************************************************************************
/
/ FUNCTION NAME: Do_smith(struct client_c *c, struct event_t *the_event)
/
/ FUNCTION: select a treasure
/
/ AUTHOR: Brian Kelly, 5/9/01
/
/ ARGUMENTS: none
/
/ RETURN VALUE: none
/
/ MODULES CALLED: pickmonster(), collecttaxes(), more(), cursedtreasure(),
/       floor(), wmove(), drandom(), sscanf(), printw(), altercoordinates(),
/       longjmp(), infloat(), waddstr(), getanswer(), getstring(), wclrtobot()
/
/ GLOBAL INPUTS: Somebetter[], Curmonster, Whichmonster, Circle, Player,
/       *stdscr, Databuf[], *Statptr, Fightenv[]
/
/ GLOBAL OUTPUTS: Whichmonster, Shield, Player
/
/ DESCRIPTION:
/       Roll up a treasure based upon monster type and size, and
/       certain player statistics.
/       Handle cursed treasure.
/
*************************************************************************/

Do_smith(struct client_t *c, struct event_t *the_event)
{
    double  dtemp;
    char error_msg[SZ_ERROR_MESSAGE];
    struct button_t buttons;
    long answer;
    int rc;

    if (c->player.sin < RND() * 2.0) {

        Do_send_line(c,
	       "Wayland Smith offers to improve a piece of your equipment!\n");

        Do_clear_buttons(&buttons, 0);
        answer = FALSE;

	if (c->player.shield > 0) {
            strcpy(buttons.button[0], "Shield\n");
	    answer = TRUE;
	}

	if (c->player.sword > 0) {
            strcpy(buttons.button[1], "Sword\n");
	    answer = TRUE;
	}

	if (c->player.quicksilver > 0) {
            strcpy(buttons.button[2], "Quicksilver\n");
	    answer = TRUE;
	}

        strcpy(buttons.button[7], "Cancel\n");

	if (!answer) {

	    Do_send_line(c,
		    "He then sees you have none, guffaws and leaves.\n");

	    Do_more(c);
	    return;
	}

        rc = Do_buttons(c, &answer, &buttons);

        if (rc != S_NORM) {
            answer = 7;
        }

            /* switch on the player's answer */
        switch (answer) {

            /* upgrade shield */
        case 0:

		/* determine the shield bonus */
	    dtemp = .018 * c->player.shield;

	    if (dtemp > 1 + 60 * the_event->arg1) {
	        dtemp = 1 + 60 * the_event->arg1;
	    }

	    if (dtemp < 1) {
		dtemp = 1;
	    }

		/* award the bonus */
	    Do_energy(c, c->player.energy, c->player.max_energy,
		    c->player.shield + dtemp, 0, FALSE);

            break;

	case 1:

		/* determine the sword bonus */
	    dtemp = .018 * c->player.sword;

	    if (dtemp > 1 + 2 * the_event->arg1) {
	        dtemp = 1 + 2 * the_event->arg1;
	    }

	    if (dtemp < 1) {
		dtemp = 1;
	    }

		/* award the bonus */
	    Do_strength(c, c->player.max_strength, c->player.sword + dtemp,
		    0, FALSE);

	    break;

	case 2:

		/* determine the quicksilver bonus */
	    dtemp = .018 * c->player.quicksilver;

	    if (dtemp > 1 + .12 * the_event->arg1) {
	        dtemp = 1 + .12 * the_event->arg1;
	    }

	    if (dtemp < 1) {
		dtemp = 1;
	    }

		/* award the bonus */
	    Do_speed(c, c->player.max_quickness, c->player.quicksilver + dtemp,
		    0, FALSE);

	    break;

	case 7:
	    return;

        default:

            sprintf(error_msg,
                    "[%s] Returned non-option in Do_smith.\n",
                    c->connection_id);

            Do_log_error(error_msg);
            Do_caught_hack(c, H_SYSTEM);
        }
        return;
    }
    else {

	Do_send_line(c,
  "Wayland Smith appears!  He glares at you for a moment, then walks away.\n");

	Do_more(c);
	return;
    }
}
