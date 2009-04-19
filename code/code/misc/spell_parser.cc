/////////////////////////////////////////////////////////////////
// 
//     spell_parser.cc : All functions related to spell parsing
//
//     Copyright (c) 1998, SneezyMUD Development Team
//     All Rights reserved.
//
/////////////////////////////////////////////////////////////////

#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "low.h"
#include "monster.h"
#include "disc_shaman_armadillo.h"
#include "disc_shaman_healing.h"
#include "disc_air.h"
#include "disc_alchemy.h"
#include "disc_earth.h"
#include "disc_fire.h"
#include "disc_sorcery.h"
#include "disc_spirit.h"
#include "disc_water.h"
#include "disc_aegis.h"
#include "disc_wrath.h"
#include "disc_shaman.h"
#include "disc_afflictions.h"
#include "disc_cures.h"
#include "disc_hand_of_god.h"
#include "disc_deikhan.h"
#include "disc_animal.h"
#include "disc_nature.h"
#include "disc_ranged.h"
#include "disc_adventuring.h"
#include "disc_shaman_frog.h"
#include "disc_shaman_spider.h"
#include "disc_shaman_skunk.h"
#include "disc_shaman_control.h"
#include "obj_opal.h"
#include "weather.h"

int TBeing::useMana(spellNumT spl)
{
  int arrayMana;
  int rounds;
  spl = getSkillNum(spl);
  discNumT das = getDisciplineNumber(spl, FALSE);
  if (das == DISC_NONE) {
    vlogf(LOG_BUG, format("useMana() with bad discipline for spell=%d") %  spl);
    return 0;
  }

  arrayMana = getDiscipline(das)->useMana(getSkillValue(spl),discArray[spl]->minMana);

// divide total mana/rounds for each spell if spell tasked
  if (IS_SET(discArray[spl]->comp_types, SPELL_TASKED) &&
      discArray[spl]->lag >= 0) {
    rounds =  discArray[spl]->lag + 2;
    return arrayMana/rounds;
  } else {
    return arrayMana;
  }
}

// LIFEFORCE
int TBeing::useLifeforce(spellNumT spl)
{
  int arrayLifeforce;
  int rounds;
  int temp;
  int temper;
  spl = getSkillNum(spl);
  discNumT das = getDisciplineNumber(spl, FALSE);
  if (das == DISC_NONE) {
    vlogf(LOG_BUG, format("useLifeforce() with bad discipline for spell=%d") %  spl);
    return 0;
  }
  temp = discArray[spl]->minLifeforce;
  temper = getSkillValue(spl);
  arrayLifeforce = getDiscipline(das)->useLifeforce(temper,temp);
//  arrayLifeforce = getDiscipline(das)->useLifeforce(getSkillValue(spl),discArray[spl]->minLifeforce);

// divide total LF/rounds for each spell if spell tasked
  if (IS_SET(discArray[spl]->comp_types, SPELL_TASKED) &&
      discArray[spl]->lag >= 0) {
    rounds = discArray[spl]->lag + 2;
    return arrayLifeforce/rounds;
  } else {
    return arrayLifeforce;
  }
}

// END LIFEFORCE
 
double TBeing::usePiety(spellNumT spl)
{
  double arrayPiety;
  int rounds;

  spl = getSkillNum(spl);
  discNumT das = getDisciplineNumber(spl, FALSE);
  if (das == DISC_NONE) {
    vlogf(LOG_BUG, format("usePiety() with bad discipline for spell=%d") %  spl);
    return 0;
  }

  arrayPiety =  getDiscipline(das)->usePerc(getSkillValue(spl), (double) discArray[spl]->minPiety);

// divide total piety/rounds for each prayer if prayer tasked
  if (IS_SET(discArray[spl]->comp_types, SPELL_TASKED) &&
      discArray[spl]->lag > 0) {
    rounds =  discArray[spl]->lag + 2;
    return arrayPiety/rounds;
  } else {
    return arrayPiety;
  }
}

bool TBeing::circleFollow(const TBeing *victim) const
{
  const TBeing *k;

  for (k = victim; k; k = k->master) {
    if (k == this)
      return TRUE;
  }
  return FALSE;
}

void TBeing::stopFollower(bool remove, stopFollowerT textLimits) // default argument
{
  followData *j, *k;
  affectedData aff;

  if (!master)
    return;

  if (affectedBySpell(AFFECT_PET) ||
      affectedBySpell(AFFECT_CHARM) ||
      affectedBySpell(AFFECT_THRALL)) {
    // make pet retrainable 
    aff.type = AFFECT_ORPHAN_PET;
    aff.level = 0;
    aff.duration  = 80 * UPDATES_PER_MUDHOUR;
    aff.location = APPLY_NONE;
    aff.modifier = 0;   
    aff.bitvector = 0;
    if (master) {
      char * tmp = mud_str_dup(master->name);
      aff.be = (TThing *) tmp;
    }
    if (roomp) {
      affectTo(&aff, -1);
    } else {
      vlogf(LOG_BUG, format("%s having AFFECT_ORPHAN_PET without a roomp  in stop follower, master is %s") %  getName() % master->getName());
    }
    // take charm off so text is sent
    REMOVE_BIT(specials.affectedBy, AFF_CHARM | AFF_GROUP);
  }
  if (isAffected(AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, this, 0, master, TO_CHAR);
    if (textLimits == STOP_FOLLOWER_SILENT) {
    } else {
      act("$n realizes that $N is a jerk!", FALSE, this, 0, master, TO_NOTVICT);
      act("$n hates your guts!", FALSE, this, 0, master, TO_VICT);
    }
    if (affectedBySpell(SPELL_ENSORCER) && remove)
      affectFrom(SPELL_ENSORCER);
    if (affectedBySpell(SPELL_HYPNOSIS) && remove)
      affectFrom(SPELL_HYPNOSIS);
  } else {
    int levelLimit = (isPlayerAction(PLR_STEALTH) ? MAX_MORT : 0);

    act("You stop following $N.", FALSE, this, 0, master, TO_CHAR);
    if (textLimits == STOP_FOLLOWER_SILENT) {
    } else if (textLimits == STOP_FOLLOWER_DEFAULT) {
      act("$n stops following $N.", FALSE, this, 0, master, TO_NOTVICT, NULL, levelLimit);
      act("$n stops following you.", FALSE, this, 0, master, TO_VICT, NULL, levelLimit);
    } else if (textLimits == STOP_FOLLOWER_CHAR_NOTVICT) { 
      act("$n stops following $N.", FALSE, this, 0, master, TO_NOTVICT, NULL, levelLimit);
    } else if (textLimits == STOP_FOLLOWER_CHAR_VICT) {
      act("$n stops following you.", FALSE, this, 0, master, TO_VICT, NULL, levelLimit);
    }
  }
  if (!master->followers) {
    vlogf(LOG_BUG, "master->followers is NULL in stopFollowers");
    REMOVE_BIT(specials.affectedBy, AFF_CHARM | AFF_GROUP);
    master = NULL;
    return;
  }
  if (master->followers->follower == this) {    // Head of follower-list? */
    k = master->followers;
    master->followers = k->next;
    delete k;
  } else {             // locate follower who is not head of list 
    for (k = master->followers; k->next && k->next->follower != this; k = k->next);

    if ((j = k->next)) {
      k->next = j->next;
      delete j;
    }
  }
  master = NULL;
  REMOVE_BIT(specials.affectedBy, AFF_CHARM | AFF_GROUP);
}

// Called when a character that follows/is followed dies 
void TBeing::dieFollower()
{
  followData *j, *k;

  if (master)
    stopFollower(TRUE);

  for (k = followers; k; k = j) {
    j = k->next;
    k->follower->stopFollower(TRUE);
  }
}

void TBeing::addFollower(TBeing *foll, bool textLimits) // default argument
{
  followData *k,
             *followIndex;

  if (foll->master) {
    vlogf(LOG_BUG, format("add_follower error: this: %s, leader %s, master %s.") %  
          foll->getName() % getName() % foll->master->getName());
    foll->master = NULL;
  }
  foll->master = this;

  k = new followData;

  k->follower = foll;

  if (followers) {
    for (followIndex = followers;
         followIndex && followIndex->next;
         followIndex = followIndex->next);

    // This is bad and shouldn't happen, but you never know.
    if (!followIndex) {
      k->next = followers;
      followers = k;
    } else {
      k->next = NULL;
      followIndex->next = k;
    }
  } else {
    k->next = followers;
    followers = k;
  }

  int levelLimit = (foll->isPlayerAction(PLR_STEALTH) ? MAX_MORT : 0);

  act("You now follow $N.", FALSE, foll, 0, this, TO_CHAR);
  if (textLimits) {
  } else {
    act("$n starts following you.", TRUE, foll, 0, this, TO_VICT, NULL, levelLimit);
    act("$n now follows $N.", TRUE, foll, 0, this, TO_NOTVICT, NULL, levelLimit);
  }
  if (desc && IS_SET(desc->autobits, AUTO_AUTOGROUP)){
    if (!isAffected(AFF_GROUP))
      SET_BIT(specials.affectedBy, AFF_GROUP);
    doGroup(foll->name, true);
  }
}

void TBeing::saySpell(spellNumT si)
{
  sstring spellWords;
  TThing *t=NULL;
  sstring toLayman;
  sstring toExpert;

  static const sstring syls[][2] =
  {
    {" ", " "},
    {"ar", "snee"},
    {"au", "ba"},
    {"bless", "tor"},
    {"blind", "jul"},
    {"bur", "flag"},
    {"cu", "dol"},
    {"ca", "pul"},
    {"de", "umo"},
    {"en", "thar"},
    {"from", "al"},
    {"heal", "dufi"},
    {"hands", "sunt"},
    {"light", "eybo"},
    {"lo", "yi"},
    {"mor", "arb"},
    {"move", "ido"},
    {"ness", "izza"},
    {"ning", "nar"},
    {"ous", "gal"},
    {"per", "duda"},
    {"ra", "gru"},
    {"re", "sar"},
    {"son", "sabru"},
    {"se", "or"},
    {"tect", "infra"},
    {"tri", "cula"},
    {"ven", "nofo"},
    {"a", "a"},
    {"b", "b"},
    {"c", "q"},
    {"d", "e"},
    {"e", "z"},
    {"f", "y"},
    {"g", "o"},
    {"h", "p"},
    {"i", "u"},
    {"j", "y"},
    {"k", "t"},
    {"l", "r"},
    {"m", "w"},
    {"n", "i"},
    {"o", "a"},
    {"p", "s"},
    {"q", "d"},
    {"r", "f"},
    {"s", "g"},
    {"t", "h"},
    {"u", "j"},
    {"v", "z"},
    {"w", "x"},
    {"x", "n"},
    {"y", "l"},
    {"z", "k"},
  };

  spellWords = discArray[si]->name;
  for(unsigned int iReplace = 0; iReplace < cElements(syls); iReplace++)
    spellWords.inlineReplaceString(syls[iReplace][0], syls[iReplace][1]);

  if (discArray[si]->minMana) {
    toLayman = format("$n utters the incantation, '%s'") % spellWords;
    toExpert = format("$n utters the incantation, '%s'") % discArray[si]->name;
  } else if (discArray[si]->minLifeforce) {
    toLayman = format("$n chants the invokation, '%s'") % spellWords;
    toExpert = format("$n chants the invokation, '%s'") % discArray[si]->name;
  } else {
    toLayman = format("$n utters the holy words, '%s'") % spellWords;
    toExpert = format("$n utters the holy words, '%s'") % discArray[si]->name;
  }

  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
    TBeing *temp_char = dynamic_cast<TBeing *>(t);
    if (!temp_char)
      continue;
    if (temp_char != this) {
      if (temp_char->doesKnowSkill(si))
        act(toExpert, FALSE, this, 0, temp_char, TO_VICT);
      else
        act(toLayman, FALSE, this, 0, temp_char, TO_VICT);
    }
  }
  updatePos();
}

static int preflight_mana(TBeing *ch, spellNumT spl)
{
  int howMuch = 0, totalAmt = 0;

  howMuch = ch->useMana(spl);
  totalAmt = ch->getMana() + ch->mostPowerstoneMana(); 

  if (IS_SET(discArray[spl]->comp_types, SPELL_TASKED)) 
    howMuch *= discArray[spl]->lag + 2;
   
  return((howMuch <= totalAmt));
}

// LIFEFORCE
static int preflight_lifeforce(TBeing *ch, spellNumT spl)
{
  int howMuch = 0, totalAmt = 0;

  howMuch = ch->useLifeforce(spl);
  totalAmt = ch->getLifeforce(); 

  if (IS_SET(discArray[spl]->comp_types, SPELL_TASKED)) 
    howMuch *= discArray[spl]->lag + 2;
   
  return((howMuch <= totalAmt));
}
// END LIFEFORCE

static int preflight_piety(TBeing *ch, spellNumT spl)
{
  double howMuch = 0, totalAmt = 0;

  howMuch = ch->usePiety(spl);
  totalAmt = ch->getPiety();

  if (IS_SET(discArray[spl]->comp_types, SPELL_TASKED)) 
    howMuch *= discArray[spl]->lag + 2;

  return ((howMuch <= totalAmt));
}

int TBeing::reconcilePiety(spellNumT spl, bool checking)
{
  double total;

  if (desc && isImmortal())
    return TRUE;

  if (!isImmortal() && !preflight_piety(this, spl)) {
// this is possible.
// spell is only denied if piety < min-piety for spell
// distraction or bad wizardry could require more piety than min-piety
//     vlogf(LOG_BUG, format("%s (spell=%s(%d)) Failed the second of two consecutive prefligh_piety() tests.") %  getName() % discArray[spl]->name % spl);
    if (checking) 
      return FALSE;

  } else if (checking) 
    return TRUE;
  
  total = usePiety(spl);

  if (getPiety() > total) 
    addToPiety(-total);
  else {
    int moveTotal;
    total -= getPiety();
    moveTotal = max(15, (int) total);
    setPiety(0);
    if (getMove() >= moveTotal) {
      sendTo("You have run out of piety and your prayer starts to eat your vitality.\n\r");
      addToMove(-moveTotal);
    } else 
      return FALSE;
  }
  return TRUE;
}

// if spl == TYPE_UNDEFINED, will use mana
// otherwise uses mana appropriate for spl
int TBeing::reconcileMana(spellNumT spl, bool checking, int mana)
{
  int mana_to_burn;

  if (desc && isImmortal())
    return TRUE;
    
  if (spl > TYPE_UNDEFINED) {
    if (!isImmortal() && !preflight_mana(this, spl)) {
// this is possible.
// spell is only denied if mana < min-mana for spell
// distraction or bad wizardry could require more mana than min-mana
//      vlogf(LOG_BUG, format("%s (spell=%s(%d)) Failed the second of two consecutive preflight_mana() tests.") %  getName() % discArray[spl]->name % spl);
      if (checking) {
        return FALSE;
      } else {
      }
    } else if (checking) {
      return TRUE;
    }
    mana_to_burn = useMana(spl);
  } else {
    mana_to_burn = mana;
  }

  if (mana_to_burn > 0) {
    if (getMana() >= mana_to_burn) {
      setMana(max(0, getMana() - mana_to_burn));
    } else {
      mana_to_burn -= getMana();
      mana_to_burn = max(15, mana_to_burn);
      setMana(0);
      if (getMove() >= mana_to_burn) {
        sendTo("You have run out of mana and your spell starts to eat your vitality.\n\r");
        addToMove(-mana_to_burn);
      } else {
        return FALSE;
      }
    }
  }
  return TRUE;
}

// LIFEFORCE
int TBeing::reconcileLifeforce(spellNumT spl, bool checking, int lifeforce)
{
  int lifeforce_to_burn;

  if (desc && isImmortal())
    return TRUE;
    
  if (spl > TYPE_UNDEFINED) {
    if (!isImmortal() && !preflight_lifeforce(this, spl)) {
      if (checking) {
        return FALSE;
      } else {
      }
    } else if (checking) {
      return TRUE;
    }
    lifeforce_to_burn = useLifeforce(spl);
  } else {
    lifeforce_to_burn = lifeforce;
  }
  if (lifeforce_to_burn > 0) {
    if (getLifeforce() >= lifeforce_to_burn) {
      setLifeforce(max(0, getLifeforce() - lifeforce_to_burn));
    } else {
      lifeforce_to_burn -= getLifeforce();
      lifeforce_to_burn = max(15, lifeforce_to_burn);
      setLifeforce(0);
    }
  }
  updatePos();
  return TRUE;
}
// END LIFEFORCE

char *skip_spaces(char *sstring)
{
  for (; *sstring && (*sstring) == ' '; sstring++);

  return (sstring);
}

spellNumT searchForSpellNum(const sstring &arg, exactTypeT exact)
{
  spellNumT i = MIN_SPELL;

  for (i = MIN_SPELL; i < MAX_SKILL; i++) {
    if (hideThisSpell(i))
      continue;

    if (!exact) {
      if (is_abbrev(arg, discArray[i]->name, MULTIPLE_YES))
        return i;
    } else {
    //  if (is_exact_name(arg, discArray[i]->name))
      if (is_exact_spellname(arg, discArray[i]->name))
        return i;
    }
  }

  return TYPE_UNDEFINED;
}

static void badCastSyntax(const TBeing *ch, spellNumT which)
{
  if (which <= TYPE_UNDEFINED) {
    // unknown spell, so make very generic
    ch->sendTo("Syntax : cast/pray <spell name> <argument>\n\r");
    ch->sendTo("See the CAST/PRAY help file for more details!\n\r");
    return;
  }

  bool cast = ((getSpellType(discArray[which]->typ) == SPELL_CASTER) || (getSpellType(discArray[which]->typ) == SPELL_DANCER));

  if (discArray[which]->targets & TAR_IGNORE) {
    ch->sendTo("This spell isn't working right.\n\r");
    return;
  }
  if (IS_SET(discArray[which]->targets, TAR_NAME)) {
    if (cast) {
      ch->sendTo(format("Syntax : cast %s <argument>\n\r") % discArray[which]->name);
      ch->sendTo("See the CAST help file for more details!\n\r");
    } else {
      ch->sendTo(format("Syntax : pray %s <argument>\n\r") % discArray[which]->name);
      ch->sendTo("See the PRAY help file for more details!\n\r");
    }
    return;
  }
  sstring tars = "";
  unsigned int targets = discArray[which]->targets;
  if (!IS_SET(targets, TAR_SELF_NONO) &&
       (targets & (TAR_CHAR_ROOM | TAR_CHAR_WORLD | TAR_FIGHT_SELF | TAR_SELF_ONLY)))
    tars += (tars.empty() ? "self" : " | self");
  if (!IS_SET(targets, TAR_SELF_ONLY) &&
       (targets & (TAR_CHAR_ROOM | TAR_CHAR_WORLD)))
    tars += (tars.empty() ? "others" : " | others");

  if (targets & (TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_WORLD | TAR_OBJ_EQUIP))
    tars += (tars.empty() ? "object" : " | object");

  if (tars.empty()) {
    vlogf(LOG_BUG, format("Unknown targets for spell %d") %  which);
    tars += "???";
  }

  if (cast) {
    ch->sendTo(format("Syntax : cast %s <%s>\n\r") % discArray[which]->name % tars);
    ch->sendTo("See the CAST help file for more details!\n\r");
  } else {
    ch->sendTo(format("Syntax : pray %s <%s>\n\r") % discArray[which]->name % tars);
    ch->sendTo("See the PRAY help file for more details!\n\r");
  }
}

// returns DELETE_THIS
int TBeing::doPray(const char *argument)
{
  char kludge[256];
  char arg[256];
  char *n;
  int spaces = 0;
  char buf[256], buf2[256], buf3[256], argbak[256];

// why can't mobs call this command?
//  if (!isPc() && !desc)
//       return FALSE;

  if (isPc() && GetMaxLevel() > MAX_MORT && !hasWizPower(POWER_NO_LIMITS)) {
    sendTo("You are currently not permitted to cast prayers, sorry.\n\r");
    return FALSE;
  }


  if (!hasHands()) {
    sendTo("Sorry, you don't have the right form for that.\n\r");
    return FALSE;
  }

  if(!doesKnowSkill(SKILL_DEVOTION) && 
     !hasClass(CLASS_CLERIC) && !hasClass(CLASS_DEIKHAN)){
    sendTo("You do not have the faith required to pray.\n\r");
    return FALSE;
  }


  if (nomagic("Sorry, your deity refuses to contact you here."))
    return FALSE;
  
  // Eat spaces off the end and off the beginning
  strncpy(arg, argument, cElements(arg));
  while (isspace(*arg))
    strcpy(arg, &arg[1]);

  if (cantHit > 0) {
    sendTo("You're too busy.\n\r");
    return FALSE;
  }

  if (!*arg) {
    badCastSyntax(this, TYPE_UNDEFINED);
    sendTo("You do NOT need to include ''s around the <prayer name>.\n\r");
    return FALSE;
  }
  for (n = arg; *n; n++) {
    if (isspace(*n))
      spaces++;
  }
  n--;
  while (isspace(*n)) {
    *n = '\0';
    spaces--;
    n--;
  }
  one_argument(arg, kludge, cElements(kludge));
  if (is_abbrev(kludge, "paralyze")) {
    strcpy(argbak, arg);
    strcpy(arg, one_argument(arg, buf, cElements(buf)));   // buf == paralyze
    strcpy(arg, one_argument(arg, buf2, cElements(buf2)));  // buf2 == NULL, target, "limb"

    if (!*buf2) {
      // pray paralyze - target on fight()
      if (!doesKnowSkill(getSkillNum(SPELL_PARALYZE))) {
        sendTo("You don't know that prayer!\n\r");
        return FALSE;
      }
      if (!fight()) {
        badCastSyntax(this, SPELL_PARALYZE);
        return FALSE;
      }
      return doDiscipline(SPELL_PARALYZE, "");
    } else if (is_abbrev(buf2, "limb")) {
      if (!doesKnowSkill(getSkillNum(SPELL_PARALYZE_LIMB))) {
        sendTo("You don't know that prayer!\n\r");
        return FALSE;
      }

      strcpy(arg,  one_argument(arg, buf3, cElements(buf3)));  // buf3 = NULL, or targ
      if (!*buf3) {
        // pray paralyze limb - target on fight()
        if (!fight()) {
          badCastSyntax(this, SPELL_PARALYZE_LIMB);
          return FALSE;
        }
        return doDiscipline(SPELL_PARALYZE_LIMB,"");
      }
      return doDiscipline(SPELL_PARALYZE_LIMB, buf3);
    } else {
      if (!doesKnowSkill(getSkillNum(SPELL_PARALYZE))) {
        sendTo("You don't know that prayer!\n\r");
        return FALSE;
      }
      return doDiscipline(SPELL_PARALYZE, buf2);
    }
  } if (is_abbrev(kludge, "heal")) {
    strcpy(argbak, arg);
    strcpy(arg, one_argument(arg, buf, cElements(buf)));  // buf == heal
    strcpy(arg, one_argument(arg, buf2, cElements(buf2)));
    // pray heal <targ>
    // pray heal spray
    // pray heal light <targ>
    // pray heal light spray

    if (!*buf2) {
      // pray heal - target on self
      if (!doesKnowSkill(getSkillNum(SPELL_HEAL))) {
        sendTo("You don't know that prayer!\n\r");
        return FALSE;
      }
      return doDiscipline(SPELL_HEAL, "");
    } else if (!strcmp(buf2, "spray") || !strcmp(buf2, "spra") || !strcmp(buf2, "spr"
)) {
      // old style was to parse for if abbrev of spray but that would capture 
      //   any "s" like pray heal sp for pray heal spowder would goto spray
      // pray heal spray - no targs
      if (!doesKnowSkill(getSkillNum(SPELL_HEAL_SPRAY))) {
        sendTo("You don't know that prayer!\n\r");
        return FALSE;
      }
      return doDiscipline(SPELL_HEAL_SPRAY, "");
    } else if (!*arg) {
      // pray heal <target>
      // pray heal light - autotarget on self
      // pray heal serious - autotarget on self
      // pray heal critical - autotarget on self
      // pray heal full - autotarget on self

      if (is_abbrev(buf2, "light")) {
        // pray heal light - autotarget
        if (!doesKnowSkill(getSkillNum(SPELL_HEAL_LIGHT))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HEAL_LIGHT, "");
      } else if (is_abbrev(buf2, "serious")) {
        // pray heal serious - autotarget
        if (!doesKnowSkill(getSkillNum(SPELL_HEAL_SERIOUS))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HEAL_SERIOUS, "");
      } else if (is_abbrev(buf2, "critical")) {
        // pray heal critical - autotarget
        if (!doesKnowSkill(getSkillNum(SPELL_HEAL_CRITICAL))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HEAL_CRITICAL, "");
      } else if (is_abbrev(buf2, "full")) {
        // pray heal full - autotarget
        if (!doesKnowSkill(getSkillNum(SPELL_HEAL_FULL))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HEAL_FULL, "");
      } else {
        // pray heal <target>
        if (!doesKnowSkill(getSkillNum(SPELL_HEAL))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HEAL, buf2);
      }
    } else {
      // buf2 = serious/crit
      one_argument(arg, buf3, cElements(buf3));
      // buf3 = spray or targets-name
      if (!strcmp(buf3, "spray") || !strcmp(buf3, "spra") || !strcmp(buf3, "spr")) {
        if (is_abbrev(buf2, "critical")) {
          if (!doesKnowSkill(getSkillNum(SPELL_HEAL_CRITICAL_SPRAY))) {
            sendTo("You don't know that prayer!\n\r");
            return FALSE;
          }
          return doDiscipline(SPELL_HEAL_CRITICAL_SPRAY, "");
        } else if (is_abbrev(buf2, "full")) {
          if (!doesKnowSkill(getSkillNum(SPELL_HEAL_FULL_SPRAY))) {
            sendTo("You don't know that prayer!\n\r");
            return FALSE;
          }
          return doDiscipline(SPELL_HEAL_FULL_SPRAY, "");
        }
        // gets here on something dumb like "pray heal light spray"
      }
      if (is_abbrev(buf2, "light")) {
        if (!doesKnowSkill(getSkillNum(SPELL_HEAL_LIGHT))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HEAL_LIGHT, buf3);
      } else if (is_abbrev(buf2, "critical")) {
        if (!doesKnowSkill(getSkillNum(SPELL_HEAL_CRITICAL))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HEAL_CRITICAL, buf3);
      } else if (is_abbrev(buf2, "serious")) {
        if (!doesKnowSkill(getSkillNum(SPELL_HEAL_SERIOUS))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HEAL_SERIOUS, buf3);
      } else if (is_abbrev(buf2, "full")) {
        if (!doesKnowSkill(getSkillNum(SPELL_HEAL_FULL))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HEAL_FULL, buf3);
      }
    }
    // this can happen: heal llight batopr
    sendTo("That's not a prayer request!\n\r");
    return FALSE;
  } if (is_abbrev(kludge, "harm")) {
    strcpy(argbak, arg);
    strcpy(arg, one_argument(arg, buf, cElements(buf)));  // buf == harm
    strcpy(arg, one_argument(arg, buf2, cElements(buf2)));

    // pray harm <targ>
    // pray harm light <targ>

    if (!*buf2) {
    // pray harm - target on self
      if (!doesKnowSkill(getSkillNum(SPELL_HARM))) {
        sendTo("You don't know that prayer!\n\r");
        return FALSE;
      }

      if (!fight()) {
        badCastSyntax(this, SPELL_HARM);
        return FALSE;
      }
      return doDiscipline(SPELL_HARM, "");
#if 0
    } else if (!strcmp(buf2, "spray") || !strcmp(buf2, "spra") || !strcmp(buf2,
"spr")) { 
      // old style was to parse for if abbrev of spray but that would capture
       //any"s"likepray heal sp for pray heal spowder would goto spray
      if (!doesKnowSkill(getSkillNum(SPELL_HARM_SPRAY))) {
        sendTo("You don't know that prayer!\n\r");
        return FALSE;
      }
      return doDiscipline(SPELL_HARM_SPRAY, "");
#endif

    } else if (!*arg) {
      // !*arg means buf=harm, buf2=targ
      // OR
      // buf=harm, buf2="serious, light, critical, full"
      // if fighting, autotarget

      if (is_abbrev(buf2, "light")) {
        if (!doesKnowSkill(getSkillNum(SPELL_HARM_LIGHT))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        if (!fight()) {
          badCastSyntax(this, SPELL_HARM_LIGHT);
          return FALSE;
        }
        return doDiscipline(SPELL_HARM_LIGHT, "");
      } else if (is_abbrev(buf2, "serious")) {
        if (!doesKnowSkill(getSkillNum(SPELL_HARM_SERIOUS))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        if (!fight()) {
          badCastSyntax(this, SPELL_HARM_SERIOUS);
          return FALSE;
        }
        return doDiscipline(SPELL_HARM_SERIOUS, "");
      } else if (is_abbrev(buf2, "critical")) {
        if (!doesKnowSkill(getSkillNum(SPELL_HARM_CRITICAL))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        if (!fight()) {
          badCastSyntax(this, SPELL_HARM_CRITICAL);
          return FALSE;
        }
        return doDiscipline(SPELL_HARM_CRITICAL, "");
#if 0
      } else if (is_abbrev(buf2, "full")) {
        if (!doesKnowSkill(getSkillNum(SPELL_HARM_FULL))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        if (!fight()) {
          badCastSyntax(this, SPELL_HARM_FULL);
          return FALSE;
        }
        return doDiscipline(SPELL_HARM_FULL, "");
      } else if (is_abbrev(buf2, "spray")) {
        if (!doesKnowSkill(getSkillNum(SPELL_HARM_SPRAY))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HARM_SPRAY, "");
#endif
      } else {
        if (!doesKnowSkill(getSkillNum(SPELL_HARM))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HARM, buf2);
      }
    } else {
      // buf2 = serious/crit
      one_argument(arg, buf3, cElements(buf3));
      // buf3 = spray or targets-name
#if 0
      if (!strcmp(buf3, "spray") || !strcmp(buf3, "spra") || !strcmp(buf3, "spr")) {
        if (is_abbrev(buf2, "critical")) {
          if (!doesKnowSkill(getSkillNum(SPELL_HARM_CRITICAL_SPRAY))) {
            sendTo("You don't know that prayer!\n\r");
            return FALSE;
          }
          return doDiscipline(SPELL_HARM_CRITICAL_SPRAY, "");
        } else if (is_abbrev(buf2, "full")) {
          if (!doesKnowSkill(getSkillNum(SPELL_HARM_FULL_SPRAY))) {
            sendTo("You don't know that prayer!\n\r");
            return FALSE;
          }
          return doDiscipline(SPELL_HARM_FULL_SPRAY, "");
        }
        // gets here on something dumb like "pray heal light spray"
      }
#endif
      if (is_abbrev(buf2, "light")) {
        if (!doesKnowSkill(getSkillNum(SPELL_HARM_LIGHT))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HARM_LIGHT, buf3);
      } else if (is_abbrev(buf2, "critical")) {
        if (!doesKnowSkill(getSkillNum(SPELL_HARM_CRITICAL))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HARM_CRITICAL, buf3);
      } else if (is_abbrev(buf2, "serious")) {
        if (!doesKnowSkill(getSkillNum(SPELL_HARM_SERIOUS))) {
          sendTo("You don't know that prayer!\n\r");
          return FALSE;
        }
        return doDiscipline(SPELL_HARM_SERIOUS, buf3);
      }
    }
    // this can happen: heal llight batopr
    sendTo("That's not a prayer request!\n\r");
    return FALSE;
  }
  spellNumT which;
  if (((which = searchForSpellNum(arg, EXACT_YES)) > TYPE_UNDEFINED) || 
      ((which = searchForSpellNum(arg, EXACT_NO)) > TYPE_UNDEFINED)) {
    if (discArray[which]->typ != SPELL_CLERIC && discArray[which]->typ != SPELL_DEIKHAN) {
      sendTo("That's not a prayer request!\n\r");
      return FALSE;
    }
    if (!doesKnowSkill(getSkillNum(which))) {
      sendTo("You don't know that prayer!\n\r");
      return FALSE;
    }
    return doDiscipline(which, "");
  } else {
    if (!spaces) 
      n = arg;
    else {
      // Parse back until we hit our space
      for (; !isspace(*n); n--);
      *n = '\0';
      n++;
    }
    if (((which = searchForSpellNum(arg, EXACT_YES)) <= TYPE_UNDEFINED) && 
        ((which = searchForSpellNum(arg, EXACT_NO)) <= TYPE_UNDEFINED)) {
      sendTo("No such prayer exists.\n\r");
      return FALSE;
    }
    if (discArray[which]->typ != SPELL_CLERIC && discArray[which]->typ != SPELL_DEIKHAN) {
      sendTo("That's not a prayer request!\n\r");
      return FALSE;
    }
    if (!doesKnowSkill(getSkillNum(which))) {
      sendTo("You don't know that prayer!\n\r");
      return FALSE;
    }
    return doDiscipline(which, n);
  }
  return FALSE;
}

spellNumT TBeing::parseSpellNum(char *arg)
{
  char *n;
  int spaces = 0;
  char kludge[256];

  while (isspace(*arg))
    strcpy(arg, &arg[1]);

  if (!*arg) {
    badCastSyntax(this, TYPE_UNDEFINED);
    sendTo("You do NOT need to include ''s around <spell name>.\n\r");
    return TYPE_UNDEFINED;
  }
  for (n = arg; *n; n++) {
    if (isspace(*n))
      spaces++;
  }
  n--;
  while (isspace(*n)) {
    *n = '\0';
    spaces--;
    n--;
  }
  one_argument(arg, kludge, cElements(kludge));
  if (isname(kludge, "telepathy")) {
    strcpy(arg, one_argument(arg, kludge, cElements(kludge)));
    if (!doesKnowSkill(SPELL_TELEPATHY)) {
      sendTo("You don't know that spell!\n\r");
      return TYPE_UNDEFINED;
    }
    return SPELL_TELEPATHY;
  }
  if (isname(kludge, "romble")) {
    strcpy(arg, one_argument(arg, kludge, cElements(kludge)));
    if (!doesKnowSkill(SPELL_ROMBLER)) {
      sendTo("You don't know that spell!\n\r");
      return TYPE_UNDEFINED;
    }
    return SPELL_ROMBLER;
  }
  spellNumT which;
  if (((which = searchForSpellNum(arg, EXACT_YES)) > TYPE_UNDEFINED) ||
      ((which = searchForSpellNum(arg, EXACT_NO)) > TYPE_UNDEFINED)) {
    if (discArray[which]->typ != SPELL_MAGE && discArray[which]->typ != SPELL_SHAMAN) {
      sendTo("That's not a magic spell!\n\r");
      return TYPE_UNDEFINED;
    }
    if (!doesKnowSkill(getSkillNum(which))) {
      sendTo("You don't know that spell!\n\r");
      return TYPE_UNDEFINED;
    }
    *arg = '\0';
    return which;
  } else {
    if (!spaces) 
      n = arg;
    else {
      // Parse back until we hit our space
      for (; !isspace(*n); n--);
      *n = '\0';
      n++;
    }
    if (((which = searchForSpellNum(arg, EXACT_YES)) <= TYPE_UNDEFINED) && 
        ((which = searchForSpellNum(arg, EXACT_NO)) <= TYPE_UNDEFINED)) {
      sendTo("No such spell exists.\n\r");
      return TYPE_UNDEFINED;
    }
    if (discArray[which]->typ != SPELL_MAGE && discArray[which]->typ != SPELL_SHAMAN) {
      sendTo("That's not a magic spell!\n\r");
      return TYPE_UNDEFINED;
    }
    if (!doesKnowSkill(getSkillNum(which))) {
      sendTo("You don't know that spell!\n\r");
      return TYPE_UNDEFINED;
    }
    strcpy(arg,n);
    return which;
  }
}

int TBeing::preCastCheck()
{
  if (!isPc() && !desc)
    return FALSE;

  if (!hasHands()) {
    sendTo("Sorry, you don't have the right form for that.\n\r");
    return FALSE;
  }

  if (isPc() && GetMaxLevel() > MAX_MORT && !hasWizPower(POWER_NO_LIMITS)) {
    sendTo("You are currently not permitted to cast spells, sorry.\n\r");
    return FALSE;
  }


  if (!isImmortal() && !hasClass(CLASS_MAGE | CLASS_RANGER | CLASS_SHAMAN)) {
    if (hasClass(CLASS_CLERIC)) {
      sendTo("Clerics can't cast spells, they pray!\n\r");
      return FALSE;
    } else if (hasClass(CLASS_DEIKHAN)) {
      sendTo("Deikhans can't cast spells, they pray!\n\r");
      return FALSE;
    } else if (hasClass(CLASS_WARRIOR)) {
      sendTo("Warriors can't cast spells!\n\r");
      return FALSE;
    } else if (hasClass(CLASS_THIEF)) {
      sendTo("Thieves can't cast spells!\n\r");
      return FALSE;
    } else if (hasClass(CLASS_RANGER)) {
      sendTo("Rangers can't cast spells!\n\r");
      return FALSE;
    } else if (hasClass(CLASS_MONK)) {
      sendTo("Monks can't cast spells!\n\r");
      return FALSE;
    }
  }
  if (nomagic("Sorry, you can't do that in this area."))
    return FALSE;

  if (cantHit > 0) {
    sendTo("You're too busy.\n\r");
    return FALSE;
  }
  return TRUE;
}

// returns DELETE_THIS
int TBeing::doCast(const char *argument)
{
  char arg[256];
  spellNumT which;

  if(!preCastCheck())
    return FALSE;
  
  strncpy(arg, argument, cElements(arg));
  arg[cElements(arg)-1] = '\0';

  if((which=parseSpellNum(arg))==TYPE_UNDEFINED)
    return FALSE;


  return doDiscipline(which, arg);
}

// finds the target indicated in n, for spell which and sets ret to it
int TBeing::parseTarget(spellNumT which, char *n, TThing **ret)
{
  TBeing *ch;
  TObj *o;
  TThing *t;
  bool ok;

  ok = FALSE;
  ch = NULL;
  o = NULL;
  bool cast = ((getSpellType(discArray[which]->typ) == SPELL_CASTER) || (getSpellType(discArray[which]->typ) == SPELL_DANCER));

  if (*n) {
    if (IS_SET(discArray[which]->targets, TAR_CHAR_ROOM)) {
      // use a variety of increasingly more generic room checks to find target
      ch = get_best_char_room(this, n, VISIBLE_YES, INFRA_NO);
      if (!ch)
        ch = get_best_char_room(this, n, VISIBLE_YES, INFRA_YES);

      if (ch) {
        if (ch->isPlayerAction(PLR_SOLOQUEST) && (ch != this) &&
            !isImmortal() && isPc()) {
          if (!cast) {
            act("$N is on a quest, you can't invoke prayers on $M!", 
                  FALSE, this, NULL, ch, TO_CHAR);
          } else {
            act("$N is on a quest, you can't cast spells on $M!", 
                  FALSE, this, NULL, ch, TO_CHAR);
          }
          return FALSE;
        }
        if (ch->isPlayerAction(PLR_GRPQUEST) && (ch != this) &&
            !isImmortal() && isPc() && !isPlayerAction(PLR_GRPQUEST)) {
          if (!cast)
            act("$N is on a group quest you aren't on!  No prayers allowed!", FALSE, this, NULL, ch, TO_CHAR);
          else
            act("$N is on a group quest you aren't on! No spells allowed!", 
                  FALSE, this, NULL, ch, TO_CHAR);
          return FALSE;
        }
        if ((discArray[which]->targets & TAR_VIOLENT) && noHarmCheck(ch)) 
          return FALSE;
          
	ok = TRUE;
      }
    }
    if (!ok && (discArray[which]->targets & TAR_CHAR_VIS_WORLD) || (discArray[which]->targets & TAR_CHAR_WORLD)) {
      if ((ch = get_pc_world(this, n, EXACT_YES, INFRA_NO, (discArray[which]->targets & TAR_CHAR_VIS_WORLD))) ||
          (ch = get_pc_world(this, n, EXACT_NO, INFRA_NO, (discArray[which]->targets & TAR_CHAR_VIS_WORLD)))) {
        if (ch->isPlayerAction(PLR_SOLOQUEST) && (ch != this) &&
            !isImmortal() && isPc()) {
          if (!cast)
            act("$N is on a quest, you can't invoke prayers on $M!", FALSE, this, NULL, ch, TO_CHAR);
          else
            act("$N is on a quest, you can't cast spells on $M!", FALSE, this, NULL, ch, TO_CHAR);
          return FALSE;
        }
        if ((discArray[which]->targets & TAR_VIOLENT) && noHarmCheck(ch)) 
          return FALSE;
        
        if (ch->isPlayerAction(PLR_GRPQUEST) && (ch != this) &&
            !isImmortal() && isPc() && !isPlayerAction(PLR_GRPQUEST)) {
          if (!cast)
            act("$N is on a group quest you aren't on!  No prayers allowed!", FALSE, this, NULL, ch, TO_CHAR);
          else
            act("$N is on a group quest you aren't on! No spells allowed!", FALSE, this, NULL, ch, TO_CHAR);
          return FALSE;
        }
        ok = TRUE;
      }
      if (discArray[which]->targets & TAR_CHAR_VIS_WORLD) {
        if (!ok && (ch = get_char_vis(this, n, NULL))) {
          ok = TRUE;
        }
      } else {
        if (!ok && (ch = get_char(n, EXACT_NO))) {
          ok = TRUE;
        }
      }
    }
    int dummy = 0;
    if (!ok && (discArray[which]->targets & TAR_OBJ_INV)) {
      if ((o = dynamic_cast<TObj *>(searchLinkedListVis(this, n, stuff, &dummy, TYPEOBJ)))) {
        ok = TRUE;
      }
    }
    if (!ok && (discArray[which]->targets & TAR_OBJ_ROOM)) {
      if ((o = dynamic_cast<TObj *>(searchLinkedListVis(this, n, roomp->stuff, &dummy, TYPEOBJ)))) 
        ok = TRUE;
    }
    if (!ok && (discArray[which]->targets & TAR_OBJ_WORLD)) {
      if ((o = get_obj_vis(ch, name, NULL, EXACT_NO)))
        ok = TRUE;
    }
    if (!ok && (discArray[which]->targets & TAR_OBJ_EQUIP)) {
      for (int i = MIN_WEAR; i < MAX_WEAR && !ok; i++) {
        if ((t = equipment[i]) && is_abbrev(n, t->getName()) &&
            (o = dynamic_cast<TObj *>(t))) {
          ok = TRUE;
        }
      }
    }
    if (!ok && (discArray[which]->targets & TAR_SELF_ONLY)) {
      if (!strcasecmp(getName(), n)) {
        ch = this;
        ok = TRUE;
      }
    }
    if (!ok && (discArray[which]->targets & TAR_NAME))
      ok = TRUE;

    if (ch && dynamic_cast<TMonster *>(ch)) {
      if ((ch->specials.act & ACT_IMMORTAL)) {
        if (!cast)
          sendTo("Invoke a prayer on an immortal being?!?! Never!!\n\r");
        else
          sendTo("Cast magic on an immortal being?!?! Never!!\n\r");
        return FALSE;
      }
    }
    if (!ok && (discArray[which]->targets & TAR_IGNORE))  
      ok = TRUE;
  } else {
    if ((discArray[which]->targets & TAR_FIGHT_SELF)) {
      // if in a fight, cast this on caster
      // these are generally healing spells
      // just to be nice, if not in fight (and no args), also cast on caster 
      // somewhat silly, because the above is the same as TAR_IGNORE
      ch = this;
      ok = TRUE;
    }
    if (!ok && (discArray[which]->targets & TAR_FIGHT_VICT)) {
      if (fight() && sameRoom(*fight())) {
        ch = fight();
        ok = TRUE;
      }
    }
    if (!ok && (discArray[which]->targets & TAR_SELF_ONLY)) {
      ch = this;
      ok = TRUE;
    }
    if (!ok && (discArray[which]->targets & TAR_IGNORE)) {
      ch = this;
      ok = TRUE;
    }
  }

  if (!ok) {
    if (*n) {
      if ((discArray[which]->targets & TAR_CHAR_VIS_WORLD) || (discArray[which]->targets & TAR_CHAR_WORLD))
        sendTo("There is no such person in this realm!\n\r");
      else if (discArray[which]->targets & TAR_CHAR_ROOM)
        sendTo("There is no such person in this room!\n\r");
      else if (discArray[which]->targets & TAR_OBJ_INV)
        sendTo("There is no such object in your possession.\n\r");
      else if (discArray[which]->targets & TAR_OBJ_ROOM)
        sendTo("There is no such object in this room!\n\r");
      else if (discArray[which]->targets & TAR_OBJ_WORLD)
        sendTo("There is no such object in this realm!\n\r");
      else if (discArray[which]->targets & TAR_OBJ_EQUIP)
        sendTo("You are not wearing anything like that.\n\r");
      else
        sendTo("Invalid target flag for that spell.  Bug this!\n\r");
    } else {
      badCastSyntax(this, which);
    }
    return FALSE;

  } else {
    if ((ch == this) && (discArray[which]->targets & TAR_SELF_NONO)) {
      if (!cast)
        sendTo("You cannot invoke this prayer upon yourself.\n\r");
      else
        sendTo("You cannot cast this spell upon yourself.\n\r");
      return FALSE;
    } else if ((ch != this) && (discArray[which]->targets & TAR_SELF_ONLY)) {
      if (!cast)
        sendTo("You can only invoke this prayer upon yourself.\n\r");
       else
        sendTo("You can only cast this spell upon yourself.\n\r");

      return FALSE;
    } else if (isAffected(AFF_CHARM) && (master == ch)) {
      sendTo("You are afraid that it could harm your master.\n\r");
      return FALSE;
    }
  }

  if(ch) *ret=(TThing *) ch;
  else if (o) *ret=(TThing *) o;

  return TRUE;
}

int TBeing::preDiscCheck(spellNumT which)
{
  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo("You can't do that while berserking!\n\r");
    return FALSE;
  }
  if ((discArray[which]->minPosition >= POSITION_CRAWLING) && fight()) {
    sendTo("You can't concentrate enough while fighting!\n\r");
    return FALSE;
  } else if (getPosition() < discArray[which]->minPosition) {
    switch (getPosition()) {
      case POSITION_SLEEPING:
        sendTo("You can't do that while sleeping.\n\r");
        break;
      case POSITION_RESTING:
        sendTo("You can't do that while resting.\n\r");
        break;
      case POSITION_SITTING:
        sendTo("You can't do that while sitting.\n\r");
        break;
      case POSITION_CRAWLING:
        sendTo("You can't do that while crawling.\n\r");
        break;
      default:
        sendTo("You can't do that while unconscious.\n\r");
        break;
    }
    return FALSE;
  }

  if ((discArray[which]->targets & TAR_VIOLENT) && 
      checkPeaceful("Violent disciplines are not allowed here!\n\r"))
    return FALSE;

  if (isPc() && !isImmortal()) {
    if (discArray[which]->minMana) {
      if (!preflight_mana(this, which)) {
        sendTo("You can't summon enough energy to cast the spell.\n\r");
        return FALSE;
      }
    } else if (discArray[which]->minLifeforce) {
      if (!preflight_lifeforce(this, which)) {
        sendTo("You don't have enough lifeforce to perform the ritual.\n\r");
        return FALSE;
      }
    } else {
      if (!preflight_piety(this, which)) {
        sendTo("You lack the piety to pray to your deity for that.\n\r");
        return FALSE;
      }
    }
  }
  return TRUE;
}


// returns DELETE_THIS
int TBeing::doDiscipline(spellNumT which, const char *n)
{
  TObj *o = NULL; 
  TThing *t = NULL;
  TBeing *ch = NULL;
  int rc = 0;
  char arg[256];

  if (!discArray[which]) {
    vlogf(LOG_BUG, format("doDiscipline called with null discArray[] (%d) (%s)") %  which % getName());
    return FALSE;
  }
  if (which <= TYPE_UNDEFINED) 
    return FALSE;

  if(!preDiscCheck(which))
    return FALSE;

  strncpy(arg, n, cElements(arg));
  arg[cElements(arg)-1] = '\0';
  if(!parseTarget(which, arg, &t))
    return FALSE;

  ch=dynamic_cast<TBeing *>(t);
  o=dynamic_cast<TObj *>(t);

#if 0
// COSMO CASTING MARKER
// check if this is right
  if (isPc() && canSpeak()) {
    if (discArray[which]->minMana && 
        (getWizardryLevel() < WIZ_LEV_NO_MANTRA))
      saySpell(which);
  }
  if (isPc() && canSpeak()) {
    if (discArray[which]->minLifeforce && 
        (getRitualismLevel() < RIT_LEV_NO_MANTRA))
      saySpell(which);
  }
#endif
  if (isPc() && canSpeak()) {
    if (discArray[which]->holyStrength && 
        IS_SET(discArray[which]->comp_types, SPELL_TASKED_EVERY) && 
        (getDevotionLevel() < DEV_LEV_NO_MANTRA))
      saySpell(which);
  }
  // We can call the spell now, switch to see which to call!
//    sendTo("Ok.\n\r");

// COSMO MARKER: Mana..Piety checked here ...have to change useMana too
  if (isPc()) {
    if (discArray[which]->minMana) {
      if (!reconcileMana(which, TRUE)) 
        return FALSE;
    } else if (discArray[which]->minLifeforce) {
      if (!reconcileLifeforce(which, TRUE)) 
        return FALSE;
    } else {
      if (!reconcilePiety(which, TRUE)) 
        return FALSE;
    }
  }
  your_deity_val = which;
  inPraying = TRUE; 

  switch(which) {
    case SPELL_GUST:
      rc = gust(this,ch);
      break;
    case SPELL_IMMOBILIZE:
      rc = immobilize(this,ch);
      break;
    case SPELL_SUFFOCATE:
      rc = suffocate(this,ch);
      break;
    case SPELL_DUST_STORM:
      rc = dustStorm(this);
      break;
    case SPELL_TORNADO:
      tornado(this);
      break;
    case SPELL_CONJURE_AIR:
      rc = conjureElemAir(this);
      break;
    case SPELL_ENTHRALL_SPECTRE:
      rc = enthrallSpectre(this);
      break;
    case SPELL_ENTHRALL_GHAST:
      rc = enthrallGhast(this);
      break;
    case SPELL_ENTHRALL_GHOUL:
      rc = enthrallGhoul(this);
      break;
    case SPELL_ENTHRALL_DEMON:
      rc = enthrallDemon(this);
      break;
    case SPELL_CREATE_WOOD_GOLEM:
      rc = createWoodGolem(this);
      break;
    case SPELL_CREATE_ROCK_GOLEM:
      rc = createRockGolem(this);
      break;
    case SPELL_CREATE_IRON_GOLEM:
      rc = createIronGolem(this);
      break;
    case SPELL_CREATE_DIAMOND_GOLEM:
      rc = createDiamondGolem(this);
      break;
    case SPELL_FEATHERY_DESCENT:
      rc = featheryDescent(this, ch);
      break;
    case SPELL_FLY:
      rc = fly(this, ch);
      break;
    case SPELL_ANTIGRAVITY:
      rc = antigravity(this);
      break;
    case SPELL_FALCON_WINGS:
      rc = falconWings(this, ch);
      break;
    case SPELL_LEVITATE:
      levitate(this, ch);
      break;
    case SPELL_PROTECTION_FROM_AIR:
      rc = protectionFromAir(this, ch);
      break;
    case SPELL_DJALLA:
      rc = djallasProtection(this, ch);
      break;
    case SPELL_LEGBA:
      rc = legbasGuidance(this, ch);
      break;
    case SPELL_IDENTIFY:
      if (!o) {
        rc = identify(this,ch);
      } else
        rc = identify(this,o);
      break;
    case SPELL_DIVINATION:
      if (!o) {
        rc = divinationBeing(this,ch);
      } else
        rc = divinationObj(this,o);
      break;
    case SPELL_EYES_OF_FERTUMAN:
      rc = eyesOfFertuman(this, ch);
      break;
    case SPELL_POWERSTONE:
      rc = powerstone(this, o);
      break;
    case SPELL_SHATTER:
      rc = shatter(this, ch);
      break;
    case SPELL_FARLOOK:
      rc = farlook(this, ch);
      break;
    case SPELL_ILLUMINATE:
      rc = illuminate(this, o);
      break;
    case SPELL_DETECT_MAGIC:
      rc = detectMagic(this, ch);
      break;
    case SPELL_DISPEL_MAGIC:
      if (!o) {
        rc = dispelMagic(this,ch);
      } else
        rc = dispelMagic(this,o);
      break;
    case SPELL_COPY:
      rc = copy(this, o);
      break;
    case SPELL_ENHANCE_WEAPON:
      rc = enhanceWeapon(this, o);
      break;
    case SPELL_MATERIALIZE:
      materialize(this, n); 
      break;
    case SPELL_SPONTANEOUS_GENERATION:
      spontaneousGeneration(this, n);
      break;
    case SPELL_ETHER_GATE:
      ethrealGate(this, o);
      break;
    case SPELL_GALVANIZE:
      rc = galvanize(this, o);   
      break;
    case SPELL_SLING_SHOT:
      rc = slingShot(this, ch);
      break;
    case SPELL_GRANITE_FISTS:
      rc = graniteFists(this, ch);
      break;
    case SPELL_METEOR_SWARM:
      rc = meteorSwarm(this, ch);
      break;
    case SPELL_PEBBLE_SPRAY:
      rc = pebbleSpray(this);
      break;
    case SPELL_SAND_BLAST:
      rc = sandBlast(this);
      break;
    case SPELL_LAVA_STREAM:
      rc = lavaStream(this);
      break;
    case SPELL_STONE_SKIN:
      rc = stoneSkin(this, ch);
      break;
    case SPELL_TRAIL_SEEK:
      rc = trailSeek(this, ch);
      break;
    case SPELL_CONJURE_EARTH:
      conjureElemEarth(this);
      break;
    case SPELL_PROTECTION_FROM_EARTH:
      protectionFromEarth(this, ch);
      break;
    case SPELL_HANDS_OF_FLAME:
      rc = handsOfFlame(this, ch);
      break;
    case SPELL_FAERIE_FIRE:
      faerieFire(this, ch);
      break;
    case SPELL_FLAMING_SWORD:
      rc = flamingSword(this, ch);
      break;
    case SPELL_INFERNO:
      rc = inferno(this, ch);
      break;
    case SPELL_FIREBALL:
      rc = fireball(this);
      break;
    case SPELL_HELLFIRE:
      rc = hellfire(this);
      break;
    case SPELL_FLAMING_FLESH:
      rc = flamingFlesh(this, ch);
      break;
    case SPELL_CONJURE_FIRE:
      rc = conjureElemFire(this);
      break;
    case SPELL_STUPIDITY:
      stupidity(this, ch);
      break;
    case SPELL_CHASE_SPIRIT:
      if (!o) {
        rc = chaseSpirits(this,ch);
      } else
        rc = chaseSpirits(this,o);
      break;
    case SPELL_FLARE:
      rc = flare(this);
      break;
    case SPELL_INFRAVISION:
      infravision(this, ch);
      break;
    case SPELL_PROTECTION_FROM_FIRE:
      protectionFromFire(this, ch);
      break;
    case SPELL_MYSTIC_DARTS:
      rc = mysticDarts(this, ch);
      break;
    case SPELL_STUNNING_ARROW:
      rc = stunningArrow(this, ch);
      break;
    case SPELL_STICKS_TO_SNAKES:
      sticksToSnakes(this, ch);
      break;
    case SPELL_DISTORT:
      rc = distort(this, ch);
      break;
    case SPELL_DEATHWAVE:
      rc = deathWave(this, ch);
      break;
    case SPELL_SOUL_TWIST:
      rc = soulTwist(this, ch);
      break;
    case SPELL_BLOOD_BOIL:
      rc = bloodBoil(this, ch);
      break;
    case SPELL_FLATULENCE:
      rc = flatulence(this);
      break;
    case SPELL_BLAST_OF_FURY:
      rc = blastOfFury(this, ch);
      break;
    case SPELL_ENERGY_DRAIN:
      rc = energyDrain(this, ch);
      break;
    case SPELL_ATOMIZE:
      rc = atomize(this, ch);
      break;
    case SPELL_COLOR_SPRAY:
      rc = colorSpray(this);
      break;
    case SPELL_ACID_BLAST:
      rc = acidBlast(this);
      break;
#if 0
    case SPELL_CHAIN_LIGHTNING:
      rc = chainLightning(this);
      break;
#endif
    case SPELL_ANIMATE:
      animate(this);
      break;
    case SPELL_SORCERERS_GLOBE:
      sorcerersGlobe(this, ch);
      break;
    case SPELL_BIND:
      bind(this, ch);
      break;
    case SPELL_TELEPORT:
      rc = teleport(this, ch);
      break;
    case SPELL_KNOT:
      rc = knot(this, ch);
      break;
    case SPELL_PROTECTION_FROM_ELEMENTS:
      protectionFromElements(this, ch);
      break;
    case SPELL_SENSE_LIFE:
      senseLife(this, ch);
      break;
    case SPELL_SENSE_LIFE_SHAMAN:
      senseLifeShaman(this, ch);
      break;
    case SPELL_SILENCE:
      silence(this, ch);
      break;
    case SPELL_STEALTH:
      stealth(this, ch);
      break;
    case SPELL_CALM:
      calm(this, ch);
      break;
    case SPELL_SQUISH:
      rc = squish(this, ch);
      break;
    case SPELL_ENSORCER:
      rc = ensorcer(this, ch);
      break;
    case SPELL_FEAR:
      rc = fear(this, ch);
      break;
    case SPELL_INTIMIDATE:
      rc = intimidate(this, ch);
      break;
    case SPELL_INVISIBILITY:
      if (!o) {
        invisibility(this, ch);
      } else
        invisibility(this, o);
      break;
    case SPELL_CLOUD_OF_CONCEALMENT:
      rc = cloudOfConcealment(this);
      break;
    case SPELL_DETECT_INVISIBLE:
      detectInvisibility(this, ch);
      break;
    case SPELL_DETECT_SHADOW:
      detectShadow(this, ch);
      break;
    case SPELL_DISPEL_INVISIBLE:
      if (!o) 
        dispelInvisible(this, ch);
      else
        dispelInvisible(this, o);
      break;
    case SPELL_TELEPATHY:
      telepathy(this, n);
      break;
    case SPELL_ROMBLER:
      rombler(this, n);
      break;
    case SPELL_RAZE:
      rc = raze(this, ch);
      break;
    case SPELL_TRUE_SIGHT:
      trueSight(this, ch);
      break;
    case SPELL_POLYMORPH:
      polymorph(this, n);
      break;
    case SPELL_ACCELERATE:
      accelerate(this, ch);
      break;
    case SPELL_HASTE:
      haste(this, ch);
      break;
    case SPELL_SLUMBER:
      rc = slumber(this, ch);
      break;
    case SPELL_FUMBLE:
      rc = fumble(this, ch);
      break;
    case SPELL_GUSHER:
      rc = gusher(this, ch);
      break;
    case SPELL_AQUATIC_BLAST:
      rc = aquaticBlast(this, ch);
      break;
    case SPELL_CARDIAC_STRESS:
      rc = cardiacStress(this, ch);
      break;
    case SPELL_FAERIE_FOG:
      faerieFog(this);
      break;
    case SPELL_ICY_GRIP:
      rc = icyGrip(this, ch);
      break;
    case SPELL_WATERY_GRAVE:
      rc = wateryGrave(this, ch);
      break;
    case SPELL_ARCTIC_BLAST:
      rc = arcticBlast(this);
      break;
    case SPELL_ICE_STORM:
      rc = iceStorm(this);
      break;
    case SPELL_TSUNAMI:
      rc = tsunami(this);
      break;
    case SPELL_CONJURE_WATER:
      conjureElemWater(this);
      break;
    case SPELL_GILLS_OF_FLESH:
      gillsOfFlesh(this, ch);
      break;
    case SPELL_AQUALUNG:
      aqualung(this, ch);
      break;
    case SPELL_BREATH_OF_SARAHAGE:
      rc = breathOfSarahage(this);
      break;
    case SPELL_PROTECTION_FROM_WATER:
      protectionFromWater(this, ch);
      break;
    case SPELL_PLASMA_MIRROR:
      rc = plasmaMirror(this);
      break;
    case SPELL_GARMULS_TAIL:
      rc = garmulsTail(this, ch);
      break;
    case SPELL_FLAMESTRIKE:
      rc = flamestrike(this, ch);
      break;
    case SPELL_CURSE:
    case SPELL_CURSE_DEIKHAN:
      if (!o)
        curse(this, ch);
      else
        curse(this, o);
      break;
    case SPELL_EARTHQUAKE:
    case SPELL_EARTHQUAKE_DEIKHAN:
      rc = earthquake(this);
      break;
    case SPELL_CALL_LIGHTNING:
    case SPELL_CALL_LIGHTNING_DEIKHAN:
      rc = callLightning(this, ch);
      break;
    case SPELL_PILLAR_SALT:
      rc = pillarOfSalt(this, ch);
      break;
    case SPELL_SPONTANEOUS_COMBUST:
      rc = spontaneousCombust(this, ch);
      break;
    case SPELL_PLAGUE_LOCUSTS:
      rc = plagueOfLocusts(this, ch);
      break;
    case SPELL_RAIN_BRIMSTONE:
    case SPELL_RAIN_BRIMSTONE_DEIKHAN:
      rc = rainBrimstone(this, ch);
      break;

    case SPELL_HARM_LIGHT:
    case SPELL_HARM_LIGHT_DEIKHAN:
      rc = harmLight(this, ch);
      break;
    case SPELL_HARM_SERIOUS:
    case SPELL_HARM_SERIOUS_DEIKHAN:
      rc = harmSerious(this, ch);
      break;
    case SPELL_HARM_CRITICAL:
    case SPELL_HARM_CRITICAL_DEIKHAN:
      rc = harmCritical(this, ch);
      break;
    case SPELL_HARM:
    case SPELL_HARM_DEIKHAN:
      rc = harm(this, ch);
      break;
    case SPELL_POISON:
    case SPELL_POISON_DEIKHAN:
        if (!o)
          poison(this, ch);
        else
          poison(this, o);
        break;
      case SPELL_PARALYZE_LIMB:
        rc = paralyzeLimb(this, ch);
        break;
      case SPELL_WITHER_LIMB:
        rc = witherLimb(this, ch);
        break;
      case SPELL_BONE_BREAKER:
        rc = boneBreaker(this, ch);
        break;
      case SPELL_PARALYZE:
        paralyze(this, ch);
        break;
      case SPELL_BLEED:
        rc = bleed(this, ch);
        break;
      case SPELL_INFECT:
      case SPELL_INFECT_DEIKHAN:
        infect(this, ch);
        break;
      case SPELL_DISEASE:
        disease(this, ch);
        break;
      case SPELL_BLINDNESS:
        blindness(this, ch);
        break;
      case SPELL_NUMB:
      case SPELL_NUMB_DEIKHAN:
        rc = numb(this, ch);
        break;
      case SPELL_HEAL_LIGHT:
      case SPELL_HEAL_LIGHT_DEIKHAN:
        healLight(this, ch);
        break;
      case SPELL_HEAL_SERIOUS:
      case SPELL_HEAL_SERIOUS_DEIKHAN:
        healSerious(this, ch);
        break;
      case SPELL_HEAL_CRITICAL:
      case SPELL_HEAL_CRITICAL_DEIKHAN:
        healCritical(this, ch);
        break;
      case SPELL_HEAL:
        heal(this, ch);
        break;
      case SPELL_HEAL_FULL:
        healFull(this, ch);
        break;
      case SPELL_HEAL_CRITICAL_SPRAY:
        healCritSpray(this);
        break;
      case SPELL_HEAL_SPRAY:
        healSpray(this);
        break;
      case SPELL_HEAL_FULL_SPRAY:
        healFullSpray(this);
        break;
      case SPELL_SALVE:
      case SPELL_SALVE_DEIKHAN:
        salve(this, ch);
        break;
      case SPELL_RESTORE_LIMB:
        restoreLimb(this, ch);
        break;
      case SPELL_KNIT_BONE:
          knitBone(this, ch);
        break;
      case SPELL_CLOT:
      case SPELL_CLOT_DEIKHAN:
          clot(this, ch);
        break;
      case SPELL_STERILIZE:
      case SPELL_STERILIZE_DEIKHAN:
          sterilize(this, ch);
        break;
      case SPELL_EXPEL:
      case SPELL_EXPEL_DEIKHAN:
          expel(this, ch);
        break;
      case SPELL_HEALING_GRASP:
        healingGrasp(this, ch);
        break;
      case SPELL_CONTROL_UNDEAD:
	controlUndead(this, ch);
        break;
      case SPELL_CLARITY:
        clarity(this, ch);
        break;
      case SPELL_CLEANSE:
        cleanse(this, ch);
        break;
      case SPELL_LICH_TOUCH:
        rc = lichTouch(this, ch);
        break;
      case SPELL_CHEVAL:
        cheval(this, ch);
        break;
      case SPELL_CELERITE:
        celerite(this, ch);
        break;
      case SPELL_VAMPIRIC_TOUCH:
        rc = vampiricTouch(this, ch);
        break;
      case SPELL_LIFE_LEECH:
        rc = lifeLeech(this, ch);
        break;
      case SPELL_HYPNOSIS:
        rc = hypnosis(this, ch);
        break;
      case SPELL_THORNFLESH:
        rc = thornflesh(this);
	break;
      case SPELL_DEATH_MIST:
	rc = deathMist(this);
	break;
      case SPELL_SHIELD_OF_MISTS:
        shieldOfMists(this, ch);
        break;
      case SPELL_SHADOW_WALK:
	rc = shadowWalk(this, ch);
	break;
      case SPELL_ENLIVEN:
        enliven(this, ch);
        break;
      case SPELL_CHRISM:
        chrism(this, n); 
	break;
      case SPELL_VOODOO:
          rc = voodoo(this, o);
        break;
      case SPELL_DANCING_BONES:
          rc = dancingBones(this, o);
        break;
      case SPELL_RESURRECTION:
          rc = resurrection(this, o);
        break;

      case SPELL_PORTAL:
          portal(this, n);
        break;
      case SPELL_CREATE_FOOD:
      case SPELL_CREATE_FOOD_DEIKHAN:
          createFood(this);
        break;
      case SPELL_CREATE_WATER:
      case SPELL_CREATE_WATER_DEIKHAN:
          createWater(this, o);
        break;
      case SPELL_HEROES_FEAST:
      case SPELL_HEROES_FEAST_DEIKHAN:
          heroesFeast(this);
        break;
      case SPELL_ASTRAL_WALK:
          rc = astralWalk(this, ch);
        break;
      case SPELL_WORD_OF_RECALL:
          wordOfRecall(this, ch);
        break;
      case SPELL_SUMMON:
          rc = summon(this, ch);
        break;
      case SPELL_SYNOSTODWEOMER:
          rc = synostodweomer(this, ch);
        break;
#if 0
      case SKILL_BARKSKIN:
          rc = barkskin(this, ch);
        break;
#endif
      case SPELL_ROOT_CONTROL:
          rootControl(this, ch);
        break;
      case SPELL_LIVING_VINES:
          livingVines(this, ch);
        break;
      case SPELL_STORMY_SKIES:
        rc = stormySkies(this, ch);
        break;
#if 0
      case SKILL_TRANSFORM_LIMB:
          transformLimb(this, n);
        break;
#endif
      case SPELL_TREE_WALK:
          rc = treeWalk(this, n);
        break;

#if 0
      case SKILL_BEAST_SOOTHER:
          rc = beastSoother(this, ch);
        break;
      case SKILL_BEAST_SUMMON:
          rc = beastSummon(this, n);
        break;
      case SKILL_TRANSFIX:
          rc = transfix(this, ch);
        break;

#endif
      case SPELL_BLESS_DEIKHAN:
      case SPELL_BLESS:
        if (!o)
          bless(this, ch);
        else
          bless(this, o);
        break;
      case SPELL_ARMOR_DEIKHAN:
      case SPELL_ARMOR:
          armor(this, ch);
        break;
      case SPELL_SANCTUARY:
          sanctuary(this, ch);
        break;
      case SPELL_RELIVE:
        relive(this,ch);
	break;
      case SPELL_CURE_POISON:
      case SPELL_CURE_POISON_DEIKHAN:
        curePoison(this, ch);
        break;
      case SPELL_CURE_PARALYSIS:
        cureParalysis(this, ch);
        break;
      case SPELL_CURE_BLINDNESS:
        cureBlindness(this, ch);
        break;
      case SPELL_CURE_DISEASE:
      case SPELL_CURE_DISEASE_DEIKHAN:
        cureDisease(this, ch);
        break;
      case SPELL_REFRESH:
      case SPELL_REFRESH_DEIKHAN:
        refresh(this, ch);
        break;
    case SPELL_SECOND_WIND:
      secondWind(this, ch);
      break;
    case SPELL_SHAPESHIFT:
      shapeShift(this, n);
      break;
    case SPELL_REMOVE_CURSE:
    case SPELL_REMOVE_CURSE_DEIKHAN:
      if (!o)
        removeCurseBeing(ch);
      else
        removeCurseObj(o);
      break;
    case SPELL_EMBALM:
      embalm(this, o);
      break;

    default:
        sendTo("Spell or discipline not yet implemented!\n\r");
        return FALSE;
  }

// COSMO MARKER: Mana..Piety taken through here ...have to change useMana too
  if (inPraying)
    inPraying = 0;

  if (isPc()) {
    if (discArray[which]->minMana) {
      reconcileMana(which, FALSE);
    } else if (discArray[which]->minLifeforce) {
      reconcileLifeforce(which, FALSE);
    } else {
      reconcilePiety(which, FALSE);
    }
  }
// COSMO MARKER: Spell lag added here
// Need to modify to seperate casting and lag spells
  // one combat round is 2 seconds 
  if (!IS_SET(discArray[which]->comp_types, SPELL_TASKED))
    addSkillLag(which, rc);

    
#if 0
  if (cast)
    learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_CASTING, 20);
  else
    learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_PRAYING, 20);
#endif

  if (IS_SET_DELETE(rc, DELETE_VICT) && ch != this) {
    delete ch;
    ch = NULL;
  }
  if (o && IS_SET_DELETE(rc, DELETE_ITEM)) {
    delete o;
    o = NULL;
  }
  if ((IS_SET_DELETE(rc, DELETE_VICT) && ch == this) ||
      IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  // this handles the "offensive" spells
  if (discArray[which]->targets & TAR_VIOLENT) {
    if (ch)
      reconcileDamage(ch, 0, which);

    if (discArray[which]->targets & TAR_AREA) {
      bool found = false;
      for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
        TBeing * victim = dynamic_cast<TBeing *>(t);
        if (!victim)
          continue;
        if (!inGroup(*victim) && !victim->isImmortal()) {
          reconcileDamage(victim, 0, which);
          if (!found) {
            if (discArray[which]->minMana) {
              act("Your enemies sense your offensive magic.", FALSE, this, NULL, NULL ,TO_CHAR);
              act("You sense offensive magic emanating from $n.", FALSE, this, NULL, NULL, TO_ROOM);
            } else if (discArray[which]->minLifeforce) {
              act("Your enemies sense your harmful invokation is directed toward them.", FALSE, this, NULL, NULL ,TO_CHAR);
              act("You sense evil from $n.", FALSE, this, NULL, NULL, TO_ROOM);
            } else {
              act("Your enemies sense your offensive praying.", FALSE, this, NULL, NULL ,TO_CHAR);
              act("You sense offensive praying emanating from $n.", FALSE, this, NULL, NULL, TO_ROOM);
            }
          }
          found = true;
        }
      }
    }
  }
  updatePos();
  // this SHOULD return TRUE - in theory
  // hope I'm not fucking everything up by making it 'correct'
  return TRUE;
  //  return FALSE;
}

void buildWeatherDamMap()
{
  int i;
  i = SPELL_GUST;
    discArray[i]->weatherData[Weather::RAINY] = -20;
//    discArray[i]->weatherData[Weather::WINDY] = 20;
  i = SPELL_HANDS_OF_FLAME;
    discArray[i]->weatherData[Weather::RAINY] = 20;
    discArray[i]->weatherData[Weather::SNOWY] = 20;
}

void buildTerrainDamMap()
{
  int i;
  i = SPELL_GUST;
    discArray[i]->sectorData[SECT_TEMPERATE_ATMOSPHERE] = 20;
    discArray[i]->sectorData[SECT_TEMPERATE_OCEAN] = -20;
    discArray[i]->sectorData[SECT_TEMPERATE_ROAD] = 5;
  i = SPELL_HANDS_OF_FLAME;
    discArray[i]->sectorData[SECT_TEMPERATE_ATMOSPHERE] = 10;
    discArray[i]->sectorData[SECT_TEMPERATE_OCEAN] = -50;
    discArray[i]->sectorData[SECT_TEMPERATE_ROAD] = -5;

}


