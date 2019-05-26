#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdexcept>

#include "extern.h"
#include "room.h"
#include "being.h"
#include "client.h"
#include "low.h"
#include "monster.h"
#include "handler.h"
#include "configuration.h"
#include "charfile.h"
#include "rent.h"
#include "account.h"
#include "statistics.h"
#include "mail.h"
#include "shop.h"
#include "person.h"
#include "database.h"
#include "spec_mobs.h"
#include "obj_player_corpse.h"
#include "obj_bag.h"
#include "obj_symbol.h"
#include "obj_money.h"
#include "obj_component.h"
#include "obj_note.h"
#include "obj_magic_item.h"
#include "obj_wand.h"
#include "obj_general_weapon.h"
#include "obj_open_container.h"
#include "corporation.h"
#include "shopowned.h"
#include "materials.h"
#include "combat.h"
#include "timing.h"

static const char ROOM_SAVE_PATH[] = "roomdata/saved";
static const int NORMAL_SLOT   = -1;
static const int CONTENTS_END  = -2;

struct SInnkeeperHate {
  int    tVNum; // Mobile VNum of the innkeeper in question.
  race_t tRace;  // Race in question
  bool   isHate; // Do I hate this race?  Or like them...
  sstring tStMessage; // Message to display for 'hated' races.
} SIKHates[] = {
  // An innkeeper can be specified more than once if they have multiple
  // Hates/Likes.  Rules:
  //   If a shopkeeper has a 'like' them they auto-hate every other race.
  //   If a shopkeeper has a 'hate' then they auto-like every other race.

  // This one is a marker one, add below it but before '_NORACE'
  {0, RACE_HUMAN, true, "Get out, we don't serve your kind here scum!"},

  {2124, RACE_DWARF, false, "We serve those of dwarven blood only thank you."},
  {10615, RACE_ELVEN, false, "We serve those of elven blood only thank you."},
  {14323, RACE_HOBBIT, false, "I'm afraid our Inn is setup for those of Gnome or Hobbit stature."},
  {14323, RACE_GNOME, false, "I'm afraid our Inn is setup for those of Gnome or Hobbit stature."},
  {24442, RACE_GNOME, false, "I'm afraid our Inn is designed for those of Gnome or Hobbit stature."},
  {24442, RACE_HOBBIT, false, "I'm afraid our Inn is designed for those of Gnome or Hobbit stature."},
  {44845, RACE_BIRDMAN, false, "How would a creature like you nest here?"},

  {0, RACE_NORACE, false, "Leave!"} // Add all new entries BEFORE this line.
};

void handleCorrupted(const char *name, char *account)
{
  char buf[200];

  sprintf(buf, "mv player/%c/%s player/corrupt/.", 
         LOWER(name[0]), sstring(name).lower().c_str());
  vsystem(buf);

  // strings
  sprintf(buf, "mv player/%c/%s.strings player/corrupt/.",
          LOWER(name[0]), sstring(name).lower().c_str());
  vsystem(buf);

  // toggles
  sprintf(buf, "mv player/%c/%s.toggle player/corrupt/.", 
         LOWER(name[0]), sstring(name).lower().c_str());
  vsystem(buf);

  // career
  sprintf(buf, "mv player/%c/%s.career player/corrupt/.", 
         LOWER(name[0]), sstring(name).lower().c_str());
  vsystem(buf);

  // rent
  sprintf(buf, "mv rent/%c/%s rent/corrupt/.", 
         LOWER(name[0]), sstring(name).lower().c_str());
  vsystem(buf);

  // followers
  sprintf(buf, "mv rent/%c/%s.fol rent/corrupt/.", 
         LOWER(name[0]), sstring(name).lower().c_str());
  vsystem(buf);
  sprintf(buf, "mv rent/%c/%s.fr rent/corrupt/.", 
         LOWER(name[0]), sstring(name).lower().c_str());
  vsystem(buf);

  // corpses
  sprintf(buf, "mv corpses/%s corpses/corrupt/.",
         sstring(name).lower().c_str());
  vsystem(buf);


  // nuke account
  sprintf(buf, "rm account/%c/%s/%s",
         LOWER(account[0]), sstring(account).lower().c_str(), sstring(name).lower().c_str());
  vsystem(buf);
}

void wipePlayerFile(const char *name)
{
  char buf[200];
  if (!name || !*name) {
    vlogf(LOG_BUG, "error in wipePlayerFile - no name (0)");
    return;
  }
  sprintf(buf, "player/%c/%s", LOWER(name[0]), sstring(name).lower().c_str());
  if (unlink(buf) != 0) {
    vlogf(LOG_FILE, format("error in unlink (0) (%s) %d") %  buf % errno);
  }

  // nuke sstrings, ignore errors
  sprintf(buf, "player/%c/%s.strings", LOWER(name[0]), sstring(name).lower().c_str());
  unlink(buf);

  // nuke toggles, ignore errors
  sprintf(buf, "player/%c/%s.toggle", LOWER(name[0]), sstring(name).lower().c_str());
  unlink(buf);

  // nuke corpse, ignore errors
  wipeCorpseFile(sstring(name).lower().c_str());
//  sprintf(buf, "corpses/%s", name.lower());
//  unlink(buf);

  // nuke career stats, ignore errors
  sprintf(buf, "player/%c/%s.career", LOWER(name[0]), sstring(name).lower().c_str());
  unlink(buf);
}

void wipeCorpseFile(const char *name)
{
  char buf[200];
  TPCorpse *corpse = NULL, *tmp = NULL;

  if (!name || !*name) {
    vlogf(LOG_BUG, "error in wipeCorpseFile - no name (0)");
    return;
  }
  for (corpse = pc_corpse_list; corpse;) {
    if (corpse->getOwner().empty()) {
      corpse = corpse->getGlobalNext();
      continue;
    }
    if (corpse->getOwner().compare(name)) {
      corpse = corpse->getGlobalNext();
      continue;
    }
    tmp = corpse;
    corpse = corpse->getGlobalNext();
    tmp->removeCorpseFromList(FALSE);
  }
  sprintf(buf, "corpses/%s", sstring(name).lower().c_str());
  unlink(buf);
}

void wipeRentFile(const char *name)
{
  char buf[200];
  if (!name || !*name) {
    vlogf(LOG_BUG, "error in wipePlayerFile - no name (0)");
    return;
  }

  sprintf(buf, "rent/%c/%s", LOWER(name[0]), sstring(name).lower().c_str());
  unlink(buf);
}

void wipeFollowersFile(const char *name)
{
  char buf[200];

  if (!name || !*name) {
    vlogf(LOG_BUG, "error in wipeFollowerFile - no name (0)");
    return;
  }
  sprintf(buf, "rent/%c/%s.fol", LOWER(name[0]), sstring(name).lower().c_str());
  unlink(buf);
  sprintf(buf, "rent/%c/%s.fr", LOWER(name[0]), sstring(name).lower().c_str());
  unlink(buf);
}

void TBeing::removePlayerFile()
{
  TPerson *tmp;

  if (desc && desc->original)
    tmp = desc->original;
  else
    tmp = dynamic_cast<TPerson *>(this);

  wipePlayerFile(tmp->name.c_str());
}

void TBeing::removeRent()
{
  // we preseume isPc() true when we call this function

  TPerson *tmp;

  if (desc && desc->original)
    tmp = desc->original;
  else
    tmp = dynamic_cast<TPerson *>(this);

  if (!tmp) {
    // being::isPc() checks for a bit set in specials.act, but could be
    // that bit is set (poly) but has no desc.  Not sure how this happens
    // so add this to track problem down
    vlogf(LOG_BUG, format("removeRent() called for a bad isPc() (%s)") %  getName()); 
    return;
  }

  wipeRentFile(tmp->name.c_str());
}

static char *raw_read_sstring(FILE * fp)
{
  char buf[MAX_STRING_LENGTH] = "\0";
  char *s = NULL;
  unsigned int i;

  for (i = 0; (fread(&(buf[i]), 1, 1, fp) == 1) && buf[i]; i++) {
    if (i >= MAX_STRING_LENGTH - 1)
      return NULL;
  }
  if (!strlen(buf))
    return NULL;

  if (!(s = new char[strlen(buf) + 1]))
    return NULL;

  strcpy(s, buf);

  return s;
}

static bool raw_read_rentObject(FILE *fp, rentObject *item, char **name, char **sd, char **d, char **ad, unsigned char /* version */)
{
  if (fread(item, sizeof(rentObject), 1, fp) != 1)
    return FALSE;

  if (item->extra_flags & ITEM_STRUNG) {
    if (!(*name = raw_read_sstring(fp)))
      return FALSE;

    if (!(*sd = raw_read_sstring(fp)))
      return FALSE;

    if (!(*d = raw_read_sstring(fp)))
      return FALSE;

    *ad = raw_read_sstring(fp);
  }
  return TRUE;
}

void ItemSave::writeFooter()
{
  signed char i;
  i = CONTENTS_END;
  fwrite(&i, sizeof(i), 1, fp);
}


bool ItemLoad::readHeader()
{
  int ret=fread(&st, sizeof(rentHeader), 1, fp);
  version=st.version;
  return (ret==1);
}

bool ItemSave::writeHeader()
{
  return (fwrite(&st, sizeof(rentHeader), 1, fp)==1);
}

bool ItemSave::writeVersion()
{
  // prepare the rent file header and write it out
  return (fwrite(&st.version, sizeof(st.version), 1, fp)==1);
}


ItemLoad::ItemLoad()
{
  fp=NULL;
}

ItemLoad::~ItemLoad()
{
  if(fp)
    fclose(fp);
  fp=NULL;
}

ItemLoadDB::ItemLoadDB(sstring ot, int o) :
  owner_type(ot),
  owner(o)
{
}

ItemLoadDB::~ItemLoadDB()
{
}


bool ItemLoad::readVersion()
{
  if (fread(&version, sizeof(version), 1, fp) != 1)
    return false;
  return true;
}

ItemSave::ItemSave()
{
  st.version = CURRENT_RENT_VERSION;
  fp=NULL;
}

ItemSave::~ItemSave()
{
  if(fp)
    fclose(fp);
  fp=NULL;
}


ItemSaveDB::ItemSaveDB(sstring ot, int o) :
  owner_type(ot),
  owner(o)
{
}

ItemSaveDB::~ItemSaveDB()
{
}


bool ItemSave::openFile(const sstring &filepath)
{
  if (!(fp = fopen(filepath.c_str(), "w+b"))) {
    return false;
  }
  return true;
}



bool ItemLoad::openFile(const sstring &filepath)
{
  if (!(fp = fopen(filepath.c_str(), "r+b"))) {
    return false;
  }
  return true;
}

bool ItemLoad::fileExists(const sstring &filepath)
{
  int ret;
  struct stat buf;

  ret=stat(filepath.c_str(), &buf);

  if(ret==-1 and errno==ENOENT)
    return false;
  else
    return true;
}


bool ItemSave::raw_write_item(TObj *o)
{
  rentObject item;
  int j;

  item.item_number = obj_index[o->getItemIndex()].virt;

  int a0, a1, a2,a3;
  o->getFourValues(&a0, &a1, &a2, &a3);
  item.value[0] = a0;
  item.value[1] = a1;
  item.value[2] = a2;
  item.value[3] = a3;

  item.extra_flags = o->getObjStat();
  item.weight = (float) o->getWeight();

  // don't use getVolume, some objs make things "smaller" (fuel)
  item.volume = o->obj_flags.volume;

  item.bitvector = o->obj_flags.bitvector;
  item.struct_points = o->obj_flags.struct_points;
  item.max_struct_points = o->obj_flags.max_struct_points;
  item.decay_time = o->obj_flags.decay_time;
  item.material_points = o->getMaterial();
  item.cost = o->obj_flags.cost;
  item.depreciation = o->getDepreciation();
  
  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    item.affected[j].type = mapSpellnumToFile(o->affected[j].type);
    item.affected[j].level = o->affected[j].level;
    item.affected[j].duration = o->affected[j].duration;
    item.affected[j].renew = o->affected[j].renew;
    item.affected[j].location = mapApplyToFile(o->affected[j].location);

    if (applyTypeShouldBeSpellnum(o->affected[j].location))
      item.affected[j].modifier = mapSpellnumToFile(spellNumT(o->affected[j].modifier));
    else
      item.affected[j].modifier = o->affected[j].modifier;

    item.affected[j].modifier2 = o->affected[j].modifier2;
    item.affected[j].bitvector = o->affected[j].bitvector;
  }

  if (fwrite(&item, sizeof(rentObject), 1, fp) != 1) {
    vlogf(LOG_BUG, "Error writing object to rent.");
    return FALSE;
  }
  if (IS_SET(item.extra_flags, ITEM_STRUNG)) {
    if (!o->name.empty()) {
      if (fwrite(o->name.c_str(), o->name.length() + 1, 1, fp) != 1) {
        vlogf(LOG_BUG, "Error writing object name to rent.");
        return FALSE;
      }
    } else
      vlogf(LOG_BUG, format("Object %d has no name!") %  o->objVnum());

    if (fwrite(o->shortDescr.c_str(), o->shortDescr.length() + 1, 1, fp) != 1) {
      vlogf(LOG_BUG, "Error writing object short description to rent.");
      return FALSE;
    }

    if (!o->getDescr().empty()) {
      if (fwrite(o->getDescr().c_str(), o->getDescr().length() + 1, 1, fp) != 1) {
        vlogf(LOG_BUG, "Error writing object description to rent.");
        return FALSE;
      }
    } else {
      vlogf(LOG_BUG, format("object %d has no descr") %  o->objVnum());
    }
    if (!o->action_description.empty()) {
      if (fwrite(o->action_description.c_str(), o->action_description.length() + 1, 1, fp) != 1) {
        vlogf(LOG_BUG, "Error writing object's action description to rent.");
        return FALSE;
      }
    } else {
      if (fwrite("", strlen("") + 1, 1, fp) != 1) {
        vlogf(LOG_BUG, "Error writing NULL object's action description to rent.");
        return FALSE;
      }
    }
  }
  return TRUE;
}

int ItemSaveDB::raw_write_item(TObj *o, int slot, int container, int rent_id)
{
  TDatabase db(DB_SNEEZY);
  int a0, a1, a2,a3;
  o->getFourValues(&a0, &a1, &a2, &a3);

  if(rent_id==-1){
    db.query("insert into rent (owner_type, owner, vnum, slot, container, val0, val1, val2, val3, extra_flags, weight, bitvector, decay, cur_struct, max_struct, material, volume, price, depreciation) values ('%s', %i, %i, %i, %i, %i, %i, %i, %i, %i, %f, %i, %i, %i, %i, %i, %i, %i, %i)",
	     owner_type.c_str(), owner, 
	     obj_index[o->getItemIndex()].virt, slot,
	     container,
	     a0, a1, a2, a3, o->getObjStat(), (float) o->getWeight(),
	     o->obj_flags.bitvector, o->obj_flags.decay_time,
	     o->obj_flags.struct_points, o->obj_flags.max_struct_points,
	     o->getMaterial(), o->obj_flags.volume, o->obj_flags.cost,
	     o->getDepreciation());
    
    db.query("select last_insert_id() as rent_id");
    db.fetchRow();

    rent_id=convertTo<int>(db["rent_id"]);
    if (0 == rent_id)
      vlogf(LOG_BUG, "Error in rent_id, value is 0 - this code should be switched to use db.lastInsertId()");
  } else {
    db.query("insert into rent (rent_id, owner_type, owner, vnum, slot, container, val0, val1, val2, val3, extra_flags, weight, bitvector, decay, cur_struct, max_struct, material, volume, price, depreciation) values (%i, '%s', %i, %i, %i, %i, %i, %i, %i, %i, %i, %f, %i, %i, %i, %i, %i, %i, %i, %i)",
	     rent_id, owner_type.c_str(), owner, 
	     obj_index[o->getItemIndex()].virt, slot,
	     container,
	     a0, a1, a2, a3, o->getObjStat(), (float) o->getWeight(),
	     o->obj_flags.bitvector, o->obj_flags.decay_time,
	     o->obj_flags.struct_points, o->obj_flags.max_struct_points,
	     o->getMaterial(), o->obj_flags.volume, o->obj_flags.cost,
	     o->getDepreciation());
  }

  int modifier=0;

  for (int j = 0; j < MAX_OBJ_AFFECT; j++) {
    if (applyTypeShouldBeSpellnum(o->affected[j].location))
      modifier = mapSpellnumToFile(spellNumT(o->affected[j].modifier));
    else
      modifier = o->affected[j].modifier;
    
    db.query("insert into rent_obj_aff (type, level, duration, renew, modifier, location, modifier2, bitvector, rent_id) values (%i, %i, %i, %i, %i, %i, %i, %i, %i)", 
	     mapSpellnumToFile(o->affected[j].type), o->affected[j].level,
	     o->affected[j].duration, o->affected[j].renew, modifier,
	     mapApplyToFile(o->affected[j].location),
	     o->affected[j].modifier2, o->affected[j].bitvector, rent_id);
  }

  if (IS_SET(o->getObjStat(), ITEM_STRUNG)) {
    db.query("insert into rent_strung (rent_id, name, short_desc, long_desc, action_desc) values (%i, '%s', '%s', '%s', '%s')",
	     rent_id, 
	     o->name.c_str(),
	     o->shortDescr.c_str(),
	     o->getDescr().c_str(),
	     o->action_description.c_str());
  }
  return rent_id;
}


TObj *ItemLoad::raw_read_item()
{
  rentObject item;
  int j, tmpcost;
  TObj *o;
  char *name = NULL, *shortDescr = NULL, *description = NULL,
  *action_description = NULL;

  if (!raw_read_rentObject(fp, &item, &name, &shortDescr, &description, &action_description, version)) {
    vlogf(LOG_BUG, "Error reading object from rent.");
    return NULL;
  }

  // this is here to catch "bad" items, or items that we otherwise want to
  // retire from the game
  // please make a note of what the item was, and why it is retired so we
  // can remove when they are outdated.
  if (
      // rabbit foot on a chain, component for defunct comp, Bat 7/13/99
      (item.item_number == 245) ||
      // soul of lost pet, component for defunct comp, Bat 7/13/99
      (item.item_number == 1413)
     ) {
    // these items are to be retired
    // its bad to return NULL here, so instead we will have it load a
    // hairball (vnum=0), that decays immediately
    const unsigned int OBJ_HAIRBALL = 1;
    item.item_number = OBJ_HAIRBALL; 
    item.decay_time = 0;
  }

  if (!(o = read_object(item.item_number, VIRTUAL))) {
    vlogf(LOG_BUG, format("Unable to load object Vnum = %d from rent.") %  item.item_number);
    return NULL;
  }

  // old items should reflect current tiny file
  // discard 0-cost components and symbols due to overhaul
  if (version >= 7 || dynamic_cast<TNote *>(o)) {

    if(version<9 && dynamic_cast<TOpenContainer *>(o)){
      item.value[1]=((item.value[1]>>8)<<16) ^ ((item.value[1]<<24)>>24);

      o->assignFourValues(item.value[0],item.value[1],item.value[2],item.value[3]);
    } else if(version<8 && dynamic_cast<TGenWeapon *>(o)){
      int x=0;

      // damage level and deviation are now merged
      SET_BITS(x, 7, 8, item.value[1]);
      SET_BITS(x, 15, 8, item.value[2]);

      o->assignFourValues(item.value[0],x,item.value[3],0);

    } else {
      o->assignFourValues(item.value[0],item.value[1],item.value[2],item.value[3]);
    }


    
    o->setObjStat(item.extra_flags);
    o->setWeight((float) item.weight);
    o->weightCorrection();
    
    o->setVolume(item.volume);
    o->obj_flags.bitvector = item.bitvector;
    o->obj_flags.struct_points = item.struct_points;
    o->obj_flags.max_struct_points = item.max_struct_points;
    o->obj_flags.decay_time = item.decay_time;
    o->setMaterial(item.material_points);
    o->setDepreciation(item.depreciation);
    
    o->obj_flags.cost = item.cost;
    
    o->updateDesc();
    
    for (j = 0; j < MAX_OBJ_AFFECT; j++) {
      o->affected[j].type = mapFileToSpellnum(item.affected[j].type);
      o->affected[j].level = item.affected[j].level;
      o->affected[j].duration = item.affected[j].duration;
      o->affected[j].renew = item.affected[j].renew;
      o->affected[j].location = mapFileToApply(item.affected[j].location);
      
      if (applyTypeShouldBeSpellnum(o->affected[j].location))
        o->affected[j].modifier = mapFileToSpellnum(item.affected[j].modifier);
      else
        o->affected[j].modifier = item.affected[j].modifier;
      
      o->affected[j].modifier2 = item.affected[j].modifier2;
      o->affected[j].bitvector = item.affected[j].bitvector;
    }
    // version 7 or less
  } else {
    if ((item.extra_flags & ITEM_STRUNG) && !o->isObjStat(ITEM_STRUNG))
      o->addObjStat(ITEM_STRUNG);  // preserve strung
    
    // deal with structure
    o->setStructPoints(min(item.struct_points, (int) obj_index[o->getItemIndex()].max_struct));
    
    if ((item.extra_flags & ITEM_CHARRED) && !o->isObjStat(ITEM_CHARRED))
      o->addObjStat(ITEM_CHARRED); // preserve charred

    if ((item.extra_flags & ITEM_RUSTY) && !o->isObjStat(ITEM_RUSTY))
      o->addObjStat(ITEM_RUSTY);
  }
    
  if (o->isObjStat(ITEM_STRUNG)) {
    if (name)
      o->name = name;
    else
      o->name = obj_index[o->getItemIndex()].name;
    
    if (shortDescr)
      o->shortDescr = shortDescr;
    else
      o->shortDescr = obj_index[o->getItemIndex()].short_desc;
    
    if (description)
      o->setDescr(description);
    else
      o->setDescr(obj_index[o->getItemIndex()].long_desc);
    
    if (action_description) 
      o->action_description = action_description;
    else if (obj_index[o->getItemIndex()].description) 
      o->action_description = obj_index[o->getItemIndex()].description;
    else 
      o->action_description = NULL;
    
    if (obj_index[o->getItemIndex()].ex_description)
      o->ex_description = new extraDescription(*obj_index[o->getItemIndex()].ex_description);
    else
      o->ex_description = NULL;

    //strung objects keep everything
    if(version<9 && dynamic_cast<TOpenContainer *>(o)){
      item.value[1]=((item.value[1]>>8)<<16) ^ ((item.value[1]<<24)>>24);

      o->assignFourValues(item.value[0],item.value[1],item.value[2],item.value[3]);
    } else if(version<8 && dynamic_cast<TGenWeapon *>(o)){
      int x=0;

      // damage level and deviation are now merged
      SET_BITS(x, 7, 8, item.value[1]);
      SET_BITS(x, 15, 8, item.value[2]);

      o->assignFourValues(item.value[0],x,item.value[3],0);

    } else {
      o->assignFourValues(item.value[0],item.value[1],item.value[2],item.value[3]);
    }
    
    o->setObjStat(item.extra_flags);
    o->setWeight((float) item.weight);
    o->weightCorrection();
    
    o->setVolume(item.volume);
    o->obj_flags.bitvector = item.bitvector;
    o->obj_flags.struct_points = item.struct_points;
    o->obj_flags.max_struct_points = item.max_struct_points;
    o->obj_flags.decay_time = item.decay_time;
    o->setMaterial(item.material_points);
    o->setDepreciation(item.depreciation);
    
    o->obj_flags.cost = item.cost;
    o->updateDesc();

    
    for (j = 0; j < MAX_OBJ_AFFECT; j++) {
      o->affected[j].type = mapFileToSpellnum(item.affected[j].type);
      o->affected[j].level = item.affected[j].level;
      o->affected[j].duration = item.affected[j].duration;
      o->affected[j].renew = item.affected[j].renew;
      o->affected[j].location = mapFileToApply(item.affected[j].location);
      
      if (applyTypeShouldBeSpellnum(o->affected[j].location))
        o->affected[j].modifier = mapFileToSpellnum(item.affected[j].modifier);
      else
        o->affected[j].modifier = item.affected[j].modifier;
      
      o->affected[j].modifier2 = item.affected[j].modifier2;
      o->affected[j].bitvector = item.affected[j].bitvector;
    }
  }
  // if they had a lantern lit, set light appropriately
  o->adjustLight();

  if((tmpcost = o->suggestedPrice())){
    o->obj_flags.cost = tmpcost;
  }

  // update the item's material type
  // this is noe done always - just checking at this load was circumvented by some DB loads
  /*if (CURRENT_RENT_VERSION > 9 &&
    convertV9MaterialToV10(o->getMaterial()) != o->getMaterial())
  {
    vlogf(LOG_OBJ, format("Object %s converting from material type %d to %d") %  o->getName() % o->getMaterial() % convertV9MaterialToV10(o->getMaterial()));
    o->setMaterial(convertV9MaterialToV10(o->getMaterial()));
  }*/

  return o;
}

TObj *ItemLoadDB::raw_read_item(int rent_id, int &slot)
{
  TDatabase db(DB_SNEEZY);
  TObj *o;

  db.query("select owner_type, owner, vnum, slot, container, val0, val1, val2, val3, extra_flags, weight, bitvector, decay, cur_struct, max_struct, material, volume, price, depreciation from rent where rent_id=%i and owner_type='%s' and owner=%i", rent_id, owner_type.c_str(), owner);

  if(!db.fetchRow()){
    vlogf(LOG_DB, format("rent object %i not found! Owner %i (%s)") % 
	  rent_id % owner % owner_type);
    return NULL;
  }

  if (!(o = read_object(convertTo<int>(db["vnum"]), VIRTUAL))) {
    vlogf(LOG_BUG, format("read_object failed on rent object %s. Owner %i (5s)") %
	  db["vnum"] % owner % owner_type);
    return NULL;
  }

  o->setObjStat(convertTo<int>(db["extra_flags"]));
  o->setWeight((float) convertTo<float>(db["weight"]));
  o->weightCorrection();
  
  o->setVolume(convertTo<int>(db["volume"]));
  o->obj_flags.bitvector = convertTo<int>(db["bitvector"]);
  o->obj_flags.struct_points = convertTo<int>(db["cur_struct"]);
  o->obj_flags.max_struct_points = convertTo<int>(db["max_struct"]);
  o->obj_flags.decay_time = convertTo<int>(db["decay"]);
  o->setMaterial(convertTo<int>(db["material"]));
  o->setDepreciation(convertTo<int>(db["depreciation"]));
  o->assignFourValues(convertTo<int>(db["val0"]),
		      convertTo<int>(db["val1"]),
		      convertTo<int>(db["val2"]),
		      convertTo<int>(db["val3"]));
  
  o->obj_flags.cost = convertTo<int>(db["price"]);

  o->updateDesc();
  
  db.query("select type, level, duration, renew, modifier, location, modifier2, bitvector from rent_obj_aff where rent_id=%i", rent_id);

  for(int j=0;db.fetchRow() && j < MAX_OBJ_AFFECT; j++){
    o->affected[j].type = mapFileToSpellnum(convertTo<int>(db["type"]));
    o->affected[j].level = convertTo<int>(db["level"]);
    o->affected[j].duration = convertTo<int>(db["duration"]);
    o->affected[j].renew = convertTo<int>(db["renew"]);
    o->affected[j].location = mapFileToApply(convertTo<int>(db["location"]));
    
    if (applyTypeShouldBeSpellnum(o->affected[j].location))
      o->affected[j].modifier = mapFileToSpellnum(convertTo<int>(db["modifier"]));
    else
      o->affected[j].modifier = convertTo<int>(db["modifier"]);
    
    o->affected[j].modifier2 = convertTo<int>(db["modifier2"]);
    o->affected[j].bitvector = convertTo<int>(db["bitvector"]);
  }

  // NULL it all out
  o->name = NULL;
  o->shortDescr = NULL;
  o->setDescr(NULL);
  o->action_description = NULL;
  o->ex_description = NULL;

  // override defaults from table
  if (o->isObjStat(ITEM_STRUNG)) {

    // get defaults
    sstring name = obj_index[o->getItemIndex()].name;
    sstring shortDesc = obj_index[o->getItemIndex()].short_desc;
    sstring longDesc = obj_index[o->getItemIndex()].long_desc;
    sstring actionDesc = obj_index[o->getItemIndex()].description;

    db.query("select name, short_desc, long_desc, action_desc from rent_strung where rent_id=%i", rent_id);
    if (db.fetchRow()) {
      if (!db["name"].empty())
        name = db["name"];   
      if (!db["short_desc"].empty())
        shortDesc = db["short_desc"];
      if (!db["long_desc"].empty())
        longDesc = db["long_desc"];
      if (!db["action_desc"].empty()) 
        actionDesc = db["action_desc"];
    }

    // set defaults on obj
    if (!name.empty())
      o->name = name;
    if (!shortDesc.empty())
      o->shortDescr = shortDesc;
    if (!longDesc.empty())
      o->setDescr(longDesc);
    if (!actionDesc.empty())
      o->action_description = actionDesc;

    if (obj_index[o->getItemIndex()].ex_description)
      o->ex_description = new extraDescription(*obj_index[o->getItemIndex()].ex_description);

  } else {
    // un-strung object - these pointers just point to static memory
    o->name = obj_index[o->getItemIndex()].name;
    o->shortDescr = obj_index[o->getItemIndex()].short_desc;
    o->setDescr(obj_index[o->getItemIndex()].long_desc);
    o->action_description = obj_index[o->getItemIndex()].description;
  }

  // if they had a lantern lit, set light appropriately
  o->adjustLight();

  int tmpcost;
  if((tmpcost = o->suggestedPrice())){
    o->obj_flags.cost = tmpcost;
  }

  
  return o;
}


  
static bool shouldRecycle(int robj)
{
  return false;

  // nuke only if item is at its max
  //  return (obj_index[robj].max_exist &&
  //      obj_index[robj].number >= obj_index[robj].max_exist);
}

static bool immortalityNukeCheck(TBeing *ch, TObj * new_obj, bool corpse)
{
  return false;

  bool immortal = ((ch && ch->desc) ? IS_SET(ch->desc->account->flags, TAccount::IMMORTAL) : FALSE);

  if (!corpse && immortal && shouldRecycle(new_obj->getItemIndex())) {
    char buf[1200];
    sprintf(buf, "Item (%s) was automatically recycled due to your immortal status.\n\r", new_obj->getName().c_str());
    autoMail(ch, NULL, buf);
    vlogf(LOG_SILENT, format("%s's %s being recycled due to immortality.") %  ch->getName() % new_obj->getName());


    delete new_obj;
    new_obj = NULL;
    return true;
  }
  return false;
}


bool ItemLoad::objToParent(signed char slot, TObj *parent, TObj *new_obj, TRoom *r, TBeing *ch)
{
  //  vlogf(LOG_PEEL, format("objToParent: %s") % new_obj->name);

  if(slot != NORMAL_SLOT){
    if (r)
      vlogf(LOG_BUG, format("Room %d.  Invalid Slot %d.") %

	    r->number % slot);
    else if (ch)
      vlogf(LOG_BUG, format("%s's objects.  Invalid slot %d.") %
	    ch->getName() % slot);
    vlogf(LOG_BUG, "Error in objsFromStore (3)");
    return false;
  }
  
  *parent += *new_obj;

  return true;
}

#if 0

bool ItemLoadDB::objToParent(signed char slot, TObj *parent, TObj *new_obj, TRoom *r, TBeing *ch)
{
  //  vlogf(LOG_PEEL, format("objToParent: %s") % new_obj->name);

  if(slot != NORMAL_SLOT){
    if (r)
      vlogf(LOG_BUG, format("Room %d.  Invalid Slot %d.") %
	    r->number % slot);
    else if (ch)
      vlogf(LOG_BUG, format("%s's objects.  Invalid slot %d.") %
	    ch->getName() % slot);
    vlogf(LOG_BUG, "Error in objsFromStore (3)");
    return false;
  }
  
  *parent += *new_obj;

  return true;
}
#endif


bool ItemLoad::objToEquipChar(unsigned char slot, TBeing *ch, TObj *new_obj, TRoom *r)
{
  //  vlogf(LOG_PEEL, format("objToEquipChar: %s") % new_obj->name);

  if (ch) {
    wearSlotT mapped_slot = mapFileToSlot( slot);
    if (!ch->canUseLimb(mapped_slot))
      *ch += *new_obj;
    else
      ch->equipChar(new_obj, mapped_slot, SILENT_YES);
  } else {
    vlogf(LOG_BUG, format("Room %d has invalid slot #.") %  
	  ((r) ? r->number : -99));
    return false;
  }
  return true;
}

#if 0
bool ItemLoadDB::objToEquipChar(unsigned char slot, TBeing *ch, TObj *new_obj, TRoom *r)
{
  //  vlogf(LOG_PEEL, format("objToEquipChar: %s") % new_obj->name);

  if (ch) {
    wearSlotT mapped_slot = mapFileToSlot( slot);
    if (!ch->canUseLimb(mapped_slot))
      *ch += *new_obj;
    else
      ch->equipChar(new_obj, mapped_slot, SILENT_YES);
  } else {
    vlogf(LOG_BUG, format("Room %d has invalid slot #.") %  
	  ((r) ? r->number : -99));
    return false;
  }
  return true;
}
#endif

bool ItemLoad::objToTarg(unsigned char slot, TBeing *ch, TObj *new_obj, TRoom *r)
{
  //  vlogf(LOG_PEEL, format("objToTarg: %s") % new_obj->name);

  if (ch)
    *ch += *new_obj;
  else if (r)
    thing_to_room(new_obj, r->number);
  else {
    vlogf(LOG_BUG, "Yikes!  An object was read with no destination in objsFromStore()!");
    return false;
  }

  return true;
}

#if 0    
bool ItemLoadDB::objToTarg(unsigned char slot, TBeing *ch, TObj *new_obj, TRoom *r)
{
  //  vlogf(LOG_PEEL, format("objToTarg: %s") % new_obj->name);

  if (ch)
    *ch += *new_obj;
  else if (r)
    thing_to_room(new_obj, r->number);
  else {
    vlogf(LOG_BUG, "Yikes!  An object was read with no destination in objsFromStore()!");
    return false;
  }

  return true;
}
#endif    

// read a list of items and their contents from storage 
bool ItemLoad::objsFromStore(TObj *parent, int *numread, TBeing *ch, TRoom *r, bool corpse)
{
  signed char slot;
  TObj *new_obj;

  while (!feof(fp)) {
    //// read slot
    if (fread(&slot, sizeof(signed char), 1, fp) != 1) {
      if (r)
        vlogf(LOG_BUG, format("  Room %d.  Couldn't read slot.") %  r->number);
      else if (ch)
        vlogf(LOG_BUG, format(" %s's objects.  Couldn't read slot.") %
	      ch->getName());
      else
        vlogf(LOG_BUG, "Error in objsFromStore (1)");

      return true;
    } else if (slot >= MAX_WEAR) {
      if (ch)
	vlogf(LOG_BUG, format("%s's objects.  Slot %d > MAX_WEAR.") %
	      ch->getName() % slot);
      else if (r)
	vlogf(LOG_BUG, format("Room %d's objects.  Slot %d > MAX_WEAR.") %
	      r->number % slot);
      vlogf(LOG_BUG, "Error in objsFromStore (4)");
      return true;
    }
    
    if (slot == CONTENTS_END)
      return false;


    //// load the object
    if(!(new_obj = raw_read_item())){
      vlogf(LOG_BUG, "Error in objsFromStore (raw_read_item)");
      return true;
    }

    //// place the object
    bool failedToParent = false;
    if (parent)
      failedToParent = !objToParent(slot, parent, new_obj, r, ch);
    else if (slot == NORMAL_SLOT)
      failedToParent = !objToTarg(slot, ch, new_obj, r);
    else
      failedToParent = !objToEquipChar(slot, ch, new_obj, r);

    if (failedToParent) {
      delete new_obj;
      vlogf(LOG_BUG, "Error parenting in objesFromStore");
      return true;
    }

    // recursively load any contained objects
    if (objsFromStore(new_obj, numread, ch, r, corpse)) {
      vlogf(LOG_BUG, "Error in objsFromStore (1)");
      delete new_obj;
      return true;  // ERROR occured 
    }
    
    if (immortalityNukeCheck(ch, new_obj, corpse))
      continue;  // new_obj invalid if this was true
    
    (*numread)++;
    if (ch)
      ch->logItem(new_obj, CMD_WEST); // rent in
    obj_index[new_obj->number].addToNumber(-1);

    repoCheckForRent(ch, new_obj, corpse);
  }

  return false;
}

#if 0
// read a list of items and their contents from storage 
bool ItemLoadDB::objsFromStore(TObj *parent, int *numread, TBeing *ch, TRoom *r, bool corpse)
{
  signed char slot;
  int container, rent_id;
  TObj *new_obj;
  TDatabase db(DB_SNEEZY);

  db.query("select rent_id, owner_type, owner, vnum, slot, container, val0, val1, val2,val3, extra_flags, weight, bitvector, decay, cur_struct, max_struct, material, volume, price, depreciation from rent where owner_type='%s' and owner=%i order by rent_id", owner_type, owner);

  while(db.fetchRow()){
    slot=convertTo<int>(db["slot"]);
    container=convertTo<int>(db["container"]);
    rent_id=convertTo<int>(db["rent_id"]);

    if (slot >= MAX_WEAR) {
      if (ch)
	vlogf(LOG_BUG, format("%s's objects.  Slot %d > MAX_WEAR.") %
	      ch->getName() % slot);
      else if (r)
	vlogf(LOG_BUG, format("Room %d's objects.  Slot %d > MAX_WEAR.") %
	      r->number % slot);
      vlogf(LOG_BUG, "Error in objsFromStore (4)");
      return true;
    }
    
    if (slot == CONTENTS_END)
      return false;


    //// load the object
    if(!(new_obj = raw_read_item())){
      vlogf(LOG_BUG, "Error in objsFromStore (raw_read_item)");
      return true;
    }

    (*numread)++;
    if(ch)
      ch->logItem(new_obj, CMD_WEST); // rent in
    obj_index[new_obj->number].addToNumber(-1);

    //// place the object
    if(container && parent){
      if(!objToParent(slot, parent, new_obj, r, ch))
	return true;
    } else {
      if(slot == NORMAL_SLOT){
	if(!objToTarg(slot, ch, new_obj, r))
	  return true;
      } else {
	if(!objToEquipChar(slot, ch, new_obj, r))
	  return true;
      }
    }

    
    if (immortalityNukeCheck(ch, new_obj, corpse))
      continue;  // new_obj invalid if this was true
    
    repoCheckForRent(ch, new_obj, corpse);

    if(!container)
      parent=new_obj;
  }

  return false;
}
#endif

void ItemSave::setFile(FILE *f)
{
  fp=f;
}

void ItemLoad::setFile(FILE *f)
{
  fp=f;
}

void ItemLoad::setVersion(unsigned char v)
{
  version=v;
}



// write a list of items and their contents to storage. 
// recursive
//
// slot = slot the item is worn on, if worn (NORMAL_SLOT if in inventory)
// o = object to save
// ch = character that is saving items
// d = delete the item after saving (for renting)
// corpse = indicate if pcorpse saving items
void ItemSave::objToStore(signed char slot, TObj *o, 
			   TBeing *ch, bool d, bool corpse = FALSE)
{
  if(!o)
    return;
  
  if (!o->isRentable()) {
    objsToStore(NORMAL_SLOT, o->stuff, ch, d, corpse);
    
    // normal item, save it
  } else {
    // write out the slot
    if (fwrite(&slot, sizeof(signed char), 1, fp) != 1) {
      vlogf(LOG_BUG, format("Error saving %s's objects -- slot write.") %
	    (ch?ch->getName():"unknown"));
      return;
    }
    
    (st.number)++;
    
    // write out the item
    if (!raw_write_item(o)) 
      vlogf(LOG_BUG, format("Rent error in %s's file") %
	    (ch?ch->getName():"UNKNOWN"));
    
    // save the contents
    objsToStore(NORMAL_SLOT, o->stuff, ch, d, corpse);
    
    // write the contents footer
    slot = CONTENTS_END;
    if (fwrite(&slot, sizeof(signed char), 1, fp) != 1) {
      vlogf(LOG_BUG, format("Error saving %s's objects -- slot write (2).") %  
	    ((ch) ? ch->getName() : "UNKNOWN"));
      return;
    }      
  }
  
  // delete the item if d is specified
  if (d) {
    if (o->parent)
      --(*o);
    if (o->riding) {
      // on a table?
      vlogf(LOG_BUG, "Error in table doing save");
    }
    ch->logItem(o, CMD_RENT);
    if (o->number >= 0)
      obj_index[o->number].addToNumber(1);
    
    delete o;
    o = NULL;
  }
}

void ItemSave::objsToStore(signed char slot, StuffList list, 
			   TBeing *ch, bool d, bool corpse = FALSE)
{
  TObj *o=NULL;

  for(StuffIter it=list.begin();it!=list.end();){
    if(!(o=dynamic_cast<TObj *>(*(it++))))
      continue;

    objToStore(slot, o, ch, d, corpse);
  }
}

void ItemSaveDB::objToStore(signed char slot, TObj *o,
			     TBeing *ch, bool d, bool corpse = FALSE,
			     int container=0)
{
  if(!o)
    return;
  
  if (!o->isRentable()) {
    objsToStore(NORMAL_SLOT, o->stuff, ch, d, corpse, container);
    // normal item, save it
  } else {
    // write out the item
    int rent_id=raw_write_item(o, slot, container);
    
    // save the contents
    objsToStore(NORMAL_SLOT, o->stuff, ch, d, corpse, rent_id);
  }
  
  // delete the item if d is specified
  if (d) {
    if (o->parent)
      --(*o);
    if (o->riding) {
      // on a table?
      vlogf(LOG_BUG, "Error in table doing save");
    }
    ch->logItem(o, CMD_RENT);
    if (o->number >= 0)
      obj_index[o->number].addToNumber(1);
    
    delete o;
    o = NULL;
  }
}

void ItemSaveDB::objsToStore(signed char slot, StuffList list, 
			   TBeing *ch, bool d, bool corpse = FALSE,
			     int container=0)
{
  TObj *o=NULL;

  for(StuffIter it=list.begin();it!=list.end();++it){
    if(!(o=dynamic_cast<TObj *>(*it)))
      continue;
    
    objToStore(slot, o, ch, d, corpse, container);
  }
}

void ItemSaveDB::clearRent()
{
  TDatabase db(DB_SNEEZY);

  db.query("delete rof from rent_obj_aff rof, rent r where r.rent_id=rof.rent_id and r.owner_type='%s' and r.owner=%i", owner_type.c_str(), owner);
  
  db.query("delete rs from rent r, rent_strung rs where r.owner=%i and r.owner_type='%s' and r.rent_id=rs.rent_id", owner, owner_type.c_str());

  db.query("delete r from rent r where r.owner=%i and r.owner_type='%s'", owner, owner_type.c_str());

}

// returns rent_id in database
int TMonster::saveItem(int shop_nr, TObj *obj, int container)
{
  ItemSaveDB is("shop", shop_nr);

  shop_index[shop_nr].addToInventoryCount(1);
  return is.raw_write_item(obj, NORMAL_SLOT, container);
}

// saves over an old rent_id, use with caution
int TMonster::saveItem(int shop_nr, int rent_id, TObj *obj, int container)
{
  ItemSaveDB is("shop", shop_nr);

  shop_index[shop_nr].addToInventoryCount(1);
  return is.raw_write_item(obj, NORMAL_SLOT, container, rent_id);
}


TObj *TMonster::loadItem(int shop_nr, int rent_id)
{
  ItemLoadDB il("shop", shop_nr);
  int slot=-1;

  return il.raw_read_item(rent_id, slot);
}

void TMonster::deleteItem(int shop_nr, int rent_id)
{
  TDatabase db(DB_SNEEZY);

  db.query("delete from rent where rent_id=%i", rent_id);
  db.query("delete from rent_obj_aff where rent_id=%i", rent_id);
  db.query("delete from rent_strung where rent_id=%i", rent_id);

  shop_index[shop_nr].addToInventoryCount(-1);
}

void TMonster::saveItems(int shop_nr)
{
  /*ItemSaveDB is("shop", shop_nr);
  TObj *obj;
  is.clearRent();

  // store worn objects
  wearSlotT ij;
  for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
    obj = dynamic_cast<TObj *>(equipment[ij]);
    if (!obj)
      continue;
    if (!(((ij == WEAR_LEG_L) && obj->isPaired()) ||
          ((ij == WEAR_EX_LEG_L) && obj->isPaired()) ||
          ((ij == HOLD_LEFT) && obj->isPaired()))) {
      is.objToStore(mapSlotToFile(ij), obj, this, FALSE);
    }
  }

  // store inventory objects
  is.objsToStore(NORMAL_SLOT, stuff, this, FALSE);*/

  // shopkeeper specific stuff - save gold
  if(isShopkeeper()){
    TDatabase db(DB_SNEEZY);
    db.query("update shopowned set gold=%i where shop_nr=%i",
	     getMoney(), find_shop_nr(number));
  }
}

void TRoom::saveItems(const sstring &)
{
  sstring filepath;
  ItemSave is;

  filepath = format("%s/%d") % ROOM_SAVE_PATH % number;

  if(stuff.empty()){
    unlink(filepath.c_str());
    return;
  }

  if(!is.openFile(filepath)){
    vlogf(LOG_BUG, format("Error saving room [%d] items.") %  number);
    return;
  }
  is.writeVersion();

  is.objsToStore(NORMAL_SLOT, stuff, NULL, FALSE);
  is.writeFooter();
}


void emailStorageBag(sstring tStMessage, sstring tStSender, TThing * tStuff)
{
  FILE * tFile;
  sstring tStMail("");

  if (gamePort != Config::Port::PROD)
    return;

  if (!(tFile = fopen("storage.temp", "w")))
    return;

  tStMail += "Subject: [Storage] " + tStSender + " " + tStMessage + "\n\r";
  tStMail += "This is an automated message sent by sneezy.\n\r";

  fprintf(tFile, "%s", tStMail.c_str());

  fclose(tFile);
}

void TRoom::loadItems()
{
  sstring filepath;
  int num_read;
  int reset;
  ItemLoad il;

  filepath = format("%s/%d") % ROOM_SAVE_PATH % number;

  if(!il.fileExists(filepath))
    return;

  if(!il.openFile(filepath)) {
    vlogf(LOG_FILE, format("Failed to open file '%s' in TRoom::loadItems() call.") % filepath);
    return;
  }
  
  reset = isRoomFlag(ROOM_SAVE_ROOM);
  // we need to fool obj_to_room into not trying to resave    
  // the room file when it loads objects into a SAVE_ROOM ... 
  if (reset)
    removeRoomFlagBit(ROOM_SAVE_ROOM);

  if(!il.readVersion()){
    vlogf(LOG_BUG, format("Error while reading version from %s.") % filepath);
    if (reset)
      setRoomFlagBit(ROOM_SAVE_ROOM);
    return;
  }

  il.objsFromStore(NULL, &num_read, NULL, this, false);
  if (reset)
    setRoomFlagBit(ROOM_SAVE_ROOM);

  // This is an automated System designed to help keep storage clean.
  // If a bag is in here that is a linkbag then we mail the owner to
  // let them know it is still here and that they need to let us know
  // if they want it or if we should purge it.  Combined with this we
  // clean up non-linkbag objects and place it into another bag to be
  // neat and tidy.  We also look for the link-note that we put inside
  // the bag and purge the bag if it's here for too long.
  if (number == Room::STORAGE) {
    /*vlogf(LOG_LOW, "Storage: Booting Storage Room");

    TThing * tThing,
      * tCont=NULL;
    TObj   * tBag = read_object(Obj::GENERIC_L_BAG, VIRTUAL);
    TBag   * tContainer;
    char     tString[256];
    charFile tSt;

    if (!tBag) {
      vlogf(LOG_LOW, "Storage: Failed to create Junk Bag.");
      return;
    }

    for(StuffIter it=stuff.begin();it!=stuff.end();){
      tThing=*(it++);

      // Remove various things.
      if (!(tContainer = dynamic_cast<TBag *>(tThing))) {
        vlogf(LOG_LOW, format("Storage: Moving Junk: %s") %  tThing->name);
        --(*tThing);
        *tBag += *tThing;
        continue;
      }

      // Remove old junk bags.
      if (sscanf(tThing->name, "linkbag %[A-Za-z]", tString) != 1) {
        vlogf(LOG_LOW, "Storage: Moving Old Junk Bag");
	for(StuffIter it=tThing->stuff.begin();it!=tThing->stuff.end();){
	  TThing *tTemp=*(it++);
          --(*tTemp);
          *tBag += *tTemp;
        }

        --(*tThing);
        delete tThing;

        continue;
      }

      // Now we verify the 'user'.  tString should have been set prior.
      if (!load_char(tString, &tSt)) {
        if (gamePort == Config::Port::PROD)
          emailStorageBag("User Deleted", tString, tThing);

        vlogf(LOG_LOW, format("Storage: Purging linkbag: %s") %  tString);
        --(*tThing);
        delete tThing;

        continue;
      }

      vlogf(LOG_LOW, format("Storage: Processing Linkbag: %s") %  tString);

      // If we got here, the bag is a linkbag and the player is around.
      for(StuffIter it=tThing->stuff.begin();it!=tThing->stuff.end() && (tCont=*it);++it) {
        TNote * tNote = dynamic_cast<TNote *>(tCont);

        if (!tNote)
          continue;
 
        int  tDay,
             tHour,
             tMin,
             tSec,
             tYear;
        char tMon[4],
             tWek[4];

	//Current time is: Mon Mar 20 00:40:14 2000 (PST)
        if (sscanf(tNote->action_description,
                   "Current time is: %s %s %d %d:%d:%d %d (%*s)",
                   tWek, tMon, &tDay, &tHour, &tMin, &tSec, &tYear) != 7) {
          vlogf(LOG_LAPSOS, format("Storage: Note:\n\r%s") %  tNote->action_description);
          continue;
        }

        struct tm tTime;

        tTime.tm_sec = tSec;
        tTime.tm_min = tMin;
        tTime.tm_mday = tDay;
        tTime.tm_year = (tYear - 1900);
        time_t tCurrentTime = time(0);
        tTime.tm_isdst = localtime(&tCurrentTime)->tm_isdst;

        if (!strcmp(tMon, "Jan"))
          tTime.tm_mon = 0;
        else if (!strcmp(tMon, "Feb"))
          tTime.tm_mon = 1;
        else if (!strcmp(tMon, "Mar"))
          tTime.tm_mon = 2;
        else if (!strcmp(tMon, "Apr"))
          tTime.tm_mon = 3;
        else if (!strcmp(tMon, "May"))
          tTime.tm_mon = 4;
        else if (!strcmp(tMon, "Jun"))
          tTime.tm_mon = 5;
        else if (!strcmp(tMon, "Jul"))
          tTime.tm_mon = 6;
        else if (!strcmp(tMon, "Aug"))
          tTime.tm_mon = 7;
        else if (!strcmp(tMon, "Sep"))
          tTime.tm_mon = 8;
        else if (!strcmp(tMon, "Oct"))
          tTime.tm_mon = 9;
        else if (!strcmp(tMon, "Nov"))
          tTime.tm_mon = 10;
        else if (!strcmp(tMon, "Dec"))
          tTime.tm_mon = 11;
        else {
          vlogf(LOG_BUG, format("Storage: Unknown Month: %s") %  tMon);
          tTime.tm_mon = 0;
        }

        if (!strcmp(tWek, "Sun"))
          tTime.tm_wday = 0;
        else if (!strcmp(tWek, "Mon"))
          tTime.tm_wday = 1;
        else if (!strcmp(tWek, "Tue"))
          tTime.tm_wday = 2;
        else if (!strcmp(tWek, "Wed"))
          tTime.tm_wday = 3;
        else if (!strcmp(tWek, "Thu"))
          tTime.tm_wday = 4;
        else if (!strcmp(tWek, "Fri"))
          tTime.tm_wday = 5;
        else if (!strcmp(tWek, "Sat"))
          tTime.tm_wday = 6;
        else {
          vlogf(LOG_BUG, format("Storage: Unknown Day: %s") %  tWek);
          tTime.tm_wday = 1;
        }

        tDay = 0;

        switch (tTime.tm_mon) {
          case 10:
            tDay += 31;
          case 9:
            tDay += 30;
          case 8:
            tDay += 31;
          case 7:
            tDay += 30;
          case 6:
            tDay += 31;
          case 5:
            tDay += 31;
          case 4:
            tDay += 30;
          case 3:
            tDay += 31;
          case 2:
            tDay += 30;
          case 1:
            tDay += 31;
          case 0:
            tDay += (!(((1900 + tTime.tm_year) - 1996) % 4) ? 29 : 28);
        }

        tDay += (tTime.tm_mday - 1);
        tTime.tm_yday = tDay;

        time_t tTempReal = mktime(&tTime);
        vlogf(LOG_LAPSOS, format("Storage: %s") %  ctime(&tTempReal));

        double tTimeDiff = difftime(tCurrentTime, mktime(&tTime)),
               tCheck    = 60.0 * 60.0 * 24.0 * 30.0;

        // Allow a bag to be 'retained' for 30 days.
        if (tTimeDiff > tCheck || tTimeDiff < -tCheck) {
          if (gamePort == Config::Port::PROD)
            emailStorageBag("Time Expired", tString, tThing);

          vlogf(LOG_LOW, format("Storage: Expired: %s") %  tString);

	  for(StuffIter it=tThing->stuff.begin();it!=tThing->stuff.end();){
	    TThing *tTemp=*(it++);
            --(*tTemp);
            *tBag += *tTemp;
          }

          --(*tThing);
          delete tThing;
	}

        break;
      }

      if (!tCont)
        vlogf(LOG_LOW, format("Storage: Unable to find rent note for: %s") %  tString);
    }

    if (tBag->stuff.empty())
      delete tBag;
    else {
      sstring tStString("");

      tBag->swapToStrung();
      tBag->addObjStat(ITEM_NOPURGE);

      delete [] tBag->getDescr();
      delete [] tBag->name;

      tStString = "A bag containing various junk.";
      tBag->setDescr(tStString);
      tStString = "bag junk various [wizard]";
      tBag->name = tStString;

      *this += *tBag;
    }*/
  }

}

void updateSavedRoom(const char *tfname)
{
  char fileName[128];
  FILE *fp;
  unsigned char version;

  if (!tfname) {
    vlogf(LOG_BUG, "  updateSavedRoom called with NULL filename!");
    return;
  }
  sprintf(fileName, "%s/%s", ROOM_SAVE_PATH, tfname);
  if (!(fp = fopen(fileName, "r+b"))) {
    vlogf(LOG_BUG, format("  Error opening the room save file for room #%s") %  tfname);
    return;
  }

  if (fread(&version, sizeof(version), 1, fp) != 1) {
    vlogf(LOG_BUG, format("Error reading version from %s.") %  fileName);
    fclose(fp);
    return;
  }

  if (!noteLimitedItems(fp, fileName, version, FALSE))
    vlogf(LOG_BUG, format("  Unable to count limited items in file  %s") %  fileName);
  fclose(fp);
}

void updateSavedRoomItems(void)
{
  dirwalk(ROOM_SAVE_PATH, updateSavedRoom);
}

void TPCorpse::removeCorpseFromList(bool updateFile)
{
  TPCorpse * tmpCorpse = NULL;
  bool found = FALSE;
  TPCorpse * otherCorpse = NULL;

  if (name.empty() || (name == "corpse player dummy") || !isObjStat(ITEM_STRUNG))
    return;

  if (!pc_corpse_list) {
    if (nextGlobalCorpse || checkOnLists())
      vlogf(LOG_BUG, "Error in removeCorpseList-- nextGlobalCorpse and no pc_corpse_list at all");
    found = TRUE;
  } else if ((this == pc_corpse_list)) {
    if (nextGlobalCorpse)
      pc_corpse_list = nextGlobalCorpse;
    else
      pc_corpse_list = NULL;
    found = TRUE;
  } else {
    for (tmpCorpse = pc_corpse_list; tmpCorpse; tmpCorpse = tmpCorpse->nextGlobalCorpse) {
      if (tmpCorpse == this) {
        vlogf(LOG_BUG,"Error in removeCorpseFromLists");
        break;
      }
      if (tmpCorpse->nextGlobalCorpse == this) {
        if (nextGlobalCorpse)
          tmpCorpse->nextGlobalCorpse = nextGlobalCorpse;
        else
          tmpCorpse->nextGlobalCorpse = NULL;
        found = TRUE;
        break;
      }
    }
  }
  if (!found && checkOnLists()) {
    vlogf(LOG_BUG, format("Error in removeCorpseList, corpse says listed but no corpse in list (%s).") %  getName());
  }
  if (previousCorpse) {
    otherCorpse = previousCorpse;
    if (nextCorpse)
      previousCorpse->nextCorpse = nextCorpse;
    else 
      previousCorpse->nextCorpse = NULL;
  }
  if (nextCorpse) {
    if (!otherCorpse)
      otherCorpse = nextCorpse;
    if (previousCorpse)
      nextCorpse->previousCorpse = previousCorpse;
    else
      nextCorpse->previousCorpse = NULL;
    nextCorpse->previousCorpse = previousCorpse;
  }
  if (!otherCorpse && !fileName.empty() && pc_corpse_list) {
    for (otherCorpse = pc_corpse_list; otherCorpse; otherCorpse = otherCorpse->nextGlobalCorpse) {
      if (otherCorpse == this) {
        vlogf(LOG_BUG, format("Big error in corpse list walking %s") %  fileName);
        continue;
      }
      if (!otherCorpse->fileName.empty() && !otherCorpse->fileName.compare(fileName))
        break;
    }
  }
  nextCorpse = NULL;
  previousCorpse = NULL;
  nextGlobalCorpse = NULL;
  setRoomNum(0);
  setNumInRoom(0);
  togOnCorpseListsOff();
  if (updateFile) {
    if (otherCorpse) {
      otherCorpse->saveCorpseToFile(); 
    } else if (!fileName.empty()) {
      wipeCorpseFile(fileName.c_str());
    }
  }
}

void TPCorpse::addCorpseToLists()
{
  TPCorpse * tmpCorpse = NULL;
  int numCorpsesInRoom = 1;
  bool found = FALSE;
  if (stuff.empty())
    return;

  if (checkOnLists())
    vlogf(LOG_BUG, format("Call to addCorpseToList for a corpse already on list (%s)") %  getName());
  if (!pc_corpse_list) {
    pc_corpse_list = this;
    nextGlobalCorpse = NULL;
  } else {
    nextGlobalCorpse = pc_corpse_list;
    pc_corpse_list = this;
  }
  previousCorpse = NULL;
  nextCorpse = NULL;
  for (tmpCorpse = pc_corpse_list; tmpCorpse; tmpCorpse  = tmpCorpse->nextGlobalCorpse) {
    if (tmpCorpse == this)
      continue;
    if (!fileName.compare(tmpCorpse->fileName)) {
      if (tmpCorpse->previousCorpse) {
        previousCorpse = tmpCorpse->previousCorpse;
        tmpCorpse->previousCorpse->nextCorpse = this;
        found = TRUE;
      } 
      tmpCorpse->previousCorpse = this;
      nextCorpse = tmpCorpse;
      break;
    }
  }

  for (tmpCorpse = this; tmpCorpse && tmpCorpse->previousCorpse; tmpCorpse = tmpCorpse->previousCorpse)

  if (found) {
    for (; tmpCorpse;tmpCorpse = tmpCorpse->nextCorpse) {
      if (tmpCorpse == this)
        continue;
      if (tmpCorpse->roomp && roomp && (tmpCorpse->roomp == roomp))
        numCorpsesInRoom++;
    }
  }
  num_corpses_in_room = numCorpsesInRoom;
  togOnCorpseListsOn();
}

void TBeing::assignCorpsesToRooms() 
{
  TRoom *rp = NULL, *rp2 = NULL;
  TThing *tmp;
  TPCorpse *corpse = NULL;
  char buf[256];
//  char *buf2;
// char *buf3;
//  char buf2[80];
//  char buf3[80];
  int num_read = 0;
  bool reset = FALSE;
  FILE *playerFile;
  memset(buf, '\0', sizeof(buf));
  ItemLoad il;

  sprintf(buf, "corpses/%s", sstring(name).lower().c_str());
  rp = real_roomp(Room::CORPSE_STORAGE);

// HAVE A BEING CALL THIS WHEN LOGGING IN
//  sprintf(buf, "rent/%c/%s", LOWER(tmp->name[0]), tmp->name.lower());


  if(!il.openFile(buf)) {
    // this isn't an error really, just means they don't have a corpse in
    // the game
    //    vlogf(LOG_FILE, format("Failed to open file '%s' in assignCorpsesToRooms() call.") % buf);
    return;
  }

  sprintf(buf, "player/%c/%s", LOWER(name[0]), sstring(name).lower().c_str());
  if (!(playerFile = fopen(buf, "r"))) {
    wipeCorpseFile(sstring(name).lower().c_str());
  } 
  fclose(playerFile);

  if (GetMaxLevel() > MAX_MORT) {
    vlogf(LOG_BUG, format("An immortal had a corpse saved (%s).") %  getName());
    wipeCorpseFile(sstring(name).lower().c_str());
    return;
  }

  if(!il.readHeader()){
    vlogf(LOG_BUG, format("Error while reading %s's corpse file header.") %
	  getName());
    return;
  }

  reset = rp->isRoomFlag(ROOM_SAVE_ROOM);
  // we need to fool obj_to_room into not trying to resave
  // the room file when it loads objects into a SAVE_ROOM ...
  if (reset)
    rp->removeRoomFlagBit(ROOM_SAVE_ROOM);

  if (il.objsFromStore(NULL, &num_read, NULL, rp, TRUE)) {
    vlogf(LOG_BUG, format("Error while reading %s's corpse file. Prepare for reimb!") % getName());
    if (reset)
      rp->setRoomFlagBit(ROOM_SAVE_ROOM);
    return;
  }
  if (reset)
    rp->setRoomFlagBit(ROOM_SAVE_ROOM);

  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end();){
    tmp=*(it++);
    corpse = dynamic_cast<TPCorpse *>(tmp);
// tmp has to be here
    if (!corpse) {
      continue;
    }
    if (isname(name, corpse->name)) {
      corpse->setOwner(sstring(name).lower().c_str());
      corpse->obj_flags.decay_time = max(corpse->obj_flags.decay_time, (short int) 60);
//      corpse->obj_flags.decay_time = max(corpse->obj_flags.decay_time, MAX_PC_CORPSE_EQUIPPED_TIME);
    }
    if (!corpse->getRoomNum()) {
      rp2 = real_roomp(Room::STORAGE);
      if (corpse->roomp)
        --(*corpse);
      *rp2 += *corpse; 
      vlogf(LOG_BUG, format("%s distributed to Storage Room (%d).") %  corpse->getName() % Room::STORAGE);
      sendTo(COLOR_BASIC, "<r>*** You had a CORPSE placed in the storage area. See a god to get it back. *** <z>\n\r");
    } else {
      rp2 = real_roomp(corpse->getRoomNum());
      if (!rp2) {
        rp2 = real_roomp(Room::STORAGE);
        if (corpse->roomp)
          --(*corpse);
        *rp2 += *corpse;
        vlogf(LOG_BUG, format("%s distributed to Storage Room (%d).") %  corpse->getName() % Room::STORAGE);
      sendTo(COLOR_BASIC, "<r>*** You had a CORPSE placed in the storage area. See a god to get it back. ***<z>\n\r");
      } else {
        if (corpse->roomp)
          --(*corpse);
        *rp2 += *corpse;
        vlogf(LOG_BUG, format("%s distributed to %s (%d).") %  corpse->getName() %
rp2->getName() % corpse->getRoomNum());
        sendTo(COLOR_BASIC, "<r>*** Your CORPSE has been restored to its place in the World ***.<z>\n\r");
      }
    }
    corpse->addCorpseToLists();
  }
}

void TPCorpse::saveCorpseToFile()
{
  char buf[256];
  TPCorpse *firstCorpse = NULL;
  TPCorpse *tmpCorpse = NULL;
  int numCorpses = 0;
  ItemSave is;

  if (fileName.empty()) {
    vlogf(LOG_BUG, format("Attempt to save a corpse with no fileName (%s)") %  getName());
    return;
  }

  memset(buf, '\0', sizeof(buf));

  sprintf(buf, "corpses/%s", fileName.c_str());

  if(!is.openFile(buf)){
    vlogf(LOG_FILE, format("Failed to open file '%s' in saveCorpseToFile() call.") % buf);
  }

  firstCorpse = this;
  while (firstCorpse) {
    if (firstCorpse->previousCorpse) {
      firstCorpse = firstCorpse->previousCorpse;
    } else {
      break;
    }
  }
  tmpCorpse = firstCorpse;
  while (tmpCorpse) {
    numCorpses += 1;
    tmpCorpse = tmpCorpse->nextCorpse;
  }


  strcpy(is.st.owner, fileName.c_str());
  is.st.number = numCorpses;
  is.st.first_update = is.st.last_update = (long) time(0);

  if(!is.writeHeader()){
    vlogf(LOG_BUG, format("Error writing corpse header for %s.") %  getName());
    return;
  }

  tmpCorpse = firstCorpse;
  is.objToStore(NORMAL_SLOT, (TObj *) tmpCorpse, NULL, FALSE, TRUE);
  is.writeFooter();

}


// msgStatus = 0: no log, 1: "saving", 2: "renting"
// d = true: prepare player for deletion
int TPerson::saveRent(bool d /*=false*/, int msgStatus /*=0*/)
{
  char buf[256];
  TPerson *tmp;
  TObj *obj;
  ItemSave is;

  if (desc && desc->original)
    tmp = desc->original;
  else
    tmp = dynamic_cast<TPerson *>(this);

  sprintf(buf, "rent/%c/%s", LOWER(tmp->name[0]), sstring(tmp->name).lower().c_str());
  if(!is.openFile(buf)){
    vlogf(LOG_BUG, format("Error opening file for saving %s's objects") %
	  getName());
    return FALSE;
  }
  strcpy(is.st.owner, getName().c_str());
  is.st.first_update = is.st.last_update = (long) time(0);
  is.st.number = 0;

  if(!is.writeHeader()){
    vlogf(LOG_BUG, format("Error writing rent header for %s.") %  getName());
    return FALSE;
  }

  for (wearSlotT ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
    obj = dynamic_cast<TObj *>(equipment[ij]);
    if (!obj)
      continue;
    if (d) {
      unequip(ij);
      is.objToStore(mapSlotToFile(ij), obj, this, d);
    } else {
      // if they're wearing a paired item, don't save the other slot 
      if (!(((ij == WEAR_LEG_L) && obj->isPaired()) ||
          ((ij == WEAR_EX_LEG_L) && obj->isPaired()) ||
          ((ij == HOLD_LEFT) && obj->isPaired()))) {
        is.objToStore(mapSlotToFile(ij), obj, this, d);
      }
    }
  }
  is.objsToStore(NORMAL_SLOT, stuff, this, d);
  is.writeFooter();

  if (msgStatus == 1 && desc) {
    vlogf(LOG_PIO, format("Saving %s [%d talens/%d bank/%.2f xps/%d items/%d age-mod]") %
        getName() % getMoney() % getBank() % getExp() % is.st.number % age_mod);
  } else if (msgStatus == 2 && desc) {
    vlogf(LOG_PIO, format("Renting %s [%d talens/%d bank/%.2f xps/%d items/%d age-mod]") %
        getName() % getMoney() % getBank() % getExp() % is.st.number % age_mod);
  }

  if (!is.st.number) 
    wipeRentFile(getName().c_str());

  if (d) {
    short save_room = in_room;
    saveChar(save_room);
    in_room = save_room;
    preKillCheck(TRUE);
    return DELETE_VICT;
  }

  return TRUE;
}


// Somewhere in here, we need to call race->makeBody().
void TPerson::loadRent()
{
  int num_read = 0;
  char buf[256];
  TPerson *tmp;
  sstring lbuf;
  ItemLoad il;
  int actual = 0;

  if (desc && desc->original)
    tmp = desc->original;
  else
    tmp = dynamic_cast<TPerson *>(this);

  // a kludge
  // we get called by the char gen routine AFTER we gave char newbie gear
  // and saved.  We would wind up double loading, so avoid this by:
  if (time(0) - tmp->player.time->birth <= 3)
    return;

  sprintf(buf, "rent/%c/%s", LOWER(tmp->name[0]), sstring(tmp->name).lower().c_str());

  if(!il.openFile(buf)){
    if (should_be_logged(this)) {
      vlogf(LOG_PIO, format("%s has no equipment.") %  getName());

      actual = meanPracsSoFar();

      vlogf(LOG_PIO, format("Loading %s [%d talens/%d bank/%.2f xps/no items/%d age-mod/no rent/%d extra pracs (%d-%d)]") %  
         getName() % getMoney() % getBank() % getExp() % age_mod %
         (actual-expectedPracs()) % actual % expectedPracs());
    }
    return;
  }
  if(!il.readHeader()){
    vlogf(LOG_BUG, format("Error while reading %s's rent file header.") %  getName());
    return;
  }
  if (il.objsFromStore(NULL, &num_read, this, NULL, false)) {
    vlogf(LOG_BUG, format("Error while reading %s's objects. Prepare for reimb!") % getName());
    return;
  }
  if (name != il.st.owner)
    vlogf(LOG_BUG, format("  %s just got %s's objects!") %
	  getName() % il.st.owner);

  if (in_room == Room::NOWHERE) {
    vlogf(LOG_PIO, "Char reconnecting after autorent");
    vlogf(LOG_PIO, format("%s was autorented for %d secs") % getName() %
        (time(0) - il.st.first_update));
  } else {
    // char was rented
    applyRentBenefits(time(0) - il.st.first_update);
  }

  il.st.first_update = il.st.last_update = time(0);

  actual = meanPracsSoFar();
    
  vlogf(LOG_PIO, format("Loading %s [%d talens/%d bank/%.2f xps/%d items/%d age-mod/%d extra pracs (%d-%d)]") %  
       getName() % getMoney() % getBank() % getExp() % il.st.number % 
       age_mod % (actual - expectedPracs()) % actual % expectedPracs());

  // silly kludge
  // because of the way the "stuff" list is saved, it essentially reverses
  // its order every reload
  // let's flip the order back...
  stuff.reverse();

  saveRent();
}


bool check_stuff_norent(TObj *obj, TBeing *owner, TBeing *recep)
{
  if (!obj)
    return false;
  if (obj->isRentable() && obj->isMonogramOwner(owner, true))
    return false;
  act("$n tells you, \"Sorry!  I refuse to store $p.\"",
        false, recep, obj, owner, TO_VICT, ANSI_ORANGE);
  return true;
}

bool check_stuff_norent(StuffList list, TBeing *owner, TBeing *recep)
{
  bool found = false;
  for (StuffIter it=list.begin();it!=list.end();++it)
    if (check_stuff_norent(dynamic_cast<TObj *>(*it), owner, recep))
      found = true;
  return found;
}

bool check_stuff_norent(equipmentData &equipment, TBeing *owner,
                        TBeing *recep)
{
  bool found = false;
  for (int i = MIN_WEAR; i < MAX_WEAR; i++) {
    TObj *obj = dynamic_cast<TObj *>(equipment[i]);
    if (!obj)
      continue;
    if (((i == WEAR_LEG_L) && obj->isPaired()) ||
          ((i == WEAR_EX_LEG_L) && obj->isPaired()) ||
          ((i == HOLD_LEFT) && obj->isPaired()))
      continue;
    if (check_stuff_norent(obj, owner, recep))
      found = true;
  }
  return found;
}

bool check_stuff_norent(TBeing *holder, TBeing *owner, TBeing *recep)
{
  bool found = false;

  if (check_stuff_norent(holder->stuff, owner, recep) ||
      check_stuff_norent(holder->equipment, owner, recep))
    found = true;

  for (followData *f = holder->followers; f; f = f->next) {
    TMonster *mon = dynamic_cast<TMonster *>(f->follower);
    if (!(mon && mon->isSaveMob(owner) && mon->sameRoom(*owner)))
      continue;
    if (check_stuff_norent(f->follower, owner, recep))
      found = true;
  }

  return found;
}

bool check_stuff_norent(TBeing *owner, TBeing *recep)
{
  return check_stuff_norent(owner, owner, recep);
}

// The player's level was previously used as the base for rent tax, but this
// harshly penalizes low-level players since a L10's income is SO much less
// than 1/5th of a L50's. Instead, use level^3 as it has a most pleasing
// curve: 0.12 @ l25, 0.5 @ l40 vs 1.0 @ l50, and scale to 2000 tax @ L50.
void charge_rent_tax(TBeing *ch, TMonster *recep, int shop_nr)
{
  const float rate = shop_index[shop_nr].getProfitBuy(NULL, ch);
  // 100.0 is because rosemary's profit_buy is currently 20
  const float base = pow(ch->GetMaxLevel(), 3) * 100.0 / pow(MAX_MORT, 3);

  // empty those pockets, but don't overdraw
  int tax = min((int)(base * rate), ch->getMoney());

  if (!tax)
    return;

  sstring msg = shop_index[shop_nr].message_buy;
  recep->doTell(ch->getName(), format(msg) % tax);

  TShopOwned tso(shop_nr, recep, ch);
  tso.doBuyTransaction(tax, "rent tax", TX_BUYING_SERVICE);
  vlogf(LOG_PIO, format("%s being charged %d talens rent tax by %s")
        % ch->getName() % tax % recep->getName());
}

int receptionist(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *recep, TObj *o)
{
  dirTypeT dir;
  roomDirData *exitp;
  TDatabase db(DB_SNEEZY);
  TCorporation corp(21);

  int shop_nr = find_shop_nr(recep->number);

  if (cmd == CMD_WHISPER)
    return shopWhisper(ch, recep, shop_nr, arg);

  if (cmd == CMD_GENERIC_PULSE) {
    TThing *t=NULL;
    TBeing *tbt;

    // Toss out idlers
    if(recep->spec==SPEC_RECEPTIONIST){
      // we check the proc, because we have a butler proc for player homes
      // obviously we don't want to toss out people in their homes
      for(StuffIter it=recep->roomp->stuff.begin();it!=recep->roomp->stuff.end() && (t=*it);++it) {
	if ((tbt = dynamic_cast<TBeing *>(t)) &&
	    tbt->getTimer() > 1 && 
	    !tbt->isImmortal()) {
	  if ((tbt->master) && tbt->master->inRoom() == tbt->inRoom()) {
	    continue;
	  }
	  recep->doSay("Hey, no loitering!  Make room for the other customers.");
	  for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
	    if (exit_ok(exitp = recep->exitDir(dir), NULL)) {
	      act("$n throws you from the inn.",
		  FALSE, recep, 0, tbt, TO_VICT);
	      act("$n throws $N from the inn.",
		  FALSE, recep, 0, tbt, TO_NOTVICT);
	      recep->throwChar(tbt, dir, FALSE, SILENT_NO, true);
	      return TRUE;
	    }
	  }
	}
      }
    }
    return TRUE;
  } else if (cmd == CMD_MOB_VIOLENCE_PEACEFUL) {
    TThing *ttt = o;
    TBeing *tbt = dynamic_cast<TBeing *>(ttt);

    recep->doSay("Hey!  Take it outside.");
    for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
      if (exit_ok(exitp = recep->exitDir(dir), NULL)) {
        act("$n throws you from the inn.",
               FALSE, recep, 0, ch, TO_VICT);
        act("$n throws $N from the inn.",
               FALSE, recep, 0, ch, TO_NOTVICT);
        recep->throwChar(ch, dir, FALSE, SILENT_NO, true);
        act("$n throws you from the inn.",
               FALSE, recep, 0, tbt, TO_VICT);
        act("$n throws $N from the inn.",
               FALSE, recep, 0, tbt, TO_NOTVICT);
        recep->throwChar(tbt, dir, FALSE, SILENT_NO, true);
        return TRUE;
      }
    }
    return TRUE;
  }

  if (cmd != CMD_RENT)
    return FALSE;

  // force poly's to return
  if (dynamic_cast<TMonster *>(ch)) {
    act("$e looks at you and says, 'Sleep in the street!'", FALSE, recep, 0, ch, TO_VICT);
    act("$e looks at $N and says, 'Sleep in the street!'", FALSE, recep, 0, ch, TO_NOTVICT);
    return TRUE;
  }

  if (!recep->awake()) {
    act("$e isn't able to talk to you...", FALSE, recep, 0, ch, TO_VICT);
    return TRUE;
  }

  if (!recep->canSee(ch) && !ch->isImmortal()) {
    act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
    return TRUE;
  }

  if(ch->affectedBySpell(AFFECT_PLAYERKILL) && !ch->isImmortal()){
    act("$n looks at you and says, 'Murderers are not allowed to stay here!'", FALSE, recep, 0, ch, TO_VICT);
    act("$n looks at $N and says, 'Murderers are not allowed to stay here!'", FALSE, recep, 0, ch, TO_NOTVICT);
    return TRUE;
  }

  if (ch->affectedBySpell(AFFECT_PLAYERLOOT) && !ch->isImmortal()) {
    act("$n motions at you then whispers, 'Someone is after you for the moment and I cannot allow you to stay here...Sorry.'", FALSE, recep, NULL, ch, TO_VICT);
    return TRUE;
  }

  bool   autoHates  = false,
         hatesMe[2] = {false, false};
  sstring tStString("");

  for (int tCounter = 0; SIKHates[tCounter].tRace != RACE_NORACE; tCounter++) {
    if (SIKHates[tCounter].tVNum != recep->mobVnum())
      continue;

    if (SIKHates[tCounter].isHate) {
      tStString = SIKHates[tCounter].tStMessage;

      if (SIKHates[tCounter].tRace == ch->getRace()) {
        tStString = SIKHates[tCounter].tStMessage;
        hatesMe[0] = true;
        hatesMe[1] = true;
      }
    } else {
      autoHates = true;

      if (SIKHates[tCounter].tRace == ch->getRace()) {
        hatesMe[0] = true;
        hatesMe[1] = false;
      }
    }
  }

  if ((hatesMe[0] ? hatesMe[1] : autoHates)) {
    recep->doAction(fname(ch->name), CMD_GROWL);

    if (!tStString.empty())
      recep->doTell(ch->getNameNOC(ch), tStString);

    for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
      if (exit_ok(exitp = recep->exitDir(dir), NULL)) {
        act("$n throws you from the inn.",
            FALSE, recep, 0, ch, TO_VICT);
        act("$n throws $N from the inn.",
            FALSE, recep, 0, ch, TO_NOTVICT);
        recep->throwChar(ch, dir, FALSE, SILENT_NO, true);

        return TRUE;
      }
    }

    return TRUE;
  }

  if (!ch->desc)
    return TRUE;

  if (check_stuff_norent(ch, recep))
    return TRUE;

  if (ch->desc->m_bIsClient)
    ch->desc->clientf(format("%d") % CLIENT_RENT);

  if (ch->isImmortal()) {
    ch->sendTo(COLOR_BASIC, "<r>WARNING<z>\n\r----------\n\r");
    ch->sendTo("Renting will almost certainly destroy your wizfile.  If you are used to\n\r");
    ch->sendTo("doing this because of mortal life then it's best to get un-used to it.\n\r");
    ch->sendTo("If you Have to rent out, such as testing, then go mortal first.\n\r");
    ch->sendTo("----------\n\r");
  }

  if (Config::RentTax() && ch->GetMaxLevel() > 5)
    charge_rent_tax(ch, recep, shop_nr);

  act("$n tells you, 'Have a nice stay!'", FALSE, recep, 0, ch, TO_VICT);
  act("$n stores your stuff in the safe, and shows you to your room.",
      FALSE, recep, 0, ch, TO_VICT);
  act("$n shows $N to $S room.", FALSE, recep, 0, ch, TO_NOTVICT);

  if (ch->desc->m_bIsClient) {
    ch->desc->clientf(format("%d") % CLIENT_RENT_END);
    return TRUE;
  }

  ch->cls();
  ch->desc->outputProcessing();

  return dynamic_cast<TPerson *>(ch)->saveRent(TRUE, 2);
}

bool noteLimitedItems(FILE * fp, const char *tag, unsigned char version, bool immortal)
{
  rentObject item;
  int depth = 0;
  char c, *n, *s, *d, *ad;
  signed char slot;

  while (1) {
    if (fread(&slot, sizeof(signed char), 1, fp) != 1) {
      vlogf(LOG_BUG, "noteLimitedItem: Failed reading slot");
      return FALSE;
    }
    if (slot == CONTENTS_END) {
      if (depth--)
        continue;
      else
        break;
    } else
      depth++;

    if (version <= 1) {
      vlogf(LOG_BUG, "old rent object read in note limited item");
      return FALSE;
    } else {
      if (fread(&item, sizeof(rentObject), 1, fp) != 1) {
        vlogf(LOG_BUG, "noteLimitedItem: Failed reading rentObject");
        return FALSE;
      }
    }
    n = s = d = ad = NULL;

    if (IS_SET(item.extra_flags, ITEM_STRUNG)) {
      // we only care about the action_description
      if (!(n = raw_read_sstring(fp))) {
        vlogf(LOG_BUG, "Serious flaw (1) in noteLimitedItem");
        return FALSE;
      }
      delete [] n;
      if (!(s = raw_read_sstring(fp))) {
        vlogf(LOG_BUG, "Serious flaw (2) in noteLimitedItem");
        return FALSE;
      }
      if (!(d = raw_read_sstring(fp))) {
        vlogf(LOG_BUG, "Serious flaw (3) in noteLimitedItem");
        delete [] s;
        return FALSE;
      }
      delete [] d;
      ad = raw_read_sstring(fp);
    }
    int robj = real_object(item.item_number);
    if (item.item_number >= 0) {
      if (robj < 0) {
        vlogf(LOG_BUG, format("BOGUS ITEM #%d found in %s's rent file!") %  
           item.item_number % tag);
        delete [] ad;
        delete [] s;
        continue;
      }

      obj_index[robj].addToNumber(1);
    }
    delete [] ad;
    delete [] s;
  }
  // make sure we can't possibly read anymore... i.e., at eof 
  if (fread(&c, 1, 1, fp)) {
    vlogf(LOG_BUG, "notelimitedItem: unexpected excess.");
    return FALSE;
  }

  return TRUE;
}

void printLimitedInRent(void)
{
  unsigned int i;
  for (i = 0; i < obj_index.size(); i++) {
    if (obj_index[i].getNumber() > 0) {
      vlogf(LOG_MISC, format("  %d - [%d] : max [%d]") % 
           obj_index[i].virt % obj_index[i].getNumber() % obj_index[i].max_exist);
      if (obj_index[i].getNumber() > obj_index[i].max_exist &&
          obj_index[i].max_exist) {
        // latter condition is because Obj::DEITY_TOKEN max exist = 0
        char buf[1024];
        sprintf(buf, "Item (%s:%d) is over max (%d).  Num: (%d).\n\r", 
            obj_index[i].name, obj_index[i].virt,
            obj_index[i].max_exist, obj_index[i].getNumber());
	// these have to be lower case
        // autoMail(NULL, "jesus", buf);
        // autoMail(NULL, "damescena", buf);

      }
    }
  }
}

void countAccounts(const char *arg) 
{
  DIR *dfd;
  struct dirent *dp;
  int count = 0;

  sstring account_path = ((sstring)(format("account/%c/%s") % arg[0] % arg)).lower();
  if (!(dfd = opendir(account_path.c_str()))) {
    vlogf(LOG_BUG, format("bad path in countAccount (%s) from arg (%s)") % account_path % arg);
    return;
  }

  bool accountActive7 = false;
  bool accountActive30 = false;

  while ((dp = readdir(dfd)) != NULL) {
    if (!strcmp(dp->d_name, "account") || !strcmp(dp->d_name, "comment") || 
        !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
      continue;
    
    // check for valid char
    sstring player_path = format("player/%c/%s") % dp->d_name[0] % dp->d_name;

    struct stat theStat;
    int ret = stat(player_path.c_str(), &theStat);
    if (ret != 0) {
      // some error occurred
      if (errno == ENOENT) {
        sstring ref_path = format("%s/%s") % account_path % dp->d_name;
        vlogf(LOG_MISC, format("Deleting reference %s to nonexistent %s in %s's account") % ref_path % player_path % arg);
        if (unlink(ref_path.c_str()) != 0)
          vlogf(LOG_FILE, format("error in unlink (12) (%s) %d") % ref_path % errno);
      } else {
        vlogf(LOG_FILE, format("ERROR: stat() failed for %s.  errno = %d") % player_path % errno);
        perror("stat");
      }
      continue;
    }
    // theStat now holds info about the pfile
    // since it is valid, increment the counter
    count++;

    if (!accountActive7) {
      if (time(0) - theStat.st_mtime <= (7 * SECS_PER_REAL_DAY)) {
        accountActive7 = true;
        AccountStats::active_account7++;
      }
    }
    if (!accountActive30) {
      if (time(0) - theStat.st_mtime <= (30 * SECS_PER_REAL_DAY)) {
        accountActive30 = true;
        AccountStats::active_account30++;
      }
    }
  }
  closedir(dfd);

  if (!count) {
    // delete this empty account 

    vlogf(LOG_MISC, format("Empty Account: %s, deleting it.") % account_path);
    
    TDatabase db(DB_SNEEZY);
    db.query("delete from account where name=lower('%s')", arg);

    // ignore unlink errors
    unlink((account_path + "/comment").c_str());
    unlink((account_path + "/acount").c_str());

    if (rmdir(account_path.c_str()) != 0)
      vlogf(LOG_FILE, format("error in rmdir (%s) %d") % account_path % errno);

    return;
  }
  // each time this is called, we have another account
  // no need to really do anything, besides just count number of calls
  AccountStats::account_number++;
}


static bool parseFollowerRentEntry(FILE *fp, TBeing *ch, const char *arg, int num)
{
  TMonster *mob;
  int tmp = 0;
  int tmp2 = 0;
  int i = 0;
  statTypeT iStat;
  float att;
  unsigned char version;
  TObj *new_obj;
  int rc;
  ItemLoad il;

  bool fp2_open = false;
  FILE *fp2 = NULL;

  try {
    if (!(mob = read_mobile(num, VIRTUAL))) {
      vlogf(LOG_BUG, format("Error loading mob %d in loadFollower") %  num);
      return false;
    }
    // Since this mob was in rent, don't double count it.
    mob_index[mob->getMobIndex()].addToNumber(-1);

    if (fscanf(fp, "%d ", &tmp) != 1)
      throw std::runtime_error("1");
    mob->specials.act = tmp;

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("2");
    mob->specials.affectedBy = tmp;
  
    // technically, we should check for AFF_SANCT here
    // if it had natural sanct, it got set in readMobile
    // if it had been cast, it should be added by the affections loop below

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("3");
    mud_assert(tmp >= MIN_FACTION && tmp < MAX_FACTIONS, "bad value");
    mob->setFaction(factionTypeT(tmp));

    if (fscanf(fp, " %f ", &att) != 1)
      throw std::runtime_error("4");
    mob->setPerc((double) att);

    if (fscanf(fp, " %f ", &att) != 1)
      throw std::runtime_error("5");
    mob->setMult((double) att);
  
    if(fscanf(fp, "\n")==EOF)
      vlogf(LOG_FILE, "Unexpected read error on follower data");
  
    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("6");
    mob->setClass(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("7");
    mob->fixLevels(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("8");
    mob->setHitroll(tmp);
  
    if (fscanf(fp, " %f ", &att) != 1)
      throw std::runtime_error("9");

    // this is for old-mob handling
    float old_ac_lev = mob->getACLevel();
    mob->setACLevel(att);
    mob->setACFromACLevel();

    // we will let HP Level be whatever the tiny mob is, and just set
    // the actual and max here
    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("10");
    mob->setHit(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("11");
    mob->setMaxHit(tmp);
  
    rc = fscanf(fp, " %f+%d \n", &att, &tmp);
    if (rc == 1) {
      vlogf(LOG_BUG, format("Old style mob found in %s's rent") %  
	    (ch ? ch->getName() : "Unknown"));
      // first, correct the file pointer so reading works ok
      rc = fscanf(fp, "d%d+%d", &tmp, &tmp2);
      if (rc != 2) {
        vlogf(LOG_BUG, "Unable to fix old-style mob in rent.");
      }
      // leave the damage what it is on the mob now
      // HP should be ok since we are saving raw values, not levels
      // fix AC though
      mob->setACLevel(old_ac_lev);
      mob->setACFromACLevel();
    } else if (rc == 2) {
      mob->setDamLevel(att);
      mob->setDamPrecision(tmp);
    } else {
      throw std::runtime_error("12");
    }

    mob->setLifeforce(9000);
    mob->setMana(mob->manaLimit());
    mob->setPiety(mob->pietyLimit());
    mob->setMove(mob->moveLimit());

    //    mob->setMaxMana(10);
    //    mob->setMaxMove(50);
  
    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("13");
    mob->setMoney(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("14");
    mob->setExp(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("14");
    mob->setMaxExp(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("15");
    mob->setRace(race_t(tmp));

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("16");
    mob->setWeight(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("17");
    mob->setHeight(tmp);

    for (iStat=MIN_STAT;iStat<MAX_STATS_USED;iStat++) {
      if(fscanf(fp, " %d ", &tmp)==EOF)
	vlogf(LOG_FILE, "Unexpected read error in follower data");
      mob->setStat(STAT_CHOSEN, iStat, tmp);
    }

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("24");
    mob->setPosition(mapFileToPos(tmp));

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("25");
    mob->default_pos = mapFileToPos(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("26");
    mob->setSexUnsafe(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("27");
    mob->spec = tmp;

    immuneTypeT ij;
    for (ij=MIN_IMMUNE; ij < MAX_IMMUNES; ij++) {
      if (fscanf(fp, " %d ", &tmp) != 1)
        throw std::runtime_error("28");
      mob->setImmunity(ij, tmp);
    }

    if (fscanf(fp, " %d ", &tmp) == 1)
      mob->setMaterial(tmp);
    else
      mob->setMaterial(MAT_UNDEFINED);
  
    if (fscanf(fp, " %d ", &tmp) == 1)
      mob->canBeSeen = tmp;
    else
      mob->canBeSeen = 0;

    if (fscanf(fp, " %d ", &tmp) == 1)
      mob->visionBonus = tmp;
    else
      mob->visionBonus = 0;

    affectedData af;
    for (i = 0; i < MAX_AFFECT; i++) {
      if (fscanf(fp, " %d ", &tmp) != 1)
        throw std::runtime_error("29");
      af.type = mapFileToSpellnum(tmp);

      if (fscanf(fp, " %d ", &tmp) != 1)
        throw std::runtime_error("30");
      af.level = tmp;

      if (fscanf(fp, " %d ", &tmp) != 1)
        throw std::runtime_error("31");
      af.duration = tmp;

      if (fscanf(fp, " %d ", &tmp) != 1)
        throw std::runtime_error("32");
      // we can't set this just yet, need to know the location stuff
      int raw_modifier = tmp;

      if (fscanf(fp, " %d ", &tmp) != 1)
        throw std::runtime_error("33");
      af.modifier2 = tmp;

      if (fscanf(fp, " %d ", &tmp) != 1)
        throw std::runtime_error("34");
      af.location = mapFileToApply(tmp);
      if (applyTypeShouldBeSpellnum(af.location))
        af.modifier = mapFileToSpellnum(raw_modifier);
      else
        af.modifier = raw_modifier;

      if (fscanf(fp, " %d ", &tmp) != 1)
        throw std::runtime_error("35");
      af.bitvector = tmp;

      // end of data
      if (af.type == -1)
        break;

      mob->affectTo(&af);
    }

    // fix limbs
    for (i = MIN_WEAR; i < MAX_WEAR; i++) {      // Initializing 
      // we are reading the ith element of the file
      // find the wear slot that corresponds to i in the file
      wearSlotT mapped_slot = mapFileToSlot(i);
      if (fscanf(fp, " %d ", &tmp) != 1)
        throw std::runtime_error("36");
      mob->setLimbFlags(mapped_slot, tmp);

      if (fscanf(fp, " %d ", &tmp) != 1)
        throw std::runtime_error("37");
      mob->setCurLimbHealth(mapped_slot, tmp);

      if (fscanf(fp, " %d ", &tmp) != 1)
        throw std::runtime_error("37b");
      version = tmp;
      if (tmp != -1 && fp2_open == false) {
        char buf[256];
        sprintf(buf, "rent/%c/%s.fr", LOWER(arg[0]), sstring(arg).lower().c_str());
        if (!(fp2 = fopen(buf, "r+b"))) 
          break;
        fp2_open = true;
      }
      il.setFile(fp2); // ItemLoad destructor will fclose this
      il.setVersion(version);
      if (tmp != -1 && (new_obj = il.raw_read_item())) {
        if (ch) {
          vlogf(LOG_SILENT, format("%s's %s rent-retrieving: (%s : %d)") %  arg % mob->getName() % new_obj->getName() % new_obj->objVnum());
          mob->equipChar(new_obj, mapped_slot, SILENT_YES);
        } else {
          // count the item
          // we want to add 1 to count the item, and another 1 because
          // the delete will reduce the number, add an additional one
          if ((new_obj->number >= 0)) {
            obj_index[new_obj->getItemIndex()].addToNumber(2);
	    
            vlogf(LOG_PIO, format("     [%d] - in %s's follower rent") %  
                     new_obj->objVnum() % arg);
          }
          delete new_obj;
        }
      }
      mob->setStuckIn(mapped_slot, NULL);
    }

    if (fscanf(fp, " %d ", &tmp) != 1)
      throw std::runtime_error("37c");

    while (tmp != -1) {
      version = tmp;
      if (fp2_open == false) {
        char buf[256];
        sprintf(buf, "rent/%c/%s.fr", LOWER(arg[0]), sstring(arg).lower().c_str());
        if (!(fp2 = fopen(buf, "r+b"))) 
          break;
        fp2_open = true;
      }
      il.setFile(fp2); // the ItemLoad destructor will close this
      il.setVersion(version);
      if ((new_obj = il.raw_read_item())) {
        if (ch) {
          vlogf(LOG_SILENT, format("%s's %s rent-retrieving: (%s : %d)") %  arg % mob->getName() % new_obj->getName() % new_obj->objVnum());
          *mob += *new_obj;
        } else {
          // count the item
          // we want to add 1 to count the item, and another 1 because
          // the delete will reduce the number, add an additional one
          if ((new_obj->number >= 0)) {
            obj_index[new_obj->getItemIndex()].addToNumber(2);
            vlogf(LOG_PIO, format("     [%d] - in %s's follower rent") %  
                     new_obj->objVnum() % arg);
          }
          delete new_obj;
        }
      }
      if (fscanf(fp, " %d ", &tmp) != 1)
        throw std::runtime_error("37d");
    }

    // configure sstrings if necessary
    if (IS_SET(mob->specials.act, ACT_STRINGS_CHANGED)) {
      mob->name = fread_string(fp);
      mob->shortDescr = fread_string(fp);
      mob->player.longDescr = fread_string(fp);
      mob->setDescr(fread_string(fp));
      mob->ex_description = NULL;
    }

    if (ch) {
      //we are actually retrieving
      *ch->roomp += *mob;
  
      act("You retrieve $N from $S storage area.", 
               FALSE, ch, 0, mob, TO_CHAR);
      act("$n retrieves $N from $S storage area.", 
               FALSE, ch, 0, mob, TO_NOTVICT);
      vlogf(LOG_SILENT, format("%s mobile-rent retrieving %s") %  ch->getName() % mob->getName());

      // if they rented while mounted, bring horse back in, but don't force
      // the follow (since they aren't mounted anymore)
      if (mob->isAffected(AFF_CHARM)) {
        ch->addFollower(mob);
      }

      // fixup any AFFECT_PET affects to have the proper memory
      // this affect modifies "TThing * be" to be a char * storing the name
      // of the master.  We didn't save this info, so we simply recreate it
      // here
      affectedData *aff;
      for (aff = mob->affected; aff; aff = aff->next) {
        if ((aff->type == AFFECT_PET) || 
            (aff->type == AFFECT_CHARM) ||
            (aff->type == AFFECT_THRALL) ||
            (aff->type == AFFECT_ORPHAN_PET) ||
            (aff->type == AFFECT_COMBAT && aff->modifier == COMBAT_RESTRICT_XP)) {
          char * tmp = mud_str_dup(ch->name);
          aff->be = reinterpret_cast<TThing *>(tmp);
        }
      }

    } else {
      // we are just logging
      // handle the count for this mob.
      // we are adding 1 to count the mob, and another one to offset the
      // decrease that will happen in delete.
      mob_index[mob->getMobIndex()].addToNumber(2);

      vlogf(LOG_PIO, format("     [%d] - mobile (%s) owned by %s") % 
                     mob->mobVnum() % mob->getName() % arg);
      thing_to_room(mob, Room::VOID);
      delete mob;
    }
    return true;
  } catch (std::runtime_error &e) {
    vlogf(LOG_PIO, format("Error reading follower data (%s mobs %i) (%s)")
        % arg % num % e.what());
    if (mob) {
      // we read the act bits early in the read, but don't alter the sstrings
      // until the end of the read, so this is a good idea
      REMOVE_BIT(mob->specials.act, ACT_STRINGS_CHANGED);
      delete mob;
    }
    return false;
  }
}

static void parseFollowerRent(FILE *fp, TBeing *ch, const char *arg)
{
  int num;
  while (fscanf(fp, "#%d\n", &num) == 1)
    if (!parseFollowerRentEntry(fp, ch, arg, num))
      break;
}


void updateRentFile(const char *who)
{
  char fileName[128];
  rentHeader h;
  FILE *fp;
  charFile pd;
  TAccount account;
  bool immortal;

  mud_assert(who != NULL, "updateRentFile called with NULL player name!");

  sprintf(fileName, "rent/%c/%s", who[0], who);

  // skip followers data
  if (strlen(fileName) > 4 && !strcmp(&fileName[strlen(fileName) - 4], ".fol"))
    return;
  if (strlen(fileName) > 3 && !strcmp(&fileName[strlen(fileName) - 3], ".fr"))
    return;

  if (!(fp = fopen(fileName, "r+b"))) {
    vlogf(LOG_BUG, format("Error opening %s's rent file!") %  who);
    return;
  }
  if ((fread(&h, sizeof(rentHeader), 1, fp)) != 1) {
    vlogf(LOG_BUG, format("  Cannot read rent file header for %s") %  who);
    fclose(fp);
    return;
  }
  if (sstring(h.owner).lower().compare(who)) {
    vlogf(LOG_BUG, format("WARNING!  rent file %s holds objects for %s!") %  who % h.owner);
    fclose(fp);
    return;
  }
  if (!load_char(who, &pd)) {
    vlogf(LOG_BUG, format("Unable to read player file for %s, so deleting rent file.") %  h.owner);
    fclose(fp);
    wipeRentFile(who);
    //removeFollowers();
    return;
  }

  if(!account.read(pd.aname)){
    vlogf(LOG_BUG, format("  Cannot read account file for %s") %  who);
    fclose(fp);
    return;
  }
  immortal = IS_SET(account.flags, TAccount::IMMORTAL);

  if (pd.load_room == Room::AUTO_RENT) {        // this person was autorented 
    pd.load_room = Room::NOWHERE;
    h.last_update = time(0);
    if (!noteLimitedItems(fp, who, h.version, immortal)) {
      vlogf(LOG_BUG, format("cannot count (1) limited items for %s") %  h.owner);
      fclose(fp);
      return;
    }
    rewind(fp);
    if (fwrite(&h, sizeof(rentHeader), 1, fp) != 1) {
      vlogf(LOG_BUG, format("Cannot write updated rent file header for %s") %  h.owner);
      fclose(fp);
      return;
    }
    fclose(fp);
    if (!raw_save_char(who, &pd)) {
      vlogf(LOG_BUG, format("Error updating player-file entry for %s in updateRentFile.") %  h.owner);
      return;
    }
    vlogf(LOG_PIO, format("   De-autorented %s") %  h.owner);
  } else {   // this person was rented as normal 

    if (!noteLimitedItems(fp, who, h.version, immortal)) {
      vlogf(LOG_BUG, format("cannot count (2) limited items for %s") %  h.owner);
      fclose(fp);
      return;
    }
    fclose(fp);
  }

  sprintf(fileName, "rent/%c/%s.fol", who[0], who);
  if ((fp = fopen(fileName, "r"))) {
    parseFollowerRent(fp, NULL, who);
    fclose(fp);
  }
}

void updateRentFiles(void)
{
  bootPulse(".", false);
  dirwalk("rent/a", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/b", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/c", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/d", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/e", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/f", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/g", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/h", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/i", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/j", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/k", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/l", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/m", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/n", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/o", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/p", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/q", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/r", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/s", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/t", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/u", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/v", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/w", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/x", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/y", updateRentFile);
  bootPulse(".", false);
  dirwalk("rent/z", updateRentFile);
  bootPulse(NULL, true);
}

void TBeing::removeFollowers()
{
  TPerson *tmp;
 
  if (desc && desc->original)
    tmp = desc->original;
  else
    tmp = dynamic_cast<TPerson *>(this);

  // a silly check, but necessary
  // we get in here after checking isPc()
  // In the case of a polymorphed, but linkdead mob, isPc() will be true
  // but !desc will exist, so we need a handler for this event
  // This blocks bad things from happening lower down for this eventuality
  if (!tmp) 
    return;

  wipeFollowersFile(tmp->name.c_str());
}

bool TBeing::isSaveMob(const TBeing *) const
{
  const TMonster *mob;

  // don't save pc followers
  if (isPc())
    return FALSE;

  if (!(mob = dynamic_cast<const TMonster *>(this)))
    return FALSE;

  // don't save oed mobs
  if (mob->number < 0)
    return FALSE;

  // DUH.... making a bounty rent out is kinda dumb
  if (mob->spec == SPEC_BOUNTY_HUNTER)
    return FALSE;

  // no renting of hero faeries
  if (mob->spec == SPEC_HERO_FAERIE)
    return FALSE;

  return TRUE;
}

bool TBeing::saveFollowers(bool rent_time)
{
  TMonster *mob;
  TBeing *ch;
  TThing *t;
  followData *f, *f2;
  char buf[256];
  FILE *fp;
  int i;
  TPerson *tmp;
  bool found = FALSE;
  unsigned char version;
  TObj *obj;
  ItemSave is;
 
  if (desc && desc->original)
    tmp = desc->original;
  else
    tmp = dynamic_cast<TPerson *>(this);

  sprintf(buf, "rent/%c/%s.fol", LOWER(tmp->name[0]), sstring(tmp->name).lower().c_str());

  if (!followers) {
    wipeFollowersFile(tmp->name.c_str());
    return FALSE;
  }
  if (!(fp = fopen(buf, "w"))) {
    vlogf(LOG_FILE, format("Can't open follower file for %s!") %  tmp->name);
    return FALSE;
  }

  bool fp2_open = false;
  FILE *fp2 = NULL;
  for (f = followers; f; f = f2) { 
    f2 = f->next;
    ch = f->follower;

    if (!ch->isSaveMob(this))
      continue;

    if (!(mob = dynamic_cast<TMonster *>(ch)))
      continue;

    // don't save if not around
    if (!mob->sameRoom(*this))
      continue;

    // we need to strip the affects off while we save stats
    // otherwise a -10 AC struct would get doubled on rent out/rent in
    // make a copy of it for now
    affectedData *a_list = NULL;
    if (mob->affected)
      a_list = new affectedData(*mob->affected);
    // now strip the affects off, and we'll put them back on at the end...
    while (mob->affected) {
      mob->affectRemove(mob->affected, SILENT_YES);
    }
    // yep, same problem with equipment
    TThing *char_eq[MAX_WEAR];
    wearSlotT ij;
    for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
      if (mob->equipment[ij])
        char_eq[ij] = unequip_char_for_save(mob, ij);
      else
        char_eq[ij] = NULL;
    }

    found = TRUE;
    fprintf(fp, "#%d\n", mob->mobVnum());
    fprintf(fp, "%ld %" PRIu64 " %d %.1f %.1f\n",
            mob->specials.act,
            mob->specials.affectedBy,
            mob->getFaction(), (double) mob->getPerc(),
            mob->getMult());
    fprintf(fp, "%d %d %d %.1f %d %d %.1f+%d\n",
            mob->getClass(),
            mob->GetMaxLevel(),
            mob->getHitroll(),
            mob->getACLevel(),
            mob->getHit(),
            mob->points.maxHit,
            mob->getDamLevel(),
            mob->getDamPrecision());
    fprintf(fp, "%d %d %d %d %d %d\n",
            mob->getMoney(), (int) mob->getExp(), (int) mob->getMaxExp(), mob->getRace(), (int) mob->getWeight(), mob->getHeight());

    for(statTypeT iStat=STAT_STR;iStat<MAX_STATS_USED;iStat++)
      fprintf(fp, "%d ", mob->chosenStats.get(iStat));
    fprintf(fp, "\n");

    fprintf(fp, "%d %d %d %d\n",
            mapPosToFile(mob->getPosition()),
            mapPosToFile(mob->default_pos),
        mob->getSex(), mob->spec);

    immuneTypeT itt;
    for (itt=MIN_IMMUNE; itt < 14; itt++) 
      fprintf(fp, "%d ", mob->getImmunity(itt));
    
    fprintf(fp, "\n");
    for (; itt < MAX_IMMUNES; itt++) 
      fprintf(fp, "%d ", mob->getImmunity(itt));
    
    fprintf(fp, "\n");

    fprintf(fp, "%d %d %d\n", mob->getMaterial(WEAR_BODY), mob->canBeSeen, mob->visionBonus);

    // store affects to preserve charm, etc
    affectedData *af;
    for (af = a_list, i = 0; i < MAX_AFFECT; i++) {
      if (af) {
        fprintf(fp, "%d %d %d %ld %ld %d %" PRIu64 "\n",
               mapSpellnumToFile(af->type),
               af->level,
               af->duration,
               applyTypeShouldBeSpellnum(af->location) ? mapSpellnumToFile(spellNumT(af->modifier)) : af->modifier,
               af->modifier2,
               mapApplyToFile(af->location),
               af->bitvector);
        af = af->next;
      } else {
        fprintf(fp, "-1 0 0 0 0 0 0\n");
        break;
      }
    }

    version = CURRENT_RENT_VERSION;

    // save limbs
    for (i = MIN_WEAR; i < MAX_WEAR; i++) {      // Initializing 
      // we are trying to write the ith element of the file
      // find the wear slot that corresponds to i in the file
      wearSlotT mapped_slot = mapFileToSlot( i);
      fprintf(fp, "%d %d", 
              mob->getLimbFlags(mapped_slot),
              mob->getCurLimbHealth(mapped_slot));
      TThing *t_obj = char_eq[mapped_slot];
      obj = dynamic_cast<TObj *>(t_obj);

      // some item duplication bugs exist due to the way pets are saved
      // pets save only when master saves
      // master is forced to save on certain events, but pet is not
      // so for example:
      // kill mob with item, order pet get item, save
      // pet file has item
      // order pet drop item, go linkdead
      // get another PC to pick up item
      // after autorenting, pet loads with item and other PC has item too
      // I put a forced doSave in order because of this, but I'm not
      // convinced there aren't other ways so might be best to not save
      // items except at rent_time
//      if (!obj || !rent_time)
      if (!obj)
        fprintf(fp, " -1\n");
      else {
        fprintf(fp, " %d\n", version);
        if (!fp2_open) {
          sprintf(buf, "rent/%c/%s.fr", LOWER(tmp->name[0]), sstring(tmp->name).lower().c_str());
          if (!(fp2 = fopen(buf, "w+b"))) {
            vlogf(LOG_BUG, format("Error opening %'s follower rent file for write.") %  tmp->name);
          } else 
            fp2_open = true;
        }
	
	is.setFile(fp2);
        if (fp2_open)
          is.raw_write_item(obj);

        if (rent_time) {
          vlogf(LOG_SILENT, format("%s's %s renting: (%s : %d)") %  getName() % mob->getName() % obj->getName() % obj->objVnum());


          delete obj;
          char_eq[mapped_slot] = NULL;
        } else {
          mob->equipChar(obj, mapped_slot, SILENT_YES);
        }
      }
    }
    for(StuffIter it=mob->stuff.begin();it!=mob->stuff.end();){
      t=*(it++);
      obj = dynamic_cast<TObj *>(t);
      if (!obj)
        continue;
      fprintf(fp, " %d\n", version);

      if (!fp2_open) {
        sprintf(buf, "rent/%c/%s.fr", LOWER(tmp->name[0]), sstring(tmp->name).lower().c_str());
        if (!(fp2 = fopen(buf, "w+b"))) {
          vlogf(LOG_BUG, format("Error opening %'s follower rent file for write.") %  tmp->name);
        } else
          fp2_open = true;
      }
        
      is.setFile(fp2);
      if (fp2_open)
        is.raw_write_item(obj);

      if (rent_time) {
        vlogf(LOG_SILENT, format("%s's %s renting: (%s : %d)") %  getName() % mob->getName() % obj->getName() % obj->objVnum());

        delete obj;
      }
    }
    fprintf(fp, " -1\n");   // stuff terminator

    // save strung mob sstrings
    if (IS_SET(mob->specials.act, ACT_STRINGS_CHANGED)) {
      fprintf(fp, "%s~\n", mob->name.replaceString("\r", "").c_str());
      fprintf(fp, "%s~\n", mob->shortDescr.replaceString("\r", "").c_str());
      fprintf(fp, "%s~\n", mob->getLongDesc().replaceString("\r", "").c_str());
      fprintf(fp, "%s~\n", mob->getDescr().replaceString("\r", "").c_str());
    }

    // throw the affects back onto the mob
    while (a_list) {
      // due to the way affectTo copies stuff, apply affects from end of list
      affectedData *aff, *prev = NULL;
      for (aff = a_list; aff->next; prev = aff, aff = aff->next);

      mob->affectTo(aff, aff->renew, SILENT_YES);

      delete aff;
      if (prev)
        prev->next = NULL;
      else
        a_list = NULL;
    }

    if (rent_time) {
      act("$n is led off to a storage area.", FALSE, mob, 0, 0, TO_ROOM);
      vlogf(LOG_SILENT, format("%s mobile-renting %s") %  getName() % mob->getName());

      // do this here so we don't show it in delete
      // also, we suppress the "realizes is a jerk" text this way
      mob->stopFollower(TRUE, STOP_FOLLOWER_SILENT);

      // Since mob is heading into rent, artificially raise number so the
      // number is kept up with properly
      mob_index[mob->getMobIndex()].addToNumber(1);

      delete mob;
      mob = NULL;
    }
  }
  fclose(fp);

  if (!found) {
    wipeFollowersFile(tmp->name.c_str());
    return FALSE;
  }

  return TRUE;
}

bool TBeing::loadFollowers()
{
  FILE *fp;
  char buf[256];
  TPerson *tmpPer;
 
  if (desc && desc->original)
    tmpPer = desc->original;
  else
    tmpPer = dynamic_cast<TPerson *>(this);

  sprintf(buf, "rent/%c/%s.fol", LOWER(tmpPer->name[0]), sstring(tmpPer->name).lower().c_str());
  if (!(fp = fopen(buf, "r"))) 
    return FALSE;
  
  parseFollowerRent(fp, this, tmpPer->name.c_str());

  fclose(fp);
  return TRUE;
}

void TBeing::reconnectEquipmentHandler(void)
{
  // This is designed to be called ONLY when char is reconnecting
  // and fixes some problems with descriptor data due to way things get
  // rented/saved.  doSave strips char naked (no affects) and then saves.
  // The load starts with them naked and puts eq back on them (reinstalling
  // affects properly).

  // the problem is if char dumps link and then reconnects.  They will be
  // saved without effects, but will reconnect with eq already on them so
  // things are not put back on properly.

  // only way around this I can come up with right now is to strip a char
  // reset the appropriate stuff, and then reequip them
  // - Batopr

  TThing *obj_array[MAX_WEAR];
  wearSlotT i;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (equipment[i]) {
      obj_array[i] = unequip(i);
    } else
      obj_array[i] = NULL;
  }

  // put all resets here
  age_mod = 0;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (obj_array[i]) 
      equipChar(obj_array[i], i, SILENT_YES);
  }
  doSave(SILENT_YES);
}

void TPerson::loadToggles()
{
  char caFilebuf[128];
  FILE *fp;
  int num;

  sprintf(caFilebuf, "player/%c/%s.toggle", LOWER(name[0]), sstring(name).lower().c_str());

  if (!(fp = fopen(caFilebuf, "r")))
    return;

  while (fscanf(fp, "%d ", &num) == 1) {
    setQuestBit(num);
  }
  fclose(fp);
}

void TPerson::saveToggles()
{
  char caFilebuf[128];
  FILE *fp;
  int num;
  unsigned int total;

  sprintf(caFilebuf, "player/%c/%s.toggle", LOWER(name[0]), sstring(name).lower().c_str());

  if (!(fp = fopen(caFilebuf, "w")))
    return;

  for (num = 1, total = 0; num < MAX_TOG_INDEX; num++) {
    if (hasQuestBit(num)) {
      total++;
      fprintf(fp, "%d ", num);
    }
  }
  fclose(fp);

  if (!total)
    unlink(caFilebuf);
}

int TBeing::doRent(const sstring &argument)
{
  sendTo("You're a mob.  You can't rent!\n\r");
  return FALSE;
}

int TPerson::doRent(const sstring &argument)
{
  // note this is sort of a special case
  // special procedures (innkeeper, personalHouse) have already been
  // taken care of before this point

  // what we really want to do is let people rent out from camping

  if (!inCamp()) {
    doNotHere();
    return FALSE;
  }
  sendTo("You opt to rough it for awhile.\n\r");
  act("$n decides to rough it for awhile.",
       TRUE, this, 0, 0, TO_ROOM);

  cls();
  desc->outputProcessing();

  int rc = saveRent(TRUE, 2);
  if (IS_SET_ONLY(rc, DELETE_VICT))
      return DELETE_THIS;
  return rc;
}

bool TObj::isRentable() const
{
  if (isObjStat(ITEM_NORENT) || (number < 0))
    return FALSE;
  return TRUE;
}

rentObject::rentObject() :
  item_number(0),
  extra_flags(0),
  weight(0.0),
  bitvector(0),
  decay_time(0),
  struct_points(0),
  max_struct_points(0),
  material_points(0),
  volume(0),
  cost(0),
  depreciation(0)
{
  memset(&value, 0, sizeof(value));

  // objAffData affected[MAX_OBJ_AFFECT];
}

pcorpseObject::pcorpseObject() :
  corpse_room(0),
  num_corpses_room(0)
{
  memset(&charName, '\0', sizeof(charName));
}

rentHeader::rentHeader() :
  version(0),
  last_update(0),
  first_update(0),
  number(0)
{
  memset(&owner, '\0', sizeof(owner));
}

void processCorpseFile(const char * cfName)
{
  char fileName[128];
  FILE *fp;
  rentHeader h;

  if (!cfName) {
    vlogf(LOG_BUG, "  processCorpseFile called with NULL filename!");
    return;
  }
  sprintf(fileName, "corpses/%s", cfName);
  if (!(fp = fopen(fileName, "r+b"))) {
    vlogf(LOG_FILE, format("  Error opening the corpse file for corpse %s") %  cfName);
    return;
  }
  if ((fread(&h, sizeof(rentHeader), 1, fp)) != 1) {
    vlogf(LOG_BUG, format("Error reading version from %s.") %  fileName);
    fclose(fp);
    return;
  }

  if (!noteLimitedItems(fp, fileName, h.version, FALSE))
    vlogf(LOG_BUG, format("  Unable to count limited items in file  %s") %  fileName);

  fclose(fp);
}

void processCorpseFiles()
{
  dirwalk("corpses", processCorpseFile);
}

rentObjAffData::rentObjAffData() :
  type(0),
  level(0),
  duration(0),
  renew(0),
  modifier(0),
  location(0),
  modifier2(0),
  bitvector(0)
{
}


// based in part on void TPerson::loadRent()
void TBeing::doClone(const sstring &arg)
{
  sstring ch_name = arg.word(0);
  sstring arg2 = arg.word(1);
  int mob_num = convertTo<int>(arg2);
  TMonster *mob;
  charFile st1;
  int ci, num_read = 0;
  ItemLoad il;

  if (powerCheck(POWER_CLONE)) {
    return;
  }

  if (!isImmortal())
    return;

  if(ch_name.empty() || arg2.empty()) {
    sendTo("Syntax: clone <playername> <mobvnum>\n\r");
    return;
  }

  if(!load_char(ch_name, &st1)) {
    sendTo("Can't find that player file.\n\r");
    return;
  }

  for(ci = 0;ci < MAX_SAVED_CLASSES;ci++) {
    if(st1.level[ci] > GetMaxLevel()) {
      sendTo("You can't clone a player of higher level than you.\n\r");
      return;
    }
  }
  
  if (!(mob = read_mobile(mob_num, VIRTUAL))) {
    sendTo("Mob not found\n\r");
    return;
  }
  thing_to_room(mob, in_room);
  mob->swapToStrung();

  mob->name = format("%s [clone]") % st1.name;

  mob->shortDescr = st1.name;

  mob->player.longDescr = format("<c>%s<1> is standing here.") % st1.name;

  if(*st1.description)
    mob->setDescr(format("%s\n\r") % st1.description);
  else 
    mob->setDescr("You see nothing special about him.\n\r");
  
  mob->setSex(sexTypeT(st1.sex));
  mob->setHeight(st1.height);
  mob->setWeight(st1.weight);
  mob->setRace(race_t(st1.race));
  
  
  // open player rent file
  sstring buf = format("rent/%c/%s") % LOWER(ch_name[0]) % ch_name.lower();
  if (!il.openFile(buf)){
    sendTo("Rent file could not be opened.  Your clone stands naked before you.\n\r");
    return;
  }
  if(!il.readHeader()){
    vlogf(LOG_BUG, format("Error while reading %s's rent file header.  Your clone stands naked before you.") %  ch_name);
    return;
  }
  if (il.objsFromStore(NULL, &num_read, mob, NULL, false)) {
    vlogf(LOG_BUG, format("Error while reading %s's objects in doClone.  Your clone stands naked before you.") %  ch_name);
    return;
  }
  
  // add NO RENT to the objects, don't want them falling into PC 
  //   hands permanently
  // ALSO - junk notes, and increase object number for these loads (since
  //   they will decrease when purged)
  TObj *o;
  TThing *i, *j;
  TObj *bo;
//  TBaseContainer *b1;
  
  for(StuffIter it=mob->stuff.begin();it!=mob->stuff.end();){
    i=*(it++);
    if ((o = dynamic_cast<TObj *>(i)))
    {
      obj_index[o->getItemIndex()].addToNumber(1);
      o->addObjStat(ITEM_NORENT);
      if ((dynamic_cast<TNote *>(o)))
      {
        o->makeScraps();
        delete o;
        continue;
      }
    } else 
      vlogf(LOG_BUG, format("did not add no-rent flag to %s when cloning") %  
	    i->name);
    
    if ((dynamic_cast<TBaseContainer *>(i))) {
      for(StuffIter itt=i->stuff.begin();itt!=i->stuff.end();){
	j=*(itt++);
        if ((bo = dynamic_cast<TObj *>(j))) 
        {
          obj_index[bo->getItemIndex()].addToNumber(1);
          bo->addObjStat(ITEM_NORENT);
        } else 
          vlogf(LOG_BUG, format("did not add no-rent flag to %s when cloning") %  
	          bo->name);
        
        if ((dynamic_cast<TNote *>(j))) 
        {
          j->makeScraps();
          delete j;
          continue;
        }
      }
    }
  }

  // this bit makes the mob TRUE for isPc, and prevents the look responses, etc
  
  sendTo("Your clone appears before you.\n\r");
}
