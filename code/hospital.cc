//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: hospital.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


/*************************************************************************

      SneezyMUD - All rights reserved, SneezyMUD Coding Team
      "hospital.cc" - Special procedures for hospitals and doctors.

*************************************************************************/

#include "stdsneezy.h"
#include "combat.h"
#include "disease.h"

int poison_price(TBeing *, affectedData *)
{
  // get more exotic later
  return 500;
}

int limb_heal_price(TBeing *ch, wearSlotT pos)
{
  int basenum;

  basenum = (ch->getMaxLimbHealth(pos) - ch->getCurLimbHealth(pos));
  basenum *= ch->GetMaxLevel() * ch->GetMaxLevel() * 2 / 100;

  if (ch->GetMaxLevel() < 6)
    basenum = 1;

  basenum = max(1,basenum);

  switch (pos) {
    case HOLD_LEFT:
    case HOLD_RIGHT:
      return (basenum);
    case WEAR_FINGER_R:
    case WEAR_FINGER_L:
    case WEAR_HAND_R:
    case WEAR_HAND_L:
    case WEAR_FOOT_R:
    case WEAR_FOOT_L:
      return (basenum);
    case WEAR_ARM_R:
    case WEAR_ARM_L:
      return (basenum * 2);
    case WEAR_LEGS_R:
    case WEAR_LEGS_L:
      return (basenum * 3);
    case WEAR_NECK:
    case WEAR_HEAD:
    case WEAR_BODY:
    case WEAR_BACK:
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
    case WEAR_WAISTE:
      return (basenum * 4);
    default:
      forceCrash("Bad pos (%d) in limb_heal_price for %s!", pos, ch->getName());
      return (basenum * 10);
  }
}

int limb_expel_price(TBeing *ch, wearSlotT pos)
{
  TThing *stuck;

  if (!(stuck = ch->getStuckIn(pos))) {
    vlogf(10, "VERY BAD! limb_expel_price called with pos(%d) char(%s) with no item stuck in!", pos, ch->getName());
    return (-1);
  }
  return stuck->expelPrice(ch, pos);
}

int TThing::expelPrice(const TBeing *ch, int pos) const
{
  vlogf(10, "Somehow %s got something besides a weapon/arrow stuck in them pos(%d)", ch->getName(), pos);
  return (1000000);
}

int limb_wound_price(TBeing *ch, int pos, unsigned short int wound)
{
  int price;

  price = ch->GetMaxLevel() * ch->GetMaxLevel();

  if (IS_SET(wound, PART_BLEEDING))
    price *= 3;

  else if (IS_SET(wound, PART_INFECTED))
    price *= 5;

  else if (IS_SET(wound, PART_LEPROSED))
    price *= 14;

  else if (IS_SET(wound, PART_BROKEN))
    price *= 7;
  
  else if (IS_SET(wound, PART_USELESS))
    price *= 8;

  else if (IS_SET(wound, PART_PARALYZED))
    price *=10;

  price /= 4;

  if (ch->GetMaxLevel() < 6)
    price /= 4;

  switch (pos) {
    case WEAR_FINGER_R:
    case WEAR_FINGER_L:
    case WEAR_HAND_R:
    case WEAR_HAND_L:
    case WEAR_FOOT_R:
    case WEAR_FOOT_L:
      return (price * 1);
    case WEAR_ARM_R:
    case WEAR_ARM_L:
      return (price * 2);
    case WEAR_LEGS_R:
    case WEAR_LEGS_L:
      return (price * 3);
    case WEAR_NECK:
    case WEAR_HEAD:
    case WEAR_BODY:
    case WEAR_BACK:
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
    case WEAR_WAISTE:
      return (price * 4);
    default:
      vlogf(10, "Bad pos (%d) in limb_wound_price!", pos);
      return (1000000);
  }
}

int limb_regen_price(TBeing *ch, int pos)
{
  int price;

  price = ch->GetMaxLevel() * ch->GetMaxLevel();

  price *= 3;

  switch (pos) {
    case WEAR_FINGER_R:
    case WEAR_FINGER_L:
    case WEAR_HAND_R:
    case WEAR_HAND_L:
    case WEAR_FOOT_R:
    case WEAR_FOOT_L:
      return (price * 1);
    case WEAR_ARM_R:
    case WEAR_ARM_L:
      return (price * 2);
    case WEAR_LEGS_R:
    case WEAR_LEGS_L:
      return (price * 3);
    case WEAR_NECK:
    case WEAR_HEAD:
    case WEAR_BODY:
    case WEAR_BACK:
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
    case WEAR_WAISTE:
      return (price * 4);
    default:
      vlogf(10, "Bad pos (%d) in limb_regen_price!", pos);
      return (1000000);
  }
}

int doctor(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  int j, count = 0, bought, res;
  wearSlotT i;
  char buf[256];
  TThing *stuck;
  int cost;

  if (cmd == CMD_GENERIC_PULSE)
    me->aiMaintainCalm();

  if (!ch->isPc())
    return FALSE;

 /* Go thru and print out what ails the person. */
  if (cmd == CMD_LIST) {
    sprintf(buf, "%s I will list out what ails you, along with a price.", ch->getName());
    me->doTell(buf);
    for (i = MIN_WEAR; i < MAX_WEAR; i++) {
      if (i == HOLD_RIGHT || i == HOLD_LEFT)
        continue;
      if (!ch->slotChance(i))
        continue;
      if (ch->isLimbFlags(i, PART_MISSING)) {
        sprintf(buf, "%s %d) Your %s is missing! (%d talens)",
                ch->getName(), ++count, ch->describeBodySlot(i).c_str(),
                limb_regen_price(ch, i));
        me->doTell(buf);
        continue;
      } else {
        for (j = 0; j < MAX_PARTS; j++) {
          if (1<<j == PART_BANDAGED)
            continue;
          if (ch->isLimbFlags(i, 1 << j)) {
            sprintf(buf, "%s %d) Your %s is %s. (%d talens)",
                 ch->getName(), ++count, ch->describeBodySlot(i).c_str(), body_flags[j],
                 limb_wound_price(ch, i, 1 << j));
            me->doTell(buf);
          }
        }
        if (ch->getCurLimbHealth(i) < ch->getMaxLimbHealth(i)) {
          double perc = (double) ch->getCurLimbHealth(i) / (double) ch->getMaxLimbHealth(i);
          sprintf(buf, "%s %d) Your %s is %s. (%d talens)",
                  ch->getName(), ++count, ch->describeBodySlot(i).c_str(),
                  LimbHealth(perc),
                  limb_heal_price(ch, i));
          me->doTell(buf);
        }
        if ((stuck = ch->getStuckIn(i))) {
          sprintf(buf, "%s %d) You have %s stuck in your %s. (%d talens)",
                  ch->getName(), ++count, stuck->shortDescr, ch->describeBodySlot(i).c_str(),
                  limb_expel_price(ch, i));
          me->doTell(buf);
        }
      }
    }
    if (ch->affected) {
      affectedData *aff;
      for (aff = ch->affected; aff; aff = aff->next) {
        if (aff->type == AFFECT_DISEASE) {
          if (!aff->level) {
            if (ch->GetMaxLevel() < 12) {
              sprintf(buf, "%s Hmm, you are just a newbie, guess I will have to take you at reduced rates.\n\r", ch->getName());
              me->doTell(buf);
            }
            if (ch->GetMaxLevel() < 3) {
              sprintf(buf, "%s %d) You have %s (%d talens).\n\r",
                    ch->getName(), ++count,
                    DiseaseInfo[DISEASE_INDEX(aff->modifier)].name,
                    DISEASE_PRICE_3);
            } else if (ch->GetMaxLevel() < 6) {
              sprintf(buf, "%s %d) You have %s (%d talens).\n\r",
                    ch->getName(), ++count,
                    DiseaseInfo[DISEASE_INDEX(aff->modifier)].name,
                    DISEASE_PRICE_6);
            } else if (ch->GetMaxLevel() < 12) {
              sprintf(buf, "%s %d) You have %s (%d talens).\n\r",
                    ch->getName(), ++count,
                    DiseaseInfo[DISEASE_INDEX(aff->modifier)].name,
                    DISEASE_PRICE_12);
            } else {
              sprintf(buf, "%s %d) You have %s (%d talens).\n\r",
                    ch->getName(), ++count, 
                    DiseaseInfo[DISEASE_INDEX(aff->modifier)].name,
                    DiseaseInfo[DISEASE_INDEX(aff->modifier)].cure_cost);
            }
            me->doTell(buf);
          }
        }
      }
    }
    if (!count) {
      sprintf(buf, "%s, I see nothing at all wrong with you!", ch->getName());
      me->doSay(buf);
    }
    return TRUE;
   /* Allow them to buy cures for their ailments. */
  } else if (cmd == CMD_BUY) {
    for (; isspace(*arg); arg++);

    if (!*arg || !arg) {
      sprintf(buf, "%s What do you want to buy? Try listing to see what ails you!", ch->getName());
      me->doTell(buf);
      return TRUE;
    }

    // doctor exists in a room that you can fight in
    // fight right next to him (or get him to hate you and follow you)
    // would create a *NICE* situation for the player
    if (ch->fight()) {
      sprintf(buf, "%s Come back when you aren't fighting.", ch->getName());
      me->doTell(buf);
      return TRUE;
    }
    if (me->master == ch) {
      sprintf(buf, "%s Your money is no good here.", ch->getName());
      me->doTell(buf);
      return TRUE;
    }

    if (!isanumber(arg)) {
      sprintf(buf, "%s To buy a cure, type \"buy <number>\". Try listing to see what ails you!", ch->getName());
      me->doTell(buf);
      return TRUE;
    }
    bought = atoi(arg);

    for (i = MIN_WEAR; i < MAX_WEAR; i++) {
      if (i == HOLD_RIGHT || i == HOLD_LEFT)
        continue;
      if (!ch->slotChance(i))
        continue;
      if (ch->isLimbFlags(i, PART_MISSING)) {
        if (++count == bought) {
          if ((ch->getMoney() + ch->getBank()) < (cost = limb_regen_price(ch, i))) {
            sprintf(buf, "%s You don't have enough money to regenerate your %s!", ch->getName(), ch->describeBodySlot(i).c_str());
            me->doTell(buf);
            return TRUE;
          } else {
            if (!ch->limbConnections(i)) {
              sprintf(buf,"%s You can't regenerate your %s until something else is regenerated first.",ch->getName(), ch->describeBodySlot(i).c_str());
              me->doTell(buf);
              return TRUE;
            }
            int cashCost = min(ch->getMoney(), cost);
            ch->addToMoney(-cashCost, GOLD_HOSPITAL);

            if (cashCost != cost) {
              cashCost = (ch->getBank() - (cost - cashCost));
              ch->setBank(cashCost);
            }

            sprintf(buf, "$n waves $s hands, utters many magic phrases and regenerates $N's %s!", ch->describeBodySlot(i).c_str());
            act(buf, TRUE, me, NULL, ch, TO_NOTVICT);
            sprintf(buf, "$n waves $s hands, utters many magic phrases and regenerates your %s!", ch->describeBodySlot(i).c_str());
            act(buf, TRUE, me, NULL, ch, TO_VICT);
            ch->setLimbFlags(i, 0);
            ch->setCurLimbHealth(i, ch->getMaxLimbHealth(i));
            if (i == WEAR_HAND_R) {
              ch->setLimbFlags(HOLD_RIGHT, 0);
              ch->setCurLimbHealth(HOLD_RIGHT, ch->getMaxLimbHealth(HOLD_RIGHT));
            } else if (i == WEAR_HAND_L) {
              ch->setLimbFlags(HOLD_LEFT, 0);
              ch->setCurLimbHealth(HOLD_LEFT, ch->getMaxLimbHealth(HOLD_LEFT));
            }
            ch->doSave(SILENT_YES);
            return TRUE;
          }
        }
      } else {
        for (j = 0; j < MAX_PARTS; j++) {
          if (1<<j == PART_BANDAGED)
            continue;

          if (ch->isLimbFlags(i, 1 << j)) {
            if (++count == bought) {
              if ((ch->getMoney() + ch->getBank()) < (cost = limb_wound_price(ch, i, 1 << j))) {
                sprintf(buf, "%s You don't have enough money to do that!", ch->getName());
                me->doTell(buf);
                return TRUE;
              } else {
                int cashCost = min(ch->getMoney(), cost);
                ch->addToMoney(-cashCost, GOLD_HOSPITAL);

                if (cashCost != cost) {
                  cashCost = (ch->getBank() - (cost - cashCost));
                  ch->setBank(cashCost);
                }

                sprintf(buf, "$n waves $s hands, utters many magic phrases and touches $N's %s!", ch->describeBodySlot(i).c_str());
                act(buf, TRUE, me, NULL, ch, TO_NOTVICT);
                sprintf(buf, "$n waves $s hands, utters many magic phrases and touches your %s!", ch->describeBodySlot(i).c_str());
                act(buf, TRUE, me, NULL, ch, TO_VICT);
                ch->sendTo("Your %s is healed!\n\r", ch->describeBodySlot(i).c_str());
                ch->remLimbFlags(i, 1 << j);
                if (i == WEAR_HAND_R) {
                  ch->remLimbFlags(HOLD_RIGHT, 1 << j);
                  ch->setCurLimbHealth(HOLD_RIGHT, ch->getMaxLimbHealth(HOLD_RIGHT));
                } else if (i == WEAR_HAND_L) {
                  ch->remLimbFlags(HOLD_LEFT, 1 << j);
                  ch->setCurLimbHealth(HOLD_LEFT, ch->getMaxLimbHealth(HOLD_LEFT));
                }
                ch->doSave(SILENT_YES);
                return TRUE;
              }
            }
          }
        }
      }
      if (ch->getCurLimbHealth(i) < ch->getMaxLimbHealth(i)) {
        if (++count == bought) {
          if ((ch->getMoney() + ch->getBank()) < (cost = limb_heal_price(ch, i))) {
            sprintf(buf, "%s You don't have enough money to heal your %s!", ch->getName(), ch->describeBodySlot(i).c_str());
            me->doTell(buf);
            return TRUE;
          } else {
            int cashCost = min(ch->getMoney(), cost);
            ch->addToMoney(-cashCost, GOLD_HOSPITAL);

            if (cashCost != cost) {
              cashCost = (ch->getBank() - (cost - cashCost));
              ch->setBank(cashCost);
            }

            sprintf(buf, "$n waves $s hands, utters many magic phrases and touches $N's %s!", ch->describeBodySlot(i).c_str());
            act(buf, TRUE, me, NULL, ch, TO_NOTVICT);
            sprintf(buf, "$n waves $s hands, utters many magic phrases and touches your %s!", ch->describeBodySlot(i).c_str());
            act(buf, TRUE, me, NULL, ch, TO_VICT);
            ch->sendTo("Your %s feels better!\n\r", ch->describeBodySlot(i).c_str());
            ch->setCurLimbHealth(i, ch->getMaxLimbHealth(i));
            if (i == WEAR_HAND_R) {
              ch->setCurLimbHealth(HOLD_RIGHT, ch->getMaxLimbHealth(HOLD_RIGHT));
            } else if (i == WEAR_HAND_L) {
              ch->setCurLimbHealth(HOLD_LEFT, ch->getMaxLimbHealth(HOLD_LEFT));
            }
            ch->doSave(SILENT_YES);
            return TRUE;
          }
        }
      }
      if ((stuck = ch->getStuckIn(i))) {
        if (++count == bought) {
          if ((ch->getMoney() + ch->getBank()) < (cost = limb_expel_price(ch, i))) {
            sprintf(buf, "%s You don't have enough money to expel %s from your %s!",
                    ch->getName(), stuck->shortDescr, ch->describeBodySlot(i).c_str());
            me->doTell(buf);
            return TRUE;
          } else {
            int cashCost = min(ch->getMoney(), cost);
            ch->addToMoney(-cashCost, GOLD_HOSPITAL);

            if (cashCost != cost) {
              cashCost = (ch->getBank() - (cost - cashCost));
              ch->setBank(cashCost);
            }

            sprintf(buf, "$n skillfully removes $p from $N's %s!", ch->describeBodySlot(i).c_str());
            act(buf, TRUE, me, stuck, ch, TO_NOTVICT);
            sprintf(buf, "$n skillfully removes $p from your %s!", ch->describeBodySlot(i).c_str());
            act(buf, TRUE, me, stuck, ch, TO_VICT);
            *ch += *(ch->pulloutObj(i, TRUE, &res));

            int rc = stuck->checkSpec(ch, CMD_OBJ_EXPELLED, "", NULL);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              delete stuck;
              stuck = NULL;
            }
            ch->doSave(SILENT_YES);
            return TRUE;
          }
        }
      }
    } /* body part for loop */
    if (ch->affected) {
      affectedData *aff = NULL;
      affectedData *aff2 = NULL;
      for (aff = ch->affected; aff; aff = aff2) {
        aff2 = aff->next;
        if (aff->type == AFFECT_DISEASE) {
          if (!aff->level) {
            if (++count == bought) {
              if (ch->GetMaxLevel() < 3) 
                cost = DISEASE_PRICE_3;
              else if (ch->GetMaxLevel() < 6) 
                cost = DISEASE_PRICE_6;
              else if (ch->GetMaxLevel() < 12) 
                cost = DISEASE_PRICE_12;
              else 
                cost = DiseaseInfo[DISEASE_INDEX(aff->modifier)].cure_cost;
              
              if ((ch->getMoney() + ch->getBank()) < cost) {
                sprintf(buf, "%s You don't have enough money to cure %s!",
                         fname(ch->name).c_str(),
                         DiseaseInfo[DISEASE_INDEX(aff->modifier)].name);
                me->doTell(buf);
                return TRUE;
              } else {
                int cashCost = min(ch->getMoney(), cost);
                ch->addToMoney(-cashCost, GOLD_HOSPITAL);

                if (cashCost != cost) {
                  cashCost = (ch->getBank() - (cost - cashCost));
                  ch->setBank(cashCost);
                }

                act("$n waves $s hands, utters many magic phrases and touches $N!", TRUE, me, NULL, ch, TO_NOTVICT);
                act("$n waves $s hands, utters many magic phrases and touches you!", TRUE, me, NULL, ch, TO_VICT);
                if (aff->modifier == DISEASE_POISON) {
                  ch->affectFrom(SPELL_POISON);
                  ch->affectFrom(SPELL_POISON_DEIKHAN);
                }
                ch->diseaseStop(aff);
                ch->affectRemove(aff);
                ch->doSave(SILENT_YES);
                return TRUE;
              }
            }
          }
        }
      }
    }
    if (!count) {
      sprintf(buf, "%s, I see nothing at all wrong with you!", ch->getName());
      me->doSay(buf);
    }
    ch->doSave(SILENT_YES);
    return TRUE;
  }
  return FALSE;
}

int healing_room(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TThing *t;
  TBeing *healed;
  int num, cost;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  for (t = rp->stuff; t; t = t->nextThing) {
    healed = dynamic_cast<TBeing *>(t);
    if (!healed)
      continue;
    if (healed->fight())
      continue;

    if (!number(0, 12)) {
      if (healed->getHit() >= healed->hitLimit()) {
        if (!number(0, 10))
          healed->sendTo("Since you require no healing, you are not charged for staying in the hospital.\n\r");
        continue;
      }
      num = 25 + number(1, healed->GetMaxLevel() / 2);
      num = min(num, (healed->hitLimit() - healed->getHit()));

#if 0
       // This is abnormally high, Im changing it Brutius 10/20/98
      cost = num * healed->GetMaxLevel() * healed->GetMaxLevel() * 8 / 100;
      cost += 1;
#endif
      cost = num * healed->GetMaxLevel() * healed->GetMaxLevel() / 100;
      if (cost > healed->getMoney()) {
        healed->sendTo("The hospital doesn't accept any medicare or insurance plans.\n\r");
        healed->sendTo("You are tossed out of the hospital for being a vagrant.\n\r");
        switch (healed->in_room) {
          case 416:
            --(*healed);
            thing_to_room(healed, 108);
            break;
          case 1353:
            --(*healed);
            thing_to_room(healed, 1303);
            break;
          case 3736:
            --(*healed);
            thing_to_room(healed, 3710);
            break;
        }
      } else {
        healed->sendTo("The hospital works wonders on your body.\n\r");
        healed->addToHit(num);
        healed->sendTo("The charge for the healing is %d talens.\n\r", cost);
        healed->addToMoney(-cost, GOLD_HOSPITAL);
      }
    }
  }
  return FALSE;
}

int emergency_room(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *)
{
  char buf[256];
  int opt, cost;
  wearSlotT i;

  if (!ch || dynamic_cast<TMonster *>(ch))
    return FALSE;

  cost = 150 * ch->GetMaxLevel();
  if (cmd == CMD_LIST) {
    ch->sendTo("1 - Healing of the Physical Self\n\r");
    ch->sendTo("2 - Healing of the Mind\n\r");
    ch->sendTo("Any of these for %d talens.\n\r", cost);
    return TRUE;
  } else if (cmd == CMD_BUY) {        /* Buy */
    arg = one_argument(arg, buf);
    opt = atoi(buf);
    if (cost > ch->getMoney()) {
      ch->sendTo("Sorry, no medicare, medicaid or insurance allowed.\n\r");
      return TRUE;
    }
    if (ch->fight()) {
      ch->sendTo("The doctors can't work on you if you are fighting.\n\r");
      return TRUE;
    }
    if ((opt >= 1) && (opt <= 2)) {
      ch->addToMoney(-cost, GOLD_HOSPITAL);
      switch (opt) {
        case 1:
          ch->setHit(ch->hitLimit());
          ch->setMove(ch->moveLimit());
          for (i = MIN_WEAR; i < MAX_WEAR; i++) {
            ch->setLimbFlags(i, 0);
            ch->setCurLimbHealth(i, ch->getMaxLimbHealth(i));
          }
          ch->sendTo("You are HEALED my child!\n\r");
          ch->updatePos();

          // this was added due to fighting in/near the hospitals
          ch->addToWait(combatRound(6));
          break;
        case 2:
          ch->setMana(ch->manaLimit());
          ch->sendTo("Your mind is filled with great thoughts.\n\r");
          ch->updatePos();

          // this was added due to fighting in/near the hospitals
          ch->addToWait(combatRound(6));
          break;
        default:
          ch->sendTo("That's not available at THIS hospital!\n\r");
          return TRUE;
      }
      return TRUE;
    }
  }
  return FALSE;
}
