//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    dissect.cc : procedures related to dissecting
//
//    Copyright 1998, SneezyMUD Development Team
//    All Rights Reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_component.h"
#include "disc_adventuring.h"
#include "cmd_dissect.h"
#include "obj_base_corpse.h"

map<unsigned short int, dissectInfo>dissect_array;

int determineDissectionItem(TBaseCorpse *corpse, int *amount, char *msg, char *gl_msg, TBeing *ch)
{
  int num = -1;

  if (num == -1) {
    // switch based on generic race
    switch(corpse->getCorpseRace()) {
      case RACE_PHOENIX:
        if (::number(0,1)) {
          num = COMP_FLAMING_SWORD;
          *amount = 50;
          sprintf(msg, "You tear $p off of $N.");
          sprintf(gl_msg, "$n tears $p off of $N.");
        } else {
          num = OBJ_PHOENIX_FEATHER;
          *amount = 50;
          sprintf(msg, "You pluck $p from $N's wing.");
          sprintf(gl_msg, "$n plucks $p from $N's wing.");
        }
        break;
      case RACE_DEER:
        num = OBJ_VENISON;
        sprintf(msg, "You carve $p from $N.");
        sprintf(gl_msg, "$n carves $p from $N.");
        *amount = 50;
        break;
      default:
        break;
    }  
  }
  if (num == -1) {
    // switch based on vnum
    // this switch should ONLY be for VERY special cases now
    // put generic stuff in the dissect file and let the LOWs maintain it
    switch(corpse->getCorpseVnum()) {
      case MOB_TIGER_SHARK:
	if(ch && ch->hasQuestBit(TOG_STARTED_MONK_BLUE)){
	  num = OBJ_MONK_QUEST_DOG_COLLAR;
	  *amount = 100;
	  sprintf(msg, "You reach deep into $N's maw and pull out $p.");
	  sprintf(gl_msg, "$n reaches deep into $N's maw and pulls out $p.");
	}
        break;
      default:
        break;
    }  
  }
  if (num == -1) {
    map<unsigned short int, dissectInfo>::const_iterator CT;
    CT = dissect_array.find(corpse->getCorpseVnum());
    if (CT != dissect_array.end()) {
      num = CT->second.loadItem;
      *amount = CT->second.amount;
      sprintf(msg, CT->second.message_to_self.c_str());
      sprintf(gl_msg, CT->second.message_to_others.c_str());
    }
  }

  return num;
}

int TBeing::doDissect(sstring argument)
{
  sstring namebuf;
  TObj *obj;
  int rc;

  one_argument(argument, namebuf);

  if (namebuf.empty()) {
    sendTo("What do you want to dissect?\n\r");
    return FALSE;
  }

  if (!hasHands() || bothHandsHurt()) {
    sendTo("You must have hands, and at least one must work, to do this.\n\r");
    return FALSE;
  }

  if (isDumbAnimal()) {
    sendTo("You are a dumb animal.  You don't understand dissection.\n\r");
    return FALSE;
  }

  if (getPosition() == POSITION_MOUNTED) {
    sendTo("You'd need to dismount first.\n\r");
    return FALSE;
  }

  if (!(obj = dynamic_cast<TObj *> (searchLinkedListVis(this, namebuf, roomp->getStuff())))) {
    if (!(obj = dynamic_cast<TObj *> (searchLinkedListVis(this, namebuf, getStuff())))) {
      sendTo(fmt("There doesn't seem to be any '%s' here to dissect.\n\r") % namebuf);
      return FALSE;
    }
  }

  if (!doesKnowSkill(SKILL_DISSECT)) {
    sendTo("You know nothing about dissection.\n\r");
    return FALSE;
  }

  // this is mostly here to stop auto-loot kicking in while 'zerking
  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo(COLOR_BASIC, "<r>You are way too blood crazed at the moment to be dissecting stuff.<1>\n\r");
    return FALSE;
  }

  rc = dissect(this, obj);
  if (rc)
    addSkillLag(SKILL_DISSECT, rc);
  if (IS_SET_DELETE(rc, DELETE_ITEM)) {
    delete obj;
    obj = NULL;
    REM_DELETE(rc, DELETE_ITEM);
  }
  return rc;
}

int dissect(TBeing * caster, TObj * corpse)
{
  return corpse->dissectMe(caster);
}

int TObj::dissectMe(TBeing *caster)
{
  act("$p: You can only dissect corpses.", 
          FALSE, caster, this, 0, TO_CHAR);
  return FALSE;
}

void readDissectionFile()
{
  dissectInfo di;
  FILE *fp;
  unsigned int mobNum;
  char buf[256];
  int res;

  const char * const dissect_file = "objdata/dissect";

  fp = fopen(dissect_file, "r");
  if (!fp) {
    vlogf(LOG_FILE, fmt("Unable to open '%s' for reading") %  dissect_file);
    return;
  }

  while (!feof(fp)) {
    res = fscanf(fp, " %d ", &mobNum); 
    if (res != 1)
      continue;
    //    res = fscanf(fp, " %d %d %d ", &di.loadItem, &di.amount, &di.count); 
    res = fscanf(fp, " %d %d ", &di.loadItem, &di.amount); 
    if (res != 2)
      continue;
    if (!fgets(buf, 256, fp))
      continue;
    di.message_to_self = buf;
    if (!fgets(buf, 256, fp))
      continue;
    di.message_to_others = buf;

    size_t end_whitespace = di.message_to_self.find_last_of("\n");
    if (end_whitespace != sstring::npos)
      di.message_to_self.erase(end_whitespace);
    end_whitespace = di.message_to_others.find_last_of("\n");
    if (end_whitespace != sstring::npos)
      di.message_to_others.erase(end_whitespace);

    dissect_array[mobNum] = di;
  }
  fclose(fp);
}
