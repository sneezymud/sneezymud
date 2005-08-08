/////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD  - All rights reserved, SneezyMUD Coding Team
//      "range.cc" - All functions and routines related to ranged combat
//
//      Ranged systems coded by Russ Russell and Benjamin Youngdahl
//
/////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <map>

#include "stdsneezy.h"
#include "range.h"
#include "combat.h"
#include "obj_quiver.h"
#include "obj_bow.h"
#include "obj_portal.h"
#include "obj_arrow.h"
#include "obj_gun.h"
#include "pathfinder.h"

#define RANGE_DEBUG 0

const char *directions[][2] =
{
  {"north", "from the south"},
  {"east", "from the west"},
  {"south", "from the north"},
  {"west", "from the east"},
  {"up", "from below"},
  {"down", "from above"},
  {"northeast", "from the southwest"},
  {"northwest", "from the southeast"},
  {"southeast", "from the northwest"},
  {"southwest", "from the northeast"}
};

const char *dir_name[] =
{
  "the north",
  "the east",
  "the south",
  "the west",
  "above",
  "below",
  "the northeast",
  "the northwest",
  "the southeast",
  "the southwest",
};

int TThing::throwMe(TBeing *ch, dirTypeT tdir, const char *vict)
{
  int iDist, rc;
  unsigned int max_distance;
  double mult;
  TBeing *targ;
  TThing *tmp;
  unsigned int count;
  char local_vict[256];

  // a little physics....
  // let the force I can throw something with be a function of my brawn
  // F = f(brawn) = mass * accelleration
  // acc = f(brawn)/m
  float acc = ch->plotStat(STAT_CURRENT, STAT_BRA, 500.0, 5000.0, 2500.0);
  acc /= max((float) 3.0, getWeight());
  // regard acc as a ft/sec^2
  // assume a throw has this constant acceleration for 0.2 secs
  // which yields a final velocity v0x
  float v0 = 0.2 * acc;
  // regard this as ft/sec
  // note, the values for acc's plotStat are roughly to correspond to
  // avg human = 65 mph fastball, best human = 90 mph fast ball
  // we fufged it and pretended baseball = 5lb (same as training dag)
  // let's assume that they throw the object semi level to ground (no up angle)
  // time aloft is then given by: h = 1/2 gt^2
  // h being height thrown from, g = gravity
  // using Earth's gravity of 32 ft/sec^2 and converting height to feet
  float tt = sqrt(2 * (ch->getPartMinHeight(ITEM_WEAR_NECK)/12.0) / 32.0);
  // regard this as secs
  // lets toss in some "smarts" checking
  // the above assumed the trajectory was at an up angle of 0 degrees
  // if the thrower is smart, he'll realize he can throw further with an
  // up angle.  Since they are trying to hit a fixed distance target, they
  // are really trying to solve a trajectory.  This ain't easy so make the
  // allowed angle be a function of int.
  // 45 degrees is obscene range
  int ang = ch->plotStat(STAT_CURRENT, STAT_FOC, 0, 45, 5);
  // figure out the limiting angle
  // i.e. above angle assumes I can safely throw a ballistic arc
  // ceilings would prevent this...
  if (ch->roomp->getRoomHeight() >= 0) {
    // vf^2 = 2ax
    // (vo sin(lim_ang))^2 = 2 * 32.0 * (room_height - my height)
    float lim_ang = sqrt(2 * 32.0 * (ch->roomp->getRoomHeight() - ch->getPartMinHeight(ITEM_WEAR_NECK))/12.0 / v0 / v0);
    lim_ang = asin(lim_ang);
    lim_ang = lim_ang * 360.0 / 2 / M_PI;  // convert from radians
    ang = min(ang, (int) lim_ang);
  }
  ang = min(max(ang, 0), 45);
  // and FINALLY, we translate all this into a range...
  // the upangle range = v0^2/g sin 2(ang)
  mult = ((v0 * v0)/32.0) * sin(2*ang*(2.0*M_PI/360.0));
  // the above formula is for returning to same height we started from
  // make allowance for how high we started from (from tt above)
  mult += v0 * cos(ang * (2.0*M_PI/360.0)) * tt;
  
  // regard the average "room" as 50 feet across, exterior = 100 ft across
  if (ch->outside())
    max_distance = (unsigned int) (mult / 100);
  else
    max_distance = (unsigned int) (mult / 50);

  // this results in typical max distances of 3-4 rooms
  // arbitrarily scale it so get 1-2 range limit
  max_distance /= 2;

  TObj *tobj = dynamic_cast<TObj *>(this);
  if (tobj && !tobj->canWear(ITEM_THROW))
    max_distance /= 2;

  // allow it to go 1 room, people get pissy otherwise
  max_distance = max(max_distance, (unsigned int) 1);

#if RANGE_DEBUG
char buf[256];
sprintf(buf, "Ranged debug: max_distance: %d\n\racceleration: %6.2f, velocity: %6.2f\n\rhangtime: %6.2f, angle: %d", max_distance, acc, v0, tt, ang);
vlogf(LOG_BUG, buf);
#endif

  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_RANGED_PROF, 20);
  
  strcpy(local_vict, vict);

  if(sstring(local_vict).isNumber() && (sscanf(local_vict, "%d", &iDist) == 1))
    *local_vict = '\0';
  else
    iDist = min(50, (int) max_distance);

  if (iDist < 0) {
    ch->sendTo("You need to supply a positive (or 0) distance to throw.\n\r");
    return FALSE;
  }
  if (tdir == 4)
    max_distance /= 2; // you can only throw upwards half as far 

  if ((iDist > 50) || (max_distance < (unsigned int) iDist)) {
    ch->sendTo("Much too far.  Maybe in your dreams!\n\r");
    ch->sendTo(fmt("You couldn't possibly throw it further than %d rooms.\n\r") % max_distance);
    return FALSE;
  }

  count = 0;
  if (*local_vict)
    targ = get_char_vis_direction(ch, local_vict, tdir, iDist, TRUE, &count);
  else
    targ = NULL;

  if (targ && ch->checkPeacefulVictim("A strange force prevents you from contemplating hurting your target.\n\r", targ))
    return FALSE;

  if (targ && ch->noHarmCheck(targ))
    return FALSE;

  // treat fliers as being 1 room further away
  if (targ && targ->isFlying() && !ch->isFlying() && ((count+1) > max_distance)){
    act("Unfortunately, $N is flying and you can't quite reach that far.",
       FALSE, ch, 0, targ, TO_CHAR);
    return FALSE;
  }

  act("You throw $p!", FALSE, ch, this, 0, TO_CHAR);
  act("$n throws $p!", TRUE, ch, this, 0, TO_ROOM);
  tmp = ch->unequip(ch->getPrimaryHold());
  if (!tmp) {
    vlogf(LOG_BUG, fmt("Bad unequip in throwThing (%s : %s)") %  getName() % ch->getName());
    ch->sendTo("Something real bad happened.  Talk to a god.\n\r");
    return FALSE;
  }
  *ch->roomp += *tmp;
  rc = throwThing(this, tdir, ch->in_room, &targ, iDist, max_distance,ch);
  if (IS_SET_DELETE(rc, DELETE_ITEM)) {
    return DELETE_THIS;
  }
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete targ;
    targ = NULL;
  }
  ch->addToWait(combatRound(2));
  ch->addToMove(-2);
  rc = checkSpec(NULL, CMD_OBJ_THROWN, NULL, NULL);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  return TRUE;
}

void TBeing::doThrow(const sstring &argument)
{
  TThing *t;
  sstring object, dir, vict;
  int rc;
  dirTypeT tdir;

  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return;

  if (getMove() <= 2) {
    sendTo("You are too tired to throw anything!\n\r");
    return;
  }
  vict="";
  object=argument.word(0);
  dir=argument.word(1);
  vict=argument.word(2);

  if (object.empty()) {
    sendTo("Syntax: throw <object> [direction] [character | distance]\n\r");
    return;
  }

  tdir = getDirFromChar(dir);
  if (tdir == DIR_NONE) {
    vict=dir;
    tdir = dirTypeT(::number(MIN_DIR, MAX_DIR-1));
  }
  if (vict.empty()) {
    if (fight())
      vict=fight()->name;
    else
      vict="mama-o-Brutius";
  }
  if (!(t = equipment[getPrimaryHold()])) {
    sendTo("You can only throw objects you are holding in your primary hand.\n\r");
    return;
  }
  TObj * tobj = dynamic_cast<TObj *>(t);
  if (tobj) {
    if (tobj->isObjStat(ITEM_NEWBIE)) {
      sendTo("That item might suck, but you can't just go throwing it around.\n\r");
      return;
    }
    if (tobj->isObjStat(ITEM_NODROP)) {
      sendTo("You can't throw a cursed item!\n\r");
      return;
    }
  }
  if (!isname(object, t->name)) {
    sendTo("You can only throw objects you are holding in your primary hand.\n\r");
    return;
  }

  rc = t->throwMe(this, tdir, vict.c_str());
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete t;
    t = NULL;
  }
  return;
}

int get_range_actual_damage(TBeing *ch, TBeing *victim, TObj *o, int dam, spellNumT attacktype)
{
  if (ch->damCheckDeny(victim, attacktype))
    return 0;
  dam = victim->skipImmortals(dam);
  if (!ch->damDetailsOk(victim, dam, TRUE))
    return 0;

  dam = ch->damageTrivia(victim, o, dam, attacktype);

  // adjust the damage based on their skill
  if (dam) {
    int q;
    if (dynamic_cast<TArrow *>(o)) {
      // shot objects are here
      // ranged spec basically allows arrow to do extra damage...
      q = 100;
      q += (ch->getSkillValue(SKILL_RANGED_PROF)/2); // 50%-100% damage
      q += (ch->getSkillValue(SKILL_RANGED_SPEC)/2); // 100%-150% damage

      dam *= q;
      dam /= 100;
    } else {
      // thrown objects are here
      q = ch->getSkillValue(SKILL_RANGED_PROF);
      dam *= q;
      dam /= 100;
    }
    dam = max(dam,1);
  }
  return (dam);
}

void pissOff(TMonster *irritated, TBeing *reason)
{
  affectedData af;
  const int PISSED_MOB_PERSIST = 250;    // make em damn persistant 

  if (!(irritated->canSee(reason, INFRA_YES)) || reason->isImmortal())
    return;

  if (irritated->getPosition() >= POSITION_SLEEPING) {
    if (!irritated->isDumbAnimal()) {
      switch (dice(1, 10)) {
        case 1:
          act("$n screams 'I must kill $N!", TRUE, irritated, 0, reason, TO_ROOM);
          break;
        case 2:
          act("$n says 'That's it, $N is toast.'", TRUE, irritated, 0, reason, TO_ROOM);
          break;
        case 3:
          act("$n growls '$N must die'", TRUE, irritated, 0, reason, TO_ROOM);
          break;
        case 4:
          act("$n begins to curse about $N.", TRUE, irritated, 0, reason, TO_ROOM);
          break;
        case 5:
          act("$n says 'Time to go killing.'", TRUE, irritated, 0, reason, TO_ROOM);
          break;
        default:
          act("$n screams in rage!", TRUE, irritated, 0, reason, TO_ROOM);
          break;
      }
    } else
      irritated->aiGrowl(NULL);

    SET_BIT(irritated->specials.act, ACT_HUNTING);
    irritated->specials.hunting = reason;
    irritated->hunt_dist = 500;
    irritated->persist = PISSED_MOB_PERSIST;
    irritated->oldRoom = irritated->inRoom();
    irritated->addHated(reason);
    if (!irritated->affectedBySpell(SPELL_TRAIL_SEEK)) {
      af.type = SPELL_TRAIL_SEEK;        // make sure mob hunts global 
      af.duration = PISSED_MOB_PERSIST;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = 0;
      irritated->affectTo(&af);
    }
  }
}

// ch is thrower of thing (sometimes NULL - falling objects)
// vict is potential victim
bool hitInnocent(const TBeing *ch, const TThing *thing, const TThing *vict)
{
  // hit innocent due to misthrow

  const TBeing * tbc = dynamic_cast<const TBeing *>(vict);

  if (ch) {
    if (ch->isImmortal())
      return FALSE;
 
    // presume anyone near thrower is safely out of the way
    if (tbc && tbc->sameRoom(*ch))
      return false;
  
    // protect the mounts of group members too
    if (tbc && tbc->rider) {
      TBeing *temp = dynamic_cast<TBeing *>(tbc->horseMaster());
      if (temp && temp->inGroup(*ch) && temp->sameRoom(*ch))
        return false;
    }
  }

  float num;

  // size of target
  num = vict->getHeight();
  num *= vict->getWeight();
  num /= 1000;
  // num should be on order of 15 - 70

  num += 1 + thing->getVolume()/1000;

  if (ch) {
    num -= ch->plotStat(STAT_CURRENT, STAT_AGI, -10, 10, 0); 
  }

  if (tbc) {
    num -= tbc->plotStat(STAT_CURRENT, STAT_PER, -5, 5, 0); 
    num -= tbc->plotStat(STAT_CURRENT, STAT_SPE, -5, 5, 0); 
  }

  return (::number(20, 100) < num);
}

// returns DELETE_ITEM if *thing should be nuked
// returns DELETE_VICT if *targ should go poof   (|| able)
// return TRUE if it hits something (stops moving), otherwise false
// cdist = current dist traveled
// mdist =  max range item can go
int catch_or_smack(TRoom *rp, TBeing **targ, TThing *thing, TBeing *ch, int cdist, int mdist)
{
  return thing->catchSmack(ch, targ, rp, cdist, mdist);
}

int TThing::catchSmack(TBeing *ch, TBeing **targ, TRoom *rp, int cdist, int mdist)
{
  TThing *c, *c_next;
  int d = 0;
  bool true_targ;
  int i;
  int resCode = 0;
  int damtype;
  int range;

  damtype = getWtype();

  for (c = rp->getStuff(); c; c = c_next) {
    c_next = c->nextThing;
    TBeing *tbt = dynamic_cast<TBeing *>(c);
    if (!tbt || (tbt == ch))
      continue;

    // range is sort of a modified cdist
    if (tbt->isFlying())
      range = cdist + 1;
    else
      range = cdist;

    // anyone we want to hit here?  (including innocents) 
    // the ch->isImmortal() checks prevent gods from hitting innocents
    if ((true_targ = (tbt == *targ)) || 
         hitInnocent(ch, this, tbt)) {
      // if we hit an innocent, treat range as being greater so that damage
      // is less than if it was intentional
      if (!true_targ && range != mdist)
        range++;

      if ((::number(1, 25) < tbt->plotStat(STAT_CURRENT, STAT_SPE, 3, 18, 13)) &&
           tbt->hasHands() && !tbt->bothHandsHurt() &&
           tbt->awake() && tbt->canGet(this, SILENT_YES)) {
        resCode = TRUE;
        act("$n catches $p.", FALSE, tbt, this, NULL, TO_ROOM);
        if (!ch->sameRoom(*tbt))
          act("In the distance, $N catches your $o.",TRUE,ch,this,tbt,TO_CHAR);
 
        if (!tbt->heldInPrimHand()) {
          act("You catch $p.",
                       FALSE,tbt,this,0,TO_CHAR);
          --(*this);
          tbt->equipChar(this, tbt->getPrimaryHold());
        } else if (!tbt->heldInSecHand()) {
          act("You catch $p.",
                       FALSE,tbt,this,0,TO_CHAR);
          --(*this);
          tbt->equipChar(this, tbt->getSecondaryHold());
        } else {
          act("You catch $p, and add it to your inventory.",
                       FALSE,tbt,this,0,TO_CHAR);
          --(*this);
          *tbt += *this;
        }
        if (!tbt->isPc()) 
          pissOff(dynamic_cast<TMonster *>(tbt), ch);
        if (cdist == 0)
          ch->setCharFighting(tbt, 0);
        return resCode;
      } else if (!ch->isImmortal() &&
                  (!(i = ch->specialAttack(tbt, SKILL_RANGED_PROF)) || 
                   i == GUARANTEED_FAILURE)) {
        act("$n dodges out of the way of $p.", FALSE, tbt, this, NULL, TO_ROOM);
        tbt->sendTo("You dodge out of its way.\n\r");
        if (!ch->sameRoom(*tbt))
          act("In the distance, $N dodges out of the way of $p.",
                 TRUE,ch,this,tbt,TO_CHAR);
        resCode = FALSE;
        if (!tbt->isPc())
          pissOff(dynamic_cast<TMonster *>(tbt), ch);
        if (cdist == 0)
          ch->setCharFighting(tbt, 0);
        return resCode;
      } else {
       // smacked by non-weapon/arrow
        if (true_targ)
          act("$n is smacked by $p!", FALSE, tbt, this, NULL, TO_ROOM);
        else
          act("$n is accidentally smacked by $p!", FALSE, tbt, this, NULL, TO_ROOM);
        act("You are unable to dodge being hit by $p!",
                  FALSE, tbt, this, NULL, TO_CHAR);
        if (!ch->sameRoom(*tbt))
          act("In the distance, $N is hit by $p.",TRUE,ch,this,tbt,TO_CHAR);
        resCode = TRUE;
        d = min(max(0, (int) (getWeight() - 5)), 10);
#if RANGE_DEBUG
        vlogf(LOG_BUG, fmt("Range debug: (2) dam ping 1: %d") %  d);
#endif
// don't do this or we wind up with acorns killing people
//        d *= mdist - range + 1;  // modify for point blank range - bat
#if RANGE_DEBUG
        vlogf(LOG_BUG, fmt("Range debug: (2) dam ping 3: %d") %  d);
#endif
        TObj *tobj = dynamic_cast<TObj *>(this);
        if (tobj) {
          d = get_range_actual_damage(ch, tbt, tobj, d, TYPE_HIT);
#if RANGE_DEBUG
          vlogf(LOG_BUG, fmt("Range debug: (2) dam ping 4: %d") %  d);
#endif

          if (::number(1, d) <= tobj->getStructPoints() &&
              tbt->roomp && !tbt->roomp->isRoomFlag(ROOM_ARENA)) {
            tobj->addToStructPoints(-1);
            if (tobj->getStructPoints() <= 0) {
              if (!ch->sameRoom(*tbt))
                act("In the distance, $p is destroyed.",TRUE,ch,tobj,0,TO_CHAR);
              tobj->makeScraps();
              ADD_DELETE(resCode, DELETE_ITEM);
            }
          }
#if RANGE_DEBUG
          vlogf(LOG_BUG, fmt("Range debug: (2) %s damaging %s with %s for %d dam") % 
                 ch->getName() % tbt->getName() % tobj->getName() % d);
#endif
          if (ch->reconcileDamage(tbt, d, getWtype()) == -1) {
            if (true_targ) {
              ADD_DELETE(resCode, DELETE_VICT);
              return resCode;
            }
            delete tbt;
            tbt = NULL;
            return resCode;
          }
        }

        if (tbt && !tbt->isPc())
          pissOff(dynamic_cast<TMonster *>(tbt), ch);
        if (cdist == 0)
          ch->setCharFighting(tbt, 0);
        if (true_targ)
          *targ = tbt;

        return resCode;
      }
    }
  }
  return FALSE;
}

int hit_obstacle_in_room(TRoom *rp, TThing *thing, TBeing *ch)
{
  TThing *t;

  for (t = rp->getStuff(); t; t = t->nextThing) {
    TObj *obj = dynamic_cast<TObj *>(t);
    if (obj) {
      if ((obj != thing) && (!number(0, 4)) && (obj->getVolume() > 10000) &&
          (obj->getVolume() > ((dice(1, 1000) * 1000)) - thing->getVolume())) {
        act("$n smacks into $N, and falls to the $g.",
             TRUE, thing, 0, obj, TO_ROOM);
        act("$p smacked into $N.", FALSE, ch, thing, obj, TO_CHAR); 
	if (thing->spec) thing->checkSpec(NULL, CMD_ARROW_HIT_OBJ, "", obj);
        return TRUE;
      }
    }
  }
  return FALSE;
}

static void barrier(TRoom *rp, dirTypeT dir, TThing *t)
{
  char buf[256];

  switch (rp->getSectorType()) {
    case SECT_TROPICAL_BUILDING:
    case SECT_TEMPERATE_BUILDING:
    case SECT_ARCTIC_BUILDING:
    case SECT_TROPICAL_CAVE:
    case SECT_TEMPERATE_CAVE:
    case SECT_ARCTIC_CAVE:
      if (dir == 5)
        sprintf(buf, "$n glances off the $g at a strange angle and comes to rest.");
      else if (dir == 4)
        sprintf(buf, "$n bounces off the ceiling and drops to the $g.");
      else
        sprintf(buf, "$n bounces off a wall and drops to the $g.");
      break;
    case SECT_ARCTIC_CITY:
    case SECT_TEMPERATE_CITY:
    case SECT_TROPICAL_CITY:
      if (dir == 5)
        sprintf(buf, "$n glances off the $g at a strange angle and comes to rest.");
      else if (dir == 4)
        sprintf(buf, "$n sails briefly into the air above before falling to the $g.");
      else
        sprintf(buf, "$n bounces off a wall and drops to the $g.");
      break;
    case SECT_JUNGLE:
    case SECT_RAINFOREST:
    case SECT_TEMPERATE_FOREST:
    case SECT_ARCTIC_FOREST:
      if (dir == 5)
        sprintf(buf, "$n glances off a stump and comes to rest.");
      else if (dir == 4)
        sprintf(buf, "$n bounces off a branch above before falling to the $g.");
      else
        sprintf(buf, "$n flies through the air a ways before hitting a tree.");
      break;
    case SECT_ARCTIC_RIVER_SURFACE:
    case SECT_ICEFLOW:
    case SECT_TEMPERATE_OCEAN:
    case SECT_TEMPERATE_RIVER_SURFACE:
    case SECT_TROPICAL_RIVER_SURFACE:
    case SECT_TROPICAL_OCEAN:
      if (dir == 5)
        sprintf(buf, "$n splashes into the water.");
      else if (dir == 4)
        sprintf(buf, "$n flies into the air, and falls into the water below.");
      else
        sprintf(buf, "$n flies through the air a ways before splashing into the water.");
      break;
    case SECT_TROPICAL_UNDERWATER:
    case SECT_TEMPERATE_UNDERWATER:
      sprintf(buf, "$n is slowed to a halt by the water.");
      break;
    case SECT_TROPICAL_ATMOSPHERE:
    case SECT_TEMPERATE_ATMOSPHERE:
    case SECT_ARCTIC_ATMOSPHERE:
      sprintf(buf, "$n loses momentum and begins to fall!");
      break;
    case SECT_DESERT:
    case SECT_TROPICAL_BEACH:
    case SECT_TEMPERATE_BEACH:
    case SECT_COLD_BEACH:
    case SECT_ARCTIC_MARSH:
    case SECT_TEMPERATE_SWAMP:
    case SECT_TROPICAL_SWAMP:
      if (dir == 5)
        sprintf(buf, "$n slams into the $g and comes to a stop.");
      else
        sprintf(buf, "$n flies a short distance before crashing into the $g.");
      break;
    case SECT_SOLID_ICE:
      sprintf(buf, "$n slams into a chunk of ice and gets stuck.");
      break;
    case SECT_SOLID_ROCK:
      sprintf(buf, "$n glances off a rock outcropping and stops flying.");
      break;
    case SECT_ASTRAL_ETHREAL:
      sprintf(buf, "A weird distortion in the matrix stops $n from flying.");
      break;
    case SECT_FIRE:
    case SECT_VOLCANO_LAVA:
      if (dir == 5)
        sprintf(buf, "$n disappears as it hits the flaming molten surface below.");
      else
        sprintf(buf, "$n goes a short ways before falling into the flaming surface below.");
      break;
    case SECT_SUBARCTIC:
    case SECT_ARCTIC_WASTE:
    case SECT_ARCTIC_ROAD:
    case SECT_TUNDRA:
    case SECT_ARCTIC_MOUNTAINS:
    case SECT_PLAINS:
    case SECT_TEMPERATE_ROAD:
    case SECT_GRASSLANDS:
    case SECT_TEMPERATE_HILLS:
    case SECT_TEMPERATE_MOUNTAINS:
    case SECT_SAVANNAH:
    case SECT_VELDT:
    case SECT_TROPICAL_ROAD:
    case SECT_TROPICAL_HILLS:
    case SECT_TROPICAL_MOUNTAINS:
    default:
      if (dir == 5)
        sprintf(buf, "$n glances off the $g at a strange angle and comes to rest.");
      else if (dir == 4)
        sprintf(buf, "$n sails briefly into the air above before falling to the $g.");
      else
        sprintf(buf, "$n flies through the air a ways before dropping to the $g.");
  }
  act(buf, TRUE, t, 0, 0, TO_ROOM);
  if (t->spec) t->checkSpec(NULL, CMD_ARROW_MISSED, "", NULL);
}

// max_dist is the maximum distance that "o" can be thrown. 
// iDist is how far user wants item to go 
// distance is actual amount flown
// return DELETE_ITEM if t should go poof
// returns DELETE_VICT if *targ should die   (|| able)
int throwThing(TThing *t, dirTypeT dir, int from, TBeing **targ, int dist, int max_dist, TBeing *ch)
{
  char capbuf[256];
  TRoom *rp, *newrp;
  int iDist = 0;
  int rc;

  // send the thing up to "dist" rooms away, checking for our target 
  while ((rp = real_roomp(from))) {
    if (iDist) {
      sprintf(capbuf, "$n %s into the room %s.", (dir == 5 ? "drops" : "flies"), directions[dir][1]);
      act(capbuf, TRUE, t, 0, 0, TO_ROOM);
      if (t->spec) t->checkSpec(NULL, CMD_ARROW_INTO_ROOM, "", NULL);

    } else {
      if (t->spec) t->checkSpec(NULL, CMD_ARROW_SHOT, "", NULL);
    }

    if (hit_obstacle_in_room(rp, t, ch))
      return FALSE;


    // max_dist here is used to modify damage based on how far away they are.
    // use the absolute max it can go (max_dist) for this calculation
    rc = catch_or_smack(rp, targ, t, ch, iDist, max_dist);
    if (IS_SET_DELETE(rc, DELETE_ITEM) || IS_SET_DELETE(rc, DELETE_VICT))
       return rc;
    else if (rc)
      return FALSE;

    // users specified to fly "dist" rooms, so probably only provided 
    // momentum for that far.  Use dist in this check, rather than max_dist
    iDist++;
    if (iDist > dist || iDist > max_dist) {
      act("$n drops to the $g.", TRUE, t, 0, 0, TO_ROOM);

      if (dir != DIR_DOWN)
        act("$p ran out of momentum and fell.", FALSE, ch, t, 0, TO_CHAR); 
      if (t->spec) t->checkSpec(NULL, CMD_ARROW_MISSED, "", NULL);
      return FALSE;
    } else if (!clearpath(from, dir) || rp->isUnderwaterSector()) {
      barrier(rp, dir,t);
      act("$p hit an obstacle and dropped to the $g.", FALSE, ch, t, 0, TO_CHAR); 
      
      if (t->spec) t->checkSpec(NULL, CMD_ARROW_MISSED, "", NULL);
      return FALSE;
    }
    // No need to check for a NULL newrp, clearpath() does that. - Russ 
    newrp = real_roomp(rp->dir_option[dir]->to_room);
    if (newrp->isRoomFlag(ROOM_PEACEFUL) || newrp->isRoomFlag(ROOM_NO_MOB)) {
      act("Strangely, $n hits a magical barrier and falls to the $g.", 
             FALSE, t, 0, 0, TO_ROOM);
      act("$p hit a magic barrier and dropped to the $g.", FALSE, ch, t, 0, TO_CHAR); 

      if (t->spec) t->checkSpec(NULL, CMD_ARROW_MISSED, "", NULL);

      return FALSE;
    }

    if (newrp->isRoomFlag(ROOM_NO_HEAL)) {
      act("Strangely, $n hits a magical barrier and falls to the $g.",
	  FALSE, t, 0, 0, TO_ROOM);
      act("$p hit a magic barrier and dropped to the $g.", FALSE, ch, t, 0, TO_CHAR);

      if (t->spec) t->checkSpec(NULL, CMD_ARROW_MISSED, "", NULL);

      return FALSE;
    }

    sprintf(capbuf, "$n %s %s out of the room.", (dir == 5 ? "drops" : "flies"), directions[dir][0]);
    act(capbuf, TRUE, t, 0, 0, TO_ROOM);
    --(*t);
    from = rp->dir_option[dir]->to_room;
    *(real_roomp(from)) += *t;
  }
  if (!rp) {
    vlogf(LOG_BUG, fmt("%s thrown into non-existant room #%d") %  capbuf % from);
    --(*t);
    thing_to_room(t, ROOM_VOID);
    return FALSE;
  }
  return FALSE;
}

// ch attempts to see targ by looking linearly in all cardinal directions
// returns the direction targ was spotted in, or -1 (failure)
// range and dir are also passed as formal vars
dirTypeT can_see_linear(const TBeing *ch, const TBeing *targ, int *rng, dirTypeT *dr)
{
  int rm, max_range = 15, range = 0;
  int height = 0;

  // add bonus from race
  max_range += ch->getMyRace()->getLOS();

  max_range += (ch->visionBonus + 1)/2;

  switch (ch->getPosition()) {
    case POSITION_MOUNTED:
      height = 4*dynamic_cast<TBeing *>(ch->riding)->getHeight()/5 + 2*ch->getHeight()/3;
      break;  
    case POSITION_STANDING:
    case POSITION_FIGHTING:
      height = ch->getHeight();
      break;
    case POSITION_SITTING:
      height = ch->getHeight()/3;
      break; 
    case POSITION_CRAWLING:
      height = ch->getHeight()/4;
      break;
    case POSITION_RESTING:
    case POSITION_SLEEPING:
    default:
      height = ch->getHeight()/6;
      break;
  }
  if (height > 80)
    max_range += 2;
  else if (height > 75)
    max_range += 1;
  else if (height < 60)
    max_range -= 2;
  else if (height < 65)
    max_range -= 1;

  if(ch->hasQuestBit(TOG_REAL_AGING)){
    if ((ch->age()->year - ch->getBaseAge() + 17) > 80)
      max_range -= 3;
    else if ((ch->age()->year - ch->getBaseAge() + 17) > 60)
      max_range -= 2;
    else if ((ch->age()->year - ch->getBaseAge() + 17) > 40)
      max_range -= 1;
    else if ((ch->age()->year - ch->getBaseAge() + 17) < 20)
      max_range += 1;
  }


  if ((ch->roomp->getWeather() == WEATHER_RAINY) ||
      (ch->roomp->getWeather() == WEATHER_LIGHTNING))
    max_range /= 2;
  else if (ch->roomp->getWeather() == WEATHER_SNOWY)
    max_range /= 4;

  dirTypeT i;
  for (i = MIN_DIR; i < MAX_DIR; i++) {
    rm = ch->in_room;
    range = 0;
    while (range < max_range) {
      range++;
      max_range -= TerrainInfo[real_roomp(rm)->getSectorType()]->thickness;
      if (clearpath(rm, i)) {
        rm = real_roomp(rm)->dir_option[i]->to_room;
        const TThing *t;
        for (t = real_roomp(rm)->getStuff(); t; t = t->nextThing) {
          if ((t == targ) && ch->canSee(t)) {
            *rng = range;
            *dr = i;
            return i;
          }
        }
      }
    }
  }
  return DIR_NONE;
}

TBeing *get_char_linear(const TBeing *ch, char *arg, int *rf, dirTypeT *df)
{
  int rm, max_range = 15, range = 0, n, n_sofar = 0;
  TThing *t;
  char *tmp, tmpname[256];

  if ((ch->getRace() == RACE_ELVEN) || (ch->getRace() == RACE_DROW))
    max_range += 5;

  max_range += (ch->visionBonus + 1)/2;

  if ((ch->roomp->getWeather() == WEATHER_RAINY) ||
      (ch->roomp->getWeather() == WEATHER_LIGHTNING))
    max_range /= 2;
  else if (ch->roomp->getWeather() == WEATHER_SNOWY)
    max_range /= 4;

  strcpy(tmpname, arg);
  tmp = tmpname;
  if (!(n = get_number(&tmp)))
    return NULL;

  // This routine counts folks in your room 
  rm = ch->in_room;
  dirTypeT i = DIR_NONE;
  range = 0;
  for (t = real_roomp(rm)->getStuff(); t; t = t->nextThing) {
    TBeing *tbt = dynamic_cast<TBeing *>(t);
    if (tbt && (isname(tmp, tbt->name)) && ch->canSee(tbt)) {
      n_sofar++;
      if (n_sofar == n) {
        *rf = range;
        *df = DIR_NONE;
        return tbt;
      }
    }
  }
  for (i = MIN_DIR; i < MAX_DIR; i++) {
    rm = ch->in_room;
    range = 0;
    while (range < max_range) {
      range++;
      max_range -= TerrainInfo[real_roomp(rm)->getSectorType()]->thickness;
      if (clearpath(rm, i)) {
        rm = real_roomp(rm)->dir_option[i]->to_room;
        for (t = real_roomp(rm)->getStuff(); t; t = t->nextThing) {
          TBeing *tbt = dynamic_cast<TBeing *>(t);
          if (tbt && (isname(tmp, tbt->getName())) && ch->canSee(tbt)) {
            n_sofar++;
            if (n_sofar == n) {
              *rf = range;
              *df = i;
              return tbt;
            }
          }
        }
      } else
        range = max_range + 1;
    }
  }
  return NULL;
}


int clearpath(int room, dirTypeT dir)
{
  TRoom *rp;

  rp = real_roomp(room);

  if (!rp || !rp->dir_option[dir])
    return FALSE;

  if (rp->dir_option[dir]->to_room < 1)
    return FALSE;

  if (!real_roomp(rp->dir_option[dir]->to_room)) {
    vlogf(LOG_BUG, fmt("Range function done in room with bad exit. (%d) Dir:[%d]") %  room % dir);
    return FALSE;
  }
  if (IS_SET(rp->dir_option[dir]->condition, EX_CLOSED))
    return FALSE;

  return (real_roomp(room)->dir_option[dir]->to_room);
}


void TBeing::doScan(const char *argument)
{
  const char *rng_desc[] =
  {
    "right here",    // 0
    "immediately",
    "nearby",
    "a short ways",
    "not too far",
    "a ways",        // 5 room
    "quite a ways",
    "way off",
    "way off",
    "far",
    "far",           // 10 rooms
    "way way off",
    "way way off",
    "real far",
    "real far",
    "very far",      // 15 rooms
    "very far",
    "extremely far",
    "extremely far",
    "on the horizon",
    "on the horizon",    // 20 rooms
    "on the horizon",
    "on the horizon",
    "on the horizon",
    "on the horizon" 
  };
  char buf[256], buf2[256];
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  int max_range = 15, range, new_rm, rm, nfnd;
  int hindered;
  bool found = FALSE;
  TThing *t;
  bool all = FALSE;

  argument_split_2(argument, arg1, arg2);
  float swt;

  dirTypeT sd = getDirFromChar(arg1);
  dirTypeT smin, smax;
  if (sd == DIR_NONE) {
    smin = MIN_DIR;
    smax = dirTypeT(MAX_DIR-1);
    swt = 1.5;
    sprintf(buf, "$n peers intently all around.");
    sprintf(buf2, "You peer intently all around, and see :\n\r");
    all = TRUE;
  } else {
    smin = sd;
    smax = sd;
    swt = 0.5;
    sprintf(buf, "$n peers intently %s.", dirs_to_blank[sd]);
    sprintf(buf2, "You peer intently %s, and see :\n\r", dirs_to_blank[sd]);
  }
  if (getMove() < (all ? 10 : 2)) {
    sendTo("You are simply too exhausted!\n\r");
    return;
  } 

  act(buf, TRUE, this, 0, 0, TO_ROOM);
  sendTo(buf2);

  if (!inLethargica())
    addToMove(all ? -10 : -2);

  if (isAffected(AFF_BLIND) && !isAffected(AFF_TRUE_SIGHT)) {
    sendTo("Nothing, you are blind.\n\r");
    return;
  }
  nfnd = 0;
  // Check in room first
  if (all) {
    // for specific directions, skip room so it doesn't count toward "nfnd"
    for (t = roomp->getStuff(); t; t = t->nextThing) {
      if (!dynamic_cast<TBeing *>(t))
        continue;
      if (t == this)
        continue;
      if (canSee(t)) {
        sstring nc_name = fmt("%30s") % t->getNameNOC(this);
        int name_pos = nc_name.find_first_not_of(" ");

        nc_name.replace(name_pos, nc_name.length()-name_pos, t->getName());

        sendTo(COLOR_MOBS, fmt("%s : right here\n\r") % nc_name);
        found = TRUE;
      } else if (canSee(t, INFRA_YES)) {
        sendTo(COLOR_MOBS, fmt("%30s : right here\n\r") % "A blob of heat");
        found = TRUE;
      }
    }
  }
  max_range -= TerrainInfo[roomp->getSectorType()]->thickness;
  max_range += (visionBonus / 10);

  // Let weather conditions play a part in range - Russ 10/14/98
  // silly immortal check, but imms gripe about it
  if (!isImmortal()) {
    if (roomp->getWeather() == WEATHER_SNOWY) {
      max_range -= 3;
      sendTo(COLOR_BASIC, "The <W>SNOW<1> greatly decreases your view!\n\r");
    } else if (roomp->getWeather() == WEATHER_RAINY) {
      max_range -= 2;
      sendTo(COLOR_BASIC, "The <B>RAIN<1> decreases your view!\n\r");
    } else if (roomp->getWeather() == WEATHER_CLOUDY) {
    max_range -= 1;
        sendTo(COLOR_BASIC, "The <k>FOG<1> slightly decreases your view!\n\r");
      //sendTo(COLOR_BASIC, "The <k>CLOUDS<1> slightly decrease your view!\n\r");
    } else if (roomp->getWeather() == WEATHER_CLOUDLESS) {
      max_range += 1;
      sendTo(COLOR_BASIC, "The <c>clear skies<1> enhance your view!\n\r");
    }
  }

  dirTypeT i;
  for (i = smin; i <= smax; i++) {
    nfnd = 0;
    hindered = FALSE;
    rm = in_room;
    range = 0;
    while ((range < max_range) && !hindered) {
      range++;
      if (clearpath(rm, i)) {
        new_rm = real_roomp(rm)->dir_option[i]->to_room;
        if (new_rm == rm)
          break;
        else
          rm = new_rm;

        for (t = real_roomp(rm)->getStuff(); t; t = t->nextThing) {
          TBeing *tbt = dynamic_cast<TBeing *>(t);
          if (!tbt)
            continue;
          if (can_see_char_other_room(this, tbt, real_roomp(rm))) {
            sstring nc_name = fmt("%30s") % tbt->getNameNOC(this);
            int name_pos = nc_name.find_first_not_of(" ");

            nc_name.replace(name_pos, nc_name.length()-name_pos, tbt->getName());
            sendTo(COLOR_MOBS, fmt("%s : %s %s\n\r") % nc_name % rng_desc[range] % dirs_to_blank[i]);
            nfnd++;
            found = TRUE;
            if (nfnd > (5 + visionBonus / 3)) {
              sendTo(fmt("The crowd hinders you from seeing any further %s.\n\r") % dirs_to_blank[i]);
              hindered = TRUE;
              break;
            }
          } else if (canSee(tbt, INFRA_YES)) {
            sendTo(COLOR_MOBS, fmt("%30s : %s %s\n\r") % "A blob of heat" % rng_desc[range] % dirs_to_blank[i]);
            nfnd++;
            found = TRUE;
            if (nfnd > (5 + visionBonus / 3)) {
              sendTo(fmt("The crowd hinders you from seeing any further %s.\n\r") % dirs_to_blank[i]);
              hindered = TRUE;
              break;
            }
          }
        }
      } else
        range = max_range + 1;
    }
  }
  if (!found)
    sendTo("Nothing.\n\r");

  addToWait(combatRound(swt));
}

// returns DELETE_THIS
int TBeing::stickIn(TThing *o, wearSlotT pos, silentTypeT silent)
{
  int rc;
  char buf[80];

  if ((pos == HOLD_RIGHT)) {
    pos = WEAR_HAND_R;
  } else if (pos == HOLD_LEFT) {
    pos = WEAR_HAND_L;
  }

  mud_assert(!o->equippedBy && !o->parent && (o->in_room == -1),
      "stickIn: item had owner at invocation");
  mud_assert(pos >= MIN_WEAR && pos < MAX_WEAR,
         "Bad slot in stickIn, %s %d", getName(), pos);
  mud_assert(slotChance(pos),
         "No slot chance in stickIn, %s %d", getName(), pos);
  mud_assert(getStuckIn(pos) == NULL,
      "stickIn: bodyPart had item stuckIn already");
  mud_assert(roomp != NULL,
      "stickIn: ch with no roomp");

  setStuckIn(pos, o);
  o->eq_stuck = pos;
  o->stuckIn = this;
  if (!silent) {
    sprintf(buf, "$p sticks in $n's %s!", describeBodySlot(pos).c_str());
    act(buf, FALSE, this, o, 0, TO_ROOM);
    sprintf(buf, "$p sticks in your %s!", describeBodySlot(pos).c_str());
    act(buf, FALSE, this, o, 0, TO_CHAR);
  }
  TObj *obj = dynamic_cast<TObj *>(o);
  if (obj && obj->spec) {
    rc = ((objSpecials[GET_OBJ_SPE_INDEX(obj->spec)].proc)
           (this, CMD_OBJ_STUCK_IN, "", obj, NULL));
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  return TRUE;
}

TThing *has_range_object(TBeing *ch, int *pos)
{
  TThing *hucked;
  TObj *tobj;

  if (!ch->equipment[HOLD_RIGHT] && !ch->equipment[HOLD_LEFT] && !ch->getStuff())
    return NULL;

  // Go thru possible places for throwing objects. 
  if ((hucked = ch->equipment[HOLD_RIGHT])) {
    tobj = dynamic_cast<TObj *>(hucked);
    if (tobj && tobj->canWear(ITEM_THROW)) {
      *pos = HOLD_RIGHT;
      return (tobj);
    }
  } else if ((hucked = ch->equipment[HOLD_LEFT])) {
    tobj = dynamic_cast<TObj *>(hucked);
    if (tobj && tobj->canWear(ITEM_THROW)) {
      *pos = HOLD_LEFT;
      return (tobj);
    }
  } else {
    for (TThing *t = ch->getStuff(); t; t = t->nextThing) {
      tobj = dynamic_cast<TObj *>(t);
      if (tobj && tobj->canWear(ITEM_THROW)) {
        *pos = -1;
        return tobj;
      }
    }
  }
  return NULL;
}

// ----------------------------------------------
int go_ok(roomDirData *exitp)
{
  return (!IS_SET(exitp->condition, EX_CLOSED | EX_LOCKED | EX_SECRET) &&
          (exitp->to_room != ROOM_NOWHERE));
}

int go_ok_smarter(roomDirData *exitp)
{
  return (!IS_SET(exitp->condition, EX_LOCKED | EX_SECRET) &&
          (exitp->to_room != ROOM_NOWHERE));
}


dirTypeT choose_exit_global(int in_room, int tgt_room, int depth)
{
  TPathFinder path(depth);

  return path.findPath(in_room, findRoom(tgt_room));
}
 
dirTypeT choose_exit_in_zone(int in_room, int tgt_room, int depth)
{
  TPathFinder path(depth);
  path.setStayZone(true);

  return path.findPath(in_room, findRoom(tgt_room));
}

int TBeing::doShoot(const char *arg)
{
  char arg1[128], arg2[128];
  TBeing *targ = NULL;
  int rc, iDist;
  dirTypeT dir;
  TThing *t;
  unsigned int count = 0;
  // Prevent: order elemental shoot <blah>
  // for Free xp.
  if ((!desc || (!isPc() && !orig)) && !(spec)) //added spec for the BM archers.. hopefully wont cause problems - Dash
    return FALSE;

  rc = sscanf(arg, "%s %s %d", arg2, arg1, &iDist);
  if (rc < 3) 
    iDist = 99;
  if (rc < 2)
    *arg1='\0';
  if (rc < 1 || rc > 3) {
    sendTo("Syntax : shoot <direction> <creature> <max-distance>\n\r");
    return FALSE;
  }

  dir = getDirFromChar(arg2);
  
  if (dir == DIR_NONE) {
    strcpy(arg1, arg2);
    dir = dirTypeT(::number(MIN_DIR, MAX_DIR-1));
  }
  if (checkPeaceful("You feel much too peaceful to contemplate violence.\n\r"))
    return FALSE;
  if (!(t = equipment[getPrimaryHold()])) {
    sendTo("You are not holding a bow to shoot!\n\r");
    return FALSE;
  }
  if (getSkillValue(SKILL_RANGED_PROF) <= 0 && !dynamic_cast<TGun *>(t)) {
    sendTo("You almost shoot yourself in the foot!\n\r");
    sendTo("You realize you don't have any clue what you're doing...get some training.\n\r");
    return FALSE;
  }
  if (arg1 && *arg1 &&
   !(targ = get_char_vis_direction(this, arg1, dir, iDist, TRUE, &count))) {
    sendTo("No creature with that name in that room.\n\r");
    sendTo("Syntax : shoot <direction> <creature> <max-distance>\n\r");
    return FALSE;
  }
  rc = t->shootMeBow(this, targ, count, dir, iDist);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete t;
    t = NULL;
  }
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_THIS;
  }

  // this is mainly for dual wielding guns, bows should be 2-handed
  if((t = equipment[getSecondaryHold()]) && 
     dynamic_cast<TObj *>(t) && !dynamic_cast<TObj *>(t)->usedAsPaired()){
    rc = t->shootMeBow(this, targ, count, dir, iDist);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete t;
      t = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      return DELETE_THIS;
    }
  }

  return TRUE;  // changed this to TRUE for success
}

// DELETE_THIS, DELETE_VICT(ch)
int TThing::shootMeBow(TBeing *ch, TBeing *, unsigned int, dirTypeT, int)
{
  act("$p isn't a bow!", FALSE, ch, this, 0, TO_CHAR);
  return FALSE;
}

int TBeing::unloadBow(const char *arg)
{
  TObj *arrow;
  TBow *bow = NULL;
  TThing *t;
 
  if (!(t = equipment[getPrimaryHold()]) ||
      !(bow = dynamic_cast<TBow *>(t)) ||
      !bow->getStuff() || !dynamic_cast<TArrow *>(bow->getStuff()))
    return FALSE;
 
  arrow = dynamic_cast<TObj *>(bow->getStuff());
  --(*arrow);
  sendTo("You uncock your bow and take out its arrow!\n\r");
  act("$n uncocks $s bow and takes out its arrow!", 
              TRUE, this, NULL, NULL, TO_ROOM);
  *this += *(unequip(getPrimaryHold()));
  *this += *arrow;
  return TRUE;
}
 
TThing * TBeing::findArrow(const char *buf, silentTypeT silent) const
{
  TThing *arrow;
  TQuiver *tQuiver;
  int     curPos;
  TThing  *tThing;

  arrow = searchLinkedListVis(this, buf, getStuff());
  if (!arrow) {
    for (curPos = MIN_WEAR; curPos < MAX_WEAR; curPos++) {
      if ((tQuiver = dynamic_cast<TQuiver *>(equipment[curPos])) &&
           !tQuiver->isClosed()) {
        if ((arrow = searchLinkedListVis(this, buf, tQuiver->getStuff()))) {
          if (!silent) {
            act("You pull $p from $N.",
                TRUE, this, arrow, tQuiver, TO_CHAR);
            act("$n pulls $p from $N.",
                TRUE, this, arrow, tQuiver, TO_ROOM);
          }
          return arrow;
        }
      }
    }
    for (tThing = getStuff(); tThing; tThing = tThing->nextThing) {
      if (!(tQuiver = dynamic_cast<TQuiver *>(tThing)) ||
           tQuiver->isClosed())
        continue;

      if (!(arrow = searchLinkedListVis(this, buf, tThing->getStuff())))
        continue;

      if (!silent) {
        act("You pull $p from $N.",
            TRUE, this, arrow, tQuiver, TO_CHAR);
      }
      return arrow;
    }

    return NULL;
  }
  return arrow;
}

void TBeing::doBload(const char *arg)
{
  char    arg1[128],
          arg2[128];
  TThing  *bow;
  TThing  *arrow;
 
  if (sscanf(arg, "%s %s", arg1, arg2) != 2) {
    sendTo("Syntax : bload <bow> <arrow>\n\r");
    return;
  }
  if (heldInPrimHand()) {
    if (!isname(arg1, heldInPrimHand()->name)) {
      sendTo(COLOR_OBJECTS, fmt("You would have a hard time firing your %s, while holding %s.\n\r") %            arg1 % sstring(heldInPrimHand()->getName()).uncap());
      return;
    } else {
      TThing *tObj = heldInPrimHand();

      *this += *(unequip(getPrimaryHold()));
      //--(*tObj);
      //*this += *tObj;
      sendTo(COLOR_OBJECTS, fmt("You remove %s to load it.\n\r") %
             sstring(tObj->getName()).uncap());
    }
  }
  if (heldInSecHand()) {
    sendTo(COLOR_OBJECTS, fmt("You would have a hard time firing your %s, while holding %s.\n\r") %          arg1 % sstring(heldInSecHand()->getName()).uncap());
    return;
  }
  if (!(bow = searchLinkedListVis(this, arg1, getStuff()))) {
    sendTo("Syntax : bload <bow> <arrow>\n\r");
    return;
  }

  if(dynamic_cast<TGun *>(bow)){
    sendTo("Use gload to load a gun.\n\r");
    return;
  } else {
    arrow = findArrow(arg2, SILENT_NO);

    if (arrow)
      arrow->bloadBowArrow(this, bow);
    else
      sendTo(fmt("You seem to have run out of '%s's.\n\r") % arg2);
  }
}

void TThing::bloadBowArrow(TBeing *ch, TThing *)
{
  ch->sendTo("You can only load your bow with arrows!\n\r");
  return;
}

void TThing::bloadArrowBow(TBeing *ch, TArrow *arrow)
{
  ch->sendTo("Arrows are usually loaded into bows!\n\r");
  return;
}
