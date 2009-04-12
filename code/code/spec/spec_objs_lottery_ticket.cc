#include "spec_objs_lottery_ticket.h"
#include "being.h"
#include "parse.h"
#include "obj.h"

LotteryPrizes prizes[NUM_LOTTERY_PRIZES] =
{
  {"lotteryprize0", 10030, 750},// offal
  {"lotteryprize1", 606, 800},  // fortune cookie
  {"lotteryprize2", 2372, 850}, // lottery ticket
  {"lotteryprize3", 8848, 900}, // cigar
  {"lotteryprize4", 2350, 950}, // 100 talen chip
  {"lotteryprize5", 2351, 975}, // 500 talen chip
  {"lotteryprize6", 31759, 985},// black tie
  {"lotteryprize7", 435, 990},  // bottle of champagne
  {"lotteryprize8", 2352, 995}, // 1k talen chip
  {"lotteryprize9", 2353, 1000},// 5k talen chip
};


int lotteryTicket(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o, TObj *)
{
  sstring buf;

  if(cmd!=CMD_SCRATCH || !ch || !o)
    return FALSE;

  if(!isname(arg, o->name))
    return FALSE;

  if(o->isObjStat(ITEM_STRUNG)){
    ch->sendTo("That ticket has already been scratched.\n\r");
    return TRUE;
  }

  o->swapToStrung();

  // choose a prize
  int chance=::number(0,1000);
  int which=0;

  for(;which<NUM_LOTTERY_PRIZES;++which){
    if(chance <= prizes[which].odds)
      break;
  }

  // add the prize identifier to the keywords
  buf=o->name;
  buf += " ";
  buf += prizes[which].name;
  delete [] o->name;
  o->name=mud_str_dup(buf);

  // add the prize description to the extra description
  // we just assume that the first extra is the correct one
  if(which==0){
    buf="This ticket is a loser.\n\r";
  } else {
    buf = format("This ticket is a winner!  The prize is %s.\n\r") %
	     obj_index[real_object(prizes[which].vnum)].short_desc;
  }

  act("$n scratches off $p.",
      TRUE,ch,o,0,TO_ROOM);
  act("You scratch off $p.",
      TRUE,ch,o,0,TO_CHAR);

  ch->sendTo(COLOR_BASIC, buf.c_str());

  buf = format("%s\n\r%s") % o->ex_description->description % buf;
  delete o->ex_description->description;
  o->ex_description->description=mud_str_dup(buf);

  

  return TRUE;
}
