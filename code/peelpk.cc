//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#include <ctime>

#include "stdsneezy.h"
#include "obj_gun.h"


#define PEELPK_TEAMSIZE 12

struct TPeelPk {
  int zones;
  bool cutdam;
  sh_int zone[4];
  int respawns[2];
  int respawn[2][4];
  int respawnlag;
  int default_respawn;
  int announce;
  int teamnum[2];
  int teamscore[2];
  TBeing *teammembers[2][PEELPK_TEAMSIZE];
  int teamscores[2][PEELPK_TEAMSIZE];
  int teamdeaths[2][PEELPK_TEAMSIZE];
  int holding[2];
  time_t endtime;
} peelPk={0, false, {0,0,0,0}, {0,0}, {{0,0,0,0}, {0,0,0,0}}, 
	  0, 100, 0, {0, 0}, {0, 0},
	  {{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,},
	  {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,}},
	  {{0,0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0}},
	  {{0,0,0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0}},
	  {0,0},0};


bool TBeing::cutPeelPkDam() const
{
  if (inPkZone())
    return peelPk.cutdam;
  return FALSE;
}

bool TBeing::inPkZone() const
{
  int i;
  
  for(i=0;i<peelPk.zones;++i){
    if(roomp->getZoneNum()==peelPk.zone[i])
      return TRUE;
  }
  return FALSE;
}

// this is used in the deconstructor, so keep it clean
void TBeing::removeFromPeelPk(TBeing *c)
{
  int i, j;

  for(i=0;i<PEELPK_TEAMSIZE;++i){
    for(j=0;j<2;++j){
      if(peelPk.teammembers[j][i]==c){
	peelPk.teammembers[j][i]=NULL;
	--peelPk.teamnum[j];
      }
    }
  }
}

void TBeing::doPeelPk(const char *argument)
{
  char buf[200], buf2[200];
  int num, i, j;
  TObj *dummy;
  int valid=0;
  Descriptor *idesc;
  TBeing *b;

  half_chop(argument, buf, buf2);

  if((!*buf && !*buf2) || !hasWizPower(POWER_WIZARD)){
    if(hasWizPower(POWER_WIZARD)){
      sendTo("Syntax : peelpk <command>\n\r");
      sendTo("Syntax : peelpk <variable|command> <value>\n\r");
      sendTo("Syntax : peelpk <variable|command> <index> <value>\n\r");
      sendTo("Variables : zones, zone, respawns, respawn, announce, holding, settimer, respawnlag, default_respawn, cutdam\n\r");
      sendTo("Commands : addmember, remmember, echoscore, checktime, resetscore, resetteam, toholding, torespawn\n\r");
      sendTo(fmt("  # zones     =  %i\n\r") % peelPk.zones);

      sendTo(fmt("  zones       =  %i, %s\n\r") % peelPk.zone[0] % 
       ((peelPk.zone[0]>0 && (unsigned int) peelPk.zone[0]<zone_table.size()) ?
	                     zone_table[peelPk.zone[0]].name : "None"));
      sendTo(fmt("              =  %i, %s\n\r") % peelPk.zone[1] %
       ((peelPk.zone[1]>0 && (unsigned int) peelPk.zone[1]<zone_table.size()) ?
        	             zone_table[peelPk.zone[1]].name : "None"));
      sendTo(fmt("              =  %i, %s\n\r") % peelPk.zone[2] %
       ((peelPk.zone[2]>0 && (unsigned int) peelPk.zone[2]<zone_table.size()) ?
        	             zone_table[peelPk.zone[2]].name : "None"));
      sendTo(fmt("              =  %i, %s\n\r") % peelPk.zone[3] %
       ((peelPk.zone[3]>0 && (unsigned int) peelPk.zone[3]<zone_table.size()) ?
        	             zone_table[peelPk.zone[3]].name : "None"));

      sendTo(fmt("  announce    =  %i\n\r") % peelPk.announce);
      sendTo(fmt("  def respawn =  %i\n\r") % peelPk.default_respawn);
      sendTo(fmt("  cutdam      =  %s\n\r") % ((peelPk.cutdam)?"is ON % damage reduced by one half":"is OFF % no damage reduction"));
    }
    if(peelPk.endtime>0)
      sendTo(fmt("  time left   =  %i:%i\n\r") % ((peelPk.endtime-time(NULL))/60) %
	     ((peelPk.endtime-time(NULL))%60));

    for(j=0;j<2;++j){
      sendTo(fmt("  Team%i       =  %i members, %i score\n\r") % 
	     j % peelPk.teamnum[j] % peelPk.teamscore[j]);
      if(hasWizPower(POWER_WIZARD)){
	sendTo(fmt("  holding     =  %i, %s\n\r  ") % peelPk.holding[j] %
	       (real_roomp(peelPk.holding[j]) ?
	       real_roomp(peelPk.holding[j])->getName() : "None"));

	sendTo(fmt("  # respawns  =  %i\n\r") % peelPk.respawns[j]);

	sendTo(fmt("  respawns    =  %i, %s\n\r") % peelPk.respawn[j][0] %
	       (real_roomp(peelPk.respawn[j][0]) ?
               real_roomp(peelPk.respawn[j][0])->getName() : "None"));
	sendTo(fmt("              =  %i, %s\n\r") % peelPk.respawn[j][1] %
	       (real_roomp(peelPk.respawn[j][1]) ?
               real_roomp(peelPk.respawn[j][1])->getName() : "None"));
	sendTo(fmt("              =  %i, %s\n\r") % peelPk.respawn[j][2] %
	       (real_roomp(peelPk.respawn[j][2]) ?
               real_roomp(peelPk.respawn[j][2])->getName() : "None"));
	sendTo(fmt("              =  %i, %s\n\r") % peelPk.respawn[j][3] %
	       (real_roomp(peelPk.respawn[j][3]) ?
               real_roomp(peelPk.respawn[j][3])->getName() : "None"));
      }
      for(i=0;i<PEELPK_TEAMSIZE;++i){
	if(peelPk.teammembers[j][i])
	  sendTo(COLOR_MOBS, fmt("%s(%i / %i)  ") % peelPk.teammembers[j][i]->getName() % 
		 peelPk.teamscores[j][i] % peelPk.teamdeaths[j][i]);
      }
      sendTo("\n\r");
    }


    return;
  }
  
  if(!strcmp(buf, "zones")){
    if(convertTo<int>(buf2) > 4)
      sendTo("The number of zones has to be 4 or less\n\r");
    else 
      peelPk.zones=convertTo<int>(buf2);
  } else if(!strcmp(buf, "zone")){
    half_chop(buf2, buf, buf2);
    if((num=convertTo<int>(buf))>=peelPk.zones || num<0)
      sendTo(fmt("The zone index must be from 0 to %i.\n\r") % (peelPk.zones-1));
    else
      peelPk.zone[num]=convertTo<int>(buf2);
  } else if(!strcmp(buf, "addzone")){
    if(peelPk.zones >= 4)
      sendTo("The number of zones has to be 4 or less\n\r");
    else
      peelPk.zone[peelPk.zones++]=convertTo<int>(buf2);
  } else if(!strcmp(buf, "respawns")){
    half_chop(buf2, buf, buf2);
    peelPk.respawns[convertTo<int>(buf)]=convertTo<int>(buf2);
  } else if(!strcmp(buf, "respawn")){
    half_chop(buf2, buf, buf2);
    j=convertTo<int>(buf);
    half_chop(buf2, buf, buf2);
    if((num=convertTo<int>(buf))>=peelPk.respawns[j]){
      sendTo(fmt("The respawn index must be from 0 to %i.\n\r") % 
	     (peelPk.respawns[j]-1));
    } else
      peelPk.respawn[j][num]=convertTo<int>(buf2);
  } else if(!strcmp(buf, "announce")){
    peelPk.announce=convertTo<int>(buf2);
  } else if(!strcmp(buf, "addmember")){
    half_chop(buf2, buf, buf2);
    num=convertTo<int>(buf);
    for(i=0;i<PEELPK_TEAMSIZE;++i){
      if(!peelPk.teammembers[num][i]){

	if (!generic_find(buf2, FIND_CHAR_WORLD, this, 
			  &peelPk.teammembers[num][i], &dummy)){
	  sendTo("Couldn't find any such creature.\n\r");
	  peelPk.teammembers[num][i]=NULL;
	  return;
	} else if (dynamic_cast<TMonster *>(peelPk.teammembers[num][i])){
	  sendTo("Can't do that to a beast.\n\r");
	  peelPk.teammembers[num][i]=NULL;
	  return;
	}

	++peelPk.teamnum[num];
	return;
      }
    }
  } else if(!strcmp(buf, "remmember")){
    half_chop(buf2, buf, buf2);
    num=convertTo<int>(buf);

    if (!generic_find(buf2, FIND_CHAR_WORLD, this,
		      &b, &dummy)){
      sendTo("Couldn't find any such creature.\n\r");
      return;
    } 

    for(i=0;i<PEELPK_TEAMSIZE;++i){
      if(b==peelPk.teammembers[num][i]){
	peelPk.teammembers[num][i]=NULL;
	
	--peelPk.teamnum[num];
      }
    }
  } else if(!strcmp(buf, "echoscore")){
    for (idesc = descriptor_list; idesc; idesc = idesc->next) {
      if ((b = idesc->character) && !idesc->connected &&
	  b->awake() &&
	  (dynamic_cast<TMonster *>(b) ||
	   (b->desc && !IS_SET(idesc->autobits, AUTO_NOSHOUT))) &&
	  !b->checkSoundproof() && 
	  !b->isPlayerAction(PLR_MAILING | PLR_BUGGING)){
	for(i=0;i<peelPk.zones;++i){
	  if(b->roomp->getZoneNum()==peelPk.zone[i]){
	    valid=1;
	    break;
	  }
	}
	if(valid){
	  b->sendTo(fmt("PkQuest: Team1: %i players, %i score\n\r") % 
		    peelPk.teamnum[0] % peelPk.teamscore[0]);
	  b->sendTo(fmt("PkQuest: Team2: %i players, %i score\n\r") % 
		    peelPk.teamnum[1] % peelPk.teamscore[1]);
	  if(peelPk.endtime>0 && (peelPk.endtime-time(NULL))>0)
	    b->sendTo(fmt("PkQuest: Remaining time: %i:%i\n\r") % 
		      ((peelPk.endtime-time(NULL))/60) %
		      (peelPk.endtime-time(NULL))%60);
	}
	valid=0;
      }
    }    
  } else if(!strcmp(buf, "holding")){
    half_chop(buf2, buf, buf2);
    peelPk.holding[convertTo<int>(buf)]=convertTo<int>(buf2);
  } else if (!strcmp(buf, "cutdam")) {
    if (!strcmp(buf2, "on")) peelPk.cutdam = TRUE;
    else if (!strcmp(buf2, "off")) peelPk.cutdam = FALSE;
    else sendTo("syntax: peelpk cutdam <on | off>");
    sendTo(fmt("PkQuest: %s\n\r") % ((peelPk.cutdam)?"Damage cut by 50 percent":"No damage modification"));
  } else if(!strcmp(buf, "respawnlag")){
    peelPk.respawnlag=convertTo<int>(buf2);
  } else if(!strcmp(buf, "settimer")){
    peelPk.endtime=time(NULL)+convertTo<int>(buf2)*60;
  } else if(!strcmp(buf, "checktime")){
    if(peelPk.endtime<=time(NULL) && peelPk.zones>0){
      peelPk.endtime=0;
      sendTo("Time is up, transferring teams.\n\r");
      for(j=0;j<2;++j){
	if(peelPk.holding[j]>0){
	  for(i=0;i<PEELPK_TEAMSIZE;++i){
	    if(peelPk.teammembers[j][i]){
	      peelPk.teammembers[j][i]->sendTo("PkQuest: Time is up, transferring to holding room.\n\r");
	      --(*peelPk.teammembers[j][i]);
	      thing_to_room(peelPk.teammembers[j][i], peelPk.holding[j]);
	    }
	  }
	}
      }
    } else {
      sendTo(fmt("Time to go: %i:%i\n\r") %
	     ((peelPk.endtime-time(NULL))/60) % 
	     (peelPk.endtime-time(NULL))%60);
    }
  } else if(!strcmp(buf, "default_respawn")){
    peelPk.default_respawn=convertTo<int>(buf2);
  } else if(!strcmp(buf, "resetscore")){
    for(j=0;j<2;++j){
      for(i=0;i<PEELPK_TEAMSIZE;++i){
	peelPk.teamscores[j][i]=0;
	peelPk.teamdeaths[j][i]=0;
      }
      peelPk.teamscore[j]=0;
    }
  } else if(!strcmp(buf, "resetteams")){
    for(j=0;j<2;++j){
      for(i=0;i<PEELPK_TEAMSIZE;++i){
	peelPk.teammembers[j][i]=NULL;	  
      }
      peelPk.teamnum[j]=0;
    }
  } else if(!strcmp(buf, "toholding")){
    if(peelPk.zones>0){
      sendTo("Transferring teams to holding rooms.\n\r");
      for(j=0;j<2;++j){
	if(peelPk.holding[j]>0){
	  for(i=0;i<PEELPK_TEAMSIZE;++i){
	    if(peelPk.teammembers[j][i]){
	      peelPk.teammembers[j][i]->sendTo("Transferring to holding room.\n\r");
	      --(*peelPk.teammembers[j][i]);
	      thing_to_room(peelPk.teammembers[j][i], peelPk.holding[j]);
	    }
	  }
	}
      }
    }
  } else if(!strcmp(buf, "torespawn")){
    if(peelPk.zones>0){
      sendTo("Transferring teams to respawn rooms.\n\r");
      for(j=0;j<2;++j){
	if(peelPk.respawn[j][0]>0){
	  for(i=0;i<PEELPK_TEAMSIZE;++i){
	    if(peelPk.teammembers[j][i]){
	      peelPk.teammembers[j][i]->sendTo("Transferring to respawn room.\n\r");
	      --(*peelPk.teammembers[j][i]);
	      thing_to_room(peelPk.teammembers[j][i], peelPk.respawn[j][0]);
	    }
	  }
	}
      }
    }
  } 
} 

void dropAllGunsAndAmmo(TBeing *ch, TThing *stuff){
  TThing *tt;

  if(!stuff || !ch)
    return;

  for(tt=stuff;tt;tt=tt->nextThing){
    if(tt->getStuff())
      dropAllGunsAndAmmo(ch, tt->getStuff());

    if(dynamic_cast<TGun *>(tt) ||
       dynamic_cast<TAmmo *>(tt)){
      // found one, so drop it
      (*tt)--;
      *ch->roomp += *tt;

      dropAllGunsAndAmmo(ch, ch->getStuff());
      break;
    }
  }
}

// returns DELETE_THIS
int TBeing::peelPkRespawn(TBeing *killer, spellNumT dmg_type)
{
  int i, valid=0, team=-1;
  Descriptor *idesc;
  TBeing *b, *k;

  if(peelPk.zones<=0 || peelPk.respawns<=0 || !isPc()){
    return FALSE;
  }
  
  for(i=0;i<peelPk.zones;++i){
    if(roomp->getZoneNum()==peelPk.zone[i]){
      valid=1;
      break;
    }
  }

  if(!valid)
    return FALSE;
  valid=0;

  for(i=0;i<PEELPK_TEAMSIZE;++i){
    if(this==peelPk.teammembers[0][i]){
      team=0;
      peelPk.teamdeaths[0][i]++;
      break;
    }
    if(this==peelPk.teammembers[1][i]){
      team=1;
      peelPk.teamdeaths[1][i]++;
      break;
    }
  }

  if(killer->isPc()){
    for(i=0;i<PEELPK_TEAMSIZE;++i){
      if(peelPk.teammembers[0][i]==killer){
	if(team==1){
	  peelPk.teamscore[0]++;
	  peelPk.teamscores[0][i]++;
	} else {
	  peelPk.teamscore[0]--;
	  peelPk.teamscores[0][i]--;
	}
	break;
      } else if(peelPk.teammembers[1][i]==killer){
	if(team==0){
	  peelPk.teamscore[1]++;
	  peelPk.teamscores[1][i]++;
	} else {
	  peelPk.teamscore[1]--;
	  peelPk.teamscores[1][i]--;
	}
	break;
      }
    }
  } else { 
    for(i=0;i<PEELPK_TEAMSIZE;++i){
      if(peelPk.teammembers[0][i]==this){
	peelPk.teamscore[0]--;
	peelPk.teamscores[0][i]--;
	break;
      } else if(peelPk.teammembers[1][i]==this){
	peelPk.teamscore[1]--;
	peelPk.teamscores[1][i]--;
	break;
      }
    }    
  }

  if(peelPk.announce){
    b=this;
    b->sendTo(COLOR_MOBS, fmt("PkQuest: %s killed by %s\n\r") % getName() % killer->getName());
    b->sendTo(fmt("PkQuest: Team1: %i players, %i score\n\r") % 
	      peelPk.teamnum[0] % peelPk.teamscore[0]);
    b->sendTo(fmt("PkQuest: Team2: %i players, %i score\n\r") % 
	      peelPk.teamnum[1] % peelPk.teamscore[1]);
    if(peelPk.endtime<=0 && (peelPk.endtime-time(NULL))>0)
      b->sendTo(fmt("PkQuest: Remaining time: %i:%i\n\r") % 
		((peelPk.endtime-time(NULL))/60) %
		(peelPk.endtime-time(NULL))%60);

    for (idesc = descriptor_list; idesc; idesc = idesc->next) {
      if ((b = idesc->character) && !idesc->connected &&
	  b->awake() &&
	  (dynamic_cast<TMonster *>(b) ||
	   (b->desc && !IS_SET(idesc->autobits, AUTO_NOSHOUT))) &&
	  !b->checkSoundproof() && 
	  !b->isPlayerAction(PLR_MAILING | PLR_BUGGING)){
	for(i=0;i<peelPk.zones;++i){
	  if(b->roomp->getZoneNum()==peelPk.zone[i]){
	    valid=1;
	    break;
	  }
	}
	if(valid){
	  b->sendTo(COLOR_MOBS, fmt("PkQuest: %s killed by %s\n\r") % getName() % killer->getName());
	  b->sendTo(fmt("PkQuest: Team1: %i players, %i score\n\r") % 
		    peelPk.teamnum[0] % peelPk.teamscore[0]);
	  b->sendTo(fmt("PkQuest: Team2: %i players, %i score\n\r") % 
		    peelPk.teamnum[1] % peelPk.teamscore[1]);
	  if(peelPk.endtime<=0 && (peelPk.endtime-time(NULL))>0)
	    b->sendTo(fmt("PkQuest: Remaining time: %i:%i\n\r") % 
		      ((peelPk.endtime-time(NULL))/60) %
		      (peelPk.endtime-time(NULL))%60);
	}
	valid=0;
      }
    }
  }

  deathCry();
  makeCorpse(dmg_type);

  dropAllGunsAndAmmo(this, getStuff());



  
  --(*this);
  if(team<0 || peelPk.respawns[team]<=0)
    thing_to_room(this, peelPk.default_respawn);
  else 
    thing_to_room(this, peelPk.respawn[team][::number(0,peelPk.respawns[team]-1)]);

  // theoretically, we could die again here because of the embeded 
  // spellWearOff.  I think it is impossible based on how it gets called
  // but JIC...
  int rc = genericRestore(RESTORE_FULL);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  for (k = character_list; k; k = k->next) {
    if (k->specials.hunting) {
      if (k->specials.hunting == this) {
        k->specials.hunting = NULL;
        if (k->affectedBySpell(SKILL_TRACK)) {
          k->sendTo(COLOR_MOBS, fmt("You stop tracking %s.\n\r") % getName());
          k->affectFrom(SKILL_TRACK);
        }
      }
    }
    TMonster *tmons = dynamic_cast<TMonster *>(k);
    if (tmons) {
      if (tmons->Hates(this, NULL))
        tmons->remHated(this, NULL);
 
      if (tmons->Fears(this, NULL))
        tmons->remFeared(this, NULL);
 
      if (tmons->targ() == this)
        tmons->setTarg(NULL);

      if (tmons->opinion.random == this)
        tmons->opinion.random = NULL;
    }
  }


  act("You wake up and try to clear your thoughts.  Maybe it was just a dream...", FALSE, this, NULL, NULL, TO_CHAR);
  act("You suddenly notice $n sitting here.  Odd, you didn't notice $m before.  $n looks confused.", FALSE, this, NULL, NULL, TO_ROOM);

  addToWait(peelPk.respawnlag);
 
  return TRUE;
}


