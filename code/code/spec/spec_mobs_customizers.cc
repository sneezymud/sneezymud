#include "extern.h"
#include "handler.h"
#include "being.h"
#include "room.h"
#include "low.h"
#include "monster.h"
#include "person.h"
#include "materials.h"
#include "obj_symbol.h"
#include "obj_window.h"
#include "obj_general_weapon.h"
#include "obj_base_clothing.h"
#include "obj_bow.h"
#include "obj_trap.h"
#include "obj_table.h"
#include "obj_drinkcon.h"
#include "corporation.h"
#include "shopowned.h"
#include "pathfinder.h"
#include "shop.h"
#include "obj_base_corpse.h"
#include "obj_player_corpse.h"
#include "obj_tool.h"
#include "obj_plant.h"
#include "obj_note.h"
#include "obj_commodity.h"
#include "obj_component.h"
#include "obj_food.h"
#include "spec_mobs.h"

extern int kick_mobs_from_shop(TMonster *myself, TBeing *ch, int from_room);


static int engraveCost(TObj *obj, TBeing *ch, unsigned int shop_nr)
{
  double cost;

  cost = obj->obj_flags.cost;

  cost *= max(1,obj->obj_flags.cost/10000);

  if(shop_nr)
    cost *= shop_index[shop_nr].getProfitBuy(obj, ch);

  return (int) cost;
}


int engraver(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  return customizer(ch, cmd, arg, me, o, customizerType::TYPE_ENGRAVER);
}

int tailor(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  return customizer(ch, cmd, arg, me, o, customizerType::TYPE_TAILOR);
}

int blacksmith(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  return customizer(ch, cmd, arg, me, o, customizerType::TYPE_BLACKSMITH);
}

// Generic customization
int customizer(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o, customizerType custtype)
{
  sstring buf;
  TObj *item;
  TBeing *final_pers;
  int cost;
  dirTypeT dir = DIR_NONE;
  roomDirData *exitp;
  int rc;
  TBeing *tbt = NULL;
  TThing *ttt;

  class reg_struct {
    public:
    int wait;
    int cost;
    char *char_name;
    char *obj_name;
    reg_struct() :
      wait(0),
      cost(0),
      char_name(NULL),
      obj_name(NULL)
    {
    }
    ~reg_struct() {
      delete [] char_name;
      char_name = NULL;
      delete [] obj_name;
      obj_name = NULL;
    }
  };
  static reg_struct *job;

  switch (cmd) {
    case CMD_WHISPER:
      return shopWhisper(ch, me, find_shop_nr(me->number), arg);
    case CMD_GENERIC_DESTROYED:
      delete (reg_struct *) me->act_ptr;
      me->act_ptr = NULL;
      return FALSE;
    case CMD_GENERIC_CREATED:
      if (!(me->act_ptr = new reg_struct())) {
        perror("failed new of customizer.");
        exit(0);
      }
      return FALSE;
    case CMD_MOB_MOVED_INTO_ROOM:

        return kick_mobs_from_shop(me, ch, (long int)o);

    case CMD_MOB_VIOLENCE_PEACEFUL:
      ttt = o;
      tbt = dynamic_cast<TBeing *>(ttt);
      me->doSay("Hey!  Take it outside.");
      for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
        if (exit_ok(exitp = me->exitDir(dir), NULL)) {
          act("$n throws you from $s shop.",
                 FALSE, me, 0, ch, TO_VICT);
          act("$n throws $N from $s shop.",
                 FALSE, me, 0, ch, TO_NOTVICT);
          me->throwChar(ch, dir, FALSE, SILENT_NO, true);
          act("$n throws you from $s shop.",
                 FALSE, me, 0, tbt, TO_VICT);
          act("$n throws $N from $s shop.",
                 FALSE, me, 0, tbt, TO_NOTVICT);
          me->throwChar(tbt, dir, FALSE, SILENT_NO, true);
          return TRUE;
        }
      }
      return TRUE;
    case CMD_GENERIC_PULSE:
      if (!(job = (reg_struct *) me->act_ptr)) {
        vlogf(LOG_PROC,"CUSTOMIZER PROC ERROR: terminating (hopefully)");
        return FALSE;
      }
      if (!job->char_name || !job->obj_name)
        return FALSE;

      if (job->wait > 0) {
        job->wait--;
        if (!job->wait) {
          buf = format("That should do it %s!") % job->char_name;
          me->doSay(buf);
          TThing *ts = NULL;
          TObj *final = NULL;
          if (!(ts = searchLinkedList(job->obj_name, me->stuff)) ||
              !(final = dynamic_cast<TObj *>(ts))) {
            me->doSay("Ack, I lost the item somehow! Tell a god immediately!  ");
            vlogf(LOG_PROC,format("customizer lost customizing item (%s)") %
                (final ? final->name : ""));
            return FALSE;
          }

          if (!(final_pers = get_char_room(job->char_name, me->in_room))) {
            me->doSay("Hmm, I seem to have lost the person I was customizing for.");
            me->doSay("Well I can't customize this now.");
            me->doSay("Hopefully they come back for this.");
            rc = me->doDrop(job->obj_name, NULL);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
            return FALSE;
          }

          final->swapToStrung();

          // resize if required
          if (custtype == TYPE_TAILOR || custtype == TYPE_BLACKSMITH) {
            wearSlotT slot = slot_from_bit(final->obj_flags.wear_flags);
            if(race_vol_constants[mapSlotToFile(slot)]) {
              final->setVolume((int)((double)final_pers->getHeight() * race_vol_constants[mapSlotToFile(slot)]));
            }
          }


          //  Remake the obj name.
          buf = format("%s %s") % final->name.c_str() % job->char_name;
          final->name = buf;

          buf = format("This is the personalized object of %s") % job->char_name;
          final->action_description = buf;


          if ((final_pers->getCarriedVolume() + final->getTotalVolume())  > final_pers->carryVolumeLimit()) {
            me->doSay("You can't carry it! I'll just drop it here for you!");
            rc = me->doDrop(job->obj_name, NULL);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
            delete [] job->char_name;
            job->char_name = NULL;
            delete [] job->obj_name;
            job->obj_name = NULL;
            job->cost = 0;
            return FALSE;
          }
          // final-weight > free final carry weight
          if (compareWeights(final->getTotalWeight(TRUE),
                (final_pers->carryWeightLimit() - final_pers->getCarriedWeight())) == -1) {
            me->doSay("You can't carry it! I'll just drop it here for you!");
            rc = me->doDrop(job->obj_name, NULL);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
            delete [] job->char_name;
            job->char_name = NULL;
            delete [] job->obj_name;
            job->obj_name = NULL;

            job->cost = 0;
            return FALSE;
          }
          buf = format("%s %s") % job->obj_name % job->char_name;
          if (me->doGive(buf, GIVE_FLAG_IGN_DEX_TEXT) == DELETE_THIS)
            return DELETE_THIS;
          delete [] job->char_name;
          job->char_name = NULL;
          delete [] job->obj_name;
          job->obj_name = NULL;

          job->cost = 0;
          return FALSE;
        } else {
          buf = format("$n works furiously to customize %s's %s.") % job->char_name % job->obj_name;
          act(buf, FALSE, me, NULL, ch, TO_ROOM);
          return FALSE;
        }
      }
      break;
    case CMD_NORTH:
    case CMD_SOUTH:
    case CMD_WEST:
    case CMD_EAST:
    case CMD_UP:
    case CMD_DOWN:
    case CMD_NE:
    case CMD_SW:
    case CMD_SE:
    case CMD_NW:
      if (!(job = (reg_struct *) me->act_ptr)) {
        return FALSE;
      }
      if (!job->char_name) {
        return FALSE;
      }
      if (job->char_name == ch->getName()) {
        buf = format("%s! Don't leave until I finish with this %s!") % ch->getName().c_str() % job->obj_name;
        me->doSay(buf);
        return TRUE;
      } else
        return FALSE;
      break;
    case CMD_VALUE: {
      for(; *arg && isspace(*arg);arg++);
      TThing *ts = NULL;
      TObj *valued = NULL;
      if (!(ts = searchLinkedListVis(ch, arg, ch->stuff)) ||
          !(valued = dynamic_cast<TObj *>(ts))) {
        me->doTell(ch->getName(), "You don't have that item.");
        return TRUE;
      }

      if (custtype == TYPE_TAILOR && valued->itemType() != ITEM_WORN) {
         me->doTell(ch->getName(), "I only customize clothing.");
         return TRUE;
      }

      if (custtype == TYPE_BLACKSMITH && valued->itemType() != ITEM_ARMOR) {
         me->doTell(ch->getName(), "I only customize armor.");
         return TRUE;
      }

      if (valued->engraveMe(ch, me, false))
        return TRUE;

      if (valued->obj_flags.cost <=  500) {
        me->doTell(ch->getName(), "This item is too cheap to be customized.");
        return TRUE;
      }
      if (!valued->action_description.empty()) {
        me->doTell(ch->getName(), "This item has already been customized!");
        return TRUE;
      }
      if (obj_index[valued->getItemIndex()].max_exist <= 10) {
        me->doTell(ch->getName(), "I refuse to customize such an artifact of beauty!");
        return TRUE;
      }
      if (valued->obj_flags.decay_time >= 0) {
        me->doTell(ch->getName(), "Sorry, but this item won't last long enough to bother with an customizing!");
        return TRUE;
      }

      cost = engraveCost(valued, ch, find_shop_nr(me->number));
      if (custtype == TYPE_TAILOR || custtype == TYPE_BLACKSMITH)
        cost *= 2;

      me->doTell(ch->getName(), format("It will cost %d talens to customize your %s.") % cost % fname(valued->name));
      return TRUE;
      }
    case CMD_MOB_GIVEN_ITEM:
      // prohibit polys and charms from engraving
      if (dynamic_cast<TMonster *>(ch)) {
        me->doTell(fname(ch->name), "I don't work for beasts.");
        return TRUE;
      }
      if (!(item = o)) {
        me->doTell(ch->getName(), "You don't have that item!");
        return TRUE;
      }
      me->logItem(item, CMD_EAST);  // log the receipt of the item

      if (custtype == TYPE_TAILOR && item->itemType() != ITEM_WORN) {
        me->doTell(ch->getName(), "I only customize clothing.");
        buf = format("%s %s") % add_bars(item->name) % fname(ch->name);
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }

      if (custtype == TYPE_BLACKSMITH && item->itemType() != ITEM_ARMOR) {
        me->doTell(ch->getName(), "I only customize armor.");
        buf = format("%s %s") % add_bars(item->name) % fname(ch->name);
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }

      if (item->engraveMe(ch, me, true))
        return TRUE;

      if (item->obj_flags.cost <= 500) {
        me->doTell(ch->getName(), "That can't be customized!");
        buf = format("%s %s") % add_bars(item->name) % fname(ch->name);
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      if (!item->action_description.empty()) {
        me->doTell(ch->getName(), "Sorry, but this item has already been customized!");
        buf = format("%s %s") % add_bars(item->name) % fname(ch->name);
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      if (obj_index[item->getItemIndex()].max_exist <= 10) {
        me->doTell(ch->getName(), "This artifact is too powerful to be customized!");
        buf = format("%s %s") % add_bars(item->name) % fname(ch->name);
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      if (item->obj_flags.decay_time >= 0) {
        me->doTell(ch->getName(), "This won't be around long enough to bother customizing it!");
        buf = format("%s %s") % add_bars(item->name) % fname(ch->name);
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }

      cost = engraveCost(item, ch, find_shop_nr(me->number));
      if (custtype == TYPE_TAILOR || custtype == TYPE_BLACKSMITH)
        cost *= 2;

      if (ch->getMoney() < cost) {
        me->doTell(ch->getName(), "I have to make a living! If you don't have the money, I don't do the work!");
        buf = format("%s %s") % add_bars(item->name) % fname(ch->name);
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      job = (reg_struct *) me->act_ptr;
      if (!job->wait || !job->char_name) {
        job->wait = max(1, (int) (item->obj_flags.max_struct_points)/7);
        buf = format("Thanks for your business, I'll take your %d talens payment in advance!") % cost;
        me->doSay(buf);

        TShopOwned tso(find_shop_nr(me->number), me, ch);
        tso.doBuyTransaction(cost, format("customizing %s") % item->getName(), TX_BUYING_SERVICE);

        job->cost = cost;
        job->char_name = new char[ch->getName().length() + 1];
        strcpy(job->char_name, ch->getName().c_str());
        job->obj_name = new char[fname(item->name).length() + 1];
        strcpy(job->obj_name, fname(item->name).c_str());
        --(*item);
        *me += *item;

        me->saveChar(Room::AUTO_RENT);
        ch->saveChar(Room::AUTO_RENT);

        return TRUE;
      } else {
        buf = format("Sorry, %s, but you'll have to wait while I work on %s's item.") % ch->getName().c_str() % job->char_name;
        me->doSay(buf);
        buf = format("%s %s") % add_bars(item->name) % fname(ch->name);
        me->doGive(buf,GIVE_FLAG_IGN_DEX_TEXT);
        return TRUE;
      }
      return FALSE;
    default:
      return FALSE;
  }
  return FALSE;
}
