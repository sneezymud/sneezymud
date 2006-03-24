/******* MUD MAIL SYSTEM MAIN FILE ***************************
 ***     written by Rasmussen (jelson@server.cs.jhu.edu)   ***
 ****   compliments of CircleMUD (circle.cs.jhu.edu 4000) ****
 ************************************************************/

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mail.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"

#define MAIL_FILE "mail"

void postmaster_send_mail(struct char_data *ch, int cmd, char *arg);
void postmaster_check_mail(struct char_data *ch, int cmd, char *arg);
void postmaster_receive_mail(struct char_data *ch, int cmd, char *arg);

extern struct index_data *mob_index;
extern struct obj_data *object_list;
extern int no_mail;
int find_name(char *name);
struct char_data *FindMobInRoomWithFunction(int room, int (*func)());


mail_index_type		*mail_index = 0; /* list of recs in the mail file  */
position_list_type 	*free_list = 0;  /* list of free positions in file */
long			file_end_pos = 0; /* length of file */

 
void push_free_list(long pos)
{
	position_list_type *new_pos;
 
	new_pos = (position_list_type *)malloc(sizeof(position_list_type));
	new_pos->position = pos;
	new_pos->next = free_list;
	free_list = new_pos;
}
 
long pop_free_list(void)
{
	position_list_type *old_pos;
	long return_value;
 
	if ((old_pos = free_list) != 0)
	{
		return_value = free_list->position;
		free_list = old_pos->next;
		free(old_pos);
		return return_value;
	}
	else
		return file_end_pos;
}
 
 
mail_index_type *find_char_in_index(char *searchee)
{
	mail_index_type *temp_rec;

	if(!*searchee) {
		vlog("Mail system -- non fatal error #1.");
		return 0;
	}

	for(temp_rec = mail_index;
	    (temp_rec && str_cmp(temp_rec->recipient, searchee));
	    temp_rec = temp_rec->next);
 
	return temp_rec;
}
 
 
void write_to_file(void *buf, int size, long filepos)
{
	FILE *mail_file;
 
	mail_file = fopen(MAIL_FILE, "r+b");
 
	if (filepos % BLOCK_SIZE)
	{
		vlog("Mail system -- fatal error #2!!!");
		no_mail = 1;
		return;
	}
 
	fseek(mail_file, filepos, 0);
	fwrite(buf, size, 1, mail_file);
 
	/* find end of file */
	fseek(mail_file, 0L, 2);
	file_end_pos = ftell(mail_file);
 	fclose(mail_file);
	return;
}


void read_from_file(void *buf, int size, long filepos)
{
	FILE *mail_file;
 
	mail_file = fopen(MAIL_FILE, "r+b");
 
	if (filepos % BLOCK_SIZE)
	{
		vlog("Mail system -- fatal error #3!!!");
		no_mail = 1;
		return;
	}
 
	fseek(mail_file, filepos, 0);
	fread(buf, size, 1, mail_file);
	fclose(mail_file);
	return;
}
 

void index_mail(char *raw_name_to_index, long pos)
{
	mail_index_type     *new_index;
	position_list_type  *new_position;
	char		    name_to_index[100]; /* I'm paranoid.  so sue me. */
	char		    *src;
	int		    i;

	if (!raw_name_to_index || !*raw_name_to_index) {
		vlog("Mail system -- non-fatal error #4.");
		return;
	}

	for (src = raw_name_to_index, i = 0; *src; )
              if (isupper(*src))
		name_to_index[i++] = tolower(*src++);
                 else name_to_index[i++] = *src++;
	name_to_index[i] = 0;

	if (!(new_index = find_char_in_index(name_to_index)))
	{
		/* name not already in index.. add it */
		new_index = (mail_index_type *)malloc(sizeof(mail_index_type));
		strncpy(new_index->recipient, name_to_index, NAME_SIZE);
		new_index->recipient[strlen(name_to_index)] = '\0';
		new_index->list_start = 0;
 
		/* add to front of list */
		new_index->next = mail_index;
		mail_index = new_index;
 	}
 
	/* now, add this position to front of position list */
	new_position = (position_list_type *)malloc(sizeof(position_list_type));
	new_position->position = pos;
	new_position->next = new_index->list_start;
	new_index->list_start = new_position;
}


/* SCAN_FILE */
/* scan_file is called once during boot-up.  It scans through the mail file
   and indexes all entries currently in the mail file. */
int scan_file(void)
{
	FILE 		   *mail_file;
	header_block_type  next_block;
	int		   total_messages = 0, block_num = 0;
	char		   buf[100];

	if (!(mail_file = fopen(MAIL_FILE, "r")))
	{
		vlog("Mail file non-existant... creating new file.");
		mail_file = fopen(MAIL_FILE, "w");
		fclose(mail_file);
		return 1;
	}
 
	while (fread(&next_block, sizeof(header_block_type), 1, mail_file))
	{
		if(next_block.block_type == HEADER_BLOCK)
		{
			index_mail(next_block.to, block_num * BLOCK_SIZE);
			total_messages++;
		}
		else
		if(next_block.block_type == DELETED_BLOCK)
			push_free_list(block_num * BLOCK_SIZE);
		block_num++;
	}
 
	file_end_pos = ftell(mail_file);
	sprintf(buf, "   %ld bytes read.", file_end_pos);
	vlog(buf);
	if (file_end_pos % BLOCK_SIZE)
	{
		vlog("Error booting mail system -- Mail file corrupt!");
		vlog("Mail disabled!");
		return 0;
	}
	sprintf(buf, "   Mail file read -- %d messages.", total_messages);
	vlog(buf);
	return 1;
} /* end of scan_file */


/* HAS_MAIL */
/* a simple little function which tells you if the guy has mail or not */
int has_mail(char *recipient)
{
	if (find_char_in_index(recipient))
		return 1;
	return 0;
}
 
/* STORE_MAIL  */
/* call store_mail to store mail.  (hard, huh? :-) )  Pass 3 pointers..
   who the mail is to (name), who it's from (name), and a pointer to the
   actual message text.			*/
 
void store_mail(char *to, char *from, char *message_pointer)
{
	header_block_type  	header;
	data_block_type		data;
	long			last_address, target_address;
	char			*msg_txt = message_pointer;
	char			*tmp;
	int			bytes_written = 0;
	int			total_length = strlen(message_pointer);
 
	assert(sizeof(header_block_type) == sizeof(data_block_type));
	assert(sizeof(header_block_type) == BLOCK_SIZE);

	if (!*from || !*to || !*message_pointer) {
		vlog("Mail system -- non-fatal error #5.");
		return;
	}
	memset(&header, 0, sizeof(header)); /* clear the record */
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

	target_address = pop_free_list();  /* find next free block */
	index_mail(to, target_address);	   /* add it to mail index in memory */
	write_to_file(&header, BLOCK_SIZE, target_address);
 
	if (strlen(msg_txt) <= HEADER_BLOCK_DATASIZE)
		return; /* that was the whole message */
 
	bytes_written = HEADER_BLOCK_DATASIZE;
	msg_txt += HEADER_BLOCK_DATASIZE; /* move pointer to next bit of text */
 
	/* find the next block address, then rewrite the header
	   to reflect where the next block is.	*/
	last_address = target_address;
	target_address = pop_free_list();
	header.next_block = target_address;
	write_to_file(&header, BLOCK_SIZE, last_address);
 
	/* now write the current data block */
	memset(&data, 0, sizeof(data)); /* clear the record */
	data.block_type = LAST_BLOCK;
	strncpy(data.txt, msg_txt, DATA_BLOCK_DATASIZE);
	data.txt[DATA_BLOCK_DATASIZE] = '\0';
	write_to_file(&data, BLOCK_SIZE, target_address);
	bytes_written += strlen(data.txt);
	msg_txt += strlen(data.txt);
 
	/* if, after 1 header block and 1 data block there is STILL
	   part of the message left to write to the file, keep writing
	   the new data blocks and rewriting the old data blocks to reflect
	   where the next block is.  Yes, this is kind of a hack, but if
	   the block size is big enough it won't matter anyway.  Hopefully,
	   MUD players won't pour their life stories out into the Mud Mail
	   System anyway.
 
	   Note that the block_type data field in data blocks is either
	   a number >=0, meaning a link to the next block, or LAST_BLOCK
	   flag (-2) meaning the last block in the current message.  This
	   works much like DOS' FAT.
	*/
 
	while(bytes_written < total_length)
	{
		last_address = target_address;
		target_address = pop_free_list();
 
		/* rewrite the previous block to link it to the next */
		data.block_type = target_address;
		write_to_file(&data, BLOCK_SIZE, last_address);
 
		/* now write the next block, assuming it's the last.  */
		data.block_type = LAST_BLOCK;
		strncpy(data.txt, msg_txt, DATA_BLOCK_DATASIZE);
		data.txt[DATA_BLOCK_DATASIZE] = '\0';
		write_to_file(&data, BLOCK_SIZE, target_address);
 
		bytes_written += strlen(data.txt);
		msg_txt += strlen(data.txt);
	}
} /* store mail */
 

/* READ_DELETE */
/* read_delete takes 1 char pointer to the name of the person whose mail
you're retrieving.  It returns to you a char pointer to the message text.
The mail is then discarded from the file and the mail index. */
 
char *read_delete(char *recipient, char *recipient_formatted)
/* recipient is the name as it appears in the index.
   recipient_formatted is the name as it should appear on the mail
   header (i.e. the text handed to the player) */
{
	header_block_type	header;
	data_block_type		data;
	mail_index_type		*mail_pointer, *prev_mail;
	position_list_type	*position_pointer;
	long			mail_address, following_block;
	char			*message, *tmstr, buf[200];
	size_t			string_size;

	if (!*recipient || !*recipient_formatted) {
		vlog("Mail system -- non-fatal error #6.");
		return 0;
	}
	if (!(mail_pointer = find_char_in_index(recipient))) {
		vlog("Stupid post-office-spec_proc-error");
		return 0;
	}
	if (!(position_pointer = mail_pointer->list_start)) {
		vlog("Stupid Rasmussen error!");
		return 0;
	}
 
	if (!(position_pointer->next)) /* just 1 entry in list. */
	{
		mail_address = position_pointer->position;
		free(position_pointer);
 
		/* now free up the actual name entry */
		if(mail_index == mail_pointer) { /* name is 1st in list */
			mail_index = mail_pointer->next;
			free(mail_pointer);
		}
		else {
			/* find entry before the one we're going to del */
			for(prev_mail = mail_index;
			    prev_mail->next != mail_pointer;
			    prev_mail = prev_mail->next);
			prev_mail->next = mail_pointer->next;
			free(mail_pointer);
		}
	}
	else {
		/* move to next-to-last record */
		while(position_pointer->next->next)
			position_pointer = position_pointer->next;
		mail_address = position_pointer->next->position;
		free(position_pointer->next);
		position_pointer->next = 0;
	}
 
	/* ok, now lets do some readin'! */
	read_from_file(&header, BLOCK_SIZE, mail_address);
 
	if (header.block_type != HEADER_BLOCK) {
		vlog("Oh dear.");
		no_mail = 1;
		vlog("Mail system disabled!");
		return 0;
	}

	tmstr = asctime(localtime(&header.mail_time));
	*(tmstr + strlen(tmstr) - 1) = '\0';
 
	sprintf(buf, " * * * * SneezyMUD Mail System * * * *\n\r"
		     "Date: %s\n\r"
		     "  To: %s\n\r"
		     "From: %s\n\r\n\r",
		     tmstr,
		     recipient_formatted,
		     header.from);

	string_size = (CHAR_SIZE * (strlen(buf) + strlen(header.txt) + 1));
	message = (char *)malloc(string_size);
	strcpy(message, buf);
	message[strlen(buf)] = '\0';
	strcat(message, header.txt);
	message[string_size - 1] = '\0';
	following_block = header.next_block;
 
	/* mark the block as deleted */
	header.block_type = DELETED_BLOCK;
	write_to_file(&header, BLOCK_SIZE, mail_address);
	push_free_list(mail_address);

	while(following_block != LAST_BLOCK)
	{
		read_from_file(&data, BLOCK_SIZE, following_block);

		string_size = (CHAR_SIZE * (strlen(message) + strlen(data.txt) + 1));
		message = (char *)realloc(message, string_size);
		strcat(message, data.txt);
		message[string_size - 1] = '\0';
		mail_address = following_block;
		following_block = data.block_type;
		data.block_type = DELETED_BLOCK;
		write_to_file(&data, BLOCK_SIZE, mail_address);
		push_free_list(mail_address);
	}
 
	return message;
}


/**************************************************************
** Below is the spec_proc for a postmaster using the above    **
** routines.  Written by Rasmussen (jelson@server.cs.jhu.edu) **
**************************************************************/

int postmaster(struct char_data *ch, int cmd, char *arg)
{
	if (!ch->desc)
		return 0; /* so mobs don't get caught here */

	switch(cmd)
	{
		case 303: /* mail */
			postmaster_send_mail(ch, cmd, arg);
			return 1;
			break;
		case 304: /* check */
			postmaster_check_mail(ch, cmd, arg);
			return 1;
			break;
		case 305: /* receive */
			postmaster_receive_mail(ch, cmd, arg);
			return 1;
			break;
		default:
			return 0;
			break;
	}
}

int mail_ok(struct char_data *ch)
{
	if (no_mail)
	{
		send_to_char("Sorry, the mail system is having technical difficulties.\n\r",
			     ch);
		return 0;
	}

	return 1;
}
		


void postmaster_send_mail(struct char_data *ch, int cmd, char *arg)
{
	struct char_data *mailman;
	char buf[200], recipient[100], *tmp;

        mailman = FindMobInRoomWithFunction(ch->in_room, postmaster);

        if (!mailman) return;

	if (GetMaxLevel(ch) < MIN_MAIL_LEVEL)
	{
		sprintf(buf, "$n tells you, 'Sorry, you have to be level %d to send mail!'",
			MIN_MAIL_LEVEL);
		act(buf, FALSE, mailman, 0, ch, TO_VICT);
		return;
	}

	if (!*arg) { /* you'll get no argument from me! */
		act("$n tells you, 'You need to specify an addressee!'",
		    FALSE, mailman, 0, ch, TO_VICT);
		return;
	}

	if (GET_GOLD(ch) < STAMP_PRICE) {
		sprintf(buf, "$n tells you, 'A stamp costs %d coins.'\n\r"
			     "$n tells you, '...which I see you can't afford.'",
			STAMP_PRICE);
		act(buf, FALSE, mailman, 0, ch, TO_VICT);
		return;
	}

	_parse_name(arg, recipient);

	if (find_name(recipient) < 0) {
		act("$n tells you, 'No one by that name is registered here!'",
		    FALSE, mailman, 0, ch, TO_VICT);
		return;
	}

	for (tmp = recipient; *tmp; tmp++)
           if (isupper(*tmp))
		*tmp = tolower(*tmp);

	act("$n starts to write some mail.",TRUE,ch,0,0,TO_ROOM);
	sprintf(buf, "$n tells you, 'I'll take %d coins for the stamp.'\n\r"
		     "$n tells you, 'Write your message, use @ when done.'",
		STAMP_PRICE);
	act(buf, FALSE, mailman, 0, ch, TO_VICT);
	GET_GOLD(ch) -= STAMP_PRICE;
        SET_BIT(ch->specials.act, PLR_MAILING);

	strcpy(ch->desc->name, recipient);
	ch->desc->str = (char **) malloc(sizeof(char *));
	*(ch->desc->str) = 0;
	ch->desc->max_str = MAX_MAIL_SIZE; 
}


void postmaster_check_mail(struct char_data *ch, int cmd, char *arg)
{
	struct char_data *mailman;
	char buf[200], recipient[100], *tmp;

        mailman = FindMobInRoomWithFunction(ch->in_room, postmaster);
   
        if (!mailman) return;

	_parse_name(GET_NAME(ch), recipient);

	for (tmp = recipient; *tmp; tmp++)
	    if (isupper(*tmp))
		*tmp = tolower(*tmp);

	if (has_mail(recipient))
		sprintf(buf, "$n tells you, 'You have mail waiting.'");
	else
		sprintf(buf, "$n tells you, 'Sorry, you don't have any mail waiting.'");
	act(buf, FALSE, mailman, 0, ch, TO_VICT);
}


void postmaster_receive_mail(struct char_data *ch, int cmd, char *arg)
{
	struct char_data *mailman;
	char buf[200], recipient[100], *tmp;
	struct obj_data *tmp_obj;

        mailman = FindMobInRoomWithFunction(ch->in_room, postmaster);
   
        if (!mailman) return;

	_parse_name(GET_NAME(ch), recipient);

	for (tmp = recipient; *tmp; tmp++)
	    if (isupper(*tmp))
		*tmp = tolower(*tmp);

	if (!has_mail(recipient)) {
		sprintf(buf, "$n tells you, 'Sorry, you don't have any mail waiting.'");
		act(buf, FALSE, mailman, 0, ch, TO_VICT);
		return;
	}

	while(has_mail(recipient)) {
		CREATE(tmp_obj, struct obj_data, 1);
		clear_object(tmp_obj);

		tmp_obj->name = strdup("mail paper letter");
		tmp_obj->short_description = strdup("a piece of mail");
		tmp_obj->description = strdup("Someone has left a piece of mail here.");

		tmp_obj->obj_flags.type_flag = ITEM_NOTE;
                tmp_obj->obj_flags.decay_time = -1;
		tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
		tmp_obj->obj_flags.weight = 1;
		tmp_obj->obj_flags.cost = 30;
		tmp_obj->obj_flags.cost_per_day = 10;
		tmp_obj->action_description = read_delete(recipient, GET_NAME(ch));

		if (!tmp_obj->action_description)
			tmp_obj->action_description = strdup("Mail system buggy, please report!!  Error #8.\n\r");

		tmp_obj->next = object_list;
		object_list = tmp_obj;

		obj_to_char(tmp_obj,ch);

		tmp_obj->item_number = -1;

		act("$n gives you a piece of mail.", FALSE, mailman, 0, ch, TO_VICT);
		act("$N gives $n a piece of mail.", FALSE, ch, 0, mailman, TO_ROOM);
	}
}

/* The end. */

