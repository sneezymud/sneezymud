#include <cmath>
#include "tweaks.h"
#include "database.h"
#include "being.h"


tweakEntry::tweakEntry() 
{
}

tweakEntry *tweakInfoT::operator[] (const tweakTypeT t)
{
  return tweaks.at(t);
}

tweakTypeT & operator++(tweakTypeT &c, int)
{
  return c = (c == MAX_TWEAK_TYPES) ? TWEAK_LOADRATE : tweakTypeT(c+1);
}

tweakInfoT::tweakInfoT(): 
    tweaks(MAX_TWEAK_TYPES + 1, nullptr)
{
}

tweakInfoT::~tweakInfoT()
{
  for (auto t : tweaks)
    delete t;
}

int tweakInfoT::loadTweaks()
{
  TDatabase db(DB_SNEEZY);

  //On load fill out all tweaks with 1.0.  This will make adding new tweaks easier by eliminating the need to write a migration.
  for(tweakTypeT i=TWEAK_LOADRATE;i<MAX_TWEAK_TYPES;i++){
    tweaks[i] = new tweakEntry();
  }

  //loadrate works well at 112.5
  tweaks[TWEAK_LOADRATE]->current = tweaks[TWEAK_LOADRATE]->target = 112.5;

  //the globaltweaks table contains multiple rows per tweak type
  //We only want to keep the last tweak update of each tweak type in memory, so we do a greatest n per group query
  db.query("select tweak_id, tweak_type, tweak_value, tweak_target, tweak_rate from globaltweaks a "
           "where tweak_id = (select max( tweak_id ) from globaltweaks b where a.tweak_type = b.tweak_type)"
  );
  //enter in the proper values. Is it possible for the query to fail and the mud still be running?
  int i = 0;
  while(db.fetchRow()){
    tweakTypeT tweak_id=(tweakTypeT) convertTo<int>(db["tweak_type"]);
    int tid=(int) convertTo<int>(db["tweak_id"]);
    double val=convertTo<double>(db["tweak_value"]);
    double tar=convertTo<double>(db["tweak_target"]);
    double rate=convertTo<double>(db["tweak_rate"]);
    tweaks[tweak_id]->id = tid;
    tweaks[tweak_id]->current = val;
    tweaks[tweak_id]->target = tar;
    tweaks[tweak_id]->rate = rate;
    i++;
  }

  loaded=true;
  return i;
}

tweakInfoT tweakInfo;

//recieved after immortal enters 'info tweak <arg>'
void tweakInfoT::doTweak(TBeing *b, sstring arg)
{
  sstring arg2;
  arg=one_argument(arg, arg2);

  if (arg2.empty()) {  //user just typed info tweak
    showTweaks(b);
    return;
  }

  //determine type of tweak
  tweakTypeT t;
  for(t=TWEAK_LOADRATE;t<MAX_TWEAK_TYPES;t++){
    if (is_abbrev(arg2, getTweakName(t))) {
      //a tweak type has been found
      if (arg.empty()) {
        showTweaks(b, t);
        return;
      }
      else {
        handleTweak(b, t, arg);
        return;
      }
    }
  }

  //if they got here they typed info tweak gibberish
  showTweaks(b);
  showTweakOptions(b);
  return;
}

//deal with options for specific tweak t
void tweakInfoT::handleTweak(TBeing *b, tweakTypeT t, sstring arg){
  sstring opt;
  arg=one_argument(arg, opt);
  tweakEntry *tweak = tweakInfo[t];
  TDatabase sneezy(DB_SNEEZY);

  //only one arguements given after type
  if (arg.empty()) { 
    if (is_abbrev(opt, "up") || is_abbrev(opt, "down")) {
      tweak->target += is_abbrev(opt, "up") ? .05 : -.05;
      tweak->rate = fabs((tweak->target - tweak->current)/30);
      b->sendTo(format("You tweaked %s a tad.\r\n") % getTweakName(t));
      sneezy.query("insert into globaltweaks (tweak_type, tweak_value, tweak_target, tweak_rate) values (%i,%f,%f,%f)", 
        t, tweak->current, tweak->target,tweak->rate);
      sneezy.query("select max( tweak_id ) as tid from globaltweaks");
      if (sneezy.fetchRow()) {
        tweak->id = (int) convertTo<int>(sneezy["tid"]);
      }
    }
    else if (is_abbrev(opt, "history")) {
      b->sendTo(COLOR_BASIC, format("\n\r<c>%s History<1>\n\r") % getTweakName(t));
      b->sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
      b->sendTo(COLOR_BASIC, format("<c>%-15f | %-12f | %-12f | %-24s <1>\n\r")
                                    % "Value" % "Target" % "Change Rate" % "Date");
      sneezy.query("select tweak_id, tweak_type, tweak_value, tweak_target, tweak_rate, datecreated from globaltweaks where tweak_type = %i", t);
      while(sneezy.fetchRow()){
        double val=convertTo<double>(sneezy["tweak_value"]);
        double tar=convertTo<double>(sneezy["tweak_target"]);
        double rate=convertTo<double>(sneezy["tweak_rate"]);
        b->sendTo(COLOR_BASIC, format("%-15f | %-12f | %-12f | %-24s\n\r")
                                    % val % tar % rate % sneezy["datecreated"]);
      }
      b->sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
    }
    else {
      showTweakOptions(b);
    }
  }

  //multiple arguements given after type
  else if ( is_abbrev(opt, "set") ){
    sstring usage = "Usage: info tweak <parameter> set <target> [transition time in seconds]\r\n";

    arg = one_argument(arg,opt);

    if (!is_number(opt) || convertTo<double>(opt)<0) {
      b->sendTo("Invalid value given for global potential. Must be a number >= 0.0.\r\n "+usage);
      return;
    }

    auto targ = convertTo<double>(opt);
    if (arg.empty()){
      // No change period given-just set it immediately. We're done.
      tweak->current = tweak->target = targ;
      vlogf(LOG_MISC, format("%s set %s to %f") % b->getName() % getTweakName(t) % tweak->current);
      b->sendTo(format("Target global %s set to %f\r\n") % getTweakName(t) % tweak->target);
      sneezy.query("insert into globaltweaks (tweak_type, tweak_value, tweak_target, tweak_rate) values (%i,%f,%f,0.0)", 
        t, tweak->current, tweak->target, 0);
      sneezy.query("select max( tweak_id ) as tid from globaltweaks");
      if (sneezy.fetchRow()) {
        tweak->id = (int) convertTo<int>(sneezy["tid"]);
      }
      return;
    }

    arg = one_argument(arg,opt);
    if (!is_number(opt) || convertTo<double>(opt)<=0) {
      b->sendTo("Invalid value given for transition time. Must be a positive number of seconds.\r\n "+usage);
      return;
    }

    tweak->target = targ;
    int change_period_seconds = convertTo<int>(opt);
    tweak->rate = fabs((tweak->target - tweak->current)/change_period_seconds);
    b->sendTo(format("Target global %s set at %f to change over %f seconds.\r\n") % getTweakName(t) % tweak->target % convertTo<int>(opt) );
    vlogf(LOG_MISC, format("%s set the target global %s at %f to change over %f seconds.\r\n") % b->getName() % getTweakName(t) % tweak->target % convertTo<int>(opt) );
    sneezy.query("insert into globaltweaks (tweak_type, tweak_value, tweak_target, tweak_rate) values (%i,%f,%f,%f)", 
        t, tweak->current, tweak->target,tweak->rate);

    //proc tweak rates does not create any new rows in db, only updates current.  To make history understandable, enter
    //two rows - the first indicates the change date, the second holds the current tweak struct
    sneezy.query("insert into globaltweaks (tweak_type, tweak_value, tweak_target, tweak_rate) values (%i,%f,%f,%f)", 
        t, tweak->current, tweak->target,tweak->rate);
    sneezy.query("select max( tweak_id ) as tid from globaltweaks");
    if (sneezy.fetchRow()) {
      tweak->id = (int) convertTo<int>(sneezy["tid"]);
    }
  }
  else {
    showTweaks(b, t);
  }
}

void tweakInfoT::showTweaks(TBeing *b, tweakTypeT t){
  b->sendTo(COLOR_BASIC, "\n\r<c>Global Tweaks<1>\n\r");
  b->sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
  b->sendTo(COLOR_BASIC, format("<c>%-22s : %-15f | %-15f | %-15f <1>\n\r") % "Tweak"
  % "Current Value" % "Target" % "Change Rate");
  if (t == MAX_TWEAK_TYPES){
    for(tweakTypeT i=TWEAK_LOADRATE;i<MAX_TWEAK_TYPES;i++){
      b->sendTo(COLOR_BASIC, format("%-22s : %-15f | %-15f | %-15f \n\r") %
        tweakInfo.getTweakName(i) % tweakInfo[i]->current % tweakInfo[i]->target % tweakInfo[i]->rate);
    }
    b->sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
  }
  else {
    b->sendTo(COLOR_BASIC, format("%-22s : %-15f | %-15f | %-15f \n\r") %
      tweakInfo.getTweakName(t) % tweakInfo[t]->current % tweakInfo[t]->target % tweakInfo[t]->rate);
    b->sendTo(COLOR_BASIC, "<c>-----------------------------------------------------------------------------<1>\n\r");
  }
}

void tweakInfoT::showTweakOptions(TBeing *b){
  sstring str = "\r\nUsage: Display or modifies the global potentials\r\n\r\n";
  str += "\tinfo tweak <parameter>\r\n";
  str += "\tinfo tweak <parameter> [up|down|history]\r\n";
  str += "\tinfo tweak <parameter> set <target> <transition time in seconds>\r\n";
  b->sendTo(str);
}

//no spaces - they will mess up parsing. ideally < 20 characters to be pretty
const sstring tweakInfoT::getTweakName(tweakTypeT tt){
  switch (tt) {
    case TWEAK_LOADRATE:
      return "LoadRate";
    case TWEAK_BURNRATE:
      return "BurnRate";
    case TWEAK_FREEZEDAMRATE:
      return "FreezeDamChance";
    case MAX_TWEAK_TYPES:
      return "";
  }
  assert(("Overran gettweakname in tweaks.cc"));
  return "";
}
