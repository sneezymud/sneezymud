#include "being.h"
#include "parse.h"
#include "obj.h"

int blackSun(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
  affectedData aff;
  TBeing* ch;

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;

  if (::number(0, 99))
    return false;

  aff.type = SPELL_PLASMA_MIRROR;
  aff.duration = Pulse::UPDATES_PER_MUDHOUR * 2;
  aff.modifier = 0;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;
  aff.level = 40;

  colorAct(COLOR_SPELLS, "<k>Ancient visions of a dark land overcome you.<1>",
    false, ch, nullptr, 0, TO_CHAR);
  colorAct(COLOR_SPELLS,
    "<k>A black sun grows unbearably <1><Y>hot<1><k> and <1><r>burns<1><k> "
    "your flesh.<1>",
    false, ch, nullptr, 0, TO_CHAR);
  colorAct(COLOR_SPELLS,
    "<k>You snap back to reality gasping for breath, as a swirling shield of "
    "plasma surround you.<1>",
    false, ch, nullptr, 0, TO_CHAR);

  colorAct(COLOR_SPELLS,
    "<k>$n looks momentarily startled as a swirling shield of plasma surrounds "
    "$m.<1>",
    false, ch, nullptr, 0, TO_ROOM);

  ch->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES);

  return true;
}
