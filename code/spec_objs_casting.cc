#include "stdsneezy.h"
#include "obj_money.h"
#include "obj_table.h"

int objCastFaerieFire(TObj *o, TBeing *targ)
{
  affectedData aff;
  if (targ->affectedBySpell(SPELL_FAERIE_FIRE))
  {
    act("$p just tried to cast something on you!", 0, NULL, o, targ, TO_VICT);
    return FALSE;
  }
  aff.type = SPELL_FAERIE_FIRE;
  aff.level = 50;
  aff.location = APPLY_ARMOR;
  aff.bitvector = 0;
  aff.duration = 10 * UPDATES_PER_MUDHOUR; // 4x longer than normal
  aff.modifier = 100 + (aff.level*4);
  targ->affectJoin(NULL, &aff, AVG_DUR_NO, AVG_EFF_YES);
  act("A faint pink outline puffs out around $N!",
      TRUE, targ, NULL, NULL, TO_ROOM);
  act("A faint pink outline puffs out around you!",
      FALSE, targ, NULL, NULL, TO_CHAR);  
  return TRUE;  
}

int objCastSorcGlobe(TObj *o, TBeing *targ)
{
  affectedData aff;
  if (targ->affectedBySpell(SPELL_SORCERERS_GLOBE))
  {
    act("$n just tried to cast something on you!", 0, o, NULL, targ, TO_VICT);
    return FALSE;
  }
  aff.type = SPELL_SORCERERS_GLOBE;
  aff.level = 50;
  aff.location = APPLY_ARMOR;
  aff.bitvector = 0;
  aff.duration = (3 + (aff.level / 2)) * UPDATES_PER_MUDHOUR;
  aff.modifier = -100;
  targ->affectJoin(NULL, &aff, AVG_DUR_NO, AVG_EFF_YES);

  targ->roomp->playsound(SOUND_SPELL_SORCERERS_GLOBE, SOUND_TYPE_MAGIC);
  act("$n is instantly surrounded by a hardened wall of air!",\
      TRUE, targ, NULL, NULL, TO_ROOM);
  act("You are instantly surrounded by a hardened wall of air!", \
      FALSE, targ, NULL, NULL, TO_CHAR);

  return TRUE;  
}

int marukalia(TBeing *targ, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  if (!cmd || !targ || !arg || !o)
    return FALSE;
  if (cmd != CMD_SAY && cmd != CMD_SAY2)
    return FALSE;
  if (!(dynamic_cast<TPerson *>(targ)))
    return FALSE;
  TTable *table = dynamic_cast<TTable *>(o);
  if (!table)
    return FALSE;

  sstring sarg = arg;
  TThing *t;
  TMoney *mon;
  vector <sstring> expected_contents;
  vector <sstring> contents;
  int talens = 0;
  int dmg;
  int rc = 0;
  
  // check what's on the item
  // 
  for (t = table->rider; t; t = t->nextRider)
  {
    if ((mon = dynamic_cast<TMoney *>(t)))
      talens += mon->getMoney();
    else
      contents.push_back(t->getName()); // list of names of items (shortDescr)
  }
  
  if (sarg.lower() == "sunday gravy")
  {
    act("<b>A faint blue aura flickers around $p.<z>", \
      TRUE, targ, o, NULL, TO_ROOM);
    act("<b>A faint blue aura flickers around $p.<z>", \
      TRUE, targ, o, NULL, TO_CHAR);
    if (talens > 1000)
      objCastSorcGlobe(o, targ);
    else
      objCastFaerieFire(o, targ);
  } else {
    // do random -10 hp target / +10 moves target
    dmg = ::number(5,15);
    if (::number(0,1)) {
      act("<B>A tongue of blue flame strikes out from $p and lashes $n.<z>", \
        TRUE, targ, o, NULL, TO_ROOM);
      act("<B>OUCH!  A tongue of blue flame strikes out from $p, burning you!<z>", \
        TRUE, targ, o, NULL, TO_CHAR);
      rc = targ->reconcileDamage(targ, dmg, DAMAGE_FIRE);
    } else {
      act("<b>A blue aura flickers around $p and reaches out to touch $n.<z>", \
        TRUE, targ, o, NULL, TO_ROOM);
      act("<b>A blue aura flickers around $p and reaches out to\n\rtouch you.\n\rYou feel refreshed.<z>", \
        TRUE, targ, o, NULL, TO_CHAR);
      targ->addToMove(dmg);
    }
  }
  act("<B>Blue flames erupt around $p.<z>", \
    TRUE, targ, o, NULL, TO_ROOM);
  act("<B>Blue flames erupt around $p.<z>", \
    TRUE, targ, o, NULL, TO_CHAR);
  // delete all items in container
  TThing *next = table->rider->nextRider;
  for (t = table->rider; t; t = next) {
    next = t->nextRider;
    act("<b>The flames incinerate $p.<z>", TRUE, targ, t, NULL, TO_CHAR);
    act("<b>The flames incinerate $p.<z>", TRUE, targ, t, NULL, TO_ROOM);
    delete t;
  }
  if (rc == -1)
    return DELETE_VICT;
  else
    return FALSE;
  
}

