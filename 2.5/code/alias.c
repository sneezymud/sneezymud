#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

#define MAX_ALIASES			 20
#define MAX_IMMORTAL_ALIASES 40
#define MAX_LEN_ALIAS_RES	256

/* #define ALIAS_FILENAME "alias\\%s" */
#define ALIAS_FILENAME "alias/%s"

struct alias_data_type {
	struct alias_data_type *next;
	struct char_data *ch;
	struct alias_type *first;
	int number;
};

struct alias_type {
	struct alias_type *next;
	char name[10];
	char text[40];
};

struct alias_data_type *alias_base;
char alias_result[MAX_LEN_ALIAS_RES];


struct alias_data_type *get_alias_ch(struct char_data *ch);
void add_alias(struct char_data *ch, struct char_data *alias_ch,
			char *alias_name, char *alias_text);
void remove_alias(struct char_data *ch, struct char_data *alias_ch,
			char *alias_name);



void do_alias(struct char_data *ch, char *argument, int cmd)
{
	char alias_name[100], alias_text[100];
	char buf[80];
	struct alias_data_type *alias_data;
	struct alias_type *alias_entry;
	struct char_data *alias_ch;

	if (IS_NPC(ch))
		if (ch->desc->original)
			alias_ch = ch->desc->original;
		else
			return;
	else
		alias_ch = ch;

	half_chop(argument, alias_name, alias_text);

	if (!*alias_name) {
		alias_data = get_alias_ch(alias_ch);
		if (!(alias_entry = alias_data->first)) {
			send_to_char("You currently have no aliases.\n\r",ch);
			return;
		}
		send_to_char("Your current aliases:\n\r\n\r",ch);
		send_to_char("Alias:      Substitution\n\r",ch);
                send_to_char("________________________\n\r",ch);
		for(;alias_entry;alias_entry = alias_entry->next) {
			sprintf(buf, "%-11.10s %s\n\r",
					alias_entry->name, alias_entry->text);
			send_to_char(buf, ch);
		}
		return;
	}

	if (!*alias_text)
		remove_alias(ch, alias_ch, alias_name);
	else
		add_alias(ch, alias_ch, alias_name, alias_text);

}



struct alias_data_type *init_alias(struct char_data *alias_ch)
{
	struct alias_data_type *alias_data;
	struct alias_data_type *k, *l;

	CREATE(alias_data, struct alias_data_type, 1);

	alias_data->ch = alias_ch;
	alias_data->next = 0;
	alias_data->first = 0;
	alias_data->number = 0;

	if (!alias_base)
		return(alias_base = alias_data);
	else {
		for(k = alias_base ; k ; l = k, k = k->next);	/* find last */
		return(l->next = alias_data);
	}
}


int fgetline(FILE *file, char *buf)
{
	int next;

	while ((next = getc(file)) != EOF)
	{
		if (next == '\n') {
			*buf = 0;
			return(0);
		}
		if (next != '\r')
			*buf++ = next;
	}
	*buf = 0;
	return(EOF);
}

int fputline(FILE *file, char *buf)
{
	int next;

	while (next = *buf++)
		if (putc(next, file) == EOF)
			return(EOF);

	putc('\n', file);
	putc('\r', file);
	return(0);
}



struct alias_data_type *load_aliases(struct char_data *alias_ch)
{
	struct alias_data_type *alias_data;
	char buf[200];
	FILE *alias_file;
	char alias_name[10], alias_text[50];
	int num = 0;

	alias_data = init_alias(alias_ch);

	sprintf(buf, ALIAS_FILENAME, alias_ch->player.name);
	if (!(alias_file = fopen(buf, "r")))
	{
		return(alias_data);
			/* Can't open alias file so must not be any aliases */
	}
	while (!feof(alias_file))
	{
		if ((fgetline(alias_file, alias_name) != EOF) && *alias_name)
			if ((fgetline(alias_file, alias_text) != EOF) && *alias_name)
			{
				add_alias(0, alias_ch, alias_name, alias_text);
				num++;
			}
	}
	fclose(alias_file);
	return(alias_data);
}



void save_aliases(char *alias_ch_name, struct alias_data_type *alias_data)
{
	struct alias_type *alias_entry;
	FILE *alias_file;
	char buf[100];

	sprintf(buf, ALIAS_FILENAME, alias_ch_name);
	if (!(alias_file = fopen(buf, "w")))
	{
		log("Unable to create new alias file");
		return;
	}

	for(alias_entry = alias_data->first; alias_entry;
									alias_entry = alias_entry->next)
	{
		fputline(alias_file, alias_entry->name);
		fputline(alias_file, alias_entry->text);
	}
	fclose(alias_file);
}



struct alias_data_type *get_alias_ch(struct char_data *alias_ch)
{
	struct alias_data_type *alias_data;

	for (alias_data = alias_base ;
			(alias_data && (alias_data->ch != alias_ch));
			alias_data = alias_data->next);

	if (!alias_data)
		alias_data = load_aliases(alias_ch);

	return(alias_data);
}



void add_alias(struct char_data *ch, struct char_data *alias_ch,
			char *alias_name, char *alias_text)
{
	struct alias_data_type *alias_data;
	struct alias_type *new_alias, *k, *l, *alias_entry;
	char buf[100];

	if ((strlen(alias_name) > 8) || (strlen(alias_text) > 40)) {
		if (ch)
			send_to_char("Alias name max 8 chars, alias text max 40 chars\n\r",ch);
		return;
	}

	if (!strcmp(alias_name, "alias") || !strncmp(alias_text, "alias", 5)) {
		if (ch)
			send_to_char("What are you trying to pull? Can't alias ALIAS!\n\r",ch);
		return;
	}

	alias_data = get_alias_ch(alias_ch);

	if (alias_data->number >= ((GetMaxLevel(alias_ch) < 51) ?
							MAX_ALIASES : MAX_IMMORTAL_ALIASES))
	{
		if (ch)
			send_to_char("You already have all the aliases you can!\n\r",ch);
		return;
	}

	if (alias_entry = alias_data->first)
		for(;alias_entry;alias_entry = alias_entry->next)
			if (!strcmp(alias_name, alias_entry->name)) {
				sprintf(buf,"You already have an alias %s (= %s).\n\r",
						alias_name, alias_entry->text);
				if (ch)
					send_to_char(buf, ch);
				return;
			}

	sprintf(buf,"Added alias %s = %s.\n\r",alias_name, alias_text);
	if (ch)
		send_to_char(buf, ch);

	CREATE(new_alias, struct alias_type, 1);
	strcpy(new_alias->name, alias_name);
	strcpy(new_alias->text, alias_text);
	*(new_alias->text + strlen(alias_text)) = 0;
	alias_data->number++;

	new_alias->next = 0;

	if (!alias_data->first)
		alias_data->first = new_alias;
	else {
		for (k = alias_data->first ; k ; l = k, k = k->next);
		l->next = new_alias;
	}
	if (ch)
		save_aliases(GET_NAME(alias_ch), alias_data);
}



void remove_alias(struct char_data *ch, struct char_data *alias_ch,
					 char *alias_name)
{
	struct alias_data_type *alias_data;
	struct alias_type *alias_entry, *k;
	char buf[80];


	alias_data = get_alias_ch(alias_ch);

	if (!(alias_entry = alias_data->first)) {
		send_to_char("You don't have any aliases!\n\r",ch);
		return;
	}
	for(k = 0 ;alias_entry;
			k = alias_entry, alias_entry = alias_entry->next)
		if(!strcmp(alias_entry->name, alias_name)) {
			sprintf(buf, "Alias %s removed.\n\r",alias_name);
			send_to_char(buf,ch);
			if (k)
				k->next = alias_entry->next;
			else
				alias_data->first = alias_entry->next;
			alias_data->number--;
			free(alias_entry);
			save_aliases(GET_NAME(ch), alias_data);
			return;
		}
	sprintf(buf, "You don't have an alias %s.\n\r",alias_name);
	send_to_char(buf, ch);
}


/* wipe all aliases for a character when that character is lost */

void delete_aliases(struct char_data *ch)
{
	struct alias_data_type *alias_data, *x;
	struct alias_type *alias_entry, *k;

	for (alias_data = alias_base, x = 0 ;
			(alias_data && (alias_data->ch != ch));
			x = alias_data, alias_data = alias_data->next);
	if (!alias_data)
		return;
	for(alias_entry = alias_data->first; alias_entry;) {
		k = alias_entry->next;
		free(alias_entry);
		alias_entry = k;
	}
	if (x)
		x->next = alias_data->next;
	else
		alias_base = alias_data->next;
	free(alias_data);
}


/* perform the actual alias parsing */
char *substitute_aliases(struct char_data *ch, char *argument)
{
	char *pos, *res, *text;
	int len, total = MAX_LEN_ALIAS_RES - 1;
	struct alias_data_type *alias_data;
	struct alias_type *alias_ch_base, *alias_entry;
	char word_store[100];
	char *word;
	int alias_flag = 0;

	if (IS_NPC(ch))
		if (!(ch = ch->desc->original))
			return(argument);

	alias_data = get_alias_ch(ch);
	if (!(alias_ch_base = alias_data->first))
		return(argument);				/* no aliases */

	res = alias_result;
	for(pos = argument; *pos ;) {
		for (len = 0, word = word_store ;
			((*word = *(pos + len)) != ' ') && (*(pos + len)) ; len++,word++);
		*word++ = 0;
		if (len) {
			if (!alias_flag)
				if (!strcmp(word_store, "alias"))
					return(argument);	/* to prevent confusing removes */
			alias_flag = 1;
			for(alias_entry = alias_ch_base;
					alias_entry && strcmp(word_store, alias_entry->name);
					alias_entry = alias_entry->next);
			if (alias_entry) {
				text = alias_entry->text;
				pos += len;
				for(;*res = *text;) {
					res++;
					text++;
					if (!(total--)) {
						send_to_char("Line + aliases too long.\n\r",ch);
						*res = 0;
						return(alias_result);
					}
				}
			}
			else {
				for(;len; len--) {
					*res++ = *pos++;
					if(!(total--)) {
						send_to_char("Line + aliases too long.\n\r",ch);
						*res = 0;
						return(alias_result);
					}
				}
			}
		}
		else {
			*res++ = *pos++;
			if (!(total--)) {
				send_to_char("Line + aliases too long.\n\r",ch);
				*res = 0;
				return(alias_result);
			}
		}
	}
	*res = 0;
	return(alias_result);
}


