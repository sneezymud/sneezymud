#include "stdsneezy.h"
#include "obj_trap.h"


string guardShout(TBeing *ch){
  string s;
  bool targVis=ch->canSee(ch->fight());

  switch (number(1, 145)) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      s = "To me my fellows!  I need thy aid.";
      break;
    case 6:
      if (targVis) {
	ssprintf(s, "One of these days, %s, POW!  ZOOM!  To the moon!", ch->pers(ch->fight()));
	break;
      }
    case 7:
    case 8:
    case 9:
      s = "Yaaarggggghhhh!";
      break;
    case 10:
    case 11:
    case 12:
    case 13:
      s = "HELP!  I am being attacked.";
      break;
    case 14:
    case 15:
    case 16:
      s = "BANZAI!  CHARGE!  DEATH BEFORE DISHONOR!  DIE DIE DIE!  SPORK!";
      break;
    case 17:
    case 18:
      s = "Aaaeeeiiih!!";
      break;
    case 19:
      s = "Kawa Bunga!!";
      break;
    case 20:
      if (targVis) {
	ssprintf(s, "All these HORTS like %s will die to me and my COHORTS!!", ch->pers(ch->fight()));
	break;
      }
    case 21:
    case 22:
    case 23:
      if (targVis) {
	ssprintf(s, "I need help! %s is attacking me at %s!", ch->pers(ch->fight()), ch->roomp->name);
	break;
      }
    case 24:
    case 25:
    case 26:
      if (targVis) {
	ssprintf(s, "%s, I'm gonna rip off your head and puke down your neck!", ch->pers(ch->fight()));
	break;
      }
    case 27:
    case 28:
    case 29:
      if (targVis) {
	ssprintf(s, "%s must think %s is pretty tough to tangle with me!!", ch->pers(ch->fight()), ch->fight()->hssh());
	break;
      }
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
      s = "DIIIIIIIIIIIEEEEEEEEE SUCKAHHHHH!!!!!!!";
      break;
    case 36:
    case 37:
    case 38:
      s = "It's butt-kicking time!!";
      break;
    case 39:
    case 40:
    case 41:
      ssprintf(s, "I need some backup here, come to %s!", ch->roomp->getName());
      break;
    case 42:
    case 43:
    case 44:
    case 45:
      if (targVis) {
	ssprintf(s, "Time to die, %s.", ch->pers(ch->fight()));
	break;
      }
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
      if (targVis) {
	ssprintf(s, "You're dead meat, %s!", ch->pers(ch->fight()));
	break;
      }
    case 53:
    case 54:
      if (targVis) {
	ssprintf(s, "The corpse of %s is about to be made available for looting at %s!", ch->pers(ch->fight()), ch->roomp->name);
	break;
      }
    case 55:
      s = "Ack!  I am hit.  Avenge me, brethren!";
      break;
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
      if (targVis) {
	ssprintf(s, "Clearly, %s wants to die!", ch->pers(ch->fight()));
	break;
      }
    case 61:
      ssprintf(s, "Hey I'm kicking %s's ass right now...anyone want to join me?!", ch->pers(ch->fight()));
      break;
    case 62:
    case 63:
      s = "Stand back!  This one's all mine.";
      break;
    case 64:
      s = "Have you ever danced with the devil under the pale moonlight?";
      break;
    case 65:
    case 66:
      if (targVis) {
	ssprintf(s, "Foolish %s thinks %s can beat me!", ch->pers(ch->fight()), ch->fight()->hssh());
	break;
      }
    case 67:
    case 68:
    case 69:
      if (targVis) {
	ssprintf(s, "You think you're tough, %s?  Take that!", ch->pers(ch->fight()));
	break;
      }
    case 70:
    case 71:
    case 72:
      ssprintf(s, "Help!  Criminals at %s!", ch->roomp->name);
      break;
    case 73:
      if (targVis) {
	ssprintf(s, "Hey!! Come and check this out! %s is wearing pink chainmail! HAHAHAHAHA!!!", ch->pers(ch->fight()));
	break;
      }
    case 74:
    case 75:
    case 76:
    case 77:
      if (targVis) {
	ssprintf(s, "%s is going to have a VERY bad day once I finish with %s!", ch->pers(ch->fight()), ch->fight()->hmhr());
	break;
      }
    case 78:
    case 79:
    case 80:
      s = "I'm about to ROCK YOUR WORLD!";
      break;
    case 81:
      if (targVis) {
	ssprintf(s, "%s's face is about to be stamped into 200 gorrilla cookies!", ch->pers(ch->fight()));
	break;
      }
    case 82:
    case 83:
    case 84:
    case 85:
      if (targVis) {
	ssprintf(s, "They'll be picking up pieces of %s for weeks when I get finished with %s!", ch->pers(ch->fight()), ch->fight()->hmhr());
	break;
      }
    case 86:
    case 87:
    case 88:
      s = "Yaaaaarrrggggggghhhhhh!";
      break;
    case 89:
    case 90:
    case 91:
      if (targVis) {
	ssprintf(s, "I'm your worst nightmare, %s!", ch->pers(ch->fight()));
	break;
      }
    case 92:
      s = "Chiefs!  Gimme some help... This mug be gacking me most heinously!";
      break;
    case 93:
    case 94:
    case 95:
      s = "Like... OUCH!";
      break;
    case 96:
    case 97:
    case 98:
      if (targVis) {
	ssprintf(s, "I hate it when newbies, like %s, attack me!", ch->pers(ch->fight()));
	break;
      }
    case 99:
    case 100:
      s = "Eat some of this!";
      break;
    case 101:
    case 102:
    case 103:
      if (targVis) {
	ssprintf(s, "It's time to remind %s just what a wimp %s is!", ch->pers(ch->fight()), ch->fight()->hssh());
	break;
      }
    case 104:
    case 105:
      if (targVis) {
	ssprintf(s, "It's time to remind %s just what the words 'You wish your wounds would stop BLEEDING so much mean!!!", ch->pers(ch->fight()));
	break;
      }
    case 106:
    case 107:
    case 108:
    case 109:
      ssprintf(s, "Ruffians at %s!", ch->roomp->name);
      break;
    case 110:
    case 111:
    case 112:
      if (targVis) {
	ssprintf(s, "%s is gonna die at my hands!", ch->pers(ch->fight()));
	break;
      }
    case 113:
    case 114:
    case 115:
      if (targVis) {
	ssprintf(s, "Just wait until my friends get here, %s!", ch->pers(ch->fight()));
	break;
      }
    case 116:
      if (targVis) {
	ssprintf(s, "%s fights like a wombat!", ch->pers(ch->fight()));
	break;
      }
    case 117:
      if (targVis) {
	ssprintf(s, "%s's momma wears combat boots!", ch->pers(ch->fight()));
	break;
      }
    case 118:
    case 119:
    case 120:
      if (targVis) {
	ssprintf(s, "When I get through with %s, %s'll wish %s'd never heard the name %s!", ch->pers(ch->fight()), ch->fight()->hssh(), ch->fight()->hssh(), MUD_NAME);
	break;
      }
    case 121:
    case 122:
      ssprintf(s, "I need a cleric and two bags of marshmallows at %s....STAT!", ch->roomp->name);
      break;
    case 123:
    case 124:
    case 125:
      if (targVis) {
	ssprintf(s, "Anybody want a piece of %s?  I'm tanking!", ch->pers(ch->fight()));
	break;
      }
    case 126:
    case 127:
      if (targVis) {
	ssprintf(s, "I hope you brought your recall scrolls with you, %s!  Cuz' you ain't walking away from this one!", ch->pers(ch->fight()));
	break;
      }
    case 128:
    case 129:
    case 130:
      if (targVis) {
	ssprintf(s, "Bandits and marauders at %s!  Help me destroy them!", ch->roomp->name);
	break;
      }
    case 131:
      if (targVis) {
	ssprintf(s, "I'm gonna stomp %s's butt right out of %s!", ch->pers(ch->fight()), MUD_NAME);
	break;
      }
    case 132:
    case 133:
    case 134:
    case 135:
      if (targVis) {
	ssprintf(s, "%s is a bloody coward!", ch->pers(ch->fight()));
	break;
      }
    case 136:
    case 137:
    case 138:
    case 139:
    case 140:
      if (targVis) {
	ssprintf(s, "%s is going down!  HARD!", ch->pers(ch->fight()));
	break;
      }    
    case 141:
    case 142:
    case 143:
    case 144:
    case 145:
      s = "'Tis but a flesh wound.";
      break;
    default:
      s = "Buggy ass code.  I puke on the coders!";
  }                                // end switch 
  
  return s;
}


int cityguard(TBeing *, cmdTypeT cmd, const char *, TMonster *ch, TObj *)
{
  TBeing *tch = NULL;
  TThing *t1 = NULL, *t2 = NULL;
  TTrap *trap;
  char buf[256], buf2[256], buf3[256];
  int rc = 0, num = 0, num2 = 0;
  string s;

  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake())
    return FALSE;

  ch->aiMaintainCalm();

  if (ch->task || ch->spelltask)
    return FALSE;

  if (ch->fight() && ch->fight()->isPc() && ch->canSpeak()) {
    if (::number(0,99) < 65)
      return FALSE;   // have them shout a bit less 

    s=guardShout(ch);
    strcpy(buf, s.c_str());

    if (!number(0, 20))
      ch->doShout(cap(buf));
    else if (::number(0,2)) {
      sprintf(buf2, "$n rears back %s head and shouts loudly.", ch->hshr());
      sprintf(buf3, "$n shouts, \"%s\"", cap(buf));
      act(buf2, TRUE, ch, 0, 0, TO_ROOM);
      act(buf3, TRUE, ch, 0, 0, TO_ROOM);
    }
   
    if (ch->fight()) {
      for (t1 = ch->roomp->getStuff(); t1; t1 = t2) {
        t2 = t1->nextThing;
        TBeing *tbt = dynamic_cast<TBeing *>(t1);
        if (!tbt || (ch == tbt))
          continue;
        if (tbt->spec != SPEC_CITYGUARD)
          continue;
        if (tbt->fight()) {
          if (ch->fight() == tbt->fight())
            num2++;
        } 
        continue;
      }
      num = ::number(1, 2);
      for (t1 = ch->roomp->getStuff(); t1; t1 = t2) {
        t2 = t1->nextThing;
        TBeing *tbt = dynamic_cast<TBeing *>(t1);
        if(!tbt)
          continue;
        if (ch == tbt)
          continue;
        if (tbt->spec != SPEC_CITYGUARD)
          continue;
        if (tbt->fight())
          continue;
        if (tbt->master)
          continue;
        if (tbt->desc)
          continue;
        if (::number(0,1))
          tbt->doSay("I will come to your assistance!");
        if (tbt->getPosition() < POSITION_STANDING)
          tbt->doStand();
        rc = tbt->doAssist("", ch, TRUE);
        num -= 1;
      }
      if (ch->fight() && (num > 0) && (num2 < 4))
        CallForGuard(ch, ch->fight(), num);
    }
    return TRUE;
  }
  if (ch->checkPeaceful("") || ch->fight())
    return FALSE;


  ///////////////////////////////////////////////////////////////
  // not fighting stuff
  //

  for (t1 = ch->roomp->getStuff(); t1; t1 = t2) {
    t2 = t1->nextThing;
    tch = dynamic_cast<TBeing *>(t1);

    if((trap=dynamic_cast<TTrap *>(t1)) && trap->getTrapCharges()>0){
      ch->doSay("Whoa, this looks dangerous!");
      act("$n disarms $p.", FALSE, ch, trap, 0, TO_ROOM);
      trap->setTrapCharges(0);
    }

    if (!tch)
      continue;

    if (tch == ch || !ch->canSee(tch))
      continue;

    if (tch->isImmortal() && tch->isPlayerAction(PLR_NOHASSLE))
      continue;

    if (!ch->isUndead() && !ch->isDiabolic()) {
      //TObj *amulet = NULL; // something special for my amulet - dash
      //      if (tch->isUndead() && 
      //	  (amulet = dynamic_cast<TObj *>(ch->equipment[WEAR_NECK])) && 
      //	  obj_index[amulet->getItemIndex()].virt != 9597)
      //	continue;

      if ((tch->isUndead() || tch->isDiabolic()) && 
	  !tch->inGrimhaven() && !tch->isPc()) {
        if (!ch->checkSoundproof())
          act("$n screams 'Get thee back to the underworld that spawned you!!!!'", FALSE, ch, 0, 0, TO_ROOM);

        rc = ch->takeFirstHit(*tch);
        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          delete tch;
          tch = NULL;
        } else if (IS_SET_DELETE(rc, DELETE_THIS)) 
          return DELETE_THIS;
        
        return TRUE;
      } else if(tch->hasDisease(DISEASE_LEPROSY) && !tch->isPc()){
	if(!ch->checkSoundproof())
	  act("$n screams 'There is no mercy for your kind, leper!'", FALSE, ch, 0, 0, TO_ROOM);

	rc=ch->takeFirstHit(*tch);
	

        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          delete tch;
          tch = NULL;
        } else if (IS_SET_DELETE(rc, DELETE_THIS)) 
          return DELETE_THIS;
	return TRUE;
      }

    } else {
      // an undead or demon guard
      if (!tch->isUndead() && !tch->isDiabolic()) {
        if (!ch->checkSoundproof())
          act("$n screams 'This place belongs to the UnLiving!!!!'", FALSE, ch, 0, 0, TO_ROOM);

        if ((rc = ch->takeFirstHit(*tch)) == DELETE_VICT) {
          delete tch;
          tch = NULL;
        } else if (rc == DELETE_THIS) 
          return DELETE_THIS;
        
        return TRUE;
      }
    }
    if (ch->roomp->isCitySector() && !(ch->specials.act & ACT_AGGRESSIVE) && 
         (tch->specials.act & ACT_AGGRESSIVE) && 
         !(tch->specials.act & ACT_WIMPY) && ch->canSee(tch)) {
      if (!ch->checkSoundproof())
        act("$n screams 'Protect the innocent!!!'",FALSE,ch,0,0,TO_ROOM);
      if ((rc = ch->takeFirstHit(*tch)) == DELETE_VICT) {
        delete tch;
        tch = NULL;
      } else if (rc == DELETE_THIS) 
        return DELETE_THIS;
      
      return TRUE;
    }
  }
  return FALSE;
}
