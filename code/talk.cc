//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "talk.cc" - All functions related to player communications
//  
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "disease.h"
#include "obj_pen.h"
#include "obj_note.h"
#include "database.h"

// many of the talk features colorize the says/tells/etc for easier viewing
// If I do "say this <r>color<z> is cool", I would expect to see color in
// red, and "this ", " is cool" be the 'normal' say color.
// unfortunately, turning off red (<z>) makes everything go back to
// normal, and we lose the 'normal' color.
// To get around this, we parse the say statement, and convert any <z>, <Z>,
// or <1> to a 'replacement' color sstring and then send it out.
// unfortunately, we also need to "unbold", so we need to send both the
// normal <z> as well as the replacement
static void convertStringColor(const sstring replacement, sstring & str)
{
  // we use <tmpi> to represent a dummy placeholder which we convert to
  // <z> at the end
  sstring repl = "<tmpi>";
  repl += replacement;
 
  while (str.find("<z>") != sstring::npos)  
    str.replace(str.find("<z>"), 3, repl);

  while (str.find("<Z>") != sstring::npos)  
    str.replace(str.find("<Z>"), 3, repl);

  while (str.find("<1>") != sstring::npos)  
    str.replace(str.find("<1>"), 3, repl);

  while (str.find("<tmpi>") != sstring::npos)  
    str.replace(str.find("<tmpi>"), 6, "<z>");
}

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
  TThing *tmp, *tmp2;
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

  garbleRoom = garble(NULL, arg, SPEECH_SAY, GARBLE_SCOPE_EVERYONE);
  
  sendTo(COLOR_COMM, fmt("<g>You say, <z>\"%s%s\"\n\r") % 
   colorString(this, desc, garbleRoom, NULL, COLOR_BASIC, FALSE) %
   norm());

  // show everyone in room the say.
  for (tmp = roomp->getStuff(); tmp; tmp = tmp2) {
    tmp2 = tmp->nextThing;
    
    if (!(mob = dynamic_cast<TBeing *>(tmp)))
      continue;
    
    if (!(d = mob->desc) || mob == this ||
        (mob->getPosition() <= POSITION_SLEEPING))
      continue;
    
    capbuf=sstring(mob->pers(this)).cap();
    tmpbuf = fmt("%s") % colorString(mob, mob->desc, capbuf, NULL, COLOR_NONE, FALSE); 
    
    if (mob->isPc()) {

      // note: this means only PCs get individualed garbles in a 'say'
      sstring garbleTo = garble(mob, garbleRoom, SPEECH_SAY, GARBLE_SCOPE_INDIVIDUAL);

      if (hasColorStrings(NULL, capbuf, 2)) {
        if (IS_SET(mob->desc->plr_color, PLR_COLOR_MOBS)) {
          tmpbuf = fmt("%s") % colorString(mob, mob->desc, capbuf, NULL, COLOR_MOBS, FALSE);
          mob->sendTo(COLOR_COMM, fmt("%s says, \"%s%s\"\n\r") % tmpbuf % garbleTo % mob->norm());
          if (d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
            garbedBuf = fmt("%s") % colorString(this, mob->desc, garbleTo, NULL, COLOR_NONE, FALSE);
            d->clientf(fmt("%d|%s|%s") % CLIENT_SAY % tmpbuf % garbedBuf);
          }
        } else {
          mob->sendTo(COLOR_COMM, fmt("<c>%s says, <z>\"%s\"\n\r") % tmpbuf % garbleTo);
            if (d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
              nameBuf = fmt("<c>%s<z>") % tmpbuf;
              garbedBuf = fmt("%s") % colorString(this, mob->desc, garbleTo, NULL, COLOR_NONE, FALSE);
              d->clientf(fmt("%d|%s|%s") % CLIENT_SAY % colorString(this, mob->desc, nameBuf, NULL, COLOR_NONE, FALSE) % garbedBuf);
            }
        }
      } else {
        mob->sendTo(COLOR_COMM, fmt("<c>%s says, <z>\"%s\"\n\r") % tmpbuf % garbleTo);
        if (d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
          nameBuf = fmt("<c>%s<z>") % tmpbuf;
          garbedBuf = fmt("%s") % colorString(this, mob->desc, garbleTo, NULL, COLOR_NONE, FALSE);
          d->clientf(fmt("%d|%s|%s") 
              % CLIENT_SAY 
              % colorString(this, mob->desc, nameBuf, NULL, COLOR_NONE, FALSE) 
              % garbedBuf);
        }
      }
    } else {
      mob->sendTo(COLOR_COMM, fmt("%s says, \"%s\"\n\r") % sstring(getName()).cap() % 
            colorString(this, mob->desc, garbleRoom, NULL, COLOR_COMM, FALSE));
      if (d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
        d->clientf(fmt("%d|%s|%s") % CLIENT_SAY % sstring(getName()).cap() %
       colorString(this, mob->desc, garbleRoom, NULL, COLOR_NONE, FALSE));
      }
    }
  }
  
  // everyone needs to see the say before the response gets triggered
  for (tmp = roomp->getStuff(); tmp; tmp = tmp2) {
    tmp2 = tmp->nextThing;
    mob = dynamic_cast<TBeing *>(tmp);
    if (!mob)
      continue;
    
    if (mob == this || (mob->getPosition() <= POSITION_SLEEPING))
      continue;
    
    if (isPc() && !mob->isPc()) { 
      TMonster *tmons = dynamic_cast<TMonster *>(mob);
      tmons->aiSay(this, NULL);
      rc = tmons->checkResponses(this, 0, garbleRoom, CMD_SAY);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete tmons;
        tmons = NULL;
      }
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
  if (garbleFlags & GARBLE_FLAG_BLAHBLAH)
    action = "whines";
  else if (garbleFlags & GARBLE_FLAG_WAHWAH)
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
          vlogf(LOG_BUG, fmt("sendShout: shouter %s had an embedded color code") % shouter);
          shouter = tmp_shouter;
        }
      }

      sstring namebuf, namebufc, argbuf, messagebuf;

      argbuf = ch->garble(b, arg, SPEECH_SHOUT, GARBLE_SCOPE_INDIVIDUAL);

      namebufc = colorString(b, i, shouter.cap(), NULL, COLOR_NONE, FALSE);
      if (hasColorStrings(NULL, shouter, 2)) {
        if (IS_SET(b->desc->plr_color, PLR_COLOR_MOBS)) {
          namebuf = colorString(b, i, shouter.cap(), NULL, COLOR_MOBS, FALSE);
        } else {
          namebuf = colorString(b, i, shouter.cap(), NULL, COLOR_NONE, FALSE);
        }
      } else {
        if (IS_SET(b->desc->plr_color, PLR_COLOR_MOBS)) {
          namebuf = fmt("%s%s%s") % green() % shouter.cap() % norm();
        } else {
          namebuf = shouter;
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
        i->clientf(fmt("%d|%s|%s") % CLIENT_SHOUT % namebufc % messagebuf);

      b->sendTo(COLOR_SHOUTS, fmt("%s %s, \"%s%s\"\n\r") %
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

  sstring garbled = garble(NULL, arg, SPEECH_SHOUT, GARBLE_SCOPE_EVERYONE);

  sendTo(COLOR_COMM, fmt("<g>You shout<Z>, \"%s%s\"\n\r") % 
   colorString(this, desc, garbled, NULL, COLOR_BASIC, FALSE) % norm());
  act("$n rears back $s head and shouts loudly.", FALSE, this, 0, 0, TO_ROOM);

  loseSneak();

  if (isPc())
    addToMove(-30);

  addToWait(combatRound(0.5));

  descriptor_list->sendShout(this, garbled);
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

    garbled = garble(NULL, arg, SPEECH_GROUPTELL, GARBLE_SCOPE_EVERYONE);

    convertStringColor("<r>", garbled);

    sendTo(fmt("You tell your group: %s%s%s\n\r") % red() % colorString(this, desc, garbled, NULL, COLOR_BASIC, FALSE) % norm());
  }
  if (k->isAffected(AFF_GROUP) && !k->checkSoundproof()) {
    if (k->desc && (k->desc->m_bIsClient || IS_SET(k->desc->prompt_d.type, PROMPT_CLIENT_PROMPT)) && (k != this)) {
      k->desc->clientf(fmt("%d|%s|%s") % CLIENT_GROUPTELL % colorString(this, k->desc, getName(), NULL, COLOR_NONE, FALSE) % colorString(this, k->desc, garbled, NULL, COLOR_NONE, FALSE));
    }
    // a crash bug lies here....cut and paste from windows notepad
    // plays with the next few lines for some reason
    garbledTo = garble(k, garbled, SPEECH_GROUPTELL, GARBLE_SCOPE_INDIVIDUAL);
    buf = fmt("$n: %s%s%s") % k->red() % colorString(this, k->desc, garbledTo, NULL, COLOR_COMM, FALSE) % k->norm();
    act(buf, 0, this, 0, k, TO_VICT);
  }
  for (f = k->followers; f; f = f->next) {
    if ((f->follower != this) && f->follower->isAffected(AFF_GROUP) && !f->follower->checkSoundproof()) {

      // garble this string for the individual recipient
      garbledTo = garble(f->follower, garbled, SPEECH_GROUPTELL, GARBLE_SCOPE_INDIVIDUAL);

      if (f->follower->desc && (f->follower->desc->m_bIsClient || IS_SET(f->follower->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))) {
        f->follower->desc->clientf(fmt("%d|%s|%s") % CLIENT_GROUPTELL % colorString(this, k->desc, getName(), NULL, COLOR_NONE, FALSE) % colorString(this, f->follower->desc, garbledTo, NULL, COLOR_NONE, FALSE));
      }
      buf = fmt("$n: %s%s%s") % f->follower->red() % colorString(this, f->follower->desc, garbledTo, NULL, COLOR_COMM, FALSE) % f->follower->norm();
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
        sendTo(fmt("You need to tell level %d gods something!?!\n\r") % levnum);
        return;
      }
    } else 
      levnum = 0;
  }

  if (!levnum) {
    sendTo(fmt("You tell the gods: %s") %
         colorString(this, desc, arg, NULL, COLOR_BASIC, TRUE, TRUE));
  } else {
    if (levnum <= MAX_MORT) {
      sendTo(fmt("Hey dummy, all the gods are at least level %d.\n\r") %
             (MAX_MORT+1));
      return;
    }

    // wiznet at levnum > my_level ought to be allowed
    // "i think you guys ought to watch xxx, i suspect he is cheating"

    sendTo(fmt("You tell level %d+ gods: %s") % levnum %
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
        convertStringColor("<c>", str);
        if (critter->GetMaxLevel() >= GOD_LEVEL1 && 
      toggleInfo[TOG_WIZBUILD]->toggle) {
          buf = fmt ("%s$n: %s%s%s") %
                 i->purple() % i->cyan() %
                 str % i->norm();
          act(buf, 0, this, 0, i->character, TO_VICT);

    if (!i->m_bIsClient && IS_SET(i->prompt_d.type, PROMPT_CLIENT_PROMPT))
      i->clientf(fmt("%d|%d|%d|%s|%s") % CLIENT_WIZNET % levnum % gamePort % getName() % str);
        } else if (critter->hasWizPower(POWER_WIZNET_ALWAYS)) {
          buf = fmt("[nobuilders] %s$n: %s%s%s") %
                 i->purple() % i->cyan() %
                 str % i->norm();
          act(buf, 0, this, 0, i->character, TO_VICT);

    if (!i->m_bIsClient && IS_SET(i->prompt_d.type, PROMPT_CLIENT_PROMPT))
      i->clientf(fmt("%d|%d|%d|%s|%s") % CLIENT_WIZNET % levnum % gamePort % getName() % str);
        }
      } else {
        str = colorString(this, i, 
          arg.substr(arg.find_first_of(" "), arg.length()-1),
          NULL, COLOR_COMM, FALSE);
        convertStringColor("<c>", str);
        
        if (critter->GetMaxLevel() >= GOD_LEVEL1 && 
      toggleInfo[TOG_WIZBUILD]->toggle &&
            critter->GetMaxLevel() >= levnum) {
          buf = fmt ("%s[builders] (level: %d) $n: %s%s%s") %
                 i->purple() % levnum % i->cyan() %
                 str % i->norm();
          act(buf, 0, this, 0, i->character, TO_VICT);

    if (!i->m_bIsClient && IS_SET(i->prompt_d.type, PROMPT_CLIENT_PROMPT))
      i->clientf(fmt("%d|%d|%d|%s|%s") % CLIENT_WIZNET % levnum % gamePort % getName() % str);
        } else if (critter->hasWizPower(POWER_WIZNET_ALWAYS) &&
                   critter->GetMaxLevel() >= levnum) {
          buf = fmt("%s(level: %d) $n: %s%s%s") % 
                 i->purple() % levnum % i->cyan() %
                 str % i->norm();
          act(buf, 0, this, 0, i->character, TO_VICT);

    if (!i->m_bIsClient && IS_SET(i->prompt_d.type, PROMPT_CLIENT_PROMPT))
      i->clientf(fmt("%d|%d|%d|%s|%s") % CLIENT_WIZNET % levnum % gamePort % getName() % str);
        }
      }
    }
  }
  return;
}

int TBeing::doSign(const sstring &arg)
{
  sstring word, buf;
  TThing *t, *t2;
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

  buf = garble(NULL, arg, SPEECH_SIGN, GARBLE_SCOPE_EVERYONE);

  sendTo(fmt("You sign, \"%s\"\n\r") % arg);
  if (buf!=arg)
    sendTo("You're not sure you got it completely right.\n\r");

  buf = fmt("$n signs, \"%s\"") % buf;

  for (t = roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;

    TBeing *ch = dynamic_cast<TBeing *>(t);
    if (!ch)
      continue;
    if (!ch->awake())
      continue;
    if (ch != this && ch->doesKnowSkill(SKILL_SIGN)) {
      if (bSuccess(SKILL_SIGN))
      {
        sstring bufToVict = garble(ch, buf, SPEECH_SIGN, GARBLE_SCOPE_INDIVIDUAL);
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


// returns DELETE_THIS on death of this
// triggerSpecOnPerson prevents this from being constant
int TBeing::doTell(const sstring &name, const sstring &message, bool visible)
{
  TBeing *vict;
  sstring capbuf;
  int rc;

  if (isAffected(AFF_SILENT)) {
    sendTo("You can't make a sound!\n\r");
    act("$n waves $s hands and points silently toward $s mouth.", TRUE, this, 0, 0, TO_ROOM);
    return FALSE;
  }
  if (!isImmortal() && applySoundproof())
    return FALSE;


  if (isDumbAnimal()) {
    sendTo("You are a dumb animal; you can't talk!\n\r");
    return FALSE;
  }
  if (isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL)) {
    sendTo("What a dumb master you have, charmed mobiles can't tell.\n\r");
    return FALSE;
  }

  if(name.empty() || message.empty()){
    sendTo("Whom do you wish to tell what??\n\r");
    return FALSE;
  } else if (!(vict = get_pc_world(this, name, EXACT_YES, INFRA_NO, visible))) {
    if (!(vict = get_pc_world(this, name, EXACT_NO, INFRA_NO, visible))) {
      if (!(vict = get_char_vis_world(this, name, NULL, EXACT_YES))) {
        if (!(vict = get_char_vis_world(this, name, NULL, EXACT_NO))) {
          sendTo(fmt("You fail to tell to '%s'\n\r") % name);
          return FALSE;
        }
      }
    }
  }
  if (isPlayerAction(PLR_GODNOSHOUT) && (vict->GetMaxLevel() <= MAX_MORT)) {
    sendTo("You have been sanctioned by the gods and can't tell to them!!\n\r");
    return FALSE;
  }
  if (this == vict) {
    sendTo("You try to tell yourself something.\n\r");
    return FALSE;
  }

  if(hasQuestBit(TOG_IS_MUTE) && (!vict->isImmortal() || !vict->isPc())){
    sendTo("You're mute, you can't talk.\n\r");
    return FALSE;
  }
  
  if ((vict->getPosition() == POSITION_SLEEPING) && !isImmortal()) {
    act("$E is asleep, shhh.", FALSE, this, 0, vict, TO_CHAR);
    return FALSE;
  }
  if (vict->getPosition() <= POSITION_STUNNED) { // Russ 01/06/95
    act("$E is stunned or wounded badly and can't hear your tells!.",
         FALSE, this, 0, vict, TO_CHAR);
    return FALSE;
  }
  if (dynamic_cast<TMonster *>(vict) && !(vict->desc)) {
    sendTo("No-one by that name here.\n\r");
    return FALSE;
  }
  if (!vict->desc) {
    act("$E can't hear you.", TRUE, this, NULL, vict, TO_CHAR);
    return FALSE;
  }
  if (vict->desc->connected) {
    act("$E is editing or writing. Try again later.", TRUE, this, NULL, vict, TO_CHAR);
    return FALSE;
  }
  if (!vict->desc->connected && vict->isPlayerAction(PLR_MAILING)) {
    sendTo("They are mailing. Try again later.\n\r");
    return FALSE;
  }
  if (!vict->desc->connected && vict->isPlayerAction(PLR_BUGGING)) {
    sendTo("They are critiquing the mud.  Try again later.\n\r");
    return FALSE;
  }
  if (vict->checkSoundproof() && !isImmortal()) {
    sendTo("Your words don't reach them; must be in a silent zone.\n\r");
    return FALSE;
  }

  int drunkNum = getCond(DRUNK);
  sstring garbed;
  garbed = garble(vict, message, SPEECH_TELL);

  if(vict->isImmortal() && drunkNum>0)
    garbed=message;
  rc = vict->triggerSpecialOnPerson(this, CMD_OBJ_TOLD_TO_PLAYER, garbed.c_str());
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

  if (vict->hasColor()) {
    if (hasColorStrings(NULL, capbuf, 2)) {
      if (IS_SET(vict->desc->plr_color, PLR_COLOR_MOBS)) {
        nameBuf = fmt("%s") % colorString(vict, vict->desc, capbuf.cap(), NULL, COLOR_MOBS, FALSE);
      } else {
        nameBuf = fmt("<p>%s<z>") % colorString(vict, vict->desc, capbuf.cap(), NULL, COLOR_NONE, FALSE);
      }
    } else {
      nameBuf = fmt("<p>%s<z>") % capbuf.cap();
    }
  } else {
    nameBuf = fmt("%s") % capbuf.cap();
  }

  sendTo(COLOR_COMM, fmt("<G>You tell %s<z>, \"%s\"\n\r") % vict->getName() % colorString(this, desc, garbed, NULL, COLOR_BASIC, FALSE));


  // we only color the sstring to the victim, so leave this AFTER
  // the stuff we send to the teller.
  convertStringColor("<c>", garbed);

  if(vict->isImmortal() && drunkNum>0){
    vict->sendTo(COLOR_COMM, fmt("%s drunkenly tells you, \"<c>%s<z>\"\n\r") %
     nameBuf % garbed);
  } else {
    vict->sendTo(COLOR_COMM, fmt("%s tells you, \"<c>%s<z>\"\n\r") %
     nameBuf % garbed);
  }

  TDatabase db(DB_SNEEZY);
  db.query("insert into tellhistory (tellfrom, tellto, tell, telltime) values ('%s', '%s', '%s', now())", capbuf.cap().c_str(), vict->getName(), garbed.c_str());

  // this is probably too slow, cron job or something would be better
  //  db.query("delete from tellhistory where tellto='%s' and telltime not in (select telltime from tellhistory where tellto='%s' order by telltime desc limit 25)", vict->getName(), vict->getName());



  Descriptor *d = vict->desc;
  if (d && d->m_bIsClient || IS_SET(d->prompt_d.type, PROMPT_CLIENT_PROMPT)) {
    garbedBuf = fmt("<c>%s<z>") % garbed;
    d->clientf(fmt("%d|%s|%s") % CLIENT_TELL %
        colorString(vict, vict->desc, nameBuf, NULL, COLOR_NONE, FALSE) %
        colorString(vict, vict->desc, garbedBuf, NULL, COLOR_NONE, FALSE));
  }

  // set up last teller for reply's use
  // If it becomes a "someone tells you", ignore
  if (vict->desc && vict->canSee(this, INFRA_YES) && isPc())
    strncpy(vict->desc->last_teller, this->name, cElements(vict->desc->last_teller));

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
  TThing *bThing;
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
  garbed= garble(vict, message, SPEECH_WHISPER);

  buf = fmt("$n whispers to you, \"%s\"") % colorString(this, vict->desc, garbed, NULL, COLOR_COMM, TRUE);

  if (vict->desc && !vict->desc->m_bIsClient && IS_SET(vict->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))
    vict->desc->clientf(fmt("%d|%s|%s") % CLIENT_WHISPER % colorString(this, vict->desc, getName(), NULL, COLOR_NONE, FALSE) % colorString(this, vict->desc, garbed, NULL, COLOR_NONE, FALSE));

  act(buf, TRUE, this, 0, vict, TO_VICT);
  sendTo(COLOR_MOBS, fmt("You whisper to %s, \"%s\"\n\r") % vict->getName() % colorString(this, desc, garbed, NULL, COLOR_BASIC, FALSE));
  act("$n whispers something to $N.", TRUE, this, 0, vict, TO_NOTVICT);

  // Lets check the room for any thives we might have using spy.
  // If it's a pc with spy, then they must be equal/greater than the speaker
  // level or they don't get the message.  And messages to/from immorts are
  // not overheard and immortals won't overhear messages either.
  for (bThing = roomp->getStuff(); bThing; bThing = bThing->nextThing) {
    if ((bOther = dynamic_cast<TPerson *>(bThing)) &&
        bOther->desc && bOther->isPc() &&
        bOther->affectedBySpell(SKILL_SPY) &&
        bOther->GetMaxLevel() >= GetMaxLevel() &&
        bOther != this && bOther != vict &&
        !bOther->isImmortal() && !isImmortal() && !vict->isImmortal()) {
      buf = fmt("$n whispers to %s, \"%s\"") %
              vict->getName() %
  colorString(this, bOther->desc, garbed, NULL, COLOR_COMM, TRUE);
      act(buf, TRUE, this, 0, bOther, TO_VICT);
    }
  }

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
    garbled=garble(vict, message, SPEECH_ASK);

    buf = fmt("$n asks you, \"%s\"") % garbled;
    act(buf, TRUE, this, 0, vict, TO_VICT);
    sendTo(COLOR_MOBS, fmt("You ask %s, \"%s\"\n\r") %
     vict->getName() % garbled);

    if (vict->desc && !vict->desc->m_bIsClient && IS_SET(vict->desc->prompt_d.type, PROMPT_CLIENT_PROMPT))
      vict->desc->clientf(fmt("%d|%s|%s") % CLIENT_ASK % colorString(this, vict->desc, getName(), NULL, COLOR_NONE, FALSE) % colorString(this, vict->desc, garbled, NULL, COLOR_NONE, FALSE));

    act("$n asks $N a question.", TRUE, this, 0, vict, TO_NOTVICT);
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
    ch->sendTo("Ok...go ahead and write. End the note with a ~.\n\r");

    // New memory stuff. Set up with its own sstrings, and set it strung - Russ 
    if (objVnum() >= 0) {
      swapToStrung();
    }
    act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
#if 0
    if (ch->desc->m_bIsClient)
      ch->desc->clientf(fmt("%d|%d") % CLIENT_STARTEDIT % MAX_NOTE_LENGTH);
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
    for(paper=getStuff();paper && !dynamic_cast<TNote *>(paper);paper=paper->nextThing);
    if(!paper){
      sendTo("write (on) papername (with) penname.\n\r");
      return;
    }
  }
  if(!*penname){
    for(pen=getStuff();pen && !dynamic_cast<TPen *>(pen);pen=pen->nextThing);
    if(!pen){
      sendTo("write (on) papername (with) penname.\n\r");
      return;
    }
  }

  if (!paper && !(paper = searchLinkedListVis(this, papername, getStuff()))) {
    sendTo(fmt("You have no %s.\n\r") % papername);
    return;
  }
  if (!pen && !(pen = searchLinkedListVis(this, penname, getStuff()))) {
    sendTo(fmt("You have no %s.\n\r") % penname);
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
