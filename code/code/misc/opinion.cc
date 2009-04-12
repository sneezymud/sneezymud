//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "room.h"
#include "being.h"
#include "extern.h"
#include "monster.h"

int TMonster::remHated(const TBeing *hatee, const char *n)
{
  charList *list, *t, *t2;
  char namebuf[256];
  int i;

  if (hatee)
    strcpy(namebuf, hatee->getName());
  else
    strcpy(namebuf, n);

  for (i = 0, list = hates.clist; list; list = t, i++) {
    t = list->next;
    if (!strcmp(list->name, namebuf)) {
      if (hates.clist == list) {
        hates.clist = t;
      } else {
        for (t2 = hates.clist; t2->next != list; t2 = t2->next);
        t2->next = list->next;
      }
      delete list;
    }
  }
  if (!hates.clist) {
    REMOVE_BIT(hatefield, HATE_CHAR);
  }
  if (!hatefield) {
    REMOVE_BIT(specials.act, ACT_HATEFUL);
  }
  return ((hatee) ? TRUE : FALSE);
}

bool TBeing::addHated(TBeing *)
{
  return false;
}

bool TMonster::addHated(TBeing *hatee)
{
  if (this == hatee)
    return FALSE;

  // insure NPC hate PC ONLY!
  if (isPc() || !hatee->isPc())
    return FALSE;

  // We need to make sure an immortal mobile will not gain a player hatred, period.
  if (IS_SET(specials.act, ACT_IMMORTAL))
    return FALSE;

  if (hatee) {
    if (Hates(hatee, NULL))	/* hate someone only once - SG */
      return FALSE;
    // so it won't add someone to the hate list if they have other reasons (racial, etc) to hate

    bool silent_multi_hate = TRUE;
    if (this->in_room == hatee->in_room)
      silent_multi_hate = FALSE;
    
    if (multiHates(hatee, silent_multi_hate)) {
      // mob is hating a new player from the same account as an existing hatee
      // checking room in case hate has other causes, like a god setting hatred
      // player can probably work around this using a ranged weapon, but whatever
      
      if (!silent_multi_hate) {
        doAction(fname(hatee->name), CMD_ACCUSE);
      }
    }
    
    if (!awake())               /* don't add to the list if mob is !awake */
      return FALSE;

    charList *list = new charList();
    list->name          = mud_str_dup(hatee->name);
    list->next          = hates.clist;
    list->account_id    = hatee->player.account_id;
    list->player_id    = hatee->player.player_id;

    /*
      Times: (Game Hours)
        Default Maximum Hate: 219 hours ( (50 + 50 + 5) * (250 / 120) = 218.75 )
        Default Minimum Hate:   2 hours ( ( 1 +  1 + 5) * ( 30 / 120) =   1.75 )
     */
    list->iHateStrength = (long)((GetMaxLevel() + hatee->GetMaxLevel() + 5) * ((double)getStat(STAT_CURRENT, STAT_FOC) / 120.0));
    hates.clist = list;

    SET_BIT(specials.act, ACT_HATEFUL);
    SET_BIT(hatefield, HATE_CHAR);

    if (hatee->isImmortal())
      hatee->sendTo(COLOR_MOBS, format("--- %s hates you.\n\r") % sstring(getName()).cap());
  }
  return ((hatee) ? TRUE : FALSE);
}

int TMonster::addHatred(zoneHateT parm_type, int parm)
{
  switch (parm_type) {
    case OP_SEX:
      SET_BIT(hatefield, HATE_SEX);
      if (parm != SEX_MALE && parm != SEX_FEMALE && parm != SEX_NEUTER) {
        vlogf(LOG_BUG, format("bad parm to adHatred-sex for %s : %d") %  getName() % parm);
        parm = 0;
      }
      hates.sex = sexTypeT(parm);
      break;
    case OP_RACE:
      if (!IS_SET(hatefield, HATE_RACE))
	SET_BIT(hatefield, HATE_RACE);
      if (parm < RACE_NORACE || parm >= MAX_RACIAL_TYPES) {
        vlogf(LOG_BUG, format("Bad parm to addHatred-race for %s : %d") %  getName() % parm);
        parm = 0;
      }
      hates.race = race_t(parm);
      break;
#if 0
    case OP_GOOD:
      if (!IS_SET(hatefield, HATE_GOOD))
	SET_BIT(hatefield, HATE_GOOD);
      hates.good = parm;
      break;
    case OP_EVIL:
      if (!IS_SET(hatefield, HATE_EVIL))
	SET_BIT(hatefield, HATE_EVIL);
      hates.evil = parm;
      break;
#endif
    case OP_CLASS:
      if (!IS_SET(hatefield, HATE_CLASS))
	SET_BIT(hatefield, HATE_CLASS);
      hates.Class = parm;
      break;
    case OP_VNUM:
      if (!IS_SET(hatefield, HATE_VNUM))
	SET_BIT(hatefield, HATE_VNUM);
      hates.vnum = parm;
      break;
    case OP_UNUSED:
    case OP_UNUSED2:
    case OP_CHAR:
    case MAX_HATE:
      vlogf(LOG_LOW, format("Bad use of Hate flags on %s") %  getName());
      return TRUE;
  }
  if (!IS_SET(specials.act, ACT_HATEFUL)) {
    SET_BIT(specials.act, ACT_HATEFUL);
  }
  return TRUE;
}

int TMonster::remHatred(unsigned short bitv)
{
  REMOVE_BIT(hatefield, bitv);
  if (!hatefield)
    if (!isPc())
      REMOVE_BIT(specials.act, ACT_HATEFUL);

  return TRUE;
}

bool TMonster::multiHates(const TBeing *v, bool silent)
{
  if (!hatefield)
    return FALSE;
    
  if (isPc() || !v->isPc())
    return FALSE;    
 
  if (this == v)
    return FALSE;
  
  if (!v->player.account_id || !v->player.player_id)
    return FALSE;
    
  bool multi = FALSE;
  // loop over the hated list, looking for matching account id
  if (IS_SET(hatefield, HATE_CHAR)) {
    if (hates.clist) {
      TBeing *m = dynamic_cast<TBeing *>(this);
      charList *i;
      for (i = hates.clist; i; i = i->next) {
        if (i->account_id == v->player.account_id && i->player_id != v->player.player_id && !v->hasWizPower(POWER_MULTIPLAY)) {
          // shout it, log it
          if (!silent) {
            if (m)
              m->doShout(format("%s smells so much like %s it's creepy.") % v->getName() % i->name);
          }
          
          vlogf(LOG_CHEAT, format("MULTIPLAY: Players %s and %s are both hated by %s.") % v->getName() % i->name % getName());
          multi = TRUE;
        }
      }
    }
  }
  return multi;
}

bool TMonster::Hates(const TBeing *v, const char *n) const
{
  if (!hatefield)
    return FALSE;

  sstring namebuf;

  if (!v) 
    namebuf = n;
  else {
    if(dynamic_cast<const TMonster *>(v)){
      namebuf = v->name;
    } else {
      namebuf = v->getName();
    }
  }

  if (namebuf.empty())
    return FALSE;
 
  if (this == v)
    return FALSE;

  if (IS_SET(hatefield, HATE_CHAR)) {
    if (hates.clist) {
      charList *i;
      for (i = hates.clist; i; i = i->next) {
        sstring tmpname = i->name;
        if (namebuf == tmpname) {
          return TRUE;
        }
      }
    }
  }
  if (IS_SET(hatefield, HATE_RACE) && v)
    if (hates.race != -1)
      if (hates.race == v->getRace())
	return TRUE;

  if (IS_SET(hatefield, HATE_SEX) && v)
    if (hates.sex == v->getSex())
      return TRUE;

// this needs to be set for factions, not align
#if 0
  if (IS_SET(hatefield, HATE_GOOD) && v)
    if (v->isGood())
      return TRUE;

  if (IS_SET(hatefield, HATE_EVIL) && v)
    if (v->isEvil())
      return TRUE;
#endif

  if (IS_SET(hatefield, HATE_CLASS) && v)
    if (v->hasClass(hates.Class))
      return TRUE;

  if (IS_SET(hatefield, HATE_VNUM) && v) {
    const TMonster *mv = dynamic_cast<const TMonster *>(v);
    if (mv && (hates.vnum == mob_index[mv->getMobIndex()].virt))
      return TRUE;
  }

  return FALSE;
}

bool TMonster::Fears(const TBeing *v, const char *s) const
{
  if (!fearfield)
    return false;

  charList *i;
  const char *buf;

  if (!awake())
    return FALSE;

  if (!IS_SET(specials.act, ACT_AFRAID))
    return FALSE;

  if (!v)
    buf = s;
  else
    buf = v->getName();

  if (!buf)
    return FALSE;

  if (IS_SET(fearfield, FEAR_CHAR)) {
    for (i = fears.clist; i; i = i->next) {
      if (i->name)
        if (!strcmp(i->name, buf))
          return TRUE;
    }
  }
  if (IS_SET(fearfield, FEAR_RACE))
    if (fears.race != -1)
      if (fears.race == v->getRace())
	return TRUE;

  if (IS_SET(fearfield, FEAR_SEX))
    if (fears.sex == v->getSex())
      return TRUE;

#if 0
  if (IS_SET(fearfield, FEAR_GOOD))
    return TRUE;

  if (IS_SET(fearfield, FEAR_EVIL))
    return TRUE;
#endif

  if (IS_SET(fearfield, FEAR_CLASS))
    if (v->hasClass(fears.Class))
      return TRUE;

  if (IS_SET(fearfield, FEAR_VNUM)) {
    sendTo(format("You fear %i \n\r") % fears.vnum);
    const TMonster *mv = dynamic_cast<const TMonster *>(v);
    if (mv && (fears.vnum == mv->mobVnum()))
      return TRUE;
  }
  return FALSE;
}

int TMonster::remFeared(const TBeing *hatee, const char *n)
{
  charList *list, *t, *tmp;
  char buf[256];

  if (hatee)
    strcpy(buf, hatee->getName());
  else
    strcpy(buf, n);
 
  if (!IS_SET(specials.act, ACT_AFRAID))
    return FALSE;

  for (list = fears.clist; list; list = tmp) {
    tmp = list->next;
    if (!strcmp(list->name, buf)) {
      if (list == fears.clist) {
        fears.clist = tmp;  // head of list
      } else {
        // find previous to current, set it to my next
        for (t = fears.clist;t && t->next != list; t = t->next);
        t->next = list->next;
      }
      delete list;
    }
  }
  if (!fears.clist)
    REMOVE_BIT(fearfield, FEAR_CHAR);

  if (!fearfield)
    REMOVE_BIT(specials.act, ACT_AFRAID);

  return ((hatee) ? TRUE : FALSE);
}

int TMonster::addFeared(TBeing *hatee)
{
  if (hatee == specials.hunting) {	/* dont' hunt someone we fear */
    persist = 0;
    specials.hunting = 0;
    hunt_dist = 0;
  }
  if (hatee) {
    if (Fears(hatee, NULL))	// fear only once
      return FALSE;

    charList *list = new charList();
    list->name = mud_str_dup(hatee->getName());
    list->next = fears.clist;
    fears.clist = list;

    SET_BIT(specials.act, ACT_AFRAID);
    SET_BIT(fearfield, FEAR_CHAR);

    if (hatee->isImmortal())
      hatee->sendTo(COLOR_MOBS, format("--- %s fears you.  (as well they should)\n\r") % sstring(getName()).cap());
  }
  return ((hatee) ? TRUE : FALSE);
}


int TMonster::addFears(zoneHateT parm_type, int parm)
{
  switch (parm_type) {
    case OP_SEX:
      if (!IS_SET(fearfield, FEAR_SEX))
	SET_BIT(fearfield, FEAR_SEX);
      if (parm != SEX_MALE && parm != SEX_FEMALE && parm != SEX_NEUTER) {
        vlogf(LOG_BUG, format("bad parm to addFears-sex for %s : %d") %  getName() % parm);
        parm = 0;
      }
      fears.sex = sexTypeT(parm);
      break;
    case OP_RACE:
      if (!IS_SET(fearfield, FEAR_RACE))
	SET_BIT(fearfield, FEAR_RACE);
      if (parm < RACE_NORACE || parm >= MAX_RACIAL_TYPES) {
        vlogf(LOG_BUG, format("Bad parm to addFears-race for %s : %d") %  getName() % parm);
        parm = 0;
      }
      fears.race = race_t(parm);
      break;
#if 0
    case OP_GOOD:
      if (!IS_SET(fearfield, FEAR_GOOD))
	SET_BIT(fearfield, FEAR_GOOD);
      fears.good = parm;
      break;
    case OP_EVIL:
      if (!IS_SET(fearfield, FEAR_EVIL))
	SET_BIT(fearfield, FEAR_EVIL);
      fears.evil = parm;
      break;
#endif
    case OP_CLASS:
      if (!IS_SET(fearfield, FEAR_CLASS))
	SET_BIT(fearfield, FEAR_CLASS);
      fears.Class = parm;
      break;
    case OP_VNUM:
      if (!IS_SET(fearfield, FEAR_VNUM))
	SET_BIT(fearfield, FEAR_VNUM);
      fears.vnum = parm;
      break;
    case OP_UNUSED:
    case OP_UNUSED2:
    case OP_CHAR:
    case MAX_HATE:
      vlogf(LOG_LOW, format("Bad use of Fear flags on %s") %  getName());
      return TRUE;
  }
  SET_BIT(specials.act, ACT_AFRAID);

  return TRUE;
}


TBeing * TMonster::findAHatee(void)
{
  TBeing *tmp_ch = NULL;
  TThing *t=NULL;

  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
    tmp_ch = dynamic_cast<TBeing *>(t);
    if (!tmp_ch)
      continue;
    if (Hates(tmp_ch, NULL) && canSee(tmp_ch) && (this != tmp_ch) &&
         !(tmp_ch->isImmortal() && 
               tmp_ch->isPlayerAction(PLR_NOHASSLE))) {
      return (tmp_ch);
    }
  }
  return NULL;
}

TBeing * TMonster::findAFearee(void)
{
  TBeing *tmp_ch = NULL;
  TThing *t=NULL;

  for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end() && (t=*it);++it) {
    tmp_ch = dynamic_cast<TBeing *>(t);
    if (!tmp_ch)
      continue;
    if (Fears(tmp_ch, NULL) && canSee(tmp_ch) && (this != tmp_ch) &&
         !(tmp_ch->isImmortal() &&
               tmp_ch->isPlayerAction(PLR_NOHASSLE))) {
      return (tmp_ch);
    }
  }
  return NULL;
}


// these two are to make the monsters completely forget about them.
// Either ch is NULL and s is used or vice versa. char *s is used when
// characters delete themselves and dont have associated TBEing pointers,
// just name of old character is used in this case - Brutius
void DeleteHatreds(const TBeing *ch, const char *s)
{
  TBeing *i;

  for (i = character_list; i; i = i->next) {
    if (i->isPc())
      continue;
    TMonster *tmp = dynamic_cast<TMonster *>(i);

    if (tmp->Hates(ch, s))
      tmp->remHated(ch, s);

    if (ch && (tmp->targ() == ch)) {
      tmp->setTarg(NULL);
      tmp->setAnger((tmp->anger() + tmp->defanger())/2);
      tmp->DMal(11);
      tmp->DS(5);
    }
  }
}

void DeleteFears(const TBeing *ch, const char *s)
{
  TBeing *i;

  for (i = character_list; i; i = i->next) {
    if (i->isPc())
      continue;
    TMonster *tmp = dynamic_cast<TMonster *>(i);

    if (tmp->Fears(ch, s))
      tmp->remFeared(ch, s);
  }
}

void TMonster::developHatred(TBeing *v)
{
  int patience, var;

  if (this == v)
    return;

  if (Hates(v, NULL))
    return;

  // rule is that patience+randomizer < diff ==> hatred
  // Mobs shouldn't hate until they become real hurt
  // However, we should use hatred as a preventitive measure to keep newbies
  // from single-hitting high mobs, then fleeing for massive-XP.
  // so rule 1 says diff should basically 50
  // rule 2 says diff should be modified by level
  int diff = 50;
  int lev = GetMaxLevel() - v->GetMaxLevel();
  if (lev > 10)  // no hope of killing without a real big group
    diff = 100;
  else if (lev > 5)  // killable, but onyl with 2-3 people
    diff = 75;
  

  // set patience to the critters %health
  int hl = hitLimit();
  if (hl > 0)
    patience = (int) (100 * getHit() / hl);
  else
    patience = 10;

  // randomizer
  var = ::number(-20, 20);

  if (patience + var < diff)
    addHated(v);
}

void TMonster::setHunting(TBeing *tch)
{
  int dist;

  persist = GetMaxLevel();

  dist = 50 + GetMaxLevel();

  if (Hates(tch, NULL))
    dist *= 2;

  SET_BIT(specials.act, ACT_HUNTING);
  specials.hunting = tch;
  hunt_dist = dist;
  oldRoom = inRoom();

  if (tch->isImmortal())
    tch->sendTo(COLOR_MOBS,format(">>%s is hunting you from %s\n\r") % getName() % roomp->name);
}

/* ------------------------------------------------------
 isAttackerMultiplay
 returns true if the aggressor attacking me is on my hated list

 Anti-Multiplay code, Pappy 11/7/2007
 Essentially the design here is that if you try to attack an NPC which you have angered with
 one of your alts, we make you do absolutely no damage at all
 this also applies if a thrall of yours is attacking a mob which your alt angered
------------------------------------------------------ */
bool TMonster::isAttackerMultiplay(TBeing *aggressor)
{
  // check if an alt of this character is on my hate list.  If so -> multiplayer!
  if (aggressor->isPc() && IS_SET(hatefield, HATE_CHAR) && hates.clist)
  {
    if (aggressor->hasWizPower(POWER_MULTIPLAY))
      return false; // return false if the aggressor is allowed to multiplay

    for (charList *iChar = hates.clist; iChar; iChar = iChar->next)
    {
      if (iChar->account_id == aggressor->player.account_id &&
        iChar->player_id != aggressor->player.player_id)
      {
        vlogf(LOG_CHEAT, format("Multiplay: %s is attempting to multiplay harm %s which was already tagged by %s!") % aggressor->name % this->name % iChar->name);
        return true;
      }
    }
  }

  // check if a thrall of a multiplayer is attacking me
  if (aggressor->isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL) &&
    aggressor->master && aggressor->master->isPc())
  {
    return isAttackerMultiplay(aggressor->master);
  }

  return false;
}


