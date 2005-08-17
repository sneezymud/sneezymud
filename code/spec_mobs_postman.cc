#include "stdsneezy.h"

sstring randommessage(sstring from)
{
  sstring msg, buf, sbuf;
  sstring to=mob_index[::number(0,mob_index.size()-1)].short_desc;
  vector <sstring> sentences;

  sentences.push_back("The %s jumps over the %s in the pale moonlight.");
  sentences.push_back("The %s barks %s at noon.");
  sentences.push_back("They have accused me of %sing the %s - it is untrue.");
  sentences.push_back("Desperate assistance is needed with the %s.");
  sentences.push_back("Please send %s at earliest possible convenience.");
  sentences.push_back("I hope things are well with the %s.");
  sentences.push_back("I have become hopelessly in debt playing a local gambling game known as '%s'.");
  sentences.push_back("A youngster said to me today, '%s %s %s'.  I don't comprehend.");
  sentences.push_back("How have your first few days on the new job been?");
  sentences.push_back("I received the package and will send the talens immediately.");
  sentences.push_back("Some of what I say is a bold-faced lie.");
  sentences.push_back("The apocalypse is coming.");
  sentences.push_back("I love you more than words can express.");
  sentences.push_back("Some bastard killed my nephew yesterday.");
  sentences.push_back("I took a girl home last night and pounded her %s, if you know what I mean.");
  sentences.push_back("Have you ever had a woman suck on your %s?  Man that's hot.");
  sentences.push_back("My lover was killed by a robber recently.");
  sentences.push_back("I won several thousand talens playing Hi-Lo at the casino, but then spent it all on hookers and booze.");
  sentences.push_back("After having drinks with the escort at Kavod's, I took her back to the Roaring Lion...");
  sentences.push_back("Do you know of a good ointment for this rash on my crotch?");
  sentences.push_back("Were you ever able to get that rash under control?");
  

  int r=0;
  for(int i=0;i < ::number(1,3);++i){
    r=::number(0,sentences.size()-1);
    buf=sentences[r];
    while(buf.find("%s")!=sstring::npos)
      buf = fmt(buf) % RandomWord();
    msg += buf;
    msg += " ";
    sentences.erase(sentences.begin()+r);
  }

  // line wrap
  sbuf=msg;
  msg="";
  int c=0;
  for(unsigned int i=0;i<sbuf.length();msg+=sbuf[i++]){
    ++c;
    if(c>70 && sbuf[i]==' '){
      msg+="\n";
      ++i;
      c=0;
    }
  }
  
  msg = "Dear " + to + ",\n" + msg;
  msg += "\nRespectfully, ";
  msg += from;
  msg += "\n";
  
  return msg;
}

TObj *createletter(sstring from)
{
  TObj *note, *envelope;

  if (!(note = read_object(GENERIC_NOTE, VIRTUAL))) {
    vlogf(LOG_BUG, "Couldn't make a note for mail!");
    return NULL;
  }
  
  note->swapToStrung();
  delete [] note->name;
  note->name = mud_str_dup("letter mail");
  delete [] note->shortDescr;
  note->shortDescr = mud_str_dup("<o>a handwritten <W>letter<1>"); 
  delete [] note->getDescr();
  note->setDescr(mud_str_dup("A wrinkled <W>letter<1> lies here."));

  delete [] note->action_description;
  sstring msg=randommessage(from);
  note->action_description = mud_str_dup(msg);

  if (!(envelope = read_object(124, VIRTUAL))) {
    vlogf(LOG_BUG, "Couldn't load object 124!");
    return NULL;
  }
  
  *envelope += *note;

  return envelope;
}


int postman(TBeing *, cmdTypeT cmd, const char *, TMonster *me, TObj *)
{
  TObj *bag=NULL;
  TObj *obj;

  if(cmd!=CMD_GENERIC_PULSE || !me)
    return false;

  // Don't let them move, or even hunt, if they are out cold.
  if (!me->awake())
    return false;

  // wander around the world randomly
  if(!::number(0,9))
     me->randomHunt();

  if(::number(0,24))
    return false;

  if(!me)
    return false;

  // search for post bag
  for(TThing *tt=me->getStuff();tt;tt=tt->nextThing){
    if((bag=dynamic_cast<TObj *>(tt)) && bag->objVnum()==2375)
      break;
    else
      bag=NULL;
  }

  if(!bag)
    return false;

  // find random mob
  TMonster *tm=NULL;
  vector <TMonster *> mobs;
  
  for(TThing *tt=me->roomp->getStuff();tt;tt=tt->nextThing){
    if((tm=dynamic_cast<TMonster *>(tt)) && tm != me)
      mobs.push_back(tm);
  }
  if(mobs.size()==0)
    return false;
  tm=mobs[::number(0,mobs.size()-1)];

  int count=0;
  for(TThing *tt=bag->getStuff();tt;tt=tt->nextThing){
    if((obj=dynamic_cast<TObj *>(tt)) && obj->objVnum() == 124)
      count++;
  }
  
  if (!tm->isHumanoid())
    return false;

  if(!::number(0,5) || count > 10){
    for(TThing *tt=bag->getStuff();tt;tt=tt->nextThing){
      if((obj=dynamic_cast<TObj *>(tt)) && obj->objVnum() == 124){
	--(*obj);
	act("$n delivers $p to $N.",
	    FALSE, me, obj, tm, TO_ROOM);
	delete obj;
	if (!::number(0,8)) {
	  TObj *o2;
	  o2=read_object(33601, VIRTUAL);
	  *me += *o2;
	  me->doSay("You can go ahead and pay me later...");
	  me->doGive(tm,o2,GIVE_FLAG_IGN_DEX_TEXT);
	  me->doAction("", CMD_LICK);
	  me->doAction(add_bars(tm->name), CMD_WINK);
	}
	break;
      }
    }
  } else {
    // deliver letter or receive letter
    obj=createletter(tm->getName());
    
    *bag += *obj;
    
    act("$n receives $p for delivery from $N.",
	FALSE, me, obj, tm, TO_ROOM);
  }

  return true;
}
