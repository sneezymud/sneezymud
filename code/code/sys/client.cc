//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "client.cc" - All functions and routines related to client/server 
//
//      The client/server protocol coded by Russ Russell
//
//////////////////////////////////////////////////////////////////////////

#include <cstdarg>

#if defined(LINUX) || defined(SOLARIS)
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "extern.h"
#include "room.h"
#include "being.h"
#include "client.h"
#include "low.h"
#include "person.h"
#include "monster.h"
#include "configuration.h"
#include "account.h"
#include "charfile.h"
#include "person.h"
#include "statistics.h"
#include "combat.h"
#include "database.h"
#include "rent.h"
#include "shop.h"
#include "weather.h"

const int  CURRENT_VERSION = 19990615;
const int  BAD_VERSION = 19990614;

// clientf(...) will take a string, and tack on the CLIENT_CODE_CHAR at 
// the beginning and end, and also put a newline at the end. - Russ     

void Descriptor::clientf(const sstring &msg)
{
  // This is done so that the client message isn't somehow combined 
  // with other text and missed by the client interpreter Russ - 061299
  outputProcessing();

  // This is the last sanity check.
  if (!msg.empty() && (m_bIsClient || IS_SET(prompt_d.type, PROMPT_CLIENT_PROMPT)))
    output.putInQ(new UncategorizedComm(format("\200%s\n") % msg));
}

void TRoom::clientf(const sstring &msg)
{
  TThing *t=NULL;

  if (!msg.empty()) {
    for(StuffIter it=stuff.begin();it!=stuff.end() && (t=*it);++it) {
      if (t->isPc() && t->desc && (t->desc->m_bIsClient || IS_SET(t->desc->prompt_d.type, PROMPT_CLIENT_PROMPT)))
	t->sendTo(format("\200%s\n") % msg);
    }
  }
}


void Descriptor::send_client_motd()
{
}

void Descriptor::send_client_inventory()
{
  TBeing *ch;
  TThing *t=NULL;

  if (!(ch = character))
    return;

  for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end() && (t=*it);++it) {
    TObj * tobj = dynamic_cast<TObj *>(t);
    if (!tobj)
      continue;
    clientf(format("%d|%d|%s|%d") % CLIENT_INVENTORY % ADD % tobj->getName() % tobj->itemType());
  }
}

void Descriptor::send_client_room_people()
{
  TThing *folx=NULL;
  TBeing *ch;

  if (!(ch = character))
    return;

  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (folx=*it);++it)
    clientf(format("%d|%d|%s") % CLIENT_ROOMFOLX % ADD % folx->getName());
}

void Descriptor::send_client_room_objects()
{
  TBeing *ch;
  TThing *t=NULL;

  if (!(ch = character))
    return;

  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end() && (t=*it);++it)
    clientf(format("%d|%d|%s") % CLIENT_ROOMOBJS % ADD % t->getName());
}



void Descriptor::send_client_prompt(int, int update)
{
  TBeing *ch, *tank;
  char c_name[128], cond[128];
  char t_name[128], t_cond[128];
  int ratio;

  *c_name = *cond = *t_name = *t_cond = '\0';

  if (connected || !(ch = character))
    return;

  if (ch->fight() && ch->awake() && ch->sameRoom(*ch->fight())) {
    ratio=min(10, max(0, ((ch->fight()->getHit() * 9) / ch->fight()->hitLimit())));
    strcpy(c_name, ch->persfname(ch->fight()).c_str());
    strcpy(cond, prompt_mesg[ratio]);
    tank = ch->fight()->fight();
    if (tank) {
      if (ch->sameRoom(*tank)) {
        strcpy(t_name, ch->persfname(tank).c_str());
        ratio = min(10, max(0, ((tank->getHit() * 9) / tank->hitLimit())));
        strcpy(t_cond, prompt_mesg[ratio]);
      }
    }
    clientf(format("%d|%s|%s|%s|%s") % CLIENT_FIGHT % c_name % 
	    cond % t_name % t_cond);
  }
  if ((update & CHANGED_MANA) || (update & CHANGED_PIETY) || (update & CHANGED_LIFEFORCE)) {
    char manaBuf[80], maxManaBuf[80];
    int iClientCode = CLIENT_MANA;

    if (ch->hasClass(CLASS_CLERIC) || ch->hasClass(CLASS_DEIKHAN)) {
      sprintf(manaBuf, "%.1f", ch->getPiety());
      strcpy(maxManaBuf, "100");

      if (!m_bIsClient && IS_SET(prompt_d.type, PROMPT_CLIENT_PROMPT))
        iClientCode = CLIENT_PIETY;
    } else if (ch->hasClass(CLASS_SHAMAN)) {
      sprintf(manaBuf, "%d", ch->getLifeforce());
      strcpy(maxManaBuf, "32000");

      if (!m_bIsClient && IS_SET(prompt_d.type, PROMPT_CLIENT_PROMPT))
        iClientCode = CLIENT_LIFEFORCE;
    } else {
      sprintf(manaBuf, "%d", ch->getMana());
      sprintf(maxManaBuf, "%d", ch->manaLimit());
    }

    clientf(format("%d|%s|%s") % iClientCode % manaBuf % maxManaBuf);
  }
  if (update & CHANGED_HP) 
    clientf(format("%d|%d|%d") % CLIENT_HITPOINT % ch->getHit() % ch->hitLimit());

  if (update & CHANGED_MOVE)
    clientf(format("%d|%d|%d") % CLIENT_MOVE % ch->getMove() % ch->moveLimit());

  if (update & CHANGED_ROOM) {
    if (m_bIsClient)
      clientf(format("%d|%d") % CLIENT_ROOM % (ch->isImmortal() ? ch->in_room : -1000));

    send_client_exits();
  }

  if (update & CHANGED_EXP) {
    clientf(format("%d|%s") % CLIENT_EXP % ch->displayExp());

    if (!m_bIsClient) {
      classIndT iClass;

      for (iClass = MAGE_LEVEL_IND; iClass < MAX_CLASSES; iClass++) {
	if (ch->getLevel(iClass)) {
          if (ch->getLevel(iClass) < MAX_MORT) {
	    double iNeed = getExpClassLevel(iClass, ch->getLevel(iClass) + 1) - ch->getExp();
            char   StTemp[2048];

	    memset(&StTemp, 0, sizeof(StTemp));

	    if (ch->getExp() < 100)
	      sprintf(StTemp, "%.3f", iNeed);
	    else
	      sprintf(StTemp, "%.0f", iNeed);

            clientf(format("%d|%d|%d|%s") % CLIENT_TONEXTLEVEL % iClass % (ch->getLevel(iClass) + 1) % StTemp);
          }

          break;
        }
      }
    }
  }

  if (update & CHANGED_GOLD)
    clientf(format("%d|%d") % CLIENT_GOLD % ch->getMoney());

  if (update & CHANGED_COND)
    clientf(format("%d|%d|%d") % CLIENT_COND % ch->getCond(FULL) % ch->getCond(THIRST));

  if (update & CHANGED_POS)
    clientf(format("%d|%d") % CLIENT_POS % ch->getPosition());

  if (update & CHANGED_MUD) {
    clientf(format("%d|%s") % CLIENT_MUDTIME %
	    GameTime::hmtAsString(GameTime::hourminTime()));

  }
  //prompt_mode = -1;
}

void Descriptor::send_client_exits()
{
  TBeing *ch;
  int bits = 0,
      cbits = 0;
  roomDirData *exitdata;
  dirTypeT door;
  RoomExitComm *comm=new RoomExitComm();

  if (!(ch = character))
    return;

  for (door = MIN_DIR; door < MAX_DIR; door++) {
    if ((exitdata = ch->exitDir(door))){
      if ((exitdata->to_room != Room::NOWHERE) && (!(exitdata->condition & EX_CLOSED) || ch->isImmortal()))
        SET_BIT(bits, (1 << door));
      else if(!(exitdata->condition & EX_SECRET))
        SET_BIT(cbits, (1 << door));

      
      comm->exits[door].exit=true;
      if(exitdata->door_type != DOOR_NONE)
	comm->exits[door].door=true;
      else
	comm->exits[door].door=false;
      
      comm->exits[door].open=!IS_SET(exitdata->condition, EX_CLOSED);
    } else {
      comm->exits[door].exit=false;
    }
  }

  ch->sendTo(comm);

  if (m_bIsClient)
    clientf(format("%d|%d|%d") % CLIENT_EXITS % bits % ch->isImmortal());
  else if (IS_SET(prompt_d.type, PROMPT_CLIENT_PROMPT))
    clientf(format("%d|%d|%d|%d") % CLIENT_EXITS % bits % cbits % ch->isImmortal());
}

// returns DELETE_THIS to delete the descriptor
// returns DELETE_VICT if the desc->character should be toasted
int Descriptor::read_client(char *str2)
{
  Descriptor *k, *k2;
  TBeing *ch, *person;
  charFile st;
  int type, notify;
  //char buf[20000], tmp_name[40], wizbuf[256];
  char buf[20000], tmp_name[40];
  TRoom *rp;
  int vers = 0;

  strcpy(buf, nextToken('|', 255, str2).c_str());
  if (sscanf(buf, "%d", &type) != 1) {
    vlogf(LOG_CLIENT, format("Incorrect type (%s) in read_client") %  buf);
    return FALSE;
  }
  switch (type) {
    case CLIENT_INIT:
      m_bIsClient = TRUE;

      if (!toggleInfo[TOG_CLIENTS]->toggle) {
        clientf(format("%d|Clients not allowed at this time. Try later!|%d") % 
                CLIENT_ERROR % ERR_NOT_ALLOWED);
        outputProcessing();
        return FALSE;
      }
      if (WizLock) {
        // this may need better handling to let wizs in, but, oh well
        clientf(format("%d|The mud is presently Wizlocked.|%d") % 
                CLIENT_ERROR % ERR_NOT_ALLOWED);
        if (!lockmess.empty())
          clientf(lockmess);
        outputProcessing();
        return FALSE;
      }
      strcpy(buf, nextToken('|', 255, str2).c_str());
      vers = convertTo<int>(buf);
      if (vers <= BAD_VERSION) {
        clientf(format("%d|Your client is an old version. The latest version is %d. Please upgrade! You can upgrade from http://sneezy.saw.net/client/client.html.|%d") % CLIENT_ERROR % CURRENT_VERSION % ERR_BAD_VERSION);
        outputProcessing();
        return FALSE;
      } else if (vers < CURRENT_VERSION) {
        clientf(format("%d|You client is an old version. You can continue playing with the current version, but upgrade is recommended. The latest version is %d and can be received from http://sneezy.saw.net/client.|%d") % CLIENT_ERROR % CURRENT_VERSION % 7); //ERR_CUR_VERSION);
        outputProcessing();
      }

      output.clear();
      if (account) {
        if (IS_SET(account->flags, TAccount::IMMORTAL)) 
          vlogf(LOG_PIO, "Client Connection from *****Masked*****");
        else 
          vlogf(LOG_PIO, format("Client Connection from %s") %  host);
      }
      break;
    case CLIENT_ROOM: {
      dirTypeT door;
      roomDirData *exitdata;
      extraDescription *exptr;
      char tmpBuf[MAX_STRING_LENGTH] = "\0";
      char tmpBuf2[MAX_STRING_LENGTH] = "\0";

      if (!character || !(rp = character->roomp))
        break;

      for (door = MIN_DIR; door < MAX_DIR; door++) {
        if ((exitdata = character->exitDir(door))) 
          sprintf(tmpBuf + strlen(tmpBuf), "|1|%d|%d|%s|%s", exitdata->to_room, 
                  exitdata->door_type, exitdata->keyword, exitdata->description);
        else 
          sprintf(tmpBuf + strlen(tmpBuf), "|0");
      }

      if (rp->ex_description) {
        for (exptr = rp->ex_description; exptr; exptr = exptr->next) {
          sprintf(tmpBuf2 + strlen(tmpBuf2), "|%s|%s", exptr->keyword, exptr->description); 
        }      
      }
      char tmpbuf[25000];
      sprintf(tmpbuf, "%d|%s|%s|%d|%d|%d%s%s", CLIENT_CURRENTROOM, rp->name, rp->getDescr(),
              rp->getSectorType(),
              rp->getMoblim(), rp->getRoomHeight(), tmpBuf, tmpBuf2);

      sstring sb = tmpbuf;
      processStringForClient(sb);

      clientf(sb);
      break;
    }
    case CLIENT_ROOMNAME:
      if (!character)
        break;

      if (!character->isImmortal()) {
        character->sendTo("You cannot do this as a mortal. If this is a client bug report to Brutius. Otherwise stop trying to hack the client because the server double checks everything. Thanks. Brutius.\n\r");
        break;
      }
      strcpy(buf, nextToken('|', 255, str2).c_str());
     
      if (!(rp = character->roomp))
        break;

      delete [] rp->name;
      rp->name = mud_str_dup(buf);
      break;
    case CLIENT_ROOMSECTOR:
      if (!character)
        break;

      if (!character->isImmortal()) {
        character->sendTo("You cannot do this as a mortal. If this is a client bug report to Brutius. Otherwise stop trying to hack the client because the server double checks everything. Thanks. Brutius.\n\r");
        break;
      }
      strcpy(buf, nextToken('|', 255, str2).c_str());

      if (!(rp = character->roomp))
        break;

      rp->setSectorType((sectorTypeT) convertTo<int>(buf));
      break;
    case CLIENT_ROOMMAXCAP:
      if (!character)
        break;

      if (!character->isImmortal()) {
        character->sendTo("You cannot do this as a mortal. If this is a client bug report to Brutius. Otherwise stop trying to hack the client because the server double checks everything. Thanks. Brutius.\n\r");
        break;
      }
      strcpy(buf, nextToken('|', 255, str2).c_str());

      if (!(rp = character->roomp))
        break;

      rp->setMoblim(convertTo<int>(buf));
      break;
    case CLIENT_ROOMHEIGHT:
      if (!character)
        break;

      if (!character->isImmortal()) {
        character->sendTo("You cannot do this as a mortal. If this is a client bug report to Brutius. Otherwise stop trying to hack the client because the server double checks everything. Thanks. Brutius.\n\r");
        break;
      }
      strcpy(buf, nextToken('|', 255, str2).c_str());

      if (!(rp = character->roomp))
        break;

      rp->setRoomHeight(convertTo<int>(buf));
      break;
    case CLIENT_ROOMDESC: {
      char descrBuf[MAX_STRING_LENGTH];
      if (!character)
        break;

      if (!character->isImmortal()) {
        character->sendTo("You cannot do this as a mortal. If this is a client bug report to Brutius. Otherwise stop trying to hack the client because the server double checks everything. Thanks. Brutius.\n\r");
        break;
      }
      strcpy(descrBuf, nextToken('|', MAX_STRING_LENGTH, str2).c_str());
      strcat(descrBuf, "\n\r");
      if (!(rp = character->roomp))
        break;

      delete [] rp->getDescr();
      rp->setDescr(mud_str_dup(descrBuf));

      break;
    }
    case CLIENT_IDEA:
    case CLIENT_TYPO:
    case CLIENT_BUG: {

      sstring buffer = str2;
      buffer.inlineReplaceString("^", "\r\n");
      send_feedback(name, buffer.c_str());

      character->sendTo("Thanks for the report. It will be looked at soon!\n\r");
      character->sendTo("If necessary, a mudmail will be sent to you to inform you of any changes or possible explanations.\n\r");
      break;
    }

    case CLIENT_MAIL: {
      char buffer[10000];
      char name[256];
      const char *tc;
      int j;
      int rent_id = 0;

      strcpy(name, nextToken('|', 255, str2).c_str());

      if (ignored.isMailIgnored(this, name))
      {
        vlogf(LOG_OBJ, format("Mail: mail sent by %s was ignored by %s.") % character->getName() % name);
        break;
      }

      for (j = 0, tc = str2; *tc; tc++, j++) {
        if (j > 9995)
          break;

        if (*tc == '^') {
          buffer[j++] = '\r';
          buffer[j] = '\n';
        } else
          buffer[j] = *tc;
      }
      buffer[j] = '\0';

      if (obj && obj->canBeMailed(sstring(name)))
      {
        ItemSaveDB is("mail", GH_MAIL_SHOP);
        rent_id = is.raw_write_item(obj, -1 /*NORMAL_SLOT*/, 0);
        vlogf(LOG_OBJ, format("Mail: %s mailing %s (vnum:%i) to %s rented as rent_id:%i") %
          character->getName() % obj->getName() % obj->objVnum() % name % rent_id);
        delete obj;
      }
      if (amount > 0)
      {
        vlogf(LOG_OBJ, format("Mail: %s mailing %i talens to %s") %
          character->getName() % amount % name);
        character->addToMoney(min(0, -amount), GOLD_XFER);
      }

      store_mail(name, character->getName(), buffer, amount, rent_id);

      // clear amount, object, name
      obj = NULL;
      *(name) = '\0';
      amount = 0;

      break;
    }
    case CLIENT_WHO:
      notify = convertTo<int>(nextToken('|', 255, str2));
      prompt_mode = -1;
      for (person = character_list; person; person = person->next) {
        if (person->isPc() && person->polyed == POLY_TYPE_NONE) {
          if (dynamic_cast<TPerson *>(person) && character && (character->GetMaxLevel() >= person->getInvisLevel())) {
            if (person->isLinkdead() && character->isImmortal()) 
              clientf(format("%d|[%s]|%d|%d|%d") % CLIENT_WHO %
		      person->getName() % ADD %person->GetMaxLevel() % notify);
            else {
              if (person->isPlayerAction(PLR_ANONYMOUS) && !character->isImmortal())
                clientf(format("%d|%s|%d|0|%d") % CLIENT_WHO % person->getName() % ADD % notify);
              else
                clientf(format("%d|%s|%d|%d|%d") % CLIENT_WHO % person->getName() % ADD % person->GetMaxLevel() % notify);
            }
          }
        }
      }
      break;
    case CLIENT_CANCELEDIT:
      str = NULL;
      max_str = 0;
      connected = CON_PLYNG;
      if (character->isPlayerAction(PLR_MAILING)) 
        character->remPlayerAction(PLR_MAILING);

      if (character->isPlayerAction(PLR_BUGGING)) 
        character->remPlayerAction(PLR_BUGGING);

      if (connected == CON_WRITING) 
        connected = CON_PLYNG;

      break;
    case CLIENT_RENT: {
      objCost cost;
      int save_room;

      // if disguised or transformed, we mimic the TMonster::doRent return
      if (character && dynamic_cast<TPerson *>(character) == NULL)
      {
        character->sendTo("You're a mob.  You can't rent!\n\r");
        return 0;
      }

      character->recepOffer(NULL, &cost);
      dynamic_cast<TPerson *>(character)->saveRent(&cost, TRUE, 2);
      save_room = character->in_room;        // backup what room the PC was in
      character->saveChar(save_room);
      character->in_room = save_room;
      character->preKillCheck(TRUE);
      sprintf(buf, "$n just rented with the %s client.", MUD_NAME_VERS);
      act(buf, FALSE, character, NULL, NULL, TO_ROOM);
      return DELETE_VICT;
      break;
    }
    case CLIENT_DISCONNECT:
      character->desc = NULL;
      return DELETE_THIS;
      break;
    case CLIENT_CONNECTED:
      if(character->hasQuestBit(TOG_PERMA_DEATH_CHAR)){
	character->loadCareerStats();
	if(character->desc->career.deaths){
	  
	  writeToQ("That character is a perma death character and has died.\n\r");
	  writeToQ("Name -> ");
	  
	  // copied from above
	  character->desc = NULL;
	  character->next = character_list;
	  character_list = character;
	  
	  character->setRoom(Room::NOWHERE);
	  
	  delete character;
	  character = new TPerson(this);
	  return FALSE;
	}
      }



      for (k = descriptor_list; k; k = k2) {
        k2 = k->next;
        if (this == k)
          continue;

        if ((k->character != character) && k->character) {
          if (k->original) {
            if (k->original->getName() && 
                !strcasecmp(k->original->getName(), character->getName())) {
              delete k;
              k = NULL;
            }
          } else {
            if (k->character->getName() && 
              !strcasecmp(k->character->getName(), character->getName())) {

              if (k->character) {
                // disassociate the char from old descriptor before
                // we delete the old descriptor
                k->character->desc = NULL;
                k->character = NULL;
              }
              delete k;
              k = NULL;
            }
          }
        }
      }
      max_str = 0;
      for (ch = character_list; ch; ch = ch->next) {
        if ((!strcasecmp(character->getName(), ch->getName()) &&
            !ch->desc && !dynamic_cast<TMonster *>(ch)) ||
            (dynamic_cast<TMonster *>(ch) && ch->orig &&
             !strcasecmp(character->getName(),
                      ch->orig->getName()))) {
 
          if ((character->inRoom() >= 0) ||
              (character->inRoom() == Room::AUTO_RENT)) {
            // loadFromSt will have inRoom() == last rent
            // roomp not set yet, so just clear this value
            character->setRoom(Room::VOID);
          }
          // we need to remove the character->desc for some deletion handling
          // to work (true, but why?).  This unfortunately causes it to go
          // into character_list removal logic inside ~TBeing.  Since we're
          // not actually in the char_list yet, it crashes.  Just insert myself
          // temporarily to bypass this.
          character->desc = NULL;
          character->next = character_list;
          character_list = character;

          rp = real_roomp(Room::VOID);
          *rp += *character;
          delete character;
          ch->desc = this;
          character = ch;
          ch->setTimer(0);
          ch->setInvisLevel(0);

          if (ch->orig) {
            ch->desc->original = ch->orig;
            ch->orig = 0;
          }
          connected = CON_PLYNG;
          flush();
          writeToQ("Reconnecting character...\n\r");
          send_client_prompt(TRUE, 16383);
          clientf(format("%d|%d") % CLIENT_ENABLEWINDOW % TRUE);

          // setombatMode sends necessary client info about attack mode
          ch->setCombatMode(ch->getCombatMode());  

          load_char(ch->getName(), &st);
          ch->initDescStuff(&st);

          ch->cls();

          if (should_be_logged(character)) {
            objCost cost;

            if (IS_SET(account->flags, TAccount::IMMORTAL)) 
              vlogf(LOG_PIO, format("%s[*masked*] has reconnected (client)  (account: *masked*).") %  ch->getName());
            else 
              vlogf(LOG_PIO, format("%s[%s] has reconnected (client)  (account: %s).") %  ch->getName() % host % account->name);

            ch->recepOffer(NULL, &cost);
            dynamic_cast<TPerson *>(ch)->saveRent(&cost, FALSE, 1);
          }
          act("$n has reconnected.", TRUE, ch, 0, 0, TO_ROOM);
          ch->loadCareerStats();
          ch->loadDrugStats();
	  ch->loadGuildStats();
	  ch->loadTitle();
          if (ch->getHit() < 0) 
            dynamic_cast<TPerson *>(ch)->autoDeath();

          int rc = checkForMultiplay();
	  if(Config::ForceMultiplayCompliance()){
	    if (rc) {
	      // disconnect, but don't cause character to be deleted
	      // do this by disassociating character from descriptor
	      character = NULL;
	      
	      return DELETE_THIS;
	    }
	  }

          ch->fixClientPlayerLists(FALSE);
          return FALSE;
        }
      }
      break;
    case CLIENT_PROMPT:
      send_client_prompt(FALSE, 16383);
      break;
    case CLIENT_EXITS: 
      send_client_exits();
      break;
    case CLIENT_INVENTORY:
      send_client_inventory();
      break;
    case CLIENT_ROOMFOLX:
      send_client_room_people();
      break;
    case CLIENT_ROOMOBJS:
      send_client_room_objects();
      break;
    case CLIENT_NEWCHAR: 
      clientCreateChar(str2);
      break;
    case CLIENT_NORMAL:
      if (character) {
        if (character->isCombatMode(ATTACK_BERSERK)) {
          character->sendTo("You are berserking.\n\r");
          break;
        }
        character->sendTo(format("Setting attack mode to %snormal%s\n\r") % character->redBold() % character->norm());
        character->setCombatMode(ATTACK_NORMAL);
      }
      break;
    case CLIENT_OFFENSIVE:
      if (character) {
        if (character->isCombatMode(ATTACK_BERSERK)) {
          character->sendTo("You are berserking.\n\r");
          break;
        }
        character->sendTo(format("Setting attack mode to %soffensive%s\n\r") % character->redBold() % character->norm());
        character->setCombatMode(ATTACK_OFFENSE);
      }
      break;
    case CLIENT_DEFENSIVE:
      if (character) {
        if (character->isCombatMode(ATTACK_BERSERK)) {
          character->sendTo("You are berserking.\n\r");
          break;
        }
        character->sendTo(format("Setting attack mode to %sdefensive%s\n\r") % character->redBold() % character->norm());
        character->setCombatMode(ATTACK_DEFENSE);
      }
      break;
    case CLIENT_CHECKCHARNAME:
      strcpy(buf, nextToken('|', 255, str2).c_str());
      if (_parse_name(buf, tmp_name) || checkForCharacter(tmp_name)) {
        clientf(format("%d|0") % CLIENT_CHECKCHARNAME);
        break;
      }
      clientf(format("%d|1") % CLIENT_CHECKCHARNAME);
      break;
    case CLIENT_CHECKACCOUNTNAME: {
      static char *crypted;
      TAccount *account;
      TDatabase db(DB_SNEEZY);
      char aname[256];
      char apassword[256];
      strcpy(aname, nextToken('|', 255, str2).c_str());
      int iNew = convertTo<int>(nextToken('|', 255, str2));
      if (iNew) {
        if (bogusAccountName(buf)) {
          clientf(format("%d|0|%d") % CLIENT_CHECKACCOUNTNAME % ERR_BADACCOUNT_NAME);
          break;
        }

	db.query("select 1 from account where name=lower('%s')", buf);

	if(db.fetchRow()){
          writeToQ("Account already exists, enter another name.\n\r");
          return TRUE;
        }
      } else { 
        account = new TAccount();

        strcpy(apassword, nextToken('|', 255, str2).c_str());
        if (bogusAccountName(aname)) {
          delete account;
          account = NULL;
          clientf(format("%d|0|%d") % CLIENT_CHECKACCOUNTNAME % ERR_BADACCOUNT_NAME);
          break;
        }
	if(account->read(aname)){
          account->term = TERM_NONE;
          plr_act = PLR_COLOR;
          account->login = time(0);
          account->status = FALSE;
          account->desc = this;
          strcpy(pwd, account->passwd.c_str());
        } else
          *pwd = '\0';

        if (!*pwd) {
          delete account;
          account = NULL;
          clientf(format("%d|0|%d") % CLIENT_CHECKACCOUNTNAME % ERR_BADACCOUNT_PASSWORD);
        }
        crypted = (char *) crypt(apassword, pwd);
        if (strncmp(crypted, pwd, 10)) {
          delete account;
          account = NULL;
          clientf(format("%d|0|%d") % CLIENT_CHECKACCOUNTNAME % ERR_BADACCOUNT_PASSWORD);
        }
        if (IS_SET(account->flags, TAccount::BANISHED)) {
          writeToQ("Your account has been flagged banished.\n\r");
          sprintf(buf, "If you do not know the reason for this, contact %s\n\r",
                MUDADMIN_EMAIL);
          writeToQ(buf);
          outputProcessing();
          return DELETE_THIS;
        }
        if (IS_SET(account->flags, TAccount::EMAIL)) {
          writeToQ("The email account you entered for your account is thought to be bogus.\n\r");
          sprintf(buf, "You entered an email address of: %s\n\r", account->email.c_str());
          writeToQ(buf);
          sprintf(buf,"To regain access to your account, please send an email\n\rto: %s\n\r",
              MUDADMIN_EMAIL);
          writeToQ(buf);
          writeToQ("Indicate the name of your account, and the reason for the wrong email address.\n\r");
          outputProcessing();
          return DELETE_THIS;
        }
        // let's yank the password out of their history list
        strcpy(history[0], "");

        if (WizLock && !IS_SET(account->flags, TAccount::IMMORTAL)) {
          writeToQ("The game is currently wiz-locked.\n\r^G^G^G^G^G");
          if (!lockmess.empty()) {
            page_string(lockmess, SHOWNOW_YES);
          } else {
            FILE *signFile;
  
            if ((signFile = fopen(File::SIGN_MESS, "r"))) {
              fclose(signFile);
              sstring iosstring;
              file_to_sstring(File::SIGN_MESS, iosstring);
              page_string(iosstring, SHOWNOW_YES);
            }
          }
          // we ought to allow for them to enter the password here, but oh well

          outputProcessing();
          return DELETE_THIS;
        }

        account->status = TRUE;
        if (!IS_SET(account->flags, TAccount::BOSS)) {
        }
        clientf(format("%d|1") % CLIENT_CHECKACCOUNTNAME);
      }
      break;
    }
    case CLIENT_NEWACCOUNT: {
      char aname[256];
      char apassword[256];
      char email[256];
      char timezone[256];
      char listserver[256];
      static char *crypted;

      strcpy(aname, nextToken('|', 255, str2).c_str());
      strcpy(apassword, nextToken('|', 255, str2).c_str());
      strcpy(email, nextToken('|', 255, str2).c_str());
      strcpy(timezone, nextToken('|', 255, str2).c_str());
      strcpy(listserver, nextToken('|', 255, str2).c_str());

      account = new TAccount;
      // Does account exist or is it a bogus name? This function will return TRUE is so
      if (checkForAccount(aname, TRUE)) {
        clientf(format("%d|Account name already exists! Please try another.") % CLIENT_ERROR);
        delete account;
        account = NULL;
	return FALSE;
      } 
      if (strlen(aname) >= 10) {
        clientf(format("%d|Account name must be less than 10 characters! Try another name please.") % CLIENT_ERROR);
        delete account;
        account = NULL;
        return FALSE;
      }
      account->name=aname;

      if (strlen(apassword) < 5) {
        clientf(format("%d|Password must be longer than 5 characters.") % CLIENT_ERROR);
        delete account;
        account = NULL;
        return FALSE;
      } else if (strlen(apassword) > 10) {
        clientf(format("%d|Password must be shorter than 10 characters.") % CLIENT_ERROR);
        delete account;
        account = NULL;
        return FALSE;
      }
      if(!sstring(apassword).hasDigit()){
        clientf(format("%d|Password must contain at least 1 numerical digit.") % CLIENT_ERROR);
        delete account;
        account = NULL;
        return FALSE;
      }
      crypted =(char *) crypt(apassword, account->name.c_str());
      account->passwd=sstring(crypted).substr(0,10);

      if (illegalEmail(email, this, SILENT_YES)) {
        clientf(format("%d|The email address you entered failed validity tests, please try another one.") % CLIENT_ERROR);
        delete account;
        account = NULL;
        return FALSE;
      }
      account->email=email;

      if (!*timezone || (convertTo<int>(timezone) > 23) || (convertTo<int>(timezone) < -23)) {
        clientf(format("%d|Invalid timezone please enter a number between 23 and -23!") % CLIENT_ERROR);
        delete account;
        account = NULL;
        return FALSE;
      }
      account->time_adjust = convertTo<int>(timezone);

      switch(*listserver) {
        case '1':
          break;                                                                                                  
        case '2':                                                                                                 
          break;                                                                                                  
      }                                                                                                           
      // Account term
      account->term = TERM_NONE;

      // Save all information
      account->write(aname);    
      AccountStats::account_number++;
    
      vlogf(LOG_MISC, format("New Client Account: '%s' with email '%s'") %  account->name % account->email);
      clientf(format("%d|1") % CLIENT_CHECKACCOUNTNAME);
      break;
    } 
    default:
      vlogf(LOG_CLIENT, format("Bad type in read_client (%d)") %  type);
      break;
  }
  return TRUE;
}

void stripFrontBytes(char *s, int num)
{
  char *c = s + num;

  while ((*(s++) = *(c++)));	// tight code rocks 
}

bool is_client_sstring(char *str)
{
  if (!str || !*str)
    return FALSE;

  if (((ubyte) *str) != ((ubyte) CLIENT_CODE_CHAR))
    return FALSE;

  stripFrontBytes(str, 1);

  return TRUE;
}

// returns DELETE_THIS
int Descriptor::client_nanny(char *arg)
{
  TBeing *tmp_ch;
  Descriptor *k;
  //char login[20], passwd[40], charname[20], buf[512], tmp_name[40], wizbuf[256];
  char login[20], passwd[40], charname[20], tmp_name[40];
  charFile st;
  TRoom *rp;
  int rc;
  static char * crypted;

  strcpy(login, nextToken('|', 19, arg).c_str());
  strcpy(passwd, nextToken('|', 39, arg).c_str());
  strcpy(charname, nextToken('|', 39, arg).c_str());

  if (!*login || !*passwd || !*charname) 
    return DELETE_THIS;
  
  account = new TAccount();
  account->name=login;
  if(account->read(login)){
    account->desc = this;
    strcpy(pwd, account->passwd.c_str());
  } else
    *pwd = '\0';

  if (!*pwd) {
    clientf(format("%d|No account %s exists! Please reenter account name, or create a new account.|%d") % CLIENT_ERROR % account->name % ERR_BAD_NAME);
    delete account;
    account = NULL;
    return FALSE;   
  }
  crypted = (char *) crypt(passwd, pwd);
  if (strncmp(crypted, pwd, 10)) {
    clientf(format("%d|Incorrect password.|%d") % CLIENT_ERROR % ERR_BAD_NAME);
    delete account;
    account = NULL;
    return FALSE;
  }
  account->status = TRUE;

#if 1
  // the non-client side presumes that character is ALWAYS newed and then
  // loadFromSt done.  Some events (especially if swapping from one char
  // to another) rely on this.  So...
  delete character;
  
  character = new TPerson(this);
#else
  if (!character) 
    character = new TPerson(this);
#endif
  
  if (_parse_name(charname, tmp_name)) {
    clientf(format("%d|No such character exists! Reenter character name or create a new character.|%d") % CLIENT_ERROR % ERR_BAD_NAME);

    // deletion at this point is semi-problematic
    // we need to remove desc so the doAccountMenu() in ~TBeing skips
    // but this means character_list assumes presence
    // temporarily shove them into the char_list, and delete will
    // remove them
    character->desc = NULL;
    character->next = character_list;
    character_list = character;

    if ((character->inRoom() >= 0) ||
        (character->inRoom() == Room::AUTO_RENT)) {
       //loadFromSt will have inRoom() == last rent
       //roomp not set yet, so just clear this value
      character->setRoom(Room::VOID);
    }
// added 4/1/98 to fix a crash -- cos
    rp = real_roomp(Room::VOID);
    *rp += *character;
    delete character;
    character = NULL;
    delete account;
    account = NULL;
    outputProcessing();
    return FALSE;
  }
  if (load_char(tmp_name, &st))
    dynamic_cast<TPerson *>(character)->loadFromSt(&st);
  else {
    clientf(format("%d|No such character exists! Reenter character name or create a new character.|%d") % CLIENT_ERROR % ERR_BAD_NAME);

    // deletion at this point is semi-problematic
    // we need to remove desc so the doAccountMenu() in ~TBeing skips
    // but this means character_list assumes presence
    // temporarily shove them into the char_list, and delete will
    // remove them
    character->desc = NULL;
    character->next = character_list;
    character_list = character;

    if ((character->inRoom() >= 0) ||
        (character->inRoom() == Room::AUTO_RENT)) {
      // loadFromSt will have inRoom() == last rent
      // roomp not set yet, so just clear this value
      character->setRoom(Room::VOID);
    }
// added 4/1/98 to fix a crash -- cos
    rp = real_roomp(Room::VOID);
    *rp += *character;
    delete character;
    character = NULL;
    delete account;
    account = NULL;
    return FALSE;
  }
  if (account->name!=st.aname) {
    clientf(format("%d|That character isn't in the listed account.|%d") % 
              CLIENT_ERROR % ERR_BAD_NAME);

    // loadFromSt has initted character (improperly)
    // delete it and recreate it so initialization will be proper

    // deletion at this point is semi-problematic
    // we need to remove desc so the doAccountMenu() in ~TBeing skips
    // but this means character_list assumes presence
    // temporarily shove them into the char_list, and delete will
    // remove them
    character->desc = NULL;
    character->next = character_list;
    character_list = character;

    if ((character->inRoom() >= 0) ||
        (character->inRoom() == Room::AUTO_RENT)) {
      // loadFromSt will have inRoom() == last rent
      // roomp not set yet, so just clear this value
      character->setRoom(Room::VOID);
    }
// added 4/1/98 to fix a crash -- cos
    rp = real_roomp(Room::VOID);
    *rp += *character;
    delete character;
    character = NULL;
    delete account;
    account = NULL;
    return FALSE;
  }
  if (WizLock) {
    // this may need better handling to let wizs in, but, oh well
    clientf(format("%d|The mud is presently Wizlocked.|%d") % 
                CLIENT_ERROR % ERR_NOT_ALLOWED);
    if (!lockmess.empty())
      clientf(lockmess);
    outputProcessing();

    return DELETE_THIS;
  }

  if(character->hasQuestBit(TOG_PERMA_DEATH_CHAR)){
    character->loadCareerStats();
    if(character->desc->career.deaths){
      
      writeToQ("That character is a perma death character and has died.\n\r");
      writeToQ("Name -> ");
      
      // copied from above
      character->desc = NULL;
      character->next = character_list;
      character_list = character;
      
      character->setRoom(Room::NOWHERE);
      
      delete character;
      character = new TPerson(this);
      return FALSE;
    }
  }


  for (k = descriptor_list; k; k = k->next) {
    if ((k->character != character) && k->character) {
      if (k->original) {
        if (k->original->getName() && !strcasecmp(k->original->getName(), character->getName())) {
          clientf(format("%d") % CLIENT_CONNECTED);
          return FALSE;
        }
      } else {
        if (k->character->getName() && !strcasecmp(k->character->getName(), character->getName())) {
          clientf(format("%d") % CLIENT_CONNECTED);
          return FALSE;
        }
      }
    }
  }
  for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
    if ((!strcasecmp(character->getName(), tmp_ch->getName()) &&
        !tmp_ch->desc && !dynamic_cast<TMonster *>(tmp_ch)) ||
        (dynamic_cast<TMonster *>(tmp_ch) && tmp_ch->orig &&
         !strcasecmp(character->getName(),
                  tmp_ch->orig->getName()))) {

      //clientf(format("%d|%d") % CLIENT_GLOBAL % 0);

      if ((character->inRoom() >= 0) ||
          (character->inRoom() == Room::AUTO_RENT)) {
        // loadFromSt will have inRoom() == last rent
        // roomp not set yet, so just clear this value
        character->setRoom(Room::VOID);
      }
      // we need to remove the character->desc for some deletion handling
      // to work (true, but why?).  This unfortunately causes it to go
      // into character_list removal logic inside ~TBeing.  Since we're
      // not actually in the char_list yet, it crashes.  Just insert myself
      // temporarily to bypass this.
      character->desc = NULL;
      character->next = character_list;
      character_list = character;

      rp = real_roomp(Room::VOID);
      *rp += *character;
      delete character;
      tmp_ch->desc = this;
      character = tmp_ch;
      tmp_ch->setTimer(0);
      tmp_ch->setInvisLevel(0);

      if (tmp_ch->orig) {
        tmp_ch->desc->original = tmp_ch->orig;
        tmp_ch->orig = 0;
      }
      connected = CON_PLYNG;
      // This is a semi-kludge to fix some extra crap we had being sent
      // upon reconnect - Russ 6/15/96
      flush();

      writeToQ("Reconnecting character...\n\r");
      send_client_prompt(TRUE, 16383);
      clientf(format("%d|%d") % CLIENT_ENABLEWINDOW % TRUE);
      if (tmp_ch->getCombatMode() == ATTACK_NORMAL)
        clientf(format("%d") % CLIENT_NORMAL);
      if (tmp_ch->getCombatMode() == ATTACK_OFFENSE)
        clientf(format("%d") % CLIENT_OFFENSIVE);
      if (tmp_ch->getCombatMode() == ATTACK_DEFENSE)
        clientf(format("%d") % CLIENT_DEFENSIVE);

      tmp_ch->initDescStuff(&st);

      if (should_be_logged(character)) {
        objCost cost;

        if (IS_SET(account->flags, TAccount::IMMORTAL)) {
          vlogf(LOG_PIO, format("%s[*masked*] has reconnected (client 2)  (account: *masked*).") % 
                character->getName());
        } else {
          vlogf(LOG_PIO, format("%s[%s] has reconnected (client 2)  (account: %s).") % 
                     character->getName() % host % account->name);
        }
        character->recepOffer(NULL, &cost);
        dynamic_cast<TPerson *>(character)->saveRent(&cost, FALSE, 1);
      }
      act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
      tmp_ch->loadCareerStats();
      tmp_ch->loadDrugStats();
      tmp_ch->loadGuildStats();
      tmp_ch->loadTitle();
      if (tmp_ch->getHit() < 0) 
        dynamic_cast<TPerson *>(tmp_ch)->autoDeath();
      
      rc = checkForMultiplay();
      if(Config::ForceMultiplayCompliance()){
	if (rc) {
	  // disconnect, but don't cause character to be deleted
	  // do this by disassociating character from descriptor
	  character = NULL;
	  
	  return DELETE_THIS;
	}
      }

      if (tmp_ch->hasClass(CLASS_CLERIC) || tmp_ch->hasClass(CLASS_DEIKHAN))
        clientf(format("%d") % CLIENT_PIETY);

      tmp_ch->fixClientPlayerLists(FALSE);
      return FALSE;
    }
  }
  if (should_be_logged(character)) {
    if (IS_SET(account->flags, TAccount::IMMORTAL)) {
      vlogf(LOG_PIO, format("%s[*masked*] has connected (client)  (account: *masked*).") % 
            character->getName());
    } else {
      vlogf(LOG_PIO, format("%s[%s] has connected (client)  (account: %s).") % 
                 character->getName() % host % account->name);
    }
  }
  sendMotd(character->GetMaxLevel() > MAX_MORT);
  if (character->hasClass(CLASS_CLERIC) || character->hasClass(CLASS_DEIKHAN))
    clientf(format("%d") % CLIENT_PIETY);

  clientf(format("%d|%d") % CLIENT_ENABLEWINDOW % TRUE);
  if (character->getCombatMode() == ATTACK_NORMAL)
    clientf(format("%d") % CLIENT_NORMAL);
  if (character->getCombatMode() == ATTACK_OFFENSE)
    clientf(format("%d") % CLIENT_OFFENSIVE);
  if (character->getCombatMode() == ATTACK_DEFENSE)
    clientf(format("%d") % CLIENT_DEFENSIVE);

  rc = dynamic_cast<TPerson *>(character)->genericLoadPC();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  character->doLook("", CMD_LOOK);
  prompt_mode = 1;
  character->fixClientPlayerLists(FALSE);
  dynamic_cast<TPerson *>(character)->fixPracs();
  return TRUE;
}


sstring WhoListComm::getText(){
  return "";
}

sstring WhoListComm::getClientText(){
  sstring buf;
  if(online){
    buf=format("\200%d|%s|%d|%d|1\n") % CLIENT_WHO % who % ADD % 
      ((level==-1)?0:level);
  } else {
    buf=format("\200%d|%s|%d|0\n") % CLIENT_WHO % who % DELETE;
    buf=format("\200%d|[%s]|%d|0\n") % CLIENT_WHO % who % DELETE;
  }
  return buf;
}

sstring WhoListComm::getXML(){
  sstring buf="";

  buf+=format("<wholist>\n");
  buf+=format("  <online>%s</online>\n") % (online ? "true" : "false");
  
  if(level!=-1)
    buf+=format("  <level>%i</level>\n") % level;
  if(idle!=-1)
    buf+=format("  <idle>%i</idle>\n") % idle;
  
  buf+=format("  <linkdead>%s</linkdead>\n") % (linkdead ? "true" : "false");
  buf+=format("  <name>%s</name>\n") % who.escape(sstring::XML);
  buf+=format("  <prof>%s</prof>\n") % prof.escape(sstring::XML);
  buf+=format("  <title>%s</title>\n") % title.escape(sstring::XML);
  buf+=format("</wholist>\n");

  return buf;
}


void TBeing::fixClientPlayerLists(bool lost)
{
  Descriptor *d;
  sstring prof, title;

  for (d = descriptor_list; d; d = d->next) {
    if (d->character){
      if(isImmortal())
	prof=msgVariables(MSG_IMM_TITLE);
      else
	prof=getProfName();

      title=parseTitle(d);


      // delete the entry first
      if(d->character->isImmortal())
	d->output.putInQ(new WhoListComm(getName(), false, GetMaxLevel(),
					 getTimer(), isLinkdead(), prof, 
					 title));
      else 
	d->output.putInQ(new WhoListComm(getName(), false, -1, -1, false, 
					 prof, title));

      d->prompt_mode = -1;

      if (!lost) {
	if(d->character->isImmortal()){
	  // immortals get all info
	  d->output.putInQ(new WhoListComm(getName(), true, GetMaxLevel(),
					   getTimer(), isLinkdead(), prof,
					   title));
	} else {
	  // mortals get filtered info
	  if (d->character->canSeeWho(this)) {
	    if (isPlayerAction(PLR_ANONYMOUS)){
	      d->output.putInQ(new WhoListComm(getName(), true, -1, -1, false,
					       prof, title));
	    } else {
	      d->output.putInQ(new WhoListComm(getName(), true, GetMaxLevel(), 
					       -1, false, prof, title));
	    }
	  }
	}
      }
    }
  }
  if (desc && desc->m_bIsClient) {
    desc->account->term = TERM_NONE;
    remPlayerAction(PLR_VT100);
    remPlayerAction(PLR_ANSI);
  }
}

void processStringForClient(sstring &sb)
{
  // Go thru and change all newlines to ^

  sstring::size_type pos;
  pos = sb.find("\n\r");
  while (pos != sstring::npos) {
    sb.replace(pos, 2, "^");
    pos = sb.find("\n\r");
  }

  pos = sb.find("\r\n");
  while (pos != sstring::npos) {
    sb.replace(pos, 2, "^");
    pos = sb.find("\r\n");
  }
}

int TBeing::doClientMessage(const char *arg)
{
  if (!desc)
     return FALSE;

#if 0
  if (!desc->m_bIsClient) {
    sendTo("This command is only available for users of the SneezyMUD client (http://sneezy.saw.net/client).\n\r");
    return FALSE;
  }
#endif
  Descriptor *i;
  TBeing *b;

  for (i = descriptor_list; i; i = i->next) {
    if ((b = i->character) && (b != this) && !i->connected && i->m_bIsClient)
      b->sendTo(COLOR_COMM, format("<p>CLIENT<1> (%s): %s\n\r") % getName() % arg);  
  }
  sendTo(COLOR_COMM, format("<p>CLIENT<1>: %s\n\r") % arg);
  return TRUE;
}

int Descriptor::clientCreateAccount(char *arg)
{
  static char *crypted;

  account= new TAccount;

  // Account name
  account->name="TestAccd";

  // Account password
  crypted =(char *) crypt("Test123", account->name.c_str());
  strncpy(pwd, crypted, 10);
  *(pwd + 10) = '\0';

  account->passwd=pwd;

  // Account email
  account->email="bogus@bogus.com";

  // Something with listserver

  // Account time
  account->time_adjust = 0;

  // Account term
  account->term = TERM_NONE;

  // Save all information
  if(!account->write(account->name)){
    vlogf(LOG_CLIENT, format("Big problems in saveAccount (s)") % 
	  account->name.lower());
    return FALSE;
  }

  AccountStats::account_number++;

  vlogf(LOG_MISC, format("New Client Account: '%s' with email '%s'") %  account->name % account->email);

  return TRUE;
}

int Descriptor::clientCreateChar(char *arg)
{
  char dummy[1024];
  //char tmp_name[256], wizbuf[256];
  char tmp_name[256];

  TPerson *ch;
  strcpy(dummy, nextToken('|', 20, arg).c_str());
  //Create the actual TPerson
  ch = new TPerson(this);
  mud_assert(ch != NULL, "Mem alloc problem");

  // Name
  if (_parse_name(dummy, tmp_name)) {
    clientf(format("%d|Name contains illegal characters!|%d") % CLIENT_ERROR % ERR_BAD_NAME);
    ch->desc = NULL;
    ch->next = character_list;
    character_list = ch;

    TRoom *rp = real_roomp(Room::VOID);
    *rp += *ch;
    delete ch;
    return FALSE;
  }
  if (checkForCharacter(tmp_name)) {
    clientf(format("%d|Character already exists with name provided|%d") % CLIENT_ERROR % ERR_BAD_NAME);
    ch->desc = NULL;
    ch->next = character_list;
    character_list = ch;
  
    TRoom *rp = real_roomp(Room::VOID);
    *rp += *ch;
    delete ch;
    return FALSE;
  }

  ch->name=mud_str_dup(sstring(tmp_name).cap());

  // Sex
  strcpy(dummy, nextToken('|', 1024, arg).c_str());
  switch(convertTo<int>(dummy)) { 
    case 0:
      ch->setSex(SEX_MALE);
      break;
    case 1:
      ch->setSex(SEX_FEMALE);
      break;
    default:
      ch->setSex(SEX_MALE);
      break;
  }

  // Hands
  strcpy(dummy, nextToken('|', 1024, arg).c_str());
  switch(convertTo<int>(dummy)) {
    case 0:
      ch->addPlayerAction(PLR_RT_HANDED);
      break;
    default:
      // Do nothing - This is determine by !RIGHT_HANDED - Russ 
      break;
  }

  // Race and Terrain
  switch (convertTo<int>(nextToken('|', 1024, arg))) {
    case 1:
      ch->setRace(RACE_HUMAN);
      switch (*(nextToken('|', 1024, arg).c_str())) {
        case '1':
          ch->player.hometerrain = HOME_TER_HUMAN_URBAN;
          break;
        case '2':
          ch->player.hometerrain = HOME_TER_HUMAN_VILLAGER;
          break;
        case '3':
          ch->player.hometerrain = HOME_TER_HUMAN_PLAINS;
          break;
        case '4':
          ch->player.hometerrain = HOME_TER_HUMAN_RECLUSE;
          break;
        case '5':
          ch->player.hometerrain = HOME_TER_HUMAN_HILL;
          break;
        case '6':
          ch->player.hometerrain = HOME_TER_HUMAN_MOUNTAIN;
          break;
        case '7':
          ch->player.hometerrain = HOME_TER_HUMAN_FOREST;
          break;
        case '8':
          ch->player.hometerrain = HOME_TER_HUMAN_MARINER;
          break;
        default:
          // Send error message to Client
          break;
      }
      break;
    case 2:
      ch->setRace(RACE_ELVEN);
      break;
    case 3:
      ch->setRace(RACE_DWARF);
      break;
    case 4:
      ch->setRace(RACE_HOBBIT);
      break;
    case 5:
      ch->setRace(RACE_GNOME);
      break;
    case 6:
      ch->setRace(RACE_OGRE);
      break;
    default:
      ch->setRace(RACE_HUMAN);
      switch (*(nextToken('|', 1024, arg).c_str())) {
        case '1':
          ch->player.hometerrain = HOME_TER_HUMAN_URBAN;
          break;
        case '2':
          ch->player.hometerrain = HOME_TER_HUMAN_VILLAGER;
          break;
        case '3':
          ch->player.hometerrain = HOME_TER_HUMAN_PLAINS;
          break;
        case '4':
          ch->player.hometerrain = HOME_TER_HUMAN_RECLUSE;
          break;
        case '5':
          ch->player.hometerrain = HOME_TER_HUMAN_HILL;
          break;
        case '6':
          ch->player.hometerrain = HOME_TER_HUMAN_MOUNTAIN;
          break;
        case '7':
          ch->player.hometerrain = HOME_TER_HUMAN_FOREST;
          break;
        case '8':
          ch->player.hometerrain = HOME_TER_HUMAN_MARINER;
          break;
        default:
          // Send error message to Client
          break;
      }
      break;
  } 

  character = ch;

  // Class
  switch (*(nextToken('|', 1024, arg).c_str())) {        
    case '1':
      ch->setClass(CLASS_WARRIOR);
      break;
    case '2':
      ch->setClass(CLASS_CLERIC);
      break;
    case '3':
      ch->setClass(CLASS_MAGE);
      break;
    case '4':
      ch->setClass(CLASS_THIEF);
      break;
    default:
      //Send Client error message
      break;
  }
  character = NULL;

  //Stats

  ch->chosenStats.values[STAT_STR] = convertTo<int>(nextToken('|', 1024, arg));
  ch->chosenStats.values[STAT_BRA] = convertTo<int>(nextToken('|', 1024, arg));
  ch->chosenStats.values[STAT_CON] = convertTo<int>(nextToken('|', 1024, arg));
  ch->chosenStats.values[STAT_DEX] = convertTo<int>(nextToken('|', 1024, arg));
  ch->chosenStats.values[STAT_AGI] = convertTo<int>(nextToken('|', 1024, arg));
  ch->chosenStats.values[STAT_INT] = convertTo<int>(nextToken('|', 1024, arg));
  ch->chosenStats.values[STAT_WIS] = convertTo<int>(nextToken('|', 1024, arg)); 
  ch->chosenStats.values[STAT_FOC] = convertTo<int>(nextToken('|', 1024, arg));
  ch->chosenStats.values[STAT_PER] = convertTo<int>(nextToken('|', 1024, arg));
  ch->chosenStats.values[STAT_CHA] = convertTo<int>(nextToken('|', 1024, arg));
  ch->chosenStats.values[STAT_KAR] = convertTo<int>(nextToken('|', 1024, arg));
  ch->chosenStats.values[STAT_SPE] = convertTo<int>(nextToken('|', 1024, arg));

  // Check if everything sums to 0, if not send an error message. 

  if (ch->chosenStats.values[STAT_STR] +
      ch->chosenStats.values[STAT_BRA] +
      ch->chosenStats.values[STAT_CON] +
      ch->chosenStats.values[STAT_DEX] +
      ch->chosenStats.values[STAT_AGI] +
      ch->chosenStats.values[STAT_INT] +
      ch->chosenStats.values[STAT_WIS] +
      ch->chosenStats.values[STAT_FOC] +
      ch->chosenStats.values[STAT_PER] +
      ch->chosenStats.values[STAT_CHA] +
      ch->chosenStats.values[STAT_KAR] +
      ch->chosenStats.values[STAT_SPE])  {
    clientf(format("%d|Stats do not add up to 0. Email being sent to Brutius to alert of possible client hack.|%d") % CLIENT_ERROR % ERR_BAD_STAT);
    ch->desc = NULL;
    ch->next = character_list;
    character_list = ch;
  
    TRoom *rp = real_roomp(Room::VOID);
    *rp += *ch;
    delete ch;
    return FALSE;
  }

  // Things done on ENTER_DONE

  ch->convertAbilities();
  ch->affectTotal();
  vlogf(LOG_PIO, format("%s [%s] new player.") %  ch->getName() % host);
  clientf(format("%d") % CLIENT_NEWCHAR);

  enum connectStateT oldconnected = connected;
  connected = CON_PLYNG;
  ch->desc = this;
  AccountStats::player_count++;

  dynamic_cast<TPerson *>(ch)->doStart();
  ch->saveChar(Room::AUTO_RENT);
  connected = oldconnected;
  dynamic_cast<TPerson *>(ch)->dropItemsToRoom(SAFE_YES, NUKE_ITEMS);

  ch->desc = NULL;
  ch->next = character_list;
  character_list = ch;

  TRoom *rp = real_roomp(Room::VOID);
  *rp += *ch;
  delete ch;
  delete account;
  account = NULL;
  return TRUE;
}

