////////////////////////////////////////////////////////////////////////// 
//
//      SneezyMUD++ - All rights reserved, SneezyMUD Coding Team
//
//      "newbie.cc" - All commands reserved for newbies
//  
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

void TBeing::doNewbieEqLoad(race_t num, ush_int num2, bool initNum)
{
  int r_num;
  TObj *obj;
  race_t raceNum;
  ush_int classNum;

  if (isImmortal() || !isPc()) {
    if (!initNum) {
      raceNum = num;
      classNum = num2;
    } else {
      raceNum = getRace();
      classNum = getClass();
    }
  } else if (desc) {
    if (spec && spec == SPEC_NEWBIE_EQUIPPER) {
      raceNum = num;
      classNum = num2;
    } else {
      raceNum = getRace();
      classNum = getClass();
    }
  } else if (spec && spec == SPEC_NEWBIE_EQUIPPER) {
    raceNum = num;
    classNum = num2;
  } else {
    vlogf(LOG_BUG, fmt("Something called doNewbieEqLoad when it shouldnt %s") %  getName());
    return;
  }

  if (raceNum == RACE_HOBBIT) {  // munchkin 
    if ((r_num = real_object(980)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj; 
    }
    if ((r_num = real_object(982)) >= 0) {
      obj = read_object(r_num, REAL);
      if (!canUseEquipment(obj, SILENT_YES)) {
        delete obj;
        obj = NULL;
      } else
        *this += *obj;
    }
    if ((r_num = real_object(105)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(983)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(984)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(985)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(986)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(987)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
#if 0
// munchkin boots, hobbits can't wear feet stuff
    if ((r_num = real_object(988)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
#endif
    if ((r_num = real_object(989)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1001)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if (((!(classNum & CLASS_MONK) && !initNum) || !hasClass(CLASS_MONK)) && (r_num = real_object(1009)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1010)) >= 0) {
      obj = read_object(r_num, REAL);
      if (!canUseEquipment(obj, SILENT_YES)) {
        delete obj;
        obj = NULL;
      } else
        *this += *obj;

    }
  } else if (raceNum == RACE_DWARF) {  // dwarven 
    if ((r_num = real_object(990)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(992)) >= 0) {
      obj = read_object(r_num, REAL);
      if (!canUseEquipment(obj, SILENT_YES)) {
        delete obj;
        obj = NULL;
      } else
        *this += *obj;
    }
    if ((r_num = real_object(105)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(993)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(994)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(995)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(996)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(997)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(998)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(999)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1001)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if (((!(classNum & CLASS_MONK) && !initNum) || !hasClass(CLASS_MONK)) && (r_num = real_object(1009)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1010)) >= 0) {
      obj = read_object(r_num, REAL);
      if (!canUseEquipment(obj, SILENT_YES)) {
        delete obj;
        obj = NULL;
      } else
        *this += *obj;

    }
  } else if (raceNum == RACE_OGRE) {  // ogre 
    if ((r_num = real_object(970)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(972)) >= 0) {
      obj = read_object(r_num, REAL);
      if (!canUseEquipment(obj, SILENT_YES)) {
        delete obj;
        obj = NULL;
      } else
        *this += *obj;
    }
    if ((r_num = real_object(105)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(973)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(974)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(975)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(976)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(977)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(978)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(979)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1001)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if (((!(classNum & CLASS_MONK) && !initNum) || !hasClass(CLASS_MONK)) && (r_num = real_object(1009)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1010)) >= 0) {
      obj = read_object(r_num, REAL);
      if (!canUseEquipment(obj, SILENT_YES)) {
        delete obj;
        obj = NULL;
      } else
        *this += *obj;

    }
  } else if (raceNum == RACE_GNOME) {       // horsehair stuff
    if ((r_num = real_object(960)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(962)) >= 0) {
      obj = read_object(r_num, REAL);
      if (!canUseEquipment(obj, SILENT_YES)) {
        delete obj;
        obj = NULL;
      } else
        *this += *obj;
    }
    if ((r_num = real_object(105)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(963)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(964)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(965)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(966)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(967)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(968)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(969)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1001)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if (((!(classNum & CLASS_MONK) && !initNum) || !hasClass(CLASS_MONK)) && (r_num = real_object(1009)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1010)) >= 0) {
      obj = read_object(r_num, REAL);
      if (!canUseEquipment(obj, SILENT_YES)) {
        delete obj;
        obj = NULL;
      } else
        *this += *obj;
    }
  } else if (raceNum == RACE_ELVEN) {       // rep cloth stuff
    if ((r_num = real_object(950)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(952)) >= 0) {
      obj = read_object(r_num, REAL);
      if (!canUseEquipment(obj, SILENT_YES)) {
        delete obj;
        obj = NULL;
      } else
        *this += *obj;
    }
    if ((r_num = real_object(105)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(953)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(954)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(955)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(956)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(957)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(958)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(959)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1001)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if (((!(classNum & CLASS_MONK) && !initNum) || !hasClass(CLASS_MONK)) && (r_num = real_object(1009)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1010)) >= 0) {
      obj = read_object(r_num, REAL);
      if (!canUseEquipment(obj, SILENT_YES)) {
        delete obj;
        obj = NULL;
      } else
        *this += *obj;
    }
  } else  {  // normal 
    if ((r_num = real_object(1000)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1002)) >= 0) {
      obj = read_object(r_num, REAL);
      if (!canUseEquipment(obj, SILENT_YES)) {
        delete obj;
        obj = NULL;
      } else
        *this += *obj;
    }
    if ((r_num = real_object(105)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1003)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1004)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1005)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1006)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1007)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1008)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1011)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1001)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if (((!(classNum & CLASS_MONK) && !initNum) || !hasClass(CLASS_MONK)) && (r_num = real_object(1009)) >= 0) {
      obj = read_object(r_num, REAL);
      *this += *obj;
      obj = read_object(r_num, REAL);
      *this += *obj;
    }
    if ((r_num = real_object(1010)) >= 0) {
      obj = read_object(r_num, REAL);
      if (!canUseEquipment(obj, SILENT_YES)) {
        delete obj;
        obj = NULL;
      } else
        *this += *obj;
    }
  }  // end of racial if-then equipment loading 
}
