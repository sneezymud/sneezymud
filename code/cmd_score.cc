//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "format.h"

void TBeing::doScore()
{
  struct time_info_data playing_time;
  sstring Buf, tString;

  sendTo(fmt("You have %s%d%s/%s%d%s hit points, %s%d%s/%s%d%s moves and ") %	 red() % getHit() % norm() %
	 green() % hitLimit() % norm() %
	 purple() % getMove() % norm() %
	 green() % moveLimit() % norm());

  if (hasClass(CLASS_DEIKHAN) || hasClass(CLASS_CLERIC))
    Buf = fmt("%s%.2f%c %spiety.\n\r") % cyan() % getPiety() % '%' % norm();
  else if (hasClass(CLASS_SHAMAN))
    Buf = fmt("%s%d %slifeforce.\n\r") % red() % getLifeforce() % norm();
  else
    Buf = fmt("%s%d%s/%s%d%s mana.\n\r") %
      orange() % getMana() % norm() %
      green() % manaLimit() % norm();

  sendTo(Buf);

  sendTo(fmt("You are %s.\n\r") % DescMoves((((double) getMove()) / ((double)         moveLimit()))));

  tString = displayExp().comify();

  sendTo(fmt("You have %s%s%s exp, and have %s%d%s talens plus %s%d%s talens in the bank.\n\r") % cyan() % tString % norm() %
         purple() % getMoney() % norm() %
         purple() % getBank() % norm());

  if (desc) {
    tString = (fmt("%.2f") % desc->session.xp).comify();

    sendTo(fmt("You have earned %s%s%s exp this session.\n\r") % cyan() % tString % norm());

    if (getExp() < getMaxExp()) {
      tString = (fmt("%.2f") % getMaxExp()).comify();
      sendTo(fmt("Your most exp before your last death was: %s%s%s\n\r") % cyan() % tString % norm());
    }

    realTimePassed((time(0) - desc->session.connect), 0, &playing_time);
    if (playing_time.day)
      playing_time.hours += playing_time.day * 24;

    sendTo(fmt("You have been playing for %s%d%s hour%s, %s%d%s minute%s and %s%d%s second%s in this session.\n\r") %         purple() % playing_time.hours   % norm() % (playing_time.hours   == 1 ? "" : "s") %
         purple() % playing_time.minutes % norm() % (playing_time.minutes == 1 ? "" : "s") %
         purple() % playing_time.seconds % norm() % (playing_time.seconds == 1 ? "" : "s"));
  }

  realTimePassed((time(0) - player.time.logon) + player.time.played,
                 0, &playing_time);

  sendTo(fmt("For a lifetime total of %s%d%s day%s and %s%d%s hour%s.\n\r") %         purple() % playing_time.day   % norm() % (playing_time.day   == 1 ? "" : "s") %
         purple() % playing_time.hours % norm() % (playing_time.hours == 1 ? "" : "s"));

  classIndT i;
  // since XP tables are all the same, the only time this should be
  // different is if they have gained in one class, but not another
  // we first check for this situation
  bool allClassesSame = true;
  for (i = MAGE_LEVEL_IND; i < MAX_CLASSES; i++)
    if (getLevel(i) && getLevel(i) != GetMaxLevel())
      allClassesSame = false;

  if (allClassesSame)
    sendTo(fmt("Your level: %s lev %2d          This ranks you as:\n\r") %           getProfName() % GetMaxLevel());
  else {
    sendTo("Your level: ");
    bool shownFirst = false;
    for (i = MAGE_LEVEL_IND; i < MAX_CLASSES; i++) {
      if (getLevel(i)) {
        sendTo(fmt("%s%s lev %2d") %             (shownFirst ? ", " : "") %
	       classInfo[i].name.cap() % getLevel(i));
        shownFirst = true;
      }
    }
    sendTo("          This ranks you as:\n\r");
  }

  char tbuf[256];
  parseTitle(tbuf, desc);
  Buf = tbuf;
  Buf += "\n\r";
  // Done this way just in case the player uses a % in their title.
  sendTo(COLOR_BASIC, fmt("%s") % Buf);

  for (i = MAGE_LEVEL_IND; i < MAX_CLASSES; i++) {
    if (getLevel(i) && getLevel(i) < MAX_MORT) {
      double need = getExpClassLevel(i, getLevel(i) + 1) - getExp();
      tString = (fmt("%.2f") % need).comify();

      if (allClassesSame) {
        sendTo(fmt("You need %s%s%s experience points to be a %sLevel %d %s%s.\n\r") %             purple() % tString % norm() % purple() % (getLevel(i)+1) %
	       getProfName() % norm());
        break;
      } else {
        // leveled in one class, but not another, show each class as own line
        sendTo(fmt("You need %s%s%s experience points to be a %sLevel %d %s%s.\n\r") %             purple() % tString % norm() % purple() % (getLevel(i)+1) %
             classInfo[i].name.cap() % norm());
      }
    }
  }

  if (!getCond(THIRST))
    sendTo(COLOR_BASIC, "<R>You are totally parched.<1>\n\r");
  else if ((getCond(THIRST) > 0) && (getCond(THIRST) <= 5))
    sendTo("Your throat is very dry.\n\r");
  else if ((getCond(THIRST) > 5) && (getCond(THIRST) <= 10))
    sendTo("You could use a little drink.\n\r");
  else if ((getCond(THIRST) > 10) && (getCond(THIRST) <= 20))
    sendTo("You are slightly thirsty.\n\r");
  else if (getCond(THIRST) > 20)
    sendTo("Your thirst is the least of your worries.\n\r");

  if (!getCond(FULL))
    sendTo(COLOR_BASIC, "<R>You are totally famished.<1>\n\r");
  else if ((getCond(FULL) > 0) && (getCond(FULL) <= 5))
    sendTo("Your stomach is growling loudly.\n\r");
  else if ((getCond(FULL) > 5) && (getCond(FULL) <= 10))
    sendTo("You could use a little bite to eat.\n\r");
  else if ((getCond(FULL) > 10) && (getCond(FULL) <= 20))
    sendTo("You are slightly hungry.\n\r");
  else if (getCond(FULL) > 20)
    sendTo("Your hunger is the least of your worries.\n\r");

  if (getCond(DRUNK) >= 20)
    sendTo("You are VERY drunk.\n\r");
  else if (getCond(DRUNK) >= 15)
    sendTo("You are very drunk.\n\r");
  else if (getCond(DRUNK) >= 10)
    sendTo("You are drunk.\n\r");
  else if (getCond(DRUNK) >= 4)
    sendTo("You are intoxicated.\n\r");
  else if (getCond(DRUNK) > 0)
    sendTo("You are feeling tipsy.\n\r");

  if (fight())
    act("You are fighting $N.", FALSE, this, NULL, fight(), TO_CHAR);
  else if (task) {
    sendTo(fmt("You are %s.\n\r") % tasks[task->task].name);
  } else {
    TBeing *tbr;
    switch (getPosition()) {
      case POSITION_DEAD:
        sendTo("You are DEAD!\n\r");
        break;
      case POSITION_MORTALLYW:
        sendTo("You are mortally wounded!  You should seek help!\n\r");
        break;
      case POSITION_INCAP:
        sendTo("You are incapacitated, slowly fading away.\n\r");
        break;
      case POSITION_STUNNED:
        sendTo("You are stunned! You can't move.\n\r");
        break;
      case POSITION_SLEEPING:
        if (riding) {
          Buf="You are sleeping on ";

          if (riding->getName())
            Buf+=objs(riding);
          else
            Buf+="A bad object";

          Buf+=".\n\r";
        } else
          Buf="You are sleeping.\n\r";

        sendTo(Buf);
        break;
      case POSITION_RESTING:
        if (riding) {
          Buf = "You are resting on ";
          if (riding->getName())
            Buf+=objs(riding);
          else
            Buf+="A horse with a bad short description, BUG THIS!";

          Buf+=".\n\r";
        } else
          Buf = "You are resting.\n\r";

        sendTo(Buf);
        break;
      case POSITION_CRAWLING:
        sendTo("You are crawling.\n\r");
        break;
      case POSITION_SITTING:
        if (riding) {
          Buf="You are sitting on ";
          if (riding->getName())
            Buf += objs(riding);
          else
	    Buf += "A bad object!";

          Buf += ".\n\r";
        } else
          Buf="You are sitting.\n\r";

        sendTo(Buf);
        break;
      case POSITION_FLYING:
         if (roomp && roomp->isUnderwaterSector())
          sendTo("You are swimming about.");
        else
          sendTo("You are flying about.\n\r");

        break;
      case POSITION_STANDING:
        sendTo("You are standing.\n\r");
        break;
      case POSITION_MOUNTED:
        tbr = dynamic_cast<TBeing *>(riding);
        if (tbr && tbr->horseMaster() == this) {
	  Buf = "You are here, riding ";
	  Buf += pers(tbr);
	  Buf += ".\n\r";
          sendTo(COLOR_MOBS, Buf);
        } else if (tbr) {
	  Buf = fmt("You are here, also riding on %s's %s%s.\n\r") %
	    pers(tbr->horseMaster()) %
	    persfname(tbr) %
	    (tbr->isAffected(AFF_INVISIBLE) ? " (invisible)" : "");

          sendTo(COLOR_MOBS, Buf);
        } else
          sendTo("You are standing.\n\r");
        break;
      default:
        sendTo("You are floating.\n\r");
        break;
    }
  }

  sendTo(fmt("You are in %s%s%s attack mode.\n\r") %         cyan() % attack_modes[getCombatMode()] % norm());

  if (getWimpy())
    sendTo(fmt("You are in wimpy mode, and will flee at %d hit points.\n\r") %
	   getWimpy());

  describeLimbDamage(this);
  sendTo(COLOR_BASIC, describeAffects(this, SHOW_ME));
}
