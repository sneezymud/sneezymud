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

// Make drunk people garble their words!
sstring TBeing::garble(const sstring &arg, int chance)
{
  sstring obuf, buf, latin;

  if (arg.empty())
    return "";

  if (chance <= 9)
    return arg;

  vector <sstring> args;
  vector <sstring>::iterator iter;
  unsigned int loc;
  argument_parser(arg, args);
  buf=obuf=arg;

  // first, lets turn things into pig latin, word by word
  for(iter=args.begin();iter!=args.end();++iter){
    ssprintf(latin, "%s%cay", (*iter).substr(1,(*iter).size()-1).c_str(),
	     (*iter)[0]);
    
    // replace the original word in obuf with whitespace
    // replace the original word in buf with the new word
    loc=obuf.find((*iter), 0);
    if(loc != sstring::npos){
      obuf.erase(loc, (*iter).size());
      obuf.insert(loc, latin.size(), ' ');
      buf.erase(loc, (*iter).size());
      buf.insert(loc, latin);
    }
  }
  
  // change some letters randomly
  for(unsigned int i=0;i<buf.size()-1;++i){
    if (::number(0, chance + 3) >= 10) {
      switch (buf[i]) {
        case 'a':
        case 'e':
        case 'i':
        case 'o':
        case 'u':
        case 'A':
        case 'E':
        case 'I':
        case 'O':
        case 'U':
          break;
        case 'z':
        case 'Z':
          buf[i] = 'y';
          break;
        default:
          if (isalpha(buf[i]))
            (buf[i])++;
          break;
      }
    }
  }
  return buf;
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
  sstring buf, garbed, capbuf, tmpbuf, nameBuf, garbedBuf;
  TThing *tmp, *tmp2;
  TBeing *mob = NULL;
  int rc;
  Descriptor *d;

  if (desc)
    desc->talkCount = time(0);

  if (applySoundproof())
    return FALSE;

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

  garbed=garble(arg, getCond(DRUNK));
  
  if (hasDisease(DISEASE_DROWNING)) 
    garbed="Glub glub glub.";
  
  sendTo(COLOR_COMM, "<g>You say, <z>\"%s%s\"\n\r", 
	 colorString(this, desc, garbed, NULL, COLOR_BASIC, FALSE).c_str(), 
	 norm());

  // show everyone in room the say.
  for (tmp = roomp->getStuff(); tmp; tmp = tmp2) {
    tmp2 = tmp->nextThing;
    
    if (!(mob = dynamic_cast<TBeing *>(tmp)))
      continue;
    
    if (!(d = mob->desc) || mob == this ||
	(mob->getPosition() <= POSITION_SLEEPING))
      continue;
    
    capbuf=good_cap(mob->pers(this));
    ssprintf(tmpbuf, "%s", colorString(mob, mob->desc, capbuf, NULL, COLOR_NONE, FALSE).c_str()); 
    
    if (mob->isPc()) {
      if (hasColorStrings(NULL, capbuf, 2)) {
	if (IS_SET(mob->desc->plr_color, PLR_COLOR_MOBS)) {
	  ssprintf(tmpbuf, "%s", colorString(mob, mob->desc, capbuf, NULL, COLOR_NONE, FALSE).c_str());
	  if (Lapspeak == 1) {
	    mob->sendTo(COLOR_COMM, "%s says, \"Heh.  %s%s  Heh.\"\n\r",
			tmpbuf.c_str(), garbed.c_str(), mob->norm());
	  } else {
	    mob->sendTo(COLOR_COMM, "%s says, \"%s%s\"\n\r", 
			tmpbuf.c_str(), garbed.c_str(), mob->norm());
	  }
	  if (d->m_bIsClient) {
	    ssprintf(garbedBuf, "%s", 
		     colorString(this, mob->desc, garbed, NULL, COLOR_NONE, FALSE).c_str());
	    d->clientf("%d|%s|%s", CLIENT_SAY, tmpbuf.c_str(), garbedBuf.c_str());
	  }
	} else {
	  if (Lapspeak == 1) {
	    mob->sendTo(COLOR_COMM, "<c>%s says, <z>\"Heh.  %s  Heh.\"\n\r",
			tmpbuf.c_str(), garbed.c_str());
	  } else {
	    mob->sendTo(COLOR_COMM, "<c>%s says, <z>\"%s\"\n\r",
			tmpbuf.c_str(), garbed.c_str());
	  }
	  if (d->m_bIsClient) {
	    ssprintf(nameBuf, "<c>%s<z>", tmpbuf.c_str());
	    ssprintf(garbedBuf, "%s", 
		     colorString(this, mob->desc, garbed, NULL, COLOR_NONE, FALSE).c_str());
	    d->clientf("%d|%s|%s", CLIENT_SAY, 
		       colorString(this, mob->desc, nameBuf, NULL, COLOR_NONE, FALSE).c_str(),
		       garbedBuf.c_str());
	  }
	}
      } else {
	if (Lapspeak == 1) {
	  mob->sendTo(COLOR_COMM, "<c>%s says, <z>\"Heh.  %s  Heh.\"\n\r",
		      tmpbuf.c_str(), garbed.c_str());
	} else {
	  mob->sendTo(COLOR_COMM, "<c>%s says, <z>\"%s\"\n\r",
		      tmpbuf.c_str(), garbed.c_str());
	}
	if (d->m_bIsClient) {
	  ssprintf(nameBuf, "<c>%s<z>", tmpbuf.c_str());
	  ssprintf(garbedBuf, "%s",
		   colorString(this, mob->desc, garbed, NULL, COLOR_NONE, FALSE).c_str());
	  d->clientf("%d|%s|%s", CLIENT_SAY,
		     colorString(this, mob->desc, nameBuf, NULL, COLOR_NONE, FALSE).c_str(),
		     garbedBuf.c_str());
	}
      }
    } else {
      if (Lapspeak == 1) {
	mob->sendTo(COLOR_COMM, "%s says, \"Heh.  %s  Heh.\"\n\r", good_cap(getName()).c_str(), 
		    colorString(this, mob->desc, garbed, NULL, COLOR_COMM, FALSE).c_str());
      } else {
	mob->sendTo(COLOR_COMM, "%s says, \"%s\"\n\r", good_cap(getName()).c_str(), 
		    colorString(this, mob->desc, garbed, NULL, COLOR_COMM, FALSE).c_str());
      }
      if (d->m_bIsClient) {
	d->clientf("%d|%s|%s", CLIENT_SAY, good_cap(getName()).c_str(),
		   colorString(this, mob->desc, garbed, NULL, COLOR_NONE, FALSE).c_str());
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
      rc = tmons->checkResponses(this, 0, garbed, CMD_SAY);
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

void Descriptor::sendShout(TBeing *ch, const char *arg)
{
  Descriptor *i;
  char capbuf[256];
  char namebuf[100];

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
      mud_str_copy(capbuf, b->pers(ch), 256);
      if (!capbuf) {
        forceCrash("No capbuf in sendShout!");
        continue;
      }
      sstring argbuf = colorString(b, i, arg, NULL, COLOR_NONE, FALSE);
      sprintf(namebuf, "<g>%s<z>", cap(capbuf));
      sstring nameStr = colorString(b, i, namebuf, NULL, COLOR_NONE, FALSE);
      if(hasColorStrings(NULL, capbuf, 2)) {
        if (IS_SET(b->desc->plr_color, PLR_COLOR_MOBS)) {
          sstring tmpbuf = colorString(b, i, cap(capbuf), NULL, COLOR_MOBS, FALSE);
          sstring tmpbuf2 = colorString(b, i, cap(capbuf), NULL, COLOR_NONE, FALSE);

          if (i->m_bIsClient)
            i->clientf("%d|%s|%s", CLIENT_SHOUT, tmpbuf2.c_str(), argbuf.c_str());
          b->sendTo(COLOR_SHOUTS, "%s shouts, \"%s<1>\"\n\r",tmpbuf.c_str(), arg);
        } else {
          if (i->m_bIsClient)
            i->clientf("%d|%s|%s%s", CLIENT_SHOUT, nameStr.c_str(), argbuf.c_str());

          b->sendTo(COLOR_SHOUTS, "<g>%s<z> shouts, \"%s<1>\"\n\r", cap(capbuf), arg);
        }
      } else {
        if (i->m_bIsClient)
          i->clientf("%d|%s|%s", CLIENT_SHOUT, nameStr.c_str(), argbuf.c_str());

        b->sendTo(COLOR_SHOUTS, "<g>%s<z> shouts, \"%s<1>\"\n\r", cap(capbuf), arg);
      }
    }
  }
}
void TBeing::doShout(const char *arg)
{
  char garbed[256];

  if (desc)
    desc->talkCount = time(0);

  if (GetMaxLevel() < 2) {
    sendTo("Sorry, you must be of higher level to shout.\n\r");
    return;
  }

  if (isAffected(AFF_SILENT)) {
    sendTo("You can't make a sound!\n\r");
    act("$n waves $s hands and points silently toward $s mouth.", TRUE, this, 0, 0, TO_ROOM);
    return;
  }
  if (isPc() && ((desc && IS_SET(desc->autobits, AUTO_NOSHOUT)) || isPlayerAction(PLR_GODNOSHOUT))) {
    sendTo("You can't shout!!\n\r");
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
  if (!dynamic_cast<TMonster *>(this) && (Silence == 1) && !isImmortal()) {
    sendTo("Shouting has been banned.\n\r");
    return;
  }
  if ((getMove() < 30) && isPc()) {
    sendTo("You don't have the energy to shout!\n\r");
    return;
  }
  if (applySoundproof())
    return;

  for (; isspace(*arg); arg++);

  if (master && isAffected(AFF_CHARM)) {
    master->sendTo("I don't think so :-)\n\r");
    return;
  }
  if (rider) {
    rider->sendTo("I don't think so :-)\n\r");
    return;
  }
  if (!*arg) {
    sendTo("You generally shout SOMETHING!\n\r");
    //sendTo("Shout? Yes! Fine! Shout we must, but WHAT??\n\r");
    return;
  }
  if ((roomp->isUnderwaterSector() || hasDisease(DISEASE_DROWNING)) &&
       !isImmortal())
    mud_str_copy(garbed, "Glub glub glub.", 256);
  else
    mud_str_copy(garbed, garble(arg, getCond(DRUNK)), 256);

  sendTo(COLOR_COMM, "<g>You shout<Z>, \"%s%s\"\n\r", colorString(this, desc, garbed, NULL, COLOR_BASIC, FALSE).c_str(), norm());
  act("$n rears back $s head and shouts loudly.", FALSE, this, 0, 0, TO_ROOM);

  loseSneak();

  if (isPc())
    addToMove(-30);

  addToWait(combatRound(0.5));
  descriptor_list->sendShout(this, garbed);
}

void TBeing::doGrouptell(const char *arg)
{
  sstring buf, garbed;
  followData *f;
  TBeing *k;

  if (desc)
    desc->talkCount = time(0);

  if (applySoundproof())
    return;

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

  for (; isspace(*arg); arg++);

  if (!*arg) {
    sendTo("Grouptell is a good command, but you need to tell your group SOMEthing!\n\r");
    return;
  } else {
    garbed = garble(arg, getCond(DRUNK));

    convertStringColor("<r>", garbed);

    sendTo("You tell your group: %s%s%s\n\r", red(), colorString(this, desc, garbed.c_str(), NULL, COLOR_BASIC, FALSE).c_str(), norm());
  }
  if (k->isAffected(AFF_GROUP) && !k->checkSoundproof()) {
    if (k->desc && k->desc->m_bIsClient && (k != this)) {
      k->desc->clientf("%d|%s|%s", CLIENT_GROUPTELL, getName(), 
        colorString(this, k->desc, garbed.c_str(), NULL, COLOR_NONE, FALSE).c_str());
    }
    // a crash bug lies here....cut and paste from windows notepad
    // plays with the next few lines for some reason
    ssprintf(buf, "$n: %s%s%s", k->red(), colorString(this, k->desc, garbed.c_str(), NULL, COLOR_COMM, FALSE).c_str(), k->norm());
    act(buf, 0, this, 0, k, TO_VICT);
  }
  for (f = k->followers; f; f = f->next) {
    if ((f->follower != this) && f->follower->isAffected(AFF_GROUP) && !f->follower->checkSoundproof()) {
      if (f->follower->desc && f->follower->desc->m_bIsClient) {
        f->follower->desc->clientf("%d|%s|%s", CLIENT_GROUPTELL, getName(), 
          colorString(this, f->follower->desc, garbed.c_str(), NULL, COLOR_NONE, FALSE).c_str());
      }
      ssprintf(buf, "$n: %s%s%s", f->follower->red(), colorString(this, f->follower->desc, garbed.c_str(), NULL, COLOR_COMM, FALSE).c_str(), f->follower->norm());
      act(buf, 0, this, 0, f->follower, TO_VICT);
    }
  }
}

void TBeing::doCommune(const char *arg)
{
  char buf[256];
  char buf2[256];
  char wizbuf[256];
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

  for (; isspace(*arg); arg++);

  if (!*arg) {
    sendTo("Communing among the gods is fine, but WHAT?\n\r");
    return;
  }
  if (*arg == '@') {
    one_argument(arg, buf2);
    mud_str_copy(buf2, &buf2[1],256);  // skip the @
    levnum = convertTo<int>(buf2);
    if (levnum > 0) {
      // only a properly formatted sstring should be changed
      arg = one_argument(arg, buf2);
      for (; isspace(*arg); arg++);
      if (!*arg) {
        sendTo("You need to tell level %d gods something!?!\n\r", levnum);
        return;
      }
    } else 
      levnum = 0;
  }

  if (!levnum) {
    sendTo("You tell the gods: %s",
         colorString(this, desc, arg, NULL, COLOR_BASIC, TRUE, TRUE).c_str());
    sprintf(wizbuf, "[%sPort:%d%s] %s%s:%s %s%s%s\n\r", red(), gamePort, norm(), purple(), getName(), norm(), cyan(), arg, norm());
    mudMessage(this, 16, wizbuf);
  } else {
    if (levnum <= MAX_MORT) {
      sendTo("Hey dummy, all the gods are at least level %d.\n\r",
             MAX_MORT+1);
      return;
    }

    // wiznet at levnum > my_level ought to be allowed
    // "i think you guys ought to watch xxx, i suspect he is cheating"

    sendTo("You tell level %d+ gods: %s", levnum,
         colorString(this, desc, arg, NULL, COLOR_BASIC, TRUE, TRUE).c_str());
  }

  for (i = descriptor_list; i; i = i->next) {
    if (i->character != this && !i->connected) {
      if (dynamic_cast<TMonster *>(i->character) && i->original) {
        critter = i->original;
      } else
        critter = i->character;

      sstring str = colorString(this, i, arg, NULL, COLOR_COMM, FALSE);
      convertStringColor("<c>", str);

      if (!levnum) {
        if (critter->GetMaxLevel() >= GOD_LEVEL1 && WizBuild) {
          sprintf(buf, "%s$n: %s%s%s",
                 i->purple(), i->cyan(),
                 str.c_str(), i->norm());
          act(buf, 0, this, 0, i->character, TO_VICT);
        } else if (critter->hasWizPower(POWER_WIZNET_ALWAYS)) {
          sprintf(buf, "[nobuilders] %s$n: %s%s%s",
                 i->purple(), i->cyan(),
                 str.c_str(), i->norm());
          act(buf, 0, this, 0, i->character, TO_VICT);
        }
      } else {
        if (critter->GetMaxLevel() >= GOD_LEVEL1 && WizBuild &&
            critter->GetMaxLevel() >= levnum) {
          sprintf(buf, "%s[builders] (level: %d) $n: %s%s%s",
                 i->purple(), levnum, i->cyan(),
                 str.c_str(), i->norm());
          act(buf, 0, this, 0, i->character, TO_VICT);
        } else if (critter->hasWizPower(POWER_WIZNET_ALWAYS) &&
                   critter->GetMaxLevel() >= levnum) {
          sprintf(buf, "%s(level: %d) $n: %s%s%s", 
                 i->purple(), levnum, i->cyan(),
                 str.c_str(), i->norm());
          act(buf, 0, this, 0, i->character, TO_VICT);
        }
      }
    }
  }
  return;
}

const sstring RandomWord()
{
  static sstring str[50] =
  {
    "argle",
    "bargle",
    "glop",
    "glyph",
    "hussamah",                 // 5 
    "rodina",
    "mustafah",
    "angina",
    "the",
    "fribble",                  // 10 */
    "fnort",
    "frobozz",
    "zarp",
    "ripple",
    "yrk",                      // 15 */
    "yid",
    "yerf",
    "oork",
    "beavis",
    "butthead",                 // 20 */
    "rod",
    "johnson",
    "tool",
    "ftagn",
    "hastur",                   // 25 */
    "brob",
    "gnort",
    "lram",
    "truck",
    "kill",                     // 30 */
    "cthulhu",
    "huzzah",
    "fish",
    "chicken",
    "summah",                   // 35 */
    "hummah",
    "cookies",
    "stan",
    "will",
    "wadapatang",               // 40 */
    "pterodactyl",
    "frob",
    "yuma",
    "gumma",
    "lo-pan",                   // 45 */
    "sushi",
    "yaya",
    "yoyodine",
    "your",
    "mother"                    // 50 */
  };
  return (str[number(0, 49)]);
}


void TBeing::doSign(const sstring &arg)
{
  sstring word, buf;
  TThing *t;
  sstring whitespace=" \f\n\r\t\v";
  

  if (arg.empty()) {
    sendTo("Yes, but WHAT do you want to sign?\n\r");
    return;
  }

  if (!roomp)
    return;

  if (!hasHands() || affectedBySpell(AFFECT_TRANSFORMED_ARMS) ||
                     affectedBySpell(AFFECT_TRANSFORMED_HANDS)) {
    sendTo("Yeah right...WHAT HANDS?!?!?!?!\n\r");
    return;
  }
  if (eitherArmHurt()) {
    sendTo("You REALLY need working arms to communicate like this...\n\r");
    return;
  }
  if (heldInPrimHand() || heldInSecHand()) {
    sendTo("You can't sign while holding things.\n\r");
    return;
  }
  if (fight()) {
    sendTo("You can't sign while fighting.\n\r");
    return;
  }

  // work through the arg, word by word.  if you fail your
  //  skill roll, the word comes out garbled. */

  unsigned int pos=0;

  while(pos!=sstring::npos){
    word=arg.substr(pos, arg.find_first_of(whitespace,pos)-pos);
    
    if(bSuccess(this, getSkillValue(SKILL_SIGN) - buf.length(), SKILL_SIGN))
      buf+=word;
    else
      buf+=RandomWord();

    // skip past the whitespace to the beginning of the next word
    pos=arg.find_first_of(whitespace,pos);
    if(pos!=sstring::npos)
      buf+=arg.substr(pos, arg.find_first_not_of(whitespace,pos)-pos);
    pos=arg.find_first_not_of(whitespace,pos);
  }

  sendTo("You sign, \"%s\"\n\r", arg.c_str());
  if (buf!=arg)
    sendTo("You're not sure you got it completely right.\n\r");

  ssprintf(buf, "$n signs, \"%s\"", buf.c_str());

  for (t = roomp->getStuff(); t; t = t->nextThing) {
    TBeing *ch = dynamic_cast<TBeing *>(t);
    if (!ch)
      continue;
    if (!ch->awake())
      continue;
    if (ch != this && ch->doesKnowSkill(SKILL_SIGN)) {
      if (bSuccess(ch, ch->getSkillValue(SKILL_SIGN), SKILL_SIGN))
        act(buf, TRUE, this, 0, ch, TO_VICT);
      else {
        // thieves are sneaky, others wouldn't see them trying to sign
        if (!hasClass(CLASS_THIEF))
          act("$n makes funny motions with $s hands.", TRUE, this, 0, ch, TO_VICT);
      }
    }
  }
}

// uses printf style arguments
int TBeing::doTell(const sstring &name, const char *fmt, ...)
{
  va_list ap;
  char buf[MAX_STRING_LENGTH];

  va_start(ap, fmt);
  vsnprintf(buf, MAX_STRING_LENGTH, fmt, ap);

  sstring sbuf;
  ssprintf(sbuf, "%s %s", name.c_str(), buf);
  
  return doTell(sbuf);
}

// returns DELETE_THIS on death of this
// triggerSpecOnPerson prevents this from being constant
int TBeing::doTell(const sstring &arg, bool visible)
{
  TBeing *vict;
  sstring name, capbuf, message;
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
  message=arg;
  one_argument(message, name);

  if(name.empty() || message.empty()){
    sendTo("Whom do you wish to tell what??\n\r");
    return FALSE;
  } else if (!(vict = get_pc_world(this, name, EXACT_YES, INFRA_NO, visible))) {
    if (!(vict = get_pc_world(this, name, EXACT_NO, INFRA_NO, visible))) {
      if (!(vict = get_char_vis_world(this, name, NULL, EXACT_YES))) {
        if (!(vict = get_char_vis_world(this, name, NULL, EXACT_NO))) {
          sendTo("You fail to tell to '%s'\n\r", name.c_str());
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
  garbed = garble(message, drunkNum);

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

  if ((roomp->isUnderwaterSector() || hasDisease(DISEASE_DROWNING)) &&
       !isImmortal() && !vict->isImmortal())
    garbed = "Glub glub glub.";

  capbuf=vict->pers(this);

  sstring garbedBuf, nameBuf;

  if (vict->hasColor()) {
    if (hasColorStrings(NULL, capbuf, 2)) {
      if (IS_SET(vict->desc->plr_color, PLR_COLOR_MOBS)) {
        ssprintf(nameBuf, "%s", colorString(vict, vict->desc, good_cap(capbuf), NULL, COLOR_MOBS, FALSE).c_str());
      } else {
        ssprintf(nameBuf, "<p>%s<z>", colorString(vict, vict->desc, good_cap(capbuf), NULL, COLOR_NONE, FALSE).c_str());
      }
    } else {
      ssprintf(nameBuf, "<p>%s<z>", good_cap(capbuf).c_str());
    }
  } else {
    ssprintf(nameBuf, "%s", good_cap(capbuf).c_str());
  }

  sendTo(COLOR_COMM, "<G>You tell %s<z>, \"%s\"\n\r", vict->getName(), colorString(this, desc, garbed.c_str(), NULL, COLOR_BASIC, FALSE).c_str());


  // we only color the sstring to the victim, so leave this AFTER
  // the stuff we send to the teller.
  convertStringColor("<c>", garbed);

  if(vict->isImmortal() && drunkNum>0){
    vict->sendTo(COLOR_COMM, "%s drunkenly tells you, \"<c>%s<z>\"\n\r",
		 nameBuf.c_str(), garbed.c_str());
  } else {
    vict->sendTo(COLOR_COMM, "%s tells you, \"<c>%s<z>\"\n\r",
		 nameBuf.c_str(), garbed.c_str());
  }

  TDatabase db(DB_SNEEZY);
  db.query("insert into tellhistory (tellfrom, tellto, tell, telltime) values ('%s', '%s', '%s', now())", good_cap(capbuf).c_str(), vict->getName(), garbed.c_str());

  // this is probably too slow, cron job or something would be better
  //  db.query("delete from tellhistory where tellto='%s' and telltime not in (select telltime from tellhistory where tellto='%s' order by telltime desc limit 25)", vict->getName(), vict->getName());



  Descriptor *d = vict->desc;
  if (d->m_bIsClient) {
    ssprintf(garbedBuf, "<c>%s<z>", garbed.c_str());
    d->clientf("%d|%s|%s", CLIENT_TELL,
        colorString(vict, vict->desc, nameBuf, NULL, COLOR_NONE, FALSE).c_str(),
        colorString(vict, vict->desc, garbedBuf, NULL, COLOR_NONE, FALSE).c_str());
  }

  // set up last teller for reply's use
  // If it becomes a "someone tells you", ignore
  if (vict->desc && vict->canSee(this, INFRA_YES))
    strcpy(vict->desc->last_teller, this->name);

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
  one_argument(message, name);

  if (name.empty() || message.empty()){
    sendTo("Whom do you want to whisper to.. and what??\n\r");
    return FALSE;
  }
  if (!(vict = get_char_room_vis(this, name))) {
    sendTo("No-one by that name here..\n\r");
    return FALSE;
  }
  if (vict == this) {
    act("$n whispers quietly to $mself.", TRUE, this, 0, 0, TO_ROOM);
    sendTo("You can't seem to get your mouth close enough to your ear...\n\r");
    return FALSE;
  }

  sstring garbed;
  garbed= garble(message, getCond(DRUNK));

  ssprintf(buf, "$n whispers to you, \"%s\"", colorString(this, vict->desc, garbed, NULL, COLOR_COMM, TRUE).c_str());

  act(buf, TRUE, this, 0, vict, TO_VICT);
  sendTo(COLOR_MOBS, "You whisper to %s, \"%s\"\n\r", vict->getName(), colorString(this, desc,garbed, NULL, COLOR_BASIC, FALSE).c_str());
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
      ssprintf(buf, "$n whispers to %s, \"%s\"",
              vict->getName(),
              colorString(this, bOther->desc, garbed, NULL, COLOR_COMM, TRUE).c_str());
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
  else if (vict == this) {
    act("$n quietly asks $mself a question.", TRUE, this, 0, 0, TO_ROOM);
    sendTo("You think about it for a while...\n\r");
  } else {
    garbled=garble(message, getCond(DRUNK));

    ssprintf(buf, "$n asks you, \"%s\"", garbled.c_str());
    act(buf, TRUE, this, 0, vict, TO_VICT);
    sendTo(COLOR_MOBS, "You ask %s, \"%s\"\n\r",
	   vict->getName(), garbled.c_str());
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
      ch->desc->clientf("%d|%d", CLIENT_STARTEDIT, MAX_NOTE_LENGTH);
#endif

    ch->desc->connected = CON_WRITING;
    ch->desc->str = &action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
  }
}

void TBeing::doWrite(const char *arg)
{
  TThing *paper = NULL, *pen = NULL;
  char papername[MAX_INPUT_LENGTH], penname[MAX_INPUT_LENGTH];

  argument_interpreter(arg, papername, penname);

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
    sendTo("You have no %s.\n\r", papername);
    return;
  }
  if (!pen && !(pen = searchLinkedListVis(this, penname, getStuff()))) {
    sendTo("You have no %s.\n\r", penname);
    return;
  }
  // ok.. now let's see what kind of stuff we've found
  pen->writeMePen(this, paper);
}

void TBeing::doReply(const sstring &arg)
{
  sstring buf;

  if (!desc || !*desc->last_teller) {
    sendTo("No one seems to have spoken to you lately.\n\r");
    return;
  }

  ssprintf(buf, "%s %s", add_bars(desc->last_teller).c_str(), arg.c_str());
  doTell(buf, FALSE);
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
