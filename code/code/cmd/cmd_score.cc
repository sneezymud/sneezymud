//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#include "extern.h"
#include "room.h"
#include "being.h"
#include "weather.h"

void TBeing::doScore()
{
  time_info_data playing_time;
  sstring Buf, tString;

  Buf = format("You have %s%d%s/%s%d%s hit points, ") % red() % getHit() % norm() % green() % hitLimit() % norm();

  if (hasClass(CLASS_DEIKHAN) || hasClass(CLASS_CLERIC)) {
    Buf+=format("%s%.2f%s %spiety, ") % cyan() % getPiety() % "%" % norm();
  }

  if (hasClass(CLASS_SHAMAN)) {
    Buf+=format("%s%d %slifeforce, ") % red() % getLifeforce() % norm();
  }

  if (hasClass(CLASS_MAGE) || hasClass(CLASS_MONK) || hasQuestBit(TOG_PSIONICIST)) {
    Buf+=format("%s%d%s/%s%d%s mana, ") % orange() %  getMana() % norm() % green() % manaLimit() % norm();
  }

  Buf+=format("and %s%d%s/%s%d%s moves.\n\r") % purple() % getMove() % norm() % green() % moveLimit()% norm();
  sendTo(Buf);

  sendTo(format("You are %s.\n\r") % DescMoves((((double) getMove()) / ((double) moveLimit()))));

  tString = displayExp().comify();

  sendTo(format("You have %s%s%s exp, and have %s%d%s talens plus %s%d%s talens in the bank.\n\r") % cyan() % tString % norm() %
         purple() % getMoney() % norm() %
         purple() % getBank() % norm());

  if (desc) {
    tString = ((sstring)(format("%.2f") % desc->session.xp)).comify();

    sendTo(format("You have earned %s%s%s exp this session.\n\r") % cyan() % tString % norm());

    if (getExp() < getMaxExp()) {
      tString = ((sstring)(format("%.2f") % getMaxExp())).comify();
      sendTo(format("Your most exp before your last death was: %s%s%s\n\r") % cyan() % tString % norm());
    }

    int total=0, count=0;
    for (spellNumT tSpell = MIN_SPELL; tSpell < MAX_SKILL; tSpell++){
      if(getDisciplineNumber(tSpell, FALSE)!=DISC_NONE &&
	 doesKnowSkill(tSpell)){
	total += getSkillValue(tSpell);
	++count;
      }
    }
    if(count > 0){
      sendTo(format("You have a total of %s%i%s skill points with an average of %s%i%s per skill.\n\r") % 
	     cyan() % total % norm() %
	     cyan() % (int)(total/count) % norm());
    }


    GameTime::realTimePassed((time(0) - desc->session.connect), 0, &playing_time);
    if (playing_time.day)
      playing_time.hours += playing_time.day * 24;

    sendTo(format("You have been playing for %s%d%s hour%s, %s%d%s minute%s and %s%d%s second%s in this session.\n\r") %
         purple() % int(playing_time.hours)   % norm() % (playing_time.hours   == 1 ? "" : "s") %
         purple() % int(playing_time.minutes) % norm() % (playing_time.minutes == 1 ? "" : "s") %
         purple() % int(playing_time.seconds) % norm() % (playing_time.seconds == 1 ? "" : "s"));
  }

  GameTime::realTimePassed((time(0) - player.time->logon) + player.time->played,
                 0, &playing_time);

  sendTo(format("For a lifetime total of %s%d%s day%s and %s%d%s hour%s.\n\r") %
         purple() % int(playing_time.day)   % norm() % (playing_time.day   == 1 ? "" : "s") %
         purple() % int(playing_time.hours) % norm() % (playing_time.hours == 1 ? "" : "s"));

  classIndT i;
  // since XP tables are all the same, the only time this should be
  // different is if they have gained in one class, but not another
  // we first check for this situation
  bool allClassesSame = true;
  for (i = MAGE_LEVEL_IND; i < MAX_CLASSES; i++)
    if (getLevel(i) && getLevel(i) != GetMaxLevel())
      allClassesSame = false;

  if (allClassesSame)
    sendTo(format("Your level: %s lev %2d          This ranks you as:\n\r") %           getProfName() % GetMaxLevel());
  else {
    sendTo("Your level: ");
    bool shownFirst = false;
    for (i = MAGE_LEVEL_IND; i < MAX_CLASSES; i++) {
      if (getLevel(i)) {
        sendTo(format("%s%s lev %2d") %             (shownFirst ? ", " : "") %
	       classInfo[i].name.cap() % getLevel(i));
        shownFirst = true;
      }
    }
    sendTo("          This ranks you as:\n\r");
  }

  Buf = parseTitle(desc);
  Buf += "\n\r";
  // Done this way just in case the player uses a % in their title.
  sendTo(COLOR_BASIC, format("%s") % Buf);

  for (i = MAGE_LEVEL_IND; i < MAX_CLASSES; i++) {
    if (getLevel(i) && getLevel(i) < MAX_MORT) {
      double need = getExpClassLevel(getLevel(i) + 1) - getExp();
      tString = ((sstring)(format("%.2f") % need)).comify();

      if (allClassesSame) {
        sendTo(format("You need %s%s%s experience points to be a %sLevel %d %s%s.\n\r") %             purple() % tString % norm() % purple() % (getLevel(i)+1) %
	       getProfName() % norm());
        break;
      } else {
        // leveled in one class, but not another, show each class as own line
        sendTo(format("You need %s%s%s experience points to be a %sLevel %d %s%s.\n\r") %             purple() % tString % norm() % purple() % (getLevel(i)+1) %
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
    sendTo(format("You are %s.\n\r") % tasks[task->task].name);
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

          if (!riding->getName().empty())
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
          if (!riding->getName().empty())
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
          if (!riding->getName().empty())
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
	  Buf = format("You are here, also riding on %s's %s%s.\n\r") %
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

  if (affectedBySpell(AFFECT_WET))
    sendTo(format("You feel %s.\n\r") % Weather::describeWet(this));

  sendTo(format("You are in %s%s%s attack mode.\n\r") %         cyan() % attack_modes[getCombatMode()] % norm());

  if (getWimpy())
    sendTo(format("You are in wimpy mode, and will flee at %d hit points.\n\r") %
	   getWimpy());

  describeLimbDamage(this);
  sendTo(COLOR_BASIC, describeAffects(this, SHOW_ME));
}
