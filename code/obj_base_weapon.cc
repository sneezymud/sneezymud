//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
// base_weapon.cc
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "combat.h"
#include "statistics.h"
#include "shop.h"
#include "obj_tool.h"
#include "obj_arrow.h"
#include "obj_general_weapon.h"
#include "obj_base_weapon.h"
#include "shopowned.h"
#include "corporation.h"

TBaseWeapon::TBaseWeapon() :
  TObj(),
  maxSharp(0),
  curSharp(0),
  damLevel(0),
  damDev(0),
  poison((liqTypeT)-1)
{
}

TBaseWeapon::TBaseWeapon(const TBaseWeapon &a) :
  TObj(a),
  maxSharp(a.maxSharp),
  curSharp(a.curSharp),
  damLevel(a.damLevel),
  damDev(a.damDev),
  poison(a.poison)
{
}

TBaseWeapon & TBaseWeapon::operator=(const TBaseWeapon &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  maxSharp = a.maxSharp;
  curSharp = a.curSharp;
  damLevel = a.damLevel;
  damDev = a.damDev;
  poison = a.poison;
  return *this;
}

TBaseWeapon::~TBaseWeapon()
{
}

void TBaseWeapon::setMaxSharp(int n)
{
  maxSharp = n;
}

int TBaseWeapon::getMaxSharp() const
{
  return maxSharp;
}

void TBaseWeapon::addToMaxSharp(int n)
{
  maxSharp += n;
}

void TBaseWeapon::setCurSharp(int n)
{
  curSharp = n;
}

int TBaseWeapon::getCurSharp() const
{
  return curSharp;
}

void TBaseWeapon::addToCurSharp(int n)
{
  curSharp += n;
}

void TBaseWeapon::setWeapDamLvl(int n)
{
  damLevel = n;
}

int TBaseWeapon::getWeapDamLvl() const
{
  return damLevel;
}

void TBaseWeapon::setWeapDamDev(int n)
{
  damDev = n;
}

int TBaseWeapon::getWeapDamDev() const
{
  return damDev;
}

void TBaseWeapon::assignFourValues(int x1, int x2, int, int)
{
  setCurSharp(GET_BITS(x1, 7, 8));
  setMaxSharp(GET_BITS(x1, 15, 8));
  setWeapDamLvl(GET_BITS(x2, 7, 8));
  setWeapDamDev(GET_BITS(x2, 15, 8));
}

void TBaseWeapon::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  int x = 0;

  SET_BITS(x, 7, 8, getCurSharp());
  SET_BITS(x, 15, 8, getMaxSharp());

  *x1 = x;

  SET_BITS(x, 7, 8, getWeapDamLvl());
  SET_BITS(x, 15, 8, getWeapDamDev());

  *x2 = x;

  *x3 = 0;
  *x4 = 0;
}

sstring TBaseWeapon::displayFourValues()
{
  char tString[256];
  int  x1,
       x2,
       x3,
       x4;

  getFourValues(&x1, &x2, &x3, &x4);
  sprintf(tString, "Current values : %d %d %d %d\n\r", x1, x2, x3, x4);
  sprintf(tString + strlen(tString),
          "Current values : Cur-Sh: %d Max-Sh: %d Lvl: %d Dev: %d %d",
          getCurSharp(), getMaxSharp(), getWeapDamLvl(), getWeapDamDev(), x4);

  return tString;
}

int TBaseWeapon::galvanizeMe(TBeing *caster, byte bKnown)
{
  if (getMaxStructPoints() < 0) {
    act("$p is as solid as it is possible.",
         FALSE, caster, this, 0, TO_CHAR);
    act("Nothing seems to happen.", FALSE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  if (caster->bSuccess(bKnown, SPELL_GALVANIZE)) {
    addToMaxStructPoints(1);
    addToStructPoints(1);
    return SPELL_SUCCESS;
  } else {
    if (critFail(caster, SPELL_GALVANIZE)) {
      CF(SPELL_GALVANIZE);
      return SPELL_CRIT_FAIL_2;  // item destroyed
    }
    addToMaxStructPoints(-2);
    addToStructPoints(-2);
    return SPELL_CRIT_FAIL;  // just to distinguish from "nothing happens"
  }
}

double TBaseWeapon::baseDamage() const
{
  double amt = damageLevel() * 1.75;

  // this is from balance details, don't change it
  if (isPaired())
    amt *= 1.1;

  return amt;
}

void TBaseWeapon::sharpenMe(TBeing *ch, TTool *tool)
{
  int sharp_move = dice(2,3);

  ch->addToMove(-sharp_move);
  if (ch->getMove() < 10) {
    act("You are much too tired to continue to sharpen $p.", 
           FALSE, ch, this, tool, TO_CHAR);
    act("$n stops sharpening, and wipes $s brow.", 
           FALSE, ch, this, tool, TO_ROOM);
    ch->stopTask();
    return;
  }
  act("You continue to sharpen $p with $P.", 
           FALSE, ch, this, tool, TO_CHAR);
  tool->addToToolUses(-1);
  if (tool->getToolUses() <= 0) {
    act("Your $o breaks due to overuse.", 
           FALSE, ch, tool, 0, TO_CHAR);
    act("$n looks startled as $e breaks $P while sharpening.", 
           FALSE, ch, this, tool, TO_ROOM);
    ch->stopTask();
    delete tool;
    return;
  }
  if (getMaxSharp() <= getCurSharp()) {
    ch->sendTo("It doesn't seem to be getting any sharper.\n\r");
    act("$n stops sharpening $p.", FALSE, ch, this, 0, TO_ROOM);
    ch->stopTask();
    return;
  }

  if (ch->bSuccess(SKILL_SHARPEN)) 
    addToCurSharp((itemType() == ITEM_ARROW) ? 2 : 1);

  // task can continue forever, so don't bother decrementing the timer
  return;
}

void TBaseWeapon::dullMe(TBeing *ch, TTool *tool)
{
  int blunt_move = dice(2,3);

  ch->addToMove(-blunt_move);
  if (ch->getMove() < 10) {
    act("You are much too tired to continue to blunt $p.", 
              FALSE, ch, this, tool, TO_CHAR);
    act("$n stops blunting $p, and wipes $s brow.", 
              FALSE, ch, this, tool, TO_ROOM);
    ch->stopTask();
    return;
  }
  act("You continue to blunt $p with $P.", FALSE, ch, this, tool, TO_CHAR);
  tool->addToToolUses(-1);
  if (tool->getToolUses() <= 0) {
    act("Your $o breaks due to overuse.", false, ch, tool, 0, TO_CHAR);
    act("$n looks startled as $e breaks $P while smoothing.", 
             FALSE, ch, this, tool, TO_ROOM);
    ch->stopTask();
    delete tool;
    return;
  }
  if (getMaxSharp() <= getCurSharp()) {
    ch->sendTo("It doesn't seem to be getting any smoother.\n\r");
    act("$n stops blunting $p.", FALSE, ch, this, 0, TO_ROOM);
    ch->stopTask();
    return;
  }
  if (ch->bSuccess(SKILL_DULL)) 
    addToCurSharp(1);

  // task can continue forever, so don't bother decrementing the timer
  return;
}

int TBaseWeapon::sharpenPrice() const
{
  int cost = obj_flags.cost;
  cost *= 5;
  cost /= 100;

  cost = max(cost, 1);
  if (!getMaxSharp())
    return cost;
  else {
    cost *= getMaxSharp() - getCurSharp();
    cost /= getMaxSharp();
  }

  cost = max(cost, 1);
  return cost;
}

int TBaseWeapon::sharpenerValueMe(const TBeing *ch, TMonster *me) const
{
  int cost;

  if (getCurSharp() == getMaxSharp()) {
    me->doTell(ch->getName(), "This weapon is perfectly ok!");
    return TRUE;
  }
  cost = sharpenPrice();

  me->doTell(ch->getName(), fmt("It will cost %d talens to totally %s your %s.") %
	     cost % (isBluntWeapon() ? "dull" : "sharpen") %
	     fname(name));
  return TRUE;
}

int TBaseWeapon::sharpenerGiveMe(TBeing *ch, TMonster *me)
{
  char buf[256];
  int cost;
  sharp_struct *job;

  if (getCurSharp() == getMaxSharp()) {
    me->doTell(ch->getName(), "That item is perfectly ok!");
    strcpy(buf, name);
    strcpy(buf, add_bars(buf).c_str());
    sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
    me->doGive(buf, GIVE_FLAG_IGN_DEX_TEXT);
    return TRUE;
  }
  cost = sharpenPrice();

  if (ch->getMoney() < cost) {
    me->doTell(ch->getName(), "I have to make a living! If you don't have the talens , I don't do the work!");
    strcpy(buf, name);
    strcpy(buf, add_bars(buf).c_str());
    sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
    me->doGive(buf);
    return TRUE;
  }
  // Now we have a weapon that can be sharpened
  job = static_cast<sharp_struct *>(me->act_ptr);
  if (!job->wait) {
#if 0
    act("You give $p to $N to be re-edged.", FALSE, ch, this, me, TO_CHAR);
    act("$n gives $p to $N to be re-edged.", FALSE, ch, this, me, TO_ROOM);
#endif
    job->wait = max((int) (getMaxSharp() - getCurSharp()) ,  (int) (getCurSharp() - getMaxSharp()));
    job->wait /= 15;
    job->wait += 1;   // gotta exit with at least 1
    sprintf(buf, "Thanks for your business, I'll take your %d talen%s payment in advance!", cost, (cost > 1) ? "s" : "");
    me->doSay(buf);
    ch->giveMoney(me, cost, GOLD_REPAIR);
    shoplog(find_shop_nr(me->number), ch, me, getName(), 
	    cost, "sharpening");
    me->saveChar(ROOM_AUTO_RENT);
    ch->saveChar(ROOM_AUTO_RENT);



    job->cost = cost;
    job->char_name = mud_str_dup(ch->getName());
    job->obj_name = mud_str_dup(fname(name));
    --(*this);
    *me += *this;
    job->isBlunt = isBluntWeapon();
    setCurSharp(getMaxSharp());
    return TRUE;
  } else {
    if (!job->char_name)
      vlogf(LOG_PROC, fmt("somehow sharpener %s didnt have a name on existing job") %  me->getName());
    sprintf(buf, "Sorry, %s, but you'll have to wait while I re-edge %s's weapon.", ch->getName(), job->char_name);
    me->doSay(buf);
    strcpy(buf, name);
    strcpy(buf, add_bars(buf).c_str());
    sprintf(buf + strlen(buf), " %s", fname(ch->name).c_str());
    me->doGive(buf);
    if (parent == me) {
      // give back failed
      me->doDrop("", this);
    }
    return TRUE;
  }
  return FALSE;
}

void TBaseWeapon::objMenu(const TBeing *ch) const
{
  ch->sendTo(fmt(VT_CURSPOS) % 3 % 1);
  ch->sendTo(fmt("%sSuggested price:%s %d%s") % ch->purple() % ch->norm() %
                 suggestedPrice() % (suggestedPrice() != obj_flags.cost ? " *" : ""));

  ch->sendTo(fmt(VT_CURSPOS) % 4 % 1);
  ch->sendTo(fmt("%sReal Lvl:%s %.2f") %
        ch->purple() % ch->norm() % weaponLevel());
                 
  ch->sendTo(fmt(VT_CURSPOS) % 4 % 18);
  ch->sendTo(fmt("%sDam Lvl:%s %.2f") %
        ch->purple() % ch->norm() % damageLevel());
                 
  ch->sendTo(fmt(VT_CURSPOS) % 4 % 32);
  ch->sendTo(fmt("%sStr Lvl:%s %.2f") %
        ch->purple() % ch->norm() % structLevel());
                 
  ch->sendTo(fmt(VT_CURSPOS) % 4 % 50);
  ch->sendTo(fmt("%sQual Lvl:%s %.2f") %
        ch->purple() % ch->norm() % sharpLevel());
}

void TBaseWeapon::changeObjValue1(TBeing *ch)
{
  if (ch->hasWizPower(POWER_OEDIT_WEAPONS)) {
    ch->specials.edit = CHANGE_WEAPON_VALUE1;
    change_weapon_value1(ch, this, "", ENTER_CHECK);
  } else {
    ch->sendTo("You lack the power to edit weapon damage.\n\r");
  }
  return;
}

void TBaseWeapon::changeObjValue2(TBeing *ch)
{
  int x1, x2, x3, x4;
  getFourValues(&x1, &x2, &x3, &x4);

  if (!ch->hasWizPower(POWER_OEDIT_WEAPONS)) {
    ch->sendTo("You lack the power to edit weapon damage.\n\r");
    return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("What does this value do? :\n\r %s\n\r") %
        ItemInfo[itemType()]->val1_info);
  ch->specials.edit = CHANGE_OBJ_VALUE2;

  ch->sendTo(fmt("Value 2 for %s : %d\n\r\n\r") %
       sstring(getName()).uncap() % x2);
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter new value.\n\r--> ");
}

void TBaseWeapon::changeObjValue3(TBeing *ch)
{
  int x1, x2, x3, x4;
  getFourValues(&x1, &x2, &x3, &x4);

  if (!ch->hasWizPower(POWER_OEDIT_WEAPONS)) {
    ch->sendTo("You lack the power to edit weapon damage.\n\r");
    return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("What does this value do? :\n\r %s\n\r") %
        ItemInfo[itemType()]->val2_info);
  ch->specials.edit = CHANGE_OBJ_VALUE3;

  ch->sendTo(fmt("Value 3 for %s : %d\n\r\n\r") %
       sstring(getName()).uncap() % x3);
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter new value.\n\r--> ");
}

int TBaseWeapon::damageMe(TBeing *ch, TBeing *v, wearSlotT part_hit)
{
  int hardness;
  char buf[256];
  TThing *tt;

  tt = v->equipment[part_hit];
  TObj *item = dynamic_cast<TObj *>(tt);

  if (item) {
    if (item->getMaxStructPoints() < 0)
      hardness = material_nums[item->getMaterial()].hardness;
    else if (item->getMaxStructPoints() != 0)
      hardness = material_nums[item->getMaterial()].hardness *
          item->getStructPoints() / item->getMaxStructPoints();
    else
      hardness = 0;
  } else {
    int maxlim = v->getMaxLimbHealth(part_hit);
    if (maxlim)
      hardness = material_nums[v->getMaterial()].hardness *
          v->getCurLimbHealth(part_hit) / maxlim;
    else
      hardness = 0;
  }
  // Check to see if it gets dulled.
  int sharp = getCurSharp();

  // this hardness check will be made for ALL types of weapon damage
  // both blunting and structural
  if ((::number(WEAPON_DAM_MIN_HARDNESS, WEAPON_DAM_MAX_HARDNESS) <= hardness) ||
      (::number(WEAPON_DAM_MIN_HARDNESS, WEAPON_DAM_MAX_HARDNESS) <= hardness)) {
    if (sharp &&
          (::number(0, WEAPON_DAM_MAX_SHARP) <= sharp)) {
      if (isBluntWeapon()) {
        // The blunter the weapon, the easier to chip a bit - bat
        sprintf(buf, "Your %s%s%s is %schipped%s by %s$n's %s.",
              ch->blue(), fname(name).c_str(), ch->norm(),
              ch->green(), ch->norm(), (item ? "$p on " : ""),
              v->describeBodySlot(part_hit).c_str());
      } else if (isPierceWeapon()) {
        // The pointier the weapon, the easier to blunt a bit - bat
        sprintf(buf, "Your %s%s%s is %sblunted%s by %s$n's %s.",
              ch->blue(), fname(name).c_str(), ch->norm(),
              ch->green(), ch->norm(), (item ? "$p on " : ""),
              v->describeBodySlot(part_hit).c_str());
      } else {
        // The sharper the weapon, the easier to notched a bit - Russ
        sprintf(buf, "Your %s%s%s is %snotched%s by %s$n's %s.",
        ch->blue(), fname(name).c_str(), ch->norm(),
              ch->green(), ch->norm(), (item ? "$p on " : ""),
              v->describeBodySlot(part_hit).c_str());
      }
      act(buf, TRUE, v, item, ch, TO_VICT);
      ch->sendTo(COLOR_OBJECTS, fmt("It is in %s condition.\n\r") %equip_condition(-1));
      addToCurSharp(-1);
    }
    // Check for structural damage
    int chance = 1000 - (int) (1000 * gold_modifier[GOLD_REPAIR].getVal());
    if (::number(0,999) >= chance) {
      // NOTE: this makes it easier to damage an item that is very damaged already
      if (::number(0, getMaxStructPoints()) >= getStructPoints()) {
        addToStructPoints(-1);
        if (getStructPoints() <= 0) {
          makeScraps();
          return DELETE_ITEM;
        } else {
          sprintf(buf, "%s%s%s is %sdamaged%s by %s$N's %s.",
               ch->purple(), sstring(getName()).cap().c_str(), ch->norm(),
             ch->red(), ch->norm(), (item ? "$p on " : ""),
               v->describeBodySlot(part_hit).c_str());
          act(buf, FALSE, ch, item, v, TO_CHAR);
	  ch->sendTo(COLOR_OBJECTS, fmt("<R>It is in<1> %s <R>condition.<1>\n\r") %equip_condition(-1));
        }
      }
    }
  }
  return FALSE;
}

int TBaseWeapon::swungObjectDamage(const TBeing *ch, const TBeing *v) const
{
  double dam;

  dam = baseDamage();
  double flux = dam * getWeapDamDev() / 10;

  int iflux = ::number((int) -flux, (int) flux);
  dam += iflux;

  dam += ch->extraDam(v, this);
  return (int) dam;
}


spellNumT getWtype_kluge(weaponT t)
{
  switch (t) {
    case WEAPON_TYPE_NONE:
      return TYPE_SMITE;
    case WEAPON_TYPE_STAB:
      return TYPE_STAB;
    case WEAPON_TYPE_WHIP:
      return TYPE_WHIP;
    case WEAPON_TYPE_SLASH:
      return TYPE_SLASH;
    case WEAPON_TYPE_SMASH:
      return TYPE_SMASH;
    case WEAPON_TYPE_CLEAVE:
      return TYPE_CLEAVE;
    case WEAPON_TYPE_CRUSH:
      return TYPE_CRUSH;
    case WEAPON_TYPE_BLUDGEON:
      return TYPE_BLUDGEON;
    case WEAPON_TYPE_CLAW:
      return TYPE_CLAW;
    case WEAPON_TYPE_BITE:
      return TYPE_BITE;
    case WEAPON_TYPE_STING:
      return TYPE_STING;
    case WEAPON_TYPE_PIERCE:
      return TYPE_PIERCE;
    case WEAPON_TYPE_PUMMEL:
      return TYPE_PUMMEL;
    case WEAPON_TYPE_FLAIL:
      return TYPE_FLAIL;
    case WEAPON_TYPE_BEAT:
      return TYPE_BEAT;
    case WEAPON_TYPE_THRASH:
      return TYPE_THRASH;
    case WEAPON_TYPE_THUMP:
      return TYPE_THUMP;
    case WEAPON_TYPE_WALLOP:
      return TYPE_WALLOP;
    case WEAPON_TYPE_BATTER:
      return TYPE_BATTER;
    case WEAPON_TYPE_STRIKE:
      return TYPE_STRIKE;
    case WEAPON_TYPE_CLUB:
      return TYPE_CLUB;
    case WEAPON_TYPE_SLICE:
      return TYPE_SLICE;
    case WEAPON_TYPE_POUND:
      return TYPE_POUND;
    case WEAPON_TYPE_THRUST:
      return TYPE_THRUST;
    case WEAPON_TYPE_SPEAR:
      return TYPE_SPEAR;
    case WEAPON_TYPE_SMITE:
      return TYPE_SMITE;
    case WEAPON_TYPE_BEAK:
      return TYPE_BEAK;
    case WEAPON_TYPE_AIR:
      return TYPE_AIR;
    case WEAPON_TYPE_EARTH:
      return TYPE_EARTH;
    case WEAPON_TYPE_FIRE:
      return TYPE_FIRE;
    case WEAPON_TYPE_WATER:
      return TYPE_WATER;
    case WEAPON_TYPE_BEAR_CLAW:
      return TYPE_BEAR_CLAW;
    case WEAPON_TYPE_SHOOT:
      return TYPE_SHOOT;
    default:
      return TYPE_HIT;
  }
}



bool TBaseWeapon::isBluntWeapon() const
{
  int count=0, total=0;
  const TGenWeapon *tgw;

  if((tgw=dynamic_cast<const TGenWeapon *>(this))){
    for(int i=0;i<3;++i){
      if(bluntType(getWtype_kluge(tgw->getWeaponType(0))))
	count+=tgw->getWeaponFreq(0);
      total+=tgw->getWeaponFreq(0);
    }
    // need at least 2/3 blunt types, for TGenWeapons

    if(count > (total/3.0*2.0))
      return true;
  }  

  // not a TGenWeapon
  return bluntType(getWtype());
}

bool TBaseWeapon::isSlashWeapon() const
{
  spellNumT wtype = getWtype();
  int count=0, total=0;
  const TGenWeapon *tgw;

  if((tgw=dynamic_cast<const TGenWeapon *>(this))){
    for(int i=0;i<3;++i){
      if(slashType(getWtype_kluge(tgw->getWeaponType(0))))
	count+=tgw->getWeaponFreq(0);
      total+=tgw->getWeaponFreq(0);
    }
    // need at least 2/3 type, for TGenWeapons

    if(count > (total/3.0*2.0))
      return true;
  }  

  // not a TGenWeapon
  return (slashType(wtype));
}

bool TBaseWeapon::isPierceWeapon() const
{
  spellNumT wtype = getWtype();
  int count=0, total=0;
  const TGenWeapon *tgw;

  if((tgw=dynamic_cast<const TGenWeapon *>(this))){
    for(int i=0;i<3;++i){
      if(pierceType(getWtype_kluge(tgw->getWeaponType(0))))
	count+=tgw->getWeaponFreq(0);
      total+=tgw->getWeaponFreq(0);
    }
    // need at least 2/3 type, for TGenWeapons

    if(count > (total/3.0*2.0))
      return true;
  }  

  // not a TGenWeapon
  return (pierceType(wtype));
}

void TBaseWeapon::divinateMe(TBeing *caster) const
{
#if 1
  caster->sendTo(fmt("It is capable of doing %s of damage for your level.\n\r") % 
          describe_damage((int) damageLevel(), caster));
#else
  caster->sendTo(fmt("It is capable of doing %s of damage.\n\r") % 
          describe_damage((int) damageLevel(), caster));
#endif
}

int TBaseWeapon::enhanceMe(TBeing *caster, int level, byte bKnown)
{
  int i;

  if (isObjStat(ITEM_MAGIC)) {
    caster->sendTo("Uhh, you can't enhance weapons that are already enchanted...\n\r");
    act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
    return SPELL_FAIL;
  }

  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
    if (affected[i].location != APPLY_NONE) {
      caster->sendTo("That item is too powerful for this spell to work on it.\n\r");
      act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
      return SPELL_FAIL;
    }
  }

  if (caster->bSuccess(bKnown, SPELL_ENHANCE_WEAPON)) {
    affected[0].location = APPLY_HITROLL;
    affected[0].modifier = 1;
    if (level > 20)
      affected[0].modifier += 1;
    if (level > 40)
      affected[0].modifier += 1;

    if (level > 10) {
      affected[1].location = APPLY_DAMROLL;
      affected[1].modifier = 1;
    }
    if (level > 30)
      affected[1].modifier += 1;
    if (level > 49)
      affected[1].modifier += 1;

    addObjStat(ITEM_MAGIC);
    addObjStat(ITEM_GLOW);

    addGlowEffects();

    switch (critSuccess(caster, SPELL_ENHANCE_WEAPON)) {
      case CRIT_S_KILL:
        CS(SPELL_ENHANCE_WEAPON);
        affected[0].modifier += 1;
//        affected[1].modifier += 1;
//        vlogf(LOG_MISC, fmt("Somehow, %s just got a critical success on enchant weapon") %  caster->getName());
        break;
      case CRIT_S_TRIPLE:
      case CRIT_S_DOUBLE:
      case CRIT_S_NONE:
        break;
    }

#if 0
    // competing philosophies here:
    // suggested price on a weapon with magic is MUCH higher then
    // non-magic weapon
    // We don't want mage PCs casting enhance to raise cost of weapon,
    // then turning around and selling the weapon to make money
    // but enchanted weapon SHOULD cost (and rent for) more than
    // the non-magic variety

    // enhance, dispel, enhance, dispel, ... sell for mega money!
    // don't do this
    obj_flags.cost *= 2;
#endif

    return SPELL_SUCCESS;
  } else {
    switch (critFail(caster, SPELL_ENHANCE_WEAPON)) {
      case CRIT_F_HITSELF:
        CF(SPELL_ENHANCE_WEAPON);
        addObjStat(ITEM_NODROP);
        curseMe();
        return SPELL_CRIT_FAIL;
      case CRIT_F_HITOTHER:
        CF(SPELL_ENHANCE_WEAPON);
        return SPELL_CRIT_FAIL_2;  // delete the obj
      default:
        caster->sendTo("Nothing seems to happen.\n\r");
        act("Nothing seems to happen.", TRUE, caster, 0, 0, TO_ROOM);
        break;
    }
    return SPELL_FAIL;
  }
}

void TBaseWeapon::sharpenMeStoneWeap(TBeing *caster, TTool *tool)
{
  if (getCurSharp() >= getMaxSharp()) {
    caster->sendTo("But it looks as sharp as it's going to get!\n\r");
    return;
  }
  if (isBluntWeapon()) {
    caster->sendTo("Generally, that weapon is not something you want sharp.\n\r");
    return;
  }

  if (tool->getToolUses() <= 0) {
    act("But your $o is completely worn out!", false, caster, tool, 0, TO_CHAR);
    return;
  }

  if (caster->getMove() < 10) {
    act("You are much too tired to sharpen $p.", FALSE, caster, this, tool, TO_CHAR);
    return;
  }
  caster->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_SHARPEN, 4);
  act("You start to sharpen $p with $P.", FALSE, caster, this, tool, TO_CHAR);
  act("$n begins sharpening $p with $P.", FALSE, caster, this, tool, TO_ROOM);

  // terminate sitting tasks
  if ((caster->task) && caster->getPosition() <= POSITION_SITTING)
    caster->stopTask();

  start_task(caster, NULL, NULL, TASK_SHARPEN, name, 350, (ushort) caster->in_room, 0, 0, 0);
}

void TBaseWeapon::dullMeFileWeap(TBeing *caster, TTool *tool)
{
  if (getCurSharp() >= getMaxSharp()) {
    caster->sendTo("But it looks as blunt as it's going to get!\n\r");
    return;
  }
  if ((isPierceWeapon()) || (isSlashWeapon())) {
    caster->sendTo("Generally, that weapon is not something you want dull.\n\r");
    return;
  }

  if (tool->getToolUses() <= 0) {
    act("But your $o is completely worn out!", false, caster, tool, 0, TO_CHAR);
    return;
  }
  if (caster->getMove() < 10) {
    act("You are much too tired to dull $p.", FALSE, caster, this, tool, TO_CHAR);
    return;
  }

  caster->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_DULL, 4);
  act("You start to blunt $p with $P.", FALSE, caster, this, tool, TO_CHAR);
  act("$n begins blunting $p with $P.", FALSE, caster, this, tool, TO_ROOM);

  // terminate sitting tasks
  if ((caster->task) && caster->getPosition() <= POSITION_SITTING)
    caster->stopTask();

  start_task(caster, NULL, NULL, TASK_DULL, name, 350, (ushort) caster->in_room, 0, 0, 0);
}

void TBaseWeapon::changeBaseWeaponValue1(TBeing *ch, const char *arg, editorEnterTypeT type)
{
  int update_num;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = CHANGE_OBJ_VALUES;
      change_obj_values(ch, this, "", ENTER_CHECK);
      return;
    }
  }
  update_num = convertTo<int>(arg);

  switch (ch->specials.edit) {
    case CHANGE_WEAPON_VALUE1:
      switch (update_num) {
        case 1:
          ch->sendTo(VT_HOMECLR);
          ch->sendTo(fmt("Current max %s: %d\n\r") %
                ((isBluntWeapon() ? "bluntness" :
                 (isPierceWeapon() ? "pointiness" :
                 "sharpness"))) %
                getMaxSharp());
          ch->sendTo(fmt("Enter new max %s.\n\r--> ") %
                ((isBluntWeapon() ? "bluntness" :
                 (isPierceWeapon() ? "pointiness" :
                 "sharpness"))));
          ch->specials.edit = CHANGE_WEAPON_MAX_SHARP;
          return;
        case 2:
          ch->sendTo(VT_HOMECLR);
          ch->sendTo(fmt("Current %s: %d\n\r") %
                ((isBluntWeapon() ? "bluntness" :
                 (isPierceWeapon() ? "pointiness" :
                 "sharpness"))) %
                 getCurSharp());
          ch->sendTo(fmt("Enter new %s.\n\r--> ") %
                ((isBluntWeapon() ? "bluntness" :
                 (isPierceWeapon() ? "pointiness" :
                 "sharpness"))));
          ch->specials.edit = CHANGE_WEAPON_SHARP;
          return;
      }
      break;
    case CHANGE_WEAPON_MAX_SHARP:
      if (type != ENTER_CHECK) {
        if ((update_num > 100) || (update_num < 0)) {
          ch->sendTo("Please enter a number from 0-100.\n\r");
          return;
        }
        setMaxSharp(update_num);
        ch->specials.edit = CHANGE_WEAPON_VALUE1;
        change_weapon_value1(ch, this, "", ENTER_CHECK);
        return;
      }
    case CHANGE_WEAPON_SHARP:
      if (type != ENTER_CHECK) {
        if ((update_num > 100) || (update_num < 0)) {
          ch->sendTo("Please enter a number from 0-100.\n\r");
          return;
        }
        setCurSharp(update_num);
        ch->specials.edit = CHANGE_WEAPON_VALUE1;
        change_weapon_value1(ch, this, "", ENTER_CHECK);
        return;
      }
    default:
      return;
  }
  ch->sendTo(VT_HOMECLR);
  if (isSlashWeapon()) {
    ch->sendTo(fmt("1) Max sharpness (Maximum sharpness item can ever be):     Max    =%d\n\r") % getMaxSharp());
    ch->sendTo(fmt("2) Sharpness (sharpness that weapon will start out with):  Current=%d\n\r") % getCurSharp());
  } else if (isBluntWeapon()) {
    ch->sendTo(fmt("1) Max Bluntness (Maximum bluntness item can ever be):     Max    =%d\n\r") % getMaxSharp());
    ch->sendTo(fmt("2) Bluntness (sharpness that weapon will start out with):  Current=%d\n\r") % getCurSharp());
  } else {
    ch->sendTo(fmt("1) Max Pointiness (Maximum pointiness item can ever have):   Max    =%d\n\r") % getMaxSharp());
    ch->sendTo(fmt("2) Pointiness (pointiness that weapon will start out with):  Current=%d\n\r") % getCurSharp());
  }
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter your choice to modify.\n\r--> ");
}

int TGenWeapon::smiteWithMe(TBeing *ch, TBeing *v)
{
  affectedData aff;
  byte bKnown = ch->getSkillValue(SKILL_SMITE);

  if ((objVnum() != WEAPON_AVENGER1) &&
      (objVnum() != WEAPON_AVENGER2) &&
      (objVnum() != WEAPON_AVENGER3)) {
    ch->sendTo(COLOR_OBJECTS, fmt("%s has no respect for someone using %s.\n\r") %
        sstring(ch->yourDeity(SKILL_SMITE, FIRST_PERSON)).cap() % getName());
    return FALSE;
  }

  if (ch->checkForSkillAttempt(SKILL_SMITE)) {
    ch->sendTo("You are not yet prepared to smite.\n\r");
    return FALSE;
  }

  if (ch->affectedBySpell(SKILL_SMITE)) {
    ch->sendTo("Your recent smite prevents you from smiting again at this time.\n\r");
    return FALSE;
  }
  int dam = ch->getSkillDam(v, SKILL_SMITE, ch->getSkillLevel(SKILL_SMITE), ch->getAdvLearning(SKILL_SMITE));

  if (!ch->isOppositeFaction(v)) {
    SV(SKILL_SMITE);
    dam /= 2;
  }

  aff.type = AFFECT_SKILL_ATTEMPT;
  aff.duration = 2 * UPDATES_PER_MUDHOUR;
  aff.modifier = SKILL_SMITE;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;
  ch->affectTo(&aff, -1);

  if (!ch->bSuccess(bKnown, SKILL_SMITE)) {
    act("You call upon $d to smite $N, but $d does not heed your plea!",
             FALSE, ch, 0, v, TO_CHAR);
    act("$n calls upon $d to smite $N, but $d does not heed $m.",
             FALSE, ch, 0, v, TO_NOTVICT);
    act("$n calls upon $d to smite you, but $d does not heed $m!",
             FALSE, ch, 0, v, TO_VICT);

    // man, this would REALLY piss me off
    TMonster *tmon = dynamic_cast<TMonster *>(v);
    if (tmon)
      tmon->developHatred(ch);

    ch->reconcileDamage(v, 0, SKILL_SMITE);

    return TRUE;
  }

  aff.type = SKILL_SMITE;
  aff.duration = 30 * UPDATES_PER_MUDHOUR;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;
  ch->affectTo(&aff, -1);

  act("You call upon $d to smite $N!",
             FALSE, ch, 0, v, TO_CHAR);
  act("$n calls upon $d to smite $N!",
             FALSE, ch, 0, v, TO_NOTVICT);
  act("$n calls upon $d to smite you!",
             FALSE, ch, 0, v, TO_VICT);
  act("A bolt of fierce blue-white energy courses from $p into $M!",
             FALSE, ch, this, v, TO_CHAR);
  act("A bolt of fierce blue-white energy courses from $p into $M!",
             FALSE, ch, this, v, TO_NOTVICT);
  act("A bolt of fierce blue-white energy courses from $p into you!!!",
             FALSE, ch, this, v, TO_VICT);

  if (ch->reconcileDamage(v, dam, SKILL_SMITE) == -1)
    return DELETE_VICT;

  // man, this would REALLY piss me off
  TMonster *tmon = dynamic_cast<TMonster *>(v);
  if (tmon)
    tmon->developHatred(ch);

  return TRUE;
}

int TBaseWeapon::poisonWeaponWeapon(TBeing *ch, TThing *poison)
{
  int rc;

  if (isBluntWeapon()) {
    ch->sendTo("Blunt weapons can't be poisoned effectively.\n\r");
    return FALSE;
  }
  if(isPoisoned()){
    ch->sendTo("That is already poisoned!\n\r");
    return FALSE;
  }

  rc = poison->poisonMePoison(ch, this);
  return rc;
}

void TBaseWeapon::curseMe()
{
  // LOWER ATTACK DICE BY -1
  if (getWeapDamLvl() > 0)
    setWeapDamLvl(getWeapDamLvl() - 1);
}

int TBaseWeapon::wieldMe(TBeing *ch, char *arg2)
{
  // Test code to flux 'Two-Handed' weapons based on size.
  if (gamePort != PROD_GAMEPORT) {
    bool canSingleWieldPrim = ch->checkWeaponWeight(this, HAND_TYPE_PRIM, false),
         canSingleWieldSecd = ch->checkWeaponWeight(this, HAND_TYPE_SEC , false);

    if (isname(name, "[paired]"))
      canSingleWieldPrim = canSingleWieldSecd = false;

#if 0
    vlogf(LOG_LAPSOS, fmt("Dynamic Paired Code Active: %s %d %d") % 
          arg2 % canSingleWieldPrim % canSingleWieldSecd);
#endif

    if (!*arg2) {
      if (!canSingleWieldPrim)
        addObjStat(ITEM_PAIRED);
      else
        remObjStat(ITEM_PAIRED);

      int nRc = ch->wear(this, WEAR_KEY_HOLD, ch);

      if (IS_SET_DELETE(nRc, DELETE_ITEM))
        return DELETE_THIS;
    } else {
      int nRc = 0;

      if (is_abbrev(arg2, "right")) {
        if ((ch->getPrimaryHold() == HOLD_RIGHT ? canSingleWieldPrim : canSingleWieldSecd))
          remObjStat(ITEM_PAIRED);
        else
          addObjStat(ITEM_PAIRED);

        nRc = ch->wear(this, WEAR_KEY_HOLD_R, ch);
      } else if (is_abbrev(arg2, "left")) {
        if ((ch->getPrimaryHold() == HOLD_LEFT ? canSingleWieldPrim : canSingleWieldSecd))
          remObjStat(ITEM_PAIRED);
        else
          addObjStat(ITEM_PAIRED);

        nRc = ch->wear(this, WEAR_KEY_HOLD_L, ch);
      } else if (is_abbrev(arg2, "both")) {
        addObjStat(ITEM_PAIRED);

        nRc = ch->wear(this, WEAR_KEY_HOLD, ch);
      } else {
        ch->sendTo("Syntax: Wield <object> <\"right\"/\"left\"/\"both\">");
        return FALSE;
      }

      if (IS_SET_DELETE(nRc, DELETE_ITEM))
        return DELETE_THIS;
    }

    return FALSE;
  }

  if (!*arg2) {
    int rc = ch->wear(this, WEAR_KEY_HOLD, ch);
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_THIS;
  } else {
    if (!isPaired()) {
      int rc = 0;
      if (is_abbrev(arg2, "right"))
        rc = ch->wear(this, WEAR_KEY_HOLD_R, ch);
      else if (is_abbrev(arg2, "left"))
        rc = ch->wear(this, WEAR_KEY_HOLD_L, ch);
      else {
        ch->sendTo("Syntax : Wield <object> right | left\n\r");
        return 0;
      }
      if (IS_SET_DELETE(rc, DELETE_ITEM))
        return DELETE_THIS;
    } else {
      ch->sendTo("That is a two handed weapon. It has to be wielded in both hands.\n\r");
      ch->sendTo("To wield it, clear both hands and type wield <sword name>\n\r");
      return 0;
    }
  }
  return 0;
}

int TBaseWeapon::expelPrice(const TBeing *, int pos) const
{
  int dam;

  dam = (int) baseDamage();

  if ((pos == WEAR_HEAD) || (pos == WEAR_NECK) || (pos == WEAR_BODY))
    return (dam * 50);
  else
    return (dam * 20);
}

int TBaseWeapon::suggestedPrice() const
{
  int price = 0;

  double weapon_lev = weaponLevel();

  // plug level into price formula
  // this formula is derived in balance document
  price = (int) (weapon_lev * max(weapon_lev, 20.0) * 30.75);

  int tohit = 0, todam = 0;
  for (int i = 0;i < MAX_OBJ_AFFECT;i++) {
    int num = affected[i].modifier;
    num = max(0,num);
    switch (affected[i].location) {
      case APPLY_HITROLL:
        tohit += num;
        break;
      case APPLY_DAMROLL:
        todam += num;
        break;
      case APPLY_HITNDAM:
        tohit += num;
        todam += num;
        break;
      case APPLY_NONE:
        break;
      case APPLY_STR:
      case APPLY_BRA:
      case APPLY_DEX:
      case APPLY_AGI:
      case APPLY_CON:
      case APPLY_INT:
      case APPLY_WIS:
      case APPLY_FOC:
      case APPLY_PER:
      case APPLY_CHA:
      case APPLY_SPE:
      case APPLY_KAR:
        price += 175 * num;
        break;
      case APPLY_AGE:
        price += 2000 * num;
        break;
      case APPLY_HIT:
      case APPLY_MANA:
        price += 800 * num;
        break;
      case APPLY_MOVE:
        price += 300 * num;
        break;
      case APPLY_CHAR_HEIGHT:
      case APPLY_CHAR_WEIGHT:
        price += 50 * num;
        break;
      case APPLY_ARMOR:
        price -= 100 * num;
        break;
#if 0
      // only way weapon can have light is to be glow/shadow
      // since those are applied at load time, means that suggestedPrice
      // would change during the load process, so don't do this, check
      // for glow/shadow directly
      case APPLY_LIGHT:
        price += 100 * num * num;
        break;
#endif
      case APPLY_NOISE:   // negative = good
        price -= 250 * affected[i].modifier;
        break;
      case APPLY_CAN_BE_SEEN:
        price += 1000 * num;
        break;
      case APPLY_VISION:
        price += 3775 * num;
        break;
      default:
        break;
    }
  }
  if (tohit) {
    // this formula is from balance notes
    int amt  = (int) (weapon_lev * max(weapon_lev, 20.0) * 450/4);
        amt -= (int) ((weapon_lev-tohit) * max(weapon_lev-tohit, 20.0) * 450/4);
    price += amt;
  }
  switch (todam) {
    case 1:
      price += 6000;
      break;
    case 2:
      price += 24000;
      break;
    case 3:
      price += 48000;
      break;
    case 4:
      price += 75000;
      break;
    case 5:
      price += 130000;
      break;
    default:
      break;
  }

  if (isObjStat(ITEM_GLOW) || isObjStat(ITEM_SHADOWY)) {
    // this is the amount of light added/subtracted
    int num = 1 + getVolume()/3000;
    num *= 100;  // cost per unit of light
    price += num;
  }

  return price;
}

void TBaseWeapon::evaluateMe(TBeing *ch) const
{
  int learn = ch->getSkillValue(SKILL_EVALUATE);
  const TGenWeapon *tgen = dynamic_cast<const TGenWeapon *>(this);

  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_EVALUATE, 10);

  ch->describeNoise(this, learn);

  if (learn > 5)
    ch->describeMaxSharpness(this, learn);

  if (tgen && learn > 10)
    ch->describeOtherFeatures(tgen, learn);

  if (learn > 20)
    ch->describeMaxStructure(this, learn);

  if (learn > 35)
    ch->describeWeaponDamage(this, learn);

  if (ch->isImmortal()) {
    ch->sendTo(COLOR_OBJECTS, fmt("IMMORTAL EVAL: %s overall is rated as a L%5.2f weapon.\n\r") % getName() % weaponLevel());
    ch->sendTo(COLOR_OBJECTS, fmt("IMMORTAL EVAL: %s damage is rated as L%5.2f.\n\r") % getName() % damageLevel());
    ch->sendTo(COLOR_OBJECTS, fmt("IMMORTAL EVAL: %s structure is rated as L%5.2f.\n\r") % getName() % structLevel());
    ch->sendTo(COLOR_OBJECTS, fmt("IMMORTAL EVAL: %s quality is rated as L%5.2f.\n\r") % getName() % sharpLevel());
  }
}

void TBaseWeapon::describeObjectSpecifics(const TBeing *ch) const
{
  ch->sendTo(COLOR_OBJECTS, fmt("You can tell that %s.\n\r") % ch->describeSharpness(this));
}

sstring TBaseWeapon::describeMySharp(const TBeing *ch) const
{
  char buf[256];
  char sharpbuf[80];
  sstring capbuf;

  if (isBluntWeapon()) {
    return ch->describeBluntness(this);
  } else if (isPierceWeapon()) {
    return ch->describePointiness(this);
  }
  int maxsharp = getMaxSharp();
  int sharp = getCurSharp();
  double diff;
  if (!maxsharp)
    diff = (double) 0;
  else
    diff = (double) ((double) sharp / (double) maxsharp);
  capbuf = colorString(ch, ch->desc, ch->objs(this), NULL, COLOR_OBJECTS, TRUE);
  if (diff <= .02)
    strcpy(sharpbuf, "is totally notched and dull");
  else if (diff < .10)
    strcpy(sharpbuf, "is virtually notched and dull now");
  else if (diff < .30)
    strcpy(sharpbuf, "has very little sharpness left");
  else if (diff < .50)
    strcpy(sharpbuf, "is starting to get notched");
  else if (diff < .70)
    strcpy(sharpbuf, "has some sharpness");
  else if (diff < .90)
    strcpy(sharpbuf, "has a good bit of sharpness");
  else if (diff < 1.0)
    strcpy(sharpbuf, "has almost all of its edge left");
  else
    strcpy(sharpbuf, "is completely sharp");

  sprintf(buf, "%s %s", capbuf.uncap().c_str(), sharpbuf);
  return buf;
}

void TBaseWeapon::descMaxStruct(const TBeing *ch, int learn) const
{
  char capbuf[80];

  if (!ch->hasClass(CLASS_RANGER) && !ch->hasClass(CLASS_WARRIOR) &&
      !ch->hasClass(CLASS_DEIKHAN))
    learn /= 3;

  int maxstruct = GetApprox(getMaxStructPoints(), learn);

  strcpy(capbuf, ch->objs(this));
  ch->sendTo(COLOR_OBJECTS,fmt("%s seems to %s.\n\r") %
           sstring(capbuf).cap() %
          ((maxstruct >= 99) ? "be virtually indestructible" :
           ((maxstruct >= 95) ? "be very durable" :
           ((maxstruct >= 91) ? "be durable" :
           ((maxstruct >= 87) ? "be fairly durable" :
           ((maxstruct >= 83) ? "be incredibly sturdy" :
           ((maxstruct >= 79) ? "be very sturdy" :
           ((maxstruct >= 75) ? "be sturdy" :
           ((maxstruct >= 71) ? "be somewhat sturdy" :
           ((maxstruct >= 67) ? "be fairly sturdy" :
           ((maxstruct >= 63) ? "be very substantial" :
           ((maxstruct >= 59) ? "be substantial" :
           ((maxstruct >= 55) ? "be somewhat substantial" :
           ((maxstruct >= 51) ? "be very well-constructed" :
           ((maxstruct >= 47) ? "be well-constructed" :
           ((maxstruct >= 43) ? "be fairly well-constructed" :
           ((maxstruct >= 39) ? "be incredibly rugged" :
           ((maxstruct >= 35) ? "be rugged" :
           ((maxstruct >= 31) ? "be somewhat rugged" :
           ((maxstruct >= 27) ? "be very strong" :
           ((maxstruct >= 23) ? "be strong" :
           ((maxstruct >= 19) ? "be somewhat strong" :
           ((maxstruct >= 15) ? "be fairly flimsy" :
           ((maxstruct >= 11) ? "be somewhat flimsy" :
           ((maxstruct >= 7) ? "be flimsy" :
           ((maxstruct >= 3) ? "be very flimsy" :
           ((maxstruct >= 0) ? "be incredibly flimsy" :
                          "be indestructible")))))))))))))))))))))))))));
}

void TBaseWeapon::specializationCheck(TBeing *ch, float *fx)
{
  int skill;

  if (isSlashWeapon() && ch->getDiscipline(DISC_SLASH))
    skill = ch->getSkillValue(SKILL_SLASH_SPEC) - 20*ch->drunkMinus();
  else if (isPierceWeapon() && ch->getDiscipline(DISC_PIERCE))
    skill = ch->getSkillValue(SKILL_PIERCE_SPEC) - 20*ch->drunkMinus();
  else if (isBluntWeapon() && ch->getDiscipline(DISC_BLUNT))
    skill = ch->getSkillValue(SKILL_BLUNT_SPEC) - 20*ch->drunkMinus();
  else
    skill = 0;

  *fx += (float) (1.0 * skill / 100.0);
}

int TBaseWeapon::catchSmack(TBeing *ch, TBeing **targ, TRoom *rp, int cdist, int mdist)
{
  int rc;
  TThing *c, *c_next;
  bool true_targ;
  int i;
  int resCode = 0;
  spellNumT damtype;
  wearSlotT phit;
  int range;

  if (dynamic_cast<TArrow *>(this)){
    if(objVnum() == 31864 || objVnum() == 31869)
      damtype = TYPE_SHOOT;
    else
      damtype = DAMAGE_ARROWS;
  } else
    damtype = getWtype();

  for (c = rp->getStuff(); c; c = c_next) {
    c_next = c->nextThing;
    if (c == ch)
      continue;
    TBeing *tb = dynamic_cast<TBeing *>(c);
    if (!tb)
      continue;

    // range is sort of a modified cdist
    if (tb->isFlying())
      range = cdist + 1;
    else
      range = cdist;

    // if we didn't get a target specified, just hit the first thing we see
    // that isn't in the same room as us
    if(!*targ){
      if(ch->roomp == rp)
	continue;

      *targ=tb;
    }

    // anyone we want to hit here?  (including innocents)
    // the ch->isImmortal() checks prevent gods from hitting innocents
    if ((true_targ = (tb == *targ)) ||
         hitInnocent(ch, this, tb)) {
      // if we hit an innocent, treat range as being greater so that damage
      // is less than if it was intentional
      if (!true_targ && range != mdist)
        range++;

      if (!ch->isImmortal() &&
            (!(i = ch->specialAttack(tb, SKILL_RANGED_PROF)) ||
            i == GUARANTEED_FAILURE)) {
	if(::number(0,1)){
	  act("$n dodges out of the way of $p.",
	      FALSE, tb, this, NULL, TO_ROOM);
	  tb->sendTo("You dodge out of its way.\n\r");
	  if (!ch->sameRoom(*tb))
	    act("In the distance, $N dodges out of the way of $p.",
		TRUE,ch,this,tb,TO_CHAR);
	} else {
	  act("$n dives behind some cover, avoiding $p.", 
	      FALSE, tb, this, NULL, TO_ROOM);
	  tb->sendTo("You dive behind some cover avoiding it.\n\r");
	  if (!ch->sameRoom(*tb))
	    act("In the distance, $N dives behind some cover, avoiding $p.",
		TRUE,ch,this,tb,TO_CHAR);
	}



	if (spec) checkSpec(tb, CMD_ARROW_DODGED, "", NULL);
        resCode = FALSE;
        if (!tb->isPc())
          pissOff(dynamic_cast<TMonster *>(tb), ch);
        if (cdist == 0)
          ch->setCharFighting(tb, 0);
        return resCode;
      } else {
        if (true_targ)
          act("$n is smacked by $p!", FALSE, tb, this, NULL, TO_ROOM);
        else
          act("$n is accidentally smacked by $p!", FALSE, tb, this, NULL, TO_ROOM);
        act("You are unable to dodge being hit by $p!",
                FALSE, tb, this, NULL, TO_CHAR);
        resCode = TRUE;
        phit = tb->getPartHit(NULL, FALSE);
        if (!isBluntWeapon() &&
             !tb->getStuckIn(phit) &&
             ::number(1, 100) < getCurSharp()) {
          --(*this);
          rc = tb->stickIn(this, phit);
          if (rc) {
            if (!ch->sameRoom(*tb))
              act("In the distance, $p embeds itself in $N.",
                   TRUE,ch,this,tb,TO_CHAR);
	    if (spec) checkSpec(ch, CMD_ARROW_EMBED, "", NULL);
          } else {
            if (!ch->sameRoom(*tb))
              act("In the distance, $N is hit by $p.",TRUE,ch,this,tb,TO_CHAR);
	    if (spec) {
	      checkSpec(tb, CMD_ARROW_GLANCE, "", NULL);
	    }
          }
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            if (true_targ)
              ADD_DELETE(resCode, DELETE_VICT);
            else {
              delete tb;
              tb = NULL;
              return resCode;
            }
          }
        } else {
          --(*this);
          *(tb->roomp) += *this;
          if (!ch->sameRoom(*c))
            act("In the distance, $N is hit by $p.",TRUE,ch,this,tb,TO_CHAR);
        }

        int d = (int) damageLevel();

        // d *= mdist - range + 1;  // modify for point blank range - bat
	// worst idea ever - peel

        d = get_range_actual_damage(ch, tb, this, d, damtype);
	
	if(isPoisoned())
	  applyPoison(tb);

	TArrow *arrow;
	if((arrow=dynamic_cast<TArrow *>(this)) && 
	   arrow->getTrapDamType()!=DOOR_TRAP_NONE){
	  tb->triggerArrowTrap(arrow);
	  arrow->setTrapLevel(0);
	  arrow->setTrapDamType(DOOR_TRAP_NONE);
	}
	
        if (c->roomp && !c->roomp->isRoomFlag(ROOM_ARENA)) {
          if (::number(1, d) <= getStructPoints()) {
            addToStructPoints(-1);
            if (getStructPoints() <= 0) {
              if (!ch->sameRoom(*tb))
                act("In the distance, $p is destroyed.",TRUE,ch,this,0,TO_CHAR);
              makeScraps();
              ADD_DELETE(resCode, DELETE_ITEM);
            }
          }
        }
#if RANGE_DEBUG
        vlogf(LOG_MISC, fmt("Range debug: (1) %s damaging %s with %s for %d dam") % 
                 ch->getName() % tb->getName() % getName() % d);
#endif
        rc = ch->applyDamage(tb, d, damtype);
        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          if (true_targ) {
            ADD_DELETE(resCode, DELETE_VICT);
            return resCode;
          }
          delete tb;
          tb = NULL;
          return resCode;
        }
        if (c && !c->isPc())
          pissOff(dynamic_cast<TMonster *>(tb), ch);
        if (cdist == 0)
          ch->setCharFighting(tb, 0);
        if (true_targ)
          *targ = tb;
        return resCode;
      }
    }
  }
  return FALSE;
}

sstring TBaseWeapon::showModifier(showModeT mode, const TBeing *ch) const
{
  sstring a;
  if (mode == SHOW_MODE_SHORT_PLUS ||
       mode == SHOW_MODE_SHORT_PLUS_INV ||
       mode == SHOW_MODE_SHORT) {
    a = " (";
    a += equip_condition(-1);
    a += ")";
    if (ch->hasWizPower(POWER_IMM_EVAL) || TestCode2) {
      char buf[256];
      sprintf(buf, " (L%d)", (int) (weaponLevel() + 0.5));
      a += buf;
    }
  }

  if(isPoisoned())
    a += " (poisoned)";

  return a;
}

void TBaseWeapon::lowCheck()
{
  int ap = suggestedPrice();
#ifdef __INSURE__
  // suggestedPrice() uses float which sometimes has some fluxuation due to
  // truncation, especially true if using things like "insure" or "purify"
  // which watch the data.  Allow a small flux on the price to avoid problems.
  int delta = ap - obj_flags.cost;
  if ((delta > 5) || (delta < -5)) {
#else
  if (ap != obj_flags.cost) {
#endif
    vlogf(LOG_LOW, fmt("base_weapon %s has a bad price (%d).  should be (%d)") % 
         getName() % obj_flags.cost % ap);
    obj_flags.cost = ap;
  }
  if (canWear(ITEM_HOLD)) {
    int amt = -itemAC();
    if (amt)
      vlogf(LOG_LOW, fmt("Holdable weapon (%s:%d) with AC.  (bad!)") % 
          getName() % objVnum());
  }
}

double TBaseWeapon::weaponLevel() const
{
  double weapon_lev = (damageLevel() * .6) +
                      (structLevel() * .3) +
                      (sharpLevel() * .1);

  return weapon_lev;
}

double TBaseWeapon::damageLevel() const
{
  double damage_level = getWeapDamLvl() / 4.0;
  return damage_level;
}

double TBaseWeapon::structLevel() const
{
  double struct_lev = max((getMaxStructPoints()-10), 0) * 2.0 / 3.0;

  return struct_lev;
}

double TBaseWeapon::sharpLevel() const
{
  double sharp_lev = max((getMaxSharp()-10), 0) * 2.0 / 3.0;

  return sharp_lev;
}

int TBaseWeapon::rentCost() const
{
  int val = TObj::rentCost();

  // for later use

  return val;
}

double TBaseWeapon::objLevel() const
{
  return weaponLevel();
}

sstring TBaseWeapon::getNameForShow(bool useColor, bool useName, const TBeing *ch) const
{
  char buf2[256];
  sprintf(buf2, "%s (L%d)",
       useName ? name : (useColor ? getName() : getNameNOC(ch).c_str()),
       (int) (objLevel() + 0.5));
  return buf2;
}

void TBaseWeapon::purchaseMe(TBeing *ch, TMonster *keeper, int cost, int shop_nr)
{
  ch->giveMoney(keeper, cost, GOLD_SHOP_WEAPON);

  shoplog(shop_nr, ch, keeper, getName(), cost, "buying");

  if(shop_index[shop_nr].isOwned()){
    TShopOwned tso(shop_nr, keeper, ch);
    
    tso.doDividend(this, cost);
    tso.doReserve();
    tso.chargeTax(this, cost);
  }

}

void TBaseWeapon::sellMeMoney(TBeing *ch, TMonster *keeper, int cost, int shop_nr)
{
  keeper->giveMoney(ch, cost, GOLD_SHOP_WEAPON);

  shoplog(shop_nr, ch, keeper, getName(), -cost, "selling");


  if(shop_index[shop_nr].isOwned()){
    TShopOwned tso(shop_nr, keeper, ch);

    tso.doReserve();
  }

}

bool TBaseWeapon::isPoisoned() const
{
  if(poison >= LIQ_WATER)
    return true;

  return false;
}


void TBaseWeapon::applyPoison(TBeing *vict)
{
  TBeing *ch;

  if(!isPoisoned())
    return;

  if((ch=dynamic_cast<TBeing *>(equippedBy))){
    act("There was something nasty on that $o!",
	FALSE, ch, this, vict, TO_VICT, ANSI_RED);
    act("You inflict something nasty on $N!",
	FALSE, ch, this, vict, TO_CHAR, ANSI_RED);
    act("There was something nasty on that $o!",
	FALSE, ch, this, vict, TO_NOTVICT, ANSI_RED);
    doLiqSpell(ch, vict, poison, 1);
  } else {
    doLiqSpell(vict, vict, poison, 1);
  }


  poison=(liqTypeT)-1;

}

void TBaseWeapon::setPoison(liqTypeT liq)
{
  if(!isPoisoned())
    poison=liq;
}
