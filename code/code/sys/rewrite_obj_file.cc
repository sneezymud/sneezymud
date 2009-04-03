//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "rewrite_obj_file.cc" - Some optional tack on code that
//       loads all the objects, and saves tehm out.
//        
//       The assumption here is that something was "corrected" during the
//       loading, and now we are saving it.
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_base_weapon.h"
#include "obj_base_cup.h"

void rewrite_obj_file()
{
  unsigned int iter;
  FILE *fp;
  fp = fopen("tiny_new.obj", "a+");
  if (!fp)
    return;
  for (iter = 0; iter < obj_index.size(); iter++) {
    TObj * obj = read_object(iter, REAL);
    if (!obj)
      continue;

    // fix problems...
#if 0
    // remove hit&dam
    int i;
    for (i=0; i < MAX_OBJ_AFFECT; i++ ) {
      if (obj->affected[i].location == APPLY_HITROLL ||
          obj->affected[i].location == APPLY_HITNDAM ||
          obj->affected[i].location == APPLY_DAMROLL) {
        obj->affected[i].location = APPLY_NONE;
        obj->affected[i].modifier = 0;
      }
    }    

#endif
#if 0
    TBaseWeapon *tbw = dynamic_cast<TBaseWeapon *>(obj);
    if (tbw) {
      // switch from XdY format to alternative
      int xx = tbw->getWeapDamLvl();
      double lvl = tbw->damageLevel();
      tbw->setWeapDamLvl((int) (lvl*4));
      int amt = 10 - xx;
      amt = min(max(0, amt), 10);
      tbw->setWeapDamDev(amt);
    }
#endif

    // strung items should be reverted back to norm
    if (obj->isObjStat(ITEM_STRUNG)) {
      obj->remObjStat(ITEM_STRUNG);

      delete [] obj->name;
      obj->name = obj_index[obj->getItemIndex()].name;

      delete [] obj->shortDescr;
      obj->shortDescr = obj_index[obj->getItemIndex()].short_desc;

      delete [] obj->getDescr();
      obj->setDescr(obj_index[obj->getItemIndex()].long_desc);

      delete [] obj->action_description;
      obj->action_description = obj_index[obj->getItemIndex()].description;
    }

    // drink containers : get rid of weight of the liquid in it
    TBaseCup *td = dynamic_cast<TBaseCup *>(obj);
    if (td) {
      td->setWeight(obj_index[td->getItemIndex()].weight);
    }

    // statues, otherobj, etc that allow light and are glow get extra effects
    // this undoes some things done in checkObjStats()
    if (obj->isObjStat(ITEM_GLOW)) {
      obj->canBeSeen += (1 + obj->getVolume()/1500);
      int i;
      int lamt = 1 + obj->getVolume()/6000;
      for (i=0; i < MAX_OBJ_AFFECT; i++ ) {
        if (obj->affected[i].location == APPLY_LIGHT && 
            obj->affected[i].modifier == lamt) {
          obj->affected[i].location = APPLY_NONE;
          obj->affected[i].modifier = 0;
          obj->addToLight(-lamt);
        }
      }
    }
    if (obj->isObjStat(ITEM_SHADOWY)) {
      obj->canBeSeen -= (1 + obj->getVolume()/1500);
      int i;
      int lamt = 1 + obj->getVolume()/6000;
      for (i=0; i < MAX_OBJ_AFFECT; i++ ) {
        if (obj->affected[i].location == APPLY_LIGHT && 
            obj->affected[i].modifier == -lamt) {
          obj->affected[i].location = APPLY_NONE;
          obj->affected[i].modifier = 0;
          obj->addToLight(lamt);
        }
      }
    }

    raw_write_out_object(obj, fp, obj->objVnum());

    delete obj;
  }
  fprintf(fp, "#99999\n$~\n");
  fclose(fp);

  return;
}
