//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//   "hospital.cc" - Special procedures for hospitals and doctors.
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "combat.h"
#include "disease.h"
#include "shop.h"
#include "shopowned.h"

int poison_price(TBeing *ch, affectedData *, int shop_nr)
{
  // get more exotic later
  return (int)(500.0 * shop_index[shop_nr].getProfitBuy(NULL, ch));
}

int syphilis_price(TBeing *ch, affectedData *, int shop_nr)
{
  // get more exotic later
  return (int)(10000.0 * shop_index[shop_nr].getProfitBuy(NULL, ch));
}

int limb_heal_price(TBeing *ch, wearSlotT pos, int shop_nr)
{
  int basenum;

  basenum = (ch->getMaxLimbHealth(pos) - ch->getCurLimbHealth(pos));
  basenum *= ch->GetMaxLevel() * ch->GetMaxLevel() * 2 / 100;

  if (ch->GetMaxLevel() < 6)
    basenum = 1;

  basenum = (int)((float)basenum * shop_index[shop_nr].getProfitBuy(NULL, ch));

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
    case WEAR_EX_FOOT_R:
    case WEAR_EX_FOOT_L:
      return (basenum);
    case WEAR_ARM_R:
    case WEAR_ARM_L:
      return (basenum * 2);
    case WEAR_LEGS_R:
    case WEAR_LEGS_L:
    case WEAR_EX_LEG_R:
    case WEAR_EX_LEG_L:
      return (basenum * 3);
    case WEAR_NECK:
    case WEAR_HEAD:
    case WEAR_BODY:
    case WEAR_BACK:
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
    case WEAR_WAISTE:
      return (basenum * 4);
    case WEAR_NOWHERE:
    case MAX_WEAR:
      break;
  }
  vlogf(LOG_BUG, fmt("Bad pos (%d) in limb_heal_price for %s!") %  pos % ch->getName());
  return (basenum * 10);
}

int limb_expel_price(TBeing *ch, wearSlotT pos, int shop_nr)
{
  TThing *stuck;

  if (!(stuck = ch->getStuckIn(pos))) {
    vlogf(LOG_BUG, fmt("VERY BAD! limb_expel_price called with pos(%d) char(%s) with no item stuck in!") %  pos % ch->getName());
    return (-1);
  }
  return (int)((float)stuck->expelPrice(ch, pos) * shop_index[shop_nr].getProfitBuy(NULL, ch));
}

int TThing::expelPrice(const TBeing *ch, int pos) const
{
  vlogf(LOG_BUG, fmt("Somehow %s got something besides a weapon/arrow stuck in them pos(%d)") %  ch->getName() % pos);
  return (1000000);
}

int limb_wound_price(TBeing *ch, wearSlotT pos, unsigned short int wound, int shop_nr)
{
  int price = ch->GetMaxLevel() * ch->GetMaxLevel();

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

  price = (int)((float)price * shop_index[shop_nr].getProfitBuy(NULL, ch));

  switch (pos) {
    case WEAR_FINGER_R:
    case WEAR_FINGER_L:
    case WEAR_HAND_R:
    case WEAR_HAND_L:
    case WEAR_FOOT_R:
    case WEAR_FOOT_L:
    case WEAR_EX_FOOT_R:
    case WEAR_EX_FOOT_L:
      return (price * 1);
    case WEAR_ARM_R:
    case WEAR_ARM_L:
      return (price * 2);
    case WEAR_LEGS_R:
    case WEAR_LEGS_L:
    case WEAR_EX_LEG_R:
    case WEAR_EX_LEG_L:
      return (price * 3);
    case WEAR_NECK:
    case WEAR_HEAD:
    case WEAR_BODY:
    case WEAR_BACK:
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
    case WEAR_WAISTE:
      return (price * 4);
    case WEAR_NOWHERE:
    case MAX_WEAR:
    case HOLD_RIGHT:
    case HOLD_LEFT:
      break;
  }
  vlogf(LOG_BUG, fmt("Bad pos (%d) in limb_wound_price!") %  pos);
  return (int)(1000000.0 * shop_index[shop_nr].getProfitBuy(NULL, ch));
}

int spell_regen_price(TBeing *ch, spellNumT spell, int shop_nr)
{
  int price = 1;

  if (spell == SPELL_BLINDNESS) {
    price = ch->GetMaxLevel() * max((int) ch->GetMaxLevel(), 20) * 1;
  }

  return (int)((float)price * shop_index[shop_nr].getProfitBuy(NULL, ch));
}

int limb_regen_price(TBeing *ch, wearSlotT pos, int shop_nr)
{
  int price = ch->GetMaxLevel() * max(20, (int) ch->GetMaxLevel()) * 3;

  price = (int)((float) price * shop_index[shop_nr].getProfitBuy(NULL, ch));

  switch (pos) {
    case WEAR_FINGER_R:
    case WEAR_FINGER_L:
    case WEAR_HAND_R:
    case WEAR_HAND_L:
    case WEAR_FOOT_R:
    case WEAR_FOOT_L:
    case WEAR_EX_FOOT_R:
    case WEAR_EX_FOOT_L:
      return (price * 1);
    case WEAR_ARM_R:
    case WEAR_ARM_L:
      return (price * 2);
    case WEAR_LEGS_R:
    case WEAR_LEGS_L:
    case WEAR_EX_LEG_R:
    case WEAR_EX_LEG_L:
      return (price * 3);
    case WEAR_NECK:
    case WEAR_HEAD:
    case WEAR_BODY:
    case WEAR_BACK:
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
    case WEAR_WAISTE:
      return (price * 4);
    case WEAR_NOWHERE:
    case MAX_WEAR:
    case HOLD_RIGHT:
    case HOLD_LEFT:
      break;
  }
  vlogf(LOG_BUG, fmt("Bad pos (%d) in limb_regen_price!") %  pos);
  return (int)(1000000.0 * shop_index[shop_nr].getProfitBuy(NULL, ch));
}

int doctor(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  int j, count = 0, bought, res;
  unsigned int shop_nr;
  wearSlotT i;
  sstring buf;
  TThing *stuck;
  int cost;

  if (cmd == CMD_GENERIC_PULSE){
    me->aiMaintainCalm();
    return false;
  }

  if (!ch->isPc())
    return FALSE;

  if(cmd != CMD_LIST && cmd != CMD_BUY && cmd != CMD_WHISPER)
    return false;

  shop_nr=find_shop_nr(me->number);

  if(cmd == CMD_WHISPER)
    return shopWhisper(ch, me, shop_nr, arg);


 /* Go thru and print out what ails the person. */
  if (cmd == CMD_LIST) {
    me->doTell(ch->getName(), "I will list out what ails you, along with a price.");
    for (i = MIN_WEAR; i < MAX_WEAR; i++) {
      if (i == HOLD_RIGHT || i == HOLD_LEFT)
        continue;
      if (!ch->slotChance(i))
        continue;
      if (ch->isLimbFlags(i, PART_MISSING)) {
        me->doTell(ch->getName(), fmt("%d) Your %s is missing! (%d talens)") %
		   ++count % ch->describeBodySlot(i) % limb_regen_price(ch, i, shop_nr));
        continue;
      } else {
        for (j = 0; j < MAX_PARTS; j++) {
          if (1<<j == PART_BANDAGED)
            continue;
          if (ch->isLimbFlags(i, 1 << j)) {
            me->doTell(ch->getName(), fmt("%d) Your %s is %s. (%d talens)") %
		       ++count % ch->describeBodySlot(i) % body_flags[j] %
		       limb_wound_price(ch, i, 1 << j, shop_nr));
          }
        }
        if (ch->getCurLimbHealth(i) < ch->getMaxLimbHealth(i)) {
          double perc = (double) ch->getCurLimbHealth(i) / (double) ch->getMaxLimbHealth(i);
          me->doTell(ch->getName(), fmt("%d) Your %s is %s. (%d talens)") %
		     ++count % ch->describeBodySlot(i) %
		     LimbHealth(perc) % limb_heal_price(ch, i, shop_nr));
        }
        if ((stuck = ch->getStuckIn(i))) {
          me->doTell(ch->getName(), fmt("%d) You have %s stuck in your %s. (%d talens)") % ++count % stuck->shortDescr % ch->describeBodySlot(i) % limb_expel_price(ch, i, shop_nr));
        }
      }
    }
    if (ch->affected) {
      affectedData *aff;
      for (aff = ch->affected; aff; aff = aff->next) {
        if (aff->type == AFFECT_DISEASE) {
	  if (ch->GetMaxLevel() < 12) {
	    me->doTell(ch->getName(), "Hmm, you are just a newbie, guess I will have to take you at reduced rates.\n\r");
	  }
	  if (ch->GetMaxLevel() < 3) {
	    buf=fmt("%d) You have %s (%d talens).\n\r") %
                    ++count % DiseaseInfo[affToDisease(*aff)].name %
                    DISEASE_PRICE_3;
	  } else if (ch->GetMaxLevel() < 6) {
	    buf=fmt("%d) You have %s (%d talens).\n\r") %
                    ++count % DiseaseInfo[affToDisease(*aff)].name %
                    DISEASE_PRICE_6;
	  } else if (ch->GetMaxLevel() < 12) {
	    buf=fmt("%d) You have %s (%d talens).\n\r") %
                    ++count % DiseaseInfo[affToDisease(*aff)].name %
                    DISEASE_PRICE_12;
	  } else {
	    buf=fmt("%d) You have %s (%d talens).\n\r") %
                    ++count % DiseaseInfo[affToDisease(*aff)].name %
                    DiseaseInfo[affToDisease(*aff)].cure_cost;
	  }
	  me->doTell(ch->getName(), buf);
        } else if (aff->type == SPELL_BLINDNESS) {
          if (!aff->shouldGenerateText())
            continue;
          me->doTell(ch->getName(), fmt("%d) Affect: %s. (%d talens).\n\r") %
                    ++count % discArray[aff->type]->name %
                    spell_regen_price(ch, SPELL_BLINDNESS, shop_nr));
	}
      }  // affects loop
    }
    if (!count) {
      me->doTell(ch->getName(), "I see nothing at all wrong with you!");
    }
    return TRUE;
   /* Allow them to buy cures for their ailments. */
  } else if (cmd == CMD_BUY) {
    for (; isspace(*arg); arg++);

    if (!*arg || !arg) {
      me->doTell(ch->getName(), "What do you want to buy? Try listing to see what ails you!");
      return TRUE;
    }

    // doctor exists in a room that you can fight in
    // fight right next to him (or get him to hate you and follow you)
    // would create a *NICE* situation for the player
    if (ch->fight()) {
      me->doTell(ch->getName(), "Come back when you aren't fighting.");
      return TRUE;
    }
    if (me->master == ch) {
      me->doTell(ch->getName(), "Your money is no good here.");
      return TRUE;
    }

    if(!sstring(arg).isNumber()){
      me->doTell(ch->getName(), "To buy a cure, type \"buy <number>\". Try listing to see what ails you!");
      return TRUE;
    }
    bought = convertTo<int>(arg);

    for (i = MIN_WEAR; i < MAX_WEAR; i++) {
      if (i == HOLD_RIGHT || i == HOLD_LEFT)
        continue;
      if (!ch->slotChance(i))
        continue;
      if (ch->isLimbFlags(i, PART_MISSING)) {
        if (++count == bought) {
          if ((ch->getMoney()) < (cost = limb_regen_price(ch, i, shop_nr))) {
            me->doTell(ch->getName(), fmt("You don't have enough money to regenerate your %s!") % ch->describeBodySlot(i));
            return TRUE;
          } else {
            if (!ch->limbConnections(i)) {
              me->doTell(ch->getName(), fmt("You can't regenerate your %s until something else is regenerated first.") % ch->describeBodySlot(i));
              return TRUE;
            }
            int cashCost = min(ch->getMoney(), cost);

	    if(me->getMoney() < cashCost){
	      me->doTell(ch->getName(), "I don't have enough money to cover my operating expenses!");
	      return TRUE;
	    }

	    TShopOwned tso(shop_nr, me, ch);
	    tso.doBuyTransaction(cashCost, ch->describeBodySlot(i),
			      "regenerating");

            buf=fmt("$n waves $s hands, utters many magic phrases and regenerates $N's %s!") % ch->describeBodySlot(i);
            act(buf, TRUE, me, NULL, ch, TO_NOTVICT);
            buf=fmt("$n waves $s hands, utters many magic phrases and regenerates your %s!") % ch->describeBodySlot(i);
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
              if ((ch->getMoney()) < (cost = limb_wound_price(ch, i, 1 << j, shop_nr))) {
                me->doTell(ch->getName(), "You don't have enough money to do that!");
                return TRUE;
              } else {
                int cashCost = min(ch->getMoney(), cost);
		
		if(me->getMoney() < cashCost){
		  me->doTell(ch->getName(), "I don't have enough money to cover my operating expenses!");
		  return TRUE;
		}

		TShopOwned tso(shop_nr, me, ch);
		tso.doBuyTransaction(cashCost, ch->describeBodySlot(i),
				  "mending");

                buf=fmt("$n waves $s hands, utters many magic phrases and touches $N's %s!") % ch->describeBodySlot(i);
                act(buf, TRUE, me, NULL, ch, TO_NOTVICT);
                buf=fmt("$n waves $s hands, utters many magic phrases and touches your %s!") % ch->describeBodySlot(i);
                act(buf, TRUE, me, NULL, ch, TO_VICT);
                ch->sendTo(fmt("Your %s is healed!\n\r") % ch->describeBodySlot(i));
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
          if ((ch->getMoney()) < (cost = limb_heal_price(ch, i, shop_nr))) {
            me->doTell(ch->getName(), fmt("You don't have enough money to heal your %s!") % ch->describeBodySlot(i));
            return TRUE;
          } else {
            int cashCost = min(ch->getMoney(), cost);

	    if(me->getMoney() < cashCost){
	      me->doTell(ch->getName(), "I don't have enough money to cover my operating expenses!");
	      return TRUE;
	    }

	    TShopOwned tso(shop_nr, me, ch);
	    tso.doBuyTransaction(cashCost, ch->describeBodySlot(i),
			      "healing");

            buf=fmt("$n waves $s hands, utters many magic phrases and touches $N's %s!") % ch->describeBodySlot(i);
            act(buf, TRUE, me, NULL, ch, TO_NOTVICT);
            buf=fmt("$n waves $s hands, utters many magic phrases and touches your %s!") % ch->describeBodySlot(i);
            act(buf, TRUE, me, NULL, ch, TO_VICT);
            ch->sendTo(fmt("Your %s feels better!\n\r") % ch->describeBodySlot(i));
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
          if ((ch->getMoney()) < (cost = limb_expel_price(ch, i, shop_nr))) {
            me->doTell(ch->getName(), fmt("You don't have enough money to expel %s from your %s!") % stuck->shortDescr % ch->describeBodySlot(i));
            return TRUE;
          } else {
            int cashCost = min(ch->getMoney(), cost);

	    if(me->getMoney() < cashCost){
	      me->doTell(ch->getName(), "I don't have enough money to cover my operating expenses!");
	      return TRUE;
	    }

	    TShopOwned tso(shop_nr, me, ch);
	    tso.doBuyTransaction(cashCost, ch->describeBodySlot(i),
			      "expelling");

            buf=fmt("$n skillfully removes $p from $N's %s!") % ch->describeBodySlot(i);
            act(buf, TRUE, me, stuck, ch, TO_NOTVICT);
            buf=fmt("$n skillfully removes $p from your %s!") % ch->describeBodySlot(i);
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
	  if (++count == bought) {
	    if (ch->GetMaxLevel() < 3) 
	      cost = DISEASE_PRICE_3;
	    else if (ch->GetMaxLevel() < 6) 
	      cost = DISEASE_PRICE_6;
	    else if (ch->GetMaxLevel() < 12) 
	      cost = DISEASE_PRICE_12;
	    else 
	      cost = DiseaseInfo[affToDisease(*aff)].cure_cost;
	    
	    if ((ch->getMoney()) < cost) {
	      me->doTell(fname(ch->name), fmt("You don't have enough money to cure %s!") % DiseaseInfo[affToDisease(*aff)].name);
	      return TRUE;
	    } else {
	      int cashCost = min(ch->getMoney(), cost);

	      if(me->getMoney() < cashCost){
		me->doTell(ch->getName(), "I don't have enough money to cover my operating expenses!");
		return TRUE;
	      }

	      TShopOwned tso(shop_nr, me, ch);
	      tso.doBuyTransaction(cashCost, DiseaseInfo[affToDisease(*aff)].name,
				"disease");
	      
	      act("$n waves $s hands, utters many magic phrases and touches $N!", TRUE, me, NULL, ch, TO_NOTVICT);
	      act("$n waves $s hands, utters many magic phrases and touches you!", TRUE, me, NULL, ch, TO_VICT);
	      if (aff->modifier == DISEASE_POISON) {
		ch->affectFrom(SPELL_POISON);
		ch->affectFrom(SPELL_POISON_DEIKHAN);
	      }
	      if (aff->modifier == DISEASE_SYPHILIS) {
		ch->affectFrom(SPELL_DEATH_MIST);
	      }
	      ch->diseaseStop(aff);
	      ch->affectRemove(aff);
	      ch->doSave(SILENT_YES);
	      return TRUE;
	    }
	  }
        } else if (aff->type == SPELL_BLINDNESS) {
          if (++count == bought) {
            cost = spell_regen_price(ch, SPELL_BLINDNESS, shop_nr);

            if ((ch->getMoney()) < cost) {
              me->doTell(fname(ch->name), fmt("You don't have enough money to cure %s!") % discArray[aff->type]->name);
              return TRUE;
            } else {
              int cashCost = min(ch->getMoney(), cost);

	      if(me->getMoney() < cashCost){
		me->doTell(ch->getName(), "I don't have enough money to cover my operating expenses!");
		return TRUE;
	      }

	      TShopOwned tso(shop_nr, me, ch);
	      tso.doBuyTransaction(cashCost, ch->describeBodySlot(i),
				"blindness");

              act("$n waves $s hands, utters many magic phrases and touches $N!", TRUE, me, NULL, ch, TO_NOTVICT);
              act("$n waves $s hands, utters many magic phrases and touches you!", TRUE, me, NULL, ch, TO_VICT);

              ch->affectFrom(aff->type);

              ch->doSave(SILENT_YES);
              return TRUE;
            }
          }
        }
      }
    }
    if (!count) {
      me->doTell(ch->getName(), " I see nothing at all wrong with you!");
    }
    ch->doSave(SILENT_YES);
    return TRUE;
  }
  return FALSE;
}

int healing_room(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TThing *t;
  TBeing *healed, *doctor;
  int num, cost, shop_nr=-1;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  // find the doctor
  switch(rp->number){
    case 416:
      shop_nr=144; // gh
      break;
    case 1353:
      shop_nr=147; // bm
      break;
    case 3736:
      shop_nr=146; // logrus
      break;
    case 16206:
      shop_nr=145; // amber
      break;
  }
  for(doctor=character_list;shop_nr>=0 && doctor;doctor=doctor->next){
    if(doctor->number==shop_index[shop_nr].keeper)
      break;
  }
  
  if(!doctor && gamePort != GAMMA_GAMEPORT){
    vlogf(LOG_BUG, fmt("couldn't find doctor for shop_nr=%i!") % shop_nr);
    return FALSE;
  }


  for (t = rp->getStuff(); t; t = t->nextThing) {
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
	  case 16206:
	    --(*healed);
	    thing_to_room(healed, 16239);
	    break;
          default:
            vlogf(LOG_PROC, fmt("Undefined room %d in healing_room") %  healed->in_room);
        }
      } else {
	if(doctor->getMoney() < cost){
	  doctor->doTell(healed->getName(), "I don't have enough money to cover my operating expenses!");
	  return TRUE;
	}

	TShopOwned tso(shop_nr, dynamic_cast<TMonster *>(doctor), healed);
	tso.doBuyTransaction(cost, "healing", "healing");

        healed->sendTo("The hospital works wonders on your body.\n\r");
        healed->addToHit(num);
        healed->sendTo(fmt("The charge for the healing is %d talens.\n\r") % cost);
      }
    }
  }
  return FALSE;
}

int emergency_room(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp)
{
  char buf[256];
  int opt, cost, shop_nr=-1;
  wearSlotT i;
  TBeing *doctor;

  if (!ch || dynamic_cast<TMonster *>(ch))
    return FALSE;

  // find the doctor
  switch(rp->number){
    case 418:
      shop_nr=144; // gh
      break;
    case 16231:
      shop_nr=145; // amber
      break;
  }

  for(doctor=character_list;shop_nr>=0 && doctor;doctor=doctor->next){
    if(doctor->number==shop_index[shop_nr].keeper)
      break;
  }

  
  if(!doctor){
    vlogf(LOG_BUG, fmt("couldn't find doctor for shop_nr=%i!") % shop_nr);
    ch->sendTo("Couldn't find the doctor, tell a god!");
    return FALSE;
  }

  TShopOwned tso(shop_nr, dynamic_cast<TMonster *>(doctor), ch);

  cost = 150 * ch->GetMaxLevel();
  if (cmd == CMD_LIST) {
    ch->sendTo("1 - Healing of the Physical Self (HP Restore)\n\r");
    ch->sendTo("2 - Healing of the Mind (Mana Restore)\n\r");
    ch->sendTo("3 - Healing of the Spirit (Lifeforce Restore)\n\r");
    ch->sendTo(fmt("Any of these for %d talens.\n\r") % cost);
    return TRUE;
  } else if (cmd == CMD_BUY) {        /* Buy */
    arg = one_argument(arg, buf);
    opt = convertTo<int>(buf);
    if (cost > ch->getMoney()) {
      ch->sendTo("Sorry, no medicare, medicaid or insurance allowed.\n\r");
      return TRUE;
    }
    
    if(doctor->getMoney() < cost){
      doctor->doTell(ch->getName(), "I don't have enough money to cover my operating expenses!");
      return TRUE;
    }

    if (ch->fight()) {
      ch->sendTo("The doctors can't work on you if you are fighting.\n\r");
      return TRUE;
    }
    if ((opt >= 1) && (opt <= 3)) {
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
	  tso.doBuyTransaction(cost, "hit points", "full heal");
          break;
        case 2:
          ch->setMana(ch->manaLimit());
          ch->sendTo("Your mind is filled with great thoughts.\n\r");
          ch->updatePos();

          // this was added due to fighting in/near the hospitals
          ch->addToWait(combatRound(6));
	  tso.doBuyTransaction(cost, "mana", "full heal");
          break;
        case 3:
          ch->setLifeforce(500);
          ch->sendTo("Your spirit has been lifted and your troubles have been crucified.\n\r");
          ch->updatePos();

          // this was added due to fighting in/near the hospitals
          ch->addToWait(combatRound(6));
	  tso.doBuyTransaction(cost, "life force", "full heal");
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

