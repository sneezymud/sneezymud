///////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "bounty_hunter.cc" - Special procedures for bounty_hunter
//
///////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "shop.h"
#include "statistics.h"
#include "pathfinder.h"

static TBeing *char_with_name(char *name)
{
  Descriptor *d;

  for (d = descriptor_list; d; d = d->next) {
    if ((d->connected == CON_PLYNG) && (d->character) &&
        (!strcasecmp(d->character->getName(), name)))
      return d->character;
  }
  return NULL;
}

// scans for the LAST item in the object list
static TObj * findHuntedItem(const TBeing *ch, const char *arg, const TObj *stopper, const char *targ)
{
  if (!targ)
    return NULL;

  
  TObj *obj, *last;
  unsigned int i;
  map<int,int>vnums;
  
  // scan the object index and make a list of vnums that match our keyword
  for (i = 0;i < obj_index.size();i++) {
    if(isname(arg, obj_index[i].name)){
      vnums[obj_index[i].virt]=1;
    }
  }
  
  // now search for the vnums we cached
  last=NULL;
  for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
    obj=*iter;
    if (vnums[obj->objVnum()]==1) {
      if (ch->canSee(obj))
	if (!targ || !dynamic_cast<TBeing *>(obj->thingHolding()) || isname(targ, dynamic_cast<TBeing *>(obj->thingHolding())->name)) {
	  last = obj;
	}
    }
    if (obj == stopper)  // we need not go further
      break;
  }
  
  return last;
}

static TObj * findHuntedItem(const TBeing *ch, const char *arg, const TObj *stopper)
{
  TObj *obj, *last;
  unsigned int i;
  map<int,int>vnums;

  // scan the object index and make a list of vnums that match our keyword
  for (i = 0;i < obj_index.size();i++) {
    if(isname(arg, obj_index[i].name)){
      vnums[obj_index[i].virt]=1;
    }
  }

  // now search for the vnums we cached
  last=NULL;
  for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
    obj=*iter;
    if (vnums[obj->objVnum()]==1) {
      if (ch->canSee(obj))
	last = obj;
    }
    if (obj == stopper)  // we need not go further
      break;
  }

  return last;
}


bounty_hunt_struct::bounty_hunt_struct(char *hi, char *hv, int chan, int lev) :
  hunted_item(hi),
  hunted_victim(hv),
  last_targ(NULL),
  num_chances(chan),
  level_command(lev),
  num_retrieved(0),
  noneBeyond(NULL),
  warned(false),
  singletarg(false),
  missionaccomplished(false)
{
}

bounty_hunt_struct::~bounty_hunt_struct()
{
  delete [] hunted_item;
  delete [] hunted_victim;
  delete [] last_targ;
}

void bounty_hunt_struct::reset()
{

  //  vlogf(LOG_DASH, fmt("hunter being reset with vict: %s") %  (hunted_victim ? hunted_victim : "NULL"));
  delete [] hunted_item;
  hunted_item = NULL;
  if (!singletarg || missionaccomplished) {
    delete [] hunted_victim;
    hunted_victim = NULL;
  }
  num_chances = 9;
  num_retrieved = 0;
  noneBeyond = NULL;
  warned = false;

  delete [] last_targ;
  last_targ = NULL;

  //  singletarg = false;
  //  missionaccomplished = false;
}

static void cleanUpHunter(TMonster *myself)
{
  myself->setDefAnger(0);
  myself->setDefMalice(0);
  myself->setAnger(0);
  myself->setMalice(0);

  // take away power to attack base on racial-hatreds, etc
  REMOVE_BIT(myself->specials.act, ACT_HATEFUL);

  myself->visionBonus = 25;  // make able to see stuff
  myself->default_pos = POSITION_STANDING;  // get the bums up
}

static int warCry(TMonster *myself, TBeing *targ, TObj *temp_obj)
{
  char buf[256];

  sprintf(buf,"%s will be mine again!",temp_obj->shortDescr);
  myself->doSay(sstring(buf).cap().c_str());
  return myself->takeFirstHit(*targ);
}

static void bountyPoof(TMonster *myself, int targ_rm)
{
  if (targ_rm == myself->inRoom())
    return;

  act("$n fades almost unnoticed into the shadows.", 
                     0, myself, 0, 0, TO_ROOM);
  --(*myself);
  thing_to_room(myself, targ_rm);
  act("$n steps from the shadows.", 0, myself, 0, 0, TO_ROOM);
}

int bounty_hunter(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  char buf[256];
  char buf2[256];
  char buf3[256];
  TBeing *targ = NULL;
  TObj *temp_obj = NULL;
  int room, rc;
  dirTypeT dir;
  int beam_in_room = 0;
  roomDirData *exitp;
  affectedData *aff = NULL;
  TPathFinder path;

  bounty_hunt_struct *job = NULL;
  const char HUNTER_ID[] = "Hunter,";

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<bounty_hunt_struct *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  }

  if (myself->act_ptr) 
    job = static_cast<bounty_hunt_struct *>(myself->act_ptr);

  //  vlogf(LOG_DASH, fmt("hunter being called - proc = %d, generic pulse? %s") %  cmd % (cmd==CMD_GENERIC_PULSE ? "yes" : "no"));

  if ((cmd != CMD_GENERIC_PULSE)) {
    if ((cmd == CMD_RENT) || (cmd == CMD_DROP) || (cmd == CMD_GIVE)) {
      if (!ch->name || !job || !job->hunted_victim || !isname(job->hunted_victim, ch->name))
        return FALSE;
      if (!arg)
        return FALSE;
      for (; *arg == ' '; arg++);
      if (myself->act_ptr) {
        job = static_cast<bounty_hunt_struct *>(myself->act_ptr);
	if (job->hunted_item != NULL) {
          if (job->singletarg)
	    temp_obj = findHuntedItem(myself, job->hunted_item, job->noneBeyond, job->hunted_victim);
	  else
	    temp_obj = findHuntedItem(myself, job->hunted_item, job->noneBeyond);
	  
          if (temp_obj && (ch == temp_obj->thingHolding())) {
            if (cmd == CMD_RENT) {
              myself->doSay("You're not getting away that easily.");
              act("$n stands between you and the receptionist!", FALSE, myself, 0, ch, TO_VICT);
              act("$n blocks $N from renting!", FALSE, myself, 0, ch, TO_NOTVICT);
              return TRUE;
            } else if (cmd == CMD_GIVE) {
              if (((sscanf(arg, " %s %s", buf, buf2) == 2) && (isname(buf, temp_obj->name))) || 
                  ((sscanf(arg, " %s to %s", buf, buf2) == 2) && (isname(buf, temp_obj->name)))) {
                if (isname(buf2, myself->name)) {
                  if (ch->doGive(arg) == DELETE_THIS)
                    return DELETE_VICT;
                  // double check that the give succeeded
                  if (temp_obj->parent == myself) {
                    myself->doAction(fname(ch->name), CMD_SMILE);
                    myself->doSay("I'm glad you've come to your senses!  I would surely smite thee!");
		    
                    myself->doSay("I will leave you alone now that I have my bounty. Goodbye!!");
		    //                    return DELETE_THIS;
		    // curious what they do if i take this line out, heh. - dash
		    //if (job->singletarg) 
		    //  job->missionaccomplished = true;
		    
		    return TRUE;
                  }
                } else {
                  myself->doSay("I'll take that!");
                  act("$n almost grabs it from your hands!  Whew!", FALSE, myself, 0, ch, TO_VICT);
                  act("$n almost grabs something from $N's hands!", FALSE, myself, 0, ch, TO_NOTVICT);
                  return TRUE;
                }
              }
            } else {
              // cmd == CMD_DROP
              sscanf(arg, " %s ", buf);
              if (isname(buf, temp_obj->name) &&
		  searchLinkedListVis(ch, buf, ch->getStuff())) {
                rc = ch->doDrop("", temp_obj);
                if (IS_SET_DELETE(rc, DELETE_ITEM)) {
                  delete temp_obj;
                  temp_obj = NULL;
                }
                if (IS_SET_DELETE(rc, DELETE_THIS))
                  return DELETE_VICT;
                myself->doSay("Thanks, sport!");
                rc = myself->doGet(arg);
                if (IS_SET_DELETE(rc, DELETE_THIS))
                  return DELETE_THIS;
                return TRUE;
              }
            }
          }
        }
      }
      return FALSE;
    } else if (((cmd == CMD_GROUP) || (cmd == CMD_FOLLOW)) && !ch->isImmortal()) {
      // need this, since mob follows pc, they might try and group it
      if (!ch->name || !job || !job->hunted_victim || !isname(job->hunted_victim, ch->name)) 
	return FALSE;
      myself->doAction(ch->name, CMD_GROWL);
      myself->doSay("No one is grouping here, scum.");
      return TRUE;
    } else if (cmd == CMD_FLEE && ch != myself) {
      vlogf(LOG_DASH, fmt("hunter flee ch->name: %s, job->vict: %s, isname? %s") %  
	    (ch->name ? ch->name : "NULL") % 
	    (job && job->hunted_victim ? job->hunted_victim : "NULL") %
	    (job && job->hunted_victim && ch->name && isname(job->hunted_victim, ch->name) ? "yes" : "no"));
      if (!ch->name || !job || !job->hunted_victim || 
	  !isname(job->hunted_victim, ch->name))
        return FALSE;
      ch->addToMove(-15);
      job = static_cast<bounty_hunt_struct *>(myself->act_ptr);
      if (job) {
        job->warned = TRUE;
        if (job->num_chances >= -99 || job->num_chances <= -96)
          job->num_chances = -96;
        else
          job->num_chances = 1;
      }
      if (::number(0,5) && myself->canSee(ch)) {
        act("$n stands between you and the exit!", FALSE, myself, 0, ch, TO_VICT);
        act("$n blocks $N from fleeing!", FALSE, myself, 0, ch, TO_NOTVICT);
        myself->doSay("You're not getting away that easily!");
        return TRUE;
      }
      return FALSE;
    } else if (cmd == CMD_CONSIDER) {
      if (!ch->name || !job || !job->hunted_victim || !isname(job->hunted_victim, ch->name))
        return FALSE;

      job = static_cast<bounty_hunt_struct *>(myself->act_ptr);
      if (job) {
	job->num_chances--;
      }
      return FALSE;

    } else if (((cmd == CMD_SAY) || (cmd == CMD_SAY2)) && 
	       (!ch || ch->isImmortal())) {
      if (!arg)
        return FALSE;
      for (; *arg == ' '; arg++);
      
      // this !ch case is whenit gets set automatically
      if (strncasecmp(arg, HUNTER_ID, strlen(HUNTER_ID)))
        return FALSE;
      
      arg += strlen(HUNTER_ID);
      job = (bounty_hunt_struct *) myself->act_ptr;
      if (!strncasecmp(arg, " status", 7)) {
        if (!job) {
          act("$n whispers 'Master $N, I am currently idle with no memory.'", FALSE, myself, 0, ch, TO_VICT);
          return TRUE;
        }
        if (job->hunted_item != NULL) {
          if (job->hunted_victim != NULL)
            sprintf(buf2, "$n whispers 'Master $N, I am hunting the %s carried by %s.'",
                    job->hunted_item, job->hunted_victim);
          else
            sprintf(buf2, "$n whispers 'Master $N, I am currently hunting %s.'", job->hunted_item);
          if (job->num_retrieved > 0) {
            act(buf2, FALSE, myself, 0, ch, TO_VICT);
            sprintf(buf2, "$n whispers 'I have already retrieved and destroyed %d.'", job->num_retrieved);
          }
        } else if (job->hunted_victim != NULL) {
          sprintf(buf2, "$n whispers 'Master $N, I am going to kill %s.'", job->hunted_victim);
          if (job->num_retrieved > 0) {
            act(buf2, FALSE, myself, 0, ch, TO_VICT);
            sprintf(buf2, "$n whispers 'I have already killed him %d %s.'", job->num_retrieved,
                    (job->num_retrieved == 1) ? "time" : "times");
          }
        } else
          sprintf(buf2, "$n whispers 'Master $N, I am currently idle with memory allocated.'");
	
        act(buf2, FALSE, myself, 0, ch, TO_VICT);
        if (job->num_chances >= -99 && job->num_chances <= -96)
          act("$n whispers 'I am showing no mercy.'", FALSE, myself, 0, ch, TO_VICT);
        else
          act("$n whispers 'I am showing mercy.  Do I *have* to?'", FALSE, myself, 0, ch, TO_VICT);
	
	if (job->singletarg) {
	  if(job->missionaccomplished)
	    act("$n whispers 'I was hunting a single target and my mission is complete.'", FALSE, myself, 0, ch, TO_VICT);
	  else
	    act("$n whispers 'I am hunting a single target and my mission is incomplete.'", FALSE, myself, 0, ch, TO_VICT);
	}
	
        sprintf(buf2, "$n whispers 'Master $N, I am using room %d as my base of operations.'", myself->oldRoom);
        act(buf2, FALSE, myself, 0, ch, TO_VICT);
	
        return TRUE;
      }
      // below here is kill and repo commands, both need to alloc if needed
      if (!myself->act_ptr) {
        myself->act_ptr = new bounty_hunt_struct(NULL, NULL, 9, ch ? ch->GetMaxLevel() : GOD_LEVEL1);
        if (!myself->act_ptr) {
          perror("failed new of hunter.");
          exit(0);
        }
        cleanUpHunter(myself);

        if (ch)
          act("$n whispers 'Master $N, I am preparing to remember your will.", FALSE, myself, 0, ch, TO_VICT);
      }
      job = static_cast<bounty_hunt_struct *>(myself->act_ptr);
      if (!job) {
        vlogf(LOG_PROC, "REPO: Unable to allocate memory for bounty hunter!  This is bad!");
        return TRUE;
      }
      if (ch && job->level_command > ch->GetMaxLevel()) {
        act("$n whispers 'Sorry, $N.  I report to a higher authority.'", FALSE, myself, 0, ch, TO_VICT);
        return TRUE;
      }
      if (sscanf(arg, " repo %s from %s", buf, buf3) == 2) {
        if (strlen(buf3) > 79 || strlen(buf) > 79)
          return TRUE;
	
        sprintf(buf2, "$n whispers 'Master $N, I will now reposess a single %s from %s.'", buf, buf3);
        if (ch)
          act(buf2, FALSE, myself, 0, ch, TO_VICT);

	vlogf(LOG_PROC, fmt("REPO: repo mob (%s) being sent after %s on %s by %s.") %  myself->getName() % buf % buf3 % (ch ? ch->getName() : "the code"));

        job->reset();
        job->hunted_item = mud_str_dup(buf);
	job->hunted_victim = mud_str_dup(buf3);
	job->singletarg = true;
        if (job->num_chances < -99 || job->num_chances > -96)
          job->num_chances = 27;
        if (!ch && (myself->specials.act &  ACT_AGGRESSIVE)) {
          REMOVE_BIT(myself->specials.act, ACT_AGGRESSIVE);
          job->num_chances = -99;
        }
        cleanUpHunter(myself);
      } if (sscanf(arg, " repo %s", buf) == 1) {
        if (strlen(buf) > 79)
          return TRUE;
        sprintf(buf2, "$n whispers 'Master $N, I will now reposess all %s.'", buf);
        if (ch)
          act(buf2, FALSE, myself, 0, ch, TO_VICT);

        job->reset();
        job->hunted_item = mud_str_dup(buf);

        if (job->num_chances < -99 || job->num_chances > -96)
          job->num_chances = 27;
        if (!ch && (myself->specials.act &  ACT_AGGRESSIVE)) {
          REMOVE_BIT(myself->specials.act, ACT_AGGRESSIVE);
          job->num_chances = -99;
        }
        cleanUpHunter(myself);
      } else if (sscanf(arg, " kill %s", buf) == 1) {
        if (strlen(buf) > 79)
          return TRUE;
        sprintf(buf2, "$n whispers 'Master $N, I will now kill %s.'", buf);
        act(buf2, FALSE, myself, 0, ch, TO_VICT);

        job->reset();
        job->hunted_victim = mud_str_dup(buf);
        if (job->num_chances < -99 || job->num_chances > -96)
          job->num_chances = 27;
      } else if (!strncasecmp(arg, " forget", 7)) {
        act("$n whispers 'Master $N, I am clearing my memory.'", FALSE, myself, 0, ch, TO_VICT);
        delete job;
        myself->act_ptr = NULL;
      } else if (!strncasecmp(arg, " no mercy", 9)) {
        act("$n whispers 'Master $N, I will have no mercy.'", FALSE, myself, 0, ch, TO_VICT);
        job->num_chances = -99;
        job->warned = TRUE;
      } else if (!strncasecmp(arg, " show mercy", 11)) {
        act("$n whispers 'Master $N, I will now show mercy.'", FALSE, myself, 0, ch, TO_VICT);
        job->num_chances = 27;
        job->warned = FALSE;
      } else {
        act("$n whispers 'Master $N, I don't understand your request.'", FALSE, myself, 0, ch, TO_VICT);
        act("$n whispers 'Please forgive me!'", FALSE, myself, 0, ch, TO_VICT);
      }
      return TRUE;
    }
    return FALSE;
  }
  // CMD_GENERIC_PULSE below here
  if (!myself->act_ptr || !myself->awake())
    return FALSE;

  job = static_cast<bounty_hunt_struct *>(myself->act_ptr);

  if (myself->oldRoom == myself->inRoom() && job->missionaccomplished) {
    myself->spec = 0;

    vlogf(LOG_PROC, fmt("REPO: %s finished repoing  %s of %s, deactivating.") %  myself->getName() %
	  job->hunted_victim % job->hunted_item);
    delete job;
    return TRUE;
  }


  //  vlogf(LOG_DASH, fmt("Bounty hunter %s, generic pulse, item = %s, victim = %s, singletarg = %s.") %  myself->getName() %
  //	(job->hunted_item ? job->hunted_item : "NULL") % (job->hunted_victim ? job->hunted_victim : "NULL") %
  //	(job->singletarg ? "TRUE" : "FALSE"));
  if (job->hunted_item) {
    if (job->singletarg)
      temp_obj = findHuntedItem(myself, job->hunted_item, job->noneBeyond, job->hunted_victim);
    else
      temp_obj = findHuntedItem(myself, job->hunted_item, job->noneBeyond);


    if (job->singletarg && !temp_obj) {
      strcpy(buf,myself->name);
      strcpy(buf, add_bars(buf).c_str());

      temp_obj = findHuntedItem(myself, job->hunted_item, job->noneBeyond, buf);
      // we'll see what happens now.
    }


    if (!temp_obj) {

      if (myself->fight())
        return FALSE;
      if (!job->singletarg || job->missionaccomplished)
	job->hunted_victim = NULL;
#if 0
// the canSee check in findHuntedItem makes this flawed
// we are able to walk entire list fairly fast, so this is not heavily needed
      job->noneBeyond = *(object_list.begin());  // nowhere in the list
#endif

      // head home
      dir = choose_exit_global(myself->inRoom(), myself->oldRoom, 2000);
      if (dir >= MIN_DIR) {
        rc = myself->goDirection(dir);
        if (IS_SET_DELETE(rc, DELETE_THIS))
          return DELETE_THIS;
        return TRUE;
      } else {
        // no path home
        bountyPoof(myself, myself->oldRoom);
        return TRUE;
      }

      if (myself->oldRoom == myself->inRoom() && job->missionaccomplished) {
	myself->spec = 0;

	vlogf(LOG_PROC, fmt("REPO: %s finished repoing  %s of %s, deactivating.") %  myself->getName() %
	      (job->hunted_victim ? job->hunted_victim : "NULL") %
	      (job->hunted_item   ? job->hunted_item : "NULL"));
	delete job;
      }
      
    }
#if 0
// the canSee check in findHuntedItem makes this flawed
// we are able to walk entire list fairly fast, so this is not heavily needed
    job->noneBeyond = temp_obj;
#endif

    targ = dynamic_cast<TBeing *>(temp_obj->thingHolding());

    if (targ) {

      if (!job->last_targ || strcmp(targ->getName(), job->last_targ)) {
	// we switched targets or something
	vlogf(LOG_PROC, fmt("REPO: hunter %s switched targets from %s to %s, reseting chances.") % myself->getName() % 
	      (job->last_targ ? job->last_targ : "NULL") % 
	      (targ->getName() ? targ->getName() : "NULL"));
	//	strcpy(job->last_targ, targ->getName());
	job->last_targ = mud_str_dup(targ->getName());
	
        if (job->num_chances < -99 || job->num_chances > -96)
          job->num_chances = 27;
        else
          job->num_chances = -99;


      }

      TBeing *tmpch = myself->fight();
      if (tmpch) {
        // If I am not fighting who has the item, don't get sidetracked
        if (tmpch != targ) {
          act("$n has no time for this right now.", false, myself, 0, 0, TO_ROOM);
	  tmpch->stopFighting();
	  myself->stopFighting();
          TThing *toto;
          // stop all fights
          for (toto = myself->roomp->getStuff(); toto; toto = toto->nextThing) {
            TBeing *tbto = dynamic_cast<TBeing *>(toto);
            if (!tbto)
              continue;
            if (tbto->fight() != myself || tbto == myself)
              continue;
            TMonster *tbtm = dynamic_cast<TMonster *>(tbto);
            if (tbtm && !tbtm->isPc()) {
              if (tbtm->Hates(tbtm->fight(), NULL))
                tbtm->remHated(tbtm->fight(), NULL);
              tbtm->DA(10);
            }
            tbto->stopFighting();
          }
          // drop through after breaking up fight
        } else
          return FALSE;  // fighting targ, so do nothing
      }

      if (targ == myself) {
        if (temp_obj->equippedBy)
          targ->unequip(temp_obj->eq_pos);

        if (job->hunted_victim != NULL) {
          myself->doAction(job->hunted_victim, CMD_THANK);
          vlogf(LOG_PROC, fmt("REPO: %s apparently repo'd %s (%d) from %s") %  myself->getName() %
                temp_obj->getName() % temp_obj->objVnum() % (job->hunted_victim ? job->hunted_victim : "NULL"));
          job->hunted_victim = NULL;
        } else {
          act("$n appears pleased.", FALSE, myself, NULL, targ, TO_ROOM);
          vlogf(LOG_PROC, fmt("REPO: Bounty apparently repo'd %s (%d)") % 
                temp_obj->getName() % temp_obj->objVnum());
        }
        delete temp_obj;
        temp_obj = NULL;

        (job->num_retrieved)++;

        if (myself->master)
          myself->stopFollower(TRUE);

        job->warned = FALSE;
        if (job->num_chances < -99 || job->num_chances > -96)
          job->num_chances = 27;
        else
          job->num_chances = -99;

        cleanUpHunter(myself);
	if (job->singletarg) {
	  vlogf(LOG_PROC, fmt("REPO: hunter %s mission complete.") %  myself->getName());
	  job->missionaccomplished = true;
	}
        return TRUE;
      }
      if (job->hunted_victim && !job->singletarg) {
        if (strcasecmp(job->hunted_victim, targ->name)) {
          // hunt target does not = who has the item, swap and reinit

          delete [] job->hunted_victim;
          job->hunted_victim = mud_str_dup(targ->name);

          if (myself->master)
            myself->stopFollower(TRUE);
          if (job->num_chances < -99 || job->num_chances > -96)
            job->num_chances = 27;
          else
            job->num_chances = -99;
          job->warned = FALSE;
        }
      } else if (!job->singletarg) {
        // I don't seem to be hunting anyone, init
        delete [] job->hunted_victim;
        job->hunted_victim = mud_str_dup(targ->name);
        if (myself->master)
          myself->stopFollower(TRUE);
        if (job->num_chances < -99 || job->num_chances > -96)
          job->num_chances = 27;
        else
          job->num_chances = -99;
        job->warned = FALSE;
      }
    }
    room = roomOfObject(temp_obj);
    if (room == ROOM_STORAGE) {
      // item is in a linkdead bag
      // mob will wind up in CS due to teleporter which is sort of bad
      if (temp_obj->parent)
        vlogf(LOG_PROC, fmt("REPO: Bounty getting %s (%d) from %s (storage)") % 
             temp_obj->getName() % temp_obj->objVnum() % 
             temp_obj->parent->getName());
      else
        vlogf(LOG_PROC, fmt("REPO: Bounty getting %s (%d) from (storage)") % 
             temp_obj->getName() % temp_obj->objVnum());
      (*temp_obj)--;
      *myself += *temp_obj;
      return TRUE;
    } else if (room != myself->inRoom()) {
      targ = dynamic_cast<TBeing *>(temp_obj->thingHolding());
      if (targ) {
        for (aff = targ->affected; aff; aff = aff->next) {
          if (aff->type == SKILL_CONCEALMENT) {
            if (::number(1,150) < aff->modifier) {
              myself->sendTo(fmt("%s##You have lost the trail.%s\n\r") % myself->orange() % myself->norm());
   
              if (aff->be == targ) {
                act("You have successfully concealed your path from $N.",
                          FALSE, targ, 0, myself, TO_CHAR);
                return FALSE;
              } else if (targ->sameRoom(*aff->be)) {
                act("$N has successfully concealed your path from $P.",
                        FALSE, targ, myself, aff->be, TO_CHAR);
                act("You have successfully concealed $n's path from $P.",
                          FALSE, targ, myself, aff->be, TO_VICT);
                return FALSE;
              }
            }
          }
        }
      }
      // path hashing is somewhat CPU intensive, keep max range short
      // or restrict number tracking
      dir=path.findPath(myself->in_room, findRoom(room));

      if (dir == DIR_NONE) {
        // unable to find a path 
        // look for a place nearby to pop into 

        TRoom *rp, *rp2;
        if (!(rp = real_roomp(room)))
          return FALSE;
        for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
          if (!(exitp = rp->dir_option[dir]))
            continue;
          if (!(rp2 = real_roomp(exitp->to_room)))
            continue;
          if (!rp2->roomIsEmpty(TRUE))
            continue;
          if (::number(0,1))  // semi-random
            continue;

          beam_in_room = exitp->to_room;
          break;
        }
        if (dir >= MAX_DIR)
          beam_in_room = room;
        
        if (beam_in_room > 0 && myself->roomp->roomIsEmpty(TRUE)) {
          act("$n looks about for clues of passage, as if tracking someone.", 
                     0, myself, 0, 0, TO_ROOM);
          bountyPoof(myself, beam_in_room);
        } else if (room > 0) {
          // no path but room is populated
          // do nothing, wait for critters to get impatient and go away
        }
        return FALSE;
      }
      rc = myself->goDirection(dir);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    } else {
      // bounty is in same room
      if (targ) {
        myself->doWake(targ->name);
        if (!myself->circleFollow(targ)) {
          if (myself->master != targ) {
            if (myself->master)
              myself->stopFollower(TRUE);
            targ->addFollower(myself);
          }
        } else {
          // pc is following the hunter
          targ->stopFollower(TRUE);
          targ->addFollower(myself);
        }

        if (job->warned)
          job->num_chances = 0;  // goes straight to default case

        if (myself->roomp->isRoomFlag(ROOM_ARENA))
          job->num_chances = -50;

        switch (job->num_chances) {
          case -99:
            myself->doAction(targ->name, CMD_POKE);
            sprintf(buf, "I'll only ask this once, %s, hand over %s.", 
                 targ->getName(), temp_obj->getName());
            myself->doSay(buf);
	    vlogf(LOG_PROC, fmt("REPO: hunter %s gave no-mercy repo ultimatium to %s for %s") %  myself->getName() % targ->getName() % temp_obj->getName());
            myself->doSay("If you do not give the item to me, I will kill you and take it myself.");
            if (!targ->isPc())
              targ->doSay("Buzz off, creep.");
          case -98:
          case -97:
            (job->num_chances)++;
            break;
          case 27:
            myself->doAction(targ->name, CMD_POKE);
            sprintf(buf, "That's a pretty nice %s you have there, %s.", 
                 fname(temp_obj->name).c_str(), targ->getName());
            myself->doSay(buf);
	    vlogf(LOG_PROC, fmt("REPO: hunter %s gave first repo warning to %s for %s") %  myself->getName() % targ->getName() % temp_obj->getName());

            myself->doSay("If you do not give the item to me, I will kill you and take it myself.");
            if (!targ->isPc())
              targ->doSay("Buzz off, creep.");
          case 26:
          case 25:
          case 24:
          case 23:
          case 22:
          case 21:
          case 20:
          case 19:
          case 17:
          case 16:
          case 15:
          case 14:
          case 13:
          case 12:
          case 10:
          case 8:
          case 7:
          case 6:
          case 5:
          case 4:
          case 3:
          case 2:
          case 1:

            (job->num_chances)--;
            break;
          case 18:
            myself->doAction(targ->name, CMD_SNARL);
            sprintf(buf, "I'm not kidding.  Hand over %s!", temp_obj->getName());
            myself->doSay(buf);
            vlogf(LOG_PROC, fmt("REPO: hunter %s gave second repo warning to %s for %s") %  myself->getName() % targ->getName() % temp_obj->getName());

            (job->num_chances)--;
            if (!targ->isPc())
              targ->doSay("I said, BUZZ OFF!");
            break;
          case 9:
            myself->doAction(targ->name, CMD_POKE);
            sprintf(buf, "This is your *LAST* warning.  Give me %s or DIE!", 
                temp_obj->getName());
            myself->doSay(buf);
            vlogf(LOG_PROC, fmt("REPO: hunter %s gave third and final repo warning to %s for %s") %  myself->getName() % targ->getName() % temp_obj->getName());

            (job->num_chances)--;
            if (!targ->isPc())
              act("$n falls down laughing at $N.", FALSE, targ, NULL, myself, TO_ROOM);
            break;
          case -50:
            // in arena zone
            // chances are, PC is trying to avoid XP loss so discourage
            // this kind of action....
            // Bat 8-17-98
            myself->doSay("I love arena areas, I get to repossess automatically!");
            act("$n gets something from $N and laughs evilly.",
                   FALSE, myself, NULL, targ, TO_ROOM);
            if (temp_obj->parent)
              --(*temp_obj);
            else if (temp_obj->equippedBy)
              targ->unequip(temp_obj->eq_pos);

            *myself += *temp_obj; 
            break;
          case 0:
          default: {
            job->warned = TRUE;
            TMonster * tmtarg = dynamic_cast<TMonster *>(targ);
            if (tmtarg) {
	      vlogf(LOG_PROC, fmt("REPO: hunter %s doing monster repo to %s for %s") %  myself->getName() % targ->getName() % temp_obj->getName());
              act("$n puts $N in a sleeper hold.", FALSE, myself, NULL, targ, TO_ROOM);
              sprintf(buf, "$n quickly surrenders $p.");
              act(buf, FALSE, targ, temp_obj, targ, TO_ROOM);
              if (temp_obj->parent)
                --(*temp_obj);
              else if (temp_obj->equippedBy)
                targ->unequip(temp_obj->eq_pos);
              *myself += *temp_obj;

              // determine if we are a shopkeeper
              unsigned int shop_nr;
              for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != tmtarg->number); shop_nr++);

              // if repo'd from a shopkeeper, make sure we save this fact
              if (shop_nr < shop_index.size()) {
                sprintf(buf, "%s/%d", SHOPFILE_PATH, shop_nr);
                tmtarg->saveItems(buf);
		shoplog(shop_nr, myself, dynamic_cast<TMonster *>(targ), temp_obj->getName(), 0, "repoing");
              }

            } else if (targ->isLinkdead()) {
	      vlogf(LOG_PROC, fmt("REPO: hunter %s doing linkdead repo to %s for %s") %  myself->getName() % targ->getName() % temp_obj->getName());
              act("$n gets something from a linkdead player and laughs evilly.",
                   FALSE, myself, NULL, targ, TO_ROOM);
              if (temp_obj->parent)
                --(*temp_obj);
              else if (temp_obj->equippedBy)
                targ->unequip(temp_obj->eq_pos);
	      
              *myself += *temp_obj; 
            } else if (targ->getTimer() >= 2 || (targ->desc && (targ->isPlayerAction(PLR_AFK)))) {
              sprintf(buf, "%s, I see you are unresponsive.", targ->getName());
              myself->doSay(buf);
	      vlogf(LOG_PROC, fmt("REPO: hunter %s doing afk repo to %s for %s") %  myself->getName() % targ->getName() % temp_obj->getName());
              myself->doSay("I'll just relieve you of this burden and be on my way.");
              act("$n gets $o from $N.",
                   FALSE, myself, temp_obj, targ, TO_NOTVICT);
              act("$n gets $o from you.",
                   FALSE, myself, temp_obj, targ, TO_VICT);

              if (temp_obj->parent)
                --(*temp_obj);
              else if (temp_obj->equippedBy)
                targ->unequip(temp_obj->eq_pos);
              *myself += *temp_obj; 
            } else if (targ->GetMaxLevel() <= 10) {
	      sprintf(buf, "%s, I would kill you now, but you are so weak I really wont bother.", targ->getName());
              myself->doSay(buf);
              vlogf(LOG_PROC, fmt("REPO: hunter %s doing lowbie repo to %s for %s") %  myself->getName() % targ->getName() % temp_obj->getName());
              myself->doSay("I'll just relieve you of this burden and be on my way.");
              act("$n gets $o from $N.",
		  FALSE, myself, temp_obj, targ, TO_NOTVICT);
              act("$n gets $o from you.",
		  FALSE, myself, temp_obj, targ, TO_VICT);

              if (temp_obj->parent)
                --(*temp_obj);
              else if (temp_obj->equippedBy)
                targ->unequip(temp_obj->eq_pos);
              *myself += *temp_obj;

	    } else {
              if (myself->checkPeaceful("")) {
                if (::number(0, 7)) {
		  if (!::number(0,3)) {
		    sprintf(buf, "%s, you can't stay here forever. I want %s!\n\r", targ->getName(), temp_obj->getName());
		    myself->doSay(buf);
		  }
                } else {
                  sprintf(buf, "I've waited long enough, %s.  I want %s!\n\r", targ->getName(), temp_obj->getName());
                  myself->doSay(buf);
		  int loopcount=0;
		  while (myself->checkPeaceful("") && loopcount <10) {
		    loopcount++;
		    vlogf(LOG_PROC, fmt("REPO: hunter %s kicking %s out of a noheal room while getting  %s") %  myself->getName() % targ->getName() % temp_obj->getName());
		    for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
		      if (exit_ok(exitp = myself->exitDir(dir), NULL)) {
			myself->throwChar(targ, dir, FALSE, SILENT_NO, true);
			
			// verify throw succeeded
			if (!myself->sameRoom(*targ)) {
			  rc = myself->goDirection(dir);
			  if (IS_SET_DELETE(rc, DELETE_THIS))
			    return DELETE_THIS;
			  
			  rc = warCry(myself, targ, temp_obj);
			  if (rc == DELETE_VICT) {
			    delete targ;
			    targ = NULL;
			  } else if (rc == DELETE_THIS) 
			    return DELETE_THIS;
			  
			  return TRUE;
			}
		      }
		    }
		  }
                }
              } else {
                rc = warCry(myself, targ, temp_obj);
                if (rc == DELETE_VICT) {
                  delete targ;
                  targ = NULL;
                } else if (rc == DELETE_THIS) 
                  return DELETE_THIS;
              }
            }
            break;
          }  // default case
        }
      } else if (temp_obj->parent) {
        vlogf(LOG_PROC, fmt("REPO: %s repo'd %s %d from %s") % 
             myself->getName() % temp_obj->getName() % temp_obj->objVnum() % 
             temp_obj->parent->getName());
        --(*temp_obj);
        *myself += *temp_obj; 
        act("$n picks up something quickly.", FALSE, myself, NULL, 0, TO_ROOM);
      } else if (temp_obj->in_room != ROOM_NOWHERE) {
        --(*temp_obj);
        *myself += *temp_obj;
        act("$n picks up something quickly.", FALSE, myself, NULL, 0, TO_ROOM);
        vlogf(LOG_PROC, fmt("REPO: %s repo'd %s %d from room %s") % 
             myself->getName() % temp_obj->getName() % temp_obj->objVnum() % 
             myself->roomp->getName());
      } else {
        vlogf(LOG_PROC, fmt("REPO: Bounty hunter stuck looking for disconnected %s.") %  job->hunted_item);
        myself->doSay("Damn am I confused!");
        delete job;
        myself->act_ptr = NULL;
      }
    }
    return TRUE;
  } else if (job->hunted_victim != NULL) {
    if (!(targ = char_with_name(job->hunted_victim)))
      return FALSE;
    if (targ->sameRoom(*myself)) {
     myself->doWake(targ->name);
      if (!myself->circleFollow(targ)) {
        if (myself->master != targ) {
          if (myself->master)
            myself->stopFollower(TRUE);
          targ->addFollower(myself);
        }
      }
      switch (job->num_chances) {
        case 27:
          myself->doAction(targ->name, CMD_POKE);
          myself->doSay("I've been hired to kill you, but I'll be nice and give you a chance to log off first.");
          myself->doSay("I suggest you comply.");
        case 26:
	case 25:
	case 24:
	case 23:
	case 22:
	case 21:
	case 20:
	case 19:
	case 17:
	case 16:
	case 15:
	case 14:
	case 13:
	case 12:
	case 11:
	case 10:
	case 8:
        case 7:
        case 5:
        case 4:
        case 1:
        case 2:
          (job->num_chances)--;
          break;
        case 18:
          myself->doAction(targ->name, CMD_SNARL);
          myself->doSay("I'm not kidding.");
          (job->num_chances)--;
          break;
        case 9:
          myself->doAction(targ->name, CMD_POKE);
          myself->doSay("This is your *LAST* warning.");
          (job->num_chances)--;
          break;
        default:
          if (myself->checkPeaceful(""))
            myself->doSay("You can't stay here forever, you know.");
          else {
            if ((rc = myself->hit(targ)) == DELETE_VICT) {
              delete targ;
              targ = NULL;
            } else if (rc == DELETE_THIS) {
              return DELETE_THIS;
            }
          }
          break;
      }
      return TRUE;
    } else if ((dir=path.findPath(myself->in_room, findRoom(targ->in_room))) > DIR_NONE){
      rc = myself->goDirection(dir);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return TRUE;
    }
  }
  return FALSE;
}

// this is a bit of level control, 9/01 - dash
void repoCheckForRent(TBeing *ch, TObj *obj, bool corpse) {
  
  // apparently players don't like repo?? disable
  return;

  TBeing *i = NULL;
  TMonster *mob = NULL;
  int objectLevel = 0;

  if (corpse || !ch || !obj || !ch->isPc())
    return;

  int buffer = 20;
  // ok, what exactly do we want here?
  // we want to call a repo check if players have eq that is too buff for them
  // so first consideration is raw level of object vs their level
  // we want it to be a higher chance for limited objects (<50)
  if (obj->max_exist < 15)
    buffer = 10;

  objectLevel = (int) (obj->objLevel() + 0.5);
  int dif = objectLevel - ch->GetMaxLevel(); 

  if (dif > buffer) {
    // 1:500 chance for 10 lev lim diff, 1/20 chance for 50 lev lim diff
    if (((100*dif)/buffer) > ::number(0, 5000)) {
      // well this is the meat of it.. now we need to randomly pick a mob roughly the level of the
      // object and send him after.
      
      int minlev, maxlev;
      int huntergroup = ( objectLevel / 10 ) + 1;
      int nummobs;

      minlev = huntergroup * 10 + 1;
      maxlev = huntergroup * 10 + 10;

      switch (huntergroup) {
	case 0:
	  // shouldn't happen, but just to be safe;
	  nummobs = stats.act_1_5 + stats.act_6_10;
	  break;
	case 1:
	  nummobs = stats.act_11_15 + stats.act_16_20;
	  break;
	case 2:
	  nummobs = stats.act_21_25 + stats.act_26_30;
	  break;
	case 3:
	  nummobs = stats.act_31_40;
	  break;
	case 4:
	  nummobs = stats.act_41_50;
	  break;
	default:
	  nummobs = stats.act_51_60;
	  break;
      }
      
      for (i = character_list; i; i = i->next) {
	if (i->GetMaxLevel() <= maxlev && i->GetMaxLevel() >= minlev && !::number(0, nummobs)
	    && (mob = dynamic_cast<TMonster *>(i))) {
	  if (i->roomp->isRoomFlag(ROOM_NO_HEAL) || i->isImmortal() || i->spec != 0 || !(i->canSpeak())) {
	    continue;
	  }
	  // code to set up the repo mob here
	  char buf[160],buf2[160], buf3[160];
	  strcpy(buf,obj->name);
	  strcpy(buf, add_bars(buf).c_str());
	  strcpy(buf3,ch->name);
	  strcpy(buf3, add_bars(buf3).c_str());
	  sprintf(buf2,"Hunter, repo %s from %s",buf,buf3);
	  vlogf(LOG_PROC,fmt("REPO: %s rent-repoing: '%s' from %s : plev: %d, olev: %d.") % 
		i->getName() % buf % ch->getName() % ch->GetMaxLevel() % objectLevel);
	  i->spec = SPEC_BOUNTY_HUNTER;
	  bounty_hunter(NULL, CMD_SAY, buf2, mob, NULL);
	  break;
	} else {
	  continue;
	}
      }
    }
  }
  return;
}
	  
      
    
  

void repoCheck(TMonster *mob, int rnum)
{

  //players still complain like mad.  taking it out again - dash 9/15/02
  return;

#if REPO_MOBS
  char buf[160],buf2[160];
  int cur_num = obj_index[rnum].getNumber();
  int max_num = obj_index[rnum].max_exist;

  // Bounty's just cause massive griping - bat 2/23/97
  // keep it in, but at low low frequency - brut 2/27/97
  if ((cur_num >= max_num) &&
      !::number(0, ((cur_num > max_num) ? 10 : 100))) {
    // reduce hunter chance on artifacts
    if (::number(1,5) <= cur_num) {
      strcpy(buf,obj_index[rnum].name);
      strcpy(buf, add_bars(buf).c_str());
      sprintf(buf2,"Hunter, repo %s",buf);
      vlogf(LOG_PROC,fmt("REPO: %s auto-hunting: '%s' : cur:%d, max:%d.") % 
               mob->getName() %buf % cur_num % max_num);
      mob->spec = SPEC_BOUNTY_HUNTER;
      bounty_hunter(NULL, CMD_SAY, buf2, mob, NULL);

#if SUPER_REPO_MOBS
      // make supertough to enforce max-exist rules
      if (cur_num > max_num) {
        mob->setMaxHit(5*mob->hitLimit());
        mob->setHit(mob->hitLimit());
      }
#endif
    }
  }
#endif
}










