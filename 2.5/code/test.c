/* ************************************************************************
	    }
	  else if (bot >= top)
	    {
	      send_to_char("There is no help on that word.\n\r", ch);
	      return;
	    }
	  else if (chk > 0)
	    bot = ++mid;
	  else
	    top = --mid;
	}
      return;
    }
  
  
  send_to_char(help, ch);
  
}





do_wizhelp(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int no, i;
  extern char *command[];        /* The list of commands (interpreter.c)  */
  /* First command is command[0]           */
  extern struct command_info cmd_info[];
  /* cmd_info[1] ~~ commando[0]            */
  
  if (IS_NPC(ch))
    return;
  
  send_to_char("The following privileged comands are available:\n\r\n\r", ch);
  
  *buf = '\0';
  
  for (no = 1, i = 0; *command[i] != '\n'; i++)
    if ((GetMaxLevel(ch) >= cmd_info[i+1].minimum_level) &&
	(cmd_info[i+1].minimum_level >= LOW_IMMORTAL))  {
      
      sprintf(buf + strlen(buf), "%-10s", command[i]);
      if (!(no % 7))
	strcat(buf, "\n\r");
      no++;
    }
  strcat(buf, "\n\r");
  page_string(ch->desc, buf, 1);
}

void do_who(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data *d;
  char buf[256];
  int count, gods;
  struct char_data      *person;
 
  /*  check for an arg */
  only_argument(argument, buf);  
  if (*buf) {
    gods = TRUE;
  }  else {
    gods = FALSE;
  }
 
   if (cmd == 234) {
    for (d = descriptor_list; d; d = d->next) {
          if ((!IS_AFFECTED(person, AFF_HIDE)) || (IS_IMMORTAL(ch))) {
            sprintf(buf,"%-25s - %s ", GET_NAME(person),
                    real_roomp(person->in_room)->name);
            if (GetMaxLevel(ch) >= LOW_IMMORTAL)
              sprintf(buf+strlen(buf),"[%d]", person->in_room);
          }
       }
    } else { 
    int listed = 0, count, lcount, l, skip = FALSE;
    char arg[256], tempbuf[256];
 
    send_to_char("Players [-? for Help]\n\r",ch);
    send_to_char("--------\n\r", ch);
    count=0;
    lcount=0;
    if (!IS_IMMORTAL(ch)) {
     if (strlen(argument) == 0) {
      for (d = descriptor_list;d;d = d->next) {
            if (!d->connected && CAN_SEE(ch, d->character) &&
          ( real_roomp((person = (d->original ? d->original:d->character)
                        )->in_room))) {
        count++;
        sprintf(buf, "%s %s\n\r", 
                GET_NAME(person),
                (person->player.title?person->player.title:"(Null)"));
        send_to_char(buf,ch);
        }
      }
    } else {
      argument = one_argument(argument, arg);
      if (arg[0] == '-') {
        if (index(arg, '?') != NULL) {
          send_to_char("[-] l=levels t=title c=color g=gods m=morts\n\r", ch);
          send_to_char("[-] [1]Mage[2]Cleric[3]Warrior[4]Thief[5]Anti[6]Paladin[7]Monk[8]Ranger\n\r\n\r", ch);
          }
          for (d = descriptor_list;d;d = d->next) {
           if (!d->connected && CAN_SEE(ch, d->character) &&
               ( real_roomp((person = (d->original ? d->original:d->character)
                        )->in_room))) {
           if (!IS_NPC(person)) {
            count++;
            skip = FALSE;
            if (index(arg,'g') != NULL) {
              if (!IS_IMMORTAL(person)) skip = TRUE;
            }
            if (index(arg,'m') != NULL) {
              if (IS_IMMORTAL(person)) skip = TRUE;
            }
            if (index(arg,'1') != NULL) {
              if (!HasClass(person,CLASS_MAGIC_USER)) skip = TRUE;
            }
            if (index(arg,'2') != NULL) {
              if (!HasClass(person,CLASS_CLERIC)) skip = TRUE;
            }
            if (index(arg,'3') != NULL) {
              if (!HasClass(person,CLASS_WARRIOR)) skip = TRUE;
            }
            if (index(arg,'4') != NULL) {
              if (!HasClass(person,CLASS_THIEF)) skip = TRUE;
            }
            if (!skip) {
                  sprintf(buf, "%-14s ", GET_NAME(person));
                  listed++;
              }
             if ((person->desc != NULL) || (index(arg,'d') != NULL)) {
               for (l = 1; l <= strlen(arg) ; l++) {
                 switch (arg[l]) {
                  case 'l': {
                   if (GetMaxLevel(person) == 60) 
                    sprintf(tempbuf,"Level:[GrandPoobah] ");
                   else if (GetMaxLevel(person) == 59) 
                    sprintf(tempbuf,"Level:[GrandWizard] ");
                   else if (GetMaxLevel(person) == 58)
                    sprintf(tempbuf,"Level:[Senior Lord] ");
                   else if (GetMaxLevel(person) == 57) 
                    sprintf(tempbuf,"Level:[Junior Lord] ");
                   else if (GetMaxLevel(person) == 56)
                    sprintf(tempbuf,"Level:[    God    ] ");
                   else if (GetMaxLevel(person) == 55)
                    sprintf(tempbuf,"Level:[Lesser  God] ");
                   else if (GetMaxLevel(person) == 54)
                    sprintf(tempbuf,"Level:[  DemiGod  ] ");
                   else if (GetMaxLevel(person) == 53)
                    sprintf(tempbuf,"Level:[   Saint   ] ");
                   else if (GetMaxLevel(person) == 52)
                    sprintf(tempbuf,"Level:[LowImmortal] ");
                   else if (GetMaxLevel(person) == 51)
                    sprintf(tempbuf,"Level:[Area Design] ");
                   else {
                    sprintf(tempbuf,"Level:[%-2d/%-2d/%-2d/%-2d] ",
                            person->player.level[0],person->player.level[1],
                            person->player.level[2],person->player.level[3]);
                    } 
                    strcat(buf,tempbuf);
                    break;
                  }
                  case 't': {
                    sprintf(tempbuf," %-16s ",(person->player.title?person->player.title:"(null)"));
                    strcat(buf,tempbuf);
                    break;
                  }              
               default: {
                    break;
                    }
                  }
                }
              }
              if (person->desc != NULL) {
                strcat(buf,"\n\r");
                send_to_char(buf,ch);
              }
            }
          }
        }
      }
     }
     sprintf(buf,"\n\rTotal players : [%d]\n\r", count);
     send_to_char(buf, ch);
    } else {
    if (strlen(argument) == 0) {
      for (person = character_list; person; person = person->next) {
        if ((!IS_NPC(person)) || (IS_SET(person->specials.act, ACT_POLYSELF))) {
          count++;
          if (person->desc == NULL) {
            lcount++;
          } else {
            sprintf(buf, "%s %s\n\r", GET_NAME(person), (person->player.title?person->player.title:"(null)"));
            send_to_char(buf, ch);
          }
        }
      }
    } else {
      argument = one_argument(argument,arg);
      if (arg[0] == '-') {
        if (index(arg,'?') != NULL) {
          send_to_char("[-]i=idle l=levels t=title h=hit/mana/move s=stats\n\r",ch);
          send_to_char("[-]d=linkdead g=God o=Mort [1]Mage[2]Cleric[3]War[4]Thief[5]Druid[6]Monk\n\r", ch);
    send_to_char("--------\n\r", ch);  
        }
        for (person = character_list; person; person = person->next) {
          if (!IS_NPC(person)) {
            count++;
            if (person->desc == NULL) lcount ++;
            skip = FALSE;
            if (index(arg,'g') != NULL) {
              if (!IS_IMMORTAL(person)) skip = TRUE;
            }
            if (index(arg,'o') != NULL) {
              if (IS_IMMORTAL(person)) skip = TRUE;
            }
            if (index(arg,'1') != NULL) {
              if (!HasClass(person,CLASS_MAGIC_USER)) skip = TRUE;
            }
            if (index(arg,'2') != NULL) {
              if (!HasClass(person,CLASS_CLERIC)) skip = TRUE;
            }
            if (index(arg,'3') != NULL) {
              if (!HasClass(person,CLASS_WARRIOR)) skip = TRUE;
            }
            if (index(arg,'4') != NULL) {
              if (!HasClass(person,CLASS_THIEF)) skip = TRUE;
            }
            if (index(arg,'7') != NULL) {
              if (!HasClass(person,CLASS_MONK)) skip = TRUE;
            }
            if (!skip) {
              if (person->desc == NULL) {
                if (index(arg,'d') != NULL) {
                  sprintf(buf, "[%-12s] ", GET_NAME(person));
                  listed++;
                }
              } else {
                if (IS_NPC(person) && 
                    IS_SET(person->specials.act, ACT_POLYSELF)) {
                  sprintf(buf, "(%-14s) ", GET_NAME(person));
                  listed++;
                } else {
                  sprintf(buf, "%-14s ", GET_NAME(person));
                  listed++;
                }
              }
              if ((person->desc != NULL) || (index(arg,'d') != NULL)) {
                for (l = 1; l <= strlen(arg) ; l++) {
                  switch (arg[l]) {
                  case 'i': {
                    sprintf(tempbuf,"Idle:[%-3d] ",person->specials.timer);
                    strcat(buf,tempbuf);
                    break;
                  }
                  case 'l': {
                    sprintf(tempbuf,"Level:[%-2d/%-2d/%-2d/%-2d] ",
                            person->player.level[0],person->player.level[1],
                            person->player.level[2],person->player.level[3],
                            person->player.level[4],person->player.level[5]);
                    strcat(buf,tempbuf);
                    break;
                  }
                  case 'h': {
                    sprintf(tempbuf,"Hit:[%-3d] Mana:[%-3d] Move:[%-3d] ",GET_HIT(person),GET_MANA(person),GET_MOVE(person));
                    strcat(buf,tempbuf);
                    break;
                  }              
                  case 's': {
                    sprintf(tempbuf,"[S:%-2d I:%-2d W:%-2d C:%-2d D:%-2d] ",GET_STR(person),GET_INT(person),GET_WIS(person),GET_CON(person),GET_DEX(person));
                    strcat(buf,tempbuf);
                    break;
                  }              
                  case 't': {
                    sprintf(tempbuf," %-16s ",(person->player.title?person->player.title:"(null)"));
                    strcat(buf,tempbuf);
                    break;
                  }              
                  default: {
                    break;
                  }
                  }
                }
              }
              if ((person->desc != NULL) || (index(arg,'d') != NULL)) {
                strcat(buf,"\n\r");
                send_to_char(buf,ch);
              }
            }
          }
        }
      } else {
        /* list gods */
        for (person = character_list; person; person = person->next) {
          if (!IS_NPC(person)) {
            count++;
            if (IS_IMMORTAL(person)) {
              if (person->desc == NULL) {
                lcount++;
              } else {
                sprintf(buf, "%s %s\n\r", GET_NAME(person), (person->player.title?person->player.title:"(null)"));
                send_to_char(buf,ch);
              }
            }
          }
        }
      }
    }
    if (listed == 0) {
      sprintf(buf, "\n\rTotal players / Link dead [%d/%d] (%2.0f%%)\n\r",
              count,lcount,((float)lcount / (int)count) * 100);
      send_to_char(buf, ch);
    } else {
      sprintf(buf, "\n\rTotal players / Link dead [%d/%d] (%2.0f%%) Number Listed: %d\n\r",
              count,lcount,((float)lcount / (int)count) * 100,listed);
      send_to_char(buf, ch);
    }
   }
  }
}
 
