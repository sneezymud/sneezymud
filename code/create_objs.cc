
//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// create_objs.c : Online object creation/saving/loading for builders.
//
///////////////////////////////////////////////////////////////////////////

extern "C" {
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
}

#include <algorithm>

#include "stdsneezy.h"
#include "obj_component.h"
#include "dirsort.h"
#include "statistics.h"
#include "database.h"
#include "obj_open_container.h"
#include "obj_trash.h"
#include "obj_book.h"
#include "obj_trap.h"
#include "obj_portal.h"
#include "obj_arrow.h"
#include "obj_base_weapon.h"
#include "obj_gun.h"
#include "obj_base_clothing.h"
#include "obj_magic_item.h"
#include "obj_potion.h"
#include "obj_scroll.h"
#include "obj_staff.h"
#include "obj_wand.h"
#include "obj_egg.h"

static void update_obj_menu(const TBeing *ch, const TObj *obj)
{
  const char *obj_edit_menu_basic =
 "%s 1)%s Name                        %s 2)%s Short Description\n\r"
 "%s 3)%s Item Type                   %s 4)%s Long Description\n\r"
 "%s 5)%s Weight                      %s 6)%s Volume\n\r"
 "%s 7)%s Extra Flags                 %s 8)%s Take Flags\n\r"
 "%s 9)%s Unused                      %s10)%s Cost/Value\n\r"
 "%s11)%s Four Values                 %s12)%s Decay time\n\r"
 "%s13)%s Max struct points           %s14)%s Struct points\n\r"
 "%s15)%s Extra Description           %s16)%s Material type\n\r"
 "%s17)%s Applys                      %s18)%s Can be seen\n\r"
 "%s19)%s Delete all extra descs      %s20)%s Change Object Special Proc\n\r"
 "%s21)%s Set item max_exist\n\r"
 "\n\r";
  const char *obj_edit_menu_advanced =
 "%s 1)%s %-20s  %s 2)%s %s\n\r"
 "%s 3)%s %-20s  %s 4)%s %s\n\r"
 "%s 5)%s %sWeight%s: %-12s  %s 6)%s %sVolume%s: %s\n\r"
 "%s 7)%s %sExtra Flags%s           %s 8)%s %sTake Flags%s\n\r"
 "%s 9)%s %sUnused%s                %s10)%s %sCost%s: %s\n\r"
 "%s11)%s %sFour Values%s           %s12)%s %sDecay%s: %s\n\r"
 "%s13)%s %sMax-Str%s: %-11s  %s14)%s %sCur-Str%s: %s\n\r"
 "%s15)%s %sExtra Description%s     %s16)%s %s\n\r"
 "%s17)%s %sApplys%s                %s18)%s %sCBS%s: %s\n\r"
 "%s19)%s %sDelete extra descs%s    %s20)%s %s\n\r"
 "%s21)%s %sMax-Exist%s: %s\n\r"
 "\n\r";

  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt(VT_CURSPOS) % 1 % 1);
  ch->sendTo(fmt("%sObject Name:%s %s") % ch->cyan() % ch->norm() % obj->name);
  ch->sendTo(fmt(VT_CURSPOS) % 2 % 1);

  itemTypeT itt = obj->itemType();
  ch->sendTo(fmt("%sItem Type :%s %s") % ch->purple() % ch->norm() % ItemInfo[itt]->name);

  // this possibly adds item-specific stuff on line 3
  obj->objMenu(ch);

  ch->sendTo(fmt(VT_CURSPOS) % 6 % 1);
  ch->sendTo("Editing Menu:\n\r");

  if (IS_SET(ch->desc->autobits, AUTO_TIPS)) {
    char tStringOut[14][256];

    strcpy(tStringOut[0], (obj->name ? obj->name : "Unknown"));
    strcpy(tStringOut[1], (obj->shortDescr ? obj->shortDescr : "Unknown"));
    strcpy(tStringOut[2], ItemInfo[itt]->name);
    strcpy(tStringOut[3], (obj->descr ? obj->descr : "Unknown"));
    sprintf(tStringOut[4], "%.0f", obj->getWeight());
    sprintf(tStringOut[5], "%d", obj->getVolume());
    sprintf(tStringOut[6], "%d", obj->obj_flags.cost);
    sprintf(tStringOut[7], "%d", obj->obj_flags.decay_time);
    sprintf(tStringOut[8], "%d", obj->getMaxStructPoints());
    sprintf(tStringOut[9], "%d", obj->getStructPoints());
    strcpy(tStringOut[10], material_nums[obj->getMaterial()].mat_name);
    sprintf(tStringOut[11], "%d", obj->canBeSeen);
    strcpy(tStringOut[12], ((obj->spec < NUM_OBJ_SPECIALS) ? (obj->spec <= 0 ? "Proc: none" : objSpecials[GET_OBJ_SPE_INDEX(obj->spec)].name) : "Confused..."));
    sprintf(tStringOut[13], "%d", obj->max_exist);

    for (int tMsgIndex = 0; tMsgIndex < 14; tMsgIndex++)
      if (strlen(tStringOut[tMsgIndex]) > 20) {
        tStringOut[tMsgIndex][16] = '\0';
        strcat(tStringOut[tMsgIndex], "...");
      }

    ch->sendTo(fmt(obj_edit_menu_advanced) %
               ch->cyan()   % ch->norm() %                           tStringOut[0] %
               ch->purple() % ch->norm() %                           tStringOut[1] %
               ch->cyan()   % ch->norm() %                           tStringOut[2] %
               ch->purple() % ch->norm() %                           tStringOut[3] %
               ch->cyan()   % ch->norm() % ch->purple() % ch->norm() % tStringOut[4] %
               ch->purple() % ch->norm() % ch->cyan()   % ch->norm() % tStringOut[5] %
               ch->cyan()   % ch->norm() % ch->purple() % ch->norm() %
               ch->purple() % ch->norm() % ch->cyan()   % ch->norm() %
               ch->cyan()   % ch->norm() % ch->purple() % ch->norm() %
               ch->purple() % ch->norm() % ch->cyan()   % ch->norm() % tStringOut[6] %
               ch->cyan()   % ch->norm() % ch->purple() % ch->norm() %
               ch->purple() % ch->norm() % ch->cyan()   % ch->norm() % tStringOut[7] %
               ch->cyan()   % ch->norm() % ch->purple() % ch->norm() % tStringOut[8] %
               ch->purple() % ch->norm() % ch->cyan()   % ch->norm() % tStringOut[9] %
               ch->cyan()   % ch->norm() % ch->purple() % ch->norm() %
               ch->purple() % ch->norm() %                           tStringOut[10] %
               ch->cyan()   % ch->norm() % ch->purple() % ch->norm() %
               ch->purple() % ch->norm() % ch->cyan()   % ch->norm() % tStringOut[11] %
               ch->cyan()   % ch->norm() % ch->purple() % ch->norm() %
               ch->purple() % ch->norm() %                           tStringOut[12] %
               ch->cyan()   % ch->norm() % ch->purple() % ch->norm() % tStringOut[13]);
  } else
    ch->sendTo(fmt(obj_edit_menu_basic) %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm() % ch->purple() % ch->norm() %
               ch->cyan() % ch->norm());


  ch->sendTo("--> ");
}

// Loading/Saving functions are below 

void TBeing::doOEdit(const char *)
{
  sendTo("Mobs may not edit.\n\r");
}

void ObjLoad(TBeing *ch, int vnum)
{
  TObj *o;
  TBaseClothing *tbc;
  int i;
  extraDescription *new_descr;
  TDatabase db(DB_IMMORTAL);

  db.query("select type, name, short_desc, long_desc, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_struct, cur_struct, decay, volume, material, max_exist, action_desc from obj where vnum=%i and owner='%s'", vnum, ch->name);

  if(!db.isResults()){
    ch->sendTo("Object not found\n\r");
    return;
  }

  if(!db.fetchRow())
    return;

  ch->sendTo(fmt("Loading saved object number %d\n\r") % vnum);

  o = makeNewObj(mapFileToItemType(convertTo<int>(db["type"])));
  o->snum   = vnum;
  o->number = -1;

  o->name = mud_str_dup(db["name"]);
  o->shortDescr = mud_str_dup(db["short_desc"]);
  o->setDescr(mud_str_dup(db["long_desc"]));

  o->setObjStat(convertTo<int>(db["action_flag"]));
  o->obj_flags.wear_flags = convertTo<int>(db["wear_flag"]);

  o->assignFourValues(convertTo<int>(db["val0"]), convertTo<int>(db["val1"]), convertTo<int>(db["val2"]), convertTo<int>(db["val3"]));

  o->setWeight(convertTo<float>(db["weight"]));
  o->obj_flags.cost = convertTo<int>(db["price"]);
  o->canBeSeen = convertTo<int>(db["can_be_seen"]);
  o->spec = convertTo<int>(db["spec_proc"]);
  o->obj_flags.max_struct_points = convertTo<int>(db["max_struct"]);
  o->obj_flags.struct_points = convertTo<int>(db["cur_struct"]);
  o->obj_flags.decay_time = convertTo<int>(db["decay"]);
  o->setVolume(convertTo<int>(db["volume"]));
  o->setMaterial(convertTo<int>(db["material"]));
  o->max_exist = convertTo<int>(db["max_exist"]);
  if(!db["action_desc"].empty())
    o->action_description=mud_str_dup(db["action_desc"]);
  else o->action_description=NULL;

  o->ex_description = NULL;


  db.query("select name, description from objextra where vnum=%i and owner='%s'", vnum, ch->name);
  
  while(db.fetchRow()){
    new_descr = new extraDescription();
    new_descr->keyword = mud_str_dup(db["name"]);
    new_descr->description = mud_str_dup(db["description"]);
    new_descr->next = o->ex_description;
    o->ex_description = new_descr;
  }

  o->setLight(0);
  i=0;

  db.query("select type, mod1, mod2 from objaffect where vnum=%i and owner='%s'", vnum, ch->name);

  while(db.fetchRow()){
    o->affected[i].location = mapFileToApply(convertTo<int>(db["type"]));

    if (applyTypeShouldBeSpellnum(o->affected[i].location))
      o->affected[i].modifier = mapFileToSpellnum(convertTo<int>(db["mod1"]));
    else
      o->affected[i].modifier = convertTo<int>(db["mod1"]);
 
    o->affected[i].modifier2 = convertTo<int>(db["mod2"]);

    if (o->affected[i].location == APPLY_LIGHT)
      o->addToLight(o->affected[i].modifier);
    o->affected[i].type = TYPE_UNDEFINED;
    o->affected[i].level = 0;
    o->affected[i].bitvector = 0;

    o->affected[i].checkForBadness(o);
    i++;
  }

  for (i++; (i < MAX_OBJ_AFFECT); i++) {
    o->affected[i].location = APPLY_NONE;
    o->affected[i].modifier = 0;
    o->affected[i].modifier2 = 0;
    o->affected[i].type = TYPE_UNDEFINED;
    o->affected[i].level = 0;
    o->affected[i].bitvector = 0;
  }

  o->addObjStat(ITEM_STRUNG);
  if (!ch->hasWizPower(POWER_OEDIT_NOPROTOS))
    o->addObjStat(ITEM_PROTOTYPE);

  if(o->obj_flags.cost == -1){
    if((tbc=dynamic_cast<TBaseClothing *>(o))){
      o->obj_flags.cost = tbc->suggestedPrice();
    }
  }
  act("You just loaded $p.", TRUE, ch, o, 0, TO_CHAR);
  act(ch->msgVariables(MSG_OEDIT, o), TRUE, ch, 0, 0, TO_ROOM);

  *ch += *o;
}
static void ObjSave(TBeing *ch, TObj *o, int vnum)
{
  ch->sendTo("Saving.\n\r");

  int tmp1, tmp2, tmp3, tmp4;
  o->getFourValues(&tmp1, &tmp2, &tmp3, &tmp4);

  TDatabase db(DB_IMMORTAL);

  //  db.query("delete from obj where vnum=%i", vnum);
  if(!db.query("insert into obj (vnum, name, short_desc, long_desc, type, action_flag, wear_flag, val0, val1, val2, val3, weight, price, can_be_seen, spec_proc, max_exist, cur_struct, max_struct, decay, volume, material, owner, action_desc) values (%i, '%s', '%s', '%s', %i, %i, %i, %i, %i, %i, %i, %f, %i, %i, %i, %i, %i, %i, %i, %i, %i, '%s', '%s')", 
	  vnum, o->name, o->shortDescr, o->getDescr(),o->itemType(), 
	  o->getObjStat(), o->obj_flags.wear_flags, tmp1, tmp2, tmp3, tmp4, 
	  o->getWeight(), o->obj_flags.cost, o->canBeSeen, o->spec, 
	  o->max_exist, o->obj_flags.struct_points, 
	  o->obj_flags.max_struct_points, o->obj_flags.decay_time, 
		 o->getVolume(), o->getMaterial(), ch->name, 
	       o->action_description?o->action_description:"")){
    ch->sendTo("Unable to save object.  Make sure that an object doesn't already exist in that slot.\n\r");
    //    ch->sendTo("Database error!  Talk to a coder ASAP.\n\r");
    return;
  }

  db.query("delete from objextra where vnum=%i and owner='%s'", vnum, ch->name);

  int i, j, k;
  char temp[2048];
  extraDescription *exdes;
  for (exdes = o->ex_description; exdes; exdes = exdes->next) {
    j = 0;
    if (exdes->description) {
      for (k = 0; k <= (int) strlen(exdes->description); k++) {
	if (exdes->description[k] != 13)
	  temp[j++] = exdes->description[k];
      }
      temp[j] = '\0';

      if(!db.query("insert into objextra (name, description, owner, vnum) values ('%s', '%s', '%s', %i)", exdes->keyword, temp, ch->name, vnum)){
	ch->sendTo("Database error!  Talk to a coder ASAP.\n\r");
	return;
      }
    } else {
      if(!db.query("insert into objextra (name, description, owner, vnum) values ('%s', '', '%s')", exdes->keyword, ch->name, vnum)){
	ch->sendTo("Database error!  Talk to a coder ASAP.\n\r");
	return;
      }
    }
  }

  if(!db.query("delete from objaffect where vnum=%i and owner='%s'", vnum, ch->name)){
    ch->sendTo("Database error!  Talk to a coder ASAP.\n\r");
    return;
  }
  
  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
    if (o->affected[i].location == APPLY_LIGHT && o->canWear(ITEM_TAKE)) {
      ch->sendTo("Removing light affects on takeable item.\n\r");
      continue;
    }
    
    if (o->affected[i].location != APPLY_NONE) {
      if(!db.query("insert into objaffect (type, mod1, mod2, owner, vnum) values (%i, %i, %i, '%s', %i)",
		 mapApplyToFile(o->affected[i].location), 
		 applyTypeShouldBeSpellnum(o->affected[i].location) ? mapSpellnumToFile(spellNumT(o->affected[i].modifier)) : o->affected[i].modifier,
		 o->affected[i].modifier2, ch->name, vnum)){
	ch->sendTo("Database error!  Talk to a coder ASAP.\n\r");
	return;
      }
    }
  }

}

static void osave(TBeing *ch, const char *argument)
{
  char i, buf[80];
  int vnum;

  TObj *obj;

  for (i = 0; *(argument + i) == ' '; i++);
  if (!*(argument + i)) {
    ch->sendTo("Syntax :oedit save <object name> <vnum>\n\r");
    return;
  }
  if (sscanf(argument, "%s %d", buf, &vnum) != 2) {
    ch->sendTo("Syntax :oedit save <object name> <vnum>\n\r");
    return;
  }
  TThing *t_obj = searchLinkedListVis(ch, buf, ch->getStuff());
  obj = dynamic_cast<TObj *>(t_obj);

  if (obj && vnum)
    ObjSave(ch, obj, vnum);
  else {
    ch->sendTo("Syntax :oedit save <object name> <vnum>\n\r");
    return;
  }
}

static void olist(TPerson *ch, bool zone=false)
{
  sstring longstr;
  TDatabase db(DB_IMMORTAL);

  if(zone){
    db.query("select vnum, name from obj where owner='%s' and vnum>%i and vnum<=%i order by vnum", ch->name, zone_table[ch->roomp->getZone()->zone_nr-1].top, ch->roomp->getZone()->top);
  } else {
    db.query("select vnum, name from obj where owner='%s' order by vnum", ch->name);
  }


  if(!db.isResults()){
    ch->sendTo("Database error!  Talk to a coder ASAP.\n\r");
    return;
  }
    

  while(db.fetchRow()){
    longstr += db["vnum"];
    longstr += " ";
    longstr += db["name"];
    longstr += "\n\r";
  }

  ch->desc->page_string(longstr, SHOWNOW_NO, ALLOWREP_YES);
}


static void ocreate(TBeing *ch)
{
  TTrash *tmp_obj;

  tmp_obj = new TTrash();

  tmp_obj->name = mud_str_dup("dummy item");
  tmp_obj->shortDescr = mud_str_dup("a dummy item");
  tmp_obj->setDescr(mud_str_dup("A dummy item lies here."));

  tmp_obj->obj_flags.wear_flags = ITEM_TAKE;
  tmp_obj->obj_flags.decay_time = -1;
  tmp_obj->setObjStat(ITEM_PROTOTYPE);
  tmp_obj->obj_flags.cost = 1;
  tmp_obj->spec = 0;
  tmp_obj->max_exist = 9999;

  *ch += *tmp_obj;

  act("$n makes a dummy object.", TRUE, ch, tmp_obj, 0, TO_ROOM);
  ch->sendTo("You make a dummy object.\n\r");
}

static void oedit(TBeing *ch, const char *arg)
{
  TObj *o;

  if (!ch->isPc() || !ch->desc)
    return;

  TThing *t_obj = searchLinkedListVis(ch, arg, ch->getStuff());
  o = dynamic_cast<TObj *>(t_obj);
  if (!o) {
    ch->sendTo("You don't have such an object.\n\r");
    return;
  }
  if (o->getStuff() || o->rider) {
    ch->sendTo("You can't edit an object unless it's empty and standalone.\n\r");
    return;
  }

  if (!ch->limitPowerCheck(CMD_OEDIT, o->getSnum())) {
    ch->sendTo("You are not allowed to edit that object, sorry.\n\r");
    return;
  }



  if (!o->isObjStat(ITEM_PROTOTYPE) && !ch->hasWizPower(POWER_OEDIT_NOPROTOS)) {
    // Turn objects into prototype to curb cheating
    o->addObjStat(ITEM_PROTOTYPE);
    ch->sendTo("Making the item a prototype.\n\r");
  }
  o->swapToStrung();

  ch->specials.edit = MAIN_MENU;
  ch->desc->connected = CON_OEDITING;
  --(*o);
  ch->desc->obj = o;

  act("$n just went into object edit mode.", FALSE, ch, 0, 0, TO_ROOM);
  update_obj_menu(ch, ch->desc->obj);
}

void oremove(TBeing *ch, int vnum)
{
  TDatabase db(DB_IMMORTAL);
  
  db.query("select * from obj where vnum=%i and owner='%s'", vnum, ch->name);

  if(!db.isResults()){
    ch->sendTo("Object not found.\n\r");
    return;
  }

  if(!db.query("delete from obj where vnum=%i and owner='%s'", vnum, ch->name) ||
     !db.query("delete from objaffect where vnum=%i and owner='%s'", vnum, ch->name) ||
     !db.query("delete from objextra where vnum=%i and owner='%s'", vnum, ch->name)){
    ch->sendTo("Database error!  Talk to a coder ASAP.\n\r");
    return;
  } else
    ch->sendTo("Removed.\n\r");
}

// This is the main function that controls all the object stuff - Russ 
void TPerson::doOEdit(const char *argument)
{
  const char *tString = NULL;
  int vnum, field, zGot, oValue/*, ac_orig, str_orig, price_orig*/;
  float oFValue;
  TObj *cObj = NULL;
  sstring tStr;
  sstring tStString(""),
         tStBuffer(""),
         tStArg("");
  char sstring[256],
       object[80],
       Buf[256],
       tTextLns[4][256] = {"\0", "\0", "\0", "\0"};

  if (!hasWizPower(POWER_OEDIT)) {
    sendTo("You do not have the power to edit objects.\n\r");
    return;
  }

  bisect_arg(argument, &field, sstring, editor_types_oedit);

  switch (field) {
    case 21:
      if (!*sstring)
        sendTo("Syntax: oed resave <object>\n\r");
      else if (!(cObj = dynamic_cast<TObj *>(searchLinkedListVis(this, sstring, getStuff()))))
	sendTo(fmt("Unable to find %s...Sorry...\n\r") % sstring);
      else if (cObj->getSnum() == cObj->objVnum() && !hasWizPower(POWER_OEDIT_IMP_POWER))
        sendTo("Unknown value on this object.  resave only usable on oed loaded objects...\n\r");

      else if (!limitPowerCheck(CMD_OEDIT, cObj->getSnum()))
	sendTo("You are not allowed to oedit that object, sorry.\n\r");

      else {
        sprintf(sstring, "%s %d", sstring, cObj->getSnum());
	sendTo(fmt("Resaving in slot %i.\n\r") % cObj->getSnum());
	oremove(this, cObj->getSnum());
        osave(this, sstring);
      }
      return;
      break;
    case 1:			// save 
#if 1
      tStArg = sstring;
      tStString=tStArg.word(0);
      tStBuffer=tStArg.word(1);


      if (tStString.empty() || tStBuffer.empty())
        sendTo("Syntax: oed save <object> <vnum>\n\r");
      else {
        if (is_abbrev(tStBuffer, "resave")) {
          if (!hasWizPower(POWER_OEDIT_IMP_POWER))
            sendTo("Syntax: oed save <object> <vnum>\n\r");
          else if (!(cObj = dynamic_cast<TObj *>(searchLinkedListVis(this, tStString, getStuff()))))
            sendTo(fmt("Unable to find %s...Sorry...\n\r") % tStString);
          else if (cObj->getSnum() <= 0)
            sendTo("That object has a bad snum.  Sorry.  Can not resave.\n\r");

	  else if (!limitPowerCheck(CMD_OEDIT, cObj->getSnum()))
	    sendTo("You are not allowed to oedit that object, sorry.\n\r");

	  else {
            sprintf(sstring, "%s %d", tStString.c_str(), cObj->getSnum());

            osave(this, sstring);
#if 1
            --(*cObj);
            delete cObj;
            cObj = NULL;
#else
            doJunk(tStString.c_str(), cObj);
#endif
            doSave(SILENT_YES);
          }
        } else
          osave(this, sstring);
      }
#else
      // zGot, cObj, tString are additions for Mithros for:
      //   load obj 100
      //   **modify obj_100**
      //   oed save obj_100 resave
      //   for: oed save obj_100 100  followed by a  junk obj_100
      // Basically for doing large db changes online.
      if ((zGot = sscanf(sstring, "%s %d", object, &vnum)) != 2) {
        if (zGot == 1) {
          cObj = dynamic_cast<TObj *>(searchLinkedListVis(this, object, getStuff()));
          tString = one_argument(sstring, object);
          if (*tString) tString++;
        }
        if (!hasWizPower(POWER_OEDIT_IMP_POWER) || !cObj || (cObj->objVnum() < 0) ||
            !*tString || !is_abbrev(tString, "resave")) {
	  sendTo("Syntax : oed save <object> <vnum>\n\r");
	  return;
        } else
          sprintf(sstring, "%s %d", object, cObj->objVnum());
      }
      osave(this, sstring);
      if (zGot == 1) {
#if 1
        --(*cObj);
        delete cObj;
        cObj = NULL;
#else
        doJunk(object, cObj);
#endif
      }
      doSave(SILENT_YES);
#endif
      return;
      break;
    case 2:			// load 
      if (sscanf(sstring, "%d", &vnum) != 1) {
	// assume that sstring is an object name
	TDatabase db(DB_IMMORTAL);

	db.query("select vnum, name from obj where owner='%s'", getName());
  
	vnum=-1;
	while(db.fetchRow()){
	  if(isname(sstring, db["vnum"])){
	    vnum=convertTo<int>(db["name"]);
	    break;
	  }
	}

	if(vnum==-1){
	  sendTo("Syntax : oed load <vnum>\n\r");
	  return;
	}
      }

      ObjLoad(this, vnum);
      return;
      break;
    case 3:			// modify 
      if (sscanf(sstring, "%s", object) != 1) {
	sendTo("Syntax : oed modify <object name>\n\r");
	return;
      }
      oedit(this, sstring);
      return;
      break;
    case 4:			// list 
      sscanf(sstring, "%s", object);
      if(!strcmp(object, "zone"))
	olist(this, true);
      else
	olist(this, false);
      return;
      break;
    case 5:			// remove 
      if (sscanf(sstring, "%d", &vnum) != 1) {
	sendTo("Syntax : oed remove <vnum>\n\r");
	return;
      }
      oremove(this, vnum);
      return;
      break;
    case 6:			// create 
      ocreate(this);
      return;
      break;
  }

  // Make sure we are a real person, bad things could happen otherwise.
  if (!desc)
    return;

  tString = sstring;
  half_chop(tString, object, sstring);

  if (!(cObj = dynamic_cast<TObj *>(searchLinkedList(object, getStuff())))) {
    sendTo("Try an object next time, it works better.\n\r");
    return;
  }
  if (!hasWizPower(POWER_OEDIT_NOPROTOS))
    cObj->addObjStat(ITEM_PROTOTYPE);
  /* This is here in case it is needed.  From what I can tell this isn't
     actually used that often, but safty first.
    if (!cObj->isObjStat(ITEM_STRUNG))
      cObj->addObjStat(ITEM_STRUNG);
  */

  else if (!limitPowerCheck(CMD_OEDIT, cObj->getSnum())) {
    sendTo("You are not allowed to oedit that object, sorry.\n\r");
    return;
  }

  switch (field) {
    case 7: // Name
      if (!*sstring) {
        sendTo("You need to give me some keywords.\n\r");
        sendTo(fmt("Current keywords: %s\n\r") % cObj->name);
        return;
      }
      cObj->swapToStrung();
      if (cObj->name)
        delete [] cObj->name;
      cObj->name = mud_str_dup(sstring);
      return;
      break;
    case 8: // Long Description
      if (!*sstring) {
        sendTo("You need to give me a long description.\n\r");
        sendTo(fmt("Current Long is:\n\r%s\n\r") % cObj->descr);
        return;
      }
      cObj->swapToStrung();
      if (cObj->descr)
        delete [] cObj->descr;
      cObj->descr = mud_str_dup(sstring);
      return;
      break;
    case 9: // Short Description
      if (!*sstring) {
        sendTo("You need to give me a short description.\n\r");
        sendTo(fmt("Current Short is:\n\r%s\n\r") % cObj->shortDescr);
        return;
      }
      cObj->swapToStrung();
      if (cObj->shortDescr)
        delete [] cObj->shortDescr;
      cObj->shortDescr = mud_str_dup(sstring);
      return;
      break;
    case 10: // Max Structure Points
      if ((sscanf(sstring, "%d", &oValue)) != 1 ||
          oValue < -1 || oValue > 32767) {
        sendTo("Incorrect Max Structure, Must be between -1 and 32767.\n\r");
        return;
      }
      cObj->obj_flags.max_struct_points = oValue;
      return;
      break;
    case 11: // Cur Structure Points
      if ((sscanf(sstring, "%d", &oValue)) != 1 ||
          oValue < -1 || oValue > 32767) {
        sendTo("Incorrect Current Structure, Must be between -1 and 32767.\n\r");
        return;
      }
      cObj->obj_flags.struct_points = oValue;
      return;
      break;
    case 12: // Weight
      if ((sscanf(sstring, "%f", &oFValue)) != 1 ||
          oFValue < 0.0 || oFValue > 500000.0) {
        sendTo("Incorrect Weight, Must be between 0 and 500000.\n\r");
        return;
      }
      cObj->setWeight(oFValue);
      return;
      break;
    case 13: // Volume
      if ((sscanf(sstring, "%d", &oValue)) != 1 ||
          oValue < 0 || oValue > 50000) {
        sendTo("Incorrect Volume, Must be between 0 and 50000.\n\r");
        return;
      }
      cObj->setVolume(oValue);
      return;
      break;
    case 14: // Cost & Value
      if ((sscanf(sstring, "%d", &oValue)) != 1 ||
          oValue < 0 || oValue > 1000000) {
        sendTo("Incorrect Cost/Value, Must be between 0 and 1000000.\n\r");
        return;
      }
      if (hasWizPower(POWER_OEDIT_COST))
        cObj->obj_flags.cost = oValue;
      else
        sendTo("Do not worry about setting cost yourself until level 52.\n\r");
      return;
      break;
    case 15: // Decay Time
      if ((sscanf(sstring, "%d", &oValue)) != 1 ||
          oValue < -1 || oValue > 10000) {
        sendTo("Incorrect Decay Time, Must be between -1(never) and 10000.\n\r");
        return;
      }
      cObj->obj_flags.decay_time = oValue;
      return;
      break;
    case 16: // Extra Description
      extraDescription *ed;
      if (dynamic_cast<TBook *>(cObj)) {
        sendTo("Please don't add extra desciptions to books.\n\r");
        return;
      }
      cObj->swapToStrung();
      if (!*sstring) {
        sendTo("Assuming Object name for extra description.\n\r");
        strcpy(sstring, cObj->name);
      }
      for (ed = cObj->ex_description; ; ed = ed->next) {
        if (!ed) {
          ed = new extraDescription();
          ed->next = cObj->ex_description;
          cObj->ex_description = ed;
          ed->keyword = mud_str_dup(sstring);
          ed->description = NULL;
          desc->str = &ed->description;
          break;
        } else if (!strcasecmp(ed->keyword, sstring)) {
          sendTo(fmt("Extra already exists, Currently is:\n\r%s\n\r") % ed->description);
          delete [] ed->description;
        }
      }
      sendTo("Enter extra description.  Terminate with a '~' on a NEW line.\n\r");
      if (desc->m_bIsClient)
        desc->clientf(fmt("%d") % CLIENT_STARTEDIT % 4000);
      if (*desc->str)
        delete [] (*desc->str);
      *desc->str = 0;
      desc->max_str = MAX_INPUT_LENGTH;
      return;
      break;
    case 17: // Can Be Seen
      if ((sscanf(sstring, "%d", &oValue)) != 1 ||
          oValue < 0 || oValue > 25) {
        sendTo("Incorrect Can Be Seen, Must be between 0 and 25.\n\r");
        return;
      }
      cObj->canBeSeen = oValue;
      return;
      break;
    case 18: // Max Exist
      if ((sscanf(sstring, "%d", &oValue)) != 1 ||
          oValue < 0 || oValue > 9999) {
        sendTo("Incorrect Max Exist, Must be between 0 and 9999(Unlimited).\n\r");
        return;
      }
      cObj->max_exist = oValue;
      return;
      break;
    case 19:
#if 1
      if (!*sstring)
        sendTo("Syntax: oed average <object> <value>\n\r");
      else
        cObj->editAverageMe(this, sstring);
#else
      if ((sscanf(sstring, "%f", &oFValue)) != 1 || oValue < 0){
	sendTo("Syntax : oed default_ac_str <object> <level>\n\r");
	return;
      }
      TBaseClothing *tbc;
      if (!(tbc = dynamic_cast<TBaseClothing *>(cObj))) {
	sendTo("This only works on armor.\n\r");
	return;
      }
      str_orig   = tbc->getMaxStructPoints();
      ac_orig    = tbc->itemAC();
      price_orig = tbc->obj_flags.cost;
      tbc->setDefArmorLevel(oFValue);
      tbc->obj_flags.cost = tbc->suggestedPrice();
      sendTo(fmt("Modified AC by %i, Structure by %i and Price by %i.\n\r") %
	     (tbc->itemAC() - ac_orig), (tbc->getMaxStructPoints() - str_orig),
	     (tbc->obj_flags.cost - price_orig));
      sendTo(fmt("Real Level: %.2f  AC Level: %.2f   Str Level: %.2f\n\r") %
             tbc->armorLevel(ARMOR_LEV_REAL),
             tbc->armorLevel(ARMOR_LEV_AC),
             tbc->armorLevel(ARMOR_LEV_STR));
#endif
      return;
      break;
    case 20: // oedit replace <long/extra> <"extra"/"text"> <"text"> <"text">
      /*
      if (strcmp("Lapsos", getName())) {
        sendTo("Please don't use this option yet, it is still being tested.\n\r");
        return;
      }
      */

      strcpy(tTextLns[0], "[]A-Za-z0-9~`!@#$%&*()_+-={}[;\':,./<>? ]");
      sprintf(Buf, "%%s \"%%%s\" \"%%%s\" \"%%%s\"", tTextLns[0], tTextLns[0], tTextLns[0]);
      tTextLns[0][0] = '\0';

      vnum = sscanf(sstring, Buf, tTextLns[0], tTextLns[1], tTextLns[2], tTextLns[3]);

      if (((!is_abbrev(tTextLns[0], "long" )                    || vnum < 2) &&
           (!is_abbrev(tTextLns[0], "extra") || !tTextLns[2][0] || vnum < 3)) ||
          !tTextLns[1][0]) {
        sendTo("Syntax: oedit replace <long/extra> <\"extra\"/\"text\"> <\"text\"> <\"text\">\n\r");
        return;
      }

      cObj->swapToStrung();

      if (is_abbrev(tTextLns[0], "long")) {
        if (!cObj->descr) {
          sendTo("Object doesn't have a description, cannot use replace.\n\r");
          return;
        }

        tStr = cObj->descr;

        if (tStr.find(tTextLns[1]) == sstring::npos) {
          sendTo("Couldn't find pattern in long description.\n\r");
          return;
        }

        tStr.replace(tStr.find(tTextLns[1]), strlen(tTextLns[1]), tTextLns[2]);

        delete [] cObj->descr;
        cObj->descr = mud_str_dup(tStr);
      } else {
        for (ed = cObj->ex_description, zGot = 1; ed; ed = ed->next) {
          if (isname(tTextLns[1], ed->keyword)) {
            tStr = ed->description;
            zGot = 0;
          }

          if (zGot == 0)
            break;
        }

        if (!ed) {
          sendTo("Wasn't able to find an extra by that name.\n\r");
          return;
        }

        if (tStr.find(tTextLns[2]) == sstring::npos) {
          sendTo("Couldn't find pattern in extra description.\n\r");
          return;
        }

        tStr.replace(tStr.find(tTextLns[2]), strlen(tTextLns[2]), tTextLns[3]);

        delete [] ed->description;
        ed->description = mud_str_dup(tStr);
      }
      return;
      break;
    default:
      sendTo("Syntax : oed <type> <flags>\n\r");
      return;
      break;
  }
}

void TObj::writeAffects(int i, FILE *fp) const
{
  if (affected[i].location == APPLY_LIGHT && canWear(ITEM_TAKE))
    return;

  if (affected[i].location != APPLY_NONE) {
    fprintf(fp, "A\n%d %ld %ld\n", 
        mapApplyToFile(affected[i].location),
        applyTypeShouldBeSpellnum(affected[i].location) ? mapSpellnumToFile(spellNumT(affected[i].modifier)) : affected[i].modifier,
        affected[i].modifier2);
  }
}

void raw_write_out_object(const TObj *o, FILE *fp, unsigned int vnum)
{
  if (o->action_description)
    fprintf(fp, "#%d\n%s~\n%s~\n%s~\n%s~\n", vnum, o->name,
	  o->shortDescr, o->getDescr(), 
          o->action_description ? o->action_description : "");
  else 
    fprintf(fp, "#%d\n%s~\n%s~\n%s~\n~\n", vnum, o->name, 
           o->shortDescr, o->getDescr());
  fprintf(fp, "%d %d %d\n", mapItemTypeToFile(o->itemType()),
	  o->getObjStat(), o->obj_flags.wear_flags);

  int tmp1, tmp2, tmp3, tmp4;
  o->getFourValues(&tmp1, &tmp2, &tmp3, &tmp4);
  fprintf(fp, "%d %d %d %d\n",
          tmp1, tmp2, tmp3, tmp4);

  fprintf(fp, "%.1f %d %d %d %d\n", o->getWeight(),
	  o->obj_flags.cost, o->canBeSeen, o->spec, o->max_exist);
  fprintf(fp, "%d %d %d %d %d\n", o->obj_flags.struct_points,
	  o->obj_flags.max_struct_points, o->obj_flags.decay_time,
	  o->getVolume(), o->getMaterial());

  int i, j, k;
  char temp[2048];
  extraDescription *exdes;
  for (exdes = o->ex_description; exdes; exdes = exdes->next) {
    j = 0;
    if (exdes->description) {
      for (k = 0; k <= (int) strlen(exdes->description); k++) {
	if (exdes->description[k] != 13)
	  temp[j++] = exdes->description[k];
      }
      temp[j] = '\0';
      fprintf(fp, "E\n%s~\n%s~\n", exdes->keyword, temp);
    } else
      fprintf(fp, "E\n%s~\n~\n", exdes->keyword);
  }
  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
    o->writeAffects(i, fp);
  }
}

void TObj::objMenu(const TBeing *ch) const
{
}

static void change_obj_name(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  if (type != ENTER_CHECK)
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  if (type != ENTER_CHECK) {
    delete [] o->name;
    o->name = mud_str_dup(arg);
    ch->specials.edit = MAIN_MENU;
    update_obj_menu(ch, o);
    return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current Object Name: %s") % o->name);
  ch->sendTo("\n\r\n\rNew Object Name: ");
  return;
}

static void change_obj_long_desc(TBeing *ch, TObj *o, editorEnterTypeT type)
{
  if (type != ENTER_CHECK) {
    ch->specials.edit = MAIN_MENU;
    update_obj_menu(ch, o);
    return;
  }
  ch->sendTo(VT_HOMECLR);

  ch->sendTo("Current Object Long Description:\n\r");
  ch->sendTo(o->getDescr());
  ch->sendTo("\n\r\n\rNew Object Description:\n\r");
  ch->sendTo("(Terminate with a ~ on the SAME LINE. Press <ENTER> again to continue)\n\r");
  delete [] o->getDescr();
  o->setDescr(NULL);
  ch->desc->str = &o->descr;
  ch->desc->max_str = MAX_STRING_LENGTH;
  return;
}

static void change_obj_weight(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  float new_weight;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
    new_weight = convertTo<float>(arg);

    if (new_weight < 0.0 || new_weight > 500000.0) {
      ch->sendTo("Please enter a number from 0.1 to 500000.0\n\r");
      return;
    } else {
      o->setWeight(new_weight);
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current object weight: %.1f") % o->getWeight());
  ch->sendTo(fmt(VT_CURSPOS) % 4 % 1);
  ch->sendTo("Select a new weight.\n\r--> ");
}

static void change_obj_volume(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  int new_vol;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
    new_vol = convertTo<int>(arg);

    if (new_vol < 0 || new_vol > 50000) {
      ch->sendTo("Please enter a number from 1-50000.\n\r");
      return;
    } else {
      o->setVolume(new_vol);
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current object volume: %d") % o->getVolume());
  ch->sendTo(fmt(VT_CURSPOS) % 4 % 1);
  ch->sendTo("Select a new volume.\n\r--> ");
}

static void change_obj_short_desc(TBeing *ch, TObj *o, editorEnterTypeT type)
{
  if (type != ENTER_CHECK) {
    ch->specials.edit = MAIN_MENU;
    update_obj_menu(ch, o);
    return;
  }
  ch->sendTo(VT_HOMECLR);

  ch->sendTo("Current Object Short Description:\n\r");
  ch->sendTo(o->shortDescr);
  ch->sendTo("\n\r\n\rNew Object Short Description:\n\r");
  ch->sendTo("ALWAYS start the short description with a lowercase letter.\n\r");
  ch->sendTo("(Terminate with a ~ on the SAME LINE. Press <ENTER> again to continue)\n\r");
  delete [] o->shortDescr;
  o->shortDescr = NULL;
  ch->desc->str = &o->shortDescr;
//  ch->desc->max_str = MAX_STRING_LENGTH;
  ch->desc->max_str = MAX_NAME_LENGTH-1;
  return;
}

static void change_obj_type(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  itemTypeT update;
  char buf[256];

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  }
  int num = convertTo<int>(arg);
  num--;

  if (type != ENTER_CHECK) {
    if (num < ITEM_UNDEFINED || num >= MAX_OBJ_TYPES)
      return;

    update = itemTypeT(num);
  
    // disallow creating player corpses or "unknown" items
    if (update == ITEM_PCORPSE ||
        update == ITEM_MARTIAL_WEAPON ||
        update == ITEM_UNDEFINED) {
      ch->sendTo("You can't set that object type.\n\r");
      return;
    }

    // whacky stuff!
    // since changing the "type" mandates a change in class, we do this.
    // we essentially want to create a "new" item of the right type
    // and then move all the data members from TObj on down over
    // ie. the base TObj is the same, above that is the default constructor.
    TObj *o2;
    o2 = makeNewObj(update);
    *o2 = *o;  // intentional use of TObj assignment operator

    if (o2->lowCheckSlots(SILENT_YES)) {
      ch->sendTo("Too many wear slots would result!  Please fix first.\n\r");
      delete o2;
      return;
    }

    delete o;
    ch->desc->obj = o2;

    ch->specials.edit = MAIN_MENU;
    update_obj_menu(ch, o2);
    return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Item Type: %s") % ItemInfo[o->itemType()]->name);

  unsigned row = 0;
  itemTypeT i;
  for (i = MIN_OBJ_TYPE; i < MAX_OBJ_TYPES; i++) {
    sprintf(buf, VT_CURSPOS, row + 3, (((i % 3) * 25) + 5));
    if (!((i + 1) % 3))
      row++;
    ch->sendTo(buf);
    ch->sendTo(fmt("%2d %s") % (i+1) % ItemInfo[i]->name);
  }
  ch->sendTo(fmt(VT_CURSPOS) % 21 % 1);
  ch->sendTo("Select the number to set to, <ENTER> to return to main menu.\n\r--> ");
}

static void change_obj_cost(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  int new_cost;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
    new_cost = convertTo<int>(arg);

    if (new_cost < 0 || new_cost > 1000000) {
      ch->sendTo("Please enter a number from 1-1000000.\n\r");
      return;
    } else {
      o->obj_flags.cost = new_cost;
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current object cost: %d") % o->obj_flags.cost);
  ch->sendTo(fmt(VT_CURSPOS) % 4 % 1);
  ch->sendTo("Select a new cost.\n\r--> ");
}

static void change_obj_extra_flags(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  int row, update;
  unsigned int i;
  int j;
  char buf[256];

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  }
  update = convertTo<int>(arg);
  update--;

  if (type != ENTER_CHECK) {
    if (update < 0 || update >= MAX_OBJ_STAT)
      return;
    i = 1 << update;

    if (i == ITEM_STRUNG) {
      ch->sendTo("This bit gets set automatically, and should never need to be manually changed.\n\r"); 
      return;
    }
    if ((i == ITEM_PROTOTYPE) && !ch->hasWizPower(POWER_OEDIT_NOPROTOS)) {
      ch->sendTo("Prototype flag requires special powers.\n\r");
      return;
    }
    if (o->isObjStat(i))
      o->remObjStat(i);
    else
      o->addObjStat(i);
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo("Object extra flags:");

  row = 0;
  for (j = 0; j < MAX_OBJ_STAT; j++) {
    sprintf(buf, VT_CURSPOS, row + 4, ((j & 1) ? 45 : 5));
    if (j & 1)
      row++;
    ch->sendTo(buf);

    ch->sendTo(fmt("%2d [%s] %s") % (j + 1) % ((o->isObjStat(1 << j)) ? "X" : " ") % extra_bits[j]);
  }
  ch->sendTo(fmt(VT_CURSPOS) % 21 % 1);
  ch->sendTo("Select to number to toggle, <ENTER> to return to the main menu.\n\r--> ");
}

static void change_obj_wear_flags(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  int row, update;
  unsigned int i;
  char buf[256];

  if (type != ENTER_CHECK)
    if (!*arg || (*arg == '\n')) {
      if (o->lowCheckSlots(SILENT_YES)) {
        ch->sendTo("Too many wear slots defined!  Please fix.\n\r");
        return;
      }
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  update = convertTo<int>(arg);
  update--;

  if (type != ENTER_CHECK) {
    if (update < 0 || update >= (int) MAX_ITEM_WEARS)
      return;
    i = 1 << update;

    if (i == ITEM_WEAR_UNUSED1 || i == ITEM_WEAR_UNUSED2) {
      // unused bits
      ch->sendTo("Please do not set these bits.\n\r");
      return;
    }

    if (o->canWear(i))
      REMOVE_BIT(o->obj_flags.wear_flags, i);
    else
      SET_BIT(o->obj_flags.wear_flags, i);
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo("Object wear flags:");

  row = 0;
  for (i = 0; i < MAX_ITEM_WEARS; i++) {
    sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
    if (i & 1)
      row++;
    ch->sendTo(buf);

    ch->sendTo(fmt("%2d [%s] %s") % (i+1) % ((o->obj_flags.wear_flags & (1 << i)) ? "X" : " ") % wear_bits[i]);
  }
  ch->sendTo(fmt(VT_CURSPOS) % 21 % 1);
  ch->sendTo("Select to number to toggle, <ENTER> to return to the main menu.\n\r--> ");
}

static void change_obj_max_struct_points(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  int new_struct;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
    new_struct = convertTo<int>(arg);

    if (new_struct < -1 || new_struct > 32767) {
      ch->sendTo("Please enter a number from 1-32767.\n\r");
      return;
    } else {
      o->obj_flags.max_struct_points = new_struct;
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current object max structs: %d") % o->obj_flags.max_struct_points);
  ch->sendTo(fmt(VT_CURSPOS) % 4 % 1);
  ch->sendTo("Select a new max struct point.\n\r--> ");
}

static void change_obj_max_exist(TBeing *ch, TObj *obj, const char *arg, editorEnterTypeT type)
{
  int max_exist;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, obj);
      return;
    }
    max_exist = convertTo<int>(arg);
    if (max_exist < 0 || max_exist > 9999) {
      ch->sendTo("Please enter a number from 0 to 9999.\n\r");
      return;
    } else {
      obj->max_exist = max_exist;
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, obj);
      return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current max_exist: %d") %obj->max_exist);
  ch->sendTo(fmt(VT_CURSPOS) % 22 % 1);

  ch->sendTo("Select a new max_exist.\n\r--> ");
}

static void change_obj_applys(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  int row, apply, num, number1 = 0, number2 = 0;
  int done = FALSE;
  char buf[256];

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  }
  if (type != ENTER_CHECK) {
    num = sscanf(arg, "%d %d %d", &apply, &number1, &number2);

    // apply should be in range 1..MAX_APPLY
    if ((apply-1) < MIN_APPLY || (apply-1) >= MAX_APPLY_TYPES)
      return;

    //applyTypeT att = mapFileToApply(apply-1);
    applyTypeT att = applyTypeT(apply-1);

    if ((num != 2) && (num != 3)) {
      ch->sendTo("Syntax : <apply number> <arg>\n\r");
      return;
    }
    if (num == 3) {
      // offset the arrays for applies 
      if (att == APPLY_IMMUNITY)
        number1--;

      if (number1 < 0) {
        ch->sendTo("Invalid number.\n\r");
        return;
      }
    }
    if ((att == APPLY_IMMUNITY) && (num != 3)) {
      ch->sendTo(fmt("Syntax : %d <immunity #> <amount>\n\r") % (APPLY_IMMUNITY+1));
      return;
    } else if ((att == APPLY_SPELL) && (num != 3)) {
      ch->sendTo(fmt("Syntax : %d <skill #> <amount>\n\r") % (APPLY_SPELL + 1));
      return;
    } else if ((att == APPLY_DISCIPLINE) && (num != 3)) {
      ch->sendTo(fmt("Syntax : %d <discipline #> <amount>\n\r") % (APPLY_DISCIPLINE + 1));
      return;
    } else if ((att != APPLY_SPELL) &&
               (att != APPLY_DISCIPLINE) &&
               (att != APPLY_IMMUNITY) && 
         (num == 3) && (number2 != 0)) {
      ch->sendTo("Syntax : <apply number> <arg>\n\r");
      return;
    } else if ((att == APPLY_SPELL) && 
               (!discArray[number1] || !strcmp(discArray[number1]->name, ""))) {
      ch->sendTo("Illegal skill/spell!\n\r");
      return;
    } else if ((att == APPLY_DISCIPLINE) &&
         ((number1 < 0) || (number1 >= MAX_DISCS)  || 
          !(*discNames[number1].practice))) {
      ch->sendTo("Illegal Discipline!\n\r");
      return;
    } else if ((att == APPLY_IMMUNITY) && (number1 < 0 || number1 >= MAX_IMMUNES)) {
      ch->sendTo("Illegal immunity!\n\r");
      return;
    } else if (num == 2) 
      number2 = 0;

    if (!apply_types[att].assignable) {
      ch->sendTo("You can't set that number.\n\r");
      return;
    } else if ((att == APPLY_SPELL) &&!discArray[number1]) {
      ch->sendTo("Illegal skill/spell number.\n\r");
      return;
    } else if (o->addApply(ch, att)) 
      return;
    else {
      if (att == APPLY_LIGHT)
        o->addToLight(number1);

      int i;
      for (i = 0; (!done && (i < MAX_OBJ_AFFECT)); i++) {
	if (num == 2 && !number1) {
	  if (o->affected[i].location == att) {
	    o->affected[i].location = APPLY_NONE;
	    o->affected[i].modifier = 0;
            o->affected[i].modifier2 = 0;
	    done = TRUE;
	  }
        } else if (att == APPLY_ARMOR && o->affected[i].location == APPLY_ARMOR) {
	  o->affected[i].modifier = number1;
	  o->affected[i].modifier2 = number2;
	  done = TRUE;
	} else if (!number2 && num == 3) {
	  if (o->affected[i].location == att) {
	    o->affected[i].location = APPLY_NONE;
	    o->affected[i].modifier = 0;
            o->affected[i].modifier2 = 0;
	    done = TRUE;
	  }
	} else {
	  if (o->affected[i].location == APPLY_NONE) {
	    o->affected[i].location = att;
	    o->affected[i].modifier = number1;
            o->affected[i].modifier2 = number2;
	    done = TRUE;
	  }
	}
      }
      if (!done) {
	ch->sendTo("Sorry, you can only have 5 applys. Remove an apply if you want another.\n\r");
	return;
      }
      change_obj_applys(ch, o, "", ENTER_CHECK);
      return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  row = 0;

  applyTypeT att;
  unsigned int iter = 0;
  for (att = MIN_APPLY; att < MAX_APPLY_TYPES; att++) {
    if (!apply_types[att].assignable)
      continue;
    sprintf(buf, VT_CURSPOS, row + 2, (((iter % 4) * 20) + 1));
    if ((iter % 4) == 3)
      row++;
    iter++;

    ch->sendTo(buf);
    // ch->sendTo(fmt("%2d %s") % mapApplyToFile(att) + 1 % apply_types[att].name);
    ch->sendTo(fmt("%2d %s") % (att + 1) % apply_types[att].name);
  }
  ch->sendTo(fmt(VT_CURSPOS) % 15 % 1);

  int i;
  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
    if (o->affected[i].location == APPLY_SPELL) {
      ch->sendTo(fmt("Affects %s: %s (%d) by %d\n\r") %
        apply_types[o->affected[i].location].name %
        discArray[o->affected[i].modifier]->name % 
        o->affected[i].modifier %
        o->affected[i].modifier2);
    } else if (o->affected[i].location == APPLY_IMMUNITY) {
      ch->sendTo(fmt("Affects %s: %s (%d) by %d\n\r") %
        apply_types[o->affected[i].location].name %
        immunity_names[o->affected[i].modifier] % (o->affected[i].modifier+1) %
        o->affected[i].modifier2);
    } else if (o->affected[i].location == APPLY_ARMOR &&
               (dynamic_cast<TBaseClothing *>(o))) {
      TBaseClothing *aObj = dynamic_cast<TBaseClothing *>(o);
      ch->sendTo(fmt("Affects %s by %d  [Level:%.2f]\n\r") %
                 apply_types[o->affected[i].location].name % o->affected[i].modifier %
                 aObj->armorLevel(ARMOR_LEV_AC));
    } else {
      ch->sendTo(fmt("Affects %s by %d\n\r") %
          apply_types[o->affected[i].location].name %
          o->affected[i].modifier);
    }
  }
  ch->sendTo(fmt(VT_CURSPOS) % 20 % 1);
  ch->sendTo("Enter the number of apply and amount to apply seperated by a space.\n\r");
  ch->sendTo("To delete an apply, type the number and 0 seperated by a space.\n\r");
  ch->sendTo("For example enter 21 2 to give a +2 hitndam apply.\n\r");
  ch->sendTo("Press <ENTER> to return to main menu.\n\r--> ");
}

static void change_obj_struct_points(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  int new_struct;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
    new_struct = convertTo<int>(arg);

    if (new_struct < -1 || new_struct > 32767) {
      ch->sendTo("Please enter a number from 1-32767.\n\r");
      return;
    } else {
      o->obj_flags.struct_points = new_struct;
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current object structs: %d") % o->obj_flags.struct_points);
  ch->sendTo(fmt(VT_CURSPOS) % 4 % 1);
  ch->sendTo("Select a new struct point.\n\r--> ");
}

void change_obj_values(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  int update;
  int x1, x2, x3, x4;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  }
  update = convertTo<int>(arg);
  update--;

  o->getFourValues(&x1, &x2, &x3, &x4);

  if (type != ENTER_CHECK) {
    switch (ch->specials.edit) {
      case CHANGE_OBJ_VALUES:
        switch (update) {
	  case 0:
            o->changeObjValue1(ch);
            break;
	  case 1:
            o->changeObjValue2(ch);
            break;
	  case 2:
            o->changeObjValue3(ch);
            break;
	  case 3:
            o->changeObjValue4(ch);
            break;
	  default:
            break;
	}
        return;
      case CHANGE_OBJ_VALUE1:
	update++;
	if (update > ItemInfo[o->itemType()]->val0_max) {
	  ch->sendTo(fmt("Value 1 for this item type can't be over %d.\n\r") %
		ItemInfo[o->itemType()]->val0_max);
	  return;
	}
	if (update < ItemInfo[o->itemType()]->val0_min) {
	  ch->sendTo(fmt("Value 1 for this item type can't be under %d.\n\r") %
		ItemInfo[o->itemType()]->val0_min);
	  return;
	}
        o->assignFourValues(update, x2, x3, x4);
	ch->specials.edit = CHANGE_OBJ_VALUES;
	change_obj_values(ch, o, "", ENTER_CHECK);
	return;
      case CHANGE_OBJ_VALUE2:
	update++;
	if (update > ItemInfo[o->itemType()]->val1_max) {
	  ch->sendTo(fmt("Value 2 for this item type can't be over %d.\n\r") %
		ItemInfo[o->itemType()]->val1_max);
	  return;
	}
	if (update < ItemInfo[o->itemType()]->val1_min) {
	  ch->sendTo(fmt("Value 2 for this item type can't be under %d.\n\r") %
		ItemInfo[o->itemType()]->val1_min);
	  return;
	}
        if (o->changeItemVal2Check(ch, update))
          return;

        if (dynamic_cast<TScroll *>(o) ||
            dynamic_cast<TPotion *>(o)) {
          // update here is the spellNumT value, and shouldn't need mapping
          // unfortunately, assignFourValues is about to map it for us (like
          // we want for tinyfile values) so make a kludge for this
          update = mapSpellnumToFile(spellNumT(update));
        }

        o->assignFourValues(x1, update, x3, x4);
	ch->specials.edit = CHANGE_OBJ_VALUES;
	change_obj_values(ch, o, "", ENTER_CHECK);
	return;
      case CHANGE_OBJ_VALUE3:
	update++;
	if (update > ItemInfo[o->itemType()]->val2_max) {
	  ch->sendTo(fmt("Value 3 for this item type can't be over %d.\n\r") %
		ItemInfo[o->itemType()]->val2_max);
	  return;
	}
	if (update < ItemInfo[o->itemType()]->val2_min) {
	  ch->sendTo(fmt("Value 3 for this item type can't be under %d.\n\r") %
		ItemInfo[o->itemType()]->val2_min);
	  return;
	}
        if (o->changeItemVal3Check(ch, update))
          return;

        if (dynamic_cast<TScroll *>(o) ||
            dynamic_cast<TPotion *>(o)) {
          // update here is the spellNumT value, and shouldn't need mapping
          // unfortunately, assignFourValues is about to map it for us (like
          // we want for tinyfile values) so make a kludge for this
          update = mapSpellnumToFile(spellNumT(update));
        }

        o->assignFourValues(x1, x2, update, x4);
	ch->specials.edit = CHANGE_OBJ_VALUES;
	change_obj_values(ch, o, "", ENTER_CHECK);
	return;
      case CHANGE_OBJ_VALUE4:
	update++;
	if (update > ItemInfo[o->itemType()]->val3_max) {
	  ch->sendTo(fmt("Value 4 for this item type can't be over %d.\n\r") %
		ItemInfo[o->itemType()]->val3_max);
	  return;
	}
	if (update < ItemInfo[o->itemType()]->val3_min) {
	  ch->sendTo(fmt("Value 4 for this item type can't be under %d.\n\r") %
		ItemInfo[o->itemType()]->val3_min);
	  return;
	}
        if (o->changeItemVal4Check(ch, update))
          return;

        if (dynamic_cast<TScroll *>(o) ||
            dynamic_cast<TWand *>(o) ||
            dynamic_cast<TStaff *>(o) ||
            dynamic_cast<TPotion *>(o)) {
          // update here is the spellNumT value, and shouldn't need mapping
          // unfortunately, assignFourValues is about to map it for us (like
          // we want for tinyfile values) so make a kludge for this
          update = mapSpellnumToFile(spellNumT(update));
        }

        o->assignFourValues(x1, x2, x3, update);
	ch->specials.edit = CHANGE_OBJ_VALUES;
	change_obj_values(ch, o, "", ENTER_CHECK);
	return;
      default:
        return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(o->displayFourValues());
  /*
  ch->sendTo(fmt("Current values : %d %d %d %d") %
        x1, x2, x3, x4);
  */
  ch->sendTo(fmt(VT_CURSPOS) % 5 % 1);
  ch->sendTo("What the values mean :\n\r");
  ch->sendTo(fmt("1) %s\n\r2) %s\n\r3) %s\n\r4) %s\n\r") %
	ItemInfo[o->itemType()]->val0_info %
	ItemInfo[o->itemType()]->val1_info %
	ItemInfo[o->itemType()]->val2_info %
	ItemInfo[o->itemType()]->val3_info);

  ch->sendTo(fmt(VT_CURSPOS) % 20 % 1);
  ch->sendTo("Which value to change?\n\r--> ");
}

static void change_obj_decay(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  int new_decay;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
    new_decay = convertTo<int>(arg);

    if (new_decay < -1 || new_decay > 10000) {
      ch->sendTo("Please enter a number from 1-10000.\n\r");
      return;
    } else {
      o->obj_flags.decay_time = new_decay;
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current object decay time: %d") % o->obj_flags.decay_time);
  ch->sendTo(fmt(VT_CURSPOS) % 4 % 1);
  ch->sendTo("Select a new decay time.\n\r--> ");
}

static void change_obj_mat_type(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  int i, row, update;
  char buf[1024];

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
    update = convertTo<int>(arg);
    update--;

    switch (ch->specials.edit) {
      case CHANGE_OBJ_MAT_TYPE:
	if (update < 0 || update > 3)
	  return;
	else {
	  ch->sendTo(VT_HOMECLR);
	  switch (update) {
	    case 0:
	      ch->specials.edit = CHANGE_OBJ_MAT_TYPE1;
	      for (i = row = 0; i < MAX_MAT_GENERAL; i++) {
		sprintf(buf, VT_CURSPOS, row + 3, ((i & 1) ? 45 : 5));
		if (i & 1)
		  row++;
		ch->sendTo(buf);
		ch->sendTo(fmt("%2d %s") % (i+1) % material_nums[i].mat_name);
	      }
	      break;
	    case 1:
	      ch->specials.edit = CHANGE_OBJ_MAT_TYPE2;
	      for (i = row = 0; i < MAX_MAT_NATURE; i++) {
		sprintf(buf, VT_CURSPOS, row + 3, ((i & 1) ? 45 : 5));
		if (i & 1)
		  row++;
		ch->sendTo(buf);
		ch->sendTo(fmt("%2d %s") % (i+1) % material_nums[i+50].mat_name);
	      }
	      break;
	    case 2:
	      ch->specials.edit = CHANGE_OBJ_MAT_TYPE3;
	      for (i = row = 0; i < MAX_MAT_MINERAL; i++) {
		sprintf(buf, VT_CURSPOS, row + 3, ((i & 1) ? 45 : 5));
		if (i & 1)
		  row++;
		ch->sendTo(buf);
		ch->sendTo(fmt("%2d %s") % (i+1) % material_nums[i+100].mat_name);
	      }
	      break;
	    case 3:
	      ch->specials.edit = CHANGE_OBJ_MAT_TYPE4;
	      for (i = row = 0; i < MAX_MAT_METAL; i++) {
		sprintf(buf, VT_CURSPOS, row + 3, ((i & 1) ? 45 : 5));
		if (i & 1)
		  row++;
		ch->sendTo(buf);
		ch->sendTo(fmt("%2d %s") % (i+1) % material_nums[i+150].mat_name);
	      }
	      break;
	  }
	  ch->sendTo(fmt(VT_CURSPOS) % 20 % 1);
	  ch->sendTo("Enter a new material type.\n\r--> ");
	  return;
	}
      case CHANGE_OBJ_MAT_TYPE1:
	if (update < 0 || update >= MAX_MAT_GENERAL)
	  return;
	else {
	  o->setMaterial(update);
	  ch->specials.edit = MAIN_MENU;
	  update_obj_menu(ch, o);
	  return;
	}
      case CHANGE_OBJ_MAT_TYPE2:
	if (update < 0 || update >= MAX_MAT_NATURE)
	  return;
	else {
	  o->setMaterial(update + 50);
	  ch->specials.edit = MAIN_MENU;
	  update_obj_menu(ch, o);
	  return;
	}
      case CHANGE_OBJ_MAT_TYPE3:
	if (update < 0 || update >= MAX_MAT_MINERAL)
	  return;
	else {
	  o->setMaterial(update + 100);
	  ch->specials.edit = MAIN_MENU;
	  update_obj_menu(ch, o);
	  return;
	}
      case CHANGE_OBJ_MAT_TYPE4:
	if (update < 0 || update >= MAX_MAT_METAL)
	  return;
	else {
	  o->setMaterial(update + 150);
	  ch->specials.edit = MAIN_MENU;
	  update_obj_menu(ch, o);
	  return;
	}
      default:
        return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current material type : %s\n\r\n\r") % material_nums[o->getMaterial()].mat_name);
  for (i = 0; i <= 3; i++)
    ch->sendTo(fmt("%d) %s\n\r") % (i + 1) % material_groups[i]);

  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Which general group do you want?\n\r--> ");
}

static void change_obj_extra(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  extraDescription *ed, *prev;

  if (dynamic_cast<TBook *>(o)) {
    ch->sendTo("Please don't add extra descriptions to books.\n\r");
    ch->specials.edit = MAIN_MENU;
    update_obj_menu(ch, o);
    return;
  }

  if (type != ENTER_CHECK) {
    if (!arg || !*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
    for ( prev = ed = o->ex_description;; prev = ed, ed = ed->next) {
      if (!ed) {
        ed = new extraDescription();
	ed->next = o->ex_description;
	o->ex_description = ed;
        ed->keyword = mud_str_dup(arg);
	ed->description = NULL;
	ch->desc->str = &ed->description;
	ch->sendTo("Enter the description. Terminate with a '~' on a NEW line.\n\r");
	break;
      } else if (!strcasecmp(ed->keyword, arg)) {
        ch->sendTo(fmt("Current description:\n\r%s\n\r") % ed->description);
        ch->sendTo("This description has been deleted.  If you needed to modify it, simply readd it.\n\r");
        ch->sendTo("Press return to proceed.\n\r");
        if (prev == ed) {
          o->ex_description = ed->next;
          delete ed;
        } else {
          prev->next = ed->next;
          delete ed;
        }
	return;
      }
    }
    ch->desc->max_str = MAX_STRING_LENGTH;
    return;
  }
  ch->sendTo("Existing keywords:\n\r");
  for ( ed = o->ex_description;ed ; ed = ed->next) {
    ch->sendTo(fmt("%s\n\r") % ed->keyword);
  }
  ch->sendTo("\n\rEnter the keyword for the extra description.\n\r--> ");
  ch->specials.edit = CHANGE_OBJ_EXDESC;
  return;
}

static void change_obj_can_be_seen(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  int new_seen;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
    new_seen = convertTo<int>(arg);

    if (new_seen < 0 || new_seen > 25) {
      ch->sendTo("Please enter a number from 0-25.\n\r");
      return;
    } else {
      o->canBeSeen = new_seen;
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current object can be seen: %d") % o->canBeSeen);
  ch->sendTo(fmt(VT_CURSPOS) % 4 % 1);
  ch->sendTo("Select a new can be seen.\n\r--> ");
}

void delete_obj_extra_desc(TBeing *ch, TObj *o, const char *, editorEnterTypeT)
{
  extraDescription *exptr, *nptr;

  if (o->ex_description) {
    for (exptr = o->ex_description; exptr; exptr = nptr) {
      nptr = exptr->next;
      delete exptr;
    }
    o->ex_description = NULL;
  }
  update_obj_menu(ch, o);
}

static void change_obj_spec(TBeing *ch, TObj *obj, const char *arg, editorEnterTypeT type)
{
  char buf[256];
  int row, j, i, new_spec;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, obj);
      return;
    }
    new_spec = convertTo<int>(arg);
    if (new_spec < 0 || new_spec > NUM_OBJ_SPECIALS) {
      ch->sendTo(fmt("Please enter a number from 0 to %d.\n\r") % NUM_OBJ_SPECIALS);
      return;
    } else if (!objSpecials[new_spec].assignable && !ch->hasWizPower(POWER_OEDIT_IMP_POWER)) {
      ch->sendTo("That spec_proc has been deemed unassignable by builders sorry.\n\r");
      return;
    } else {
      obj->spec = new_spec;
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, obj);
      return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current obj spec: %s") % ((obj->spec) ? objSpecials[GET_OBJ_SPE_INDEX(obj->spec)].name : "none"));
  row = 0;
  for (i = 1, j=1; i <= NUM_OBJ_SPECIALS; i++) {
    if (!objSpecials[i].assignable)
      continue;
    sprintf(buf, VT_CURSPOS, row + 3, ((((j - 1) % 3) * 25) + 5));
    if (!(j % 3))
      row++;
    ch->sendTo(buf);
    ch->sendTo(fmt("%2d %s") % i % objSpecials[i].name);
    j++;
  }
  ch->sendTo(fmt(VT_CURSPOS) % 22 % 1);

  ch->sendTo("Select a new special procedure (0 = no procedure).\n\r--> ");
}

void change_chest_value2(TBeing *ch, TOpenContainer *o, const char *arg, editorEnterTypeT type)
{
  unsigned int loc_update;
  int row;
  unsigned int i;
  char buf[256];

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = CHANGE_OBJ_VALUES;
      change_obj_values(ch, o, "", ENTER_CHECK);
      return;
    }
  }
  loc_update = convertTo<int>(arg);

  switch (ch->specials.edit) {
    case CHANGE_CHEST_VALUE2:
      switch (loc_update) {
	case 1:
	  ch->specials.edit = CHANGE_CHEST_CONT_FLAGS;
	  change_chest_value2(ch, o, "", ENTER_CHECK);
	  return;
	case 2:
	  ch->specials.edit = CHANGE_CHEST_TRAP_TYPE;
	  change_chest_value2(ch, o, "", ENTER_CHECK);
	  return;
	case 3:
	  ch->specials.edit = CHANGE_CHEST_TRAP_DAM;
	  change_chest_value2(ch, o, "", ENTER_CHECK);
	  return;
	  return;
      }
      break;
    case CHANGE_CHEST_CONT_FLAGS:
      loc_update--;

      if (type != ENTER_CHECK) {
	if (loc_update < 0 || loc_update >= MAX_CONTAINER_FLAG)
	  return;
	i = 1 << loc_update;

        if (i == CONT_TRAPPED ||
            i == CONT_EMPTYTRAP ||
            i == CONT_GHOSTTRAP) {
          ch->sendTo("This flag is set automatically and can not be changed.\n\r"); 
          return;
        }
     
	if (o->isContainerFlag(i))
	  o->remContainerFlag(i);
	else
	  o->addContainerFlag(i);
      }
      ch->sendTo(VT_HOMECLR);
      row = 0;
      for (i = 0; i < MAX_CONTAINER_FLAG; i++) {
	sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
	if (i & 1)
	  row++;
	ch->sendTo(buf);

	ch->sendTo(fmt("%2d [%s] %s") % (i+1) % (o->isContainerFlag(1 << i) ? "X" : " ") % chest_bits[i]);
      }
      ch->sendTo(fmt(VT_CURSPOS) % 21 % 1);
      ch->sendTo("Select number to toggle. <ENTER> to return back to main menu.\n\r");
      return;
    case CHANGE_CHEST_TRAP_TYPE:
      if (type != ENTER_CHECK) {
	if (loc_update < 0 || loc_update > MAX_TRAP_TYPES)
	  return;
        if (loc_update == DOOR_TRAP_BOLT ||
            loc_update == DOOR_TRAP_DISK ||
            loc_update == DOOR_TRAP_HAMMER) {
          ch->sendTo("That trap type is not supported for containers.\n\r");
          return;
        }
        if (loc_update == 0) {
          o->remContainerFlag( CONT_TRAPPED);
          o->setContainerTrapType(DOOR_TRAP_NONE);
          o->setContainerTrapDam(0);
          ch->specials.edit = CHANGE_OBJ_VALUES;
          change_obj_values(ch, o, "", ENTER_CHECK);
          return;
        }
        o->addContainerFlag(CONT_TRAPPED);

        o->setContainerTrapType(doorTrapT(loc_update));
        ch->specials.edit = CHANGE_OBJ_VALUES;
        change_obj_values(ch, o, "", ENTER_CHECK);
        return;
      }
      ch->sendTo(VT_HOMECLR);
      row = 0;
      for (i = 0; i < MAX_TRAP_TYPES; i++) {
	sprintf(buf, VT_CURSPOS, row + 4, (((i%2) == 0) ? 5 : 45));
	if ((i%2) == 1)
	  row++;
	ch->sendTo(buf);

	ch->sendTo(fmt("%2d %s") % i % trap_types[i]);
      }
      ch->sendTo(fmt(VT_CURSPOS) % 21 % 1);
      ch->sendTo("Select number to select.\n\r");
      return;
    case CHANGE_CHEST_TRAP_DAM:
      if (type != ENTER_CHECK) {
        if (loc_update < 0 || loc_update > 100) {
          ch->sendTo("Keep damage between 0 - 100 hps.\n\r");
          return;
        }

        o->addContainerFlag( CONT_TRAPPED);

        o->setContainerTrapDam(loc_update);
        ch->specials.edit = CHANGE_OBJ_VALUES;
        change_obj_values(ch, o, "", ENTER_CHECK);
        return;
      }
      ch->sendTo(VT_HOMECLR);
      ch->sendTo("The value set will be how much damage the trap does.\n\r");
      ch->sendTo("This is a fixed value adjusted only for immunites or sancts.\n\r");
      ch->sendTo("It is non-variable.\n\r");
      ch->sendTo(fmt(VT_CURSPOS) % 21 % 1);
      ch->sendTo("Select damage for trap.\n\r");
      return;
    default:
      return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo("1) Container flags.\n\r");
  ch->sendTo("2) Trap type for chest.\n\r");
  ch->sendTo("3) Trap damage for chest.\n\r");
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter your choise to modify.\n\r--> ");
}

void change_trap_value2(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  o->changeTrapValue2(ch, arg, type);
}


void change_weapon_value1(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  o->changeBaseWeaponValue1(ch, arg, type);
}

#if 1
void change_weapon_value2(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  //o->changeBaseWeaponValue2(ch, arg, type);
}

void change_weapon_value3(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  //  o->changeBaseWeaponValue3(ch, arg, type);
}
#endif
void change_trap_value3(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  o->changeTrapValue3(ch, arg, type);
}

void change_magicitem_value1(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  o->changeMagicItemValue1(ch, arg, type);
}

void change_bed_value1(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  o->changeBedValue1(ch, arg, type);
}

void change_egg_value1(TBeing *ch, TEgg *o, const char *arg, editorEnterTypeT type)
{
  int loc_update;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = CHANGE_OBJ_VALUES;
      change_obj_values(ch, o, "", ENTER_CHECK);
      return;
    }
  }
  loc_update = convertTo<int>(arg);

  switch (ch->specials.edit) {
    case CHANGE_EGG_VALUE1:
      switch (loc_update) {
	case 1:
	  ch->sendTo(VT_HOMECLR);
	  ch->sendTo("Set whether touched.\n\r--> ");
	  ch->specials.edit = CHANGE_EGG_TOUCHED;
	  return;
	case 2:
	  ch->sendTo(VT_HOMECLR);
	  ch->sendTo("Enter new food fill value.\n\r--> ");
	  ch->specials.edit = CHANGE_EGG_FILL;
	  return;
      }
      break;
    case CHANGE_EGG_TOUCHED:
      if (type != ENTER_CHECK) {
        if ((loc_update < 0) || (loc_update > 1)) {
          ch->sendTo("Please enter 0 or 1.\n\r");
          return;
        }
        o->setEggTouched(loc_update);
        ch->specials.edit = CHANGE_EGG_VALUE1;
        change_egg_value1(ch, o, "", ENTER_CHECK);
        return;
      }
      break;
    case CHANGE_EGG_FILL:
      if (type != ENTER_CHECK) {
        if ((loc_update < 0) || (loc_update > 24)) {
          ch->sendTo("Please enter a number between 0 and 24.\n\r");
          return;
        }
        o->setFoodFill(loc_update);
        ch->specials.edit = CHANGE_EGG_VALUE1;
        change_egg_value1(ch, o, "", ENTER_CHECK);
        return;
      }
      break;
    default:
      return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo("1) Touched flag (0 = false, 1 = true).\n\r");
  ch->sendTo("2) Amount of food.\n\r");
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter your choice to modify.\n\r--> ");
}

void change_portal_value1(TBeing *ch, TPortal *o, const char *arg, editorEnterTypeT type)
{
  int loc_update;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = CHANGE_OBJ_VALUES;
      change_obj_values(ch, o, "", ENTER_CHECK);
      return;
    }
  }
  loc_update = convertTo<int>(arg);

  switch (ch->specials.edit) {
    case CHANGE_PORTAL_VALUE1:
      switch (loc_update) {
	case 1:
	  ch->sendTo(VT_HOMECLR);
	  ch->sendTo("Enter new room destination number.\n\r--> ");
	  ch->specials.edit = CHANGE_PORTAL_VNUM;
	  return;
	case 2:
	  ch->sendTo(VT_HOMECLR);
	  ch->sendTo("Enter new max number of entries.\n\r--> ");
	  ch->specials.edit = CHANGE_PORTAL_MAX;
	  return;
      }
      break;
    case CHANGE_PORTAL_MAX:
      if (type != ENTER_CHECK) {
	if ((loc_update > 100) || (loc_update < -1)) {
	  ch->sendTo("Please enter a number from -1 to 100.\n\r");
	  return;
	}
	o->setPortalNumCharges(loc_update);
        ch->specials.edit = CHANGE_PORTAL_VALUE1;
        change_portal_value1(ch, o, "", ENTER_CHECK);
	return;
      }
      break;
    case CHANGE_PORTAL_VNUM:
      if (type != ENTER_CHECK) {
	/*
	if ((loc_update >= WORLD_SIZE) || (loc_update < 0)) {
	  ch->sendTo(fmt("Please enter a number from 0-%d.\n\r") % WORLD_SIZE-1);
	  return;
	}
	*/
	o->setTarget(loc_update);
        ch->specials.edit = CHANGE_PORTAL_VALUE1;
        change_portal_value1(ch, o, "", ENTER_CHECK);
	return;
      }
    default:
      return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo("1) Portal Destination (Room portal enters into).\n\r");
  ch->sendTo("2) Max charges (Maximum number of people who can enter portal   before it is destroyed. -1 means permanent).\n\r");
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter your choice to modify.\n\r--> ");
}

void change_component_value4(TBeing *ch, TObj *o, const char *arg, editorEnterTypeT type)
{
  o->changeComponentValue4(ch, arg, type);
}

void change_portal_value3(TBeing *ch, TPortal *o, const char *arg, editorEnterTypeT type)
{
  int loc_update, i, row;
  char buf[256];

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = CHANGE_OBJ_VALUES;
      change_obj_values(ch, o, "", ENTER_CHECK);
      return;
    }
  }
  loc_update = convertTo<int>(arg);

  switch (ch->specials.edit) {
    case CHANGE_PORTAL_VALUE3:
      switch (loc_update) {
	case 1:
	  ch->sendTo(fmt("Current portal trap damage: %d\n\r") % o->getPortalTrapDam());
	  ch->sendTo("Enter new portal trap damage.\n\r--> ");
	  ch->specials.edit = CHANGE_PORTAL_TRAP_DAM;
	  return;
	case 2:
          ch->sendTo(VT_HOMECLR);
	  ch->sendTo(fmt("Current portal trap type: %d\n\r") % o->getPortalTrapType());
          row = 0;
          for (i = 0; i < MAX_TRAP_TYPES; i++) {
            sprintf(buf, VT_CURSPOS, row + 4, (((i%2) == 0) ? 5 : 45));
            if ((i%2) == 1)
              row++;
            ch->sendTo(buf);
            ch->sendTo(fmt("%2d %s") % i % trap_types[i]);
          }
          ch->sendTo(fmt(VT_CURSPOS) % 21 % 1);
	  ch->sendTo("Enter new portal trap type.\n\r--> ");
	  ch->specials.edit = CHANGE_PORTAL_TRAP_TYPE;
	  return;
      }
      break;
    case CHANGE_PORTAL_TRAP_DAM:
      if (type != ENTER_CHECK) {
	if ((loc_update > 100) || (loc_update < 0)) {
	  ch->sendTo("Please enter a number from 0-100.\n\r");
	  return;
	}
	o->setPortalTrapDam(loc_update);
        if (loc_update == 0) {
          o->remPortalFlag(EX_TRAPPED);
          o->setPortalTrapType(DOOR_TRAP_NONE);
        } else {
          o->addPortalFlag(EX_TRAPPED);
        }
        ch->specials.edit = CHANGE_PORTAL_VALUE3;
        change_portal_value3(ch, o, "", ENTER_CHECK);
	return;
      }
      break;
    case CHANGE_PORTAL_TRAP_TYPE:
      if (type != ENTER_CHECK) {
	if (loc_update < 0 || loc_update > MAX_TRAP_TYPES) {
	  ch->sendTo(fmt("Please enter a number from 0-%d.\n\r") % MAX_TRAP_TYPES);
	  return;
        }
	if (loc_update == DOOR_TRAP_BOLT ||
	    loc_update == DOOR_TRAP_DISK ||
	    loc_update == DOOR_TRAP_PEBBLE) {
          ch->sendTo("That trap type is not supported for portals.\n\r");
	  return;
        }
        o->setPortalTrapType(loc_update);
        if (loc_update == DOOR_TRAP_NONE) {
          o->remPortalFlag(EX_TRAPPED);
          o->setPortalTrapDam(0);
        } else {
          o->addPortalFlag(EX_TRAPPED);
        }
        ch->specials.edit = CHANGE_PORTAL_VALUE3;
        change_portal_value3(ch, o, "", ENTER_CHECK);
	return;
      }
      break;
    default:
      return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("1) Portal Trap Damage.   Current: %d\n\r") % o->getPortalTrapDam());
  ch->sendTo(fmt("2) Portal Trap Type.     Current: %s\n\r") % trap_types[o->getPortalTrapType()]);
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter your choice to modify.\n\r--> ");
}

void change_portal_value4(TBeing *ch, TPortal *o, const char *arg, editorEnterTypeT type)
{
  int loc_update, row;
  unsigned int i;
  int j;
  char buf[256];

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = CHANGE_OBJ_VALUES;
      change_obj_values(ch, o, "", ENTER_CHECK);
      return;
    }
  }
  loc_update = convertTo<int>(arg);

  switch (ch->specials.edit) {
    case CHANGE_PORTAL_VALUE4:
      switch (loc_update) {
	case 1:
	  ch->sendTo("Enter new key number.\n\r--> ");
	  ch->specials.edit = CHANGE_PORTAL_KEY;
	  return;
	case 2:
	  ch->specials.edit = CHANGE_PORTAL_DOOR_FLAGS;
	  change_portal_value4(ch, o, "", ENTER_CHECK);
	  return;
      }
      break;
    case CHANGE_PORTAL_KEY:
      if (type != ENTER_CHECK) {
	if ((loc_update >= WORLD_SIZE) || (loc_update < 1)) {
	  ch->sendTo(fmt("Please enter a number from 1-%d.\n\r") % (WORLD_SIZE-1));
	  return;
	}
	o->setPortalKey(loc_update);
        ch->specials.edit = CHANGE_PORTAL_VALUE4;
        change_portal_value4(ch, o, "", ENTER_CHECK);
	return;
      }
    case CHANGE_PORTAL_DOOR_FLAGS:
      loc_update--;

      if (type != ENTER_CHECK) {
	if (loc_update < 0 || loc_update >= MAX_DOOR_CONDITIONS)
	  return;
	i = 1 << loc_update;

	if (o->isPortalFlag(i))
	  o->remPortalFlag(i);
	else
	  o->addPortalFlag(i);
      }
      ch->sendTo(VT_HOMECLR);
      row = 0;
      for (j = 0; j < MAX_DOOR_CONDITIONS; j++) {
	sprintf(buf, VT_CURSPOS, row + 4, ((j & 1) ? 45 : 5));
	if (j & 1)
	  row++;
	ch->sendTo(buf);
	ch->sendTo(fmt("%2d [%s] %s") % (j + 1) % ((o->isPortalFlag(1 << j)) ? "X" : " ") % exit_bits[j]);
      }
      ch->sendTo(fmt(VT_CURSPOS) % 21 % 1);
      ch->sendTo("Select number to toggle. <ENTER> to return back to main menu,\n\r--> ");
      return;
    default:
      return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo("1) Key number (if portal can be closed and locked).\n\r");
  ch->sendTo("2) Portal \"door\" flags (closed, locked etc.).\n\r");
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter your choice to modify.\n\r--> ");
}
void change_arrow_value3(TBeing *ch, TArrow *o, const char *arg, editorEnterTypeT type)
{
  int loc_update;

  if(type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = CHANGE_OBJ_VALUES;
      change_obj_values(ch, o, "", ENTER_CHECK);
      return;
    }
  }

  loc_update = convertTo<int>(arg);

  switch (ch->specials.edit) {
    case CHANGE_ARROW_VALUE3:
      switch (loc_update) {
	case 1:
	  ch->sendTo(VT_HOMECLR);
	  ch->sendTo("Enter Arrow Trap Type.\n\r");
	  ch->sendTo("NOTE: DOOR_TRAP_TYPE does not correlate to the number of the menu choices below.  Eval the weapon after to ensure the proper trap has been set.\n\r");
	  ch->sendTo(" 0) None\n\r");
	  ch->sendTo(" 1) Fire\n\r");
	  ch->sendTo(" 2) Explosive\n\r");
	  ch->sendTo(" 3) Sleep\n\r");
	  ch->sendTo(" 4) Acid\n\r");
	  ch->sendTo(" 5) Spore\n\r");
	  ch->sendTo(" 6) Frost\n\r");
	  ch->sendTo(" 7) Spike\n\r");
	  ch->sendTo(" 8) Pebble\n\r");
	  ch->sendTo(" 9) Power\n\r");
	  ch->sendTo("10) Teleport\n\r");
	  ch->sendTo(" Your Selection [0-10]\n\r--> ");
	  ch->specials.edit = CHANGE_ARROW_TRAPTYPE;
	  return;
	case 2:
	  ch->sendTo(VT_HOMECLR);
	  ch->sendTo("Enter Arrow Trap Level.  [0-65]\n\r--> ");
	  ch->specials.edit = CHANGE_ARROW_TRAPLVL;
	  return;
      }
      break;
    case CHANGE_ARROW_TRAPTYPE:
      if (type!= ENTER_CHECK) {
	if((loc_update > 10) || (loc_update < 0)) {
	  ch->sendTo("Invalid value.  Please enter a value between 0 and 10.\n\r");
	  return;
	}

	switch(loc_update) {
	  case 0:
	    o->setTrapDamType(DOOR_TRAP_NONE);
	    break;
	  case 1:
	    o->setTrapDamType(DOOR_TRAP_FIRE);
	    break;
	  case 2:
	    o->setTrapDamType(DOOR_TRAP_TNT);
	    break;
	  case 3:
	    o->setTrapDamType(DOOR_TRAP_SLEEP);
	    break;
	  case 4:
	    o->setTrapDamType(DOOR_TRAP_ACID);
	    break;
	  case 5:
	    o->setTrapDamType(DOOR_TRAP_DISEASE);
	    break;
	  case 6:
	    o->setTrapDamType(DOOR_TRAP_FROST);
	    break;
	  case 7:
	    o->setTrapDamType(DOOR_TRAP_SPIKE);
	    break;
	  case 8:
	    o->setTrapDamType(DOOR_TRAP_PEBBLE);
	    break;
	  case 9:
	    o->setTrapDamType(DOOR_TRAP_ENERGY);
	    break;
	  case 10:
	    o->setTrapDamType(DOOR_TRAP_TELEPORT);
	    break;
	}
	ch->specials.edit = CHANGE_ARROW_VALUE3;
	change_arrow_value3(ch, o, "", ENTER_CHECK);
	return;
      }
      break;
    case CHANGE_ARROW_TRAPLVL:
      if (type != ENTER_CHECK) {
	if ((loc_update > 65) || (loc_update < 0)) {
	  ch->sendTo("Invalid value.  Please enter a value between 0 and 65.\n\r");
	  return;
	}

	o->setTrapLevel((unsigned char) loc_update);
	ch->specials.edit = CHANGE_ARROW_VALUE3;
	change_arrow_value3(ch, o, "", ENTER_CHECK);
	return;
      }
      break;
    default:
      return;
  }

  ch->sendTo(VT_HOMECLR);
  ch->sendTo("1) Arrow Trap Type\n\r");
  ch->sendTo("2) Arrow Trap Level\n\r");
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter your choice.\n\r--> ");
}
	
void change_arrow_value4(TBeing *ch, TArrow *o, const char *arg, editorEnterTypeT type)
{
  int loc_update;

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = CHANGE_OBJ_VALUES;
      change_obj_values(ch, o, "", ENTER_CHECK);
      return;
    }
  }

  loc_update = convertTo<int>(arg);

  switch (ch->specials.edit) {
    case CHANGE_ARROW_VALUE4:
      switch (loc_update) {
        case 1:
          ch->sendTo(VT_HOMECLR);
          ch->sendTo("Enter Arrow Head Type.  [0-5]\n\r--> ");
          ch->specials.edit = CHANGE_ARROW_HEAD;
          return;
        case 2:
          ch->sendTo(VT_HOMECLR);
          ch->sendTo("Enter Arrow Size.  [0-7]\n\r--> ");
          ch->specials.edit = CHANGE_ARROW_TYPE;
          return;
      }
      break;
    case CHANGE_ARROW_HEAD:
      if (type != ENTER_CHECK) {
        if ((loc_update > 5) || (loc_update < 0)) {
          ch->sendTo("Invalid value.  Please enter a value between 0 and 5.\n\r");
          return;
        }

        o->setArrowHead((unsigned char) loc_update);
        ch->specials.edit = CHANGE_ARROW_VALUE4;
        change_arrow_value4(ch, o, "", ENTER_CHECK);
        return;
      }
      break;
    case CHANGE_ARROW_TYPE:
      if (type != ENTER_CHECK) {
        if ((loc_update > 7) || (loc_update < 0)) {
          ch->sendTo("Invalid value.  Please enter a value between 0 and 7.\n\r");
          return;
        }

        o->setArrowType((unsigned char) loc_update);
        ch->specials.edit = CHANGE_ARROW_VALUE4;
        change_arrow_value4(ch, o, "", ENTER_CHECK);
        return;
      }
      break;
    default:
      return;
  }

  ch->sendTo(VT_HOMECLR);
  ch->sendTo("1) Arrow Head Type\n\r");
  ch->sendTo("2) Arrow Size\n\r");
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter your choice.\n\r--> ");
}

void obj_edit(TBeing *ch, const char *arg)
{
  if (ch->specials.edit == MAIN_MENU) {
    if (!*arg || *arg == '\n') {
      ch->desc->connected = CON_PLYNG;
      act("$n has returned from editing objects.", TRUE, ch, 0, 0, TO_ROOM);
      if (ch->desc->obj) {
	*ch += *(ch->desc->obj);
	ch->desc->obj = NULL;
      }
      // reset the terminal bars
      if (ch->vt100() || ch->ansi())
        ch->doCls(false);
      return;
    }
    switch (convertTo<int>(arg)) {
      case 0:
        ch->specials.edit = MAIN_MENU;
	update_obj_menu(ch, ch->desc->obj);
	return;
      case 1:
	ch->specials.edit = CHANGE_NAME;
	change_obj_name(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 2:
	ch->specials.edit = CHANGE_SHORT_DESC;
	change_obj_short_desc(ch, ch->desc->obj, ENTER_CHECK);
	return;
      case 3:
	ch->specials.edit = CHANGE_OBJ_TYPE;
	change_obj_type(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 4:
	ch->specials.edit = CHANGE_LONG_DESC;
	change_obj_long_desc(ch, ch->desc->obj, ENTER_CHECK);
	return;
      case 5:
	ch->specials.edit = CHANGE_OBJ_WEIGHT;
	change_obj_weight(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 6:
	ch->specials.edit = CHANGE_OBJ_VOLUME;
	change_obj_volume(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 7:
	ch->specials.edit = CHANGE_OBJ_EXTRA_FLAGS;
	change_obj_extra_flags(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 8:
	ch->specials.edit = CHANGE_OBJ_WEAR_FLAGS;
	change_obj_wear_flags(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 10:
        if (ch->hasWizPower(POWER_OEDIT_COST)) {
          ch->specials.edit = CHANGE_OBJ_COST;
	  change_obj_cost(ch, ch->desc->obj, "", ENTER_CHECK);
        } else {
          ch->sendTo("Do not worry about setting cost yourself until level 52.\n\r");
        }
	return;
      case 11:
        ch->specials.edit = CHANGE_OBJ_VALUES;
        change_obj_values(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 12:
	ch->specials.edit = CHANGE_OBJ_DECAY;
	change_obj_decay(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 13:
	ch->specials.edit = CHANGE_OBJ_MAX_STRUCTS;
	change_obj_max_struct_points(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 14:
	ch->specials.edit = CHANGE_OBJ_STRUCT_POINTS;
	change_obj_struct_points(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 15:
	ch->specials.edit = CHANGE_OBJ_EXTRA;
	change_obj_extra(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 16:
	ch->specials.edit = CHANGE_OBJ_MAT_TYPE;
	change_obj_mat_type(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 17:
        if (ch->hasWizPower(POWER_OEDIT_APPLYS)) {
          ch->specials.edit = CHANGE_OBJ_APPLYS;
          change_obj_applys(ch, ch->desc->obj, "", ENTER_CHECK);
        } else {
          ch->sendTo("Do not worry about changing applys yourself until level 53.\n\r");
          ch->specials.edit = MAIN_MENU;
        }
	return;
      case 18:
	ch->specials.edit = CHANGE_OBJ_CAN_BE_SEEN;
	change_obj_can_be_seen(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 19:
	ch->specials.edit = MAIN_MENU;
	delete_obj_extra_desc(ch, ch->desc->obj, "", ENTER_CHECK);
	return;
      case 20:
        ch->specials.edit = CHANGE_OBJ_SPEC;
        change_obj_spec(ch, ch->desc->obj, "", ENTER_CHECK);
        return;
      case 21:
        ch->specials.edit = CHANGE_OBJ_MAX_EXIST;
        change_obj_max_exist(ch, ch->desc->obj, "", ENTER_CHECK);
        return;
      default:
	ch->specials.edit = MAIN_MENU;
	update_obj_menu(ch, ch->desc->obj);
    }
  }
  switch (ch->specials.edit) {
    case CHANGE_NAME:
      change_obj_name(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_SHORT_DESC:
      change_obj_short_desc(ch, ch->desc->obj, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_TYPE:
      change_obj_type(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_LONG_DESC:
      change_obj_long_desc(ch, ch->desc->obj, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_WEIGHT:
      change_obj_weight(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_VOLUME:
      change_obj_volume(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_EXTRA_FLAGS:
      change_obj_extra_flags(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_WEAR_FLAGS:
      change_obj_wear_flags(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_COST:
      change_obj_cost(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_DECAY:
      change_obj_decay(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_VALUES:
    case CHANGE_OBJ_VALUE1:
    case CHANGE_OBJ_VALUE2:
    case CHANGE_OBJ_VALUE3:
    case CHANGE_OBJ_VALUE4:
      change_obj_values(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_EXTRA:
    case CHANGE_OBJ_EXDESC:
      change_obj_extra(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_MAX_EXIST:
      change_obj_max_exist(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_SPEC:
      change_obj_spec(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_MAX_STRUCTS:
      change_obj_max_struct_points(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_STRUCT_POINTS:
      change_obj_struct_points(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_MAT_TYPE:
    case CHANGE_OBJ_MAT_TYPE1:
    case CHANGE_OBJ_MAT_TYPE2:
    case CHANGE_OBJ_MAT_TYPE3:
    case CHANGE_OBJ_MAT_TYPE4:
      change_obj_mat_type(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_APPLYS:
      change_obj_applys(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_OBJ_CAN_BE_SEEN:
      change_obj_can_be_seen(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_CHEST_VALUE2:
    case CHANGE_CHEST_CONT_FLAGS:
    case CHANGE_CHEST_TRAP_TYPE:
    case CHANGE_CHEST_TRAP_DAM:
      change_chest_value2(ch, dynamic_cast<TOpenContainer *>(ch->desc->obj), arg, ENTER_REENTRANT);
      return;
    case CHANGE_TRAP_VALUE2:
      change_trap_value2(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_TRAP_VALUE3:
      change_trap_value3(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_WEAPON_VALUE1:
    case CHANGE_WEAPON_MAX_SHARP:
    case CHANGE_WEAPON_SHARP:
      change_weapon_value1(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_WEAPON_VALUE2:
    case CHANGE_WEAPON_DAMAGE_LEV:
    case CHANGE_WEAPON_TYPE:
      change_weapon_value2(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_WEAPON_VALUE3:
    case CHANGE_WEAPON_DAMAGE_PREC:
    case CHANGE_WEAPON_QUALITY:
      change_weapon_value3(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_MAGICITEM_LEVEL:
    case CHANGE_MAGICITEM_LEARNEDNESS:
    case CHANGE_MAGICITEM_VALUE1:
      change_magicitem_value1(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_EGG_VALUE1:
    case CHANGE_EGG_TOUCHED:
    case CHANGE_EGG_FILL:
      change_egg_value1(ch, dynamic_cast<TEgg *>(ch->desc->obj), arg, ENTER_REENTRANT);
      return;
    case CHANGE_PORTAL_VALUE1:
    case CHANGE_PORTAL_VNUM:
    case CHANGE_PORTAL_MAX:
      change_portal_value1(ch, dynamic_cast<TPortal *>(ch->desc->obj), arg, ENTER_REENTRANT);
      return;
    case CHANGE_PORTAL_VALUE3:
    case CHANGE_PORTAL_TRAP_TYPE:
    case CHANGE_PORTAL_TRAP_DAM:
      change_portal_value3(ch, dynamic_cast<TPortal *>(ch->desc->obj), arg, ENTER_REENTRANT);
      return;
    case CHANGE_PORTAL_VALUE4:
    case CHANGE_PORTAL_KEY:
    case CHANGE_PORTAL_DOOR_FLAGS:
      change_portal_value4(ch, dynamic_cast<TPortal *>(ch->desc->obj), arg, ENTER_REENTRANT);
      return;
    case CHANGE_COMPONENT_VALUE4:
      change_component_value4(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case CHANGE_BED_VALUE1:
    case CHANGE_BED_MAXUSERS:
    case CHANGE_BED_MINPOS:
      change_bed_value1(ch, ch->desc->obj, arg, ENTER_REENTRANT);
      return;
    case MAIN_MENU:
      return;
    case CHANGE_ARROW_VALUE3:
    case CHANGE_ARROW_TRAPLVL:
    case CHANGE_ARROW_TRAPTYPE:
      change_arrow_value3(ch, dynamic_cast<TArrow *>(ch->desc->obj), arg, ENTER_REENTRANT);
      return;
    case CHANGE_ARROW_VALUE4:
    case CHANGE_ARROW_HEAD:
    case CHANGE_ARROW_TYPE:
      change_arrow_value4(ch, dynamic_cast<TArrow *>(ch->desc->obj), arg, ENTER_REENTRANT);
      return;
    default:
      vlogf(LOG_EDIT, fmt("Got to bad spot in obj_edit.  char: %s   case: %d") % ch->getName() %ch->specials.edit);
      return;
  }
}

void TObj::changeObjValue1(TBeing *ch)
{
  int x1, x2, x3, x4;
  getFourValues(&x1, &x2, &x3, &x4);

  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("What does this value do? :\n\r%s\n\r") %
    ItemInfo[itemType()]->val0_info);
  ch->specials.edit = CHANGE_OBJ_VALUE1;

  ch->sendTo(fmt("Value 1 for %s : %d\n\r\n\r") %
       sstring(getName()).uncap() % x1);
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter new value.\n\r--> ");
}

void TObj::changeObjValue2(TBeing *ch)
{
  int x1, x2, x3, x4;
  getFourValues(&x1, &x2, &x3, &x4);

  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("What does this value do? :\n\r %s\n\r") %
        ItemInfo[itemType()]->val1_info);
  ch->specials.edit = CHANGE_OBJ_VALUE2;

  ch->sendTo(fmt("Value 2 for %s : %d\n\r\n\r") %
       sstring(getName()).uncap() % x2);
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter new value.\n\r--> ");
}

void TObj::changeObjValue3(TBeing *ch)
{
  int x1, x2, x3, x4;
  getFourValues(&x1, &x2, &x3, &x4);

  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("What does this value do? :\n\r %s\n\r") %
       ItemInfo[itemType()]->val2_info);
  ch->specials.edit = CHANGE_OBJ_VALUE3;

  ch->sendTo(fmt("Value 3 for %s : %d\n\r\n\r") %
       sstring(getName()).uncap() % x3);
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter new value.\n\r--> ");
}

void TObj::changeObjValue4(TBeing *ch)
{
  int x1, x2, x3, x4;
  getFourValues(&x1, &x2, &x3, &x4);

  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("What does this value do? :\n\r %s\n\r") %
	    ItemInfo[itemType()]->val3_info);
  ch->specials.edit = CHANGE_OBJ_VALUE4;

  ch->sendTo(fmt("Value 4 for %s : %d\n\r\n\r") %
       sstring(getName()).uncap() % x4);
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter new value.\n\r--> ");
}

int TObj::changeItemVal2Check(TBeing *, int)
{
  return FALSE;
}

int TObj::changeItemVal3Check(TBeing *, int)
{
  return FALSE;
}

int TObj::changeItemVal4Check(TBeing *, int)
{
  return FALSE;
}

sstring TObj::displayFourValues()
{
  char tString[256];
  int  x1,
       x2,
       x3,
       x4;

  getFourValues(&x1, &x2, &x3, &x4);
  sprintf(tString, "Current values : %d %d %d %d\n\r",
          x1, x2, x3, x4);

  return tString;
}

bool dirlistSort::operator() (const sstring &xstr, const sstring &ystr) const
{
  // sstrings come in of the form "1234 object name"
  // yank the 1st argument, parse as a number, and put in numerical order

  sstring xnum;
  sstring ynum;
  one_argument(xstr, xnum);
  one_argument(ystr, ynum);
  int xint = convertTo<int>(xnum);
  int yint = convertTo<int>(ynum);

  return xint < yint;
}

void generic_dirlist(const char *buf, const TBeing *ch)
{
  struct dirent *dp;
  DIR *dfd;

  if (!(dfd = opendir(buf))) {
    vlogf(LOG_FILE, fmt("Unable to dirwalk directory %s") %  buf);
    return;
  }
  unsigned int totcnt = 0;
  sstring longstr;

  // readdir does NOT read in any specific order (non alphabetic)
  // so sorting the buf is nice
  // there is probably a way to get readdir to read in alphabetic, but
  // since the filenames are things like "1" "100", we wouldn't want
  // alphabetic order anyway -> (1, 10, 11, 2, 20, 200, 3, ...) BAD!

  // this will be a vector that we can sort later
  vector<sstring>sort_str(0);

  while ((dp = readdir(dfd))) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
      continue;
    sstring str = buf;
    str += "/";
    str += dp->d_name;
    FILE *fp = fopen(str.c_str(), "r");
    if (fp) {
      int num;
      int rc = fscanf(fp, "#%d\n", &num);
      if (rc == 1) {
        // only handle if it's of right form
        char *n;
        n = fread_string(fp);
  
        sstring newstr;
        newstr += dp->d_name;
        newstr += " ";
        newstr += n;
        newstr += (++totcnt%2 == 0 ? "\n\r" : "\n\r");

        sort_str.push_back(newstr);
        delete [] n;
      }
      fclose(fp);
    }
  }

  // put in alphabetical order
  sort(sort_str.begin(), sort_str.end(), dirlistSort());

  unsigned int iter;
  for (iter = 0; iter < sort_str.size(); iter++)
    longstr += sort_str[iter];

  if (longstr.empty())
    longstr += "Nothing found.\n\r";

  closedir(dfd);
  ch->desc->page_string(longstr, SHOWNOW_NO, ALLOWREP_YES);
}

int TObj::addApply(TBeing *ch, applyTypeT apply)
{
  if (apply == APPLY_LIGHT && !canWear(ITEM_TAKE)) {
    ch->sendTo("If you want light on a takeable object, please do so by setting GLOW.\n\r");
    return TRUE;
  }
  return FALSE;
}

void TMagicItem::changeMagicItemValue1(TBeing *ch, const char *arg, editorEnterTypeT type)
{
  int loc_update;
 
  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = CHANGE_OBJ_VALUES;
      change_obj_values(ch, this, "", ENTER_CHECK);
      return;
    }
  }
  loc_update = convertTo<int>(arg);
 
  switch (ch->specials.edit) {
    case CHANGE_MAGICITEM_VALUE1:
      switch (loc_update) {
        case 1:
          ch->sendTo(VT_HOMECLR);
          ch->sendTo(fmt("Current level: %d\n\r") % getMagicLevel());
          ch->sendTo("Enter new max level.\n\r--> ");
          ch->specials.edit = CHANGE_MAGICITEM_LEVEL;
          return;
        case 2:
          ch->sendTo(VT_HOMECLR);
          ch->sendTo(fmt("Current learnedness: %d\n\r") % getMagicLearnedness());
          ch->sendTo("Enter new learnedness.\n\r--> ");
          ch->specials.edit = CHANGE_MAGICITEM_LEARNEDNESS;
          return;
      }
      break;
    case CHANGE_MAGICITEM_LEVEL:
      if (type != ENTER_CHECK) {
        if ((loc_update > 50) || (loc_update < 0)) {
          if (loc_update > 70 || !ch->hasWizPower(POWER_OEDIT_IMP_POWER)) {
            ch->sendTo("Please enter a number from 0-50.\n\r");
            return;
          }
        }
        setMagicLevel(loc_update);
        ch->specials.edit = CHANGE_MAGICITEM_VALUE1;
        change_magicitem_value1(ch, this, "", ENTER_CHECK);
        return;
      }
    case CHANGE_MAGICITEM_LEARNEDNESS:
      if (type != ENTER_CHECK) {
        if ((loc_update > 100) || (loc_update < 0)) {
          ch->sendTo("Please enter a number from 0-100.\n\r");
          return;
        }
        setMagicLearnedness(loc_update);
        ch->specials.edit = CHANGE_MAGICITEM_VALUE1;
        change_magicitem_value1(ch, this, "", ENTER_CHECK);
        return;
      }
    default:
      return;
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo("1) Level of spell's item casts\n\r");
  ch->sendTo("2) Learnedness of item's spells\n\r");
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter your choice to modify.\n\r--> ");
}

void TTrap::changeTrapValue2(TBeing *ch, const char *arg, editorEnterTypeT type)
{ 
  int i, row, loc_update;
  char buf[256];

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, this);
      return;
    }
  }
  loc_update = convertTo<int>(arg);
  loc_update--;

  if (type != ENTER_CHECK) {
    if (loc_update < 0 || loc_update > MAX_TRAP_EFF)
      return;
    i = 1 << loc_update;

    if (IS_SET(getTrapEffectType(), i))
      setTrapEffectType(getTrapEffectType() & ~i);
    else
      setTrapEffectType(getTrapEffectType() | i);
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo("Trap Effects:");

  row = 0;
  for (i = 0; i < MAX_TRAP_EFF; i++) {
    sprintf(buf, VT_CURSPOS, row + 4, ((i%2) ? 45 : 5));
    if (i%2)
      row++;
    ch->sendTo(buf);

    ch->sendTo(fmt("%2d [%s] %s") % (i+1) % 
              ((getTrapEffectType() & (1 << i)) ? "X" : " ") % 
              trap_effects[i]);
  }
  ch->sendTo(fmt(VT_CURSPOS) % 21 % 1);
  ch->sendTo("Select to number to toggle, <ENTER> to return to the main menu.\n\r--> ");
}


void TTrap::changeTrapValue3(TBeing *ch, const char *arg, editorEnterTypeT type)
{ 
  int i, row;
  doorTrapT loc_update;
  char buf[256];
 
  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, this);
      return;
    }
  }
  loc_update = doorTrapT(convertTo<int>(arg) - 1);

  if (type != ENTER_CHECK) {
    if (loc_update <= 0 || loc_update > MAX_TRAP_TYPES) {
      return;
    } else if (loc_update == DOOR_TRAP_SPIKE ||
               loc_update == DOOR_TRAP_BLADE ||
               loc_update == DOOR_TRAP_HAMMER) {
      ch->sendTo("That trap type is not supported for item:trap.\n\r");
      return;
    } else {
      setTrapDamType(loc_update);
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, this);
      return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Trap Damage Type: %s") % trap_types[getTrapDamType()]);
 
  row = 0;
  for (i = 0; i < MAX_TRAP_TYPES; i++) {
    sprintf(buf, VT_CURSPOS, row + 3, (((i % 3) * 25) + 5));
    if (!((i + 1) % 3))
      row++;
    ch->sendTo(buf);
    sprintf(buf, "%2d %s", i + 1, trap_types[i].c_str());
    ch->sendTo(buf);
  }
  ch->sendTo(fmt(VT_CURSPOS) % 21 % 1);
  ch->sendTo("Select the number to set to, <ENTER> to return to main menu.\n\r--> ");
}

void TObj::describeTreasure(const char *arg,int num, int price)
{
  char type[80];
  char buf[160];

  if (num == 0) {
    vlogf(LOG_EDIT, fmt("Bogus obj in describeTreasure, %s, %s") %  arg % getName());
    return;  
  } else if (num <= 2) 
    sprintf(type,"bit");
  else if (num <= 4)
    sprintf(type,"nugget");
  else if (num <= 6)
    sprintf(type,"ingot");
  else if (num <= 8)
    sprintf(type,"sovereign");
  else if (num <= 10)
    sprintf(type,"bar");
  else if (num <= 15)
    sprintf(type,"bullion");
  else 
    sprintf(type,"pile");

  swapToStrung();

  //  Remake the obj name.  
  sprintf(buf, "%s %s commodity", arg, type);
  delete [] name;
  name = mud_str_dup(buf);

  //  Remake the short description.  
  sprintf(buf,"%s %s of %s", 
    sstring(type).startsVowel() ? "an" : "a",
     type, arg);
  if (!strcmp(type,"bullion"))
    sprintf(buf,"some %s %s",
       arg, type);
  delete [] shortDescr;
  shortDescr = mud_str_dup(buf);

  //  Remake the long description.  
  sprintf(type,shortDescr);
  sprintf(buf,"%s has been left here.  What luck!", 
              sstring(type).cap().c_str());
  delete [] getDescr();
  setDescr(mud_str_dup(buf));
  
  //  set value and rent 
  obj_flags.cost = num * price;

  setWeight((float) (num + 0.5)/10.0);
}


int TThing::editAverageMe(TBeing *tBeing, const char *)
{
  tBeing->sendTo("Can not average this.  Sorry.\n\r");
  return FALSE;
}

int TBeing::editAverageMe(TBeing *tBeing, const char *tString)
{
  sstring tStString(""),
         tStBuffer(""),
         tStArg(tString);

  if (!tString || !*tString) {
    tBeing->sendTo("Syntax: med average <level[1.0-60.0]> <class>\n\r");
    return FALSE;
  }

  tStString=tStArg.word(0);
  tStBuffer=tStArg.word(1);


  classIndT tClass;
  int tStr, tBra, tCon, tDex, tAgi, tInt, tWis, tFoc, tPer, tCha, tKar, tSpe;
  double tLevel = convertTo<float>(tStString);

  if (tLevel <= 0 || tLevel > 60) {
    tBeing->sendTo("Level must be between 1 and 60.\n\r");
    return FALSE;
  }

  TMonster *tMonster;

  if (!(tMonster = dynamic_cast<TMonster *>(this))) {
    tBeing->sendTo("Target is not a monster.  Cannot do this, sorry.\n\r");
    return FALSE;
  }

  if (tStBuffer.empty()) {
    for (tClass = MIN_CLASS_IND; tClass < MAX_CLASSES; tClass++)
      if (hasClass((1 << tClass))) {
        tBeing->sendTo(fmt("Setting class to current: %s\n\r") % classInfo[tClass].name.cap());
        break;
      }

    if (tClass == MAX_CLASSES || !hasClass((1 << tClass))) {
      tBeing->sendTo("Something went wrong, tell a coder!\n\r");
      return FALSE;
    }

    tStr = getStat(STAT_CHOSEN, STAT_STR);
    tBra = getStat(STAT_CHOSEN, STAT_BRA);
    tCon = getStat(STAT_CHOSEN, STAT_CON);
    tDex = getStat(STAT_CHOSEN, STAT_DEX);
    tAgi = getStat(STAT_CHOSEN, STAT_AGI);
    tInt = getStat(STAT_CHOSEN, STAT_INT);
    tWis = getStat(STAT_CHOSEN, STAT_WIS);
    tFoc = getStat(STAT_CHOSEN, STAT_FOC);
    tPer = getStat(STAT_CHOSEN, STAT_PER);
    tCha = getStat(STAT_CHOSEN, STAT_CHA);
    tKar = getStat(STAT_CHOSEN, STAT_KAR);
    tSpe = getStat(STAT_CHOSEN, STAT_SPE);
  } else {
    tClass = getClassIndNum(tStBuffer.c_str(), EXACT_YES);

    switch (tClass) {
      case MAGE_LEVEL_IND:
        tStr = -10;
        tBra = -20;
        tCon = -10;
        tDex = -10;
        tAgi = -20;
        tInt = +20;
        tWis = +20;
        tFoc = +20;
        tPer = - 5;
        tCha = - 5;
        tKar = +20;
        tSpe = -10;
        break;
      case CLERIC_LEVEL_IND:
        tStr =   0;
        tBra = -10;
        tCon = -10;
        tDex = +10;
        tAgi =   0;
        tInt = +10;
        tWis =   0;
        tFoc = +10;
        tPer = -20;
        tCha =   0;
        tKar = +20;
        tSpe =   0;
        break;
      case WARRIOR_LEVEL_IND:
        tStr = +20;
        tBra = +10;
        tCon = +20;
        tDex = + 5;
        tAgi = + 5;
        tInt = -20;
        tWis = -20;
        tFoc = -10;
        tPer = -10;
        tCha = -10;
        tKar = +10;
        tSpe = +10;
        break;
      case THIEF_LEVEL_IND:
        tStr = -10;
        tBra = +10;
        tCon = -10;
        tDex = +10;
        tAgi = +20;
        tInt = -22;
        tWis = -23;
        tFoc = +10;
        tPer = +20;
        tCha = +10;
        tKar =   0;
        tSpe = +25;
        break;
      case SHAMAN_LEVEL_IND:
        tStr = 0;
        tBra = 0;
        tCon = 0;
        tDex = 0;
        tAgi = 0;
        tInt = 0;
        tWis = 0;
        tFoc = 0;
        tPer = 0;
        tCha = 0;
        tKar = 0;
        tSpe = 0;
        break;
      case DEIKHAN_LEVEL_IND:
        tStr = +10;
        tBra = -10;
        tCon = -20;
        tDex = -10;
        tAgi =   0;
        tInt = +10;
        tWis = +20;
        tFoc = +20;
        tPer = -10;
        tCha = -10;
        tKar =   0;
        tSpe =   0;
        break;
      case MONK_LEVEL_IND:
        tStr = +10;
        tBra =   0;
        tCon = -20;
        tDex = +10;
        tAgi = +20;
        tInt = -20;
        tWis = -20;
        tFoc = +20;
        tPer = -20;
        tCha =   0;
        tKar =   0;
        tSpe = +20;
        break;
      case RANGER_LEVEL_IND:
        tStr = +20;
        tBra = -20;
        tCon = -20;
        tDex = +10;
        tAgi = +10;
        tInt = -10;
        tWis = -10;
        tFoc = +20;
        tPer = +20;
        tCha = -20;
        tKar = +10;
        tSpe = -10;
        break;
      default:
        tStr = 0;
        tBra = 0;
        tCon = 0;
        tDex = 0;
        tAgi = 0;
        tInt = 0;
        tWis = 0;
        tFoc = 0;
        tPer = 0;
        tCha = 0;
        tKar = 0;
        tSpe = 0;
        break;
    }
  }

  setStat(STAT_CHOSEN, STAT_STR, tStr);
  setStat(STAT_CHOSEN, STAT_BRA, tBra);
  setStat(STAT_CHOSEN, STAT_CON, tCon);
  setStat(STAT_CHOSEN, STAT_DEX, tDex);
  setStat(STAT_CHOSEN, STAT_AGI, tAgi);
  setStat(STAT_CHOSEN, STAT_INT, tInt);
  setStat(STAT_CHOSEN, STAT_WIS, tWis);
  setStat(STAT_CHOSEN, STAT_FOC, tFoc);
  setStat(STAT_CHOSEN, STAT_PER, tPer);
  setStat(STAT_CHOSEN, STAT_CHA, tCha);
  setStat(STAT_CHOSEN, STAT_KAR, tKar);
  setStat(STAT_CHOSEN, STAT_SPE, tSpe);

  setClass((1 << tClass));

  for (classIndT tNewClass = MIN_CLASS_IND; tNewClass < MAX_CLASSES; tNewClass++)
    setLevel(tNewClass, 0);

  setLevel(tClass, (int) tLevel);

  tMonster->setHPLevel(tLevel);
  tMonster->setHPFromHPLevel();
  tMonster->setDamLevel(tLevel);
  tMonster->setACLevel(tLevel);
  tMonster->setACFromACLevel();
  //tMonster->setHitroll(10.0 * tLevel);
  tMonster->setHitroll(0); // hitroll is a bonus ABOVE your level

  return FALSE;
}

int TBaseWeapon::editAverageMe(TBeing *tBeing, const char *tString)
{
  if (!tBeing->hasWizPower(POWER_OEDIT_WEAPONS)) {
    tBeing->sendTo("You don't have the power to modify weapons, thus you can not use this.\n\r");
    return FALSE;
  }

  if (!tString || !*tString) {
    tBeing->sendTo("Syntax: oed average <level[1.0-60.0]>\n\r");
    return FALSE;
  }

  double tLevel = convertTo<float>(tString);

  if (tLevel <= 0.0 || tLevel > 60.0) {
    tBeing->sendTo("Level must be between 1 and 60.\n\r");
    return FALSE;
  }

  double tNewStr = ((tLevel * 3.0) / 2.0) + 10;
  double tNewDam = 4.0 * tLevel;
  double tNewShr = ((tLevel * 3.0) / 2.0) + 10;
  double tNewAvg = 10 - ((tLevel / 60) * 10);

  tBeing->sendTo(COLOR_OBJECTS, fmt("Setting %s to:  [Level: %6.2f]\n\r") % getName() % tLevel);
  tBeing->sendTo(fmt("Strength : %6.2f\n\r") % tNewStr);
  tBeing->sendTo(fmt("Damage   : %6.2f\n\r") % tNewDam);
  tBeing->sendTo(fmt("Shrapness: %6.2f\n\r") % tNewShr);
  tBeing->sendTo(fmt("Precison : %6.2f\n\r") % tNewAvg);

  setMaxSharp((int) tNewShr);
  setCurSharp((int) tNewShr);
  setWeapDamLvl((int) tNewDam);
  setWeapDamDev((int) tNewAvg);
  setStructPoints((int) tNewStr);
  setMaxStructPoints((int) tNewStr);
  obj_flags.cost = suggestedPrice();

  return FALSE;
}

int TBaseClothing::editAverageMe(TBeing *tBeing, const char *tString)
{
  if (!tBeing->hasWizPower(POWER_OEDIT_APPLYS)) {
    tBeing->sendTo("You can not set applies, AC is based off this.  Sorry.\n\r");
    return FALSE;
  }

  if (!tString || !*tString) {
    tBeing->sendTo("Syntax: oed average <level[1.0-60.0]>\n\r");
    return FALSE;
  }

  double tLevel = convertTo<float>(tString);

  if (tLevel <= 0.0 || tLevel > 60.0) {
    tBeing->sendTo("Level must be between 1 and 60.\n\r");
    return FALSE;
  }

  setDefArmorLevel(tLevel);
  obj_flags.cost = suggestedPrice();

  return FALSE;
}





void TGun::changeBaseWeaponValue1(TBeing *ch, const char *arg, editorEnterTypeT type)
{
  int new_rof;
  TGun *o=this;
  
  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
    new_rof = convertTo<int>(arg);

    if (new_rof < 1 || new_rof > 10) {
      ch->sendTo("Please enter a number from 0.1 to 500000.0\n\r");
      return;
    } else {
      o->setROF(new_rof);
      ch->specials.edit = MAIN_MENU;
      update_obj_menu(ch, o);
      return;
    }
  }
  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("Current rate of fire: %i") % o->getROF());
  ch->sendTo(fmt(VT_CURSPOS) % 4 % 1);
  ch->sendTo("Select a new rate of fire.\n\r--> ");


}
