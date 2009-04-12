//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "room.h"
#include "handler.h"
#include "monster.h"
#include <deque>

class organizer_struct {
  public:
    int speech_dur;
    time_t start_time;
    bool pause;
    bool open_debate;
    bool logging;
    std::deque<sstring>speech_list;

    organizer_struct()  :
      speech_dur(-1),
      start_time(0),
      pause(false),
      open_debate(false),
      logging(false)
    {
      // deque<sstring>speech_list(0);
    }
    ~organizer_struct()
    {
      while (!speech_list.empty()) {
        // sstring nameStr = speech_list[0];
        speech_list.pop_front();
      }
    }
};

static void announceNextSpeaker(TMonster *myself, organizer_struct *job)
{
  // sstring nameStr = job->speech_list.front();
  job->speech_list.pop_front();
  job->start_time = time(0);
  if (!job->speech_list.empty()) {
    sstring tmpstr = "The next speaker is " + job->speech_list.front();
    myself->doSay(tmpstr);
  } else
    myself->doSay("There are no speakers listed.");
}

static const char ORGANIZER_ID[] = "Organizer,";

static bool checkForSay(TBeing *ch, TMonster *myself, cmdTypeT cmd, const char *arg, int *rc)
{
  if (cmd != CMD_SAY && cmd != CMD_SAY2)
    return false;

  *rc = false;
  if (!arg)
    return true;
  for (; *arg == ' '; arg++);

  organizer_struct *job;
  job = static_cast<organizer_struct *>(myself->act_ptr);
  if (!job)
    return true;

  if (!strncasecmp(arg, ORGANIZER_ID, strlen(ORGANIZER_ID))) {
    arg += strlen(ORGANIZER_ID);
    if (!strncasecmp(arg, " show list", 10)) {
      sstring tmpString;
      myself->doTell(fname(ch->name), "The current speaker list:");
      if (job->speech_list.empty()) {
        myself->doTell(fname(ch->name), "Empty");
      } else {
        unsigned int i;
        tmpString += "";
        for (i = 0; i < job->speech_list.size(); i++) {
          tmpString += job->speech_list[i];
          tmpString += " ";
        }
        myself->doTell(fname(ch->name), tmpString);
      }
      if (job->speech_dur > 0) {
        myself->doTell(fname(ch->name), format("Speech time is restricted to %d seconds.") % job->speech_dur);
        if (!job->speech_list.empty()) {
          myself->doTell(fname(ch->name), format("%s has %ld seconds remaining.") % job->speech_list[0] % (job->speech_dur +job->start_time - time(0)));
        }
      } else {
        myself->doTell(fname(ch->name), "Speech time is unrestricted.");
      }
      *rc = true;
      return true;
    } else if (!strncasecmp(arg, " add me", 7)) {
      if (!job->speech_list.empty()) {
        unsigned int i;
        for (i = 0; i < job->speech_list.size(); i++) {
          if (ch->name == job->speech_list[i]){
            myself->doTell(fname(ch->name), "You are already in the speaker list.");
            *rc = true;
            return true;
          }
        }
      } else {
        sstring tmpstr = "The next speaker is ";
        tmpstr += ch->getName();
        myself->doSay(tmpstr);
      }

      job->speech_list.push_back(ch->name);
      myself->doTell(fname(ch->name), "You have been added as a speaker.");

      // start the cpounter if we just launched
      if (job->speech_list.size() == 1)
        job->start_time = time(0);

      *rc = true;
      return true;
    } else if (!strncasecmp(arg, " done", 5)) {
      // If i am not the speaker, dump this command
      // otherwise, skip to next speaker.
      // immorts are also able to dump to next speaker
      if (!job->speech_list.empty()) {
	if(job->speech_list[0] != ch->name &&
            !ch->isImmortal())
          return false;
      } else {
        return false;
      }

      announceNextSpeaker(myself, job);
      *rc = true;
      return true;
    } else if (!strncasecmp(arg, " pause", 6) && ch->isImmortal()) {
      job->pause = !job->pause;
      if (job->pause)
        myself->doSay("Speaker clock is paused.");
      else
        myself->doSay("Speaker clock is restarted.");
      *rc = true;
      return true;
    } else if (!strncasecmp(arg, " open_debate", 12) && ch->isImmortal()) {
      job->open_debate = !job->open_debate;
      if (job->open_debate)
        myself->doSay("Open Debate is now invoked.");
      else
        myself->doSay("Open Debate is now stopped.");
      *rc = true;
      return true;
    } else if (!strncasecmp(arg, " log", 4) && ch->isImmortal()) {
      job->logging = !job->logging;
      if (job->logging)
        myself->doSay("This meeting is now logged.");
      else
        myself->doSay("This meeting is no longer being logged.");
      *rc = true;
      return true;
    } else if (!strncasecmp(arg, " speech_time ", 13) && ch->isImmortal()) {
      int sec_time = convertTo<int>(&arg[13]);
      char buf[256];
      if (sec_time > 0)
        sprintf(buf, "Speech time is now limited to %d seconds.", sec_time);
      else
        sprintf(buf, "Speech time is now unlimited.");
      myself->doSay(buf);
      job->speech_dur = sec_time;
      job->start_time = time(0);
      *rc = true;
      return true;
    } else if (!strncasecmp(arg, " clear", 6) && ch->isImmortal()) {
      while (!job->speech_list.empty()) {
        sstring nameStr = job->speech_list[0];
        job->speech_list.pop_front();
      }
      myself->doSay("The speaker list has been cleared.");
      *rc = true;
      return true;
    }
  } else {
    // we didn't say mother may I...

    // We only want to log says, so this seems like good choice of 
    // place to put this.
    // log only says that would actually go through... (immortal, or speaker)
    // open_debate is logged elsewhere
    if (job->logging &&
         (ch->isImmortal() ||
           (!job->speech_list.empty() &&
	    job->speech_list[0] == ch->name))){
      FILE *fp;
      fp = fopen("meeting.log", "a+");
      if (fp) {
        fprintf(fp, "%s : %s\n", ch->getName(), arg);
        fclose(fp);
      }
    }
  }
  return false;
}

int meeting_organizer(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  int rc = 0;

  organizer_struct *job;
 
  if (cmd == CMD_GENERIC_DESTROYED) {
    job = static_cast<organizer_struct *>(myself->act_ptr);
    if (!job)
      return FALSE;

    delete job;
    myself->act_ptr = NULL;
    return FALSE;
  }
  if (cmd == CMD_GENERIC_CREATED) {
    if (!(myself->act_ptr = new organizer_struct())) {
      perror("failed new of organizer.");
      exit(0);
    }
    return FALSE;
  }

  if (!(job = (organizer_struct *) myself->act_ptr))
    return FALSE;

  if (cmd == CMD_GENERIC_PULSE) {
    // make sure no one goes hungry
    TThing *t=NULL;

    for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end() && (t=*it);++it) {
      if (t == myself)
        continue;
      TBeing *pers = dynamic_cast<TBeing *>(t);
      if (!pers)
        continue;
      if (pers->getCond(THIRST) >= 0)
        pers->setCond(THIRST, 24);
      if (pers->getCond(FULL) >= 0)
        pers->setCond(FULL, 24);
    }

    TBeing *ch = NULL;
    for (;;) {
      if (job->speech_list.empty())
        return FALSE;

      sstring nameStr = job->speech_list.front();
      ch = get_char_room(nameStr, myself->in_room);
      if (!ch) {
        sstring tmpstr = "The current speaker, ";
        tmpstr += nameStr;
        tmpstr += ", no longer seems to be here.";
        myself->doSay(tmpstr);
        announceNextSpeaker(myself, job);
        continue;
      }
      break;
    }
    if (job->speech_dur > 0 && !job->pause) {
      sstring nameStr = job->speech_list.front();
      if ((job->start_time + job->speech_dur) < time(0)) {
        sstring tmpstr = nameStr;
        tmpstr += "'s alloted time has expired.";
        myself->doSay(tmpstr);
        announceNextSpeaker(myself, job);
      }
    }
    return FALSE;
  }

  if (!ch->isPc())
    return FALSE;

  if (job->logging &&
      (cmd == CMD_EMOTE ||
       cmd == CMD_EMOTE2 ||
       cmd == CMD_EMOTE3)) {
    // silly bastards that aliased emote to work like a say should be shot
    // it screws up meeting logs, so deny such activity
    
    myself->doTell(fname(ch->name), "Emotting is disabled at the moment.");
    return TRUE;
  }

  if (ch->isImmortal()) {
    if (checkForSay(ch, myself, cmd, arg, &rc))
      return rc;

    return FALSE;
  }

  // if debate is open, bypass
  if (job->open_debate) {
    if (job->logging && arg &&
        (cmd == CMD_SAY || cmd == CMD_SAY2)) {
      for (; *arg == ' '; arg++);

      FILE *fp;
      fp = fopen("meeting.log", "a+");
      if (fp) {
        fprintf(fp, "%s : %s\n", ch->getName(), arg);
        fclose(fp);
      }
    }
    return FALSE;
  }

  // mortal commands handled here
  if (cmd == CMD_NORTH ||
      cmd == CMD_EAST ||
      cmd == CMD_SOUTH ||
      cmd == CMD_WEST ||
      cmd == CMD_UP ||
      cmd == CMD_DOWN ||
      cmd == CMD_NE ||
      cmd == CMD_NW ||
      cmd == CMD_SE ||
      cmd == CMD_SW ||
// limited socials = limited spam
      cmd == CMD_NOD ||
      cmd == CMD_SHAKE ||
      cmd == CMD_AGREE ||
      cmd == CMD_DISAGREE ||
// allow utility as needed
// remember they should stay focused on meeting, so don't allow too much here
      cmd == CMD_TELL || // well, it's quiet
      cmd == CMD_GT || // well, it's quiet
      cmd == CMD_LOOK ||
      cmd == CMD_WORLD ||
      cmd == CMD_TIME ||
      cmd == CMD_ATTRIBUTE ||
      cmd == CMD_WHO ||
      cmd == CMD_MOTD ||
      cmd == CMD_CLS ||
      cmd == CMD_SAVE ||
      cmd == CMD_NEWS ||
      cmd == CMD_SCORE ||
      cmd == CMD_HELP)
    return FALSE;

  if (checkForSay(ch, myself, cmd, arg, &rc))
    return rc;

  // allow the speaker to do as they please
  if (!job->speech_list.empty()) {
    if(job->speech_list[0] == ch->name){
      return FALSE;
    }
  }

  myself->doTell(fname(ch->name), "To maintain order at the meeting, you are restricted to the following commands:");
  myself->doTell(fname(ch->name), "movement, utility, NOD, SHAKE, AGREE, DISAGREE");
  myself->doTell(fname(ch->name), format("To be added to the speaker list : say %s add me") % ORGANIZER_ID);
  myself->doTell(fname(ch->name), format("To review the speaker list : say %s show list") % ORGANIZER_ID);
  myself->doTell(fname(ch->name), format("To relinquish the speaker position : say %s done") % ORGANIZER_ID);

  if (ch->GetMaxLevel() > MAX_MORT) {
    myself->doTell(fname(ch->name), format("To pause/restart the speaker clock : say %s pause") % ORGANIZER_ID);
    myself->doTell(fname(ch->name), format("To open/close debate to all : say %s open_debate") % ORGANIZER_ID);
    myself->doTell(fname(ch->name), format("To set the speech time : say %s speech_time <seconds>") % ORGANIZER_ID);
    myself->doTell(fname(ch->name), format("To clear the speaker list : say %s clear") % ORGANIZER_ID);
    myself->doTell(fname(ch->name), format("To log/unlog the meeting : say %s log") % ORGANIZER_ID);
  }

  return TRUE;
}
