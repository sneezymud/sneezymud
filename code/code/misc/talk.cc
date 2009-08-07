//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "talk.cc" - All functions related to player communications
//  
//////////////////////////////////////////////////////////////////////////

#include <stdarg.h>

#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "client.h"
#include "colorstring.h"
#include "monster.h"
#include "person.h"
#include "disease.h"
#include "obj_pen.h"
#include "obj_note.h"
#include "database.h"
#include "twitter.h"

void TBeing::disturbMeditation(TBeing *vict) const
{
  mud_assert(vict != NULL, "No vict in disturbMeditation");

  if (vict->task && !isImmortal()) {
    if (vict->task->task == TASK_PENANCE) {
      act("$n disturbs your penance!", FALSE, this, NULL, vict, TO_VICT);

      if (sameRoom(*vict))
        act("You disturb $S penance!", TRUE, this, NULL, vict, TO_CHAR);

      vict->stopTask();
    } else if (vict->task->task == TASK_MEDITATE) {
      act("$n disturbs your meditation!", FALSE, this, NULL, vict, TO_VICT);

      if (sameRoom(*vict))
        act("You disturb $S meditation!", TRUE, this, NULL, vict, TO_CHAR);
 
      vict->stopTask();
    }
  }
}


// uses printf style arguments
int TBeing::doSay(const char *fmt, ...)
{
  va_list ap;
  char buf[MAX_STRING_LENGTH];

  va_start(ap, fmt);
  vsnprintf(buf, MAX_STRING_LENGTH, fmt, ap);

  sstring sbuf=buf;

  return doSay(sbuf);
}


// may return DELETE_THIS
int TBeing::doSay(const sstring &arg)
{
  sstring capbuf, tmpbuf, nameBuf, garbedBuf, garbleRoom;
  TThing *tmp;
  TBeing *mob = NULL;
  int rc;
  Descriptor *d;

  if (desc)
    desc->talkCount = time(0);

  if (applySoundproof())
    return FALSE;

  if(hasQuestBit(TOG_IS_MUTE)){
    sendTo("You're mute, you can't talk.\n\r");
    return FALSE;
  }

  if (isDumbAnimal()) {
    sendTo("You are a dumb animal; you can't talk!\n\r");
    return FALSE;
  }

  if (isAffected(AFF_SILENT)) {
    sendTo("You can't make a sound!\n\r");
    act("$n waves $s hands and points silently toward $s mouth.",
  TRUE, this, 0, 0, TO_ROOM);
    return FALSE;
  }

  if (arg.empty()){
    sendTo("Yes, but WHAT do you want to say?\n\r");
    return FALSE;
  }

  garbleRoom = garble(NULL, arg, Garble::SPEECH_SAY, Garble::SCOPE_EVERYONE);
  
  sendTo(COLOR_COMM, format("<g>You say, <z>\"%s%s\"\n\r") % 
   colorString(this, desc, garbleRoom, NULL, COLOR_BASIC, FALSE) %
   norm());

  // show everyone in room the say.

  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
    tmp=*(it++);
    
    if (!(mob = dynamic_cast<TBeing *>(tmp)))
      continue;
    
    if (!(d = mob->desc) || mob == this ||
        (mob->getPosition() <= POSITION_SLEEPING))
      continue;
    
    capbuf=sstring(mob->pers(this)).cap();
    tmpbuf = format("%s") % colorString(mob, mob->desc, capbuf, NULL, COLOR_NONE, FALSE); 
    
    if (mob->isPc()) {

      if (mob->desc && mob->desc->ignored.isIgnored(desc))
        continue;

      // note: this means only PCs get individualed garbles in a 'say'
      sstring garbleTo = garble(mob, garbleRoom, Garble::SPEECH_SAY, Garble::SCOPE_INDIVIDUAL);

      if (hasColorStrings(NULL, capbuf, 2)) {
        if (IS_SET(mob->desc->plr_color, PLR_COLOR_MOBS)) {
          tmpbuf = format("%s") % colorString(mob, mob->desc, capbuf, NULL, COLOR_MOBS, FALSE);
          mob->sendTo(COLOR_COMM, format("%s says, \"%s%s\"\n\r") % tmpbuf % garbleTo % mob->norm());
          if (d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
            garbedBuf = format("%s") % colorString(this, mob->desc, garbleTo, NULL, COLOR_NONE, FALSE);
            d->clientf(format("%d|%s|%s") % CLIENT_SAY % tmpbuf % garbedBuf);
          }
        } else {
          mob->sendTo(COLOR_COMM, format("<c>%s says, <z>\"%s\"\n\r") % tmpbuf % garbleTo);
            if (d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
              nameBuf = format("<c>%s<z>") % tmpbuf;
              garbedBuf = format("%s") % colorString(this, mob->desc, garbleTo, NULL, COLOR_NONE, FALSE);
              d->clientf(format("%d|%s|%s") % CLIENT_SAY % colorString(this, mob->desc, nameBuf, NULL, COLOR_NONE, FALSE) % garbedBuf);
            }
        }
      } else {
        mob->sendTo(COLOR_COMM, format("<c>%s says, <z>\"%s\"\n\r") % tmpbuf % garbleTo);
        if (d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
          nameBuf = format("<c>%s<z>") % tmpbuf;
          garbedBuf = format("%s") % colorString(this, mob->desc, garbleTo, NULL, COLOR_NONE, FALSE);
          d->clientf(format("%d|%s|%s") 
              % CLIENT_SAY 
              % colorString(this, mob->desc, nameBuf, NULL, COLOR_NONE, FALSE) 
              % garbedBuf);
        }
      }
    } else {
      mob->sendTo(COLOR_COMM, format("%s says, \"%s\"\n\r") % sstring(getName()).cap() % 
            colorString(this, mob->desc, garbleRoom, NULL, COLOR_COMM, FALSE));
      if (d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
        d->clientf(format("%d|%s|%s") % CLIENT_SAY % sstring(getName()).cap() %
       colorString(this, mob->desc, garbleRoom, NULL, COLOR_NONE, FALSE));
      }
    }
  }
  
  // everyone needs to see the say before the response gets triggered
  // loop through the list, get the mobs to trigger, THEN trigger them seperately
  // this is because response triggers can affect room contents (roomp->stuff)
  if (isPc()) {
    std::list<TMonster *> mobs;
    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
      tmp=*(it++);
      if (!tmp)
        continue;
      mob = dynamic_cast<TBeing *>(tmp);
      if (!mob)
        continue;
      if (mob == this || (mob->getPosition() <= POSITION_SLEEPING))
        continue;
      if (mob->isPc())
        continue;
      TMonster *tmons = dynamic_cast<TMonster *>(mob);
      if (!tmons)
        continue;
      mobs.push_front(tmons);
    }

    for(std::list<TMonster*>::const_iterator it = mobs.begin(); 
	it != mobs.end();) {
      TMonster *tmons = *(it++);
      tmons->aiSay(this, NULL);
      rc = tmons->checkResponses(this, 0, garbleRoom, CMD_SAY);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        delete tmons;
      if (IS_SET_DELETE(rc, DELETE_VICT)) 
        return DELETE_THIS;
    }
  }

  return FALSE;
}

void Descriptor::sendShout(TBeing *ch, const sstring &arg)
{
  Descriptor *i;
  const char *action = "shouts";
  int garbleFlags = ch->getGarbles(NULL);
  if (garbleFlags & Garble::TYPE_FLAG_BLAHBLAH)
    action = "whines";
  else if (garbleFlags & Garble::TYPE_FLAG_WAHWAH)
    action = "cries";

  for (i = descriptor_list; i; i = i->next) {
    if (i->connected != CON_PLYNG)
      continue;

    TBeing *b = i->character;
    if (!b)
      continue;
    if (b == ch)
      continue;
    if (b->checkSoundproof())
      continue;
    if (b->isPlayerAction(PLR_MAILING | PLR_BUGGING))
      continue;
    if (i->ignored.isIgnored(ch->desc))
      continue;

    // don't use awake(), paralyzed should hear, asleep should not
    if (b->getPosition() <= POSITION_SLEEPING)
      continue;

    // polys and switched always hear everything
    // if it's a god shouting (and I am mortal), hear it
    // if i'm not set noshout, hear it
    if (dynamic_cast<TMonster *>(b) ||
        (!b->isImmortal() && ch->isImmortal()) ||
        (b->desc && !IS_SET(i->autobits, AUTO_NOSHOUT))) {
      sstring shouter;
      shouter = b->pers(ch);
      if (shouter.empty()) {
        vlogf(LOG_BUG, "No shouter in sendShout!");
        continue;
      }

      // Somehow color codes are creeping into the names of pc's when they
      // shout.  We'll forcibly remove color codes from players's names.
      if (ch->isPc()) {
        sstring tmp_shouter;
        tmp_shouter = stripColorCodes(shouter);
        if (tmp_shouter != shouter) {
          vlogf(LOG_BUG, format("sendShout: shouter %s had an embedded color code") % shouter);
          shouter = tmp_shouter;
        }
      }

      sstring namebuf, namebufc, argbuf, messagebuf;

      argbuf = ch->garble(b, arg, Garble::SPEECH_SHOUT, Garble::SCOPE_INDIVIDUAL);

      namebufc = colorString(b, i, shouter.cap(), NULL, COLOR_NONE, FALSE);
      if (hasColorStrings(NULL, shouter, 2)) {
        if (IS_SET(b->desc->plr_color, PLR_COLOR_MOBS)) {
          namebuf = colorString(b, i, shouter.cap(), NULL, COLOR_MOBS, FALSE);
        } else {
          namebuf = colorString(b, i, shouter.cap(), NULL, COLOR_NONE, FALSE);
        }
      } else {
        if (IS_SET(b->desc->plr_color, PLR_COLOR_MOBS)) {
          namebuf = format("%s%s%s") % green() % shouter.cap() % norm();
        } else {
          namebuf = shouter.cap();
        }
      }

      if (hasColorStrings(NULL, argbuf, 2)) {
        if (IS_SET(b->desc->plr_color, PLR_COLOR_SHOUTS)) {
          messagebuf = colorString(b, i, argbuf, NULL, COLOR_BASIC, FALSE);
        } else {
          messagebuf = colorString(b, i, argbuf, NULL, COLOR_NONE, FALSE);
        }
      } else {
        messagebuf = colorString(b, i, argbuf, NULL, COLOR_NONE, FALSE);
      }

      if (i->m_bIsClient || IS_SET(i->prompt_d.type, PROMPT_CLIENT_PROMPT))
        i->clientf(format("%d|%s|%s") % CLIENT_SHOUT % namebufc % messagebuf);

      b->sendTo(COLOR_SHOUTS, format("%s %s, \"%s%s\"\n\r") %
        namebuf % action %  messagebuf % norm());
    }
  }
}


void TBeing::doShout(const sstring &arg)
{
  if (desc)
    desc->talkCount = time(0);

  if (GetMaxLevel() < 2) {
    sendTo("Sorry, you must be of higher level to shout.\n\r");
    return;
  }

  if (isAffected(AFF_SILENT)) {
    sendTo("You can't make a sound!\n\r");
    act("$n waves $s hands and points silently toward $s mouth.", 
  TRUE, this, 0, 0, TO_ROOM);
    return;
  }
  if (isPc() && ((desc && IS_SET(desc->autobits, AUTO_NOSHOUT)) || 
     isPlayerAction(PLR_GODNOSHOUT))) {
    sendTo("You can't shout!!\n\r");
    return;
  }

  if(hasQuestBit(TOG_IS_MUTE)){
    sendTo("You're mute, you can't talk.\n\r");
    return;
  }

  if (isDumbAnimal()) {
    sendTo("You are a dumb animal; you can't talk!\n\r");
    return;
  }
  if (isAffected(AFF_CHARM) && master) {
    if (!isPc()) {
      sendTo("What a dumb master you have, charmed mobiles can't shout.\n\r");
      master->sendTo("Stop ordering your charms to shout.  *scold*  \n\r");
    } else {
      sendTo("You're charmed, you can't shout.\n\r");
    }
    return;
  }
  if (!dynamic_cast<TMonster *>(this) && toggleInfo[TOG_SHOUTING]->toggle && 
      !isImmortal()) {
    sendTo("Shouting has been banned.\n\r");
    return;
  }
  if ((getMove() < 30) && isPc()) {
    sendTo("You don't have the energy to shout!\n\r");
    return;
  }
  if (applySoundproof())
    return;

  if (master && isAffected(AFF_CHARM)) {
    master->sendTo("I don't think so :-)\n\r");
    return;
  }
  if (rider) {
    rider->sendTo("I don't think so :-)\n\r");
    return;
  }
  if (arg.empty()){
    sendTo("You generally shout SOMETHING!\n\r");
    //sendTo("Shout? Yes! Fine! Shout we must, but WHAT??\n\r");
    return;
  }

  sstring garbled = garble(NULL, arg, Garble::SPEECH_SHOUT, Garble::SCOPE_EVERYONE);

  sendTo(COLOR_COMM, format("<g>You shout<Z>, \"%s%s\"\n\r") % 
   colorString(this, desc, garbled, NULL, COLOR_BASIC, FALSE) % norm());
  act("$n rears back $s head and shouts loudly.", FALSE, this, 0, 0, TO_ROOM);

  loseSneak();

  if (isPc())
    addToMove(-30);

  addToWait(combatRound(0.5));

  descriptor_list->sendShout(this, garbled);
  twitterShout(getName(), garbled);
}

void TBeing::doGrouptell(const sstring &arg)
{
  sstring buf, garbled, garbledTo;
  followData *f;
  TBeing *k;

  if (desc)
    desc->talkCount = time(0);

  if (applySoundproof())
    return;

  if(hasQuestBit(TOG_IS_MUTE)){
    sendTo("You're mute, you can't talk.\n\r");
    return;
  }

  if (isDumbAnimal()) {
    sendTo("You are a dumb animal; you can't talk!\n\r");
    return;
  }
  if (isAffected(AFF_SILENT)) {
    sendTo("You can't make a sound!\n\r");
    act("$n waves $s hands and points silently toward $s mouth.", TRUE, this, 0, 0, TO_ROOM);
    return;
  }
  if (!isAffected(AFF_GROUP)) {
    sendTo("You don't seem to have a group.\n\r");
    return;
  }

  if (!(k = master))
    k = this;

  if (arg.empty()) {
    sendTo("Grouptell is a good command, but you need to tell your group SOMEthing!\n\r");
    return;
  } else {

    garbled = garble(NULL, arg, Garble::SPEECH_GROUPTELL, Garble::SCOPE_EVERYONE);

    garbled.convertStringColor("<r>");

    sendTo(format("You tell your group: %s%s%s\n\r") % red() % colorString(this, desc, garbled, NULL, COLOR_BASIC, FALSE) % norm());
  }
  if (k->isAffected(AFF_GROUP) && !k->checkSoundproof()) {
    if (k->desc && !k->desc->ignored.isIgnored(desc) && (k->desc->m_bIsClient || IS_SET(k->desc->prompt_d.type, PROMPT_CLIENT_PROMPT)) && (k != this)) {
      k->desc->clientf(format("%d|%s|%s") % CLIENT_GROUPTELL % colorString(this, k->desc, getName(), NULL, COLOR_NONE, FALSE) % colorString(this, k->desc, garbled, NULL, COLOR_NONE, FALSE));
    }
    // a crash bug lies here....cut and paste from windows notepad
    // plays with the next few lines for some reason
    if (!k->desc || !k->desc->ignored.isIgnored(desc)) {
      garbledTo = garble(k, garbled, Garble::SPEECH_GROUPTELL, Garble::SCOPE_INDIVIDUAL);
      buf = format("$n: %s%s%s") % k->red() % colorString(this, k->desc, garbledTo, NULL, COLOR_COMM, FALSE) % k->norm();
      act(buf, 0, this, 0, k, TO_VICT);
    }
  }
  for (f = k->followers; f; f = f->next) {
    if ((f->follower != this) && f->follower->isAffected(AFF_GROUP) && !f->follower->checkSoundproof()) {

      if (f->follower->desc && f->follower->desc->ignored.isIgnored(desc))
        continue;

      // garble this string for the individual recipient
      garbledTo = garble(f->follower, garbled, Garble::SPEECH_GROUPTELL, Garble::SCOPE_INDIVIDUAL);

      if (f->follower->desc && (f->follower->desc->m_bIsClient || IS_SET(f->follower->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))) {
        f->follower->desc->clientf(format("%d|%s|%s") % CLIENT_GROUPTELL % colorString(this, k->desc, getName(), NULL, COLOR_NONE, FALSE) % colorString(this, f->follower->desc, garbledTo, NULL, COLOR_NONE, FALSE));
      }
      buf = format("$n: %s%s%s") % f->follower->red() % colorString(this, f->follower->desc, garbledTo, NULL, COLOR_COMM, FALSE) % f->follower->norm();
      act(buf, 0, this, 0, f->follower, TO_VICT);

    }
  }
}

void TBeing::doCommune(const sstring &arg)
{
  //sstring buf, buf2, wizbuf;
  sstring buf, buf2;
  Descriptor *i;
  TBeing *critter;
  int levnum = 0;

  if (!hasWizPower(POWER_WIZNET)) {
    // In general, deny this, but want to permit switched imms to still wiznet  
    if (!desc || !desc->original || 
        !desc->original->hasWizPower(POWER_WIZNET)) {
      incorrectCommand();
      return;
    }
  }

  if (arg.empty()) {
    sendTo("Communing among the gods is fine, but WHAT?\n\r");
    return;
  }

  if (arg.word(0)[0] == '@' && arg.word(0).length() >= 2) {
    buf2 = arg.word(0).substr(1,arg.word(0).length() -1);
    levnum = convertTo<int>(buf2);
    if (levnum > 0) {
      // only a properly formatted string should be changed
      if (arg.word(1).empty()) {
        sendTo(format("You need to tell level %d gods something!?!\n\r") % levnum);
        return;
      }
    } else 
      levnum = 0;
  }

  if (!levnum) {
    sendTo(format("You tell the gods: %s") %
         colorString(this, desc, arg, NULL, COLOR_BASIC, TRUE, TRUE));
  } else {
    if (levnum <= MAX_MORT) {
      sendTo(format("Hey dummy, all the gods are at least level %d.\n\r") %
             (MAX_MORT+1));
      return;
    }

    // wiznet at levnum > my_level ought to be allowed
    // "i think you guys ought to watch xxx, i suspect he is cheating"

    sendTo(format("You tell level %d+ gods: %s") % levnum %
         colorString(this, desc, 
           arg.substr(arg.find_first_of(" ")+1, arg.length()-1),
            NULL, COLOR_BASIC, TRUE, TRUE));
  }

  for (i = descriptor_list; i; i = i->next) {
    if (i->character != this && !i->connected) {
      if (dynamic_cast<TMonster *>(i->character) && i->original) {
        critter = i->original;
      } else
        critter = i->character;

      sstring str;

      if (!levnum) {
        str = colorString(this, i, arg, NULL, COLOR_COMM, FALSE);
        str.convertStringColor("<c>");
        if (critter->GetMaxLevel() >= GOD_LEVEL1 && 
      toggleInfo[TOG_WIZBUILD]->toggle) {
          buf = format("%s$n: %s%s%s") %
                 i->purple() % i->cyan() %
                 str % i->norm();
          act(buf, 0, this, 0, i->character, TO_VICT);

    if (!i->m_bIsClient && IS_SET(i->prompt_d.type, PROMPT_CLIENT_PROMPT))
      i->clientf(format("%d|%d|%d|%s|%s") % CLIENT_WIZNET % levnum % gamePort % getName() % str);
        } else if (critter->hasWizPower(POWER_WIZNET_ALWAYS)) {
          buf = format("[nobuilders] %s$n: %s%s%s") %
                 i->purple() % i->cyan() %
                 str % i->norm();
          act(buf, 0, this, 0, i->character, TO_VICT);

    if (!i->m_bIsClient && IS_SET(i->prompt_d.type, PROMPT_CLIENT_PROMPT))
      i->clientf(format("%d|%d|%d|%s|%s") % CLIENT_WIZNET % levnum % gamePort % getName() % str);
        }
      } else {
        str = colorString(this, i, 
          arg.substr(arg.find_first_of(" "), arg.length()-1),
          NULL, COLOR_COMM, FALSE);
        str.convertStringColor("<c>");
        
        if (critter->GetMaxLevel() >= GOD_LEVEL1 && 
      toggleInfo[TOG_WIZBUILD]->toggle &&
            critter->GetMaxLevel() >= levnum) {
          buf = format("%s[builders] (level: %d) $n: %s%s%s") %
                 i->purple() % levnum % i->cyan() %
                 str % i->norm();
          act(buf, 0, this, 0, i->character, TO_VICT);

    if (!i->m_bIsClient && IS_SET(i->prompt_d.type, PROMPT_CLIENT_PROMPT))
      i->clientf(format("%d|%d|%d|%s|%s") % CLIENT_WIZNET % levnum % gamePort % getName() % str);
        } else if (critter->hasWizPower(POWER_WIZNET_ALWAYS) &&
                   critter->GetMaxLevel() >= levnum) {
          buf = format("%s(level: %d) $n: %s%s%s") % 
                 i->purple() % levnum % i->cyan() %
                 str % i->norm();
          act(buf, 0, this, 0, i->character, TO_VICT);

    if (!i->m_bIsClient && IS_SET(i->prompt_d.type, PROMPT_CLIENT_PROMPT))
      i->clientf(format("%d|%d|%d|%s|%s") % CLIENT_WIZNET % levnum % gamePort % getName() % str);
        }
      }
    }
  }
  return;
}

int TBeing::doSign(const sstring &arg)
{
  sstring word, buf;
  TThing *t;
  int rc;
  sstring whitespace=" \f\n\r\t\v";
  

  if (arg.empty()) {
    sendTo("Yes, but WHAT do you want to sign?\n\r");
    return FALSE;
  }

  if (!roomp)
    return FALSE;

  if (!hasHands() || affectedBySpell(AFFECT_TRANSFORMED_ARMS) ||
                     affectedBySpell(AFFECT_TRANSFORMED_HANDS)) {
    sendTo("Yeah right...WHAT HANDS?!?!?!?!\n\r");
    return FALSE;
  }
  if (eitherArmHurt()) {
    sendTo("You REALLY need working arms to communicate like this...\n\r");
    return FALSE;
  }
  if (heldInPrimHand() || heldInSecHand()) {
    sendTo("You can't sign while holding things.\n\r");
    return FALSE;
  }
  if (fight()) {
    sendTo("You can't sign while fighting.\n\r");
    return FALSE;
  }

  buf = garble(NULL, arg, Garble::SPEECH_SIGN, Garble::SCOPE_EVERYONE);

  sendTo(format("You sign, \"%s\"\n\r") % arg);
  if (buf!=arg)
    sendTo("You're not sure you got it completely right.\n\r");

  buf = format("$n signs, \"%s\"") % buf;

  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
    t=*(it++);

    TBeing *ch = dynamic_cast<TBeing *>(t);
    if (!ch)
      continue;
    if (!ch->awake())
      continue;
    if (ch->desc && ch->desc->ignored.isIgnored(desc))
      continue;
    if (ch != this && ch->doesKnowSkill(SKILL_SIGN)) {
      if (bSuccess(SKILL_SIGN))
      {
        sstring bufToVict = garble(ch, buf, Garble::SPEECH_SIGN, Garble::SCOPE_INDIVIDUAL);
        act(bufToVict, TRUE, this, 0, ch, TO_VICT);
      }
        if (isPc() && !ch->isPc()) { 
          TMonster *tmons = dynamic_cast<TMonster *>(ch);
          tmons->aiSay(this, NULL);
          rc = tmons->checkResponses(this, 0, buf, CMD_SIGN);
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            delete tmons;
            tmons = NULL;
          }
          if (IS_SET_DELETE(rc, DELETE_VICT)) 
            return DELETE_THIS;
        }
      else {
        // thieves are sneaky, others wouldn't see them trying to sign
        if (!hasClass(CLASS_THIEF))
          act("$n makes funny motions with $s hands.", TRUE, this, 0, ch, TO_VICT);
      }
    } else {
      // thieves are sneaky, others wouldn't see them trying to sign
      if (!hasClass(CLASS_THIEF))
        act("$n makes funny motions with $s hands.", TRUE, this, 0, ch, TO_VICT);
    }
  }

  return FALSE;
}


sstring TellFromComm::getText(){
  return format("<p>%s<z> tells you, \"<c>%s<z>\"\n\r") %
    from % text;
}

sstring TellFromComm::getClientText(){
  return getText();
}

sstring TellFromComm::getXML(){
  sstring buf="";
  
  buf+=format("<tellfrom>\n");
  buf+=format("  <to>%s</to>\n") % to.escape(sstring::XML);
  buf+=format("  <from>%s</from>\n") % from.escape(sstring::XML);
  buf+=format("  <drunk>%s</drunk>\n") % (drunk ? "true" : "false");
  buf+=format("  <mob>%s</mob>\n") % (mob ? "true" : "false");
  buf+=format("  <tell>%s</tell>\n") % text.escape(sstring::XML);
  buf+=format("</tellfrom>\n");

  return buf;
}


sstring TellToComm::getText(){
  return format("<G>You tell %s<z>, \"%s\"\n\r") % to % text;
}

sstring TellToComm::getClientText(){
  return getText();
}

sstring TellToComm::getXML(){
  sstring buf="";
  
  buf+=format("<tellto>\n");
  buf+=format("  <to>%s</to>\n") % to.escape(sstring::XML);
  buf+=format("  <from>%s</from>\n") % from.escape(sstring::XML);
  buf+=format("  <tell>%s</tell>\n") % text.escape(sstring::XML);
  buf+=format("</tellto>\n");

  return buf;
}

TBeing *findTellTarget(TBeing *me, const sstring &name, bool visible, bool mobs){
  TBeing *vict;

  if (!(vict = get_pc_world(me, name, EXACT_YES, INFRA_NO, visible)))
    if (!(vict = get_pc_world(me, name, EXACT_NO, INFRA_NO, visible)) && mobs)
      if (!(vict = get_char_vis_world(me, name, NULL, EXACT_YES)))
	vict = get_char_vis_world(me, name, NULL, EXACT_NO);

  return vict;
}


// returns DELETE_THIS on death of this
// triggerSpecOnPerson prevents this from being constant
int TBeing::doTell(const sstring &name, const sstring &message, bool visible)
{
  TBeing *vict;
  sstring capbuf;
  int rc;

  if (isAffected(AFF_SILENT)) {
    sendTo(new CmdMsgComm("tell", "You can't make a sound!\n\r"));
    act("$n waves $s hands and points silently toward $s mouth.", TRUE, this, 0, 0, TO_ROOM);
    return FALSE;
  }
  if (!isImmortal() && applySoundproof())
    return FALSE;


  if (isDumbAnimal()) {
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", "You are a dumb animal; you can't talk!\n\r"));
    return FALSE;
  }
  if (isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL)) {
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", "What a dumb master you have, charmed mobiles can't tell.\n\r"));
    return FALSE;
  }

  if(name.empty() || message.empty()){
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", "Whom do you wish to tell what??\n\r"));
    return FALSE;
  } else {
    if(!(vict=findTellTarget(this, name, visible, true))){
      if(isImmortal()){
	TDatabase db(DB_SNEEZY);
	db.query("select p1.name as name from player p1, player p2, account a where p2.name='%s' and a.account_id=p2.account_id and p1.account_id=a.account_id", name.c_str());

	while(db.fetchRow()){
	  if((vict=findTellTarget(this, db["name"], visible, false))){
	    break;
	  }
	}
      }

      sendTo(new CmdMsgComm("tell", 
			    format("You fail to tell to '%s'\n\r") % name));

      // if vict isn't NULL here, it means we found another player logged in
      // under the same account
      if(vict){
	sendTo(new CmdMsgComm("tell", format("The player '%s' is logged in under the same account.\n\r") % vict->getName()));
      }

      return FALSE;
    }
  }

  if (isPlayerAction(PLR_GODNOSHOUT) && (vict->GetMaxLevel() <= MAX_MORT)) {
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", "You have been sanctioned by the gods and can't tell to them!!\n\r"));
    return FALSE;
  }
  if (this == vict) {
    sendTo(new CmdMsgComm("tell", "You try to tell yourself something.\n\r"));
    return FALSE;
  }

  // if a player doesnt want tells, only allow the person they last
  // talked to directly to tell back
  if (!isImmortal() && vict->desc && 
      IS_SET(vict->desc->autobits, AUTO_NOTELL) && 
      strcmp(vict->desc->last_told, this->name) != 0) {
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", "That person is not receiving tells. Try again later.\n\r"));
    return FALSE;
  }

  if(hasQuestBit(TOG_IS_MUTE) && (!vict->isImmortal() || !vict->isPc())){
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", "You're mute, you can't talk.\n\r"));
    return FALSE;
  }
  
  if ((vict->getPosition() == POSITION_SLEEPING) && !isImmortal()) {
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", format("%s is asleep, shhh.") %
		 ((sstring)(canSee(vict) ? vict->hssh() : "it")).cap()));
    return FALSE;
  }
  if (vict->getPosition() <= POSITION_STUNNED) { // Russ 01/06/95
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", format("%s is stunned or wounded badly and can't hear your tells!.") %
		 ((sstring)(canSee(vict) ? vict->hssh() : "it")).cap()));
    return FALSE;
  }
  if (dynamic_cast<TMonster *>(vict) && !(vict->desc)) {
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", "No-one by that name here.\n\r"));
    return FALSE;
  }
  if (!vict->desc) {
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", format("%s can't hear you.") %
		   ((sstring)(canSee(vict) ? vict->hssh() : "it")).cap()));
    return FALSE;
  }
  if (vict->desc->connected) {
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", format("%s is editing or writing. Try again later.") %
		((sstring)(canSee(vict) ? vict->hssh() : "it")).cap()));
    return FALSE;
  }
  if (!vict->desc->connected && vict->isPlayerAction(PLR_MAILING)) {
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", "They are mailing. Try again later.\n\r"));
    return FALSE;
  }
  if (!vict->desc->connected && vict->isPlayerAction(PLR_BUGGING)) {
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", "They are critiquing the mud.  Try again later.\n\r"));
    return FALSE;
  }
  if (vict->checkSoundproof() && !isImmortal()) {
    if(desc)
      desc->output.putInQ(new CmdMsgComm("tell", "Your words don't reach them; must be in a silent zone.\n\r"));
    return FALSE;
  }

  int drunkNum = getCond(DRUNK);
  sstring garbed;
  garbed = garble(vict, message, Garble::SPEECH_TELL);

  if(vict->isImmortal() && drunkNum>0)
    garbed=message;

  rc = vict->triggerSpecialOnPerson(this, CMD_OBJ_TOLD_TO_PLAYER, 
				    garbed.c_str());
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete vict;
    vict = NULL;
  }

  if (IS_SET_DELETE(rc, DELETE_VICT)) 
    return DELETE_THIS;

  if (rc)
    return FALSE;

  capbuf=vict->pers(this);

  sstring garbedBuf, nameBuf;

  sendTo(new TellToComm(vict->getName(), capbuf.cap(), garbed));

  Descriptor *d = vict->desc;

  // if the person is ignoring, just break off the tell now - they won't know
  if (d && d->ignored.isIgnored(desc))
    return FALSE;

  // we only color the sstring to the victim, so leave this AFTER
  // the stuff we send to the teller.
  garbed.convertStringColor("<c>");

  if(vict->isImmortal() && drunkNum>0)
    d->output.putInQ(new TellFromComm(vict->getName(), capbuf.cap(), garbed, true, !isPc()));
  else
    d->output.putInQ(new TellFromComm(vict->getName(), capbuf.cap(), garbed, false, !isPc()));

  TDatabase db(DB_SNEEZY);
  queryqueue.push(format("insert into tellhistory (tellfrom, tellto, tell, telltime) values ('%s', '%s', '%s', now())") % capbuf.cap().escape(sstring::SQL) % ((sstring)vict->getName()).escape(sstring::SQL) % garbed.escape(sstring::SQL));


  if (d && d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
    garbedBuf = format("<c>%s<z>") % garbed;
    d->clientf(format("%d|%s|%s") % CLIENT_TELL %
        colorString(vict, vict->desc, capbuf, NULL, COLOR_NONE, FALSE) %
        colorString(vict, vict->desc, garbedBuf, NULL, COLOR_NONE, FALSE));
  }

  // set up last teller for reply's use
  // If it becomes a "someone tells you", ignore
  if (vict->desc && vict->canSee(this, INFRA_YES) && isPc())
    strncpy(vict->desc->last_teller, this->name, cElements(vict->desc->last_teller));

  // if you told to someone, remember who you last told to for use later
  if (desc && vict->desc && isPc() && vict->isPc())
    strncpy(desc->last_told, vict->name, cElements(desc->last_told));

  if (desc && inGroup(*vict))
    desc->talkCount = time(0);

  if (vict->desc && (vict->isPlayerAction(PLR_AFK) || (IS_SET(vict->desc->autobits, AUTO_AFK) && (vict->getTimer() >= 5)))) 
    act("$N appears to be away from $S terminal at the moment.", TRUE, this, 0, vict, TO_CHAR);
 
  //  disturbMeditation(vict);
  return FALSE;
}


// returns DELETE_THIS if this should be toasted
int TBeing::doWhisper(const sstring &arg)
{
  TBeing *vict, *bOther = NULL;
  TThing *bThing=NULL;
  sstring name, message, buf;
  int rc;

  if (isAffected(AFF_SILENT)) {
    sendTo("You can't make a sound!\n\r");
    act("$n waves $s hands and points silently toward $s mouth.", TRUE, this, 0, 0, TO_ROOM);
    return FALSE;
  }
  if (applySoundproof())
    return FALSE;

  if (isDumbAnimal()) {
    sendTo("Beasts don't talk.\n\r");
    return FALSE;
  }
  message=arg;
  message=one_argument(message, name);

  if (name.empty() || message.empty()){
    sendTo("Whom do you want to whisper to.. and what??\n\r");
    return FALSE;
  }

  if (!(vict = get_char_room_vis(this, name))) {
    sendTo("No-one by that name here..\n\r");
    return FALSE;
  }
  if(hasQuestBit(TOG_IS_MUTE) && (!vict->isImmortal() || !vict->isPc())){
    sendTo("You're mute, you can't talk.\n\r");
    return FALSE;
  }

  if (vict == this) {
    act("$n whispers quietly to $mself.", TRUE, this, 0, 0, TO_ROOM);
    sendTo("You can't seem to get your mouth close enough to your ear...\n\r");
    return FALSE;
  }

  sstring garbed;
  garbed= garble(vict, message, Garble::SPEECH_WHISPER);

  buf = format("$n whispers to you, \"%s\"") % colorString(this, vict->desc, garbed, NULL, COLOR_COMM, TRUE);

  if (vict->desc && !vict->desc->m_bIsClient && IS_SET(vict->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))
    vict->desc->clientf(format("%d|%s|%s") % CLIENT_WHISPER % colorString(this, vict->desc, getName(), NULL, COLOR_NONE, FALSE) % colorString(this, vict->desc, garbed, NULL, COLOR_NONE, FALSE));

  if (!vict->desc || !vict->desc->ignored.isIgnored(desc))
    act(buf, TRUE, this, 0, vict, TO_VICT);
  sendTo(COLOR_MOBS, format("You whisper to %s, \"%s\"\n\r") % vict->getName() % colorString(this, desc, garbed, NULL, COLOR_BASIC, FALSE));
  act("$n whispers something to $N.", TRUE, this, 0, vict, TO_NOTVICT);

  // Lets check the room for any thives we might have using spy.
  // If it's a pc with spy, then they must be equal/greater than the speaker
  // level or they don't get the message.  And messages to/from immorts are
  // not overheard and immortals won't overhear messages either.
  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (bThing=*it);++it) {
    if ((bOther = dynamic_cast<TPerson *>(bThing)) &&
        bOther->desc && bOther->isPc() &&
        bOther->affectedBySpell(SKILL_SPY) &&
        bOther->GetMaxLevel() >= GetMaxLevel() &&
        bOther != this && bOther != vict &&
        !bOther->isImmortal() && !isImmortal() && !vict->isImmortal()) {
      buf = format("$n whispers to %s, \"%s\"") %
              vict->getName() %
  colorString(this, bOther->desc, garbed, NULL, COLOR_COMM, TRUE);
      act(buf, TRUE, this, 0, bOther, TO_VICT);
    }
  }

  if (!vict->desc || !vict->desc->ignored.isIgnored(desc))
    disturbMeditation(vict);

  if (!vict->isPc()) {
    rc = dynamic_cast<TMonster *>(vict)->checkResponses( this, NULL, garbed, CMD_WHISPER);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete vict;
      vict = NULL;
    } else if (IS_SET_DELETE(rc, DELETE_VICT)) {
      return DELETE_THIS;
    }
  }
  return FALSE;
}

// returns DELETE_THIS is this should go poof
int TBeing::doAsk(const sstring &arg)
{
  TBeing *vict;
  sstring name, message, garbled, buf;
  int rc;

  if (isAffected(AFF_SILENT)) {
    sendTo("You can't make a sound!\n\r");
    act("$n waves $s hands and points silently toward $s mouth.", TRUE, this, 0, 0, TO_ROOM);
    return FALSE;
  }
  if (applySoundproof())
    return FALSE;
  if (isDumbAnimal()) {
    sendTo("Beasts don't talk.\n\r");
    return FALSE;
  }

  message=arg;
  message=one_argument(message, name);

  if (name.empty() || message.empty())
    sendTo("Whom do you want to ask something...and what??\n\r");
  else if (!(vict = get_char_room_vis(this, name)))
    sendTo("No-one by that name here...\n\r");

  else if(hasQuestBit(TOG_IS_MUTE) && (!vict->isImmortal() || !vict->isPc())){
    sendTo("You're mute, you can't talk.\n\r");
    return FALSE;
  }

  else if (vict == this) {
    act("$n quietly asks $mself a question.", TRUE, this, 0, 0, TO_ROOM);
    sendTo("You think about it for a while...\n\r");
  } else {
    garbled=garble(vict, message, Garble::SPEECH_ASK);

    buf = format("$n asks you, \"%s\"") % garbled;
    if (!vict->desc || !vict->desc->ignored.isIgnored(desc))
      act(buf, TRUE, this, 0, vict, TO_VICT);
    sendTo(COLOR_MOBS, format("You ask %s, \"%s\"\n\r") %
     vict->getName() % garbled);

    if (vict->desc && !vict->desc->m_bIsClient && IS_SET(vict->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))
      vict->desc->clientf(format("%d|%s|%s") % CLIENT_ASK % colorString(this, vict->desc, getName(), NULL, COLOR_NONE, FALSE) % colorString(this, vict->desc, garbled, NULL, COLOR_NONE, FALSE));

    act("$n asks $N a question.", TRUE, this, 0, vict, TO_NOTVICT);
    if (!vict->desc || !vict->desc->ignored.isIgnored(desc))
      disturbMeditation(vict);
    if (!vict->isPc()) {
      rc = dynamic_cast<TMonster *>(vict)->checkResponses( this, NULL, garbled, CMD_ASK);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete vict;
        vict = NULL;
      } else if (IS_SET_DELETE(rc, DELETE_VICT)) {
        return DELETE_THIS;
      }
    }
  }
  return FALSE;
}

void TThing::writeMePen(TBeing *ch, TThing *)
{
  act("You can not write with $p.", FALSE, ch, this, 0, TO_CHAR);
}

void TThing::writeMeNote(TBeing *ch, TPen *)
{
  act("You can't write on $p.", FALSE, ch, this, 0, TO_CHAR);
}

void TNote::writeMeNote(TBeing *ch, TPen *)
{
  const int MAX_NOTE_LENGTH = 10000;   

  if (action_description) {
    ch->sendTo("There's something written on it already.\n\r");
    return;
  } else {
    // we can write - hooray! (This hooray is a ghee Stargazerism. - Russ)
    ch->sendTo("Ok...go ahead and write. End the note with a ~ or cancel with `.\n\r");

    // New memory stuff. Set up with its own sstrings, and set it strung - Russ 
    if (objVnum() >= 0) {
      swapToStrung();
    }
    act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
#if 0
    if (ch->desc->m_bIsClient)
      ch->desc->clientf(format("%d|%d") % CLIENT_STARTEDIT % MAX_NOTE_LENGTH);
#endif

    ch->desc->connected = CON_WRITING;
    ch->desc->str = &action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
  }
}

void TBeing::doWrite(const char *arg)
{
  TThing *paper = NULL, *pen = NULL;
  char papername[256], penname[256];

  argument_interpreter(arg, papername, cElements(papername), penname, cElements(penname));

  if (!desc)
    return;

  if (isAffected(AFF_BLIND)) {
    sendTo("You can't write when you can't see!!\n\r");
    return;
  }
  if(!*papername){
    for(StuffIter it=stuff.begin();it!=stuff.end() && (paper=*it);++it);
    if(!paper){
      sendTo("write (on) papername (with) penname.\n\r");
      return;
    }
  }
  if(!*penname){
    for(StuffIter it=stuff.begin();it!=stuff.end() && (pen=*it);++it);
    if(!pen){
      sendTo("write (on) papername (with) penname.\n\r");
      return;
    }
  }

  if (!paper && !(paper = searchLinkedListVis(this, papername, stuff))) {
    sendTo(format("You have no %s.\n\r") % papername);
    return;
  }
  if (!pen && !(pen = searchLinkedListVis(this, penname, stuff))) {
    sendTo(format("You have no %s.\n\r") % penname);
    return;
  }
  // ok.. now let's see what kind of stuff we've found
  pen->writeMePen(this, paper);
}

void TBeing::doReply(const sstring &arg)
{
  if (!desc || !*desc->last_teller) {
    sendTo("No one seems to have spoken to you lately.\n\r");
    return;
  }

  doTell(add_bars(desc->last_teller), arg, FALSE);
}

bool TBeing::canSpeak()
{
  affectedData *aff;

  if (isAffected(AFF_SILENT))
    return FALSE;
  if (checkSoundproof())
    return FALSE;
  if (isDumbAnimal())
    return FALSE;
  if(hasQuestBit(TOG_IS_MUTE))
    return FALSE;

  // this is modified from hasDisease().
  // slightly more efficient to do it like this
  for (aff = affected; aff; aff = aff->next) {
    if (aff->type == AFFECT_DISEASE) {
      if (aff->modifier == DISEASE_GARROTTE)
        return FALSE;
      if (aff->modifier == DISEASE_SUFFOCATE)
        return FALSE;
      if (aff->modifier == DISEASE_DROWNING)
        return FALSE;
    }
  }

  return TRUE;
}
