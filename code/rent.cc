#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "stdsneezy.h"
#include "rent.h"
#include "statistics.h"
#include "mail.h"
#include "shop.h"
#include "database.h"
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

static const char ROOM_SAVE_PATH[] = "roomdata/saved";
static const int NORMAL_SLOT   = -1;
static const int CONTENTS_END  = -2;
static const int CORPSE_END  = -3;

#define FREE_RENT true

static const int LIMITED_RENT_ITEM = 9;  
// in 5.2 anything with max exists <= this number will be charged rent

static const bool FreeRent = true;
//BOD decision 8-28-01

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

  {0, RACE_NORACE, false, "Leave!"} // Add all new entries BEFORE this line.
};

// this returns a number indicating how long we "think" it should
// take to get to level "lev"
// it is somewhat arbitrary
//
int secs_to_level(int lev)
{
  lev--;
  if (lev <= 0)
    return 0;

  // how much damage (average/round) am I doing
  float dam_level = 0.9 * lev;

  // what is the average mob have for hp
  float avg_mob_hp = (11.0 + 4.5) * lev;

  // average combat length
  float rounds_combat = avg_mob_hp / dam_level;

  // converted to number of seconds
  int time_combat = (int) (rounds_combat * PULSE_COMBAT / ONE_SECOND);

  // now figure out regen
  // assume that a fiar fight consumes 100% of players HP each kill
  int avg_pc_hp = 15 + (10*lev);

  // numer of hps gained back per tick
  int regen_per_tick;
  regen_per_tick = (int) (stats.hit_gained_attempts == 0 ? 1 :
        ((float) stats.hit_gained / (float) stats.hit_gained_attempts));

  float ticks_regen = (float) avg_pc_hp / (float) regen_per_tick;

  int secs_regen = (int) (ticks_regen * (PULSE_UPDATE/2) / ONE_SECOND);

  int tot_time = secs_regen + time_combat;

  // and i have to kill a bunch of mobs
  tot_time *= kills_to_level(lev);

  // don't forget to add in how long it took me to get to last level.
  tot_time += secs_to_level(lev);

  return tot_time;
}

float power_level_number(int lev)
{
  float levelfactor;

  // if they were doing it themself, this is the number of days it
  // should take.
  levelfactor = (float) secs_to_level(lev) / (float) (60*60*24);

  // make an allowance for somebody being super cool
  levelfactor *= 0.6;

  // this formula is roughly geared toward the "standard" difficulty
  // if we have made game harder, adjust accordingly.
  levelfactor /= stats.xp_modif;

  return  levelfactor;
}

static ubyte credit_factor(const TBeing *ch)
{
  // we are going to modify rent credit to adjust it for players that seem
  // to have been leveled too fast.  This is somewhat arbitrary
  // In theory, we want it to take about 1 day of play to hit L10 and
  // 25 days to hit L50.
  // I have basically graphed the curve to be playtime in days = (L/10) ^ 2
  // this should return a value between 0-100

  struct time_info_data playing_time; 
  float playtime, levelfactor;

  realTimePassed((time(0) - ch->player.time.logon) +
                                  ch->player.time.played, 0, &playing_time);
  playtime = (float) playing_time.day;
  playtime += (float) playing_time.hours / 24.0;
  if (playtime <= 0.0)
    playtime = 1.0/24.0;

// Cosmo 12/22/97 the xp_modif changed to apply to levelfactor by dividing
// done in other function above 
  levelfactor = power_level_number(ch->GetMaxLevel());

  if (playtime >= levelfactor || levelfactor <= 0.0)
    return 100;

  return (ubyte) ( 100 * playtime / levelfactor);
}

double getLevMod(unsigned short int Class, unsigned int lev)
{
  double lev_mod = 0;  // warriors is 0.0
  if (IS_SET(Class, CLASS_MAGE))
    lev_mod = max(lev_mod, 10.0);
  if (IS_SET(Class, CLASS_CLERIC))
    lev_mod = max(lev_mod, 7.0);
  if (IS_SET(Class, CLASS_THIEF))
    lev_mod = max(lev_mod, 5.0);
  if (IS_SET(Class, CLASS_DEIKHAN))
    lev_mod = max(lev_mod, 3.0);
  if (IS_SET(Class, CLASS_MONK))
    lev_mod = max(lev_mod, 10.0);
  if (IS_SET(Class, CLASS_RANGER))
    lev_mod = max(lev_mod, 3.0);

  // phase in rent credit loss over few levels
  // this is mostly here so all newbies are essentially the same
  lev_mod = min(lev_mod, lev/3.0);
  return lev_mod;
}

unsigned int rent_credit(ush_int Class, unsigned int orig_lev, unsigned int numClasses)
{
#ifdef FREE_RENT
  // for 5.2 we're going for 0 base rent credit for all classes, then charging on basis of max exists
  return (unsigned int)0;
#endif

  // First, establish credit for the AC and struct of the player's
  // equipment.
  // this should be level based, but tweak that level for class since
  // certain classes ought to be using lower quality AC.
  double lev = orig_lev;

  // make corrections to lev, based on class wearability
  // this is here (c.f. balance notes) so that we can restrict the
  // AC from equipment a given class is able to have.
  double lev_mod = getLevMod(Class, orig_lev);

  lev -= lev_mod;
  lev = max(lev, 1.0);

  double num = (lev * max(20.0, lev) * RENT_CREDIT_VAL);

  // next, give credit for a weapon.
  // use the real level rather than the modified level we used above since
  // damage capacity is not class-modified.
  // a weapon should be an extra 20.5% of the price
  num += (orig_lev * max(20, (int) orig_lev) * RENT_CREDIT_VAL * .205);

  // make allowances for sundry items
  // this includes: water skin, food, lanterns, fuel, bags, whetstones, etc
  // assume need 1000 talens total for this crap
  num += min((int) orig_lev * 50, 1000);

  // make some allowances for other items a class might need as "appropriate"
  if (IS_SET(Class, CLASS_MAGE)) {
    // allowance for components
    // basically, give them spare capacity for 20 kills
    // c.f. balance notes for more discussion
    int amt = 15 * orig_lev * orig_lev;

    // don't be overly generous with this to multiclass
    amt /= numClasses;

    num += amt;
  }
  if (IS_SET(Class, CLASS_CLERIC)) {
    // allowance for symbols
    // permit them to carry 1.5 symbols of their level
    // symbol costs 15 * L^2 (rents for 1/2)
    int amt = (int) (1.5 * 15 * orig_lev * orig_lev / 2);
    amt /= numClasses;
    num += amt;
  }
  if (IS_SET(Class, CLASS_DEIKHAN)) {
    // allowance for symbols
    // permit them to carry 1.0 symbols of their level
    // symbol costs 15 * L^2 (rents for 1/2)
    int amt = (int) (1.0 * 15 * orig_lev * orig_lev / 2);
    amt /= numClasses;
    num += amt;
  }
  if (IS_SET(Class, CLASS_RANGER)) {
    // allowance for components
    // basically, give them spare capacity for 6 kills
    int amt = 5 * orig_lev * orig_lev;
    amt /= numClasses;
    num += amt;
  }

  return (unsigned int) num;
}

unsigned int TBeing::rentCredit() const
{
  unsigned int num =  rent_credit(getClass(), GetMaxLevel(), howManyClasses());

  // correct for powerleveling
  ubyte cred = credit_factor(this);
  num *= cred;
  num /= 100;
 
  return num;
}

void handleCorrupted(const char *name, char *account)
{
  char buf[200];

  sprintf(buf, "mv player/%c/%s player/corrupt/.", 
         LOWER(name[0]), sstring(name).lower().c_str());
  vsystem(buf);

  // wizpower
  sprintf(buf, "mv player/%c/%s.wizpower player/corrupt/.", 
         LOWER(name[0]), sstring(name).lower().c_str());
  vsystem(buf);

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
    vlogf(LOG_FILE, fmt("error in unlink (0) (%s) %d") %  buf % errno);
  }

  // nuke wizpowers, ignore errors
  sprintf(buf, "player/%c/%s.wizpower", LOWER(name[0]), sstring(name).lower().c_str());
  unlink(buf);

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

  wipePlayerFile(tmp->name);
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
    vlogf(LOG_BUG, fmt("removeRent() called for a bad isPc() (%s)") %  getName()); 
    return;
  }

  wipeRentFile(tmp->name);
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
  memset(&st, 0, sizeof(rentHeader));
  fp=NULL;
}

ItemLoad::~ItemLoad()
{
  if(fp)
    fclose(fp);
}

bool ItemLoad::readVersion()
{
  if (fread(&version, sizeof(version), 1, fp) != 1)
    return false;
  return true;
}

ItemSave::ItemSave()
{
  memset(&st, 0, sizeof(rentHeader));
  st.version = CURRENT_RENT_VERSION;
  fp=NULL;
}

ItemSave::~ItemSave()
{
  if(fp)
    fclose(fp);
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

  ret=stat(filepath.c_str(), NULL);

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
    if (o->name) {
      if (fwrite(o->name, strlen(o->name) + 1, 1, fp) != 1) {
        vlogf(LOG_BUG, "Error writing object name to rent.");
        return FALSE;
      }
    } else
      vlogf(LOG_BUG, fmt("Object %d has no name!") %  o->objVnum());

    if (fwrite(o->shortDescr, strlen(o->shortDescr) + 1, 1, fp) != 1) {
      vlogf(LOG_BUG, "Error writing object short description to rent.");
      return FALSE;
    }

    if (o->getDescr()) {
      if (fwrite(o->getDescr(), strlen(o->getDescr()) + 1, 1, fp) != 1) {
        vlogf(LOG_BUG, "Error writing object description to rent.");
        return FALSE;
      }
    } else {
      vlogf(LOG_BUG, fmt("object %d has no descr") %  o->objVnum());
    }
    if (o->action_description) {
      if (fwrite(o->action_description, strlen(o->action_description) + 1, 1, fp) != 1) {
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

TObj *ItemLoad::raw_read_item()
{
  rentObject item;
  int j;
  TObj *o;
  char *name = NULL, *shortDescr = NULL, *description = NULL,
  *action_description = NULL;
  unsigned char oldversion=CURRENT_RENT_VERSION;

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

  // update these items - maror, 12-29-03
  // this isn't really a good way to do this
  // there's an if statement below to set back to current version
  if ((item.item_number >= 1120 && item.item_number <= 1131) || //admantium
      (item.item_number >= 8837 && item.item_number <= 8850) || //dark grey
      (item.item_number >= 9600 && item.item_number <= 9611) || //whale
      (item.item_number == 9624) || // whale hood
      (item.item_number == 27109) || // hawk ring
      (item.item_number >= 10049 && item.item_number <= 10062) || //emerald
      (item.item_number >= 10600 && item.item_number <= 10611) || //sylvanplate
      (item.item_number >= 10620 && item.item_number <= 10631) || //sylvansilk
      (item.item_number >= 23214 && item.item_number <= 23233)){  //dark blue
    oldversion=version;
    version = 6;
  }

      

  if (!(o = read_object(item.item_number, VIRTUAL))) {
    vlogf(LOG_BUG, fmt("Unable to load object Vnum = %d from rent.") %  item.item_number);
    return NULL;
  }

// old items should reflect current tiny file
  if (version >= 7 || 
      // discard 0-cost components and symbols due to overhaul
      dynamic_cast<TNote *>(o)) {


    if(version<9 && dynamic_cast<TOpenContainer *>(o)){
      item.value[1]=((item.value[1]>>8)<<16) ^ ((item.value[1]<<24)>>24);

      o->assignFourValues(item.value[0],item.value[1],item.value[2],item.value[3]);
    } else if(version<8 && dynamic_cast<TGenWeapon *>(o)){
      int x;

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
    if ((item.extra_flags & ITEM_STRUNG) &&
	!o->isObjStat(ITEM_STRUNG))
      o->addObjStat(ITEM_STRUNG);  // preserve strung
    
    // deal with structure
    o->setStructPoints(min(item.struct_points, (int) obj_index[o->getItemIndex()].max_struct));
    
    if ((item.extra_flags & ITEM_CHARRED) && !o->isObjStat(ITEM_CHARRED))
      o->addObjStat(ITEM_CHARRED); // preserve charred
  }
    
  if (o->isObjStat(ITEM_STRUNG)) {
    if (name)
      o->name = name;
    else
      o->name = mud_str_dup(obj_index[o->getItemIndex()].name);
    
    if (shortDescr)
	o->shortDescr = shortDescr;
    else
      o->shortDescr = mud_str_dup(obj_index[o->getItemIndex()].short_desc);
    
    if (description)
      o->setDescr(description);
    else
      o->setDescr(mud_str_dup(obj_index[o->getItemIndex()].long_desc));
    
    if (action_description) 
      o->action_description = action_description;
    else if (obj_index[o->getItemIndex()].description) 
      o->action_description = mud_str_dup(obj_index[o->getItemIndex()].description);
    else 
      o->action_description = NULL;
    
    if (obj_index[o->getItemIndex()].ex_description)
      o->ex_description =
	new extraDescription(*obj_index[o->getItemIndex()].ex_description);
    else
      o->ex_description = NULL;

//strung objects keep everything
#if 1 
    if(version<9 && dynamic_cast<TOpenContainer *>(o)){
      item.value[1]=((item.value[1]>>8)<<16) ^ ((item.value[1]<<24)>>24);

      o->assignFourValues(item.value[0],item.value[1],item.value[2],item.value[3]);
    } else if(version<8 && dynamic_cast<TGenWeapon *>(o)){
      int x;

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
#endif 
  }
  // if they had a lantern lit, set light appropriately
  o->adjustLight();
  
  if ((item.item_number >= 1120 && item.item_number <= 1131) || //admantium
      (item.item_number >= 8837 && item.item_number <= 8850) || //dark grey
      (item.item_number >= 9600 && item.item_number <= 9611) || //whale
      (item.item_number == 9624) || // whale hood
      (item.item_number == 27109) || // hawk ring
      (item.item_number >= 10049 && item.item_number <= 10062) || //emerald
      (item.item_number >= 10600 && item.item_number <= 10611) || //sylvanplate
      (item.item_number >= 10620 && item.item_number <= 10631) || //sylvansilk
      (item.item_number >= 23214 && item.item_number <= 23233)){  //dark blue
    version=oldversion;
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

  bool immortal = ((ch && ch->desc) ? IS_SET(ch->desc->account->flags, ACCOUNT_IMMORTAL) : FALSE);

  if (!corpse && immortal && shouldRecycle(new_obj->getItemIndex())) {
    char buf[1200];
    sprintf(buf, "Item (%s) was automatically recycled due to your immortal status.\n\r", new_obj->getName());
    autoMail(ch, NULL, buf);
    vlogf(LOG_SILENT, fmt("%s's %s being recycled due to immortality.") %  ch->getName() % new_obj->getName());


    delete new_obj;
    new_obj = NULL;
    return true;
  }
  return false;
}

// read a list of items and their contents from storage 
bool ItemLoad::objsFromStore(TObj *parent, int *numread, TBeing *ch, TRoom *r, bool corpse)
{
  signed char slot;
  TObj *new_obj;

  while (!feof(fp)) {
    if (fread(&slot, sizeof(signed char), 1, fp) != 1) {
      if (r)
        vlogf(LOG_BUG, fmt("  Room %d.  Couldn't read slot.") %  r->number);
      else if (ch)
        vlogf(LOG_BUG, fmt(" %s's objects.  Couldn't read slot.") %  ch->getName());
      else
        vlogf(LOG_BUG, "Error in objsFromStore (1)");

      return TRUE;
    }
    
    if (slot == CONTENTS_END)
      return FALSE;
    if (parent) {
      if (slot == NORMAL_SLOT) {
        if ((new_obj = raw_read_item())) {
          (*numread)++;
          if (ch)
            ch->logItem(new_obj, CMD_WEST);  // rent in
	  obj_index[new_obj->number].addToNumber(-1);

          *parent += *new_obj;

          if (objsFromStore(new_obj, numread, ch, r, corpse)) {
            vlogf(LOG_BUG, "Error in objsFromStore (1)");
            return TRUE;  // ERROR occured 
          }

          if (immortalityNukeCheck(ch, new_obj, corpse))
            continue;  // new_obj invalid if this was true

	  repoCheckForRent(ch, new_obj, corpse);

        } else {
          vlogf(LOG_BUG, "Error in objsFromStore (2)");
          return TRUE;
        }
      } else {
        if (r)
          vlogf(LOG_BUG, fmt("Room %d.  Invalid Slot %d.") %  r->number % slot);
        else if (ch)
          vlogf(LOG_BUG, fmt("%s's objects.  Invalid slot %d.") %  ch->getName() % slot);
        vlogf(LOG_BUG, "Error in objsFromStore (3)");
        return TRUE;
      }
    } else {
      if (slot >= MAX_WEAR) {
        if (ch)
          vlogf(LOG_BUG, fmt("%s's objects.  Slot %d > MAX_WEAR.") %  ch->getName() % slot);
        else if (r)
          vlogf(LOG_BUG, fmt("Room %d's objects.  Slot %d > MAX_WEAR.") %  r->number % slot);
        vlogf(LOG_BUG, "Error in objsFromStore (4)");
        return TRUE;
      } else if (slot >= 0) {
        if ((new_obj = raw_read_item())) {
          (*numread)++;
          if (ch)
            ch->logItem(new_obj, CMD_WEST);  // rent in
	  obj_index[new_obj->number].addToNumber(-1);

          if (ch) {
            wearSlotT mapped_slot = mapFileToSlot( slot);
            if (!ch->canUseLimb(mapped_slot))
              *ch += *new_obj;
            else
              ch->equipChar(new_obj, mapped_slot, SILENT_YES);

          } else
            vlogf(LOG_BUG, fmt("Room %d has invalid slot #.") %  
		  ((r) ? r->number : -99));

          if (objsFromStore(new_obj, numread, ch, r, corpse)) {
            vlogf(LOG_BUG, "Error in objsFromStore (5)");
            return TRUE;  // ERROR occured 
          }

          if (immortalityNukeCheck(ch, new_obj, corpse))
            continue;  // new_obj invalid if this was true

	  repoCheckForRent(ch, new_obj, corpse);

        } else {
          vlogf(LOG_BUG, "Error in objsFromStore (6)");
          return TRUE;
        }
      } else if (slot == NORMAL_SLOT) {
        if ((new_obj = raw_read_item())) {
          if (ch)
            ch->logItem(new_obj, CMD_WEST);  // rent in
	  obj_index[new_obj->number].addToNumber(-1);

          (*numread)++;
          if (ch)
            *ch += *new_obj;
          else if (r)
            thing_to_room(new_obj, r->number);
          else
            vlogf(LOG_BUG, "Yikes!  An object was read with no destination in objsFromStore()!");
          if (objsFromStore(new_obj, numread, ch, r, FALSE)) {
            vlogf(LOG_BUG, "Error in objsFromStore (7)");
            return TRUE;            // ERROR occured 
          }

          if (immortalityNukeCheck(ch, new_obj, corpse))
            continue;  // new_obj invalid if this was true

	  repoCheckForRent(ch, new_obj, corpse);


        } else {
          vlogf(LOG_BUG, "Error in objsFromStore (8)");
          return TRUE;
        }
      } else {
        if (r)
          vlogf(LOG_BUG, fmt(" Room %d.  Invalid slot %d.") %  r->number % slot);
        else if (ch) 
          vlogf(LOG_BUG, fmt(" %s's objects.  Invalid slot %d.") %  ch->getName() % slot);
        
        vlogf(LOG_BUG, "Error in objsFromStore (9)");
        return TRUE;
      }
    }
  }
  return FALSE;
}

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
void ItemSave::objsToStore(signed char slot, TObj *o, 
			   TBeing *ch, bool d, bool corpse = FALSE)
{
  if (!o)
    return;

  // ignore beings
  TThing *ttt = o;
  if (dynamic_cast<TBeing *>(ttt)) {
    // TRoom::saveItems  calls this, we don't want to save beings that might
    // be hanging out in the room
    objsToStore(NORMAL_SLOT, (TObj *) o->nextThing, ch, d, corpse);

    // with persistent rooms, we don't need pcorpse saving
#if 0
    // save pcorpses
  } else if (corpse && !(o->parent && o->parent->isPc())) {
    // sanity check
    TPCorpse * tmpcorpse = dynamic_cast<TPCorpse *>(o);
    if (!tmpcorpse)
      return;

    if (fwrite(&slot, sizeof(signed char), 1, fp) != 1) {
      vlogf(LOG_BUG, fmt("Error saving %s's objects -- slot write.") %
	    tmpcorpse->getName());
      return;
    }

    (st.number)++;
    if (!raw_write_item(tmpcorpse))
      vlogf(LOG_BUG, fmt("Rent error in %s's file") %  tmpcorpse->getName());

    objsToStore(NORMAL_SLOT, dynamic_cast<TObj *>(tmpcorpse->getStuff()), 
		ch, d, FALSE);
    slot = CONTENTS_END;
    if (fwrite(&slot, sizeof(signed char), 1, fp) != 1) {
      vlogf(LOG_BUG, fmt("Error saving %s's objects -- slot write.") % 
	    ((ch) ? ch->getName() : "UNKNOWN"));
      return;
    }
    if (tmpcorpse->getNext()) 
      objsToStore(NORMAL_SLOT, tmpcorpse->getNext(), ch, d, corpse);
#endif

    // if it's not rentable, save what it contains and
    // move on to the next item in the list
  } else if (!o->isRentable()) {
    objsToStore(NORMAL_SLOT, (TObj *) o->getStuff(), ch, d, corpse);
    objsToStore(NORMAL_SLOT, (TObj *) o->nextThing, ch, d, corpse);


    // normal item, save it
  } else {
    // write out the slot
    if (fwrite(&slot, sizeof(signed char), 1, fp) != 1) {
      vlogf(LOG_BUG, fmt("Error saving %s's objects -- slot write.") %
	    (ch?ch->getName():"unknown"));
      return;
    }

    (st.number)++;

    // write out the item
    if (!raw_write_item(o)) 
      vlogf(LOG_BUG, fmt("Rent error in %s's file") %
	    (ch?ch->getName():"UNKNOWN"));

    // save the contents
    objsToStore(NORMAL_SLOT, (TObj *) o->getStuff(), ch, d, corpse);

    // write the contents footer
    slot = CONTENTS_END;
    if (fwrite(&slot, sizeof(signed char), 1, fp) != 1) {
      vlogf(LOG_BUG, fmt("Error saving %s's objects -- slot write (2).") %  
	    ((ch) ? ch->getName() : "UNKNOWN"));
      return;
    }

    // if there's something else in the list
    if (o->nextThing) {
      // and it has a name, store it
      if (o->nextThing->getName()) {
        objsToStore(NORMAL_SLOT, (TObj *) o->nextThing, ch, d, corpse);
      } else {
        o->nextThing = NULL;
        vlogf(LOG_BUG, fmt("Error saving %s's objects -- nextThing.") % 
          ((ch) ? ch->getName() : "UNKNOWN"));
      }
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

void TBeing::addObjCost(TBeing *re, TObj *obj, objCost *cost, sstring &str)
{
  int temp;
  char buf[256];

  if (!obj)
    return;

  silentTypeT silent = SILENT_NO;
  if (desc && IS_SET(desc->autobits, AUTO_NOSPAM))
    silent = SILENT_YES;
  
  if (obj->isRentable()) {
    temp = max(0, obj->rentCost());
#ifdef FREE_RENT
    // in sneezy 5.2 we don't want to charge for anything that isn't limited. -dash 01/01
    if(obj->max_exist > LIMITED_RENT_ITEM) temp = 0;
    //    vlogf(LOG_DASH, fmt("%s getting cost on %s, max exist %d, limit %d, cost %d") %  getName() % obj->getName() %
    //	  obj->max_exist % LIMITED_RENT_ITEM % temp);
    
    // BOD decision 8-28-01 - no rent :)
    if (FreeRent) temp = 0;

#endif
    cost->total_cost += temp;
    if (re) {
      if (desc && desc->m_bIsClient) {
        if(!FreeRent) {
	  sprintf(buf, "%-30s : %d talens/day\n\r", 
		  obj->getName(), temp);
	} else
	  sprintf(buf, "%-30s \n\r",
		  obj->getName());
      }

        str += buf;
    } else if (!silent && re) {
        if (!FreeRent) 
	  sendTo(COLOR_OBJECTS, fmt("%-30s : %d talens/day\n\r") % obj->getName() % temp);
	else
	  sendTo(COLOR_OBJECTS, fmt("%-30s \n\r") % obj->getName());

    }
    if (temp<=100)
      cost->lowrentobjs++;
    cost->no_carried++;
  } else {
    if (re) {
      act("$n tells you 'Sorry!  I refuse to store $p.'", 
        FALSE, re, obj, this, TO_VICT, ANSI_ORANGE);
    }
    cost->ok = FALSE;
  }
  addObjCost(re, (TObj *) (obj->getStuff()), cost, str);
  addObjCost(re, (TObj *) (obj->nextThing), cost, str);
}

bool TBeing::recepOffer(TBeing *recep, objCost *cost)
{
  char buf[256];
  int i, actual_cost;
  unsigned int credit;
  TObj *obj;
  sstring str;
  followData *f;

  if (!cost)
    return FALSE;

  bool client = (desc && desc->m_bIsClient);

  cost->total_cost = 0;
  cost->ok = TRUE;
  cost->no_carried = 0;

  // add up cost for the player
  addObjCost(recep, (TObj *) getStuff(), cost, str);

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    obj = dynamic_cast<TObj *>(equipment[i]);
    if (!obj)
      continue;
    if (!(((i == WEAR_LEGS_L) && obj->isPaired()) ||
          ((i == WEAR_EX_LEG_L) && obj->isPaired()) ||
          ((i == HOLD_LEFT) && obj->isPaired()))) {
      addObjCost(recep, obj, cost, str);// equip
    }
  }
  // add up cost for followers
  for (f = followers; f; f = f->next) { 
    TMonster *ch = dynamic_cast<TMonster *>(f->follower);
    if (!ch)
      continue;
    
    if (!ch->isSaveMob(this))
      continue;

    // don't save if not around
    if (!ch->sameRoom(*this))
      continue;

    actual_cost = ch->petPrice() / 4;

    silentTypeT silent = SILENT_NO;
    if (desc && IS_SET(desc->autobits, AUTO_NOSPAM))
      silent = SILENT_YES;

    if (FreeRent) actual_cost = 0;

    if (recep) {
      if (desc && desc->m_bIsClient) {
        if (!FreeRent) 
	  sprintf(buf, "%-30s : %d talens/day ********** Storage fee \n\r", ch->getName(), actual_cost);
        else
	  sprintf(buf, "%-30s - Pet/Charm/Thrall/Mount \n\r", ch->getName());
	str += buf;
      } else if (!silent) {
	if (!FreeRent) 
	  sendTo(COLOR_OBJECTS, fmt("%-30s : %d talens/day   ********** Storage fee \n\r") % ch->getName() % actual_cost);
        else
          sprintf(buf, "%-30s - Pet/Charm/Thrall/Mount \n\r", ch->getName());

      }
    }
    cost->total_cost += actual_cost;

    // mob's inventory
    addObjCost(recep, (TObj *) (ch->getStuff()), cost, str);

    // mob's equipment
    for (i = MIN_WEAR; i < MAX_WEAR; i++) {
      obj = dynamic_cast<TObj *>(ch->equipment[i]);
      if (!obj)
        continue;
      if (!(((i == WEAR_LEGS_L) && obj->isPaired()) ||
            ((i == WEAR_EX_LEG_L) && obj->isPaired()) ||
           ((i == HOLD_LEFT) && obj->isPaired()))) {
        addObjCost(recep, obj, cost, str);// equip
      }
    }
  }
  if (recep) {
    if (!cost->ok)
      return FALSE;
  }
  if (isImmortal()) {
    if (recep)
      if (!FreeRent)
	act("$n tells you 'Considering you're immortal, you can rent for free.'",0, recep, 0, this, TO_VICT);

    cost->total_cost = 0;
    if (client && recep) {
      processStringForClient(str);

      desc->clientf(fmt("%d") % CLIENT_RENT);
      sendTo(str);
      desc->clientf(fmt("%d") % CLIENT_RENT_END);
    }
    return TRUE;
  }
  if (cost->no_carried > MAX_OBJ_SAVE) {
    sprintf(buf, "$n tells you, \"Sorry, but I can't store more than %d items.\n\rYou have %d items.\"", MAX_OBJ_SAVE, cost->no_carried);
    if (recep)
      act(buf, FALSE, recep, 0, this, TO_VICT);

    return FALSE;
  }
  if (recep && hasClass(CLASS_MONK) && ((cost->no_carried-cost->lowrentobjs) > 35)) {
    sendTo("You remember your vow not to carry over 35 items, and change your mind.\n\r");
    sendTo(fmt("You are currently carrying %d items.\n\r") % (cost->no_carried-cost->lowrentobjs));
    return FALSE;
  }
  if (recep) {
    if (!FreeRent) {
      sprintf(buf, "$n tells you, \"That totals to be %d talens.\"", cost->total_cost);
      act(buf, TRUE, recep, NULL, this, TO_VICT);
      if (cost->total_cost/GetMaxLevel() > 5000)
	vlogf(LOG_BUG, fmt("%s has %d value in equipment and is level %d") %  getName() % cost->total_cost % GetMaxLevel());

      if (cost->no_carried && (cost->no_carried < 10) && (((cost->total_cost)/(cost->no_carried)) > 15000)) 
	vlogf(LOG_BUG, fmt("%s has only %d items with an %d average cost, please check") %  
	      getName() % cost->no_carried % (cost->total_cost/cost->no_carried));
    }
  }

#if RENT_RESTRICT_INNS_BY_LEVEL
  // note that you could use autorent to get around this rent credit reduction
  if (recep && (recep->GetMaxLevel() < GetMaxLevel()) {
    sprintf(buf,"%s I can only grant rent credit through level %d.",
           getName(),recep->GetMaxLevel());
    recep->doTell(buf);
    sprintf(buf,"%s That's %d talens of credit.",
            getName(),recep->rentCredit());
    recep->doTell(buf);
    credit = recep->rentCredit();
  } else
    credit = rentCredit();
#else
  credit = rentCredit();  
#endif
  if (desc) {
    desc->best_rent_credit = max(credit, desc->best_rent_credit);
    credit = desc->best_rent_credit;
  }
#ifdef FREE_RENT
  credit = 0;
#endif
  actual_cost = cost->total_cost - credit;
  cost->total_cost = (actual_cost < 0) ? 0 : actual_cost;

  // sprintf(buf, "$n tells you 'You have been given a rent credit of %d talens.'", credit);
  //if (recep) 
  //  act(buf, FALSE, recep, 0, this, TO_VICT);
  
  if (FreeRent) {
    if (recep) {
      act("$n tells you \"Have a nice stay!\"", FALSE, recep, 0, this, TO_VICT);
    }
  } else if (!cost->total_cost) {
    if (recep) {
      sprintf(buf, "$n tells you 'That puts your daily rent at %d talens.'", cost->total_cost);
      act(buf, FALSE, recep, 0, this, TO_VICT);
      act("$n tells you 'I guess that means you rent free.", FALSE, recep, 0, this, TO_VICT);
    }
  } else {
    int daily_cost = cost->total_cost;
    int adjusted_cost = (int) (daily_cost * gold_modifier[GOLD_RENT].getVal());
    cost->total_cost = adjusted_cost;

    if (recep) {
#if 1
      //  sprintf(buf, "$n tells you 'Your stuff is %d talens over your credit.'", daily_cost);
      //act(buf, FALSE, recep, 0, this, TO_VICT);
      sprintf(buf, "$n tells you 'The current rent multiplier is %.2f.'", gold_modifier[GOLD_RENT].getVal());
      act(buf, FALSE, recep, 0, this, TO_VICT);
#endif

      sprintf(buf, "$n tells you 'That puts your daily rent at %d talens.'", cost->total_cost);
      act(buf, FALSE, recep, 0, this, TO_VICT);
      
#if FACTIONS_IN_USE
      if (isSameFaction(recep) && !recep->isUnaff()) {
        act("$n tells you 'Because you are of the same allegiance, I will give you a discount based on your faction percentage.", FALSE, recep, 0, this, TO_VICT);
        cost->total_cost /= (int) ((double) 200/(200 - (int) getPerc())); 
        sprintf(buf, "$n tells you 'That puts your daily rent at %d talens.'", cost->total_cost);
        act(buf, FALSE, recep, 0, this, TO_VICT);
      }
#endif
    }
  }
  if (cost->total_cost > (getMoney() + getBank())) {
    if (recep)
      act("$n tells you 'You don't have enough money on you or in the bank.'",FALSE,recep,0,this,TO_VICT);

    return FALSE;
  } else if (cost->total_cost > getMoney()) {
    if (recep) {
      act("$n tells you '... Your bank account is footing part of the bill.'", FALSE, recep, 0, this, TO_VICT);
      sprintf(buf, "$n tells you 'You can afford to rent for at most %d day%s.'",
        (getMoney() + getBank()) / (cost->total_cost),
        (((getMoney() + getBank()) / (cost->total_cost)) == 1 ? "" : "s"));
      act(buf, FALSE, recep, 0, this, TO_VICT);
    }
  } else if (cost->total_cost) {
    if (recep) {
      sprintf(buf, "$n tells you 'You can afford to rent for at most %d day%s.'",
        (getMoney() + getBank()) / (cost->total_cost),
        (((getMoney() + getBank()) / (cost->total_cost)) == 1 ? "" : "s"));
      act(buf, FALSE, recep, 0, this, TO_VICT);
      sprintf(buf, "$n tells you 'After %d day%s, money will be drawn against your bank balance.",
              getMoney() / (cost->total_cost),
              ((getMoney() / (cost->total_cost)) == 1 ? "" : "s"));
      act(buf, FALSE, recep, 0, this, TO_VICT);
    }
  } else {
    if (recep && !FreeRent) 
      act("$n tells you \"You can afford to rent as long as you'd like.\"", FALSE, recep, 0, this, TO_VICT);
  }
  if (client && recep) {
    processStringForClient(str);

    desc->clientf(fmt("%d") % CLIENT_RENT);
    sendTo(str);
    desc->clientf(fmt("%d") % CLIENT_RENT_END);
  }
  return TRUE;
}

void TMonster::saveItems(const sstring &filepath)
{
  TObj *obj;
  ItemSave is;

  if (!is.openFile(filepath)) {
    vlogf(LOG_FILE, fmt("Failed to open file '%s' in TMonster::saveItems() call.") % filepath);
    return;
  }

  is.writeVersion();

  // store worn objects
  wearSlotT ij;
  for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
    obj = dynamic_cast<TObj *>(equipment[ij]);
    if (!obj)
      continue;
    if (!(((ij == WEAR_LEGS_L) && obj->isPaired()) ||
          ((ij == WEAR_EX_LEG_L) && obj->isPaired()) ||
          ((ij == HOLD_LEFT) && obj->isPaired()))) {
      is.objsToStore(mapSlotToFile(ij), obj, this, FALSE);
    }
  }

  // store inventory objects
  is.objsToStore(NORMAL_SLOT, (TObj *) getStuff(), this, FALSE);

  // write the rent file footer
  is.writeFooter();

  // shopkeeper specific stuff - save gold
  if(isShopkeeper()){
    TDatabase db(DB_SNEEZY);
    db.query("update shop set gold=%i where shop_nr=%i",
	     getMoney(), find_shop_nr(number));
  }
}

void TRoom::saveItems(const sstring &)
{
  sstring filepath;
  ItemSave is;

  filepath = fmt("%s/%d") % ROOM_SAVE_PATH % number;

  if(!getStuff()){
    unlink(filepath.c_str());
    return;
  }

  if(!is.openFile(filepath)){
    vlogf(LOG_BUG, fmt("Error saving room [%d] items.") %  number);
    return;
  }
  is.writeVersion();

  is.objsToStore(NORMAL_SLOT, (TObj *) getStuff(), NULL, FALSE);
  is.writeFooter();
}


void emailStorageBag(sstring tStMessage, sstring tStSender, TThing * tStuff)
{
  FILE * tFile;
  sstring tStMail("");

  if (gamePort != PROD_GAMEPORT)
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

  filepath = fmt("%s/%d") % ROOM_SAVE_PATH % number;

  if(!il.fileExists(filepath))
    return;

  if(!il.openFile(filepath)) {
    vlogf(LOG_FILE, fmt("Failed to open file '%s' in TRoom::loadItems() call.") % filepath);
    return;
  }
  
  reset = isRoomFlag(ROOM_SAVE_ROOM);
  // we need to fool obj_to_room into not trying to resave    
  // the room file when it loads objects into a SAVE_ROOM ... 
  if (reset)
    removeRoomFlagBit(ROOM_SAVE_ROOM);

  if(!il.readVersion()){
    vlogf(LOG_BUG, fmt("Error while reading version from %s.") % filepath);
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
  if (number == ROOM_STORAGE) {
    vlogf(LOG_LOW, "Storage: Booting Storage Room");

    TThing * tThing,
           * tCont,
           * tThingNext;
    TObj   * tBag = read_object(GENERIC_L_BAG, VIRTUAL);
    TBag   * tContainer;
    char     tString[256];
    charFile tSt;

    if (!tBag) {
      vlogf(LOG_LOW, "Storage: Failed to create Junk Bag.");
      return;
    }

    for (tThing = getStuff(); tThing; tThing = tThingNext) {
      tThingNext = tThing->nextThing;

      // Remove various things.
      if (!(tContainer = dynamic_cast<TBag *>(tThing))) {
        vlogf(LOG_LOW, fmt("Storage: Moving Junk: %s") %  tThing->name);
        --(*tThing);
        *tBag += *tThing;
        continue;
      }

      // Remove old junk bags.
      if (sscanf(tThing->name, "linkbag %[A-Za-z]", tString) != 1) {
        vlogf(LOG_LOW, "Storage: Moving Old Junk Bag");
        while ((tThing->getStuff())) {
          TThing * tTemp = tThing->getStuff();
          --(*tTemp);
          *tBag += *tTemp;
        }

        --(*tThing);
        delete tThing;

        continue;
      }

      // Now we verify the 'user'.  tString should have been set prior.
      if (!load_char(tString, &tSt)) {
        if (gamePort == PROD_GAMEPORT)
          emailStorageBag("User Deleted", tString, tThing);

        vlogf(LOG_LOW, fmt("Storage: Purging linkbag: %s") %  tString);
        --(*tThing);
        delete tThing;

        continue;
      }

      vlogf(LOG_LOW, fmt("Storage: Processing Linkbag: %s") %  tString);

      // If we got here, the bag is a linkbag and the player is around.
      for (tCont = tThing->getStuff(); tCont; tCont = tCont->nextThing) {
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
          vlogf(LOG_LAPSOS, fmt("Storage: Note:\n\r%s") %  tNote->action_description);
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
          vlogf(LOG_BUG, fmt("Storage: Unknown Month: %s") %  tMon);
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
          vlogf(LOG_BUG, fmt("Storage: Unknown Day: %s") %  tWek);
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
        vlogf(LOG_LAPSOS, fmt("Storage: %s") %  ctime(&tTempReal));

        double tTimeDiff = difftime(tCurrentTime, mktime(&tTime)),
               tCheck    = 60.0 * 60.0 * 24.0 * 30.0;

        // Allow a bag to be 'retained' for 30 days.
        if (tTimeDiff > tCheck || tTimeDiff < -tCheck) {
          if (gamePort == PROD_GAMEPORT)
            emailStorageBag("Time Expired", tString, tThing);

          vlogf(LOG_LOW, fmt("Storage: Expired: %s") %  tString);

          while ((tThing->getStuff())) {
            TThing * tTemp = tThing->getStuff();
            --(*tTemp);
            *tBag += *tTemp;
          }

          --(*tThing);
          delete tThing;
	}

        break;
      }

      if (!tCont)
        vlogf(LOG_LOW, fmt("Storage: Unable to find rent note for: %s") %  tString);
    }

    if (!tBag->getStuff())
      delete tBag;
    else {
      sstring tStString("");

      tBag->swapToStrung();
      tBag->addObjStat(ITEM_NOPURGE);

      delete [] tBag->getDescr();
      delete [] tBag->name;

      tStString = "A bag containing various junk.";
      tBag->setDescr(mud_str_dup(tStString));
      tStString = "bag junk various [wizard]";
      tBag->name = mud_str_dup(tStString);

      *this += *tBag;
    }
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
    vlogf(LOG_BUG, fmt("  Error opening the room save file for room #%s") %  tfname);
    return;
  }

  if (fread(&version, sizeof(version), 1, fp) != 1) {
    vlogf(LOG_BUG, fmt("Error reading version from %s.") %  fileName);
    fclose(fp);
    return;
  }

  if (!noteLimitedItems(fp, fileName, version, FALSE))
    vlogf(LOG_BUG, fmt("  Unable to count limited items in file  %s") %  fileName);
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

  if (!name || (name && !(strcmp(name, "corpse player dummy"))) || !isObjStat(ITEM_STRUNG))
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
      if ((tmpCorpse == this)) {
        vlogf(LOG_BUG,"Error in removeCorpseFromLists");
        break;
      }
      if ((tmpCorpse->nextGlobalCorpse == this)) {
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
    vlogf(LOG_BUG, fmt("Error in removeCorpseList, corpse says listed but no corpse in list (%s).") %  getName());
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
        vlogf(LOG_BUG, fmt("Big error in corpse list walking %s") %  fileName);
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
  if (!getStuff())
    return;

  if (checkOnLists())
    vlogf(LOG_BUG, fmt("Call to addCorpseToList for a corpse already on list (%s)") %  getName());
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
    if ((tmpCorpse == this))
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
  #if 1 
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
  rp = real_roomp(ROOM_CORPSE_STORAGE);

// HAVE A BEING CALL THIS WHEN LOGGING IN
//  sprintf(buf, "rent/%c/%s", LOWER(tmp->name[0]), tmp->name.lower());


  if(!il.openFile(buf)) {
    // this isn't an error really, just means they don't have a corpse in
    // the game
    //    vlogf(LOG_FILE, fmt("Failed to open file '%s' in assignCorpsesToRooms() call.") % buf);
    return;
  }

  sprintf(buf, "player/%c/%s", LOWER(name[0]), sstring(name).lower().c_str());
  if (!(playerFile = fopen(buf, "r"))) {
    wipeCorpseFile(sstring(name).lower().c_str());
  } 
  fclose(playerFile);

  if (GetMaxLevel() > MAX_MORT) {
    vlogf(LOG_BUG, fmt("An immortal had a corpse saved (%s).") %  getName());
    wipeCorpseFile(sstring(name).lower().c_str());
    return;
  }

  if(!il.readHeader()){
    vlogf(LOG_BUG, fmt("Error while reading %s's corpse file header.") %
	  getName());
    return;
  }

  reset = rp->isRoomFlag(ROOM_SAVE_ROOM);
  // we need to fool obj_to_room into not trying to resave
  // the room file when it loads objects into a SAVE_ROOM ...
  if (reset)
    rp->removeRoomFlagBit(ROOM_SAVE_ROOM);

  if (il.objsFromStore(NULL, &num_read, NULL, rp, TRUE)) {
    vlogf(LOG_BUG, fmt("Error while reading %s's corpse file. Prepare for reimb!") % getName());
    if (reset)
      rp->setRoomFlagBit(ROOM_SAVE_ROOM);
    return;
  }
  if (reset)
    rp->setRoomFlagBit(ROOM_SAVE_ROOM);

  for (tmp = rp->getStuff(); tmp;) {
    corpse = dynamic_cast<TPCorpse *>(tmp);
// tmp has to be here
    tmp = tmp->nextThing;
    if (!corpse) {
      continue;
    }
    if (isname(name, corpse->name)) {
      corpse->setOwner(sstring(name).lower().c_str());
      corpse->obj_flags.decay_time = max(corpse->obj_flags.decay_time, (short int) 60);
//      corpse->obj_flags.decay_time = max(corpse->obj_flags.decay_time, MAX_PC_CORPSE_EQUIPPED_TIME);
    }
    if (!corpse->getRoomNum()) {
      rp2 = real_roomp(ROOM_STORAGE);
      if (corpse->roomp)
        --(*corpse);
      *rp2 += *corpse; 
      vlogf(LOG_BUG, fmt("%s distributed to Storage Room (%d).") %  corpse->getName() % ROOM_STORAGE);
      sendTo(COLOR_BASIC, "<r>*** You had a CORPSE placed in the storage area. See a god to get it back. *** <z>\n\r");
    } else {
      rp2 = real_roomp(corpse->getRoomNum());
      if (!rp2) {
        rp2 = real_roomp(ROOM_STORAGE);
        if (corpse->roomp)
          --(*corpse);
        *rp2 += *corpse;
        vlogf(LOG_BUG, fmt("%s distributed to Storage Room (%d).") %  corpse->getName() % ROOM_STORAGE);
      sendTo(COLOR_BASIC, "<r>*** You had a CORPSE placed in the storage area. See a god to get it back. ***<z>\n\r");
      } else {
        if (corpse->roomp)
          --(*corpse);
        *rp2 += *corpse;
        vlogf(LOG_BUG, fmt("%s distributed to %s (%d).") %  corpse->getName() %
rp2->getName() % corpse->getRoomNum());
        sendTo(COLOR_BASIC, "<r>*** Your CORPSE has been restored to its place in the World ***.<z>\n\r");
      }
    }
    corpse->addCorpseToLists();
  }
  return;
#endif
}

void TPCorpse::saveCorpseToFile()
{
  char buf[256];
  TPCorpse *firstCorpse = NULL;
  TPCorpse *tmpCorpse = NULL;
  int numCorpses = 0;
  bool onlyCorpse = FALSE;
  ItemSave is;

  if (fileName.empty()) {
    vlogf(LOG_BUG, fmt("Attempt to save a corpse with no fileName (%s)") %  getName());
    return;
  }

  memset(buf, '\0', sizeof(buf));

  sprintf(buf, "corpses/%s", fileName.c_str());

  if(!is.openFile(buf)){
    vlogf(LOG_FILE, fmt("Failed to open file '%s' in saveCorpseToFile() call.") % buf);
    onlyCorpse = TRUE; 
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
  is.st.gold_left = (int) in_room; 
  is.st.original_gold = 0;
  is.st.total_cost = 0;
  is.st.first_update = is.st.last_update = (long) time(0);

  if(!is.writeHeader()){
    vlogf(LOG_BUG, fmt("Error writing corpse header for %s.") %  getName());
    return;
  }

  tmpCorpse = firstCorpse;
#if 0 
  while (tmpCorpse) {
    objsToStore(NORMAL_SLOT, (TObj *) tmpCorpse, NULL, FALSE, TRUE);
    tmpCorpse = tmpCorpse->nextCorpse;
  }
  is.writeFooter();
#else
  is.objsToStore(NORMAL_SLOT, (TObj *) tmpCorpse, NULL, FALSE, TRUE);
  is.writeFooter();
#endif

}

// msgStatus = 0, no log.
// msgStatus = 1, "saving"
// msgStatus = 2, "renting"
void TPerson::saveRent(objCost *cost, bool d, int msgStatus)
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
    vlogf(LOG_BUG, fmt("Error opening file for saving %s's objects") %
	  getName());
    return;
  }
  strcpy(is.st.owner, getName());
  is.st.number = (int) cost->no_carried;
  is.st.gold_left = (int) getMoney();
  is.st.original_gold = (int) getMoney();
  is.st.total_cost = (int) cost->total_cost;
  is.st.first_update = is.st.last_update = (long) time(0);


  if(!is.writeHeader()){
    vlogf(LOG_BUG, fmt("Error writing rent header for %s.") %  getName());
    return;
  }
  is.st.number = 0;        // reset to count actual # saved 

  wearSlotT ij;
  for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
    obj = dynamic_cast<TObj *>(equipment[ij]);
    if (!obj)
      continue;
    if (d) {
      unequip(ij);
      is.objsToStore(mapSlotToFile(ij), obj, this, d);
    } else {
      // if they're wearing a paired item, don't save the other slot 
      if (!(((ij == WEAR_LEGS_L) && obj->isPaired()) ||
          ((ij == WEAR_EX_LEG_L) && obj->isPaired()) ||
          ((ij == HOLD_LEFT) && obj->isPaired()))) {
        is.objsToStore(mapSlotToFile(ij), obj, this, d);
      }
    }
  }
  is.objsToStore(NORMAL_SLOT, (TObj *) getStuff(), this, d);
  is.writeFooter();

  if (d)
    setStuff(NULL);

#if 0
  // a nice idea, but no longer works since "cost" is tracking mobs +
  // items on mobs.  st is only tracking items on me.
  if (is.st.number != cost->no_carried)
    vlogf(LOG_BUG, fmt("Number of items saved [%d] for %s does not match # charged for [%d]") %  is.st.number % getName() % cost->no_carried);
#endif

  if (msgStatus == 1 && desc) {
    vlogf(LOG_PIO, fmt("Saving %s [%d talens/%d bank/%.2f xps/%d items/%d age-mod/%d rent]") %  
        getName() % getMoney() % getBank() % getExp() % is.st.number % 
        age_mod % is.st.total_cost);
  } else if (msgStatus == 2 && desc) {
    vlogf(LOG_PIO, fmt("Renting %s [%d talens/%d bank/%.2f xps/%d items/%d age-mod/%d rent]") %  
        getName() % getMoney() % getBank() % getExp() % is.st.number % 
        age_mod % is.st.total_cost);
  }
  last_rent = is.st.total_cost;

  if (!is.st.number) 
    wipeRentFile(getName());
}

// this is used to load the items a shopkeeper has
void TMonster::loadItems(const sstring &filepath)
{
  int num_read = 0;
  ItemLoad il;

  if(!il.openFile(filepath)) {
    vlogf(LOG_FILE, fmt("Failed to open file '%s' in TMonster::loadItems() call.") % filepath);
    return;
  }

  if(!il.readVersion()){
    vlogf(LOG_BUG, fmt("Error while reading version from %s.") %  filepath);
    return;
  }

  il.objsFromStore(NULL, &num_read, this, NULL, FALSE);
}

TObj *TBeing::findMostExpensiveItem()
{
  int i, high = 0;
  TObj *o = NULL;
  TThing *t, *t2;

  for(i = MIN_WEAR; i < MAX_WEAR; i++) {
    TObj *obj = dynamic_cast<TObj *>(equipment[i]);
    if (obj) {
      if (obj->rentCost() > high) {
        o = obj;
        high = obj->rentCost();
      }
      for (t2 = obj->getStuff(); t2; t2 = t2->nextThing) {
        obj = dynamic_cast<TObj *>(t2);
        if (!obj)
          continue;
        if (obj->rentCost() > high) {
          o = obj;
          high = obj->rentCost();
        }
      }
    }
  }
  for (t = getStuff(); t; t = t->nextThing) {
    TObj *obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    if (obj->rentCost() > high) {
      o = obj;
      high = obj->rentCost();
    }
    for (t2 = t->getStuff(); t2; t2 = t2->nextThing) {
      obj = dynamic_cast<TObj *>(t2);
      if (!obj)
        continue;
      if (obj->rentCost() > high) {
        o = obj;
        high = obj->rentCost();
      }
    }
  }
  return o;
}

void TThing::moneyMove(TBeing *ch)
{
  TThing *t, *t2;
  for (t = getStuff(); t; t = t2) {
    t2 = t->nextThing;
    t->moneyMove(ch);
  }
}

void TMoney::moneyMove(TBeing *ch)
{
  if (equippedBy) {
    ch->unequip(eq_pos);
  } else if (parent) {
    (*this)--;
  }

  ch->addToMoney(getMoney(), GOLD_XFER);
  vlogf(LOG_PIO, fmt("Found %d talens on %s's person during rent check") % 
            getMoney() % ch->getName()); 
  delete this;
}

void TBeing::moneyCheck()
{
  int i;
  TThing *t, *t2;

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if ((t = equipment[i])) {
      t->moneyMove(this);
    }
  }
  for (t = getStuff(); t; t = t2) {
    t2 = t->nextThing;
    t->moneyMove(this);
  }
}

// Somewhere in here, we need to call race->makeBody().
void TPerson::loadRent()
{
  int num_read = 0, timegold, gone, amt;
  TObj *i = NULL;
  //char buf[256], wizbuf[256];
  char buf[256];
  objCost cost;
  TPerson *tmp;
  sstring lbuf;
  ItemLoad il;

  if (desc && desc->original)
    tmp = desc->original;
  else
    tmp = dynamic_cast<TPerson *>(this);

  // a kludge
  // we get called by the char gen routine AFTER we gave char newbie gear
  // and saved.  We would wind up double loading, so avoid this by:
  if (time(0) - tmp->player.time.birth <= 3)
    return;

  sprintf(buf, "rent/%c/%s", LOWER(tmp->name[0]), sstring(tmp->name).lower().c_str());

  if(!il.openFile(buf)){
    if (should_be_logged(this)) {
      vlogf(LOG_PIO, fmt("%s has no equipment.") %  getName());
      vlogf(LOG_PIO, fmt("Loading %s [%d talens/%d bank/%.2f xps/no items/%d age-mod/no rent]") %  
         getName() % getMoney() % getBank() % getExp() % age_mod);
    }
    return;
  }
  if(!il.readHeader()){
    vlogf(LOG_BUG, fmt("Error while reading %s's rent file header.") %  getName());
    return;
  }
  if (il.objsFromStore(NULL, &num_read, this, NULL, false)) {
    vlogf(LOG_BUG, fmt("Error while reading %s's objects. Prepare for reimb!") % getName());
    return;
  }
  if (strcmp(name, il.st.owner))
    vlogf(LOG_BUG, fmt("  %s just got %s's objects!") %
	  getName() % il.st.owner);

#if 0
  // A nice idea, but the two are now out of synch since the rent header
  // has number of my items plus mob follower's items.
  if (il.st.number != num_read) {
    vlogf(LOG_BUG, fmt("Error while reading %s's objects.  %d in rent file, only %d loaded.") %  getName() % il.st.number % num_read);
    return;
  }
#endif

  // Three hour grace period after crash or autorent. 
  if (!FreeRent && in_room == ROOM_NOWHERE && 
      (il.st.first_update+ 3*SECS_PER_REAL_HOUR > time(0))) {
    vlogf(LOG_PIO, "Character reconnecting inside grace period.");
    sendTo("You connected within the autorent grace period.\n\r");
  } else {
    if (in_room == ROOM_NOWHERE) {
      vlogf(LOG_PIO, "Char reconnecting after autorent");
      applyAutorentPenalties(time(0) - il.st.first_update);
    } else {
      // char was rented
      applyRentBenefits(time(0) - il.st.first_update);
    }

    gone = il.st.original_gold - il.st.gold_left;
    timegold = (int) (((float) ((float) il.st.total_cost/(float) SECS_PER_REAL_DAY)) * (time(0) - il.st.last_update));
    // this is a kludge cuz total is going negative sometimes somehow - Bat 
    if (timegold < 0) {
      vlogf(LOG_BUG,fmt("ERROR: timegold rent charged negative for %s.") % il.st.owner);
      vlogf(LOG_BUG,fmt("ERROR: %s   daily cost: %d timegold: %d") % il.st.owner %il.st.total_cost %timegold);
      vlogf(LOG_BUG,fmt("ERROR: %s   current time: %d, update time: %d") % il.st.owner %time(0) %il.st.last_update);
      vlogf(LOG_BUG,fmt("ERROR: %s   time differential: int: %d") % il.st.owner %(time(0) - il.st.last_update));
      timegold = 0;
    }
    vlogf(LOG_PIO, fmt("%s ran up charges of %d since last update, %d total charges") %  getName() % timegold % (gone + timegold));

    int total_rent=(timegold + gone)>il.st.total_cost?il.st.total_cost:(timegold + gone);
    if (!FreeRent)
      sendTo(fmt("You ran up charges of %d talen%s in rent.\n\r") % total_rent %      (((total_rent) == 1) ? "" : "s"));
    addToMoney(-(total_rent), GOLD_RENT);


   // NOTE:  I realize we can give out gold doing this, but my guess 
   // the only way barring bugs is via timeshifting.   

    il.st.first_update = il.st.last_update = time(0);

    if (getMoney() < 0) {
      addToMoney(points.bankmoney, GOLD_XFER);
      setBank(0);
      // silly last ditch effort
      // it is possible that they have money in a bag, so look
      // for this and make adjustments
      moneyCheck();

      if (getMoney() < 0) {
        vlogf(LOG_PIO, fmt("%s ran out of money in rent") %  getName());
        sendTo("You ran out of money in rent.\n\rSome of your belongings were confiscated.\n\r");

        // Rent will now take items one by one to meet the rent requirements
        // instead of taking all items regardless. - Russ

        // Ideally, we should first take items on followers, then the
        // follower itself here, before doing my items...
        // unfortunately, followers have yet to be loaded at this point.
        // so, we will figure out current rent cost, and compare difference
        // to achieve a delta, which can only be explained by "followers"
        objCost curCost;
        recepOffer(NULL, &curCost);
        int diff = il.st.total_cost - curCost.total_cost;
        if (diff > 0) {
          vlogf(LOG_PIO, fmt("%s had followers taken by rent.") %  getName());
          addToMoney(diff, GOLD_SHOP);

          sprintf(buf, "Your followers, and any items they may have had, were confiscated for %d talens to meet your rent obligations.", diff);
          lbuf += buf;

          removeFollowers();
        }

        while (getMoney() < 0 && (i = findMostExpensiveItem())) {
          amt = i->obj_flags.cost;
          addToMoney(amt, GOLD_SHOP);

          if (i->equippedBy)
            unequip(i->eq_pos);
          else if (i->parent)
            --(*i);

          vlogf(LOG_PIO, fmt("%s had item '%s' taken by rent.") %  getName() % i->getName());
          TThing *t;
          while ((t = i->getStuff())) {
            (*t)--;
            *this += *t;
          }
            
#if RENT_SELL_TO_PAWN
          if (pawnman) {
            *pawnman += *i;
            sprintf(buf, "%s has been sold to %s for %d talens to meet your rent obligations.\n\r", i->getName(.cap()).c_str(), pawnman->getName(), amt);
            lbuf += buf;
          } else {
            vlogf(LOG_BUG, "Pawnman is NULL! Putting rent items in brutius office!");
            thing_to_room(i, 5); 
          }
#else
          // this just clutters up the pawnguy, plus he has to sell a bunch
          // of junk so other people use him as a junkyard.
          sprintf(buf, "%s has been confiscated for %d talens to meet your rent obligations.\n\r", sstring(i->getName()).cap().c_str(), amt);
          lbuf += buf;

          vlogf(LOG_SILENT, fmt("%s's %s being recycled due to rent obligations.") %  
                 getName() % i->getName());
          delete i;
          i = NULL;
#endif
        }
        autoMail(this, NULL, lbuf.c_str());
        if (getMoney() < 0)
          setMoney(0);   //  value of items didn't cover them - Batopr 
      } else {
        sendTo("You didn't have enough cash, but your bank account covered the difference.\n\r");
        sendTo("The remaining bank balance was moved to your character's money.\n\r");
        vlogf(LOG_PIO, fmt("Bank account saved %s from losing items.") %  getName());
      }
    }
  }
  vlogf(LOG_PIO, fmt("Loading %s [%d talens/%d bank/%.2f xps/%d items/%d age-mod/%d rent]") %  
       getName() % getMoney() % getBank() % getExp() % il.st.number % 
       age_mod % il.st.total_cost);

  // silly kludge
  // because of the way the "stuff" list is saved, it essentially reverses
  // its order every reload
  // let's flip the order back...
  TThing *tmp2 = getStuff();
  TThing *t2;
  setStuff(NULL);
  while (tmp2) {
    t2 = tmp2->nextThing;
    tmp2->nextThing = getStuff();
    setStuff(tmp2);
    tmp2 = t2;
  }

  /*
  if (isImmortal()) {
    sprintf(wizbuf, "[%sINTERPORT INFO%s] %s has just connected to port %d.\n\r", cyan(), norm(), getName(), gamePort);
    mudMessage(this, 16, wizbuf); 
  }
  */

  recepOffer(NULL, &cost);
  saveRent(&cost, FALSE, 0);
  return;
}

int TComponent::noteMeForRent(sstring &tStString, TBeing *ch, TThing *tList, int *tCount)
{
  int         tCost    = 0,
              lCount   = 0;
  sstring tString, tBuffer;
  TThing     *tMarker;
  bool        hasPrior = false;
  TComponent *tObj;

  for (tMarker = tList; tMarker; tMarker = tMarker->nextThing) {
    if (tMarker == this)
      break;

    if (!(tObj = dynamic_cast<TComponent *>(tMarker)))
      continue;

    if (isSimilar(tObj) && rentCost() == tObj->rentCost() &&
        isRentable() == tObj->isRentable() &&
        getComponentCharges()    == tObj->getComponentCharges() &&
        getComponentSpell()      == tObj->getComponentSpell()) {
      hasPrior = true;
      break;
    }
  }

  if (hasPrior)
    return 0;

  for (tMarker = nextThing; tMarker; tMarker = tMarker->nextThing) {
    if (!(tObj = dynamic_cast<TComponent *>(tMarker)))
      continue;

    if (isSimilar(tObj) && rentCost() == tObj->rentCost() &&
        isRentable() == tObj->isRentable() &&
        getComponentCharges()    == tObj->getComponentCharges() &&
        getComponentSpell()      == tObj->getComponentSpell()) {
      *tCount = *tCount + 1;
      lCount++;
    }
  }

  tBuffer = fmt("%c-%ds : ") % '%' % (30 + (strlen(getName()) - getNameNOC(ch).length()));

  if (isRentable()) {
    tBuffer+="%5d talens/day";
    *tCount = *tCount + 1;
    lCount++;
    tCost = (max(0, rentCost()) * lCount);
#ifdef FREE_RENT
    if(max_exist > LIMITED_RENT_ITEM) tCost = 0;
#endif
    tString = fmt(tBuffer) % getName() % tCost;
    if (FreeRent) {
      if (lCount == 1)
	tString+="\n\r";
      else {
	tBuffer = fmt("  x%3d\n\r") % lCount;
	tString+=tBuffer;
      }
    } else {
      
      if (lCount == 1)
	tString+="\n\r";
      else {
	tBuffer = fmt("  [%5dx%3d]\n\r") % max(0, rentCost()) % lCount;
	tString+=tBuffer;
      }
    }
    //sprintf(tString, "%-30s : %5d talens/day  [x%3d]\n\r", getName(), tCost, lCount);
    tStString += tString;
  } else {
    tBuffer+="NOT RENTABLE";
    lCount++;
    tString=fmt(tBuffer) % getName();

    if (lCount == 1)
      tString+="\n\r";
    else {
      tBuffer = fmt("      [x%3d]\n\r") % lCount;
      tString+=tBuffer;
    }

    //sprintf(tString, "%-30s : NOT RENTABLE  x%3d\n\r", getName(), lCount);
    tStString += tString;
  }

  return tCost;
}

// (int) return : Cost for this item(block)
// (sstring)     : The running note output sstring.
// (thing)      : The list the item is in, or the item itself.
// (tCount)     : A running count of total items.
int TObj::noteMeForRent(sstring &tStString, TBeing *ch, TThing *, int *tCount)
{
  int  tCost = 0;
  char tString[256],
       tBuffer[256];

  sprintf(tBuffer, "%%-%ds : ", (30 + (strlen(getName()) - strlen(getNameNOC(ch).c_str()))));

  if (isRentable()) {
    if (!FreeRent) 
      strcat(tBuffer, "%5d talens/day\n\r");
    else
      strcat(tBuffer, "\n\r");
   *tCount = *tCount + 1;
    tCost = max(0, rentCost());
#ifdef FREE_RENT
    if(max_exist > LIMITED_RENT_ITEM) tCost = 0;
#endif
    sprintf(tString, tBuffer, getName(), tCost);
    tStString += tString;
  } else {
    strcat(tBuffer, "NOT RENTABLE\n\r");
    sprintf(tString, tBuffer, getName());
    tStString += tString;
  }

  return tCost;
}

void TBeing::makeRentNote(TBeing *recip)
{
  char        buf[1024];
  sstring      longBuf("");
  sstring      tStBuffer("");
  TThing     *t, *t2;
  int         i, temp;
  objCost     cost;
  TObj       *obj  = NULL, *tObj = NULL;
  int         num  = 0;
  followData *f;
  TMonster   *ch;

  cost.total_cost = 0;

  for (t = getStuff(); t; t = t->nextThing)  {
    if (!(obj = dynamic_cast<TObj *>(t)))
      continue;

    cost.total_cost += obj->noteMeForRent(longBuf, this, getStuff(), &num);

    for (t2 = obj->getStuff(); t2; t2 = t2->nextThing) {
      if (!(tObj = dynamic_cast<TObj *>(t2)))
        continue;

      cost.total_cost += tObj->noteMeForRent(longBuf, this, obj->getStuff(), &num);
    }
  }

  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
    if (!(obj = dynamic_cast<TObj *>(equipment[i])))
      continue;

    if (!(((i == WEAR_LEGS_L) && obj->isPaired()) ||
          ((i == WEAR_EX_LEG_L) && obj->isPaired()) ||
          ((i == HOLD_LEFT) && obj->isPaired()))) {
      cost.total_cost += obj->noteMeForRent(longBuf, this, obj, &num);

      for (t = obj->getStuff(); t; t = t->nextThing) {
        if (!(tObj = dynamic_cast<TObj *>(t)))
          continue;

        cost.total_cost += tObj->noteMeForRent(longBuf, this, obj->getStuff(), &num);
      }
    }
  }

  // add up cost for followers
  for (f = followers; f; f = f->next) {
    ch = dynamic_cast<TMonster *>(f->follower);
    if (!ch)
      continue;

    if (!ch->isSaveMob(this))
      continue;

    // don't save if not around
    if (!ch->sameRoom(*this))
      continue;

    temp = ch->petPrice() / 4;
    if (FreeRent) { 
      temp = 0;
      sprintf(buf, "%-30s : Pet/Charm/Thrall/Mount \n\r",
              ch->getName());
    } else {
    sprintf(buf, "%-30s : %5d talens/day ********** Storage fee \n\r",
              ch->getName(), temp);
    }
    longBuf += buf;
    cost.total_cost += temp;

    for (i = MIN_WEAR; i < MAX_WEAR; i++) {
      if (!(obj = dynamic_cast<TObj *>(ch->equipment[i])))
        continue;

      if (!(((i == WEAR_LEGS_L) && obj->isPaired()) ||
           ((i == WEAR_EX_LEG_L) && obj->isPaired()) ||
          ((i == HOLD_LEFT) && obj->isPaired()))) {
        cost.total_cost += obj->noteMeForRent(longBuf, this, obj, &num);

        for (t = obj->getStuff(); t; t = t->nextThing) {
          if (!(tObj = dynamic_cast<TObj *>(t)))
            continue;

          cost.total_cost += tObj->noteMeForRent(longBuf, this, obj->getStuff(), &num);
        }
      }
    }

    for (t = ch->getStuff(); t; t = t->nextThing)  {
      if (!(obj = dynamic_cast<TObj *>(t)))
        continue;
  
      cost.total_cost += obj->noteMeForRent(longBuf, this, getStuff(), &num);
  
      for (t2 = obj->getStuff(); t2; t2 = t2->nextThing) {
        if (!(tObj = dynamic_cast<TObj *>(t2)))
          continue;
  
        cost.total_cost += tObj->noteMeForRent(longBuf, this, obj->getStuff(), &num);
      }
    }
  }

  tStBuffer += "\n\r";
  sprintf(buf, "%d total items.\n\r", num);
  tStBuffer += buf;

  if (!FreeRent) {
#if 1
    
    sprintf(buf, "Total cost is : %d\n\r", cost.total_cost);
    tStBuffer += buf;
    
#else
    
    sprintf(buf, "My storage fee is : %d\n\r", storageFee(this));
    tStBuffer += buf;
    sprintf(buf, "Total cost is : %d\n\r", storageFee(this) + cost.total_cost);
    tStBuffer += buf;
    
#endif
    unsigned int credit = rentCredit();
#ifdef FREE_RENT
    credit = 0;
#endif
    if (desc) {
      if (recip->isImmortal()) {
	//      sprintf(buf, "Minimal Rent Credit is : %d\n\rActual ", credit);
	//tStBuffer += buf;
      }
      desc->best_rent_credit = max(credit, desc->best_rent_credit);
      credit = desc->best_rent_credit;
    }
#ifdef FREE_RENT
    credit = 0;
#endif
    
    //  sprintf(buf, "Rent Credit is : %d\n\r", credit);
    //tStBuffer += buf;
    if (credit >= (unsigned int) cost.total_cost) {
      sprintf(buf, "Daily Rent Cost : 0\n\r");
      tStBuffer += buf;
    } else {
      //sprintf(buf, "Equipment Cost : %d\n\r",
      //           max((int) (cost.total_cost-credit), 0));
      //tStBuffer += buf;
      sprintf(buf, "Current Rent Factor : %.2f\n\r",
	      gold_modifier[GOLD_RENT].getVal());
      tStBuffer += buf;
      sprintf(buf, "Daily Rent Cost : %d\n\r",
	      max((int) ((cost.total_cost-credit)*gold_modifier[GOLD_RENT].getVal()), 0));
      tStBuffer += buf;
      sprintf(buf, "Total Days Rentable: On-Hand: %d / Total: %d\n\r",
	      (int) (getMoney() / ((cost.total_cost - credit) * gold_modifier[GOLD_RENT].getVal())),
	      (int) ((getMoney() + getBank()) / ((cost.total_cost - credit) * gold_modifier[GOLD_RENT].getVal())));
      tStBuffer += buf;
    }
  }
  // semi-weird : we want the note to have the summary, the itemized list,
  // and then the summary again...
  longBuf += tStBuffer;
  tStBuffer += "\n\r";
  tStBuffer += longBuf;

  char *dummy = mud_str_dup(tStBuffer);
  TNote * note = createNote(dummy);
  if (!note) {
    return; 
  }
  note->addObjStat(ITEM_NEWBIE);
  *recip += *note;
  return;
}

int receptionist(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *recep, TObj *o)
{
  objCost cost;
  sh_int save_room;
  char buf[256];
  dirTypeT dir;
  roomDirData *exitp;

  if (cmd == CMD_GENERIC_PULSE){
    TThing *t;
    TBeing *tbt;

    // Toss out idlers
    if(recep->spec==SPEC_RECEPTIONIST){
      // we check the proc, because we have a butler proc for player homes
      // obviously we don't want to toss out people in their homes
      for (t = recep->roomp->getStuff(); t; t = t->nextThing) {
	if ((tbt = dynamic_cast<TBeing *>(t)) &&
	    tbt->getTimer() > 1 && 
	    !tbt->isImmortal()) {
	  if ((tbt->master) && tbt->master->inRoom() == tbt->inRoom()) {
	    // vlogf(LOG_DASH, fmt("saving %s from loitering code, master is %s, room is (%d == %d)") % tbt->getName() %
	    //	tbt->master->getName() % tbt->inRoom() % tbt->master->inRoom());
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
  } else if (cmd == CMD_MOB_MOVED_INTO_ROOM) {
#if 0
// works fine, but how am I supposed to rent my horse?

    if (dynamic_cast<TBeing *>(ch->riding)) {
      sprintf(buf, "Hey, get that damn %s out of my inn!",
          fname(ch->riding->name));
      recep->doSay(buf);
      act("You throw $N out.",
         FALSE, recep, 0, ch, TO_CHAR);
      act("$n throws you out of the inn.",
         FALSE, recep, 0, ch, TO_VICT);
      act("$n throws $N out of the inn.",
         FALSE, recep, 0, ch, TO_NOTVICT);
      --(*ch->riding);
      thing_to_room(ch->riding, (int) o);
      --(*ch);
      thing_to_room(ch, (int) o);
      return TRUE;
    } else if (dynamic_cast<TBeing *>(ch->rider)) {
      --(*ch->rider);
      thing_to_room(ch->rider, (int) o);
      --(*ch);
      thing_to_room(ch, (int) o);
      return TRUE;
    }
#endif
    return FALSE;
  }

  if ((cmd != CMD_RENT) && (cmd != CMD_OFFER))
    return FALSE;

  // force poly's to return
  if (dynamic_cast<TMonster *>(ch)) {
    act("$e looks at you and says 'Sleep in the street!'", FALSE, recep, 0, ch, TO_VICT);
    act("$e looks at $N and says 'Sleep in the street!'", FALSE, recep, 0, ch, TO_NOTVICT);
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
    act("$n motions at you then whispers, \"Someone is after you for the moment and I can not allow you to stay here...Sorry.\"", FALSE, recep, NULL, ch, TO_VICT);
    return TRUE;
  }

#if RENT_RESTRICT_INNS_BY_LEVEL
  // remnant of code that only let high level pc's rent out of grim
  if (recep->GetMaxLevel() < ch->GetMaxLevel()) {
    sprintf(buf,"%s I can only grant rent credit through level %d.",
           ch->getName(),recep->GetMaxLevel());
    recep->doTell(buf);
    sprintf(buf,"%s That's %d talens of credit.",
            ch->getName(),recep->rentCredit());
    recep->doTell(buf);
  }

#endif

  bool   autoHates  = false,
         autoLikes  = false,
         hatesMe[2] = {false, false};
  sstring tStString("");

  for (int tCounter = 0; SIKHates[tCounter].tRace != RACE_NORACE; tCounter++) {
    if (SIKHates[tCounter].tVNum != recep->mobVnum())
      continue;

    if (SIKHates[tCounter].isHate) {
      tStString = SIKHates[tCounter].tStMessage;
      autoLikes = true;

      if (SIKHates[tCounter].tRace == ch->getRace()) {
        tStString = SIKHates[tCounter].tStMessage;
        hatesMe[0] = true;
        hatesMe[1] = true;
      }
    } else {
      autoHates = true;

      if (SIKHates[tCounter].tRace == ch->getRace())
        hatesMe[0] = true;
        hatesMe[1] = false;
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

  if (cmd == CMD_RENT) {
    if (ch->isImmortal()) {
      ch->sendTo(COLOR_BASIC, "<r>WARNING<z>\n\r----------\n\r");
      ch->sendTo("Renting will almost certainly destroy your wizfile.  If you are used to\n\r");
      ch->sendTo("doing this because of mortal life then it's best to get un-used to it.\n\r");
      ch->sendTo("If you Have to rent out, such as testing, then go mortal first.\n\r");
      ch->sendTo("----------\n\r");
    }

    if (ch->recepOffer(recep, &cost)) {
      if (ch->desc && !ch->desc->m_bIsClient) {
        act("$n stores your stuff in the safe, and shows you to your room.", FALSE, recep, 0, ch, TO_VICT);
        act("$n shows $N to $S room.", FALSE, recep, 0, ch, TO_NOTVICT);
    
        dynamic_cast<TPerson *>(ch)->saveRent(&cost, TRUE, 2);
        save_room = ch->in_room;        // backup what room the PC was in 
        ch->saveChar(save_room);
        ch->in_room = save_room;

        ch->cls();
        ch->fullscreen();

        // this delete should not save in_room
        ch->preKillCheck(TRUE);
        ch->desc->outputProcessing();
  
        // we've been rented, notify to destroy ch
        return DELETE_VICT;
      }
    }
  } else if (cmd == CMD_OFFER) {

    // get an offer on someone else...
    one_argument(arg, buf);
    if (ch->isImmortal() && *buf) {
      TBeing *vict = get_pc_world(ch, buf, EXACT_NO);
      if (vict) {
        vict->makeRentNote(ch);
        recep->doTell(ch->getName(), fmt("Here is a note with %s's items listed.") % vict->getName());
        return TRUE;
      }
    }

    ch->makeRentNote(ch);
    recep->doTell(ch->getName(), "Here is a note with your items listed.");
  }
  return TRUE;
}

bool noteLimitedItems(FILE * fp, const char *tag, unsigned char version, bool immortal)
{
  rentObject item;
  int depth = 0;
  char c, *n, *s, *d, *ad;
  signed char slot;

  version = version;

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
        vlogf(LOG_BUG, fmt("BOGUS ITEM #%d found in %s's rent file!") %  
           item.item_number % tag);
        delete [] ad;
        delete [] s;
        continue;
      }

      vlogf(LOG_MISC, fmt("     [%d] - %s%s") %  item.item_number % tag % 
	    (immortal ? "  (immortal)" : ""));
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
      vlogf(LOG_MISC, fmt("  %d - [%d] : max [%d]") % 
           obj_index[i].virt % obj_index[i].getNumber() % obj_index[i].max_exist);
      if (obj_index[i].getNumber() > obj_index[i].max_exist &&
          obj_index[i].max_exist) {
        // latter condition is because DEITY_TOKEN max exist = 0
        char buf[1024];
        sprintf(buf, "Item (%s:%d) is over max (%d).  Num: (%d).\n\r", 
            obj_index[i].name, obj_index[i].virt,
            obj_index[i].max_exist, obj_index[i].getNumber());
	// these have to be lower case
        // autoMail(NULL, "jesus", buf);
        autoMail(NULL, "damescena", buf);

      }
    }
  }
}

void countAccounts(const char *arg) 
{
  DIR *dfd;
  struct dirent *dp;
  int count = 0;
  char buf[128];
  char buf2[256];

  sprintf(buf, "account/%c/%s", LOWER(arg[0]), sstring(arg).lower().c_str());
  if (!(dfd = opendir(buf))) {
    vlogf(LOG_BUG, fmt("bad call to countAccount (%s)") %  buf);
    return;
  }

  bool accountActive7 = false;
  bool accountActive30 = false;

  while ((dp = readdir(dfd)) != NULL) {
    if (!strcmp(dp->d_name, "account") || !strcmp(dp->d_name, "comment") || 
        !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
      continue;
    
    // check for valid char
    sprintf(buf2, "player/%c/%s", dp->d_name[0], dp->d_name);

    struct stat theStat;
    int ret = stat(buf2, &theStat);
    if (ret != 0) {
      // some error occurred
      if (errno == ENOENT) {
        vlogf(LOG_MISC, fmt("Deleting reference to %s in %s's account") %  buf2 % arg);
        sprintf(buf2, "%s/%s", buf, dp->d_name);
        vlogf(LOG_MISC, fmt("Deleting %s") %  buf2);
        if (unlink(buf2) != 0)
          vlogf(LOG_FILE, fmt("error in unlink (12) (%s) %d") %  buf2 % errno);
      } else {
        vlogf(LOG_FILE, fmt("ERROR: stat() failed for %s.  errno = %d") %  buf2 % errno);
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
        accStat.active_account7++;
      }
    }
    if (!accountActive30) {
      if (time(0) - theStat.st_mtime <= (30 * SECS_PER_REAL_DAY)) {
        accountActive30 = true;
        accStat.active_account30++;
      }
    }
  }
  closedir(dfd);

  if (!count) {
    // delete this empty account 

    vlogf(LOG_MISC, fmt("Empty Account: %s, deleting it.") %  buf);
    sprintf(buf2, "account/%c/%s/account", LOWER(arg[0]), sstring(arg).lower().c_str());
    if (unlink(buf2) != 0)
      vlogf(LOG_FILE, fmt("error in unlink (13) (%s) %d") %  buf2 % errno);

    sprintf(buf2, "account/%c/%s/comment", LOWER(arg[0]), sstring(arg).lower().c_str());
    unlink(buf2);  // probably doesn't exist, so no error...

    if (rmdir(buf) != 0)
      vlogf(LOG_FILE, fmt("error in rmdir (%s) %d") %  buf % errno);

    return;
  }
  // each time this is called, we have another account
  // no need to really do anything, besides just count number of calls
  accStat.account_number++;
}

static void deleteDuringRead(TMonster *mob)
{
  // we read the act bits early in the read, but don't alter the sstrings
  // until the end of the read, so this is a good idea
  REMOVE_BIT(mob->specials.act, ACT_STRINGS_CHANGED);

  delete mob;
}

static void parseFollowerRent(FILE *fp, TBeing *ch, const char *arg)
{
  TMonster *mob;
  int tmp = 0;
  int tmp2 = 0;
  int i = 0, num = 0;
  statTypeT iStat;
  float att;
  unsigned char version;
  TObj *new_obj;
  int rc;

  bool fp2_open = false;
  FILE *fp2 = NULL;
  while (fscanf(fp, "#%d\n", &num) == 1) {
    if (!(mob = read_mobile(num, VIRTUAL))) {
      vlogf(LOG_BUG, fmt("Error loading mob %d in loadFollower") %  num);
      break;
    }
    // Since this mob was in rent, don't double count it.
    mob_index[mob->getMobIndex()].addToNumber(-1);

    if (fscanf(fp, "%d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (1)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->specials.act = tmp;

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (2)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->specials.affectedBy = tmp;
  
    // technically, we should check for AFF_SANCT here
    // if it had natural sanct, it got set in readMobile
    // if it had been cast, it should be added by the affections loop below

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (3)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mud_assert(tmp >= MIN_FACTION && tmp < MAX_FACTIONS, "bad value");
    mob->setFaction(factionTypeT(tmp));

    if (fscanf(fp, " %f ", &att) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (4)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setPerc((double) att);

    if (fscanf(fp, " %f ", &att) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (5)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setMult((double) att);
  
    fscanf(fp, "\n");
  
    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (6)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setClass(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (7)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->fixLevels(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (8)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setHitroll(tmp);
  
    if (fscanf(fp, " %f ", &att) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (9)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
// this is for old-mob handling
float old_ac_lev = mob->getACLevel();
    mob->setACLevel(att);
    mob->setACFromACLevel();

    // we will let HP Level be whatever the tiny mob is, and just set
    // the actual and max here
    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (10)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setHit(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (11)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setMaxHit(tmp);
  
    rc = fscanf(fp, " %f+%d \n", &att, &tmp);
    if (rc == 1) {
      vlogf(LOG_BUG, fmt("Old style mob found in %s's rent") %  
	    (ch ? ch->getName() : "Unknown"));
      // first, correct the file pointer so reading works ok
      rc = fscanf(fp, "d%d+%d", &tmp, &tmp2);
      if (rc != 2) {
        vlogf(LOG_BUG, "Unable to fix old-style mob in rent.");
        deleteDuringRead(mob);
        break;
      }
      // leave the damage what it is on the mob now
      // HP should be ok since we are saving raw values, not levels
      // fix AC though
      mob->setACLevel(old_ac_lev);
      mob->setACFromACLevel();

    } else if (rc != 2) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (12)") %  arg % num);
      deleteDuringRead(mob);
      break;
    } else {
      mob->setDamLevel(att);
      mob->setDamPrecision(tmp);
    }

    mob->setLifeforce(9000);
    mob->setMana(mob->manaLimit());
    mob->setPiety(mob->pietyLimit());
    mob->setMove(mob->moveLimit());

    //    mob->setMaxMana(10);
    //    mob->setMaxMove(50);
  
    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (13)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setMoney(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (14)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setExp(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (14)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setMaxExp(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (15)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setRace(race_t(tmp));

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (16)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setWeight(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (17)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setHeight(tmp);

    for (iStat=MIN_STAT;iStat<MAX_STATS_USED;iStat++) {
      fscanf(fp, " %d ", &tmp);
      mob->setStat(STAT_CHOSEN, iStat, tmp);
    }

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (24)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setPosition(mapFileToPos(tmp));

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (25)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->default_pos = mapFileToPos(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (26)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->setSexUnsafe(tmp);

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (27)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    mob->spec = tmp;

    immuneTypeT ij;
    for (ij=MIN_IMMUNE; ij < MAX_IMMUNES; ij++) {
      if (fscanf(fp, " %d ", &tmp) != 1) {
        vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (28)") %  arg % num);
        deleteDuringRead(mob);
        break;
      }
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
      if (fscanf(fp, " %d ", &tmp) != 1) {
        vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (29)") %  arg % num);
        deleteDuringRead(mob);
        break;
      }
      af.type = mapFileToSpellnum(tmp);

      if (fscanf(fp, " %d ", &tmp) != 1) {
        vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (30)") %  arg % num);
        deleteDuringRead(mob);
        break;
      }
      af.level = tmp;

      if (fscanf(fp, " %d ", &tmp) != 1) {
        vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (31)") %  arg % num);
        deleteDuringRead(mob);
        break;
      }
      af.duration = tmp;

      if (fscanf(fp, " %d ", &tmp) != 1) {
        vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (32)") %  arg % num);
        deleteDuringRead(mob);
        break;
      }
      // we can't set this just yet, need to know the location stuff
      int raw_modifier = tmp;

      if (fscanf(fp, " %d ", &tmp) != 1) {
        vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (33)") %  arg % num);
        deleteDuringRead(mob);
        break;
      }
      af.modifier2 = tmp;

      if (fscanf(fp, " %d ", &tmp) != 1) {
        vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (34)") %  arg % num);
        deleteDuringRead(mob);
        break;
      }
      af.location = mapFileToApply(tmp);

      if (applyTypeShouldBeSpellnum(af.location))
        af.modifier = mapFileToSpellnum(raw_modifier);
      else
        af.modifier = raw_modifier;
      if (fscanf(fp, " %d ", &tmp) != 1) {
        vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (35)") %  arg % num);
        deleteDuringRead(mob);
        break;
      }
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
      if (fscanf(fp, " %d ", &tmp) != 1) {
        vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (36)") %  arg % num);
        deleteDuringRead(mob);
        break;
      }
      mob->setLimbFlags(mapped_slot, tmp);

      if (fscanf(fp, " %d ", &tmp) != 1) {
        vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (37)") %  arg % num);
        deleteDuringRead(mob);
        break;
      }
      mob->setCurLimbHealth(mapped_slot, tmp);

      if (fscanf(fp, " %d ", &tmp) != 1) {
        vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (37b)") %  arg % num);
        deleteDuringRead(mob);
        break;
      }
      version = tmp;
      if (tmp != -1 && fp2_open == false) {
        char buf[256];
        sprintf(buf, "rent/%c/%s.fr", LOWER(arg[0]), sstring(arg).lower().c_str());
        if (!(fp2 = fopen(buf, "r+b"))) 
          break;
        fp2_open = true;
      }
      ItemLoad il;
      il.setFile(fp2);
      il.setVersion(version);
      if (tmp != -1 && (new_obj = il.raw_read_item())) {
        if (ch) {
          vlogf(LOG_SILENT, fmt("%s's %s rent-retrieving: (%s : %d)") %  arg % mob->getName() % new_obj->getName() % new_obj->objVnum());
          mob->equipChar(new_obj, mapped_slot, SILENT_YES);
        } else {
          // count the item
          // we want to add 1 to count the item, and another 1 because
          // the delete will reduce the number, add an additional one
          if ((new_obj->number >= 0)) {
            obj_index[new_obj->getItemIndex()].addToNumber(2);
	    
            vlogf(LOG_PIO, fmt("     [%d] - in %s's follower rent") %  
                     new_obj->objVnum() % arg);
          }
          delete new_obj;
        }
      }
      mob->setStuckIn(mapped_slot, NULL);
    }

    if (fscanf(fp, " %d ", &tmp) != 1) {
      vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (37c)") %  arg % num);
      deleteDuringRead(mob);
      break;
    }
    while (tmp != -1) {
      version = tmp;
      if (fp2_open == false) {
        char buf[256];
        sprintf(buf, "rent/%c/%s.fr", LOWER(arg[0]), sstring(arg).lower().c_str());
        if (!(fp2 = fopen(buf, "r+b"))) 
          break;
        fp2_open = true;
      }
      ItemLoad il;
      il.setFile(fp2);
      il.setVersion(version);
      if ((new_obj = il.raw_read_item())) {
        if (ch) {
          vlogf(LOG_SILENT, fmt("%s's %s rent-retrieving: (%s : %d)") %  arg % mob->getName() % new_obj->getName() % new_obj->objVnum());
          *mob += *new_obj;
        } else {
          // count the item
          // we want to add 1 to count the item, and another 1 because
          // the delete will reduce the number, add an additional one
          if ((new_obj->number >= 0)) {
            obj_index[new_obj->getItemIndex()].addToNumber(2);
            vlogf(LOG_PIO, fmt("     [%d] - in %s's follower rent") %  
                     new_obj->objVnum() % arg);
          }
          delete new_obj;
        }
      }
      if (fscanf(fp, " %d ", &tmp) != 1) {
        vlogf(LOG_BUG, fmt("Error reading follower data (%s mobs %d) (37d)") %  arg % num);
        deleteDuringRead(mob);
        break;
      }
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
      vlogf(LOG_SILENT, fmt("%s mobile-rent retrieving %s") %  ch->getName() % mob->getName());

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
            (aff->type == AFFECT_ORPHAN_PET)) {
          char * tmp = mud_str_dup(ch->name);
          aff->be = (TThing *) tmp;
        }
      }

    } else {
      // we are just logging
      // handle the count for this mob.
      // we are adding 1 to count the mob, and another one to offset the
      // decrease that will happen in delete.
      mob_index[mob->getMobIndex()].addToNumber(2);

      vlogf(LOG_PIO, fmt("     [%d] - mobile (%s) owned by %s") % 
                     mob->mobVnum() % mob->getName() % arg);
      thing_to_room(mob, ROOM_VOID);
      delete mob;
    }
  }
  if (fp2_open)
    fclose(fp2);
}

// this routine simply tracks items and mobs held in rent by players.
// the cost of such items and mobs is already calculated in the person's
// rent, and that has alread been calculated by chargeRent().
// our main goal here is just to keep up with the max_exists and such for
// mobile rent
static void chargeMobileRent(const char *who)
{
  char fileName[256];
  FILE *fp;

  sprintf(fileName, "rent/%c/%s.fol", who[0], who);

  if (!(fp = fopen(fileName, "r"))) {
    // no follower info
    return;
  }

  parseFollowerRent(fp, NULL, who);

  fclose(fp);
  return;
}

void chargeRent(const char *who)
{
  char fileName[128];
  rentHeader h;
  long days_passed, secs_lost, total;
  FILE *fp, *afp;
  charFile pd;
  accountFile acc;
  bool immortal;

  mud_assert(who != NULL, "chargeRent called with NULL player name!");

  sprintf(fileName, "rent/%c/%s", who[0], who);

  // skip followers data
  if (strlen(fileName) > 4 && !strcmp(&fileName[strlen(fileName) - 4], ".fol"))
    return;
  if (strlen(fileName) > 3 && !strcmp(&fileName[strlen(fileName) - 3], ".fr"))
    return;

  if (!(fp = fopen(fileName, "r+b"))) {
    vlogf(LOG_BUG, fmt("Error opening %s's rent file!") %  who);
    return;
  }
  memset(&h, 0, sizeof(rentHeader));
  if ((fread(&h, sizeof(rentHeader), 1, fp)) != 1) {
    vlogf(LOG_BUG, fmt("  Cannot read rent file header for %s") %  who);
    fclose(fp);
    return;
  }
#if 1
  if (sstring(h.owner).lower().compare(who)) {
    vlogf(LOG_BUG, fmt("WARNING!  rent file %s holds objects for %s!") %  who % h.owner);
    fclose(fp);
    return;
  }
#else
  // a debug case for if we copied someone's rent file for problem resolution
  strcpy(h.owner, who);
#endif
  if (!load_char(who, &pd)) {
    vlogf(LOG_BUG, fmt("Unable to read player file for %s, so deleting rent file.") %  h.owner);
    fclose(fp);
    wipeRentFile(who);
    //removeFollowers();
    return;
  }

  sprintf(fileName, "account/%c/%s/account", LOWER(pd.aname[0]), 
        sstring(pd.aname).lower().c_str());
  if (!(afp = fopen(fileName, "r"))) {
    vlogf(LOG_FILE, fmt("Error opening %s's account file!") %  who);
    fclose(fp);
    return;
  }
  if ((fread(&acc, sizeof(accountFile), 1, afp)) != 1) {
    vlogf(LOG_BUG, fmt("  Cannot read account file for %s") %  who);
    fclose(afp);
    fclose(fp);
    return;
  }
  fclose(afp);
  immortal = IS_SET(acc.flags, ACCOUNT_IMMORTAL);

  days_passed = ((time(0) - h.last_update) / SECS_PER_REAL_DAY);
  secs_lost = ((time(0) - h.last_update) % SECS_PER_REAL_DAY);
  if (pd.load_room == ROOM_AUTO_RENT) {        // this person was autorented 
    pd.load_room = ROOM_NOWHERE;
    h.last_update = time(0);
    if (!noteLimitedItems(fp, who, h.version, immortal)) {
      vlogf(LOG_BUG, fmt("cannot count (1) limited items for %s") %  h.owner);
      fclose(fp);
      return;
    }
    rewind(fp);
    if (fwrite(&h, sizeof(rentHeader), 1, fp) != 1) {
      vlogf(LOG_BUG, fmt("Cannot write updated rent file header for %s") %  h.owner);
      fclose(fp);
      return;
    }
    fclose(fp);
    if (!raw_save_char(who, &pd)) {
      vlogf(LOG_BUG, fmt("Error updating player-file entry for %s in chargeRent.") %  h.owner);
      return;
    }
    vlogf(LOG_PIO, fmt("   De-autorented %s") %  h.owner);
  } else {   // this person was rented as normal 
    total = (h.total_cost * days_passed);

#if 1
    if (!noteLimitedItems(fp, who, h.version, immortal)) {
      vlogf(LOG_BUG, fmt("cannot count (2) limited items for %s") %  h.owner);
      fclose(fp);
      return;
    }
    fclose(fp);
#else


    // note, use of float required
    // 20K rent/day * 80K secs/day > max(long)
    total += (int) (h.total_cost * ((float) secs_lost / (float) SECS_PER_REAL_DAY));

    // this is a kludge cuz total is going negative sometimes somehow - Bat 
    if (total < 0) {
      total = -total;  
      vlogf(LOG_BUG,fmt("ERROR: total rent charged negative for %s.") % h.owner);
      vlogf(LOG_BUG,fmt("ERROR: %s   daily cost: %d    days: %d    secs: %d    total: %d") % h.owner %h.total_cost %days_passed %secs_lost %-total);
    }

    if (total > (h.gold_left + pd.bankmoney)) {
      vlogf(LOG_MISC, fmt("   %s will run out of money on login.") %  h.owner);
      vlogf(LOG_MISC, fmt("   %s had %d total cost and %d gold left(including bank)") % 
                h.owner % total % (h.gold_left + pd.bankmoney));
      pd.money = 0;
      pd.bankmoney = 0;
      pd.load_room = ROOM_NOWHERE;
      if (!noteLimitedItems(fp, who, h.version, immortal)) {
        vlogf(LOG_BUG, fmt("cannot count (2) limited items for %s") %  h.owner);
        fclose(fp);
        return;
      }
      fclose(fp);
      if (!raw_save_char(who, &pd)) {
        vlogf(LOG_BUG, fmt("Error updating player-file entry for %s in chargeRent.") %  h.owner);
        return;
      }
    } else {
      if (!noteLimitedItems(fp, who, h.version, immortal)) {
        vlogf(LOG_BUG, fmt("cannot count (3) limited items for %s") %  h.owner);
        fclose(fp);
        return;
      }
      h.gold_left -= total;
      h.last_update = time(0);
      rewind(fp);
      if (fwrite(&h, sizeof(rentHeader), 1, fp) != 1) {
        vlogf(LOG_BUG, fmt("Cannot write updated rent file header for %s") %  h.owner);
        fclose(fp);
        return;
      }
      fclose(fp);
#if 0
// redundant
      if (!days_passed)
        vlogf(LOG_SILENT, fmt("   same day %d coin update of %s") %  total % h.owner);
#endif

      vlogf(LOG_PIO, fmt("   charged %s %d talens rent.  (%d per day)(left: %d)(bank: %d)") %  h.owner % total % h.total_cost % h.gold_left % pd.bankmoney);
    }
#endif
  }
  chargeMobileRent(who);
}

void updateRentFiles(void)
{
  bootPulse(".", false);
  dirwalk("rent/a", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/b", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/c", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/d", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/e", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/f", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/g", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/h", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/i", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/j", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/k", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/l", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/m", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/n", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/o", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/p", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/q", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/r", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/s", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/t", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/u", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/v", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/w", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/x", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/y", chargeRent);
  bootPulse(".", false);
  dirwalk("rent/z", chargeRent);
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

  wipeFollowersFile(tmp->name);

  return;
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

  return TRUE;
}

bool TBeing::saveFollowers(bool rent_time)
{
  TMonster *mob;
  TBeing *ch;
  TThing *t, *t2;
  followData *f, *f2;
  char buf[256];
  FILE *fp;
  int i;
  TPerson *tmp;
  char temp[4096];
  int j, k;
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
    wipeFollowersFile(tmp->name);
    return FALSE;
  }
  if (!(fp = fopen(buf, "w"))) {
    vlogf(LOG_FILE, fmt("Can't open follower file for %s!") %  tmp->name);
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
    fprintf(fp, "%ld %ld %d %.1f %.1f\n",
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

    fprintf(fp, "%d %d %d\n", mob->getMaterial(), mob->canBeSeen, mob->visionBonus);

    // store affects to preserve charm, etc
    affectedData *af;
    for (af = a_list, i = 0; i < MAX_AFFECT; i++) {
      if (af) {
        fprintf(fp, "%d %d %d %ld %ld %d %ld\n",
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
            vlogf(LOG_BUG, fmt("Error opening %'s follower rent file for write.") %  tmp->name);
          } else 
            fp2_open = true;
        }
	
	is.setFile(fp2);
        if (fp2_open)
          is.raw_write_item(obj);

        if (rent_time) {
          vlogf(LOG_SILENT, fmt("%s's %s renting: (%s : %d)") %  getName() % mob->getName() % obj->getName() % obj->objVnum());


          delete obj;
          char_eq[mapped_slot] = NULL;
        } else {
          mob->equipChar(obj, mapped_slot, SILENT_YES);
        }
      }
    }
    for (t = mob->getStuff(); t ; t = t2) {
      t2 = t->nextThing;
      obj = dynamic_cast<TObj *>(t);
      if (!obj)
        continue;
      fprintf(fp, " %d\n", version);

      if (!fp2_open) {
        sprintf(buf, "rent/%c/%s.fr", LOWER(tmp->name[0]), sstring(tmp->name).lower().c_str());
        if (!(fp2 = fopen(buf, "w+b"))) {
          vlogf(LOG_BUG, fmt("Error opening %'s follower rent file for write.") %  tmp->name);
        } else
          fp2_open = true;
      }
        
      is.setFile(fp2);
      if (fp2_open)
        is.raw_write_item(obj);

      if (rent_time) {
        vlogf(LOG_SILENT, fmt("%s's %s renting: (%s : %d)") %  getName() % mob->getName() % obj->getName() % obj->objVnum());

        delete obj;
      }
    }
    fprintf(fp, " -1\n");   // stuff terminator

    // save strung mob sstrings
    if (IS_SET(mob->specials.act, ACT_STRINGS_CHANGED)) {
      for (j = 0, k = 0; k <= (int) strlen(mob->name); k++) {
        if (mob->name[k] != 13)
          temp[j++] = mob->name[k];
      }
      temp[j] = '\0';
      fprintf(fp, "%s~\n", temp);

      for (j = 0, k = 0; k <= (int) strlen(mob->shortDescr); k++) {
        if (mob->shortDescr[k] != 13)
          temp[j++] = mob->shortDescr[k];
      }
      temp[j] = '\0';
      fprintf(fp, "%s~\n", temp);

      for (j = 0, k = 0; k <= (int) strlen(mob->getLongDesc()); k++) {
        if (mob->getLongDesc()[k] != 13)
          temp[j++] = mob->getLongDesc()[k];
      }
      temp[j] = '\0';
      fprintf(fp, "%s~\n", temp);

      for (j = 0, k = 0; k <= (int) strlen(mob->getDescr()); k++) {
        if (mob->getDescr()[k] != 13)
          temp[j++] = mob->getDescr()[k];
      }
      temp[j] = '\0';
      fprintf(fp, "%s~\n", temp);
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
#if 0
    // we reequipped when we saved objs
    // and reequip...
    for (ij = MIN_WEAR; ij < MAX_WEAR; ij++) {
      if (char_eq[ij])
        mob->equipChar(char_eq[ij], ij, SILENT_YES);
    }
#endif

    if (rent_time) {
      act("$n is led off to a storage area.", FALSE, mob, 0, 0, TO_ROOM);
      vlogf(LOG_SILENT, fmt("%s mobile-renting %s") %  getName() % mob->getName());

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

  // don't close the follower rent file until after all followers written
  if (fp2_open)
    fclose(fp2);

  if (!found) {
    wipeFollowersFile(tmp->name);
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
  
  parseFollowerRent(fp, this, tmpPer->name);

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
  return;
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
  if (!argument.empty()) {
    if (is_abbrev(argument, "credit")) {
      int lev;
      sstring sb;
      char buf[256];

      if (FreeRent) {
	sb = "Rent is free! Who needs credit?\n\r";
      } else {

	sb = "Rent Credit by level for [";
	sb += getProfAbbrevName();
	sb += "]:\n\r\n\r";
	
	for (lev = 1; lev <= MAX_MORT/4 + 1; lev++) {
	  int lev0 = lev;
	  int lev1 = lev0 + 1*(MAX_MORT/4 +1);
	  int lev2 = lev0 + 2*(MAX_MORT/4 +1);
	  int lev3 = lev0 + 3*(MAX_MORT/4 +1);
	  
	  if (lev0 <= MAX_MORT) {
	    sprintf(buf, "%s[%2d]%s %s%10d%s ",
		    cyan(), lev0, norm(), orange(),
		    rent_credit(getClass(), lev0, howManyClasses()),
		    norm());
	    sb += buf;
	  }
	  if (lev1 <= MAX_MORT) {
	    sprintf(buf, "%s[%2d]%s %s%10d%s ",
		    cyan(), lev1, norm(), orange(),
		    rent_credit(getClass(), lev1, howManyClasses()),
		    norm());
	    sb += buf;
	  }
	  if (lev2 <= MAX_MORT) {
	    sprintf(buf, "%s[%2d]%s %s%10d%s ",
		    cyan(), lev2, norm(), orange(),
		    rent_credit(getClass(), lev2, howManyClasses()),
		    norm());
	    sb += buf;
	  }
	  if (lev3 <= MAX_MORT) {
	    sprintf(buf, "%s[%2d]%s %s%10d%s\n\r",
		    cyan(), lev3, norm(), orange(),
		    rent_credit(getClass(), lev3, howManyClasses()),
		    norm());
	    sb += buf;
	  } else {
	    sb += "\n\r";
	  }
	}
	sb += "\n\r";
      }
      if (desc)
        desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
      return TRUE;
    } else {
      sendTo("Syntax: rent credit\n\r");
      return FALSE;
    }
    return FALSE;
  }

  objCost cost;
  sh_int save_room;

  // note this is sort of a special case
  // special procedures (innkeeper, personalHouse) have already been
  // taken care of before this point

  // what we really want to do is let people rent out from camping

  if (!inCamp()) {
    doNotHere();
    return FALSE;
  }
  recepOffer(this, &cost);
  sendTo("You opt to rough it for awhile.\n\r");
  act("$n decides to rough it for awhile.",
       TRUE, this, 0, 0, TO_ROOM);

  cls();
  fullscreen();

  dynamic_cast<TPerson *>(this)->saveRent(&cost, TRUE, 2);
  save_room = inRoom();  /* backup what room the PC was in */
  saveChar(save_room);
  in_room = save_room;
 
  preKillCheck(TRUE);

  return DELETE_THIS;
}

int TObj::rentCost() const
{
  if (FreeRent) return 0;
  return obj_flags.cost / 2 + obj_flags.cost % 2;
}

int TWand::rentCost() const
{
  if (FreeRent) return 0;
  int num = TMagicItem::rentCost();

  num *= getCurCharges();
  num /= max(1, getMaxCharges());
  return num;
}

int TMoney::rentCost() const
{
  // talens in a bag shouldn't cost anything to rent
  return 0;
}

bool TObj::isRentable() const
{
  if (isObjStat(ITEM_NORENT) || (number < 0))
    return FALSE;
  return TRUE;
}

objCost::objCost() :
  total_cost(0),
  no_carried(0),
  lowrentobjs(0),
  ok(0)
{
}

objCost::~objCost() 
{
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
  original_gold(0),
  gold_left(0),
  total_cost(0),
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
    vlogf(LOG_FILE, fmt("  Error opening the corpse file for corpse %s") %  cfName);
    return;
  }
  if ((fread(&h, sizeof(rentHeader), 1, fp)) != 1) {
    vlogf(LOG_BUG, fmt("Error reading version from %s.") %  fileName);
    fclose(fp);
    return;
  }

  if (!noteLimitedItems(fp, fileName, h.version, FALSE))
    vlogf(LOG_BUG, fmt("  Unable to count limited items in file  %s") %  fileName);

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

  for(ci = 0;ci <= RANGER_LEVEL_IND;ci++) {
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

  delete [] mob->name;
  mob->name = mud_str_dup(fmt("%s [clone]") % st1.name);

  delete [] mob->shortDescr;
  mob->shortDescr = mud_str_dup(st1.name);

  delete [] mob->player.longDescr;
  mob->player.longDescr = mud_str_dup(fmt("<c>%s<1> is standing here.") % st1.name);

  delete [] mob->getDescr();
  if(*st1.description)
    mob->setDescr(mud_str_dup(fmt("%s\n\r") % st1.description));
  else 
    mob->setDescr(mud_str_dup("You see nothing special about him.\n\r"));
  
  mob->setSex(sexTypeT(st1.sex));
  mob->setHeight(st1.height);
  mob->setWeight(st1.weight);
  mob->setRace(race_t(st1.race));
  
  
  // open player rent file
  sstring buf = fmt ("rent/%c/%s") % LOWER(ch_name[0]) % ch_name.lower();
  if (!il.openFile(buf)){
    sendTo("Rent file could not be opened.  Your clone stands naked before you.\n\r");
    return;
  }
  if(!il.readHeader()){
    vlogf(LOG_BUG, fmt("Error while reading %s's rent file header.  Your clone stands naked before you.") %  ch_name);
    return;
  }
  if (il.objsFromStore(NULL, &num_read, mob, NULL, false)) {
    vlogf(LOG_BUG, fmt("Error while reading %s's objects in doClone.  Your clone stands naked before you.") %  ch_name);
    return;
  }
  
  // add NO RENT to the objects, don't want them falling into PC 
  //   hands permanently
  // ALSO - junk notes, and increase object number for these loads (since
  //   they will decrease when purged)
  wearSlotT ij;
  TObj *o;
  for (ij = MIN_WEAR; ij < MAX_WEAR; ij++){
    if((o = dynamic_cast<TObj *>(mob->equipment[ij]))) 
    {
      obj_index[o->getItemIndex()].addToNumber(1);
      o->addObjStat(ITEM_NORENT);
      if ((dynamic_cast<TNote *>(o)))
      {
        o->makeScraps();
        delete o;
      }
    } else if (mob->equipment[ij]) 
      vlogf(LOG_BUG, fmt("did not add no-rent flag to item %s in slot %d when cloning") % mob->equipment[ij]->name % (int) ij);
  }
  
  TThing *i, *j, *tmp;
  TObj *bo;
//  TBaseContainer *b1;
  
  for (i = mob->getStuff(); i; i=i)
  {
    if ((o = dynamic_cast<TObj *>(i)))
    {
      obj_index[o->getItemIndex()].addToNumber(1);
      o->addObjStat(ITEM_NORENT);
      if ((dynamic_cast<TNote *>(o)))
      {
        i = o->nextThing;
        o->makeScraps();
        delete o;
        continue;
      }
    } else 
      vlogf(LOG_BUG, fmt("did not add no-rent flag to %s when cloning") %  
	    i->name);
    
    if ((dynamic_cast<TBaseContainer *>(i))) {
      for (j = i->getStuff(); j; j=j) {
        if ((bo = dynamic_cast<TObj *>(j))) 
        {
          obj_index[bo->getItemIndex()].addToNumber(1);
          bo->addObjStat(ITEM_NORENT);
        } else 
          vlogf(LOG_BUG, fmt("did not add no-rent flag to %s when cloning") %  
	          bo->name);
        
        if ((dynamic_cast<TNote *>(j))) 
        {
          tmp = j->nextThing;
          j->makeScraps();
          delete j;
          j = tmp;
          continue;
        }
        j = j->nextThing;
      }
    }
    i = i->nextThing;
  }

  // this bit makes the mob TRUE for isPc, and prevents the look responses, etc
  
  sendTo("Your clone appears before you.\n\r");
  return;
}

