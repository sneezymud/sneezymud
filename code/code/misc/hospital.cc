//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//   "hospital.cc" - Special procedures for hospitals and doctors.
//
//////////////////////////////////////////////////////////////////////////


#include "extern.h"
#include "room.h"
#include "monster.h"
#include "handler.h"
#include "configuration.h"
#include "combat.h"
#include "disease.h"
#include "shop.h"
#include "shopowned.h"
#include "spec_mobs.h"
#include "person.h"

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
    case WEAR_LEG_R:
    case WEAR_LEG_L:
    case WEAR_EX_LEG_R:
    case WEAR_EX_LEG_L:
      return (basenum * 3);
    case WEAR_NECK:
    case WEAR_HEAD:
    case WEAR_BODY:
    case WEAR_BACK:
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
    case WEAR_WAIST:
      return (basenum * 4);
    case WEAR_NOWHERE:
    case MAX_WEAR:
      break;
  }
  vlogf(LOG_BUG, format("Bad pos (%d) in limb_heal_price for %s!") %  pos % ch->getName());
  return (basenum * 10);
}

int limb_expel_price(TBeing *ch, wearSlotT pos, int shop_nr)
{
  TThing *stuck;

  if (!(stuck = ch->getStuckIn(pos))) {
    vlogf(LOG_BUG, format("VERY BAD! limb_expel_price called with pos(%d) char(%s) with no item stuck in!") %  pos % ch->getName());
    return (-1);
  }
  return (int)((float)stuck->expelPrice(ch, pos) * shop_index[shop_nr].getProfitBuy(NULL, ch));
}

int TThing::expelPrice(const TBeing *ch, int pos) const
{
  vlogf(LOG_BUG, format("Somehow %s got something besides a weapon/arrow stuck in them pos(%d)") %  ch->getName() % pos);
  return (1000000);
}

int limb_wound_price(TBeing *ch, wearSlotT pos, unsigned short int wound, int shop_nr)
{
  int price = ch->GetMaxLevel() * ch->GetMaxLevel();

  if (IS_SET(wound, PART_BLEEDING) || IS_SET(wound, PART_BRUISED))
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
    case WEAR_LEG_R:
    case WEAR_LEG_L:
    case WEAR_EX_LEG_R:
    case WEAR_EX_LEG_L:
      return (price * 3);
    case WEAR_NECK:
    case WEAR_HEAD:
    case WEAR_BODY:
    case WEAR_BACK:
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
    case WEAR_WAIST:
      return (price * 4);
    case WEAR_NOWHERE:
    case MAX_WEAR:
    case HOLD_RIGHT:
    case HOLD_LEFT:
      break;
  }
  vlogf(LOG_BUG, format("Bad pos (%d) in limb_wound_price!") %  pos);
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
    case WEAR_LEG_R:
    case WEAR_LEG_L:
    case WEAR_EX_LEG_R:
    case WEAR_EX_LEG_L:
      return (price * 3);
    case WEAR_NECK:
    case WEAR_HEAD:
    case WEAR_BODY:
    case WEAR_BACK:
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
    case WEAR_WAIST:
      return (price * 4);
    case WEAR_NOWHERE:
    case MAX_WEAR:
    case HOLD_RIGHT:
    case HOLD_LEFT:
      break;
  }
  vlogf(LOG_BUG, format("Bad pos (%d) in limb_regen_price!") %  pos);
  return (int)(1000000.0 * shop_index[shop_nr].getProfitBuy(NULL, ch));
}

int doctorCost(int shop_nr, TBeing *ch, diseaseTypeT disease)
{
  int cost=0;

  if(!ch)
    return 0;

  if (ch->GetMaxLevel() < 3) {
    cost = DISEASE_PRICE_3;
  } else if (ch->GetMaxLevel() < 6) {
    cost = DISEASE_PRICE_6;
  } else if (ch->GetMaxLevel() < 12) {
    cost = DISEASE_PRICE_12;
  } else {
    cost = DiseaseInfo[disease].cure_cost;
  }

  cost = (int)((float) cost * shop_index[shop_nr].getProfitBuy(NULL, ch));
  
  return cost;
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
    me->doTell(ch, "I will list out what ails you, along with a price.");
    for (i = MIN_WEAR; i < MAX_WEAR; i++) {
      if (i == HOLD_RIGHT || i == HOLD_LEFT)
        continue;
      if (!ch->slotChance(i))
        continue;
      if (ch->isLimbFlags(i, PART_MISSING)) {
        me->doTell(ch, format("%d) Your %s is missing! (%d talens)") %
		   ++count % ch->describeBodySlot(i) % limb_regen_price(ch, i, shop_nr));
        continue;
      } else {
        for (j = 0; j < MAX_PARTS; j++) {
          if (1<<j == PART_BANDAGED)
            continue;
          if (ch->isLimbFlags(i, 1 << j)) {
            me->doTell(ch, format("%d) Your %s is %s. (%d talens)") %
		       ++count % ch->describeBodySlot(i) % body_flags[j] %
		       limb_wound_price(ch, i, 1 << j, shop_nr));
          }
        }
        if (ch->getCurLimbHealth(i) < ch->getMaxLimbHealth(i)) {
          double perc = (double) ch->getCurLimbHealth(i) / (double) ch->getMaxLimbHealth(i);
          me->doTell(ch, format("%d) Your %s is %s. (%d talens)") %
		     ++count % ch->describeBodySlot(i) %
		     LimbHealth(perc) % limb_heal_price(ch, i, shop_nr));
        }
        if ((stuck = ch->getStuckIn(i))) {
          me->doTell(ch, format("%d) You have %s stuck in your %s. (%d talens)") % ++count % stuck->shortDescr % ch->describeBodySlot(i) % limb_expel_price(ch, i, shop_nr));
        }
      }
    }
    if (ch->affected) {
      affectedData *aff;
      for (aff = ch->affected; aff; aff = aff->next) {
        if (aff->type == AFFECT_DISEASE) {
	  if (ch->GetMaxLevel() < 12) {
	    me->doTell(ch, "Hmm, you are just a newbie, guess I will have to take you at reduced rates.\n\r");
	  }
	  buf=format("%d) You have %s. (%d talens)") %
	    ++count % DiseaseInfo[affToDisease(*aff)].name %
	    doctorCost(shop_nr, ch, affToDisease(*aff));
	  me->doTell(ch, buf);
        } else if (aff->type == SPELL_BLINDNESS) {
          if (!aff->shouldGenerateText())
            continue;
          me->doTell(ch, format("%d) Affect: %s. (%d talens).\n\r") %
                    ++count % discArray[aff->type]->name %
                    spell_regen_price(ch, SPELL_BLINDNESS, shop_nr));
	}
      }  // affects loop
    }
    if (!count) {
      me->doTell(ch, "I see nothing at all wrong with you!");
    }
    return TRUE;
   /* Allow them to buy cures for their ailments. */
  } else if (cmd == CMD_BUY) {
    for (; isspace(*arg); arg++);

    if (!*arg || !arg) {
      me->doTell(ch, "What do you want to buy? Try listing to see what ails you!");
      return TRUE;
    }

    // doctor exists in a room that you can fight in
    // fight right next to him (or get him to hate you and follow you)
    // would create a *NICE* situation for the player
    if (ch->fight()) {
      me->doTell(ch, "Come back when you aren't fighting.");
      return TRUE;
    }
    if (me->master == ch) {
      me->doTell(ch, "Your money is no good here.");
      return TRUE;
    }

    if(!sstring(arg).isNumber()){
      me->doTell(ch, "To buy a cure, type \"buy <number>\". Try listing to see what ails you!");
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
            me->doTell(ch, format("You don't have enough money to regenerate your %s!") % ch->describeBodySlot(i));
            return TRUE;
          } else {
            if (!ch->limbConnections(i)) {
              me->doTell(ch, format("You can't regenerate your %s until something else is regenerated first.") % ch->describeBodySlot(i));
              return TRUE;
            }
            int cashCost = min(ch->getMoney(), cost);

	    if(me->getMoney() < cashCost){
	      me->doTell(ch, "I don't have enough money to cover my operating expenses!");
	      return TRUE;
	    }

	    TShopOwned tso(shop_nr, me, ch);
	    tso.doBuyTransaction(cashCost, format("regenerating %s") % 
				 ch->describeBodySlot(i),
				 TX_BUYING_SERVICE);

            buf=format("$n waves $s hands, utters many magic phrases and regenerates $N's %s!") % ch->describeBodySlot(i);
            act(buf, TRUE, me, NULL, ch, TO_NOTVICT);
            buf=format("$n waves $s hands, utters many magic phrases and regenerates your %s!") % ch->describeBodySlot(i);
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
                me->doTell(ch, "You don't have enough money to do that!");
                return TRUE;
              } else {
                int cashCost = min(ch->getMoney(), cost);
		
		if(me->getMoney() < cashCost){
		  me->doTell(ch, "I don't have enough money to cover my operating expenses!");
		  return TRUE;
		}

		TShopOwned tso(shop_nr, me, ch);
		tso.doBuyTransaction(cashCost, format("mending %s") % 
				     ch->describeBodySlot(i),
				     TX_BUYING_SERVICE);

                buf=format("$n waves $s hands, utters many magic phrases and touches $N's %s!") % ch->describeBodySlot(i);
                act(buf, TRUE, me, NULL, ch, TO_NOTVICT);
                buf=format("$n waves $s hands, utters many magic phrases and touches your %s!") % ch->describeBodySlot(i);
                act(buf, TRUE, me, NULL, ch, TO_VICT);
                ch->sendTo(format("Your %s is healed!\n\r") % ch->describeBodySlot(i));
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
            me->doTell(ch, format("You don't have enough money to heal your %s!") % ch->describeBodySlot(i));
            return TRUE;
          } else {
            int cashCost = min(ch->getMoney(), cost);

	    if(me->getMoney() < cashCost){
	      me->doTell(ch, "I don't have enough money to cover my operating expenses!");
	      return TRUE;
	    }

	    TShopOwned tso(shop_nr, me, ch);
	    tso.doBuyTransaction(cashCost, format("healing %s") % 
				 ch->describeBodySlot(i),
				 TX_BUYING_SERVICE);

            buf=format("$n waves $s hands, utters many magic phrases and touches $N's %s!") % ch->describeBodySlot(i);
            act(buf, TRUE, me, NULL, ch, TO_NOTVICT);
            buf=format("$n waves $s hands, utters many magic phrases and touches your %s!") % ch->describeBodySlot(i);
            act(buf, TRUE, me, NULL, ch, TO_VICT);
            ch->sendTo(format("Your %s feels better!\n\r") % ch->describeBodySlot(i));
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
            me->doTell(ch, format("You don't have enough money to expel %s from your %s!") % stuck->shortDescr % ch->describeBodySlot(i));
            return TRUE;
          } else {
            int cashCost = min(ch->getMoney(), cost);

	    if(me->getMoney() < cashCost){
	      me->doTell(ch, "I don't have enough money to cover my operating expenses!");
	      return TRUE;
	    }

	    TShopOwned tso(shop_nr, me, ch);
	    tso.doBuyTransaction(cashCost, format("expelling %s") % 
				 ch->describeBodySlot(i),
				 TX_BUYING_SERVICE);

            buf=format("$n skillfully removes $p from $N's %s!") % ch->describeBodySlot(i);
            act(buf, TRUE, me, stuck, ch, TO_NOTVICT);
            buf=format("$n skillfully removes $p from your %s!") % ch->describeBodySlot(i);
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
	    cost = doctorCost(shop_nr, ch, affToDisease(*aff));

	    if ((ch->getMoney()) < cost) {
	      me->doTell(ch, format("You don't have enough money to cure %s!") % DiseaseInfo[affToDisease(*aff)].name);
	      return TRUE;
	    } else {
	      int cashCost = min(ch->getMoney(), cost);

	      if(me->getMoney() < cashCost){
		me->doTell(ch, "I don't have enough money to cover my operating expenses!");
		return TRUE;
	      }

	      TShopOwned tso(shop_nr, me, ch);
	      tso.doBuyTransaction(cashCost, format("curing disease %s") %
				   DiseaseInfo[affToDisease(*aff)].name,
				   TX_BUYING_SERVICE);
	      
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
              me->doTell(ch, format("You don't have enough money to cure %s!") % discArray[aff->type]->name);
              return TRUE;
            } else {
              int cashCost = min(ch->getMoney(), cost);

	      if(me->getMoney() < cashCost){
		me->doTell(ch, "I don't have enough money to cover my operating expenses!");
		return TRUE;
	      }

	      TShopOwned tso(shop_nr, me, ch);
	      tso.doBuyTransaction(cashCost, format("curing blindness %i") %
				   ch->describeBodySlot(i),
				   TX_BUYING_SERVICE);

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
      me->doTell(ch, " I see nothing at all wrong with you!");
    }
    ch->doSave(SILENT_YES);
    return TRUE;
  }
  return FALSE;
}

TMonster* getDoctor(int hospital_room, int &shop_nr)
{
  int doctor_room = -1;

  // find the doctor
  switch(hospital_room){
    case 416:
    case 418: // emergency room
      shop_nr=144; // gh
      doctor_room=419;
      break;
    case 1353:
      shop_nr=147; // bm
      doctor_room=1352;
      break;
    case 3736:
      shop_nr=146; // logrus
      doctor_room=3753;
      break;
    case 16206:
    case 16231: // emergency room
      shop_nr=145; // amber
      doctor_room=16200;
      break;
    default:
      return NULL;
  }

  TRoom *oproom = real_roomp(doctor_room);
  if (!oproom)
    return NULL;

  TThing *t = NULL;
  for(StuffIter it=oproom->stuff.begin();it!=oproom->stuff.end() && (t=*it);++it){
    TMonster* doctor = dynamic_cast<TMonster*>(t);
    if(doctor && doctor->number==shop_index[shop_nr].keeper)
      return doctor;
  }
  return NULL;
}


int healing_room(TBeing *, cmdTypeT cmd, const char *, TRoom *rp)
{
  TThing *t=NULL;
  TPerson *healed;
  TMonster *doctor = NULL;
  int num, cost, shop_nr = -1;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
    healed = dynamic_cast<TPerson *>(t);
    if (!healed)
      continue;
    if (healed->fight())
      continue;

    if (healed->getHit() >= healed->hitLimit()) {
      if (!number(0, 10))
        healed->sendTo("Since you require no healing, you are not charged for staying in the hospital.\n\r");
      continue;
    }

    num = 25 + number(1, healed->GetMaxLevel() / 2);
    num = min(num, (healed->hitLimit() - healed->getHit()));
    cost = num * healed->GetMaxLevel() * healed->GetMaxLevel() / 100;

    if (cost > healed->getMoney()) {
      healed->sendTo("The hospital doesn't accept credit.\n\r");
      healed->sendTo("You are directed toward an exit out of the hospital for being a vagrant.\n\r");
      continue;
    }

    if (!doctor)
      doctor = getDoctor(rp->in_room, shop_nr);

    if(!doctor){
      if (gamePort != Config::Port::GAMMA)
        vlogf(LOG_BUG, format("Couldn't find doctor for shop_nr=%i!") % shop_nr);
      return FALSE;
    }

    if(doctor->getMoney() < cost) {
      doctor->doTell(healed, "I don't have enough money to cover my operating expenses!");
      return TRUE;
    }

    TShopOwned tso(shop_nr, doctor, healed);
    tso.doBuyTransaction(cost, "healing", TX_BUYING_SERVICE);

    healed->sendTo("The hospital works wonders on your body.\n\r");
    healed->addToHit(num);
    healed->sendTo(format("The charge for the healing is %d talens.\n\r") % cost);

  } // end for
  return FALSE;
}

int emergency_room(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp)
{
  char buf[256];
  int opt, cost, shop_nr=-1;
  wearSlotT i;
  TBeing *doctor;

  if (cmd != CMD_BUY && cmd != CMD_LIST)
    return FALSE;

  if (!ch || dynamic_cast<TMonster *>(ch))
    return FALSE;

  cost = 150 * ch->GetMaxLevel();
  if (cmd == CMD_LIST) {
    ch->sendTo("1 - Healing of the Physical Self (HP Restore)\n\r");
    ch->sendTo("2 - Healing of the Mind (Mana Restore)\n\r");
    ch->sendTo("3 - Healing of the Spirit (Lifeforce Restore)\n\r");
    ch->sendTo(format("Any of these for %d talens.\n\r") % cost);
    return TRUE;
  } else if (cmd == CMD_BUY) {        /* Buy */
    // find the doctor
    doctor = getDoctor(rp->in_room, shop_nr);
    
    if(!doctor){
      vlogf(LOG_BUG, format("couldn't find doctor for shop_nr=%i!") % shop_nr);
      ch->sendTo("Couldn't find the doctor, tell a god!");
      return FALSE;
    }
    TShopOwned tso(shop_nr, dynamic_cast<TMonster *>(doctor), ch);
    arg = one_argument(arg, buf, cElements(buf));
    opt = convertTo<int>(buf);
    if (cost > ch->getMoney()) {
      ch->sendTo("Sorry, no medicare, medicaid or insurance allowed.\n\r");
      return TRUE;
    }
    
    if(doctor->getMoney() < cost){
      doctor->doTell(ch, "I don't have enough money to cover my operating expenses!");
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
	        tso.doBuyTransaction(cost, "full heal", TX_BUYING_SERVICE);
          break;
        case 2:
          ch->setMana(ch->manaLimit());
          ch->sendTo("Your mind is filled with great thoughts.\n\r");
          ch->updatePos();

          // this was added due to fighting in/near the hospitals
          ch->addToWait(combatRound(6));
	        tso.doBuyTransaction(cost, "full mana", TX_BUYING_SERVICE);
          break;
        case 3:
          ch->setLifeforce(500);
          ch->sendTo("Your spirit has been lifted and your troubles have been crucified.\n\r");
          ch->updatePos();

          // this was added due to fighting in/near the hospitals
          ch->addToWait(combatRound(6));
	        tso.doBuyTransaction(cost, "full life force", TX_BUYING_SERVICE);
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

