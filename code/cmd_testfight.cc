#if 0
extern "C" {
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/param.h>

#ifdef SOLARIS
#include <sys/file.h>
#endif

int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
}
#endif

#include "stdsneezy.h"
#include "combat.h"
#include "spelltask.h"

static unsigned int num_fighting = 0;
static unsigned long left1 = 0;
static unsigned long tot1 = 0;
static unsigned int iter1 = 0;
static unsigned long left2 = 0;
static unsigned long tot2 = 0;
static unsigned int iter2 = 0;
static int mob1_num = 0;
static int mob2_num = 0;
static bool automated = false;
static int highNum = -1;
static int lowNum = -1;
static int changed_class = 0;

static void fastFight();

static void test_fight_start(bool same_time)
{
  num_fighting = 0;
  left1 = 0;
  tot1 = 0;
  iter1 = 0;
  left2 = 0;
  tot2 = 0;
  iter2 = 0;

  vlogf(LOG_MISC, fmt("Testing mob %d vs %d.   %s") %  mob1_num % mob2_num %
      (same_time ? "Same time fight" : "mob1 hitting"));

  if (changed_class)
    vlogf(LOG_MISC, fmt("Creating mob %d as class %d") %  mob1_num % changed_class);

  int vnum;
  for (vnum = 0; vnum < WORLD_SIZE; vnum++) {
    if (real_roomp(vnum))
      continue;

    TMonster *mob1;
    TMonster *mob2;

    // position in charcter_list affects whic one gets to do specials first
    // so don't let that be a factor by switching back and forth
    if (num_fighting%2 == 0) {
      mob1 = read_mobile(mob1_num, VIRTUAL);
      mob2 = read_mobile(mob2_num, VIRTUAL);
    } else {
      mob2 = read_mobile(mob2_num, VIRTUAL);
      mob1 = read_mobile(mob1_num, VIRTUAL);
    }

    if (!mob1 || !mob2) {
      vlogf(LOG_MISC, "Failed testfight mob load");
      delete mob1;
      delete mob2;
      return;
    }

    TRoom *rp = new TRoom(vnum);
    if (!rp) {
      vlogf(LOG_MISC, "Failed testfight room load");
      delete mob1;
      delete mob2;
      return;
    }
    // just to be safe
    rp->name = mud_str_dup("A test fighting room");
    rp->setDescr(mud_str_dup("A room for testing fights.\n\r"));
    rp->setRoomFlagBit(ROOM_INDOORS | ROOM_ALWAYS_LIT);
    rp->setSectorType(SECT_TEMPERATE_BUILDING);
    rp->putInDb(vnum);

    if (changed_class) {
      mob1->setClass(changed_class);
      mob1->fixLevels(mob1->GetMaxLevel());
      mob1->setExp(mob1->determineExp());
      
      // fix disciplines too
      delete mob1->discs;
      mob1->discs = NULL;
      mob1->assignDisciplinesClass();
    }

    *rp += *mob1;
    *rp += *mob2;

    // let the critters defend self
    mob1->quickieDefend();
    mob2->quickieDefend();

    affectedData aff;
    aff.type = AFFECT_TEST_FIGHT_MOB;
    aff.duration = PERMANENT_DURATION;
    aff.modifier = 1;
  
    aff.level = 1;
    aff.be = mob2;
    mob1->affectTo(&aff, -1);
  
    aff.level = 2;
    aff.be = mob1;
    mob2->affectTo(&aff, -1);

    if (!same_time) {
      do {
        mob1->takeFirstHit(*mob2);
      } while (!mob1->fight());
    } else {
      // set both fighting, no advantage to either
      mob1->setCharFighting(mob2);
      mob1->setVictFighting(mob2);
    }

    num_fighting++;
    if (num_fighting >= 100)
      break;
  }

  fastFight();
}

static void repTheStats()
{
  unsigned iTot = iter1 + iter2;
  // these will report the average %HP they had left
  float perc1 = (float) left1 * 100.0 * iTot / (float) tot1 / (float) iter1;
  float perc2 = (float) left2 * 100.0 * iTot / (float) tot2 / (float) iter2;

  vlogf(LOG_MISC, fmt("m1(%d): %lu/%lu (%.4f%%)(%d) : m2(%d): %lu/%lu (%.4f%%)(%d)") % 
        mob1_num % left1 % tot1 %
        perc1 % iter1 %
        mob2_num % left2 % tot2 %
        perc2 % iter2);

  FILE *fp;
  fp = fopen("testfight.log", "a+");
  if (fp) {
    fprintf(fp, "m1(%d): %lu/%lu (%.4f%%)(%d) : m2(%d): %lu/%lu (%.4f%%)(%d)\n",
        mob1_num, left1, tot1,
        perc1, iter1,
        mob2_num, left2, tot2,
        perc2, iter2);
    fclose(fp);
  }

  if (automated) {
    if (mob1_num >= 1701 && mob1_num <= 1750) {
      int old_mob2 = mob2_num;
      if (iter1 < iter2) {
        mob2_num--;
        lowNum = iter1;
      } else {
        mob2_num++;
        highNum = iter1;
      }

      // useful for class testing
      if ((changed_class &&
           ((highNum >= ((int) iTot/2) && lowNum <= ((int) iTot/2) && lowNum >= 0) || mob2_num > 1750 || mob2_num < 1701)) ||
      // useful for finding doubling level, and a good default
          (!changed_class &&
           ((mob2_num > (mob1_num + 5)) || (perc2 > 50.0) || mob2_num > 1750))) {
        // be a bit smart about this and start next test properly
        int offset = old_mob2 - mob1_num;
        if (mob1_num == 1750)
          mob1_num = 1701;
        else
          mob1_num++;

        highNum = -1;
        lowNum = -1;
        mob2_num = mob1_num + offset;

        if (mob2_num < 1701 || mob2_num > 1750) {
          automated = false;
          vlogf(LOG_MISC, "Testing stopped.");
          return;
        }
      }
      test_fight_start(true);
    }
  }
}

// we get here primarily from damageEpilog() via die()
// don't delete ch or v since we aren't set up for it...
void test_fight_death(TBeing *ch, TBeing *v, int mod)
{
  // we can't delete ch or v, but we CAN clean up the void from past fights
  // so do this first..
  TRoom *rp2 = real_roomp(ROOM_VOID);
  TThing *t, *t2;
  for (t = rp2->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    t->purgeMe(NULL);
  }

  // ch is dead and had the TEST_FIGHT_MOB affect
  // update statistics, and remove this room since it was a mock up

  TRoom *rp = ch->roomp;

  num_fighting--;
  
#if 0
  vlogf(LOG_MISC, fmt("Test fight in room %d: %s beat %s by %.2f%% (%d)") % 
     ch->in_room % v->getName() % ch->getName() % v->getPercHit() % num_fighting);
#else
  if (!(num_fighting%100) ||
      (!(num_fighting/100) && !(num_fighting%10)) ||
      (!(num_fighting/10))) {
      vlogf(LOG_MISC, fmt("There are %d fights remaining (%d to %d).") %  num_fighting % iter1 % iter2);
  }
#endif

  if (mod == 1) {
    // 1 died, ch = 1, v = 2
    left2 += v->getHit();
    tot1 += ch->hitLimit();
    tot2 += v->hitLimit();
    iter2++;
  } else {
    // 2 died, ch = 2, v = 1
    left1 += v->getHit();
    tot1 += v->hitLimit();
    tot2 += ch->hitLimit();
    iter1++;
  }

  if (num_fighting == 0) {
    repTheStats();
  }

  ch->affectFrom(AFFECT_TEST_FIGHT_MOB);
  v->affectFrom(AFFECT_TEST_FIGHT_MOB);

  // take ch out of room and move to void
  --(*ch);
  thing_to_room(ch, ROOM_VOID);

  while ((t = rp->getStuff())) {
    --(*t);
    thing_to_room(t, ROOM_VOID);
  }
  delete rp;
}

void TBeing::doTestFight(const char *arg)
{
  if (!hasWizPower(POWER_WIZARD)) {
    sendTo("Prototype command.  You need to be a developer to use this.\n\r");
    return;
  }

  char cmob1[256], cmob2[256];
  arg = one_argument(arg, cmob1, cElements(cmob1));
  arg = one_argument(arg, cmob2, cElements(cmob2));

  if (*cmob1 && is_abbrev(cmob1, "automated")) {
    automated = !automated;
    if (automated)
      sendTo("Test fights are now automated.\n\r");
    else
      sendTo("Test fights are no longer automated.\n\r");
    return;
  } else if (*cmob1 && is_abbrev(cmob1, "abort")) {
    TBeing *ch, *ch2;
    for (ch = character_list; ch; ch = ch2) {
      ch2 = ch->next;
      if (ch->affectedBySpell(AFFECT_TEST_FIGHT_MOB))
        delete ch;
    }
    repTheStats();
    return;
  } else if (*cmob1 && is_abbrev(cmob1, "class")) {
    changed_class = convertTo<int>(cmob2);
    sendTo(fmt("Changed class is now %d.\n\r") % changed_class);
    return;
  }

  if (!*cmob1 || !*cmob2) {
    sendTo("Syntax: testfight <mob vnum> <mob vnum>\n\r");
    return;
  }
  if (num_fighting) {
    sendTo("Test was in progress.  Ending old test.\n\r");
    repTheStats();
  }

  mob1_num = convertTo<int>(cmob1);
  mob2_num = convertTo<int>(cmob2);

  highNum = -1;
  lowNum = -1;

  // if they give extra args, have mob1 hit mob2, otherwise at same time
  test_fight_start(*arg ? false : true);
  return;
}

static void fastFight()
{
  // fights take 5-10 minutes to finish
  // some of this is actual CPU usage, but rest is just inter round lag
  // so lets sit in a tight loop and force things to resolve...
  // this will ABSOLUTELY lag the game

  // most of this logic is stripped out of socket.cc: gameLoop()
  if (gamePort == PROD_GAMEPORT)
    return;

  int pulse = 0;
  int rc;
#if 0
  fd_set input_set, output_set, exc_set;
  struct timeval last_time, now, timespent, timeout, null_time;
  struct timeval opt_time;
  Descriptor *point;
#endif

  while (num_fighting && gCombatList) {
    pulse++;

#if 0
    // Check what's happening out there
    FD_ZERO(&input_set);
    FD_ZERO(&output_set);
    FD_ZERO(&exc_set);
    FD_SET(m_sock, &input_set);
    for (point = descriptor_list; point; point = point->next) {
      FD_SET(point->socket->m_sock, &input_set);
      FD_SET(point->socket->m_sock, &exc_set);
      FD_SET(point->socket->m_sock, &output_set);
    }

    // check out the time
    gettimeofday(&now, NULL);
    timespent=timediff(&now, &last_time);
    timeout=timediff(&opt_time, &timespent);
    last_time.tv_sec = now.tv_sec + timeout.tv_sec;
    last_time.tv_usec = now.tv_usec + timeout.tv_usec;
    if (last_time.tv_usec >= 1000000) {
      last_time.tv_usec -= 1000000;
      last_time.tv_sec++;
    }
#ifndef SOLARIS
//    sigsetmask(mask);
#endif
#ifdef LINUX
    // linux uses a nonstandard style of "timedout" (the last parm of select)
    // it gets hosed each select() so must be reinited here
    null_time.tv_sec = 0;
    null_time.tv_usec = 0;
#endif
    if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
      perror("Error in Select (poll)");
      return;
    }
    if (select(0, 0, 0, 0, &timeout) < 0) {
      perror("Error in select (sleep)");
    }

    // close any connections with an exceptional condition pending
    for (point = descriptor_list; point; point = next_to_process) {
      next_to_process = point->next;
      if (FD_ISSET(point->socket->m_sock, &exc_set)) {
        FD_CLR(point->socket->m_sock, &input_set);
        FD_CLR(point->socket->m_sock, &output_set);
        delete point;
      }
    }
#endif

    int combat = (pulse % PULSE_COMBAT);
    int tick_updates = (pulse % PULSE_MUDHOUR);
    int mobstuff = (pulse % PULSE_MOBACT);
    int points = (pulse % PULSE_UPDATE);

    if (!combat)
      perform_violence(pulse);

    if (!combat || !mobstuff || !tick_updates || !points) {
      TBeing *tmp_ch, *temp;
      for (tmp_ch = character_list; tmp_ch; tmp_ch = temp) {
        temp = tmp_ch->next;  // just for safety

        TMonster * tm = dynamic_cast<TMonster *>(tmp_ch);
        if (tm && !tm->isTestmob()) {
          if (gamePort == PROD_GAMEPORT) {
            temp = tm->next;
            continue;
          } else {
            // gets rid of all mobs not involved in testfight
            temp = tm->next;
            int nm = tm->mobVnum();
            if (nm != mob1_num && nm != mob2_num) {
              delete tm;
              tm = NULL;
            }
            continue;
          }
        } else if (tm) {
          if (!tm->fight()) {
            rc = tm->die(DAMAGE_NORMAL);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              temp = tm->next;
              delete tm;
              tm = NULL;
              continue;
            }
          }
        }

        if (!mobstuff) {
          if (!tmp_ch->isPc() && tm) {
            rc = tm->mobileActivity(pulse);
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              temp = tm->next;
              delete tm;
              tm = NULL;
              continue;
            }
          }
        }

        if (!combat) {

          if (tmp_ch->spelltask) {
            rc = (tmp_ch->cast_spell(tmp_ch, CMD_TASK_CONTINUE, pulse));
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              temp = tmp_ch->next;
              delete tmp_ch;
              tmp_ch = NULL;
              continue;
            }
          }

          rc = tmp_ch->updateAffects();
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            // died in update (disease)
            temp = tmp_ch->next;
            delete tmp_ch;
            tmp_ch = NULL;
            continue;
          }
          // soak up attack if not in combat
          if ((tmp_ch->cantHit > 0) && !tmp_ch->fight())
            tmp_ch->cantHit--;
        }

        if (!points) {
          rc = tmp_ch->updateHalfTickStuff();
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            if (!tmp_ch)
              continue;

            temp = tmp_ch->next;
            delete tmp_ch;
            tmp_ch = NULL;
            continue;
          }
        }
        if (!tick_updates) {
          rc = tmp_ch->updateTickStuff();
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            temp = tmp_ch->next;
            if (!dynamic_cast<TBeing *>(tmp_ch)) {
              // something may be corrupting tmp_ch below - bat 8-18-98
              vlogf(LOG_BUG, "forced crash.  How did we get here?");
            }
            delete tmp_ch;
            tmp_ch = NULL;
            continue;
          }
        }
        temp = tmp_ch->next;
      }
    }
    if (pulse >= 2400)
      pulse = 0;

    descriptor_list->outputProcessing();

    tics++;  // to force checkpointing to work OK
  }
}
