//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: mail.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "mail.h"

static const char * const MAIL_FILE = "mobdata/mail";
// may not exceed NAME_SIZE (15) chars
static const char * const SNEEZY_ADMIN = "Grimhaven Admin";
static const char * const DEAD_LETTER_GOD = "Batopr";

mail_index_type *mail_index = 0;        /* list of recs in the mail file  */
position_list_type *free_list = 0;      /* list of free positions in file */
long file_end_pos = 0;          /* length of file */

void push_free_list(long pos)
{
  position_list_type *new_pos;

  new_pos = new position_list_type();
  new_pos->position = pos;
  new_pos->next = free_list;
  free_list = new_pos;
}

long pop_free_list(void)
{
  position_list_type *old_pos;
  long return_value;

  if ((old_pos = free_list) != 0) {
    return_value = free_list->position;
    free_list = old_pos->next;
    delete old_pos;
    return return_value;
  } else
    return file_end_pos;
}

mail_index_type *find_char_in_index(const char *searchee)
{
  mail_index_type *temp_rec;

  if (!*searchee) {
    vlogf(8, "Mail system -- non fatal error #1.");
    return 0;
  }
  for (temp_rec = mail_index;
       (temp_rec && strcasecmp(temp_rec->recipient, searchee));
       temp_rec = temp_rec->next);

  return temp_rec;
}

void write_to_file(void *buf, int size, long filepos)
{
  if (no_mail)
    return;

  FILE *mail_file;

  mail_file = fopen(MAIL_FILE, "r+b");

  if (filepos % BLOCK_SIZE) {
    vlogf(8, "Mail system -- fatal error #2!!!");
    no_mail = 1;
    if (mail_file)
      fclose(mail_file);
    return;
  }
  fseek(mail_file, filepos, 0);
  fwrite(buf, size, 1, mail_file);

  // find end of file
  fseek(mail_file, 0L, 2);
  file_end_pos = ftell(mail_file);
  fclose(mail_file);
  return;
}

void read_from_file(void *buf, int size, long filepos)
{
  if (no_mail)
    return;

  FILE *mail_file;

  mail_file = fopen(MAIL_FILE, "r+b");

  if (filepos % BLOCK_SIZE) {
    vlogf(8, "Mail system -- fatal error #3!!!");
    no_mail = 1;
    if (mail_file)
      fclose(mail_file);
    return;
  }
  fseek(mail_file, filepos, 0);
  fread(buf, size, 1, mail_file);
  fclose(mail_file);
  return;
}

void index_mail(const char *raw_name_to_index, long pos)
{
  mail_index_type *new_index;
  position_list_type *new_position;
  char name_to_index[100];      /* I'm paranoid.  so sue me. */
  const char *src;
  int i;

  if (!raw_name_to_index || !*raw_name_to_index) {
    vlogf(8, "Mail system -- non-fatal error #4.");
    return;
  }
  for (src = raw_name_to_index, i = 0; *src;)
    if (isupper(*src))
      name_to_index[i++] = tolower(*src++);
    else
      name_to_index[i++] = *src++;
  name_to_index[i] = 0;

  if (!(new_index = find_char_in_index(name_to_index))) {
   /* name not already in index.. add it */
    new_index = new mail_index_type();
    strncpy(new_index->recipient, name_to_index, NAME_SIZE);
    new_index->recipient[strlen(name_to_index)] = '\0';
    new_index->list_start = 0;

   /* add to front of list */
    new_index->next = mail_index;
    mail_index = new_index;
  }
 /* now, add this position to front of position list */
  new_position = new position_list_type();
  new_position->position = pos;
  new_position->next = new_index->list_start;
  new_index->list_start = new_position;
}

// scan_file is called once during boot-up.  It scans through the
// mailfile and indexes all entries currently in the mail file. 
bool scan_file(silentTypeT silent)
{
  FILE *mail_file;
  header_block_type next_block;
  int total_messages = 0, block_num = 0;

  if (!(mail_file = fopen(MAIL_FILE, "r"))) {
    vlogf(0, "Mail file non-existant... creating new file.");
    mail_file = fopen(MAIL_FILE, "w");
    fclose(mail_file);
    return 1;
  }
  while (fread(&next_block, sizeof(header_block_type), 1, mail_file)) {
    if (next_block.block_type == HEADER_BLOCK) {
      index_mail(next_block.to, block_num * BLOCK_SIZE);
      total_messages++;

      if ((total_messages%50) == 0)
        bootPulse(".", false);

    } else if (next_block.block_type == DELETED_BLOCK)
      push_free_list(block_num * BLOCK_SIZE);

    block_num++;
  }
  file_end_pos = ftell(mail_file);
  fclose(mail_file);

  if (!silent)
    vlogf(0, "   %ld bytes read.", file_end_pos);

  if (file_end_pos % BLOCK_SIZE) {
    vlogf(0, "Error booting mail system -- Mail file corrupt!");
    vlogf(0, "Mail disabled!");
    return 0;
  }
  bootPulse(NULL, true);

  if (!silent)
    vlogf(0, "   Mail file read -- %d messages.", total_messages);

  mail_index_type *temp_rec;
  temp_rec = mail_index;
  unsigned int mail_count = 0;

  // check the deliverability of all the mail so that we purge mail for
  // which there is no player...  Also, remove mail for player's that never
  // log in...
  bootPulse("Checking mail deliverability:", false);
  while (temp_rec && !no_mail) {
    mail_count++;
    if ((mail_count%50) == 0)
      bootPulse(".", false);

    // mail recipient no longer exists (dead letter)
    // this check for "batopr" is because 1st boot after pwipe causes
    // problems since batopr also doesn't exist we go into nasty loop
    if (strcmp(temp_rec->recipient, DEAD_LETTER_GOD)) {
      charFile st;
      bool lc = load_char(temp_rec->recipient, &st);
      bool del_me = true;
      if (lc) {
        unsigned long delivery_days = (time(0) - st.last_logon)/SECS_PER_REAL_DAY;
        if (delivery_days < 30LU)
          del_me = false;
      }
      if (del_me) {
        char *tmp;
        vlogf(5, "Dead mail message exists to %s, removing",
               temp_rec->recipient);
#if 0
  // I don't want to receive this shit anymore !!!
        char deadbuf[sizeof(next_block.txt)];
        sprintf(deadbuf, "Dead letter originally to %s\n\r",
               temp_rec->recipient);
  
        tmp = read_delete(temp_rec->recipient, temp_rec->recipient); 
        sprintf(deadbuf + strlen(deadbuf), tmp);
        delete [] tmp;
        autoMail(NULL, DEAD_LETTER_GOD, deadbuf);
#else
        tmp = read_delete(temp_rec->recipient, temp_rec->recipient);
        delete [] tmp;
#endif

        // OK, start the check over since we've removed one and added one...
        temp_rec = mail_index;
      } else
        temp_rec = temp_rec->next;
    } else
      temp_rec = temp_rec->next;
  }
  return 1;
}                               /* end of scan_file */

bool has_mail(const char *recipient)
{
  
  if (find_char_in_index(recipient))
    return TRUE;

  return FALSE;
}

void store_mail(const char *to, const char *from, const char *message_pointer)
{
  header_block_type header;
  data_block_type data;
  long last_address, target_address;
  const char *msg_txt = message_pointer;
  char *tmp;
  int bytes_written = 0;
  int total_length = strlen(message_pointer);

  mud_assert(sizeof(header_block_type) == sizeof(data_block_type), 
        "store_mail: point 1");
  mud_assert(sizeof(header_block_type) == BLOCK_SIZE,
        "store_mail: point 2");

  if (!*from || !*to || !*message_pointer) {
    vlogf(0, "Mail system -- non-fatal error #5.");
    return;
  }
  memset((char *) &header, 0, sizeof(header));   /* clear the record */
  header.block_type = HEADER_BLOCK;
  header.next_block = LAST_BLOCK;
  strncpy(header.txt, msg_txt, HEADER_BLOCK_DATASIZE);
  strncpy(header.from, from, NAME_SIZE);
  strncpy(header.to, to, NAME_SIZE);
  for (tmp = header.to; *tmp; tmp++)
    if (isupper(*tmp))
      *tmp = tolower(*tmp);

  header.mail_time = time(0);
  header.txt[HEADER_BLOCK_DATASIZE] = header.from[NAME_SIZE] = header.to[NAME_SIZE] = '\0';

  target_address = pop_free_list();     /* find next free block */
  index_mail(to, target_address);       /* add it to mail index in memory */
  write_to_file(&header, BLOCK_SIZE, target_address);

  if (strlen(msg_txt) <= HEADER_BLOCK_DATASIZE)
    return;                     /* that was the whole message */

  bytes_written = HEADER_BLOCK_DATASIZE;
  msg_txt += HEADER_BLOCK_DATASIZE;     /* move pointer to next bit of text */

  last_address = target_address;
  target_address = pop_free_list();
  header.next_block = target_address;
  write_to_file(&header, BLOCK_SIZE, last_address);

  memset((char *) &data, 0, sizeof(data));       /* clear the record */
  data.block_type = LAST_BLOCK;
  strncpy(data.txt, msg_txt, DATA_BLOCK_DATASIZE);
  data.txt[DATA_BLOCK_DATASIZE] = '\0';
  write_to_file(&data, BLOCK_SIZE, target_address);
  bytes_written += strlen(data.txt);
  msg_txt += strlen(data.txt);

  while (bytes_written < total_length) {
    last_address = target_address;
    target_address = pop_free_list();

    data.block_type = target_address;
    write_to_file(&data, BLOCK_SIZE, last_address);

    data.block_type = LAST_BLOCK;
    strncpy(data.txt, msg_txt, DATA_BLOCK_DATASIZE);
    data.txt[DATA_BLOCK_DATASIZE] = '\0';
    write_to_file(&data, BLOCK_SIZE, target_address);

    bytes_written += strlen(data.txt);
    msg_txt += strlen(data.txt);
  }
}                               /* store mail */

char *read_delete(const char *recipient, const char *recipient_formatted)
{
  header_block_type header;
  data_block_type data;
  mail_index_type *mail_pointer, *prev_mail;
  position_list_type *position_pointer;
  long mail_address, following_block;
  char *message, *tmstr, buf[200];
  size_t string_size;

  if (!*recipient || !*recipient_formatted) {
    vlogf(0, "Mail system -- non-fatal error #6.");
    return NULL;
  }
  if (!(mail_pointer = find_char_in_index(recipient))) {
    vlogf(0, "Stupid post-office-spec_proc-error");
    return NULL;
  }
  if (!(position_pointer = mail_pointer->list_start)) {
    vlogf(0, "Stupid Rasmussen error!");
    return NULL;
  }
  if (!(position_pointer->next)) {      /* just 1 entry in list. */
    mail_address = position_pointer->position;
    delete position_pointer;

   /* now free up the actual name entry */
    if (mail_index == mail_pointer) {   /* name is 1st in list */
      mail_index = mail_pointer->next;
      delete mail_pointer;
    } else {
     /* find entry before the one we're going to del */
      for (prev_mail = mail_index; prev_mail->next != mail_pointer; prev_mail = prev_mail->next);
      prev_mail->next = mail_pointer->next;
      delete mail_pointer;
    }
  } else {
    while (position_pointer->next->next)
      position_pointer = position_pointer->next;
    mail_address = position_pointer->next->position;
    delete position_pointer->next;
    position_pointer->next = NULL;
  }
  read_from_file(&header, BLOCK_SIZE, mail_address);

  if (header.block_type != HEADER_BLOCK) {
    vlogf(0, "Oh dear.  btype %ld/%ld", header.block_type, HEADER_BLOCK);
    no_mail = 1;
    vlogf(0, "Mail system disabled!");
    return NULL;
  }
  tmstr = asctime(localtime(&header.mail_time));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  sprintf(buf, " * * * * %s Mail System * * * *\n\r"
          "Date: %s\n\r"
          "  To: %s\n\r"
          "From: %s\n\r\n\r",
          MUD_NAME, 
          tmstr,
          recipient_formatted,
          header.from);

  string_size = ((strlen(buf) + strlen(header.txt) + 1));
  message = new char[string_size];
  strcpy(message, buf);
  strcat(message, header.txt);
  following_block = header.next_block;

  /* mark the block as deleted */
  header.block_type = DELETED_BLOCK;
  write_to_file(&header, BLOCK_SIZE, mail_address);
  push_free_list(mail_address);

  while (following_block != LAST_BLOCK && !no_mail) {
    read_from_file(&data, BLOCK_SIZE, following_block);

    string_size = ((strlen(message) + strlen(data.txt) + 1));
    char *tmpm = message;
    message = new char[string_size];
    strcpy(message, tmpm);
    strcat(message, data.txt);
    delete [] tmpm;
    mail_address = following_block;
    following_block = data.block_type;
    data.block_type = DELETED_BLOCK;
    write_to_file(&data, BLOCK_SIZE, mail_address);
    push_free_list(mail_address);
  }
  return message;
}

int postmaster(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  if (!ch->desc)
    return FALSE;               /* so mobs don't get caught here */

  switch (cmd) {
    case CMD_MAIL: 
      ch->postmasterSendMail(arg, myself);
      return TRUE;
      break;
    case CMD_CHECK: 
      ch->postmasterCheckMail(myself);
      return TRUE;
      break;
    case CMD_RECEIVE:
      ch->postmasterReceiveMail(myself);
      return TRUE;
      break;
    default:
      return FALSE;
      break;
  }
}

int mail_ok(TBeing *ch)
{
  if (no_mail) {
    ch->sendTo("Sorry, the mail system is having technical difficulties.\n\r");
    return FALSE;
  }
  return TRUE;
}

void TBeing::postmasterSendMail(const char *arg, TMonster *me)
{
  char buf[200], recipient[100], *tmp;
  charFile st;
  int i, imm = FALSE;

// added this check - bat
  if (!mail_ok(this))
    return;

  if (!*arg) {
    sprintf(buf, "%s You need to specify an addressee!", getName());
    me->doTell(buf);
    return;
  }
  if (_parse_name(arg, recipient)) {
    sendTo("Illegal name, please try another.\n\r");
    return;
  }
  for (tmp = recipient; *tmp; tmp++)
    if (isupper(*tmp))
      *tmp = tolower(*tmp);

  if (!load_char(recipient, &st)) {
    sendTo("No such player to mail to!\n\r");
    return;
  }
  imm = isImmortal();

  for (i = 0;!imm && i < 8; i++)
    if (st.level[i] > MAX_MORT)
      imm = TRUE;

  // let anybody mail to immortals
  if (GetMaxLevel() < MIN_MAIL_LEVEL && !imm) {
    sprintf(buf, "%s Sorry, you have to be level %d to send mail!",
            getName(), MIN_MAIL_LEVEL);
    me->doTell(buf);
    return;
  }

  if (getMoney() < STAMP_PRICE && !imm) {
    sprintf(buf, "%s A stamp costs %d talens.", fname(name).c_str(), STAMP_PRICE);
    me->doTell(buf);
    sprintf(buf, "%s ...which I see you can't afford.", fname(name).c_str());
    me->doTell(buf);
    return;
  }

  act("$n starts to write some mail.", TRUE, this, 0, 0, TO_ROOM);
  if (!imm) {
    sprintf(buf, "%s I'll take %d talens for the stamp.", fname(name).c_str(), 
         STAMP_PRICE);
    me->doTell(buf);
    addToMoney(-STAMP_PRICE, GOLD_HOSPITAL);
  } else if (isImmortal()) {
    sprintf(buf, "%s Since you're high and mighty, I'll waive the fee.",
         fname(name).c_str());
    me->doTell(buf);
  } else {
    sprintf(buf, "%s Since you're mailing an immortal, I'll waive the fee.",
         fname(name).c_str());
    me->doTell(buf);
  }
  if (!desc->client) {
    sprintf(buf, "%s Write your message, use ~ when done, or ` to cancel.", fname(name).c_str());
    me->doTell(buf);
    addPlayerAction(PLR_MAILING);
    desc->connected = CON_WRITING;
    strcpy(desc->name, recipient);

    desc->str = new (char *);
    *desc->str = new char[1];
    *(*desc->str) = '\0';
    desc->max_str = MAX_MAIL_SIZE;
  }
  if (desc->client)
    desc->clientf("%d|%s", CLIENT_MAIL, recipient);
}


void TBeing::postmasterCheckMail(TMonster *me)
{
  char buf[200], recipient[100], *tmp;

  _parse_name(getName(), recipient);

// added this check - bat
  if (!mail_ok(this))
    return;

  for (tmp = recipient; *tmp; tmp++)
    if (isupper(*tmp))
      *tmp = tolower(*tmp);

  if (has_mail(recipient))
    sprintf(buf, "%s You have mail waiting.", getName());
  else
    sprintf(buf, "%s Sorry, you don't have any mail waiting.", getName());

  me->doTell(buf);
}

void TBeing::postmasterReceiveMail(TMonster *me)
{
  char buf[200], recipient[100], *tmp;
  TObj *note;

  _parse_name(getName(), recipient);

  // added this check - bat
  if (!mail_ok(this))
    return;

  for (tmp = recipient; *tmp; tmp++)
    if (isupper(*tmp))
      *tmp = tolower(*tmp);

  if (!has_mail(recipient)) {
    sprintf(buf, "%s Sorry, you don't have any mail waiting.", fname(name).c_str());
    me->doTell(buf);
    return;
  }
  while (has_mail(recipient)) {
    if (!(note = read_object(GENERIC_NOTE, VIRTUAL))) {
      vlogf(10, "Couldn't make a note removed from board!");
      return;
    }
    note->swapToStrung();
    delete [] note->name;
    note->name = mud_str_dup("mail paper letter");
    delete [] note->shortDescr;
    note->shortDescr = mud_str_dup("a piece of mail"); 
    delete [] note->getDescr();
    note->setDescr(mud_str_dup("Someone has left a piece of mail here."));
    delete [] note->action_description;
    note->action_description = read_delete(recipient, getName());
    if (!note->action_description)
      note->action_description = mud_str_dup("Mail system buggy, please report!!  Error #8.\n\r");

    *this += *note;

    act("$n gives you a piece of mail.", FALSE, me, 0, this, TO_VICT);
    act("$N gives $n a piece of mail.", FALSE, this, 0, me, TO_ROOM);
  }
}

void autoMail(TBeing *ch, const char *targ, const char *msg)
{
  // from field limited to 15 chars by mail structure

  if (ch)
    store_mail(ch->getName(), SNEEZY_ADMIN, msg);
  else if (targ)
    store_mail(targ, SNEEZY_ADMIN, msg);
  else
    vlogf(5, "Error in autoMail");

  return;
}

position_list_type::position_list_type() :
  position(0L),
  next(NULL)
{
}

position_list_type::~position_list_type()
{
}

mail_index_type::mail_index_type() :
  list_start(NULL),
  next(NULL)
{
  *recipient = '\0';
}

mail_index_type::~mail_index_type()
{
}

void cleanUpMail()
{
  while (mail_index) {
    mail_index_type *tmp = mail_index;
    mail_index = mail_index->next;
    delete tmp;
  }
  while (free_list) {
    position_list_type * tmp = free_list;
    free_list = free_list->next;
    delete tmp;
  }
}
