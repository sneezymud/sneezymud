#include "room.h"
#include "low.h"
#include "handler.h"
#include "monster.h"
#include "obj_component.h"
#include "extern.h"
#include "disc_sorcery.h"
#include "disc_earth.h"
#include "disc_water.h"
#include "disc_afflictions.h"
#include "materials.h"

int vampire(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;

  if (ch->spelltask)
    return FALSE;

  if (ch->fight() && ch->sameRoom(*ch->fight())) {
    act("$n touches $N!", TRUE, ch, 0, ch->fight(), TO_NOTVICT);
    act("$n touches you in an attempt to suck away your energy!",
        TRUE, ch, 0, ch->fight(), TO_VICT);

    if (!ch->doesKnowSkill(SPELL_ENERGY_DRAIN))
      ch->setSkillValue(SPELL_ENERGY_DRAIN,120);

    return energyDrain(ch,ch->fight());
  }
  return FALSE;
}


int kraken(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *opp;

  if ((cmd != CMD_MOB_COMBAT) || !ch->awake())
    return FALSE;

  if ((opp = ch->fight()) && opp->sameRoom(*ch)) {
    act("$n sprays $N with an inky black cloud!", 
           1, ch, 0, opp, TO_NOTVICT);
    act("$n sprays you with an inky black cloud!", 1, ch, 0, opp, TO_VICT);
    if (!opp->affectedBySpell(SPELL_BLINDNESS)) {
      opp->sendTo("You have been BLINDED!\n\r");

      opp->rawBlind(myself->GetMaxLevel(), 
                    (myself->GetMaxLevel()/20 + 1) * UPDATES_PER_MUDHOUR,
                    SAVE_YES);
    }
    return TRUE;
  }
  return FALSE;
}

int ascallion(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int i;
  TBeing *vict;
  TMonster *mob;

  if ((cmd != CMD_MOB_COMBAT) || !me->awake())
    return FALSE;
  if (!(vict = me->fight()))
    return FALSE;
  if (!vict->sameRoom(*me))
    return FALSE;
  if (::number(0,10))
    return FALSE;

  act("$n spews forth young to protect her!",0, me, 0, 0, TO_ROOM);
  for (i = 0; i < dice(2,3);i++) {
    if (!(mob = read_mobile(Mob::ASCALLION,VIRTUAL))) {
      vlogf(LOG_PROC, "Bad mob in ascallion spec_proc");
      return FALSE;
    }
    *me->roomp += *mob;
    me->addFollower(mob);
    mob->reconcileDamage(vict,0,DAMAGE_NORMAL);
    mob->addHated(vict);
  }
  return TRUE;
}

int electricEel(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *vict;

  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;
  if (!(vict = myself->fight()))
    return FALSE;
  if (!vict->sameRoom(*myself))
    return FALSE;
  if (::number(0,10))
    return FALSE;

  int dam = dice(3,6);
  act("$n emits a shock into the water!", 0, myself, 0, 0, TO_ROOM);
  vict->sendTo("You've been fried!\n\r");
  act("$n has been fried!", 0, vict, 0, 0, TO_ROOM);

  if (myself->reconcileDamage(vict,dam, DAMAGE_ELECTRIC) == -1) {
    delete vict;
    vict = NULL;
  }

  return TRUE;
}

int poisonHit(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *vict;

  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;
  if (!(vict = myself->fight()))
    return FALSE;
  if (!vict->sameRoom(*myself))
    return FALSE;
  if (vict->isImmune(IMMUNE_POISON, WEAR_BODY))
    return FALSE;
  if (vict->affectedBySpell(SPELL_POISON))
    return FALSE;

  if (::number(1,10) > 3)
    return FALSE;

  affectedData aff;
  aff.type = SPELL_POISON;
  aff.level = myself->GetMaxLevel();
  aff.duration = (aff.level) * UPDATES_PER_MUDHOUR;
  aff.modifier = -20;
  aff.location = APPLY_STR;
  aff.bitvector = AFF_POISON;

  act("$n wraps $s suckers around $N!", 0, myself, 0, vict, TO_NOTVICT);
  act("$n wraps $s suckers around YOU!", 0, myself, 0, vict, TO_VICT);

  vict->sendTo("You've been poisoned!\n\r");
  vict->affectTo(&aff);

  return TRUE;
}

int poisonBite(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *vict;

  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;
  if (!(vict = myself->fight()))
    return FALSE;
  if (!vict->sameRoom(*myself))
    return FALSE;
  if (vict->isImmune(IMMUNE_POISON, WEAR_BODY))
    return FALSE;
  if (vict->affectedBySpell(SPELL_POISON))
    return FALSE;

  if (::number(0,9))
    return FALSE;

  affectedData aff;
  aff.type = SPELL_POISON;
  aff.level = myself->GetMaxLevel();
  aff.duration = (aff.level) * UPDATES_PER_MUDHOUR;
  aff.modifier = -20;
  aff.location = APPLY_STR;
  aff.bitvector = AFF_POISON;

  act("$n bares $s fangs and bites $N hard!", 0, myself, 0, vict, TO_NOTVICT);
  act("$n bares $s fangs and bites you hard!", 0, myself, 0, vict, TO_VICT);

  act("$N has been poisoned!", 0, myself, 0, vict, TO_NOTVICT);
  vict->sendTo("You've been poisoned!\n\r");
  vict->affectTo(&aff);

  return TRUE;
}

// These are {MobVNum, ToRoom, ChanceOfSwallow, ChanceOfDeath}
// if ChanceOfDeath == -1 then they cannot die from this mob.
const int SWALLOWER_TO_ROOM_PROC[][4] = {
  {12402, 13480, 33, 40},
  {27912, 28499, 80, -1}
};

// Increment if you add to the above.
const int MAX_SWALLOWER_TO_ROOM = 2;

int belimus(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int rc,
      targetSwallower = -1;
  TBeing *vict;
  TBeing *tmp;

  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;

  if (!(vict = myself->fight()))
    return FALSE;

  if (!vict->sameRoom(*myself))
    return FALSE;

  for (int swallowerIndex = 0; swallowerIndex < MAX_SWALLOWER_TO_ROOM; swallowerIndex++)
    if (SWALLOWER_TO_ROOM_PROC[swallowerIndex][0] == myself->mobVnum()) {
      targetSwallower = swallowerIndex;
      break;
    }

  if (targetSwallower == -1 ||
      targetSwallower >= MAX_SWALLOWER_TO_ROOM) {
    vlogf(LOG_PROC, format("Mobile in belimus() proc that isn't hard coded.  [%s] [%d]") % 
          myself->getName() % myself->mobVnum());
    return FALSE;
  }

  if (::number(0, 100) > SWALLOWER_TO_ROOM_PROC[targetSwallower][2])
    return FALSE;

  act("$n lunges at $N with $s maw gaping wide!",
       FALSE, myself, 0, vict, TO_NOTVICT);
  act("$n lunges at you with $s maw gaping wide!",
       FALSE, myself, 0, vict, TO_VICT);

  if (vict->isLucky(levelLuckModifier(30))) {
    vict->sendTo("Thank your deities, You leap aside at the last moment!\n\r");
    act("$n leaps aside at the last moment!", FALSE, vict, 0,0, TO_ROOM);
    return TRUE;
  }

  if (vict->getMyRace()->hasTalent(TALENT_FROGSLIME_SKIN) && ::number(0,1))
  {
    act("Your skin-poison defenses activate in the panic!", TRUE, vict, 0, myself, TO_CHAR);
    act("$N starts to choke on you and spits you out!", TRUE, vict, 0, myself, TO_CHAR);
    act("Oh wait!  That creature's skin tastes horrible!", TRUE, vict, 0, myself, TO_VICT);
    act("You spit out the little blighter.", TRUE, vict, 0, myself, TO_VICT);
    act("$N looks sick.", TRUE, vict, 0, myself, TO_ROOM);
    act("$N spits out $n in disgust.", TRUE, vict, 0, myself, TO_ROOM);
    return TRUE;
  }

  if (myself->fight())
    myself->stopFighting();
  if (vict->fight())
    vict->stopFighting();

  act("$n screams as $e is swallowed whole!", FALSE, vict, 0,0, TO_ROOM);

  tmp = dynamic_cast<TBeing *>(vict->riding);
  if (tmp) {
    vict->dismount(POSITION_STANDING);
    --(*tmp);
    thing_to_room(tmp, SWALLOWER_TO_ROOM_PROC[targetSwallower][1]);
    act("$n's mawed corpse arrives tumbling down $N's throat!",
        FALSE, tmp, 0, myself, TO_ROOM);
    vlogf(LOG_PROC, format("%s killed by belimus-swallow[%s] at %s (%d)") % 
          tmp->getName() % myself->getName() %
          tmp->roomp->getName() % tmp->inRoom());

    rc = tmp->die(DAMAGE_EATTEN);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete tmp;
      tmp = NULL;
    }
  }
  tmp = dynamic_cast<TBeing *>(vict->rider);
  if (tmp) {
    tmp->dismount(POSITION_STANDING);
    --(*tmp);
    thing_to_room(tmp, SWALLOWER_TO_ROOM_PROC[targetSwallower][1]);
    act("$n's mawed corpse arrives tumbling down $N's throat!",
        FALSE, tmp, 0, myself, TO_ROOM);
    tmp->rawKill(DAMAGE_EATTEN, myself);
    delete tmp;
    tmp = NULL;
  }
    
  --(*vict);
  thing_to_room(vict, SWALLOWER_TO_ROOM_PROC[targetSwallower][1]);

  vict->sendTo("You have been swallowed whole!!!!\n\r");

  if ((SWALLOWER_TO_ROOM_PROC[targetSwallower][3] != -1) &&
      (::number(1, 100) < SWALLOWER_TO_ROOM_PROC[targetSwallower][3])) {
    vict->sendTo(format("%s chomps down upon you, biting you in two!!!!\n\r") % myself->getName());
    act("$n's mawed corpse arrives tumbling down $N's throat!",
        FALSE, vict, 0, myself, TO_ROOM);
    vlogf(LOG_PROC, format("%s killed by Belimus-swallow[%s] at %s (%d)") % 
          vict->getName() % myself->getName() %

          vict->roomp->getName() % vict->inRoom());
    rc = vict->die(DAMAGE_EATTEN);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete vict;
      vict = NULL;
    }
    return TRUE;  
  }
  act("$n arrives tumbling down $N's throat screaming!",
        FALSE, vict, 0, myself, TO_ROOM);
  vict->doLook("", CMD_LOOK);
  return TRUE;
}

int ram(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;

  TBeing * vict = myself->fight();
  if (!vict)
    return FALSE;

  if (!vict->sameRoom(*myself))
    return FALSE;
  if (vict->riding)
    return FALSE;
  if (vict->getPosition() > POSITION_STANDING)
    return FALSE;
  if (::number(0,5))
    return FALSE;

  int damage = dice(5,6);

  act("$n lowers $s head and charges at you!", 
             FALSE, myself, 0, vict, TO_VICT);
  act("$n lowers $s head and charges at $N!",
             FALSE, myself, 0, vict, TO_NOTVICT);
  if (!vict->isLucky(levelLuckModifier(myself->GetMaxLevel()))) {
    act("$n slams $s head into your midriff.",
             FALSE, myself, 0, vict, TO_VICT);
    act("$n slams $s head into $N's midriff.",
             FALSE, myself, 0, vict, TO_NOTVICT);
    if (myself->reconcileDamage(vict, damage, DAMAGE_RAMMED) == -1) {
      delete vict;
      vict = NULL;
      return TRUE;
    }
    myself->cantHit += myself->loseRound(1);
    vict->cantHit += vict->loseRound(2);

    vict->setPosition(POSITION_SITTING);
  } else {
    act("You sidestep quickly, and $n thunders by.   TORO!",
             FALSE, myself, 0, vict, TO_VICT);
    act("$N sidesteps quickly, and $n thunders by $M.   TORO!",
             FALSE, myself, 0, vict, TO_NOTVICT);
  }
  return TRUE;
}

int paralyzeBreath(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *v;
  affectedData aff;
  affectedData aff2;
  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;

  if (!(v = myself->fight()) || !v->sameRoom(*myself)) 
    return FALSE;

  if (v->isAffected(AFF_PARALYSIS) || myself->checkForSkillAttempt(SPELL_PARALYZE)) {
    return FALSE;
  }

  act("$n spits out some noxious fumes at $N.", 
              TRUE, myself, NULL, v, TO_NOTVICT);
  act("$n spits out some noxious fumes at you!", TRUE, myself, NULL, v, TO_VICT);
  act("You spit some noxious fumes out at $N.", TRUE, myself, NULL, v, TO_CHAR);

  if (v->isImmune(IMMUNE_PARALYSIS, WEAR_BODY)) {
    act("Your immunity saves you.", false, v, 0, 0, TO_CHAR);
    act("$n's immunity saves $m.", false, v, 0, 0, TO_ROOM);
    return FALSE;
  }

  if(!v->isImmortal()){
    aff.type = SPELL_PARALYZE;
    aff.level = myself->GetMaxLevel();
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_PARALYSIS;

    // each update is a combat round long...
    
    aff.duration = min(10, number(1, aff.level));

    aff.modifier = 0;
    
    v->affectTo(&aff);
    // this should keep paralyze proc mobs from paralyzing the same person right when he wakes up 10-20-00 -dash
    aff2.type = AFFECT_SKILL_ATTEMPT;
    aff2.level = myself->GetMaxLevel();
    aff2.location = APPLY_NONE;
    aff2.bitvector = 0;
    aff2.duration = aff.duration + (::number(1,3)); //one round should be enough, might as well randomize it a ittle though
    aff2.modifier = SPELL_PARALYZE;
    myself->affectTo(&aff2);
  } else {
    v->sendTo("Good thing you are immortal.\n\r");
  }    

  return TRUE;
}

int paralyzeBite(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TBeing *v;
  affectedData aff;
  affectedData aff2;
  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;

  if (!(v = myself->fight()) || !v->sameRoom(*myself))
    return FALSE;

  if (v->isAffected(AFF_PARALYSIS)|| myself->checkForSkillAttempt(SPELL_PARALYZE)) {
    return FALSE;
  }

  act("You bite $N!", 1, myself, 0, v, TO_CHAR);
  act("$n bites you!", 1, myself, 0, v, TO_VICT);
  act("$n bites $N!", 1, myself, 0, v, TO_NOTVICT);

  if (v->isImmune(IMMUNE_PARALYSIS, WEAR_BODY)) {
    act("Your immunity saves you.", false, v, 0, 0, TO_CHAR);
    act("$n's immunity saves $m.", false, v, 0, 0, TO_ROOM);
    return FALSE;
  }

  if(!v->isImmortal()){
    aff.type = SPELL_PARALYZE;
    aff.level = myself->GetMaxLevel();
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_PARALYSIS;
    aff.duration = min(10, number(1, aff.level));
    aff.modifier = 0;
    
    v->affectTo(&aff);
    // this should keep paralyze proc mobs from paralyzing the same person right when he wakes up 10-20-00 -dash
    aff2.type = AFFECT_SKILL_ATTEMPT;
    aff2.level = myself->GetMaxLevel();
    aff2.location = APPLY_NONE;
    aff2.bitvector = 0;
    aff2.duration = aff.duration + (::number(1,3)); //one round should be enough, might as well randomize it a ittle though
    aff2.modifier = SPELL_PARALYZE;
    myself->affectTo(&aff2);
  } else {
    v->sendTo("Good thing you are immortal.\n\r");
  }  

  return TRUE;
}

// Arch does two drains at once!
int arch_vampire(TBeing *ch, cmdTypeT cmd, const char *, TMonster *, TObj *)
{
  if ((cmd != CMD_MOB_COMBAT) || !ch->awake())
    return FALSE;

  if (ch->spelltask)
    return FALSE;

  if (ch->fight() && ch->fight()->sameRoom(*ch)) {
    act("$n bites $N!", 1, ch, 0, ch->fight(), TO_NOTVICT);
    act("$n bites you!", 1, ch, 0, ch->fight(), TO_VICT);
    if (!ch->doesKnowSkill(SPELL_ENERGY_DRAIN))
      ch->setSkillValue(SPELL_ENERGY_DRAIN,2*ch->GetMaxLevel());

    energyDrain(ch,ch->fight());

#if 0
// energyDrain goes through spelltask stuff, so can't double cast it anymore
    if (ch->fight() && ch->fight()->sameRoom(*ch)) {
      energyDrain(ch,ch->fight());
    }
#endif

    return TRUE;
  }
  return FALSE;
}


int rust_monster(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int i;
  TBeing *tmp_ch;
  TThing *t;

  if ((cmd != CMD_MOB_COMBAT))
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (!me->roomp || me->roomp->isRoomFlag(ROOM_ARENA))
    return FALSE;

  for(StuffIter it=me->roomp->stuff.begin();it!=me->roomp->stuff.end();){
    t=*(it++);
    tmp_ch = dynamic_cast<TBeing *>(t);
    if (!tmp_ch)
      continue;
    if (tmp_ch->fight() == me) {
      for (i = MIN_WEAR; i < MAX_WEAR; i++) {
        TObj *eq = dynamic_cast<TObj *>(tmp_ch->equipment[i]);
        if (eq) {
          if (eq->canRust()) {
            if ((number(0, eq->getStructPoints()) < 10) &&
                 number(0,101) <= material_nums[eq->getMaterial()].acid_susc) {
              act("$n reaches out and touches your $o!", 
                         FALSE, me, eq, tmp_ch, TO_VICT);
              act("$n reaches out and touches $N's $o!", 
                         FALSE, me, eq, tmp_ch, TO_NOTVICT);
              act("$p is decayed by $N's rust!", FALSE, tmp_ch, eq, me, TO_ROOM);
              act("$p is decayed by $N's rust!", FALSE, tmp_ch, eq, me, TO_CHAR);
              eq->addToStructPoints(-1);
              if (eq->getStructPoints() <= 0) {
                if (!eq->makeScraps()){
                  delete eq;
		  eq = NULL;
		}
              }

              return TRUE;
            }
          }
        }
      }
    }
  }
  return FALSE;
}


int Fireballer(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TRoom *rp;
  TBeing *tmp, *temp;
  int dam;

  if (!me || !ch || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  rp = me->roomp;
  if (rp && rp->isUnderwaterSector()) {
    me->sendTo("The water completely dissolves your fireball!\n\r");
    return FALSE;
  }
  act("$n calls forth a mighty Djinn to do $s bidding.",TRUE,me,0,0,TO_ROOM);
  act("You call forth a mighty Djinn to smite your foes.",TRUE,me,0,0,TO_CHAR);

  for (tmp = character_list; tmp; tmp = temp) {
    temp = tmp->next;
    if (me->sameRoom(*tmp) && (me != tmp) &&
        !tmp->isImmortal() ) {
      if (!me->inGroup(*tmp)) {
        dam = dice(me->GetMaxLevel(), 4);
        me->reconcileHurt(tmp, 0.02);
        act("The Djinn breathes a gust of flame at you!",TRUE,tmp,0,0,TO_CHAR);
        act("The Djinn bathes $n in a breath of flame.",TRUE,tmp,0,0,TO_ROOM);
        if (tmp->isLucky(levelLuckModifier(me->GetMaxLevel()))) {
          act("....You are able to dodge the majority of the flame.",TRUE,tmp,0,0,TO_CHAR);
          dam /= 2;
        }
        if (me->reconcileDamage(tmp, dam, SPELL_FIRE_BREATH) == -1) {
          delete tmp;
          tmp = NULL;
        }
      }
    } else if (tmp->isImmortal() && me->sameRoom(*tmp)) {
      act("The Djinn chokes on a hairball.",TRUE,tmp,0,0,TO_CHAR);
      act("$n causes the Djinn to choke on a hairball before it can breathe at $m.",TRUE,tmp,0,0,TO_ROOM);
    } else if ((me != tmp) && (tmp->in_room != Room::NOWHERE) && (rp->getZoneNum() == tmp->roomp->getZoneNum())) {
      tmp->sendTo("You hear a loud explosion and feel a gust of hot air.\n\r");
    }
  }

  return TRUE;
}

int Teleporter(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int rc;
  TBeing *vict;

  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!(vict = me->fight()))
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n summons a Djinn to do $s bidding.",TRUE,me,0,0,TO_ROOM);
  act("You summon forth a Djinn.",TRUE,me,0,0,TO_CHAR);
  
  act("The Djinn looks in your direction and snaps his fingers!",TRUE,vict,0,0,TO_CHAR);
  act("The Djinn looks in $n's direction and snaps his fingers.",TRUE,vict,0,0,TO_ROOM);

  rc = vict->genericTeleport(SILENT_NO);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete vict;
    vict = NULL;
  }

  return TRUE;
}

int MSwarmer(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TObj *stone;
  int ret, rc=0;

  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;
  if (!me->fight())
    return FALSE;
  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n pulls a small stone out of $s robes.",TRUE,me,0,0,TO_ROOM);
  act("You pull a small stone out of your robes.",TRUE,me,0,0,TO_CHAR);

  act("$e throws it to the $g and looks skyward hopefully.",TRUE,me,0,0,TO_ROOM);
  act("You toss the stone to the $g in hopes of bringing forth meteors.",TRUE,me,0,0,TO_CHAR);

  me->setSkillValue(SPELL_METEOR_SWARM, 100);

  if (number(1,5) > 2) {
    ret = castMeteorSwarm(me, me->fight());
    if (IS_SET(ret, CASTER_DEAD)) {
      ADD_DELETE(rc, DELETE_THIS);
      return (rc + TRUE);
    }
  } else {
    stone = read_object(COMP_SKY_ROCK, VIRTUAL);
    *me->roomp += *stone;
    act("$n curses as $s spell fizzles.",TRUE,me,0,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  me->addSkillLag(SPELL_METEOR_SWARM, 0);
  return TRUE;
}

int IceStormer(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int ret, rc=0;

  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  if (!me->in_room || me->in_room == Room::NOWHERE)
    return FALSE;

  act("$n pulls a small cube of ice from out of a pouch.",TRUE,me,0,0,TO_ROOM);
  act("You pull a small cube of ice from out of a pouch.",TRUE,me,0,0,TO_CHAR);
  act("$e crushes it against $s belt and throws the flakes to the $g.",TRUE,me,0,0,TO_ROOM);
  act("You crush the flakes and throw them to the $g.",TRUE,me,0,0,TO_CHAR);

  me->setSkillValue(SPELL_ICE_STORM, 100);

  if (number(1,5) > 2) {
    ret = castIceStorm(me);
    if (IS_SET(ret, CASTER_DEAD)) {
      ADD_DELETE(rc, DELETE_THIS);
      return (rc + TRUE);
    }

  } else {
    *me->roomp += *read_object(COMP_ICEBERG_CORE, VIRTUAL);
    act("$n curses as $s spell fizzles.",TRUE,me,0,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  me->addSkillLag(SPELL_ICE_STORM, 0);
  return TRUE;
}

int Edrain(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int ret, rc = FALSE;
  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n utters some mysterious words and holds forth $s hand.",TRUE,me,0,0,TO_ROOM);
  act("You utter some dark words of power and hold forth your hand.",TRUE,me,0,0,TO_CHAR);
  act("A black orb rises from $n's hand - it spins straight up...",TRUE,me,0,0,TO_ROOM);
  act("You create a black orb and see it rise before you...",TRUE,me,0,0,TO_CHAR);
  act("A scathing beam of light from the orb sears $n!!",TRUE,me->fight(),0,0,TO_ROOM);
  act("A scathing beam of light from the orb sears you!!!",TRUE,me->fight(),0,0,TO_CHAR);

  me->setSkillValue(SPELL_ENERGY_DRAIN, 100);

  if (number(1,5)>2) {
    ret = castEnergyDrain(me, me->fight());
    if (IS_SET(ret, CASTER_DEAD)) {
      ADD_DELETE(rc, DELETE_THIS);
      return (rc + TRUE);
    }
  } else {
    act("$n curses as $s spell fizzles.",TRUE,me,0,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  me->addSkillLag(SPELL_ENERGY_DRAIN, 0);
  return TRUE;
}

int LBolter(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int ret, rc=0;
  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n utters some mysterious words and holds forth $s hand.",TRUE,me,0,0,TO_ROOM);
  act("You utter some dark words of power and hold forth your hand.",TRUE,me,0,0,TO_CHAR);
  act("A brilliant red orb crackling with power rises slowly from $s palm.",TRUE,me,0,0,TO_ROOM);
  act("A brilliant red orb crackling with power rises skyward.",TRUE,me,0,0,TO_CHAR);
  act("A mighty blast is unleashed from the orb aimed at $n.",TRUE,me->fight(),0,0,TO_ROOM);
  act("A mighty blast is unleashed from the orb at you!!!",TRUE,me->fight(),0,0,TO_CHAR);

  if (number(1,5)>2) {
    ret = castBlastOfFury(me, me->fight());
    if (IS_SET(ret, CASTER_DEAD)) {
      ADD_DELETE(rc, DELETE_THIS);
      return (rc + TRUE);
    }
  } else {
    *me->roomp += *read_object(COMP_BLOOD_SCORN_WOMAN,VIRTUAL);
    act("$n curses as $s spell fizzles.",TRUE,me,0,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  me->addSkillLag(SPELL_BLAST_OF_FURY, 0);
  return TRUE;
}

int Disser(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int rc=0, ret;

  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n begins to chant a incantation...",TRUE,me,0,0,TO_ROOM);
  act("You begin to chant a incantation...",TRUE,me,0,0,TO_CHAR);
  me->doSay("Ashes to ashes, dust to dust... Ashes to ashes, dust to dust...");
  act("$n points at $N ominously.",TRUE,me,0,me->fight(),TO_NOTVICT);
  act("$n points at you ominously.",TRUE,me,0,me->fight(),TO_VICT);
  act("You point at $N.",TRUE,me,0,me->fight(),TO_CHAR);

  me->setSkillValue(SPELL_ATOMIZE, 100);

  if (number(1,5)>2) {
    ret = atomize(me, me->fight());
    if (IS_SET(ret, CASTER_DEAD)) {
      ADD_DELETE(rc, DELETE_THIS);
      return (rc + TRUE);
    }

  } else {
    *me->roomp += *read_object(COMP_DRAGON_BONE,VIRTUAL);
    act("$n curses as $s spell fizzles.",TRUE,me,0,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  return TRUE;
}

int Witherer(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  TBeing *vict = me->fight();
  if (!vict)
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n begins to chant a mantra...",TRUE,me,0,0,TO_ROOM);
  act("You begin to chant a mantra...",TRUE,me,0,0,TO_CHAR);
  me->doSay("You put you left foot in... You take your left foot out...");
  act("$n points at $N ominously.",TRUE,me,0,vict,TO_NOTVICT);
  act("$n points at you ominously.",TRUE,me,0,vict,TO_VICT);
  act("You point at $N.",TRUE,me,0,vict,TO_CHAR);

  me->setSkillValue(SPELL_WITHER_LIMB, 100);

  if (number(1,5)>2) {
    int ret = witherLimb(me, vict);
    if (IS_SET_DELETE(ret, DELETE_VICT)) {
      delete vict;
      vict = NULL;
    }
  } else {
    act("$n curses as $s spell fizzles.",TRUE,me,0,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  return TRUE;
}

int Paralyzer(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  affectedData aff;
  TBeing *v;

  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!(v = me->fight()))
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  if (v->isAffected(AFF_PARALYSIS) || 
      v->isImmune(IMMUNE_PARALYSIS, WEAR_BODY)) {
    return FALSE;
  }

  act("$n utters something unintelligible...",TRUE,me,0,0,TO_ROOM);
  act("You utter something in a foreign tongue...",TRUE,me,0,0,TO_CHAR);
  me->doSay("  ...rigor mortis corpus feline a mehi inamici.. ");
  act("$n pulls a dead cat out of a bag and throws it at $N's feet.",
         TRUE,me,0,v,TO_NOTVICT);
  act("$n pulls a dead cat from a bag and throws it at your feet.",
         TRUE,me,0,v,TO_VICT);
  act("You throw a dead cat at $N's feet.",TRUE,me,0,v,TO_CHAR);

  act("Suddenly, the cat comes back to life!!",TRUE,me,0,0,TO_ROOM);
  act("Suddenly, the cat comes back to life!!",TRUE,me,0,0,TO_CHAR);

  aff.type = SPELL_PARALYZE;
  aff.level = me->GetMaxLevel();
  aff.location = APPLY_NONE;
  aff.bitvector = AFF_PARALYSIS;
  aff.duration = 2 * number(1, aff.level);
  aff.modifier = 0;
 
  v->affectTo(&aff);
  *me->roomp += *read_mobile(Mob::SMALL_CAT,VIRTUAL);

  return TRUE;
}

int AcidBlaster(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TObj *obj;
  int rc=0, ret;

  if (!me || cmd != CMD_MOB_COMBAT)
    return FALSE;

  if (!me->fight())
    return FALSE;

  if (me->getPosition() <= POSITION_SITTING)
    return FALSE;

  act("$n produces an opaque vial from inside $s robes.",TRUE,me,0,0,TO_ROOM);
  act("You produce an opaque vial from inside your robes.",TRUE,me,0,0,TO_CHAR);
  act("$n grins and heaves it in $N's direction.",TRUE,me,0,me->fight(),TO_ROOM);
  act("You grin and heave it at $N.",TRUE,me,0,me->fight(),TO_CHAR);

  me->setSkillValue(SPELL_ACID_BLAST, 100);

  if (number(1,5)>2) {
    act("The vial shatters and acid spews forth!!",TRUE,me,0,0,TO_ROOM);
    ret = acidBlast(me);
    if (IS_SET(ret, CASTER_DEAD)) {
      ADD_DELETE(rc, DELETE_THIS);
      return (rc + TRUE);
    }
  } else {
    obj = read_object(COMP_ACID_BLAST, VIRTUAL);
    *me->roomp += *obj;
    act("$p remains intact.",TRUE,me,obj,0,TO_ROOM);
    act("The spell fizzles  :(",TRUE,me,0,0,TO_CHAR);
  }
  return TRUE;
}

int kingUrik(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  int rc;
  TBeing *vict;

  if (!me)
    return FALSE;
  if ((cmd != CMD_MOB_COMBAT))
    return FALSE;
  if (!(vict = me->fight()))
    return FALSE;

  if (number(0,2))
    return FALSE;

  me->wait += 4;

  act("$n touches you, sucking your life energy.",TRUE,me,0,vict,TO_VICT);
  act("You touch $N and slurp some emergy.",TRUE,me,0,vict,TO_CHAR);
  act("$n touches $N and drains some life energy from $M.",TRUE,ch,0,vict,TO_NOTVICT);

  int dam = dice((me->GetMaxLevel()/2),3);
  if (!(dam = vict->getActualDamage(me,NULL,dam,DAMAGE_DRAIN))) {
    vict->sendTo("Fortunately, it has no effect on you.\n\r");
    return FALSE;
  }
  rc = me->applyDamage(vict,dam,DAMAGE_DRAIN);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete vict;
    vict = NULL;
    return TRUE;
  }

  vict->addToMana(-min(vict->getMana(),dice(12,4)));
  vict->addToMove(-dice(10,4));

  return TRUE;
}

int rock_worm(TBeing *ch, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TBeing *vict;

  if (!me)
    return FALSE;
  if ((cmd != CMD_MOB_COMBAT))
    return FALSE;
  if (!(vict = me->fight()))
    return FALSE;

  if (number(0,2))
    return FALSE;

  return TRUE;
}

int paralyzeGaze(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *){
  TBeing *v;
  affectedData aff;

  if ((cmd != CMD_MOB_COMBAT) || !myself->awake())
    return FALSE;

  if (!(v = myself->fight()) || !v->sameRoom(*myself)) 
    return FALSE;

  if (::number(0,10))
    return FALSE;

  act("$n fixes $N with a penetrating gaze.", 
              TRUE, myself, NULL, v, TO_NOTVICT);
  act("$n fixes you with a penetrating gaze.  Suddenly, you have trouble moving.", TRUE, myself, NULL, v, TO_VICT);
  act("You fix $N with a penetrating gaze.", TRUE, myself, NULL, v, TO_CHAR);

  if (v->isAffected(AFF_PARALYSIS) || 
      v->isImmune(IMMUNE_PARALYSIS, WEAR_BODY)) {
    act("Your immunity saves you.", false, v, 0, 0, TO_VICT);
    act("$n's immunity saves $m.", false, v, 0, 0, TO_ROOM);
    return FALSE;
  } else {
    act("Suddenly, $N has trouble moving!", TRUE, myself, NULL, v, TO_NOTVICT);
    act("Suddenly, you have trouble moving!", TRUE, myself, NULL, v, TO_VICT);
    act("Suddenly, $N has trouble moving!", TRUE, myself, NULL, v, TO_CHAR);
  }

  if(!v->isImmortal()){
    aff.type = SPELL_PARALYZE;
    aff.level = myself->GetMaxLevel();
    aff.location = APPLY_NONE;
    aff.bitvector = AFF_PARALYSIS;
    aff.duration = 1;
    aff.modifier = 0;
    
    v->affectTo(&aff);
  }

  return TRUE;

}

/* -------------------------------------------------------------------
  Target Dummy Proc (Pappy)
  the target dummy is used for target practice.  It should be used sparingly
  on mobs, since it auto-heals itself every round.
------------------------------------------------------------------- */
int targetDummy(TBeing *character, cmdTypeT cmd, const char *argIn, TMonster *myself, TObj *)
{
  if (!myself)
    return FALSE;

  static std::map<sstring, sessionData> s_beginningSession;

  sstring sargIn = argIn;

  // general cleanup code - don't allow us to fight real mobs!
  if (((cmd == CMD_MOB_COMBAT || cmd == CMD_MOB_COMBAT2) && myself->fight() && !myself->fight()->isPc()) ||
      (cmd == CMD_MOB_COMBAT_ONATTACKED && !character->isPc()))
  {
    myself->doSay("Magical error!  Recalibrate thaumaturgical plane conduits and restart incantation!");
    return DELETE_THIS;
  }

  // initialize logic
  if (cmd == CMD_GENERIC_INIT)
  {
    myself->setExp(0);
    return FALSE;
  }

  // we are ending practice combat
  if ((cmd == CMD_GENERIC_DESTROYED ||
      (cmd == CMD_GENERIC_PULSE && !myself->fight())) && !myself->procData.empty())
  {
    // restore agian, just in case
    myself->genericRestore(RESTORE_FULL);

    // we will wait for 4 generic ticks before printing status report (we use spaces as tick marks)
    if (cmd == CMD_GENERIC_PULSE && sstring::npos == myself->procData.find("    ", myself->procData.length()-4))
    {
      myself->procData += " ";
      return TRUE;
    }

    // clear hate list
    REMOVE_BIT(myself->specials.act, ACT_HUNTING);
    REMOVE_BIT(myself->specials.act, ACT_HATEFUL);
    while (myself->hates.clist)
      myself->remHated(NULL, myself->hates.clist->name);
    while (myself->fears.clist)
      myself->remFeared(NULL, myself->fears.clist->name);
    myself->setSusp(myself->defsusp());

    // get list of players to report to
    int cPlayers = myself->procData.trim().split(',', NULL);
    if (cPlayers < 1)
      return TRUE;

    sstring *players = new sstring[cPlayers];
    myself->procData.split(',', players);

    for(int iPlayer = 0;iPlayer < cPlayers;iPlayer++)
    {
      if (players[iPlayer].empty())
        continue;
      TBeing *player = get_pc_world(NULL, players[iPlayer], EXACT_YES, INFRA_YES, false);
      if (NULL == player)
        continue;

      // first, get the descriptor of our last target
      sessionData sdOriginal = s_beginningSession[players[iPlayer]];
      s_beginningSession[players[iPlayer]].setToZero();

      // then, compare with the original session data
      sessionData sdCompare = player->desc->session - sdOriginal;

      int roundstotalout = 0, roundstotalin = 0;
      int specialtotalout = 0, specialtotalin = 0, meleetotalout = 0, meleetotalin = 0;
      int swingstotalout = 0, swingstotalin = 0, hitstotalout = 0, hitstotalin = 0;
      int potentialdamin = 0, potentialdamout = 0;

      for(attack_mode_t iMode = ATTACK_NORMAL;iMode < MAX_ATTACK_MODE_TYPE; iMode++)
      {
        roundstotalout += sdCompare.rounds[iMode];
        roundstotalin += sdCompare.rounds_received[iMode];
        meleetotalout += sdCompare.combat_dam_done[iMode];
        meleetotalin += sdCompare.combat_dam_received[iMode];
        specialtotalout += sdCompare.skill_dam_done[iMode];
        specialtotalin += sdCompare.skill_dam_received[iMode];
        swingstotalout += sdCompare.swings[iMode];
        swingstotalin += sdCompare.swings_received[iMode];
        hitstotalout += sdCompare.hits[iMode];
        hitstotalin += sdCompare.hits_received[iMode];
        potentialdamout += sdCompare.potential_dam_done[iMode];
        potentialdamin += sdCompare.potential_dam_received[iMode];
      }

      float meleedprout = 0, specialdprout = 0, meleedprin = 0, specialdprin = 0;
      float dprout = 0, dprin = 0;
      int specialperout = 0, meleeperout = 0, specialperin = 0, meleeperin = 0;
      int hitperout = 0, hitperin = 0;

      if (roundstotalout > 0)
      {
        meleedprout = meleetotalout / roundstotalout;
        specialdprout = specialtotalout / roundstotalout;
      }
      if (roundstotalin > 0)
      {
        meleedprin = meleetotalin / roundstotalin;
        specialdprin = specialtotalin / roundstotalin;
      }
      dprout = meleedprout + specialdprout;
      dprin = meleedprin + specialdprin;
      if ((meleetotalout + specialtotalout) > 0)
      {
        specialperout = int((float(specialtotalout) / float(meleetotalout + specialtotalout)) * 100.0);
        meleeperout =  int((float(meleetotalout) / float(meleetotalout + specialtotalout)) * 100.0);
      }
      if ((meleetotalin + specialtotalin) > 0)
      {
        specialperin = int((float(specialtotalin) / float(meleetotalin + specialtotalin)) * 100.0);
        meleeperin =  int((float(meleetotalin) / float(meleetotalin + specialtotalin)) * 100.0);
      }
      if (swingstotalout > 0)
        hitperout = int((float(hitstotalout) / float(swingstotalout)) * 100.0);
      if (swingstotalin > 0)
        hitperin = int((float(hitstotalin) / float(swingstotalin)) * 100.0);

      // then, print it all out
      // total dam given, total received, rounds
      // DPR, hit %
      // APR, avoid %
      player->sendTo("You feel a magical voice in your head.  It says:\n\r");
      player->sendTo("Training session ending: Status report.\n\r");
      player->sendTo(format("You did %d damage total, received %d damage total over %d rounds.\n\r") % (meleetotalout + specialtotalout) % (meleetotalin + specialtotalin) % roundstotalout);
      player->sendTo(format("Average %.1f damage per round, %d% hit chance.\n\r") % dprout % hitperout);
      player->sendTo(format("Average %.1f damage taken per round, %d% of hits avoided.\n\r") % dprin % (100 - hitperin));
      if (sdCompare.hones > 0)
        player->sendTo(format("You also honed your combat skills %d times.\n\r") % sdCompare.hones);
    }

    // clear the targets list
    delete[] players;
    myself->procData.clear();

    return TRUE;
  }

  // our opponent is asking us to stop fighting
  if ((cmd == CMD_SAY || cmd == CMD_SAY2) && character == myself->fight() &&
    (sargIn.lower() == "stop" || sargIn.lower() == "halt"))
  {
    if (myself->fight() == character)
      myself->stopFighting();

    if (character && character->fight() == myself)
      character->stopFighting();

    // clear hate
    REMOVE_BIT(myself->specials.act, ACT_HUNTING);
    REMOVE_BIT(myself->specials.act, ACT_HATEFUL);
    while (myself->hates.clist)
      myself->remHated(NULL, myself->hates.clist->name);
    while (myself->fears.clist)
      myself->remFeared(NULL, myself->fears.clist->name);
    myself->setSusp(myself->defsusp());

    // set tick marks so we report right away
    myself->procData += "    ";

    myself->doSay("Dequeuing magical imperitave: Halting training session!  SIGTHAUM fault.");

    return FALSE; // returning true will cancel the speak
  }

  // we are beginning practice combat
  if ((cmd == CMD_MOB_COMBAT_ONATTACKED && sstring::npos == myself->procData.find(character->name)) ||
      ((cmd == CMD_MOB_COMBAT || cmd == CMD_MOB_COMBAT2) && myself->fight() && sstring::npos == myself->procData.find(myself->fight()->name)))
  {
    TBeing *attacker = (cmd == CMD_MOB_COMBAT_ONATTACKED) ? character : myself->fight();
    sstring attackerName = attacker->name;
    if (!attacker || !attacker->desc)
    {
      vlogf(LOG_PROC, "targetDummy proc being attacked by invalid attacker (mob)!");
      return FALSE;
    }

    // save the target
    if (!myself->procData.empty())
      myself->procData += ",";
    myself->procData += attacker->name;

    // save the session data
    s_beginningSession[attackerName] = attacker->desc->session;

    myself->doSay("Dequeuing magical imperitave: Training session beginning!");
    myself->doSay("Please issue the 'halt' or 'stop' commands to re-initialize thaumaturgical conduits.");
  }

  // anything other than combat?  just stop now
  if (cmd != CMD_MOB_COMBAT && cmd != CMD_MOB_COMBAT2)
    return FALSE;

  // clear out tick marks
  myself->procData = myself->procData.trim();

  // heal us up
  //reporting damage here can be misleading, so we'll not do that for now.
  int damage = myself->hitLimit() - myself->getHit();
  if (damage > 0)
  {
    act("The wounds on $n magically heal!", TRUE, myself, NULL, NULL, TO_ROOM);
    act("The wounds on your body completely heal!", TRUE, myself, NULL, NULL, TO_CHAR);
    myself->genericRestore(RESTORE_FULL);
  }
  damage = !myself->fight() ? 0 : myself->fight()->hitLimit() - myself->fight()->getHit();
  if (damage > 0 && myself->fight()->fight() == myself)
  {
    act("The wounds on $n magically heal!", TRUE, myself->fight(), NULL, NULL, TO_ROOM);
    act("The wounds on your body completely heal!", TRUE, myself->fight(), NULL, NULL, TO_CHAR);
    myself->fight()->genericRestore(RESTORE_FULL);
  }

  return TRUE; // returning true from here will cancel normal mob class-based moves
}



