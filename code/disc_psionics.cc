//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_psionics.h"

CDPsionics::CDPsionics() :
  CDiscipline(),
  skTelepathy(),
  skTeleSight(),
  skTeleVision(),
  skMindFocus(),
  skPsiBlast(),
  skMindThrust(),
  skPsyCrush(),
  skKineticWave(),
  skMindPreservation(),
  skTelekinesis()
{
}

CDPsionics::CDPsionics(const CDPsionics &a) :
  CDiscipline(a),
  skTelepathy(a.skTelepathy),
  skTeleSight(a.skTeleSight),
  skTeleVision(a.skTeleVision),
  skMindFocus(a.skMindFocus),
  skPsiBlast(a.skPsiBlast),
  skMindThrust(a.skMindThrust),
  skPsyCrush(a.skPsyCrush),
  skKineticWave(a.skKineticWave),
  skMindPreservation(a.skMindPreservation),
  skTelekinesis(a.skTelekinesis)
{
}

CDPsionics & CDPsionics::operator=(const CDPsionics &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  //  skAdvancedPsionics = a.skAdvancedPsionics;
  skTelepathy = a.skTelepathy;
  skTeleSight = a.skTeleSight;
  skTeleVision = a.skTeleVision;
  skMindFocus = a.skMindFocus;
  skPsiBlast = a.skPsiBlast;
  skMindThrust = a.skMindThrust;
  skPsyCrush = a.skPsyCrush;
  skKineticWave = a.skKineticWave;
  skMindPreservation = a.skMindPreservation;
  skTelekinesis = a.skTelekinesis;

  return *this;
}

CDPsionics::~CDPsionics()
{
}


// many of the talk features colorize the says/tells/etc for easier viewing
// If I do "say this <r>color<z> is cool", I would expect to see color in
// red, and "this ", " is cool" be the 'normal' say color.
// unfortunately, turning off red (<z>) makes everything go back to
// normal, and we lose the 'normal' color.
// To get around this, we parse the say statement, and convert any <z>, <Z>,
// or <1> to a 'replacement' color string and then send it out.
// unfortunately, we also need to "unbold", so we need to send both the
// normal <z> as well as the replacement
static void convertStringColor(const string replacement, string & str)
{
  // we use <tmpi> to represent a dummy placeholder which we convert to
  // <z> at the end
  string repl = "<tmpi>";
  repl += replacement;
 
  while (str.find("<z>") != string::npos)  
    str.replace(str.find("<z>"), 3, repl);

  while (str.find("<Z>") != string::npos)  
    str.replace(str.find("<Z>"), 3, repl);

  while (str.find("<1>") != string::npos)  
    str.replace(str.find("<1>"), 3, repl);

  while (str.find("<tmpi>") != string::npos)  
    str.replace(str.find("<tmpi>"), 6, "<z>");
}

// Make drunk people garble their words!
static string garble(const char *arg, int chance)
{
  char *tmp;
  char temp[256];
  char word[80], latin[80];
  int i;

  if (!*arg)
    return "";

  if (chance <= 9)
    return arg;

  for (;!isalpha(*arg); arg++);
// get rid of bad things at the beginning of string

  // first, lets turn things into pig latin
  *temp = '\0';
  *word = '\0';
  i = 0;
  while (*arg) {
    while (*arg && isalpha(*arg)) {
      word[i++] = *arg;
      arg++;
    }
    word[i] = '\0';
    i = 0;
    sprintf(latin, "%s%cay", &word[1], word[0]);
    sprintf(temp, "%s%s%c", temp, latin, *arg);
    if (*arg)
      arg++;
  }
  
  // Since say handles spaces, no need to worry about them 
  for (tmp = temp; *tmp; tmp++) {
    if (number(0, chance + 3) >= 10) {
      switch (*tmp) {
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
          *tmp = 'y';
          break;
        default:
          if (isalpha(*tmp))
            (*tmp)++;
          break;
      }
    }
  }
  return (temp);
}


int TBeing::doPTell(const char *arg, bool visible){
  TBeing *vict;
  char name[100], capbuf[256], message[MAX_INPUT_LENGTH + 40];
  int rc, drunkNum=0;

  if(!doesKnowSkill(SKILL_PSITELEPATHY)){
    sendTo("You are not telepathic!\n\r");
    return FALSE;
  }

  if (isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL)) {
    sendTo("What a dumb master you have, charmed mobiles can't tell.\n\r");
    return FALSE;
  }
  half_chop(arg, name, message);

  if (!*name || !*message) {
    sendTo("Whom do you wish to telepath what??\n\r");
    return FALSE;
  } else if (!(vict = get_pc_world(this, name, EXACT_YES, INFRA_NO, visible))) {
    if (!(vict = get_pc_world(this, name, EXACT_NO, INFRA_NO, visible))) {
      if (!(vict = get_char_vis_world(this, name, NULL, EXACT_YES))) {
        if (!(vict = get_char_vis_world(this, name, NULL, EXACT_NO))) {
          sendTo("You fail to telepath to '%s'\n\r", name);
          return FALSE;
        }
      }
    }
  }
  if (isPlayerAction(PLR_GODNOSHOUT) && (vict->GetMaxLevel() <= MAX_MORT)) {
    sendTo("You have been sanctioned by the gods and can't telepath to them!!\n\r");
    return FALSE;
  }
  if (this == vict) {
    sendTo("You try to telepath yourself something.\n\r");
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

  if(!bSuccess(this, getSkillValue(SKILL_PSITELEPATHY), SKILL_PSITELEPATHY))
    drunkNum=20;
  else 
    drunkNum = getCond(DRUNK);

  string garbed;
  garbed = garble(message, drunkNum);

  rc = vict->triggerSpecialOnPerson(this, CMD_OBJ_TOLD_TO_PLAYER, garbed.c_str());
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete vict;
    vict = NULL;
  }
  if (IS_SET_DELETE(rc, DELETE_VICT)) 
    return DELETE_THIS;

  if (rc)
    return FALSE;

  mud_str_copy(capbuf, vict->pers(this), 256);  // Use Someone for tells (invis gods, etc)

  char garbedBuf[256];
  char nameBuf[256];
  if (vict->hasColor()) {
    if (hasColorStrings(NULL, capbuf, 2)) {
      if (IS_SET(vict->desc->plr_color, PLR_COLOR_MOBS)) {
        sprintf(nameBuf, "%s", colorString(vict, vict->desc, cap(capbuf), NULL, COLOR_MOBS, FALSE).c_str());
      } else {
        sprintf(nameBuf, "<p>%s<z>", colorString(vict, vict->desc, cap(capbuf), NULL, COLOR_NONE, FALSE).c_str());
      }
    } else {
      sprintf(nameBuf, "<p>%s<z>", cap(capbuf));
    }
  } else {
    sprintf(nameBuf, "%s", cap(capbuf));
  }

  sendTo(COLOR_COMM, "<G>You telepath %s<z>, \"%s\"\n\r", vict->getName(), colorString(this, desc, garbed.c_str(), NULL, COLOR_BASIC, FALSE).c_str());

  // we only color the string to the victim, so leave this AFTER
  // the stuff we send to the teller.
  convertStringColor("<c>", garbed);
  vict->sendTo(COLOR_COMM, "%s telepaths you, \"<c>%s<z>\"\n\r",
            nameBuf, garbed.c_str());

  Descriptor *d = vict->desc;
  if (d->m_bIsClient) {
    sprintf(garbedBuf, "<c>%s<z>", garbed.c_str());
    d->clientf("%d|%s|%s", CLIENT_TELL,
        colorString(vict, vict->desc, nameBuf, NULL, COLOR_NONE, FALSE).c_str(),
        colorString(vict, vict->desc, garbedBuf, NULL, COLOR_NONE, FALSE).c_str());
  }

  // set up last teller for reply's use
  // If it becomes a "someone tells you", ignore
  if (vict->desc && isPc() && vict->canSee(this, INFRA_YES))
    strcpy(vict->desc->last_teller, getName());

  if (desc && inGroup(*vict))
    desc->talkCount = time(0);

  if (vict->desc && (vict->isPlayerAction(PLR_AFK) || (IS_SET(vict->desc->autobits, AUTO_AFK) && (vict->getTimer() >= 5)))) 
    act("$N appears to be away from $S terminal at the moment.", TRUE, this, 0, vict, TO_CHAR);
 
  return FALSE;
}

int TBeing::doPSay(const char *arg){
  char buf[MAX_INPUT_LENGTH + 40];
  char garbed[256];
  *buf = '\0';
  TThing *tmp, *tmp2;
  TBeing *mob = NULL;
  int rc;
  char capbuf[256];
  char tmpbuf[256], nameBuf[256], garbedBuf[256];
  Descriptor *d;
  int drunkNum=0;

  if(!doesKnowSkill(SKILL_PSITELEPATHY)){
    sendTo("You are not telepathic!\n\r");
    return FALSE;
  }


  if (desc)
    desc->talkCount = time(0);

  for (; isspace(*arg); arg++);

  if (!*arg)
    sendTo("Yes, but WHAT do you want to say telepathically?\n\r");
  else {
    if(!bSuccess(this, getSkillValue(SKILL_PSITELEPATHY), SKILL_PSITELEPATHY))
      drunkNum=20;
    else 
      drunkNum=getCond(DRUNK);
    
    mud_str_copy(garbed, garble(arg, drunkNum).c_str(), 256);

    sendTo(COLOR_COMM, "<g>You think to the room, <z>\"%s%s\"\n\r", 
            colorString(this, desc, garbed, NULL, COLOR_BASIC, FALSE).c_str(), norm());
    // show everyone in room the say.
    for (tmp = roomp->getStuff(); tmp; tmp = tmp2) {
      tmp2 = tmp->nextThing;
          
      if (!(mob = dynamic_cast<TBeing *>(tmp)))
        continue;

      if (!(d = mob->desc) || mob == this)
        continue;

      mud_str_copy(capbuf, mob->pers(this), 256);
      cap(capbuf);
      sprintf(tmpbuf, "%s", colorString(mob, mob->desc, capbuf, NULL, COLOR_NONE, FALSE).c_str()); 

      if (mob->isPc()) {
        if (hasColorStrings(NULL, capbuf, 2)) {
          if (IS_SET(mob->desc->plr_color, PLR_COLOR_MOBS)) {
            sprintf(tmpbuf, "%s", colorString(mob, mob->desc, capbuf, NULL, COLOR_NONE, FALSE).c_str());
	    mob->sendTo(COLOR_COMM, "%s thinks, \"%s%s\"\n\r", tmpbuf, garbed, mob->norm());
            if (d->m_bIsClient) {
              sprintf(garbedBuf, "%s", 
                colorString(this, mob->desc, garbed, NULL, COLOR_NONE, FALSE).c_str());
              d->clientf("%d|%s|%s", CLIENT_SAY, tmpbuf, garbedBuf);
            }
          } else {
	    mob->sendTo(COLOR_COMM, "<c>%s thinks, <z>\"%s\"\n\r", tmpbuf, garbed);
            if (d->m_bIsClient) {
              sprintf(nameBuf, "<c>%s<z>", tmpbuf);
              sprintf(garbedBuf, "%s", 
                colorString(this, mob->desc, garbed, NULL, COLOR_NONE, FALSE).c_str());
              d->clientf("%d|%s|%s", CLIENT_SAY, 
                colorString(this, mob->desc, nameBuf, NULL, COLOR_NONE, FALSE).c_str(),
                garbedBuf);
            }
          }
        } else {
	  mob->sendTo(COLOR_COMM, "<c>%s thinks, <z>\"%s\"\n\r", tmpbuf, garbed);
          if (d->m_bIsClient) {
            sprintf(nameBuf, "<c>%s<z>", tmpbuf);
            sprintf(garbedBuf, "%s",
            colorString(this, mob->desc, garbed, NULL, COLOR_NONE, FALSE).c_str());
            d->clientf("%d|%s|%s", CLIENT_SAY,
                colorString(this, mob->desc, nameBuf, NULL, COLOR_NONE, FALSE).c_str(),
                garbedBuf);
          }
        }
      } else {
	mob->sendTo(COLOR_COMM, "%s thinks, \"%s\"\n\r", good_cap(getName()).c_str(), 
		    colorString(this, mob->desc, garbed, NULL, COLOR_COMM, FALSE).c_str());
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

      if (mob == this)
        continue;

      if (isPc() && !mob->isPc()) { 
        TMonster *tmons = dynamic_cast<TMonster *>(mob);
        tmons->aiSay(this, garbed);
        rc = tmons->checkResponses(this, 0, garbed, CMD_SAY);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tmons;
          tmons = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT)) 
          return DELETE_THIS;
      }
    }
  }
  return FALSE;
}

void TBeing::doPShout(const char *msg){
  Descriptor *i;
  char garbed[256];
  int drunkNum=0;
  
  if(!doesKnowSkill(SKILL_PSITELEPATHY)){
    sendTo("You are not telepathic!\n\r");
    return;
  }
  

  if (!*msg) {
    sendTo("What do you wish to broadcast to the world?\n\r");
    return;
  } else {
    if(!bSuccess(this, getSkillValue(SKILL_PSITELEPATHY), SKILL_PSITELEPATHY))
      drunkNum=20;
    else 
      drunkNum=getCond(DRUNK);
    
    mud_str_copy(garbed, garble(msg, drunkNum).c_str(), 256);


    sendTo(COLOR_SPELLS, "You telepathically send the message, \"%s<z>\"\n\r", garbed);
    for (i = descriptor_list; i; i = i->next) {
      if (i->character && (i->character != this) &&
	  !i->connected && !i->character->checkSoundproof() &&
	  (dynamic_cast<TMonster *>(i->character) ||
	   (!IS_SET(i->autobits, AUTO_NOSHOUT)) ||
	   !i->character->isPlayerAction(PLR_GODNOSHOUT))) {
	i->character->sendTo(COLOR_SPELLS, "Your mind is flooded with a telepathic message from %s.\n\r", getName());
	i->character->sendTo(COLOR_SPELLS, "The message is, \"%s%s\"\n\r", garbed, i->character->norm());
      }
    }
  }
  return;
}

void TBeing::doTelevision(const char *arg)
{
  int target;
  char buf1[128];
  TBeing *vict;
  bool visible = TRUE;

  if(!doesKnowSkill(SKILL_TELE_VISION)){
    sendTo("You have not yet mastered psionics well enough to do that.\n\r");
    return;
  }

  if (!*arg) {
    sendTo("Whom do you wish to television??\n\r");
    return;
  } else if (!(vict = get_pc_world(this, arg, EXACT_YES, INFRA_NO, visible))) {
    if (!(vict = get_pc_world(this, arg, EXACT_NO, INFRA_NO, visible))) {
      if (!(vict = get_char_vis_world(this, arg, NULL, EXACT_YES))) {
        if (!(vict = get_char_vis_world(this, arg, NULL, EXACT_NO))) {
          sendTo("You can't sense '%s' anywhere.\n\r", arg);
          return;
        }
      }
    }
  }


  target = vict->roomp->number;

  if (target == ROOM_NOCTURNAL_STORAGE ||
      target == ROOM_VOID ||
      target == ROOM_IMPERIA ||
      target == ROOM_HELL ||
      target == ROOM_STORAGE ||
      target == ROOM_POLY_STORAGE ||
      target == ROOM_CORPSE_STORAGE ||
      target == ROOM_Q_STORAGE ||
      target == ROOM_DONATION ||
      target == ROOM_DUMP) {
    nothingHappens(SILENT_YES);
    act("You can't seem to look there right now.",
        false, this, 0, 0, TO_CHAR);
    return ;
  }

  if (bSuccess(this, getSkillValue(SKILL_TELE_VISION), SKILL_TELE_VISION)) {
    sprintf(buf1, "You peer through the eyes of %s and see...",
	    vict->getName());
    act(buf1, FALSE, this, 0, 0, TO_CHAR);

    sprintf(buf1, "%d look", target);
    doAt(buf1, true);

    return;
  } else {
    sprintf(buf1, "You can't seem to get a handle on %s's mind.",
	    vict->getName());
    act(buf1, FALSE, this, 0, 0, TO_CHAR);

    return;
  }

  return;
}

void TBeing::doMindfocus(const char *){
  if (affectedBySpell(SKILL_MIND_FOCUS)) {
    sendTo("You are already focusing your mind.\n\r");
    return;
  }

  int bKnown=getSkillValue(SKILL_MIND_FOCUS);
  affectedData aff;

  if (bSuccess(this, bKnown, SKILL_MIND_FOCUS)){
    act("You begin to focus your mind on regenerating your psionic powers.", TRUE, this, NULL, NULL, TO_CHAR);

    aff.type      = SKILL_MIND_FOCUS;
    aff.level     = bKnown;
    aff.duration  = (bKnown / 10) * UPDATES_PER_MUDHOUR;
    aff.location  = APPLY_NONE;
    affectTo(&aff, -1);
  } else {
    act("You try to focus your mind, but you can't concentrate.",
	TRUE, this, NULL, NULL, TO_CHAR);
  }

  return;
}

TBeing *psiAttackChecks(TBeing *caster, spellNumT sk, const char *tString){
  char tTarget[256];
  TObj *tobj=NULL;
  TBeing *tVictim=NULL;

  if (caster->checkBusy(NULL))
    return NULL;

  if (!caster->doesKnowSkill(sk)) {
    caster->sendTo("You do not have the skill to use %s.\n\r", 
		   discArray[sk]->name);
    return NULL;
  }


  if (tString && *tString){
    only_argument(tString, tTarget);
    generic_find(tTarget, FIND_CHAR_ROOM, caster, &tVictim, &tobj);
  } else if (caster->fight()) {
    tVictim = caster->fight();
  }

  if(!tVictim){
    caster->sendTo("Who do you want to use %s on?\n\r", discArray[sk]->name);
    return NULL;
  }

  if (caster->checkPeaceful("You feel too peaceful to contemplate violence here.\n\r") 
      || tVictim->isImmortal() || tVictim->inGroup(*caster))
    return NULL;

  return tVictim;
}

void psiAttackFailMsg(TBeing *ch, TBeing *tVictim){
  act("You fail to breach $N's mind with your psionic powers.",
      FALSE, ch, NULL, tVictim, TO_CHAR);
  act("You feel a malevolent psionic power emanating from $n towards you, but it quickly dissipates.",
      TRUE, ch, NULL, tVictim, TO_VICT);
}

int TBeing::doPsiblast(const char *tString){
  // decreases int/wis/foc
  TBeing *tVictim=NULL;

  if(!(tVictim=psiAttackChecks(this, SKILL_PSI_BLAST, tString)))
    return FALSE;
  
  int bKnown=getSkillValue(SKILL_PSI_BLAST);
  int tDamage=0;

  if (bSuccess(this, bKnown, SKILL_PSI_BLAST)) {
    act("You send a blast of psionic power towards $N!",
        FALSE, this, NULL, tVictim, TO_CHAR);
    
    act("A look of shocked pain appears on $N's face.",
	TRUE, this, NULL, tVictim, TO_CHAR);
    act("$n sends a blast of psionic power into your mind.",
	TRUE, this, NULL, tVictim, TO_VICT);
    act("A look of shocked pain appears on $N's face as $n glares at $M.",
	TRUE, this, NULL, tVictim, TO_NOTVICT);

    if (!tVictim->affectedBySpell(SKILL_PSI_BLAST)) {
      affectedData aff;
      int count=0;

      // I do a success roll for each affect to mix things up a bit

      if (bSuccess(this, bKnown, SKILL_PSI_BLAST)) {
	aff.type      = SKILL_PSI_BLAST;
	aff.level     = bKnown;
	aff.duration  = (bKnown / 10) * UPDATES_PER_MUDHOUR;
	aff.location  = APPLY_INT;
	aff.modifier   = -(::number(bKnown/3, bKnown/2));
	tVictim->affectTo(&aff, -1);
	++count;
      }

      if (bSuccess(this, bKnown, SKILL_PSI_BLAST)) {
	aff.type      = SKILL_PSI_BLAST;
	aff.level     = bKnown;
	aff.duration  = (bKnown / 10) * UPDATES_PER_MUDHOUR;
	aff.location  = APPLY_WIS;
	aff.modifier   = -(::number(bKnown/3, bKnown/2));
	tVictim->affectTo(&aff, -1);
	++count;
      }

      if (bSuccess(this, bKnown, SKILL_PSI_BLAST)) {
	aff.type      = SKILL_PSI_BLAST;
	aff.level     = bKnown;
	aff.duration  = (bKnown / 10) * UPDATES_PER_MUDHOUR;
	aff.location  = APPLY_FOC;
	aff.modifier   = -(::number(bKnown/3, bKnown/2));
	tVictim->affectTo(&aff, -1);
	++count;
      }

      if(count){
	act("$N seems to have suffered a temporary disorientation.",
	    TRUE, this, NULL, tVictim, TO_CHAR);
	act("You seem to be suffering from a temporary disorientation.",
	    TRUE, this, NULL, tVictim, TO_VICT);
	act("$N seems to have suffered a temporary disorientation.",
	    TRUE, this, NULL, tVictim, TO_NOTVICT);
      }

    }

    tDamage = getSkillDam(tVictim, SKILL_PSI_BLAST,
                          getSkillLevel(SKILL_PSI_BLAST),
                          getAdvLearning(SKILL_PSI_BLAST));
  } else {
    psiAttackFailMsg(this, tVictim);
  }


  int rc = reconcileDamage(tVictim, tDamage, SKILL_CHI);

  addSkillLag(SKILL_PSI_BLAST, rc);

  if (IS_SET_ONLY(rc, DELETE_VICT)) {
    delete tVictim;
    tVictim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  

  return TRUE;
}


int TBeing::doMindthrust(const char *tString){
  TBeing *tVictim=NULL;

  if(!(tVictim=psiAttackChecks(this, SKILL_MIND_THRUST, tString)))
    return FALSE;
  
  int bKnown=getSkillValue(SKILL_MIND_THRUST);
  int tDamage=0;

  if (bSuccess(this, bKnown, SKILL_MIND_THRUST)) {
    act("You use your psionic powers to stab at $N's mind!",
        FALSE, this, NULL, tVictim, TO_CHAR);
    
    act("$N winces in pain.",
	TRUE, this, NULL, tVictim, TO_CHAR);
    act("$n squints at you, causing a sharp stabbing pain in your mind.",
	TRUE, this, NULL, tVictim, TO_VICT);
    act("$n squints at $N causing $M to wince suddenly.",
	TRUE, this, NULL, tVictim, TO_NOTVICT);
    tDamage = getSkillDam(tVictim, SKILL_MIND_THRUST,
                          getSkillLevel(SKILL_MIND_THRUST),
                          getAdvLearning(SKILL_MIND_THRUST));

    
    if(bSuccess(this, bKnown/4, SKILL_MIND_THRUST) && tVictim->spelltask)
      tVictim->addToDistracted(::number(1,2), FALSE);
  } else {
    psiAttackFailMsg(this, tVictim);
  }

  int rc = reconcileDamage(tVictim, tDamage, SKILL_CHI);
  addSkillLag(SKILL_MIND_THRUST, rc);

  if (IS_SET_ONLY(rc, DELETE_VICT)) {
    delete tVictim;
    tVictim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }

  return TRUE;
}

int TBeing::doPsycrush(const char *tString){
  // blindness and/or flee
  TBeing *tVictim=NULL;
  int doflee=0;

  if(!(tVictim=psiAttackChecks(this, SKILL_PSYCHIC_CRUSH, tString)))
    return FALSE;
  
  int bKnown=getSkillValue(SKILL_PSYCHIC_CRUSH);
  int tDamage=0;
  int level = getSkillLevel(SKILL_PSYCHIC_CRUSH);

  if (bSuccess(this, bKnown, SKILL_PSYCHIC_CRUSH)) {
    act("You reach out with your psionic powers and CRUSH $N's psyche!",
        FALSE, this, NULL, tVictim, TO_CHAR);

    if(bSuccess(this, bKnown/4, SKILL_PSYCHIC_CRUSH) && 
       !tVictim->affectedBySpell(SPELL_BLINDNESS) &&
       !tVictim->isAffected(AFF_TRUE_SIGHT) &&
       !isNotPowerful(tVictim, level, SPELL_BLINDNESS, SILENT_YES)){
				     
      act("$N's eyes open wide in shock.",
	  TRUE, this, NULL, tVictim, TO_CHAR);
      act("$n's psychic crush is too much for you to bear and your vision goes blank.",
	  TRUE, this, NULL, tVictim, TO_VICT);
      act("$N's eyes open wide in shock as $n glares at $m.",
	  TRUE, this, NULL, tVictim, TO_NOTVICT);

      // very short duration
      int duration = PULSE_COMBAT * 2;

      tVictim->rawBlind(level, duration, SAVE_NO);

    } else {
      act("$N winces in pain.",
	  TRUE, this, NULL, tVictim, TO_CHAR);
      act("$n crushes your psyche.",
	  TRUE, this, NULL, tVictim, TO_VICT);
      act("$N winces in pain as $n glares at $m.",
	  TRUE, this, NULL, tVictim, TO_NOTVICT);
    }

    if(bSuccess(this, bKnown, SKILL_PSYCHIC_CRUSH) &&
       !isNotPowerful(tVictim, level, SPELL_FEAR, SILENT_YES)){ // flee
      doflee=1;
    }

    
    tDamage = getSkillDam(tVictim, SKILL_PSYCHIC_CRUSH,
			  getSkillLevel(SKILL_PSYCHIC_CRUSH),
			  getAdvLearning(SKILL_PSYCHIC_CRUSH));
    
  } else {
    psiAttackFailMsg(this, tVictim);
  }

  int rc = reconcileDamage(tVictim, tDamage, SKILL_CHI);
  addSkillLag(SKILL_PSYCHIC_CRUSH, rc);

  if (IS_SET_ONLY(rc, DELETE_VICT)) {
    delete tVictim;
    tVictim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  
  if(doflee && tVictim){
      act("An overwhelming sense of panic causes you to flee.",
	  TRUE, this, NULL, tVictim, TO_VICT);
    tVictim->doFlee("");
  }


  return TRUE;
}

int TBeing::doKwave(const char *tString){
  // bash
  TBeing *tVictim=NULL;

  if(!(tVictim=psiAttackChecks(this, SKILL_KINETIC_WAVE, tString)))
    return FALSE;
  
  int bKnown=getSkillValue(SKILL_KINETIC_WAVE);
  int tDamage=0;

  if (bSuccess(this, bKnown, SKILL_KINETIC_WAVE)) {
    act("You set loose a wave of kinetic force at $N!",
        FALSE, this, NULL, tVictim, TO_CHAR);

    if(1){
      if (tVictim->riding) {
	act("You knock $N off $p.", 
	    FALSE, this, tVictim->riding, tVictim, TO_CHAR);
	act("$n knocks $N off $p.", 
	    FALSE, this, tVictim->riding, tVictim, TO_NOTVICT);
	act("$n knocks you off $p.", 
	    FALSE, this, tVictim->riding, tVictim, TO_VICT);
	tVictim->dismount(POSITION_STANDING);
      }


      act("$n sends $N sprawling with a kinetic force wave!",
	  FALSE, this, 0, tVictim, TO_NOTVICT);
      act("You send $N sprawling.", FALSE, this, 0, tVictim, TO_CHAR);
      act("You tumble as $n knocks you over with a kinetic wave.",
	  FALSE, this, 0, tVictim, TO_VICT, ANSI_BLUE);

      int rc = tVictim->crashLanding(POSITION_SITTING);
      if (IS_SET_ONLY(rc, DELETE_VICT)) {
	delete tVictim;
	tVictim = NULL;
	REM_DELETE(rc, DELETE_VICT);
      }

      float wt = combatRound(discArray[SKILL_KINETIC_WAVE]->lag);
      wt = (wt * 100.0 / getSkillDiffModifier(SKILL_KINETIC_WAVE));
      wt += 1;
      tVictim->addToWait((int) wt);
      
      if (tVictim->spelltask) 
	tVictim->addToDistracted(1, FALSE);
    } else {
      act("You pound $N with a kinetic wave.",
	  TRUE, this, NULL, tVictim, TO_CHAR);
      act("$n pounds you with a kinetic wave.",
	  TRUE, this, NULL, tVictim, TO_VICT);
      act("$N is pounded by a kinetic wave.",
	  TRUE, this, NULL, tVictim, TO_NOTVICT);
    }

    tDamage = getSkillDam(tVictim, SKILL_KINETIC_WAVE,
			  getSkillLevel(SKILL_KINETIC_WAVE),
                          getAdvLearning(SKILL_KINETIC_WAVE));
  } else {
    psiAttackFailMsg(this, tVictim);
  }

  int rc = reconcileDamage(tVictim, tDamage, SKILL_CHI);
  addSkillLag(SKILL_KINETIC_WAVE, rc);

  if (IS_SET_ONLY(rc, DELETE_VICT)) {
    delete tVictim;
    tVictim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }

  return TRUE;
}

