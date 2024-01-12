#include "being.h"
#include "comm.h"
#include "liquids.h"
#include "obj.h"
#include "obj_base_weapon.h"
#include "parse.h"
#include "thing.h"

int tequilaCutlass(TBeing* vict, cmdTypeT cmd, const char* arg, TObj* o,
  TObj*) {
  TBaseWeapon* cutlass;
  TBeing* ch;

  if (cmd != CMD_GENERIC_PULSE)
    return false;

  if (::number(0, 19))
    return false;

  if (!(cutlass = dynamic_cast<TBaseWeapon*>(o)))
    return false;

  if (!(ch = dynamic_cast<TBeing*>(o->equippedBy)))
    return false;  // weapon not equipped (carried or on ground)

  cutlass->setPoison(LIQ_TEQUILA);

  act("<o>Tequila oozes from $n's $o and drips to the $g.<1>", 0, ch, o, 0,
    TO_ROOM);
  act(
    "<o>Your $o oozes tequila, which runs down the length of the blade and "
    "drips to the $g.<1>",
    0, ch, o, 0, TO_CHAR);

  ch->dropPool(1, LIQ_TEQUILA);

  return true;
}
