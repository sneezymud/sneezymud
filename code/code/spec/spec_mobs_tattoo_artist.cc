#include "monster.h"
#include "database.h"
#include "extern.h"

int tattooArtist(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  const char *tattoos[]={"A tattoo of a <r>twisting oriental red dragon.<1>",
			 "A tattoo of a <g>coiled serpent, fangs bared.<1>",
			 "A tattoo of the word '<k>LOVE<1>'.",
			 "A tattoo of the word '<k>HATE<1>'.",
			 "A tattoo of an <p>elegant purple butterfly<1>.",
			 "A tattoo of a <r>small rose<1>.",
			 "A tattoo of a <k>ferocious black panther<1>.",
			 "A tattoo of a <W>skull<1> with <r>red eyes<1>.",
			 "A tattoo of a <W>beautiful naked angel.<1>",
			 "A tattoo of the words '<k>property of Mezan<1>'.",
			 "A tattoo of the underlying <w>skeleton<1>.",
			 "A tattoo of a <k>cross<1>.",
			 "A tattoo of an <k>upside down cross<1>.",
			 "A tattoo of a dagger.",
			 "A tattoo of a <k>bat with wings spread wide.<1>",
			 "A tattoo of the word '<k>sinner<1>'.",
			 "A tattoo of the word '<k>free<1>'.",
			 "A tattoo of a <g>ring of thorns<1>.",
			 "A tattoo of a <Y>lightning bolt<1>.",
			 "A tattoo of the words '<k>untouched by man<1>'.",
			 NULL};
  int ntattoos=20;
  int i;
  char buf[256];


  if(cmd == CMD_LIST){
    myself->doTell(ch->getName(), "I charge 10000 talens for a tattoo.  They are permanent.");
    myself->doTell(ch->getName(), "You can buy the following tattoos from me:");
    for(i=0;i<ntattoos;++i)
      myself->doTell(ch->getName(), format("%i) %s") % (i+1) % tattoos[i]);

    return TRUE;
  } else if(cmd==CMD_BUY){
    arg=one_argument(arg, buf, cElements(buf));

    if(!(i=convertTo<int>(buf)) || i>ntattoos){
      myself->doTell(ch->getName(), "I don't understand, which tattoo do you want?");
      return FALSE;
    }

    one_argument(arg, buf, cElements(buf));
    
    wearSlotT slot=WEAR_NOWHERE;
    int slot_i;
    if ((slot_i = old_search_block(buf, 0, strlen(buf), bodyParts, 0)) > 0) {
      slot = wearSlotT(--slot_i);
      if (!ch->slotChance(slot)) {
        myself->doTell(ch->getName(), "Where do you want the tattoo?");
        return FALSE;
      }
    } else {
      myself->doTell(ch->getName(), "Where do you want the tattoo?");
      return FALSE;
    }
    if(slot==WEAR_LEG_R || slot==WEAR_LEG_L){
      myself->doTell(ch->getName(), "Sorry, it is against my policy to tattoo legs.");
      myself->doTell(ch->getName(), "It's not like you're gonna run around pantless to show it off anyway!");
      return FALSE;
    }
    
    TDatabase db(DB_SNEEZY);
    db.query("select 1 from tattoos where name='%s' and location=%i",
	     ch->getName(), slot);

    if(db.fetchRow()){
      myself->doTell(ch->getName(), "You already have a tattoo there.");
      return FALSE;
    }


    if(ch->getMoney() < 10000){
      myself->doTell(ch->getName(), "Hey buddy, you don't even have the money!  Get out of here!");
      return FALSE;
    }

    ch->setMoney(ch->getMoney()-10000);

    myself->doEmote("takes your money and carefully inks out the tattoo.");


    db.query("insert into tattoos (name, tattoo, location) values ('%s', '%s', %i)", ch->getName(), tattoos[i-1], slot);

    myself->doSay("There you go, all set.");
    
    return TRUE;
  }

  return FALSE;
}
