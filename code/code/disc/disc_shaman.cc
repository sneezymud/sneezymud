#include <stdio.h>

#include "handler.h"
#include "extern.h"
#include "person.h"
#include "disc_shaman.h"
#include "monster.h"
#include "being.h"  // for TBeing and related methods
#include "client.h"  // for Descriptor and related methods
#include "colorstring.h"  // for COLOR_OBJECTS, ANSI_RED, etc.
#include "combat.h"  // for act, start_task, etc.
#include "garble.h"  // for Garble class and methods
#include "obj_base_corpse.h"  // for TBaseCorpse and related methods
#include "obj_tool.h"  // for TTool and related methods
#include "spelltask.h"  // for taskDiffT, start_cast, etc.

TObj* TBeing::getWornShamanMask() {
  auto wornOnHead =
    equipment[WEAR_HEAD] ? dynamic_cast<TObj*>(equipment[WEAR_HEAD]) : nullptr;

  return wornOnHead && wornOnHead->spec &&
             strcmp(objSpecials[GET_OBJ_SPE_INDEX(wornOnHead->spec)].name,
               "Shaman's Totem Mask") == 0
           ? wornOnHead
           : nullptr;
}

TTool* TBeing::getHeldTotem() {
  auto toolInPrimary = dynamic_cast<TTool*>(heldInPrimHand());
  auto toolInSecondary = dynamic_cast<TTool*>(heldInSecHand());
  auto totemInPrimary =
    toolInPrimary && toolInPrimary->getToolType() == TOOL_TOTEM;
  auto totemInSecondary =
    toolInSecondary && toolInSecondary->getToolType() == TOOL_TOTEM;
  return totemInPrimary     ? toolInPrimary
         : totemInSecondary ? toolInSecondary
                            : nullptr;
}

void TBeing::doSacrifice(const char* arg) {
  for (; isspace(*arg); arg++)
    ;

  positionTypeT position = getPosition();

  if (position != POSITION_STANDING && position != POSITION_FLYING) {
    sendTo("Have some respect! Stand to perform the sacrifice!\n\r");
    return;
  }

  if (!doesKnowSkill(SKILL_SACRIFICE)) {
    sendTo("You don't have a clue about sacrificing anything.\n\r");
    return;
  }

  if (task) {
    sendTo("You're already busy doing something.\n\r");
    return;
  }

  if (!getWornShamanMask() && !getHeldTotem()) {
    sendTo("You must be holding a totem to perform the ritual.\n\r");
    return;
  }

  // Check to see if argument passed exists in room
  TBeing* dummy = nullptr;
  TObj* target = nullptr;
  if (!generic_find(arg, FIND_OBJ_ROOM, this, &dummy, &target)) {
    sendTo(format("You do not see a %s here.\n\r") % arg);
    return;
  }

  // Check to see if corpse is a corpse
  auto corpse = dynamic_cast<TBaseCorpse*>(target);
  if (!corpse) {
    sendTo(COLOR_OBJECTS,
      format("You cannot sacrifice %s.\n\r") % target->getName());
    return;
  }

  // Don't allow sacrificing corpses that are being butchered or skinned. If
  // sacrifice finishes before those skills a crash will likely occur.
  if (corpse->isCorpseFlag(CORPSE_PC_BUTCHERING | CORPSE_PC_SKINNING)) {
    act("That corpse is currently being... processed... by someone else.",
      false, this, corpse, nullptr, TO_CHAR);
    return;
  }

  if (corpse->isCorpseFlag(CORPSE_SACRIFICE)) {
    act("Someone must be sacrificing $p currently.", false, this, corpse,
      nullptr, TO_CHAR);
    return;
  }

  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    act("You aren't able to sacrifice $p.", FALSE, this, corpse, nullptr,
      TO_CHAR);
    return;
  }

  if (!corpse->isCorpseFlag(CORPSE_SACRIFICE))
    corpse->addCorpseFlag(CORPSE_SACRIFICE);

  sendTo("You start the sacrificial ritual.\n\r");
  act("$n begins to chant over a corpse.", false, this, nullptr, nullptr,
    TO_ROOM);
  start_task(this, corpse, nullptr, TASK_SACRIFICE, "", 2, inRoom(), 0, 0, 5);
}

int rombler(TBeing* caster, int, short bKnown) {
  Descriptor* i;
  sstring msg = caster->spelltask->orig_arg;
  sstring pgbuff;
  //  for (; isspace(*msg); msg++);

  if (caster->isPc() &&
      ((caster->desc && IS_SET(caster->desc->autobits, AUTO_NOSHOUT)) ||
        caster->isPlayerAction(PLR_GODNOSHOUT))) {
    caster->sendTo(
      "You aren't allowed to invoke the ritual drumming at this time!!\n\r");
    return SPELL_FAIL;
  }

  msg = caster->garble(NULL, msg, Garble::SPEECH_SHOUT, Garble::SCOPE_EVERYONE);

  if (caster->bSuccess(bKnown, SPELL_ROMBLER)) {
    if (msg.size() == 0) {
      caster->sendTo("Drumming without spirits to send is moot.\n\r");
      caster->nothingHappens(SILENT_YES);
    } else {
      caster->sendTo(COLOR_SPELLS,
        format("<g>You romble to the world, \"<z>%s<g>\"<z>\n\r") % msg);
      for (i = descriptor_list; i; i = i->next) {
        if (i->character && (i->character != caster) && !i->connected &&
            !i->character->checkSoundproof() &&
            (dynamic_cast<TMonster*>(i->character) ||
              (!IS_SET(i->autobits, AUTO_NOSHOUT)) ||
              !i->character->isPlayerAction(PLR_GODNOSHOUT))) {
          if (i->character->doesKnowSkill(SPELL_ROMBLER) ||
              i->character->isImmortal()) {
            pgbuff = caster->garble(i->character, msg, Garble::SPEECH_SHOUT,
              Garble::SCOPE_INDIVIDUAL);
            i->character->sendTo(COLOR_SPELLS,
              format("<Y>%s<z> rombles, \"<o>%s%s\"\n\r") % caster->getName() %
                pgbuff % i->character->norm());
            if (!i->m_bIsClient &&
                IS_SET(i->prompt_d.type, PROMPT_CLIENT_PROMPT))
              i->clientf(
                format("%d|%s|%s") % CLIENT_ROMBLER %
                colorString(i->character, i, caster->getName(), NULL,
                  COLOR_NONE, FALSE) %
                colorString(i->character, i, pgbuff, NULL, COLOR_NONE, FALSE));

          } else {
            int num = ::number(0, 3);
            if (num == 0) {
              i->character->sendTo(COLOR_SPELLS,
                "<p>In the faint distance you hear savage drumming.<z>\n\r");
            }
            if (num == 1) {
              i->character->sendTo(COLOR_SPELLS,
                "<o>Savage drumming can be heard in the distance.<z>\n\r");
            }
            if (num == 2) {}
            if (num == 3) {}
          }
        }
      }
      caster->addToMove(-5);
    }
    return SPELL_SUCCESS;
  } else {
    caster->nothingHappens();
    return SPELL_FAIL;
  }
}

int rombler(TBeing* caster, const char* msg) {
  taskDiffT diff;

  if (!bPassShamanChecks(caster, SPELL_ROMBLER, NULL))
    return FALSE;

  lag_t rounds = discArray[SPELL_ROMBLER]->lag;
  diff = discArray[SPELL_ROMBLER]->task;

  start_cast(caster, NULL, NULL, caster->roomp, SPELL_ROMBLER, diff, 1, msg,
    rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castRombler(TBeing* caster) {
  int ret, level;

  level = caster->getSkillLevel(SPELL_ROMBLER);
  int bKnown = caster->getSkillValue(SPELL_ROMBLER);

  if ((ret = rombler(caster, level, bKnown)) == SPELL_SUCCESS) {}
  return TRUE;
}

int embalm(TBeing* caster, TObj* corpse) {
  taskDiffT diff;

  if (!bPassMageChecks(caster, SPELL_EMBALM, corpse))
    return FALSE;

  lag_t rounds = discArray[SPELL_EMBALM]->lag;
  diff = discArray[SPELL_EMBALM]->task;

  start_cast(caster, NULL, corpse, caster->roomp, SPELL_EMBALM, diff, 2, "",
    rounds, caster->in_room, 0, 0, TRUE, 0);
  return TRUE;
}

int castEmbalm(TBeing* caster, TObj* corpse) {
  int level;

  level = caster->getSkillLevel(SPELL_EMBALM);

  embalm(caster, corpse, level, caster->getSkillValue(SPELL_EMBALM));

  return FALSE;
}

int embalm(TBeing* caster, TObj* o, int level, short bKnown) {
  TBaseCorpse* corpse;

  if (!caster->bSuccess(bKnown, SPELL_EMBALM)) {
    caster->nothingHappens();
    return SPELL_FAIL;
  }

  if (!(corpse = dynamic_cast<TBaseCorpse*>(o))) {
    act("$N is not a corpse!  You can only embalm corpses.", FALSE, caster,
      NULL, o, TO_CHAR);
    act("$n looks momentarily befuddled.", FALSE, caster, NULL, corpse,
      TO_ROOM);
    return SPELL_FAIL;
  }

  if (corpse->obj_flags.decay_time < 0) {
    act("$N is not decaying and would not benefit from embalming.", FALSE,
      caster, NULL, corpse, TO_CHAR);
    act("$n looks momentarily befuddled.", FALSE, caster, NULL, corpse,
      TO_ROOM);
    return SPELL_FAIL;
  }

  corpse->obj_flags.decay_time += (bKnown / 2);  // 1-50

  act("$N stiffens and takes on a rubbery appearance.", FALSE, caster, NULL,
    corpse, TO_CHAR, ANSI_RED);
  act("The steady march of the flesh devourers has been slowed.", FALSE, caster,
    NULL, corpse, TO_CHAR, ANSI_RED);
  act("$N stiffens and takes on a rubbery appearance.", FALSE, caster, NULL,
    corpse, TO_ROOM, ANSI_RED);
  act("The steady march of the flesh devourers has been slowed.", FALSE, caster,
    NULL, corpse, TO_ROOM, ANSI_RED);

  return SPELL_SUCCESS;
}
