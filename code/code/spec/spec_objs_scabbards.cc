#include <stdio.h>

#include "extern.h"
#include "being.h"
#include "obj.h"
#include "limbs.h"
#include "comm.h"
#include "parse.h"
#include "structs.h"
#include "db.h"
#include "log.h"
#include "immunity.h"
#include "enum.h"
#include "defs.h"
#include "spells.h"

int scabbardSpikedTitanium(TBeing*, cmdTypeT cmd, const char*, TObj* me, TObj*) {
  TBeing* tmp;
  wearSlotT limb;
  TObj* eq;

  if (cmd != CMD_GENERIC_PULSE)
    return FALSE;

  if (!(tmp = dynamic_cast<TBeing*>(me->equippedBy)))
    return FALSE;

  // must be worn on waist or back
  if (me->eq_pos != WEAR_WAIST && me->eq_pos != WEAR_BACK)
    return FALSE;

  // wearer must have toggle 223
  if (!tmp->hasQuestBit(223))
    return FALSE;

  // pick a random limb
  limb = pickRandomLimb();

  // check for equipment worn on that limb
  if (!(eq = dynamic_cast<TObj*>(tmp->equipment[limb])))
    return FALSE;

  // give that object the ITEM_SPIKED flag
  if (!eq->isObjStat(ITEM_SPIKED)) {
    eq->addObjStat(ITEM_SPIKED);
    act("$p on your $o suddenly sprouts wicked spikes!", FALSE, tmp, eq, me, TO_CHAR);
    act("$p on $n's $o suddenly sprouts wicked spikes!", FALSE, tmp, eq, me, TO_ROOM);
  }

  return TRUE;
}

// o is being hit, ch is o's owner, v is doing the hitting, with weapon
int wickedSpikes(TBeing* v, cmdTypeT cmd, const char*, TObj* o, TObj* weapon) {
  TBeing* ch;
  int rc, dam;
  wearSlotT slot;
  TObj* spike;
  char buf[256];

  if (cmd != CMD_OBJ_BEEN_HIT || !v || !o)
    return FALSE;

  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return FALSE;

  // pick a random limb
  for (slot = pickRandomLimb();; slot = pickRandomLimb()) {
    if (notBleedSlot(slot))
      continue;
    if (!v->slotChance(slot))
      continue;
    if (v->getStuckIn(slot))
      continue;
    break;
  }

  // create and embed a spike
  if (!(spike = read_object(13713, VIRTUAL))) {
    vlogf(LOG_PROC, "wickedSpikes couldn't load spike object 13713!");
    return FALSE;
  }

  dam = ::number(3, 12);

  sprintf(buf,
    "<k>A wicked spike breaks off from <1>$p<k> and embeds itself in <1>$n<k>'s <1>%s<k>!<1>",
    v->describeBodySlot(slot).c_str());
  act(buf, 0, v, o, 0, TO_ROOM);
  sprintf(buf,
    "<k>A wicked spike breaks off from <1>$p<k> and embeds itself in your <1>%s<k>!<1>",
    v->describeBodySlot(slot).c_str());
  act(buf, 0, v, o, 0, TO_CHAR);

  v->stickIn(spike, slot);

  if (!v->isImmune(IMMUNE_BLEED, slot)) {
    v->rawBleed(slot, 250, SILENT_YES, CHECK_IMMUNITY_NO);
    v->rawInfect(slot, 250, SILENT_YES, CHECK_IMMUNITY_NO);

    sprintf(buf, "<r>Blood<k> pours from the wound!<1>");
    act(buf, 0, v, o, 0, TO_ROOM);
    sprintf(buf, "<r>Blood<k> pours from the wound!<1>");
    act(buf, 0, v, o, 0, TO_CHAR);
  }

  rc = ch->reconcileDamage(v, dam, TYPE_STAB);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    // remove the ITEM_SPIKED flag and proc from the object before returning
    o->remObjStat(ITEM_SPIKED);
    o->spec = 0;
    return DELETE_VICT;
  }

  // remove the ITEM_SPIKED flag and proc from the object
  o->remObjStat(ITEM_SPIKED);
  o->spec = 0;

  return TRUE;
}
