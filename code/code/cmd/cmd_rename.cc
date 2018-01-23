/*************************************************************************

      SneezyMUD++ - All rights reserved, SneezyMUD Coding Team
      "cmd_rename.cc" - the rename command and related things

*************************************************************************/

#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "extern.h"
#include "handler.h"
#include "being.h"
#include "database.h"
#include "charfile.h"
#include "account.h"
#include "monster.h"

static void renamePersonalizeFix(TThing *t, const char * orig_name, const char * new_name)
{
  if (!t)
    return;
  TObj *obj = dynamic_cast<TObj *>(t);
  if (!obj)
    return;

  if (obj->isPersonalized() && !obj->action_description.empty()) {
    char buf[256], persbuf[256];
    strcpy(buf, obj->action_description.c_str());
    if ((sscanf(buf, "This is the personalized object of %s.", persbuf)) == 1) {
      if (!strcmp(persbuf, orig_name)) {
        // we are personalized already, so not bothering to swap to strung again
        vlogf(LOG_MISC, format("Personalized object (%s) on %s, being restrung.") % 
           obj->getName() % new_name);
        sprintf(buf, "This is the personalized object of %s", new_name);
        obj->action_description = buf;
      }
    }
  }

}

void renamePersonalizeFix(StuffList list, const char * orig_name, const char * new_name)
{
  for(StuffIter it=list.begin();it!=list.end();++it){
    renamePersonalizeFix(*it, orig_name, new_name);
    renamePersonalizeFix((*it)->stuff, orig_name, new_name);
  }

}


void TBeing::doNameChange(const char *argument)
{
  char orig_name[80], new_name[80], tmp_name[80];
  char tmpbuf2[160], arg[80];
  sstring tmpbuf;
  TBeing *vict;
  FILE *fp;

  argument = one_argument(argument, orig_name, cElements(orig_name));
  argument = one_argument(argument, new_name, cElements(new_name));

  if (!*orig_name || !*new_name) {
    sendTo("Syntax: rename <person> <new_name>\n\r");
    return;
  }
  if (_parse_name(new_name, tmp_name)) {
    sendTo("Illegal name for new_name.\n\r");
    return;
  }
  if (!(vict = get_char_room_vis(this, orig_name))) {
    sendTo("You don't see them here.\n\r");
    return;
  }
  // in case they abbreviated the person's name, grab the true name
  strcpy(orig_name, vict->getName().c_str());

  TMonster *mons = dynamic_cast<TMonster *>(vict);
  if (!isImmortal() || !hasWizPower(POWER_RENAME)) {
    // mortal rename for pets
    // have both checks above so Imm can go mort to rename a pet

    if (!mons) {
      sendTo("You can't rename that.\n\r");
      return;
    }

    if (mons->mobVnum() <= 0) {
      sendTo("You can't rename that.\n\r");
      return;
    }
    if (mons->isPc()) {
      sendTo("You can't rename players.\n\r");
      return;
    }
    if (IS_SET(mons->specials.act, ACT_STRINGS_CHANGED)) {
      act("$N already has a name.", FALSE, this, 0, mons, TO_CHAR);
      return;
    }
    if (!isImmortal()) {
      if (!mons->isPet(PETTYPE_PET | PETTYPE_THRALL) || mons->master != this) {
        act("$N is not your pet.", FALSE, this, 0, mons, TO_CHAR);
        return;
      }
    }
    charFile st;
    if (load_char(new_name, &st)) {
      act("You can't name $N that.", FALSE, this, 0, mons, TO_CHAR);
      return;
    }
    char tmpname[20];
    if (_parse_name(new_name, tmpname)) {
      act("You can't name $N that.", FALSE, this, 0, mons, TO_CHAR);
      return;
    }
    
    mons->swapToStrung();

    //  Remake the pet's name.  
    tmpbuf = format("%s %s") % mons->name % new_name;
    mons->name = tmpbuf;

    // remake the short desc
    //    sprintf(tmpbuf2, stripColorCodes(mons->getName()).c_str());
    sprintf(tmpbuf2, "%s", mons->getName().c_str());
    one_argument(tmpbuf2, arg, cElements(arg));
    if (!strcmp(arg, "a") || !strcmp(arg, "an"))
      tmpbuf=format("\"%s\", the %s") % sstring(new_name).cap() %
	one_argument(tmpbuf2, arg, cElements(arg));
    else
      tmpbuf = format("\"%s\", %s") % sstring(new_name).cap() % mons->getName();

    mons->shortDescr = tmpbuf;

    // remake the long desc
    tmpbuf += " is here.\n\r";
    mons->player.longDescr = tmpbuf;

    act("You grant $N a new name.",
        FALSE, this, 0, mons, TO_CHAR);
    act("$n grants $N a new name.",
        FALSE, this, 0, mons, TO_NOTVICT);
    act("$n grants you a new name.",
        FALSE, this, 0, mons, TO_VICT);
    return;
  }

  if (vict == this) {
    sendTo("You can't change your own name, sorry.\n\r");
    return;
  }
  if (!vict->desc) {
    sendTo("That creature doesn't seem to be connected.\n\r");
    return;
  }
  if (!vict->desc->account) {
    sendTo("That creature lacks an account.\n\r");
    return;
  }
  if (desc->checkForCharacter(tmp_name)) {
    sendTo("That player already exists.\n\r");
    return;
  }
  
  // check for corspse file
  tmpbuf = format("corpses/%s") % sstring(orig_name).lower();
  if ((fp = fopen(tmpbuf.c_str(), "r"))) {
    fclose(fp);
    sendTo("That player has a corpse file.\n\r");
    return;
  }

  strcpy(tmp_name, sstring(tmp_name).cap().c_str());
  vict->name = tmp_name;

  wipePlayerFile(orig_name);
  wipeRentFile(orig_name);

  TDatabase db(DB_SNEEZY);

  db.query("update player set name=lower('%s') where name=lower('%s')", 
	   tmp_name, orig_name);

  tmpbuf=format("account/%c/%s/%s") % LOWER(vict->desc->account->name[0]) %
    sstring(vict->desc->account->name).lower() % sstring(orig_name).lower();

  if (unlink(tmpbuf.c_str()) != 0)
    vlogf(LOG_FILE, format("error in unlink (11) (%s) %d") %  tmpbuf % errno);
  
  if (vict->GetMaxLevel() > MAX_MORT) {
    tmpbuf=format("mv immortals/%s/ immortals/%s/") % orig_name % sstring(tmp_name).cap();
    vsystem(tmpbuf);
  }

  vict->doSave(SILENT_NO);
  tmpbuf=format("The World shall now know %s as %s.") % sstring(orig_name).cap() %
    vict->getName();
  doSystem(tmpbuf);
  vict->fixClientPlayerLists(FALSE);

  // OK, now for some fun
  // what if they had personalized items, gotta change the damn names...
  wearSlotT slot;
  for (slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
    renamePersonalizeFix(vict->equipment[slot], orig_name, tmp_name);
  }
  // and again for inventory
  renamePersonalizeFix(vict->stuff, orig_name, tmp_name);
}

void TBeing::doDescription()
{
  if (!desc)
    return ;

  if (fight())  {
    sendTo("You cannot perform that action while fighting!\n\r");
    return;
  }
  if (!isPc()) {
    sendTo("Ugly monsters like you can't change a description!\n\r");
    return;
  }
#if 0 
  if (desc->m_bIsClient) {
    // it winds up sending their desc as a bug report...

    sendTo("Sorry, descriptions can not be editted through the client.\n\r");
    return;
  }
#endif
  sendTo("Enter your description. Maximum length is 500 characters\n\r");
  sendTo("End the description with a '~'.  Use '`' to cancel.\n\r");
  descr = "";
  desc->connected = CON_WRITING;
  desc->str = &descr;
  desc->edit_str_maxlen = 500;
#if 0
  if (desc->m_bIsClient)
    desc->clientf(format("%d|%d") % CLIENT_STARTEDIT % 500);
#endif
}
