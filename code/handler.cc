//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include <cstdarg>

extern "C" {
#include <unistd.h>
}

#include "stdsneezy.h"
#include "combat.h"
#include "disease.h"
#include "range.h"
#include "connect.h"
#include "shop.h"
#include "obj_spellbag.h"
#include "obj_bag.h"
#include "obj_table.h"
#include "obj_tool.h"
#include "obj_base_weapon.h"
#include "obj_base_clothing.h"


// fname will look for the first non-alpha character
// I added - and ' as valid fname chars since this allows us to do better things
// with items and mobs in the name field.  - Bat 6-30-97
const sstring fname(const char *namelist)
{
  char holder[60];
  register char *point;

  for (point = holder; isalpha(*namelist) || (*namelist == '-') || (*namelist == '\''); namelist++, point++)
    *point = *namelist;

  *point = '\0';

  return (holder);
}

// split up "str", using delimiters "sep" and place resulting sstrings in "argv"
int split_string(const sstring &str, const sstring &sep, vector<sstring> &argv)
{
  unsigned int pos=0, last=0;

  while((pos=str.find_first_of(sep,last)) != sstring::npos){
    argv.push_back(str.substr(last,pos-last));
    last=pos+1;
  }

  argv.push_back(str.substr(last,str.length()-last));

  return argv.size();
}

bool isname(const sstring &str, const sstring &namelist)
{
  vector <sstring> argv, xargv;
  unsigned int i, j;

  if (namelist.empty())
    return false;

  split_string(str, "- \t\n\r,", argv);
  split_string(namelist, "- \t\n\r,", xargv);

  for(i=0;i < argv.size();i++) {
    for(j=0;j < xargv.size();j++) {
      if (!xargv[j].empty() && is_abbrev(argv[i], xargv[j])) {
        xargv[j] = "";
        break;
      }
    }
    if (j >= xargv.size())
      return FALSE;
  }
  return TRUE;
}

bool is_exact_spellname(const sstring &str, const sstring &namelist)
{
  vector <sstring> argv, xargv;
  unsigned int i, j;

  split_string(str, "- \t\n\r,", argv);
  split_string(namelist, "- \t\n\r,", xargv);

  if(!is_abbrev(argv[0], xargv[0]))
    return FALSE;

  for (i = 0; i < argv.size(); i++) {
    for (j = 0; j < xargv.size(); j++) {
      if(!xargv[j].empty() && argv[i].lower() == xargv[j].lower()){
        xargv[j] = "";
        break;
      }
    }
    if (j >= xargv.size())
      return FALSE;
  }
  return TRUE;
}

bool is_exact_name(const sstring &str, const sstring &namelist)
{
  vector <sstring> argv, xargv;
  unsigned int i, j;

  split_string(str, "- \t\n\r,", argv);
  split_string(namelist, "- \t\n\r,", xargv);

  for (i = 0; i < argv.size(); i++) {
    for (j = 0; j < xargv.size(); j++) {
      if(argv[i].lower() == xargv[j].lower()){
        xargv[j] = "";
        break;
      }
    }
    if (j >= xargv.size())
      return FALSE;
  }
  return TRUE;
}

void TBeing::affectChange(unsigned long original, silentTypeT silent)
{
  unsigned long current = specials.affectedBy;
  int rc;

  if (silent)
    return;

  if (!IS_SET(current, AFF_BLIND) && IS_SET(original, AFF_BLIND)) {
    sendTo("Phew, you can see again, thankfully.\n\r");
  }
  if (IS_SET(current, AFF_BLIND) && !IS_SET(original, AFF_BLIND)) {
    sendTo("Ack, you can't see a damn thing!  Who turned out the lights?\n\r");
  }
  if (!IS_SET(current, AFF_INVISIBLE) && IS_SET(original, AFF_INVISIBLE)) {
    sendTo("You are no longer invisible.\n\r");
    act("$n appears suddenly!", FALSE, this,0,0,TO_ROOM);
  }
  if (IS_SET(current, AFF_INVISIBLE) && !IS_SET(original, AFF_INVISIBLE)) {
    sendTo("You vanish.\n\r");
    act("$n vanishes!", FALSE, this,0,0,TO_ROOM);
  }
  if (!IS_SET(current, AFF_SHADOW_WALK) && IS_SET(original, AFF_SHADOW_WALK)) {
    sendTo("You no longer walk among the shadows.\n\r");
    act("$n suddenly starts making noise.", FALSE, this,0,0,TO_ROOM);
  }
  if (IS_SET(current, AFF_SHADOW_WALK) && !IS_SET(original, AFF_SHADOW_WALK)) {
    sendTo("You become transparent.\n\r");
    act("$n becomes transparent!", FALSE, this,0,0,TO_ROOM);
  }
  if (!IS_SET(current, AFF_SENSE_LIFE) && IS_SET(original, AFF_SENSE_LIFE)) {
    sendTo("You feel cutoff from your awareness of the life around you.\n\r");
  }
  if (IS_SET(current, AFF_SENSE_LIFE) && !IS_SET(original, AFF_SENSE_LIFE)) {
    sendTo("You suddenly feel more aware of the life around you.\n\r");
  }
  if (!IS_SET(current, AFF_LEVITATING) && IS_SET(original, AFF_LEVITATING)) {
    sendTo("You stop levitating!\n\r");
    act("$n stops levitating!", FALSE, this,0,0,TO_ROOM);
  }
  if (IS_SET(current, AFF_LEVITATING) && !IS_SET(original, AFF_LEVITATING)) {
    sendTo("You float upwards a few inches.  You're levitating!\n\r");
    act("$n begins to levitate!", FALSE, this,0,0,TO_ROOM);
  }
  if (!IS_SET(current, AFF_SANCTUARY) && IS_SET(original, AFF_SANCTUARY)) {
    sendTo("You are no longer surrounded by a brilliant, white light.\n\r");
    act("A brilliant, white light disappears from around $n!", FALSE, this,0,0,TO_ROOM);
  }
  if (IS_SET(current, AFF_SANCTUARY) && !IS_SET(original, AFF_SANCTUARY)) {
    sendTo("You are now surrounded by a brilliant, white light.\n\r");
    act("A brilliant, white light surrounds $n!", FALSE, this,0,0,TO_ROOM);
  }
  if (!IS_SET(current, AFF_FLYING) && IS_SET(original, AFF_FLYING)) {
    if (roomp && roomp->isFlyingSector()) {
      sendTo("You lose your ability to fly. Lucky for you the magic in this room prevents your fall.\n\r");
    } else {
      sendTo("You lose your ability to fly.\n\r");
      if (roomp && isFlying()) {
        // roomp is needed since this is sometimes called by dead critters
        rc = crashLanding(POSITION_SITTING);
        if (IS_SET_DELETE(rc, DELETE_THIS))
        // I don't think this can happen, so won't bother to do work to pass the return if it does
          vlogf(LOG_BUG, "fix problem in affectChange and crashLanding");
      }
    }
  }
  if (IS_SET(current, AFF_FLYING) && !IS_SET(original, AFF_FLYING)) {
    sendTo("You feel light as a feather.  You bet you could fly.\n\r");
//        act("$n starts flying about!", FALSE, this,0,0,TO_ROOM);
  }
  if (!IS_SET(current, AFF_PARALYSIS) && IS_SET(original, AFF_PARALYSIS)) {
    if (getPosition() > POSITION_DEAD) {
      sendTo("You can move again.\n\r");
      act("$n starts moving again!", FALSE, this,0,0,TO_ROOM);
    }
  }
  if (IS_SET(current, AFF_PARALYSIS) && !IS_SET(original, AFF_PARALYSIS)) {
    sendTo("You freeze up.  You can't seem to move at all.\n\r");
    act("$n suddenly stops moving!", FALSE, this,0,0,TO_ROOM);
  }
}

void TBeing::addSkillApply(spellNumT skillNum, sbyte amt)
{
  skillApplyData *temp = NULL;

  for (temp = skillApplys; temp; temp = temp->nextApply) {
    if (temp->skillNum == skillNum) {
      temp->amount += amt;
      temp->numApplys += 1;
      return;
    }
  } 

  // gets here only if didn't find one
  temp = new skillApplyData;
  temp->skillNum = skillNum;
  temp->amount = amt;
  temp->numApplys = 1;

  temp->nextApply = skillApplys;
  skillApplys = temp;
}

void TBeing::remSkillApply(spellNumT skillNum, sbyte amt)
{
  skillApplyData *temp = NULL, *temp2 = NULL;
 
  if (!skillApplys) {
    vlogf(LOG_BUG, fmt("Somehow, remSkillApply was called with no skillApplys (%s) (%d)") %  getName() % skillNum);
    return;
  }
  for (temp = skillApplys; temp; temp = temp2) {
    temp2 = temp->nextApply;
    if (temp->skillNum == skillNum) {
      temp->numApplys -= 1;
      temp->amount -= amt;

      if (temp->numApplys <= 0) {
        if (temp == skillApplys) {
          // apply is first in list
          skillApplys = temp->nextApply;
        } else {
          skillApplyData *prev;
          for (prev = skillApplys; prev && prev->nextApply != temp; prev = prev->nextApply);
          if (!prev) {
            vlogf(LOG_BUG, "Could't locate previous skill_apply while removing");
            return;
          }
          // adjust linked list
          prev->nextApply = temp->nextApply;
        }

        // sanity
        if (temp->amount)
          vlogf(LOG_BUG, fmt("Somehow, remSkillApply had a last apply that didnt bring affect to 0 (%s) (%d)") %  getName() % skillNum);

        delete temp;
      }
      return;
    }
  }

  // !temp....
  vlogf(LOG_BUG, fmt("Somehow, skillApplys had no skill to remove in remSkillApply (%s) (%d)") %  getName() % skillNum); 
}

void TBeing::affectModify(applyTypeT loc, long mod, long mod2, unsigned long bitv, bool add, silentTypeT silent)
{
  int tmpInt;
  CDiscipline *cd;

  // special case, just treat it as a bitvector
  if (loc == APPLY_SPELL_EFFECT)
    bitv = mod;

  if (loc != APPLY_IMMUNITY && loc != APPLY_DISCIPLINE && loc != APPLY_SPELL) {
    if (add)
      SET_BIT(specials.affectedBy, bitv);
    else {
      REMOVE_BIT(specials.affectedBy, bitv);
      mod = -mod;
    }
  }

  switch(loc) {
    case APPLY_NONE:
    case APPLY_NOISE:
    case APPLY_LIGHT:
    case APPLY_SEX:
      return;
    case APPLY_SPELL:
      if (!discArray[mod]) {
        vlogf(LOG_BUG, fmt("ILLEGAL skill (%d) on being %s.  Ignoring.") %  mod % getName());
        return;
      }
      if (add) {
        addSkillApply(spellNumT(mod), mod2);
      } else {
        remSkillApply(spellNumT(mod),mod2);
      }
      return;
    case APPLY_DISCIPLINE: {
      discNumT dnt = mapFileToDisc(mod);
      if ((dnt == DISC_NONE) || !discNames[dnt].disc_num) {
        vlogf(LOG_BUG, fmt("ILLEGAL Discipline (%d) on being %s.  Ignoring.") %  mod % getName());
        return;
      }
      if (!(cd = getDiscipline(dnt))) {
        return;
      }      
      if (add) {
        tmpInt = cd->getLearnedness() + mod2;
        if (tmpInt > MAX_DISC_LEARNEDNESS)
          tmpInt = MAX_DISC_LEARNEDNESS;
        cd->setLearnedness(tmpInt);
      } else {
        tmpInt = cd->getLearnedness() - mod2;;
        if (tmpInt < 0)
          tmpInt = 0;
        if (cd->getNatLearnedness() == MAX_DISC_LEARNEDNESS) 
          tmpInt = MAX_DISC_LEARNEDNESS;
        cd->setLearnedness(tmpInt);
      }
      return;
      }
    case APPLY_IMMUNITY:
      if (mod < MIN_IMMUNE || mod >= MAX_IMMUNES) {
        vlogf(LOG_BUG, "Bad immunity value");
        return;
      }
      if (add) 
        addToImmunity(immuneTypeT(mod), mod2);
      else 
        addToImmunity(immuneTypeT(mod), -mod2);
      return;
    case APPLY_SPELL_EFFECT:
      // we handled this above by setting mod to bitv
      return;
    case APPLY_STR:
      addToStat(STAT_CURRENT, STAT_STR, mod);
      checkForStr(silent);
      return;
    case APPLY_BRA:
      addToStat(STAT_CURRENT, STAT_BRA, mod);
      return;
    case APPLY_DEX:
      addToStat(STAT_CURRENT, STAT_DEX, mod);
      return;
    case APPLY_AGI:
      addToStat(STAT_CURRENT, STAT_AGI, mod);
      return;
    case APPLY_CON:
      addToStat(STAT_CURRENT, STAT_CON, mod);
      return;
    case APPLY_INT:
      addToStat(STAT_CURRENT, STAT_INT, mod);
      return;
    case APPLY_WIS:
      addToStat(STAT_CURRENT, STAT_WIS, mod);
      return;
    case APPLY_FOC:
      addToStat(STAT_CURRENT, STAT_FOC, mod);
      return;
    case APPLY_KAR:
      addToStat(STAT_CURRENT, STAT_KAR, mod);
      return;
    case APPLY_SPE:
      addToStat(STAT_CURRENT, STAT_SPE, mod);
      return;
    case APPLY_PER:
      addToStat(STAT_CURRENT, STAT_PER, mod);
      return;
    case APPLY_CHA:
      addToStat(STAT_CURRENT, STAT_CHA, mod);
      return;
    case APPLY_AGE:
      {
        // changing the age will affect stats
        // natural stats are recalculated on the fly, so no probs
        // current stats, however, must be updated to reflect.
        // figure out how much natural changes by, and change current to match
        statTypeT whichStat;
        for (whichStat = MIN_STAT; whichStat < MAX_STATS; whichStat++) {
          int oldVal = getStat(STAT_NATURAL, whichStat);
          age_mod += mod;
          int newVal = getStat(STAT_NATURAL, whichStat);

          setStat(STAT_CURRENT, whichStat, getStat(STAT_CURRENT, whichStat) + newVal - oldVal);
          age_mod -= mod;
        }

        // change the age for real
        age_mod += mod;
        return;
      }
    case APPLY_CHAR_WEIGHT:
      setWeight(getWeight() + mod);
      return;
    case APPLY_CHAR_HEIGHT:
      //    setHeight(getHeight() + mod);
      return;
    case APPLY_MANA:
      points.maxMana += mod;
      return;
    case APPLY_CURRENT_HIT:
      points.hit += mod;
      return;
    case APPLY_HIT:
      points.maxHit += mod;
      return;
    case APPLY_MOVE:
      points.maxMove += mod;
      if (!silent && getMove() > moveLimit())
        setMove(moveLimit());
      return;
    case APPLY_ARMOR:
      //      points.armor += mod;
      return;
    case APPLY_SPELL_HITROLL:
      // we don't do a *10 here, assume anywhere its set is setting properly
      setSpellHitroll(getSpellHitroll() + mod);
      return;
    case APPLY_HITROLL:
      setHitroll(getHitroll() + 10*mod);
      return;
    case APPLY_DAMROLL:
      setDamroll(getDamroll() + mod);
      return;
    case APPLY_HITNDAM:
      setHitroll(getHitroll() + 10*mod);
      setDamroll(getDamroll() + mod);
      return;
    case APPLY_CAN_BE_SEEN:
      canBeSeen += mod;
      return;
    case APPLY_VISION:
      visionBonus += mod;
      return;
    case APPLY_PROTECTION:
      addToProtection(mod);
      return;
    case MAX_APPLY_TYPES:
      break;
  }
  vlogf(LOG_BUG, fmt("Unknown apply adjust attempt (::affectModify()) %s loc: %d.") %  getName() % loc);
  vlogf(LOG_BUG, "how'd this happen");
}

bool affectShouldApply(const TObj *obj, wearSlotT pos)
{
  // this second check is to see if they are holding an item normally worn
  // elsewhere.  While we want to allow them to hold it, its effects should
  // only kick in if its worn in the right manner.
  if (obj && ((pos != HOLD_RIGHT && pos != HOLD_LEFT) || obj->canWear(ITEM_HOLD))) {
    return true;
  }
  return false;
}

// This updates a character by subtracting everything he is affected by 
// restoring original abilities, and then affecting all again          
void TBeing::affectTotal()
{
  affectedData *af;
  int j, value = 0;
  TThing *t;
  TObj *o = NULL;
  CDiscipline *cd = NULL;
  bool discAdd = FALSE, skillAdd = FALSE;
  bool skillAfAdd = FALSE, discAfAdd = FALSE;
  
  wearSlotT i;
  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if ((t = equipment[i]) && (o = dynamic_cast<TObj *>(t))) {
      if (o->usedAsPaired() && (i == WEAR_ARM_L || i == WEAR_WRIST_L ||
          i == WEAR_HAND_L || i == WEAR_FINGER_L || i == WEAR_LEGS_L ||
          i == WEAR_FOOT_L || i == WEAR_EX_LEG_L || i == WEAR_EX_FOOT_L))
        continue;
      if (!affectShouldApply(o, i))
        continue;
      for (j = 0; j < MAX_OBJ_AFFECT; j++) {
        if (o->affected[j].location == APPLY_SPELL)
          skillAdd = TRUE;
        if (o->affected[j].location == APPLY_DISCIPLINE)
          discAdd = TRUE;
        affectModify(o->affected[j].location,
                     o->affected[j].modifier,
                     o->affected[j].modifier2,
                     o->obj_flags.bitvector, FALSE, SILENT_YES);
      }
    }
  }
  for (af = affected; af; af = af->next) {
    if (af->location == APPLY_SPELL)
      skillAfAdd = TRUE;
    if (af->location == APPLY_DISCIPLINE)
      discAfAdd = TRUE;
    affectModify(af->location, af->modifier, af->modifier2, af->bitvector, FALSE, SILENT_YES);
  }
//  convertAbilities();
  // if I set myself, don't fix me...
  if (!isImmortal()) {
    statTypeT ij;
    for (ij = MIN_STAT; ij < MAX_STATS_USED; ij++)
      setStat(STAT_CURRENT, ij, getStat(STAT_NATURAL, ij));
  }

// discs and skills next

  if (!discs) {
// if no discs goto end of this section--do not do disc/skills 
#if 0
    // this appears to happen.  I think during polymorphing
    if (isPc() && (GetMaxLevel() > 1)) 
      vlogf(LOG_BUG,fmt("PC in affectTotal without discs %s") %  getName());
#endif
  } else {

// FIRST DISCIPLINE LEARNING

    discNumT dnt;
    for (dnt = MIN_DISC; dnt < MAX_DISCS; dnt++) {
      if (!(cd = getDiscipline(dnt)))
        continue;

      if (isImmortal()) {
        cd->setNatLearnedness(MAX_DISC_LEARNEDNESS);
        cd->setLearnedness(MAX_DISC_LEARNEDNESS);
        continue;
      }

// set learning equal to nat learning

      cd->setLearnedness(cd->getNatLearnedness());

// add in equipment-- needed here so we can get right skills
//  in case wearing disc equipment

      if (discAdd) {
        for (i = MIN_WEAR; i < MAX_WEAR; i++) {
          if ((t = equipment[i]) && (o = dynamic_cast<TObj *>(t))) {
            if (o->usedAsPaired() && (i == WEAR_ARM_L || i == WEAR_WRIST_L ||
                 i == WEAR_HAND_L || i == WEAR_FINGER_L || i == WEAR_LEGS_L ||
                 i == WEAR_FOOT_L || i == WEAR_EX_LEG_L || i == WEAR_EX_FOOT_L))
              continue;
            if (!affectShouldApply(o, i))
              continue;
            for (j = 0; j < MAX_OBJ_AFFECT; j++) {
              if (o->affected[j].location != APPLY_DISCIPLINE)
                continue;
              if (mapFileToDisc(o->affected[j].modifier) != dnt)
                continue;
              affectModify(o->affected[j].location,
                     o->affected[j].modifier,
                     o->affected[j].modifier2,
                     o->obj_flags.bitvector, TRUE, SILENT_YES);
            }
          }
        }
      }
      if (discAfAdd) {
        for (af = affected; af; af = af->next) {
          if (af->location != APPLY_DISCIPLINE) 
            continue;
          if (mapFileToDisc(af->modifier) != dnt)
            continue;
          affectModify(af->location, af->modifier, af->modifier2, af->bitvector, TRUE, SILENT_YES);
        }
      }
      // even after all that if the disc is unlearned, make it so
      if (cd->getNatLearnedness() <= 0)
        cd->setLearnedness(0);
    }
// NOW FOR SKILLS 

    spellNumT num;
    for (num = MIN_SPELL; num < MAX_SKILL; num++) {
      if (hideThisSpell(num))
        continue;

      discNumT das = getDisciplineNumber(num, FALSE);
      if (das == DISC_NONE) {
        vlogf(LOG_BUG, fmt("Bad disc for skill %d in affectTotal()") %  num);
        continue;
      }
      if (isImmortal()) {
        setNatSkillValue(num, MAX_SKILL_LEARNEDNESS);
        setSkillValue(num, MAX_SKILL_LEARNEDNESS);
        continue;
      }
      
      if (!(cd = getDiscipline(das)))
        continue;

      if (!cd->getNatLearnedness() || (cd->getNatLearnedness() < discArray[num]->start)) {
        // use NatLearning above, if never learned never learned
        // may have to reevaluate but for now it looks right 122297 cos

        setNatSkillValue(num, SKILL_MIN);
        setSkillValue(num, SKILL_MIN);
        continue;
      }
      if ((desc || isPc()) && discArray[num]->toggle && 
                !hasQuestBit(discArray[num]->toggle)) {
        setNatSkillValue(num, SKILL_MIN);
        setSkillValue(num, SKILL_MIN);
        continue;
      } 

// First skills that are not learn by doing
      if (discArray[num]->startLearnDo < 0) {
// Set natLearning
// remember we  store natLearning so be careful with it

        value = discArray[num]->learn * 
                  (cd->getNatLearnedness() - discArray[num]->start + 1);
        value = max(value, 1);
        value = max(value, discArray[num]->learn);
        value = min(value, (int) MAX_SKILL_LEARNEDNESS);
        setNatSkillValue(num, value);

// set Current learning with no learn by doing numbers
        if (cd->getLearnedness() == cd->getNatLearnedness()) {
          setSkillValue(num, value);
        } else {
          if (cd->getLearnedness() < discArray[num]->start) {
            setSkillValue(num, 0);
          } else {
            value = getMaxSkillValue(num); // uses Learnedness not NatLear
            value = max(value, 1);
            value = max(value, discArray[num]->learn);
            value = min(value, (int) MAX_SKILL_LEARNEDNESS);
            setSkillValue(num, value);
          }
        }
      } else {  // learn by doing
// set natural learning, be very careful since permanint numbers
        value = discArray[num]->learn *
                  (cd->getNatLearnedness() - discArray[num]->start + 1);
        value = min(value, (int) discArray[num]->startLearnDo);
        value = max((int) getRawNatSkillValue(num), value);
        value = max(value, 1);
        value = min(value, (int) MAX_SKILL_LEARNEDNESS);

        setNatSkillValue(num, value);

        if (cd->getLearnedness() == cd->getNatLearnedness()) {
          value = getRawNatSkillValue(num);
          setSkillValue(num, value);
        } else {
          value = getMaxSkillValue(num);  // uses disc Learn not disc NatLearn
          value = min((int) getRawNatSkillValue(num), value);
          value = max(value, 1);
          value = min(value, (int) MAX_SKILL_LEARNEDNESS);
          if (cd->getLearnedness() <= cd->getNatLearnedness()) {
             setSkillValue(num, value);
          } else { 
            if (discArray[num]->startLearnDo > value) {
              value = getMaxSkillValue(num);
              value = min(value, (int) discArray[num]->startLearnDo);
              setSkillValue(num, value);
            } else {
              setSkillValue(num, value);
            }
          }
        }
      }
#if 0
      if (skillAdd) {
        for (i = MIN_WEAR; i < MAX_WEAR; i++) {
          if ((t = equipment[i]) && (o = dynamic_cast<TObj *>(t))) {
            if (o->usedAsPaired())
              continue;
            for (j = 0; j < MAX_OBJ_AFFECT; j++) {
              if (!(o->affected[j].location == APPLY_SPELL))
                continue;
              if (!(o->affected[j].modifier == num))
                continue;
              affectModify(o->affected[j].location,
                           o->affected[j].modifier,
                           o->affected[j].modifier2,
                           o->obj_flags.bitvector, TRUE, SILENT_YES);
            }
          }
        }
      }

      if (skillAfAdd) {
        for (af = affected; af; af = af->next) {
          if (!(af->location == APPLY_SPELL))
            continue;
          if (!(af->modifier == num))
            continue;
          affectModify(af->location, af->modifier, af->modifier2, af->bitvector, TRUE, SILENT_YES);
        }
      }
#endif
    }
  }

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if ((t = equipment[i]) && (o = dynamic_cast<TObj *>(t))) {
      if (o->usedAsPaired() && (i == WEAR_ARM_L || i == WEAR_WRIST_L ||
          i == WEAR_HAND_L || i == WEAR_FINGER_L || i == WEAR_LEGS_L ||
          i == WEAR_FOOT_L || i == WEAR_EX_LEG_L || i == WEAR_EX_FOOT_L))
        continue;
      if (!affectShouldApply(o, i))
        continue;
      for (j = 0; j < MAX_OBJ_AFFECT; j++) {
#if 1
// take out discipline since done above
        if (o->affected[j].location == APPLY_DISCIPLINE)
          continue;

#else
// take out skills/disc since done above
        if ((o->affected[j].location == APPLY_SPELL) ||
                 (o->affected[j].location == APPLY_DISCIPLINE))
          continue;
#endif
        affectModify(o->affected[j].location,
                     o->affected[j].modifier,
                     o->affected[j].modifier2,
                     o->obj_flags.bitvector, TRUE, SILENT_YES);
      }
    }
  }

  for (af = affected; af; af = af->next) {
#if 1
    if (af->location == APPLY_DISCIPLINE)
      continue;
#else
    if ((af->location == APPLY_SPELL) || (af->location == APPLY_DISCIPLINE))
      continue;
#endif
    affectModify(af->location, af->modifier, af->modifier2, af->bitvector, TRUE, SILENT_YES);
  }

  /* Make certain values are between 5..250, not < 5 and not > 30! */
  int minrange, maxrange;
//Speef - limits Everyone including mobs to 190.  Removing.
#if 0 
  if (isImmortal() && isPc()) {
    minrange=0;
    maxrange=250;
  } else {
    minrange=30;
    maxrange=190;
  }
#endif
  minrange = 30;
  maxrange = 250;
  setStat(STAT_CURRENT, STAT_STR, 
          min(max(minrange, getStat(STAT_CURRENT, STAT_STR)), maxrange));
  setStat(STAT_CURRENT, STAT_BRA, 
          min(max(minrange, getStat(STAT_CURRENT, STAT_BRA)), maxrange));
  setStat(STAT_CURRENT, STAT_DEX, 
          min(max(minrange, getStat(STAT_CURRENT, STAT_DEX)), maxrange));
  setStat(STAT_CURRENT, STAT_AGI, 
          min(max(minrange, getStat(STAT_CURRENT, STAT_AGI)), maxrange));
  setStat(STAT_CURRENT, STAT_CON, 
          min(max(minrange, getStat(STAT_CURRENT, STAT_CON)), maxrange));

  setStat(STAT_CURRENT, STAT_INT, 
          min(max(minrange, getStat(STAT_CURRENT, STAT_INT)), maxrange));
  setStat(STAT_CURRENT, STAT_WIS, 
          min(max(minrange, getStat(STAT_CURRENT, STAT_WIS)), maxrange));
  setStat(STAT_CURRENT, STAT_FOC, 
          min(max(minrange, getStat(STAT_CURRENT, STAT_FOC)), maxrange));

  setStat(STAT_CURRENT, STAT_CHA, 
          min(max(minrange, getStat(STAT_CURRENT, STAT_CHA)), maxrange));
  setStat(STAT_CURRENT, STAT_SPE, 
          min(max(minrange, getStat(STAT_CURRENT, STAT_SPE)), maxrange));
  setStat(STAT_CURRENT, STAT_KAR, 
          min(max(minrange, getStat(STAT_CURRENT, STAT_KAR)), maxrange));
  setStat(STAT_CURRENT, STAT_PER, 
          min(max(minrange, getStat(STAT_CURRENT, STAT_PER)), maxrange));
}

void TBeing::affectTo(affectedData *af, int renew, silentTypeT silent)
{
  mud_assert(af->type != TYPE_UNDEFINED, "applying undefined affect to %s", getName());

  affectedData *a;
  int origamt = specials.affectedBy;

  //mud_assert(af->duration != 0, "affectTo() with 0 duration affect");
  a = new affectedData(*af);

  a->next = affected;
  if (renew == -1) 
    a->renew = -1;
  else if (renew) 
    a->renew = renew;
  else 
    a->renew = a->duration / 2;
  
  affected = a;

  affectModify(a->location, a->modifier, a->modifier2, a->bitvector, TRUE, silent);
  affectTotal();

  affectChange(origamt, silent);
}

void TBeing::affectRemove(affectedData *af, silentTypeT silent)
{
  affectedData *af2;
  int origamt = specials.affectedBy;

  if (!affected) {
    vlogf(LOG_BUG, fmt("Affect removed from char (%s) without affect") %  getName());
    vlogf(LOG_BUG, fmt("Location : %d, Modifier %d, Bitvector %d") %  af->location % af->modifier % af->bitvector);
    return;
  } else
    affectModify(af->location, af->modifier, af->modifier2, af->bitvector, FALSE, silent);

  if (affected == af)
    affected = af->next;
  else {
    for (af2 = affected; (af2->next) && (af2->next != af); af2 = af2->next);
    if (af2->next != af) {
      vlogf(LOG_BUG, "Could not locate affected_type in affected. (handler.c, affectRemove)");
      return;
    }
    af2->next = af->next;       /* skip the af element */
  }
  delete af;
  af = NULL;
  affectTotal();

  affectChange(origamt, silent);
}


// Call affect_remove with every spell of spelltype "skill"
void TBeing::affectFrom(spellNumT skill)
{
  affectedData *hjp, *next_aff;

  for (hjp = affected; hjp; hjp = next_aff) {
    next_aff = hjp->next;
    if (hjp->type == skill)
      affectRemove(hjp);
  }
}

void TBeing::removeSkillAttempt(spellNumT skill)
{
  affectedData *hjp = NULL, *next_aff = NULL;

  for (hjp = affected; hjp; hjp = next_aff) {
    next_aff = hjp->next;
    if (hjp->type == AFFECT_SKILL_ATTEMPT) {
      if (hjp->modifier == skill)
        affectRemove(hjp);
    }
  }
}

int TBeing::checkForSkillAttempt(spellNumT skill)
{
  affectedData *hjp;

  if (!affected)
    return FALSE;

  for (hjp = affected; hjp; hjp = hjp->next) {
    if (hjp->type == AFFECT_SKILL_ATTEMPT) {
      if (hjp->modifier == skill) 
        return TRUE;
    }
  }
  return FALSE;
}


bool TBeing::affectedBySpell(spellNumT skill) const
{
  affectedData *hjp;

  for (hjp = affected; hjp; hjp = hjp->next) {
    if (hjp->type == skill)
      return TRUE;
  }
  return FALSE;
}

// used with polymorph transfers of affects
// transfer spells and 'oddball' stuff (drunk, disease, pkill...)
int TBeing::polyAffectJoin(TBeing * recipient)
{
  affectedData *affp;
  for (affp = affected; affp; affp = affp->next) {
    if (!((affp->type > MIN_SPELL && affp->type < MAX_SKILL) ||
          (affp->type > FIRST_ODDBALL_AFFECT && 
           affp->type < LAST_ODDBALL_AFFECT) ||
           affp->type == AFFECT_SKILL_ATTEMPT)) 
        continue;
    recipient->affectJoin(recipient, affp, AVG_DUR_NO, AVG_EFF_YES, FALSE);
  }
  return TRUE;
}

// avg_dur FALSE : duration will be cumulative
// avg_dur TRUE : the average of the existing and new affect dur wil be used
// avg_mod FALSE : modifier will be cumulative
// avg_mod TRUE : the average of the existing and new affect mod wil be used
// avg_mod DOES take into account mod2 for skills/immunes
int TBeing::affectJoin(TBeing * caster, affectedData *af, avgDurT avg_dur, avgEffT avg_mod, bool text)
{
  affectedData *hjp;
  bool found = FALSE;
  int renew = 0;

  // Remove any 'bit' that the player may already have set.
  // This prevents a number of casts == lose ability.
  // This is better than tracking down each one and dealing with it.
  if (af->bitvector && isAffected(af->bitvector) &&
      !affectedBySpell(af->type))
    af->bitvector = 0;

  for (hjp = affected; !found && hjp; hjp = hjp->next) {
    if ((hjp->type == af->type) &&
        (hjp->location == af->location)) {
      if (af->location == APPLY_IMMUNITY ||
          af->location == APPLY_SPELL) {
        if (af->modifier != hjp->modifier) 
          continue;
      }
      if (!hjp->canBeRenewed()) {
        if (text) {
          if (this == caster || !caster) 
            sendTo("You can't increase the duration of that effect any further.\n\r"); 
          else {
            caster->sendTo("You can't increase the duration of that effect any further.\n\r"); 
            act("$N can't increase the duration of that effect any further.",
                 FALSE, this, 0, caster, TO_CHAR);
          }
        }
        return FALSE;
      }
      // Don't do this if renew is already set!
      if ((renew = hjp->renew) <= 0)
        renew = max(af->duration, hjp->duration) / 2;

      af->duration += hjp->duration;

      if (avg_dur)
        af->duration /= 2;

      if (af->location == APPLY_IMMUNITY ||
          af->location == APPLY_SPELL) {
        af->modifier2 += hjp->modifier2;
        if (avg_mod)
          af->modifier2 /= 2;
      } else {
        af->modifier += hjp->modifier;
        if (avg_mod)
          af->modifier /= 2;
      }
      affectRemove(hjp, SILENT_YES);
      // hjp is invalid here
      affectTo(af, renew, SILENT_YES);
      found = TRUE;
      return TRUE;
    }
  }
  if (!found) {
    affectTo(af);
    return TRUE;
  }
  return FALSE;
}

void thing_to_room(TThing *ch, int room)
{
  TRoom *rp;

  if (!(rp = real_roomp(room))) {
    vlogf(LOG_BUG, fmt("thing_to_room() called with bogus room: %d") %  room);
    room = 0;
    rp = real_roomp(room);
  }
  
  *rp += *ch;

}

void TBeing::equipChar(TThing *obj, wearSlotT pos, silentTypeT silent)
{
  // silent is used to determine if the effects of equipping should be shown
  // when they equipped.
  // for things where you temporarily remove an item from someone and then
  // put it back on, it should be set to FALSE
  int j;

  mud_assert(pos >= MIN_WEAR && pos < MAX_WEAR, "pos in equip_char(%s %s %d) was out of range!!", getName(), obj->name, pos);

  if (equipment[pos]) {
    vlogf(LOG_BUG, fmt("equip_char(%s %s %d) called with position already equipped!") %  getName() % obj->name % pos);
    return;
  }
  if (obj->parent) {
    vlogf(LOG_BUG, "EQUIP: Obj is in something when equip.");
    obj->parent = NULL;
  }
  if (obj->in_room != ROOM_NOWHERE) {
    vlogf(LOG_BUG, "EQUIP: Obj is in_room when equip.");
    obj->in_room = ROOM_NOWHERE;
    return;
  }
  if (obj->stuckIn) {
    vlogf(LOG_BUG, "EQUIP: Obj is stuck in someone when equip.");
    obj->stuckIn = NULL;
  }
  TBaseClothing *tbc = dynamic_cast<TBaseClothing *>(obj);
  if (tbc && tbc->isShield() && (pos == getPrimaryHold())) {
    sendTo("You can't equip a shield in your primary hand.\n\r");
    if (!heldInSecHand()) {
      pos = getSecondaryHold();
      sendTo("So you put it in your other hand.\n\r");
    } else {
      *this += *obj;
      return;
    }

  }

#if 0
  if (!canUseLimb(pos)) {
    sendTo(fmt("Your limb is busted and %s drops off.\n\r") %obj->shortDescr);
    act("$n drops $s $o.", TRUE, this, obj, 0, TO_ROOM);
    *roomp += *obj;
    return;
  }
#endif

  if (pos == HOLD_RIGHT) {
    if (!canUseLimb(WEAR_HAND_R)) {
      if (roomp) {
        sendTo(COLOR_BASIC, fmt("Your hand is damaged and you drop %s.\n\r") %obj->shortDescr);
        *roomp += *obj;
      } else 
        *this += *obj;
      
      return;
    }
  } else if (pos == HOLD_LEFT) {
    if (!canUseLimb(WEAR_HAND_L)) {
      if (roomp) {
        sendTo(COLOR_BASIC, fmt("Your hand is damaged and you drop %s.\n\r") %obj->shortDescr);
        *roomp += *obj;
      } else 
        *this += *obj;
      
      return;
    }
  }


  obj->equippedBy = this;
  obj->eq_pos = pos;  
  equipment.wear(obj, pos);

  int origamt = specials.affectedBy;

  TObj *to = dynamic_cast<TObj *>(obj);
  if (to && affectShouldApply(to, pos)) {
    for (j = 0; j < MAX_OBJ_AFFECT; j++) {
      affectModify(to->affected[j].location,
                   to->affected[j].modifier,
                   to->affected[j].modifier2,
                   to->obj_flags.bitvector, TRUE, silent);
    }
  }

  affectChange(origamt, silent);

  addToLight(obj->getLight());
  if (roomp && (in_room >= 0))
    roomp->addToLight(obj->getLight());
}

// *res should be -1 if this dies
TThing *TBeing::pulloutObj(wearSlotT numx, bool safe, int *res)
{
  int dam;
  TThing *o;
  
  *res = 0;

  if (!(o = getStuckIn(numx))) {
    vlogf(LOG_BUG, fmt("pulloutObj() called with no stuck in object for pos %d on char %s!") %  numx % getName());
    return NULL;
  }
  setStuckIn(numx, NULL);
  o->equippedBy = NULL;
  o->stuckIn = NULL;
  o->eq_pos = WEAR_NOWHERE;
  o->eq_stuck = WEAR_NOWHERE;
  if (o->spec) checkSpec(this, CMD_ARROW_RIPPED, "", NULL);

  TBaseWeapon *tbw = dynamic_cast<TBaseWeapon *>(o); 
  if (tbw && !safe) {

    dam = (int) (1.0 * tbw->baseDamage());
    if (reconcileDamage(this, dam,tbw->getWtype()) == -1) {
      *res = -1;
      return o;
    } else {
      if (dice(1, 100) < tbw->getCurSharp() && !isImmune(IMMUNE_BLEED)) {
        char buf[256];
        sprintf(buf, "$n's %s starts bleeding as $p is taken out of it.", 
                 describeBodySlot(numx).c_str());
        act(buf, TRUE, this, tbw, NULL, TO_ROOM);
        sprintf(buf, "Your %s starts bleeding as $p is taken out of it.",
                 describeBodySlot(numx).c_str());
        act(buf, FALSE, this, tbw, NULL, TO_CHAR);
        rawBleed(numx, 5 * tbw->getCurSharp(), SILENT_YES, CHECK_IMMUNITY_NO);
      }
    }
  }
  return o;
}

void TTool::unequipMe(TBeing *ch)
{
  if (getToolType() == TOOL_GARROTTE) {
    ch->diseaseFrom(DISEASE_GARROTTE);
  }
}

TThing *TBeing::unequip(wearSlotT pos)
{
  int j;
  TThing *o;

  if ((pos < MIN_WEAR) || (pos >= MAX_WEAR))
    return NULL;
 
  if (!equipment[pos])
    return NULL;

  o = equipment.remove(pos);

  if (o->parent || o->riding || (o->in_room != ROOM_NOWHERE))
    vlogf(LOG_BUG, "Item was two places(or more) in unequip()");

  o->equippedBy = NULL;
  o->stuckIn = NULL;
  o->eq_pos = WEAR_NOWHERE;
  o->eq_stuck = WEAR_NOWHERE;

  int origamt = specials.affectedBy;
  TObj *to = dynamic_cast<TObj *>(o);
  if (to &&
     ((pos != HOLD_RIGHT && pos != HOLD_LEFT) || to->canWear(ITEM_HOLD))) {
    for (j = 0; j < MAX_OBJ_AFFECT; j++) {
      affectModify(to->affected[j].location,
                   to->affected[j].modifier,
                   to->affected[j].modifier2,
                   to->obj_flags.bitvector, FALSE, SILENT_NO);
    }
  } 

  affectTotal();
  affectChange(origamt, SILENT_NO);

  if (getMana() >= manaLimit())
    setMana(manaLimit());
  if (getHit() >= hitLimit())
    setHit(hitLimit());

  addToLight(-o->getLight());
  if (roomp)
    roomp->addToLight(-o->getLight());

  o->unequipMe(this);

  return (o);
}

TThing *unequip_char_for_save(TBeing *ch, wearSlotT pos)
{
  int j;
  TThing *o;

  if ((pos < MIN_WEAR) || (pos >= MAX_WEAR))
    return NULL;

  if (!ch->equipment[pos])
    return NULL;

  o = ch->equipment.remove(pos);

  if (o->parent || o->riding || (o->in_room != ROOM_NOWHERE))
    vlogf(LOG_BUG, "Item was two places(or more) in unequip()");

  o->equippedBy = NULL;
  o->stuckIn = NULL;
  o->eq_pos = WEAR_NOWHERE;
  o->eq_stuck = WEAR_NOWHERE;

//  int origamt = ch->specials.affectedBy;
  TObj *to = dynamic_cast<TObj *>(o);
  if (to &&
     ((pos != HOLD_RIGHT && pos != HOLD_LEFT) || to->canWear(ITEM_HOLD))) {
    for (j = 0; j < MAX_OBJ_AFFECT; j++) {
      ch->affectModify(to->affected[j].location,
                   to->affected[j].modifier,
                   to->affected[j].modifier2,
                   to->obj_flags.bitvector, FALSE, SILENT_YES);
    } 
  } 

  ch->affectTotal();

//  ch->affectChange(origamt, SILENT_YES);

  ch->addToLight(-o->getLight());
  if (ch->roomp)
    ch->roomp->addToLight(-o->getLight());

  return (o);
}

int get_number(char **name)
{
  int i;
  char *ppos, numx[MAX_INPUT_LENGTH] = "";

  if ((ppos = (char *) strchr(*name, '.')) && ppos[1]) {
    *ppos++ = '\0';
    strcpy(numx, *name);
    strcpy(*name, ppos);

    for (i = 0; *(numx + i); i++) {
      if (*(numx + i) == ' ')
        continue;
      if (!isdigit(*(numx + i))) {
        return (0);
      } else {
        break;
      }
    }
    return (convertTo<int>(numx));
  }
  return 1;
}

TThing *get_thing_in_equip(TBeing *ch, const char *arg, equipmentData equipment, wearSlotT *j, bool vis, int *count)
{
  int num;
  int numx;
  char tmpname[MAX_INPUT_LENGTH], *tmp;

  if (!*arg)
    return NULL;

  strcpy(tmpname, arg);
  tmp = tmpname;

  if (!(numx = get_number(&tmp)))
    return (0);

  num = (count ? *count : 0);

  for ((*j) = MIN_WEAR; (*j) < MAX_WEAR; (*j)++) {
    TThing *tt = equipment[(*j)];
    if (!tt)
      continue;
    if (!vis || ch->canSee(tt)) {
      if (isname(tmp, tt->name)) {
        TObj *tobj = dynamic_cast<TObj *>(tt);
        if (tobj && tobj->isPaired()) {
          if (ch->isRightHanded()) {
            if (((*j) == WEAR_LEGS_L) || ((*j) == HOLD_LEFT))
              continue;
          } else {
            if (((*j) == WEAR_LEGS_L) || ((*j) == HOLD_RIGHT))
            continue;
          }
        }
        num++;
        if (num == numx)
          return tt;
      }
    }
  }
  if (count)
    *count = num;
  return NULL;
}

TThing *get_thing_stuck_in_vis(TBeing *ch, const char *arg, wearSlotT *j, int *count, TBeing *v)
{
  int num;
  int numx;
  char tmpname[MAX_INPUT_LENGTH], *tmp;
  TThing *t;

  if (!*arg)
    return NULL;

  strcpy(tmpname, arg);
  tmp = tmpname;

  if (!(numx = get_number(&tmp)))
    return (0);

  num = (count ? *count : 0);

  for (*j = MIN_WEAR; *j < MAX_WEAR; (*j)++) {
    if ((t = v->getStuckIn(*j))) {
      if (ch->canSee(t)) {
        if (isname(tmp, t->name)) {
          num++;
          if (num == numx)
            return (t);
        }
      }
    }
  }
  if (count)
    *count = num;
  return NULL;
}

TThing *searchLinkedList(const char * name, TThing *list, thingTypeT type)
{
  const sstring tmps = name;
  return searchLinkedList(tmps, list, type);
}

TThing *searchLinkedList(const sstring & name, TThing *list, thingTypeT type)
{
  TThing *i, *t;
  int j, numx;
  char tmpname[MAX_INPUT_LENGTH], *tmp;

  strcpy(tmpname, name.c_str());
  tmp = tmpname;

  if (!(numx = get_number(&tmp)))
    return (0);
 
  for (t = list, j = 1; t && (j <= numx); t = t->nextThing) {
    if (isname(tmp, t->name)) {
      if (type == TYPETHING || 
          ((type == TYPEBEING) && dynamic_cast<TBeing *>(t)) ||
          ((type == TYPEOBJ) && dynamic_cast<TObj *>(t)) ||
          ((type == TYPEPC) && dynamic_cast<TPerson *>(t)) ||
          ((type == TYPEMOB) && dynamic_cast<TMonster *>(t))) {
        if (j == numx)
          return t;
        j++;
      }
    }
    if (dynamic_cast<TTable *>(t) && t->rider) {
      for (i = t->rider; i; i = i->nextRider) {
        if (isname(tmp, i->name)) {
          if (type == TYPETHING || 
              ((type == TYPEBEING) && dynamic_cast<TBeing *>(i)) ||
              ((type == TYPEOBJ) && dynamic_cast<TObj *>(i)) ||
              ((type == TYPEPC) && dynamic_cast<TPerson *>(i)) ||
              ((type == TYPEMOB) && dynamic_cast<TMonster *>(i))) {
            if (j == numx)
              return i;
            j++;
          }
        }
      }
    }
  }
  return NULL;
}

// Search a given list for an object, and return a pointer to that object 
TThing *get_thing_on_list(const char *name, TThing *list)
{
  TThing *i;
  int j, numx;
  char tmpname[MAX_INPUT_LENGTH], *tmp;

  strcpy(tmpname, name);
  tmp = tmpname;

  if (!(numx = get_number(&tmp)))
    return (0);

  for (i = list, j = 1; i && (j <= numx); i = i->nextRider)
    if (isname(tmp, i->name)) {
      if (j == numx)
        return (i);
      j++;
    }
  return (0);
}

// Search a given list for an object number, and return a ptr to that obj 
TThing *get_thing_on_list_num(int num, TThing *list)
{
  TThing *i;

  for (i = list; i; i = i->nextRider)
    if (i->number == num)
      return (i);

  return NULL;
}

// Search a given list for an object number, and return a ptr to that obj 
TThing *get_thing_in_list_num(int num, TThing *list)
{
  TThing *i;

  for (i = list; i; i = i->nextThing) {
    if (i->number == num)
      return i;
  }
  return NULL;
}

// this gets used by shop code to get the "num"th thing in the "list"
TObj *get_num_obj_in_list(TBeing *ch, int num, TThing *list, int shop_nr)
{
  vector<TObj *>cond_obj_vec(0);
  vector<int>cond_tot_vec(0);

  TThing *i;
  bool found = false;
  unsigned int k;

  if (num <= 0)
    return NULL;

  for (i = list; i; i = i->nextThing) {
    TObj *to = dynamic_cast<TObj *>(i);
    if (to && ch->canSee(to)) {
      if ((to->getValue() >= 1) &&
          !to->isObjStat(ITEM_NEWBIE) &&
          shop_index[shop_nr].willBuy(to)) {
        found = FALSE;
        for (k = 0; (k < cond_obj_vec.size() && !found); k++) {
          if (cond_obj_vec.size() > 0) {
            if (to->isShopSimilar(cond_obj_vec[k])) {
              cond_tot_vec[k] += 1;
              found = TRUE;
            }
          }
        }
        if (!found) {
          cond_obj_vec.push_back(to);
          cond_tot_vec.push_back(1);
        }
      }
    }
  }                             // for loop
  if (cond_obj_vec.size()) {
    if (((unsigned int) num - 1) < cond_obj_vec.size())
      return (cond_obj_vec[(num-1)]);
  }
  return NULL;
}

// search the entire world for an object, and return a pointer  
TObj *get_obj(const char *name, exactTypeT exact)
{
  int j, numx;
  char tmpname[MAX_INPUT_LENGTH], *tmp;

  strcpy(tmpname, name);
  tmp = tmpname;
  if (!(numx = get_number(&tmp)))
    return (0);

  j=1;
  for(TObjIter iter=object_list.begin();
      iter!=object_list.end() && (j <= numx); ++iter){
    if ((exact && is_exact_name(tmp, (*iter)->name)) ||
        (!exact && isname(tmp, (*iter)->name))) {
      if (j == numx)
        return (*iter);
      j++;
    }
  }
  return (0);
}


// search the entire world for an object number, and return a pointer  
TObj *get_obj_num(int nr)
{
  for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
    if ((*iter) && (*iter)->number == nr)
      return (*iter);
  }
  return (0);
}

// search a room for a char, and return a pointer if found.. 
TBeing *get_char_room(const sstring &name, int room, int *count)
{
  TThing *i;
  int j, numx;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  strcpy(tmpname, name.c_str());
  tmp = tmpname;
  if (!(numx = get_number(&tmp)))
    return (0);

  j = (count ? *count : 0);

  for (i = real_roomp(room)->getStuff(); i && (j <= numx); i = i->nextThing) {
    TBeing *tbt = dynamic_cast<TBeing *>(i);
    if (tbt && isname(tmp, tbt->name)) {
      j++;
      if (j == numx)
        return tbt;
    }
  }
  if (count)
    *count = j;
  return NULL;
}

// search all over the world for a char, and return a pointer if found 
TBeing *get_char(const char *name, exactTypeT exact)
{
  TBeing *i;
  int j, numx;
  char tmpname[MAX_INPUT_LENGTH], *tmp;

  if (!*name)
    return NULL;

  strcpy(tmpname, name);
  tmp = tmpname;
  if (!(numx = get_number(&tmp)))
    return (0);

  for (i = character_list, j = 1; i && (j <= numx); i = i->next) {
    if (i->name) {
      if ((exact && is_exact_name(tmp, i->name)) ||
          (!exact && isname(tmp, i->name))) {
        if (j == numx)
          return (i);
        j++;
      }
    }
  }
  return (0);
}


// search all over the world for a char num, and return a pointer if found
TBeing *get_char_num(int nr)
{
  TBeing *i;

  for (i = character_list; i; i = i->next)
    if (i->number == nr)
      return (i);

  return (0);
}

void TThing::newOwner(TThing *ch)
{
  if (getStuff())
    stuff->newOwner(ch);
  if (nextThing)
    nextThing->newOwner(ch);

  parent = ch;
}

void TObj::update(int use)
{
  if (obj_flags.decay_time > 0) {
    obj_flags.decay_time -= use;
  }

  if (getStuff())
    getStuff()->update(use);

  if (nextThing) {
    if (nextThing != this)
      nextThing->update(use);
  }
} 

void TBeing::updateCharObjects()
{
  int i;

  for (i = MIN_WEAR; i < MAX_WEAR; i++)
    if (equipment[i])
      equipment[i]->update(1);

  if (getStuff())
    getStuff()->update(1);
}

void extract_edit_char(TMonster *ch)
{                          
  if (ch->number > -1)         
    mob_index[ch->getMobIndex()].addToNumber(-1);

  mobCount--;

  // stick me back in the list so I can be safely delete
  ch->next = character_list;
  character_list = ch;
  thing_to_room(ch, ROOM_VOID);

  delete ch;
  ch = NULL;
}



// Here follows high-level versions of some earlier routines, ie functions
// which incorporate the actual player-data.                              

TBeing *get_char_room_vis(const TBeing *ch, const sstring &name, int *count, exactTypeT exact, infraTypeT infra)
{
  TThing *i;
  TBeing *mob = NULL;
  int j, numx;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;
  sstring tStName("");

  if (!ch) {
    vlogf(LOG_BUG, "NULL ch in get_char_room_vis");
    return NULL;
  }
  if (name.empty() || !ch->roomp)
    return NULL;

  strcpy(tmpname, name.c_str());
  tmp = tmpname;

  if (!(numx = get_number(&tmp)))
    return NULL;

  j = (count ? *count : 0);
  for (i = ch->roomp->getStuff(); i && (j <= numx); i = i->nextThing) {
    mob = dynamic_cast<TBeing *>(i);
    if (!mob)
      continue;

    if ((!exact && isname(tmp, mob->name)) ||
        (exact && is_exact_name(tmp, mob->name))) {

      if (ch->canSee(mob, infra) || (mob == ch->riding)) {
        j++;
        if (j == numx)
          return (mob);
      }
    }

    tStName = "blobs";
    tStName += mob->getMyRace()->getSingularName();

#if 1
    if (isname(tmp, tStName)) {
#else
    if (is_abbrev(tmp, "blobs")) {
#endif
      if ((mob != ch) && !ch->canSee(mob) && ch->canSee(mob, INFRA_YES)) {
        j++;
        if (j == numx)
          return mob;
      }
    }
  }
  if (count)
    *count = j;
  return NULL;
}

// Here follows high-level versions of some earlier routines, ie functions
// which incorporate the actual player-data.

// if we return non-null, count holds the range
// otherwise, count will hold the number of matches (for shoot at 3.xx)
// max_dist is the max-distance we will look at
TBeing *get_char_vis_direction(const TBeing *ch, char *name, dirTypeT dir, unsigned int max_dist, bool here, unsigned int *count)
{
  TThing *t;
  unsigned int numx;
  int rm;
  unsigned int j = 0;
  unsigned int range = 0;
  char tmpname[256];
  char *tmp;
  TRoom *rp;

  if (!name || !*name)
    return NULL;

  strcpy(tmpname, name);
  tmp = tmpname;
  if (!(numx = (unsigned int) get_number(&tmp)))
    return NULL;

  if ((rm = ch->in_room) < 0)
    return NULL;

  rp = ch->roomp;
  if (count)
    j = *count;
  else
    j = 0;

  while (range <= max_dist) {
    if ((range == 0 && here) || range > 0) {
      for (t = rp->getStuff(); t && (j <= numx); t = t->nextThing) {
        TBeing *tbt = dynamic_cast<TBeing *>(t);
        if (tbt && isname(tmp, tbt->name) && 
            // use same func as scan.  technically might need a canSeeInfra too
            can_see_char_other_room(ch, tbt, rp)) {
          j++;
          if (j == numx) {
            if (count)
              *count = range;
            return tbt;
          }
        }
      }
    }

    if (!clearpath(rm, dir))
      break;

    range++;
    if (!(rm = rp->dir_option[dir]->to_room)) {
      vlogf(LOG_BUG, "Problem (1) in get_char_vis_direction");
      break;
    }
    if (!(rp = real_roomp(rm))) {
      vlogf(LOG_BUG, "Problem (2) in get_char_vis_direction");
      break;
    }
  }
  if (count)
    *count = j;
  return NULL;
}

TBeing *get_pc_world(const TBeing *ch, const sstring &name, exactTypeT exact, infraTypeT infra, bool visible)
{
  TBeing *i;

  if (name.empty())
    return NULL;

  for (i = character_list; i; i = i->next) {
    if (i->isPc()) {
      if ((!exact && isname(name, i->name)) || (exact && is_exact_name(name, i->name))) {
        if (visible) {
          if (ch->canSee(i, infra))
            return i;
        } else
          return i; 
      }
    }
  }
  return NULL;
}

// get a character from anywhere in the world, doesn't care much about
// being in the same room... 
TBeing *get_char_vis_world(const TBeing *ch, const sstring &name, int *count, exactTypeT exact, infraTypeT infra)
{
  TBeing *i;
  int j, numx;
  char tmpname[MAX_INPUT_LENGTH], *tmp;

  if (name.empty())
    return NULL;

  strcpy(tmpname, name.c_str());
  tmp = tmpname;
  if (!(numx = get_number(&tmp)))
    return (0);

  j = count ? *count : 1;

  for (i = character_list; i && (j <= numx); i = i->next) {
    if (!i->name) {
      vlogf(LOG_BUG, "Something with NULL i->name in get_char_vis_world()");
      continue;
    }
    if ((!exact && isname(tmp, i->name)) ||
        (exact && is_exact_name(tmp, i->name))) {
      if (ch->canSee(i, infra)) {
        if (j == numx)
          return (i);
        j++;
      }
    }
  }
  if (count)
    *count = j;

  return 0;
}

TBeing *get_char_vis(const TBeing *ch, const sstring &name, int *count, infraTypeT infra)
{
  TBeing *i;

  // check locally, then globally
  // look for exact name match before abbreviation
  // potential problem is finds local abbreviation, before global exact
  // but this is probably what we want to do

  if ((i = get_char_room_vis(ch, name, count, EXACT_YES, infra)))
    return (i);

  if ((i = get_char_room_vis(ch, name, count, EXACT_NO, infra)))
    return (i);

  if ((i = get_char_vis_world(ch, name, count, EXACT_YES, infra)))
    return i;

  return get_char_vis_world(ch, name, count, EXACT_NO, infra);
}

TThing *get_thing_in_list_getable(TBeing *ch, const char *name, TThing *list)
{
  TThing *o;

  for (o = list; o; o = o->nextThing) {
    if (!ch->canSee(o))
      continue;
    if (((o->thingHolding() == ch) &&
         ((ch->getCarriedVolume() + (o->getTotalVolume() - o->getReducedVolume(NULL))) <= ch->carryVolumeLimit())) ||
        ((o->thingHolding() != ch) &&
         ((ch->getCarriedVolume() + o->getTotalVolume()) <= ch->carryVolumeLimit()))) {
      if ((o->thingHolding() == ch) ||
          // o->weight <= ch weight limit
          (compareWeights(o->getTotalWeight(TRUE),
                 ch->carryWeightLimit() - ch->getCarriedWeight()) != -1)) {
        TObj *tobj = dynamic_cast<TObj *>(o);
        if (tobj) {
          if (ch->isImmortal() || 
               (tobj->canWear(ITEM_TAKE) && !tobj->isObjStat(ITEM_PROTOTYPE))) {
            if (!*name || isname(name, tobj->name))
              return(tobj);
          }
        }
#if 0
        } else {
          if (!*name || isname(name, o->name)) {
            if (ch->canGet(o, SILENT_NO));
              return(o);
          }
        }
#endif
      }
    }
  }
  return NULL;
}

TThing *get_thing_on_list_getable(TBeing *ch, const char *name, TThing *list)
{
  TThing *o, *next_o;

  for (o = list; o; o = next_o) {
    next_o = o->nextRider;
    if (ch->canSee(o)) {
      if (((o->thingHolding() == ch) &&
           ((ch->getCarriedVolume() + (o->getTotalVolume() - o->getReducedVolume(NULL))) <= ch->carryVolumeLimit())) ||
          ((o->thingHolding() != ch) &&
           ((ch->getCarriedVolume() + o->getTotalVolume()) <= ch->carryVolumeLimit()))) {
        if ((o->thingHolding() == ch) ||
            // o->weight <= ch weight limit
            (compareWeights(o->getTotalWeight(TRUE),
                   ch->carryWeightLimit() - ch->getCarriedWeight()) != -1)) {
          TObj *tobj = dynamic_cast<TObj *>(o);
          if (tobj) {
            if (ch->isImmortal() ||
              (tobj->canWear(ITEM_TAKE) && !tobj->isObjStat(ITEM_PROTOTYPE))) {
              if (!*name || isname(name, tobj->name))
                return tobj;
            }
          } else {
          }
#if 0
            if (!*name || isname(name, o->name))
              return(o);
          }
#endif
        }
      }
    }
  }
  return NULL;
}

TThing *searchLinkedListVis(const TBeing *ch, sstring name, TThing *list, int *count, thingTypeT type)
{
  TThing *i, *t;
  int j, numx;
  char tmpname[MAX_INPUT_LENGTH], *tmp;

  strcpy(tmpname, name.c_str());
  tmp = tmpname;

  if (!(numx = get_number(&tmp)))
    return NULL;

  j = (count ? *count : 0);

  for (t = list; t && (j < numx); t = t->nextThing) {
    if (t->name && isname(tmp, t->name)) {
      if (ch->canSee(t)) {
        if (type == TYPETHING || 
              ((type == TYPEBEING) && dynamic_cast<TBeing *>(t)) ||
              ((type == TYPEOBJ) && dynamic_cast<TObj *>(t)) ||
              ((type == TYPEPC) && dynamic_cast<TPerson *>(t)) ||
              ((type == TYPEMOB) && dynamic_cast<TMonster *>(t))) {
          j++;
          if (j == numx)
            return t;
        }
      }
    }
    if (dynamic_cast<TTable *>(t)) {
      for (i = t->rider; i; i = i->nextRider) {
        if (isname(tmp, i->name) && ch->canSee(i)) {
          if (type == TYPETHING || 
              ((type == TYPEBEING) && dynamic_cast<TBeing *>(i)) ||
              ((type == TYPEOBJ) && dynamic_cast<TObj *>(i)) ||
              ((type == TYPEPC) && dynamic_cast<TPerson *>(i)) ||
              ((type == TYPEMOB) && dynamic_cast<TMonster *>(i))) {
            j++;
            if (j == numx)
              return i;
          }
        }
      }
    }
  }
  if (count) 
    *count = j;
  return NULL;
}

TThing *get_thing_on_list_vis(TBeing *ch, const char *name, TThing *list)
{
  TThing *i;
  int j, numx;
  char tmpname[MAX_INPUT_LENGTH], *tmp;

  if (!*name)
    return NULL;

  strcpy(tmpname, name);
  tmp = tmpname;
  if (!(numx = get_number(&tmp)))
    return (0);
  for (i = list, j = 1; i && (j <= numx); i = i->nextRider) {
    if (isname(tmp, i->name)) {
      if (ch->canSee(i)) {
        if (j == numx)
          return (i);
        j++;
      }
    }
  }
  return (0);
}

TObj *get_obj_vis_world(TBeing *ch, const char *name, int *count, exactTypeT exact)
{
  int j, numx;
  char tmpname[MAX_INPUT_LENGTH], *tmp;

  if (!*name)
    return NULL;

  strcpy(tmpname, name);
  tmp = tmpname;
  if (!(numx = get_number(&tmp)))
    return (0);

  j = count ? *count : 1;

  // ok.. no luck yet. scan the entire obj list 
  for(TObjIter iter=object_list.begin();
      iter!=object_list.end() && (j <= numx);++iter){
    if ((exact && is_exact_name(tmp, (*iter)->name)) ||
        (!exact && isname(tmp, (*iter)->name))) {
      if (ch->canSee(*iter)) {
        if (j == numx)
          return (*iter);
        j++;
      }
    }
  }
  if (count)
    *count = j;
  return (0);
}

// search the entire world for an object, and return a pointer 
TObj *get_obj_vis(TBeing *ch, const char *name, int *count, exactTypeT exact)
{
  TObj *i;
  int numx;
  char tmpname[MAX_INPUT_LENGTH], *tmp;

  if (!*name)
    return NULL;

  strcpy(tmpname, name);
  tmp = tmpname;
  if (!(numx = get_number(&tmp)))
    return (0);

  // int j = count ? *count : 0;

  TThing *ti = searchLinkedListVis(ch, name, ch->getStuff(), count, TYPEOBJ);
  i = dynamic_cast<TObj *>(ti);
  if (i)
    return (i);

  ti = searchLinkedListVis(ch, name, ch->roomp->getStuff(), count, TYPEOBJ);
  i = dynamic_cast<TObj *>(ti);
  if (i)
    return (i);

  return get_obj_vis_world(ch, name, count, exact);
}

// inv, eq, items in room
TObj *get_obj_vis_accessible(TBeing *ch, const sstring &name)
{
  TThing *i;
  TObj *obj = NULL;
  int j = 1, numx, k;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  if (name.empty())
    return NULL;

  strcpy(tmpname, name.c_str());
  tmp = tmpname;
  if (!(numx = get_number(&tmp)))
    return NULL;

  // scan items carried
  for (i = ch->getStuff(); i && j <= numx; i = i->nextThing) {
    obj = dynamic_cast<TObj *>(i);
    if (obj && isname(tmp, obj->name) && ch->canSee(obj)) {
      if (j == numx)
        return obj;
      else
        j++;
    }
  }
  // scan through equipment
//  for (k = MIN_WEAR, j = 1; k < MAX_WEAR && j <= numx; k++) {
  for (k = MIN_WEAR; k < MAX_WEAR && j <= numx; k++) {
    if ((i = ch->equipment[k]) && (obj = dynamic_cast<TObj *>(i)) &&
        isname(tmp, obj->name) && ch->canSee(obj)) {
      if (obj->isPaired()) {
        if (ch->isRightHanded()) {
          if ((k == HOLD_LEFT) || (k == WEAR_LEGS_L))
            continue;
        } else {
          if ((k == HOLD_RIGHT) || (k == WEAR_LEGS_L))
            continue;
        }
      }
      if (j == numx)
        return obj;
      else 
        j++;
    }
  }
  for (i = ch->roomp->getStuff();i && j <= numx; i = i->nextThing) {
    obj = dynamic_cast<TObj *>(i);
    if (obj && isname(tmp, obj->name) && ch->canSee(obj)) {
      if (j == numx)
        return obj;
      else
        j++;
    }
  }
  return NULL;
}

// Generic Find, designed to find any object/character                    
// Calling :                                                              
//  *arg      is the sting containing the sstring to be searched for.       
//            This sstring doesn't have to be a single word, the routine    
//            extracts the next word itself.                              
//  bv        All those bits that you want to "search through".            
//            Bit found will be result of the function                     
//  *ch       This is the person that is trying to "find"                  
//  **tar_ch  Will be NULL if no character was found, otherwise points     
//  **tar_obj Will be NULL if no object was found, otherwise points        
//                                                                        
// The routine returns a pointer to the next word in *arg (just like the  
// one_argument routine).                                                 

TObj *generic_find_obj(sstring arg, int bv, TBeing *ch)
{
  TBeing *tar_ch;
  TObj *o;

  generic_find(arg.c_str(), bv, ch, &tar_ch, &o);

  return o;
}

TBeing *generic_find_being(sstring arg, int bv, TBeing *ch)
{
  TBeing *tar_ch;
  TObj *o;

  generic_find(arg.c_str(), bv, ch, &tar_ch, &o);

  return tar_ch;
}


int generic_find(const char *arg, int bv, TBeing *ch, TBeing **tar_ch, TObj **obj)
{
  const char *ignore[] =
  {
    "the",
    "in",
    "on",
    "at",
    "\n"
  };
  int i;
  bool found = FALSE;
  int count = 0, numx = 0;
  char name[256];
  TThing *t;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

//  strcpy(tmpname, name);
  strcpy(tmpname, arg);
  tmp = tmpname;
  if (!(numx = get_number(&tmp)))
    return (0);

  while (*arg && !found) {
    for (; *arg == ' '; arg++);

    for (i = 0; (name[i] = *(arg + i)) && (name[i] != ' '); i++);
    name[i] = 0;
    arg += i;
    if (search_block(name, ignore, TRUE) > -1)
      found = TRUE;
  }
  if (!name[0])
    return (0);

  strcpy(tmpname, name);
  tmp = tmpname;
  if (!(numx = get_number(&tmp)))
    return (0);

  *tar_ch = NULL;
  *obj = NULL;

  // please leave this ordering alone
  // look for beings first
  // look in inventory before worn 
  // look at worn before room
  // look in room before world
  //  look for extra descriptions in room before world
  // look in world
  // I'm pretty sure this is the order we want everywhere
  // inc: doLook, ...
  // if you don't want this order, do something different...

  if (bv & FIND_CHAR_ROOM) {
    if ((*tar_ch = get_char_room_vis(ch, name, &count)))
      return FIND_CHAR_ROOM;
  }
  if (bv & FIND_CHAR_WORLD) {
    if ((*tar_ch = get_char_vis(ch, name, &count)))
      return FIND_CHAR_WORLD;
  }
  if (bv & FIND_OBJ_INV) {
    if ((t = searchLinkedListVis(ch, name, ch->getStuff(), &count, TYPEOBJ))) {
      *obj = dynamic_cast<TObj *>(t);
      if (*obj) {
        return FIND_OBJ_INV;
      }
    }
  }
  if (bv & FIND_OBJ_EQUIP) {
    wearSlotT j;
    if ((t = get_thing_in_equip(ch, name, ch->equipment, &j, TRUE, &count))) {
      *obj = dynamic_cast<TObj *>(t);
      if (*obj) {
        return FIND_OBJ_EQUIP;
      }
    }
  }
  if (bv & FIND_OBJ_ROOM) {
    if ((t = searchLinkedListVis(ch, name, ch->roomp->getStuff(), &count, TYPEOBJ))) {
      *obj = dynamic_cast<TObj *>(t);
      if (*obj) {
        return FIND_OBJ_ROOM;
      }
    }
  }
  if (bv & FIND_ROOM_EXTRA) {
    if (ch->roomp && ch->roomp->ex_description &&
      ch->roomp->ex_description->findExtraDesc(tmp)) {
      count++;
      if (count == numx)
        return FIND_ROOM_EXTRA;
    }
  }
  if (bv & FIND_OBJ_WORLD) {
    if ((*obj = get_obj_vis(ch, name, NULL, EXACT_NO)))
      return FIND_OBJ_WORLD;
  }
  return (0);
}

// looks in hands and bags
TThing *get_thing_char_using(TBeing *ch, const char *arg, int vnum, bool check_bag, bool check_spellbag)
{
  TThing *t, *t2;
  TThing *bag;
  int num = 1, j = 1;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = NULL;

  if (arg) {
    strcpy(tmpname, arg);
    tmp = tmpname;

    if (!(num = get_number(&tmp)))
      return (0);
  }

  t = ch->heldInPrimHand();
  if (t && ch->canSee(t)) {
    TObj *tobj = dynamic_cast<TObj *>(t);
    if (((vnum >= 0) && tobj && (tobj->objVnum() == vnum)) ||
        (tmp && *tmp && isname(tmp, t->name))) {
      if (j++ == num) 
        return t;
    } else if (dynamic_cast<TSpellBag *>(t) && check_spellbag) {
      bag = t;
      for (t = bag->getStuff(); t; t = t->nextThing) {
        TObj *to = dynamic_cast<TObj *>(t);
        if (((vnum >= 0) && to && (to->objVnum() == vnum)) ||
            (tmp && *tmp && isname(tmp, t->name))) 
          if (j++ == num) 
            return t;
      }
    } else if (dynamic_cast<TBag *>(t) && check_bag) {       
      bag = t;
      for (t = bag->getStuff();t;t = t->nextThing) {
        TObj *to = dynamic_cast<TObj *>(t);
        if (((vnum >= 0) && to && (to->objVnum() == vnum)) ||
            (tmp && *tmp && isname(tmp, t->name))) 
          if (j++ == num) 
            return t;
      }
    }
  }
  t = ch->heldInSecHand();
  if (t && ch->canSee(t)) {
    TObj *tobj = dynamic_cast<TObj *>(t);
    if (((vnum >= 0) && tobj && (tobj->objVnum() == vnum)) ||
        (tmp && *tmp && isname(tmp, t->name))) {
      if (j++ == num) 
        return t;
    } else if (dynamic_cast<TSpellBag *>(t) && check_spellbag) {
      bag = t;
      for (t = bag->getStuff();t;t = t->nextThing) {
        TObj *to = dynamic_cast<TObj *>(t);
        if (((vnum >= 0) && to && (to->objVnum() == vnum)) ||
            (tmp && *tmp && isname(tmp, t->name))) 
          if (j++ == num) 
            return t;
      }
    } else if (dynamic_cast<TBag *>(t) && check_bag) {
      bag = t;
      for (t = bag->getStuff();t;t = t->nextThing) {
        TObj *to = dynamic_cast<TObj *>(t);
        if (((vnum >= 0) && to && (to->objVnum() == vnum)) ||
            (tmp && *tmp && isname(tmp, t->name))) 
          if (j++ == num) 
            return t;
      }
    }
  }
  for (t = ch->getStuff(); t ; t = t->nextThing) {
    if (!ch->canSee(t))
      continue;
    TObj *to = dynamic_cast<TObj *>(t);
    if (((vnum >= 0) && to && (to->objVnum() == vnum)) ||
        (tmp && *tmp && isname(tmp, t->name))) {
      if (j++ == num) 
        return t;
    } else if (dynamic_cast<TSpellBag *>(to) && check_spellbag) {
      bag = to;
      for (t2 = bag->getStuff();t2;t2 = t2->nextThing) {
        TObj *to2 = dynamic_cast<TObj *>(t2);
        if (((vnum >= 0) && to2 && (to2->objVnum() == vnum)) ||
            (tmp && *tmp && isname(tmp, t2->name))) 
          if (j++ == num) 
            return t2;
      }
    } else if (dynamic_cast<TBag *>(to) && check_bag) {
      bag = to;
      for (t2 = bag->getStuff();t2;t2 = t2->nextThing) {
        TObj *to2 = dynamic_cast<TObj *>(t2);
        if (((vnum >= 0) && to2 && (to2->objVnum() == vnum)) ||
            (tmp && *tmp && isname(tmp, t2->name))) 
          if (j++ == num) 
            return t2;
      }
    }
  }
  return NULL;
}

void TBeing::addCaptive(TBeing *ch)
{
  if (!ch)
    return;

  if (ch->getCaptiveOf()) {
    vlogf(LOG_BUG, fmt("addCaptive : trying to add captive (%s) to (%s) when they were already captured.") % 
      ch->getName() % getName());
    return;
  }
  if (getCaptiveOf()) {
    vlogf(LOG_BUG, fmt("addCaptive : trying to add captive (%s) to (%s) who is also captive.") % 
      ch->getName() % getName());
    return;
  }

  ch->setNextCaptive(getCaptive());
  setCaptive(ch);
  ch->setCaptiveOf(this);

  return;
}

void TBeing::remCaptive(TBeing *ch)
{
  TBeing *t, *last;

  if (!this) {
    vlogf(LOG_BUG, "remCaptive called by NULL being.");
    return;
  }

  if (!ch->getCaptiveOf()) {
    vlogf(LOG_BUG,fmt("remCaptive : trying to remove %s when not a captive.") %  ch->getName());
    return;
  }
  last = NULL;
  for (t = getCaptive() ; t; last = t, t = t->getNextCaptive()) {
    if (t == ch)
      break;
  }
  if (!t) {
    vlogf(LOG_BUG,fmt("remCaptive could not find %s in captive list of %s.") % 
      ch->getName() % getName());
    return;
  }  
  if (!last) {
    // head of list
    setCaptive(t->getNextCaptive());
  } else {
    last->setNextCaptive(t->getNextCaptive());
  }
  t->setCaptiveOf(NULL);
  t->setNextCaptive(NULL);
  return;
}

// this duplicates functionality of the C assert() function
// if parm is false, the mud will vlogf errorMsg and then drop a core.

void mud_assert(int parm, const char *errorMsg,...)
{
  char message[512];
  va_list ap;
 
  if (parm)
    return;

  va_start(ap, errorMsg);
  vsprintf(message, errorMsg, ap);
  va_end(ap);

  vlogf(LOG_BUG, fmt("ASSERTION FAILED: %s") %  message);
  abort();    // force a crash
}


// provides "this"'s name, with no color as defined by ch
const sstring TThing::getNameNOC(const TBeing *ch) const
{
  return colorString(ch, ch->desc, getName(), NULL, COLOR_NONE, TRUE);
}

TBeing *get_best_char_room(const TBeing *ch, const char *name, visibleTypeT visible, infraTypeT infra)
{
  TBeing *i;

  if (visible) {
    i = get_char_room_vis(ch, name, NULL, EXACT_YES, infra);

    if (!i)
      i = get_char_room_vis(ch, name, NULL, EXACT_NO, infra);
  } else {
    // see invis, who cares about infra in this situation...
    i = get_char_room(name, ch->inRoom(), NULL);
  }
  
  return i;
}
