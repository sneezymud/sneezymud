/*
 *  file: Interpreter.c , Command interpreter module.      Part of DIKUMUD *
 *  Usage: Procedures interpreting user command                            *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <dirent.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <sys/syscall.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>
#include <unistd.h>
#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "limits.h"
#include "mail.h"
#include "race.h"
#include "spells.h"


#define COMMANDO(number,min_pos,pointer,min_level) {      \
							    cmd_info[(number)].command_pointer = (pointer);         \
							      cmd_info[(number)].minimum_position = (min_pos);        \
								cmd_info[(number)].minimum_level = (min_level); }

#define NOT !
#define AND &&
#define OR ||

#define STATE(d) ((d)->connected)
#define MAX_CMD_LIST 400

extern char *crypt(const char *, const char *);
extern const struct title_type titles[8][ABS_MAX_LVL];
extern char motd[MAX_STRING_LENGTH];
extern struct char_data *character_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern char ansi[MAX_STRING_LENGTH];
extern struct index_data *mob_index;
extern struct index_data *obj_index;
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct char_data *board_kludge_char;
struct command_info cmd_info[MAX_CMD_LIST];

char echo_on[]  = {IAC, WONT, TELOPT_ECHO, '\r', '\n', '\0'};
char echo_off[] = {IAC, WILL, TELOPT_ECHO, '\0'};
int WizLock;
int Silence = 0;
int plr_tick_count=0;



/* external fcntls */

void set_title(struct char_data *ch);
void init_char(struct char_data *ch);
void store_to_char(struct char_file_u *st, struct char_data *ch);
int create_entry(char *name);
/* int special(struct char_data *ch, int cmd, char *arg);
 */
void log(char *str);

void do_move(struct char_data *ch, char *argument, int cmd);
void do_look(struct char_data *ch, char *argument, int cmd);
void do_read(struct char_data *ch, char *argument, int cmd);
void do_say(struct char_data *ch, char *argument, int cmd);
void do_exit(struct char_data *ch, char *argument, int cmd);
void do_snoop(struct char_data *ch, char *argument, int cmd);
void do_insult(struct char_data *ch, char *argument, int cmd);
void do_quit(struct char_data *ch, char *argument, int cmd);
void do_qui(struct char_data *ch, char *argument, int cmd);
void do_help(struct char_data *ch, char *argument, int cmd);
void do_who(struct char_data *ch, char *argument, int cmd);
void do_whozone(struct char_data *ch, char *argument, int cmd);
void do_emote(struct char_data *ch, char *argument, int cmd);
void do_echo(struct char_data *ch, char *argument, int cmd);
void do_trans(struct char_data *ch, char *argument, int cmd);
void do_kill(struct char_data *ch, char *argument, int cmd);
void do_stand(struct char_data *ch, char *argument, int cmd);
void do_sit(struct char_data *ch, char *argument, int cmd);
void do_rest(struct char_data *ch, char *argument, int cmd);
void do_sleep(struct char_data *ch, char *argument, int cmd);
void do_wake(struct char_data *ch, char *argument, int cmd);
void do_force(struct char_data *ch, char *argument, int cmd);
void do_get(struct char_data *ch, char *argument, int cmd);
void do_drop(struct char_data *ch, char *argument, int cmd);
void do_news(struct char_data *ch, char *argument, int cmd);
void do_wiznews(struct char_data *ch, char *argument, int cmd);  /* SG */
void do_atlas(struct char_data *ch, char *argument, int cmd);
void do_monitor(struct char_data *ch, char *argument, int cmd);
void do_score(struct char_data *ch, char *argument, int cmd);
void do_loglist(struct char_data *ch, char *argument, int cmd);
void do_checklog(struct char_data *ch, char *argument, int cmd);
void do_deathcheck(struct char_data *ch, char *argument, int cmd);
void do_inventory(struct char_data *ch, char *argument, int cmd);
void do_equipment(struct char_data *ch, char *argument, int cmd);
void do_shout(struct char_data *ch, char *argument, int cmd);
void do_not_here(struct char_data *ch, char *argument, int cmd);
void do_tell(struct char_data *ch, char *argument, int cmd);
void do_wear(struct char_data *ch, char *argument, int cmd);
void do_wield(struct char_data *ch, char *argument, int cmd);
void do_grab(struct char_data *ch, char *argument, int cmd);
void do_remove(struct char_data *ch, char *argument, int cmd);
void do_put(struct char_data *ch, char *argument, int cmd);
void do_shutdown(struct char_data *ch, char *argument, int cmd);
void do_save(struct char_data *ch, char *argument, int cmd);
void do_hit(struct char_data *ch, char *argument, int cmd);
void do_string(struct char_data *ch, char *arg, int cmd);
void do_give(struct char_data *ch, char *arg, int cmd);
void do_stat(struct char_data *ch, char *arg, int cmd);
void do_guard(struct char_data *ch, char *arg, int cmd);
void do_time(struct char_data *ch, char *arg, int cmd);
void do_weather(struct char_data *ch, char *arg, int cmd);
void do_load(struct char_data *ch, char *arg, int cmd);
void do_purge(struct char_data *ch, char *arg, int cmd);
void do_shutdow(struct char_data *ch, char *arg, int cmd);
void do_idea(struct char_data *ch, char *arg, int cmd);
void do_typo(struct char_data *ch, char *arg, int cmd);
void do_bug(struct char_data *ch, char *arg, int cmd);
void do_whisper(struct char_data *ch, char *arg, int cmd);
void do_cast(struct char_data *ch, char *arg, int cmd);
void do_at(struct char_data *ch, char *arg, int cmd);
void do_goto(struct char_data *ch, char *arg, int cmd);
void do_ask(struct char_data *ch, char *arg, int cmd);
void do_drink(struct char_data *ch, char *arg, int cmd);
void do_eat(struct char_data *ch, char *arg, int cmd);
void do_pour(struct char_data *ch, char *arg, int cmd);
void do_sip(struct char_data *ch, char *arg, int cmd);
void do_taste(struct char_data *ch, char *arg, int cmd);
void do_order(struct char_data *ch, char *arg, int cmd);
void do_follow(struct char_data *ch, char *arg, int cmd);
void do_rent(struct char_data *ch, char *arg, int cmd);
void do_bload(struct char_data *ch, char *arg, int cmd);
void do_advance(struct char_data *ch, char *arg, int cmd);
void do_close(struct char_data *ch, char *arg, int cmd);
void do_open(struct char_data *ch, char *arg, int cmd);
void do_lock(struct char_data *ch, char *arg, int cmd);
void do_unlock(struct char_data *ch, char *arg, int cmd);
void do_exits(struct char_data *ch, char *arg, int cmd);
void do_enter(struct char_data *ch, char *arg, int cmd);
void do_leave(struct char_data *ch, char *arg, int cmd);
void do_write(struct char_data *ch, char *arg, int cmd);
void do_flee(struct char_data *ch, char *arg, int cmd);
void do_sneak(struct char_data *ch, char *arg, int cmd);
void do_hide(struct char_data *ch, char *arg, int cmd);
void do_backstab(struct char_data *ch, char *arg, int cmd);
void do_pick(struct char_data *ch, char *arg, int cmd);
void do_steal(struct char_data *ch, char *arg, int cmd);
void do_bash(struct char_data *ch, char *arg, int cmd);
void do_rescue(struct char_data *ch, char *arg, int cmd);
void do_kick(struct char_data *ch, char *arg, int cmd);
void do_examine(struct char_data *ch, char *arg, int cmd);
void do_info(struct char_data *ch, char *arg, int cmd);
void do_users(struct char_data *ch, char *arg, int cmd);
void do_where(struct char_data *ch, char *arg, int cmd);
void do_levels(struct char_data *ch, char *arg, int cmd);
void do_reroll(struct char_data *ch, char *arg, int cmd);
void do_pray(struct char_data *ch, char *arg, int cmd);
void do_brief(struct char_data *ch, char *arg, int cmd);
void do_cls(struct char_data *ch, char *arg, int cmd);
void do_bamfin(struct char_data *ch, char *arg, int cmd);
void do_bamfout(struct char_data *ch, char *arg, int cmd);
void do_terminal(struct char_data *ch, char *arg, int cmd);
void do_prompt(struct char_data *ch, char *arg, int cmd);
void do_glance(struct char_data *ch, char *arg, int cmd);
void do_wizlist(struct char_data *ch, char *arg, int cmd);
void do_consider(struct char_data *ch, char *arg, int cmd);
void do_group(struct char_data *ch, char *arg, int cmd);
void do_restore(struct char_data *ch, char *arg, int cmd);
void do_return(struct char_data *ch, char *argument, int cmd);
void do_switch(struct char_data *ch, char *argument, int cmd);
void do_quaff(struct char_data *ch, char *argument, int cmd);
void do_recite(struct char_data *ch, char *argument, int cmd);
void do_use(struct char_data *ch, char *argument, int cmd);
void do_pose(struct char_data *ch, char *argument, int cmd);
void do_noshout(struct char_data *ch, char *argument, int cmd);
void do_plr_noshout(struct char_data *ch, char *argument, int cmd);
void do_wizhelp(struct char_data *ch, char *argument, int cmd);
void do_credits(struct char_data *ch, char *argument, int cmd);
void do_compact(struct char_data *ch, char *argument, int cmd);
void do_wimpy(struct char_data *ch, char *argument, int cmd);   /* jdb -8-16 */
void do_commune(struct char_data *ch, char *argument, int cmd); /* jdb - 9-1 */
void do_nohassle(struct char_data *ch, char *argument, int cmd);  /* jdb 9-6 */
void do_system(struct char_data *ch, char *argument, int cmd);  /* jdb 9-16 */
void do_pull(struct char_data *ch, char *argument, int cmd);  /* jdb 9-16 */
void do_stealth(struct char_data *ch, char *argument, int cmd); /* jdb 9-17 */
void do_edit(struct char_data *ch, char *arg, int cmd); /* jdb 9-29 */
void do_set(struct char_data *ch, char *arg, int cmd); /* jdb 9-29 */
void do_rsave(struct char_data *ch, char *arg, int cmd); /* jdb 10-5 */
void do_rload(struct char_data *ch, char *arg, int cmd); /* jdb 10-5 */
void do_wizlock(struct char_data *ch, char *arg, int cmd); /* jdb 10-15 */
void do_highfive(struct char_data *ch, char *arg, int cmd); /* jdb 10-30 */
void do_title(struct char_data *ch, char *arg, int cmd); /* jdb 11-3 */
void do_uptime(struct char_data *ch, char *arg, int cmd); /* jdb 12-3 */
void do_instazone(struct char_data *ch, char *arg, int cmd); /* jdb 12-3 */
void do_disarm(struct char_data *ch, char *arg, int cmd); /* jdb 12-3 */
void do_junk(struct char_data *ch, char *arg, int cmd); /* jdb 12-17 */
void do_gain(struct char_data *ch, char *arg, int cmd); /* jdb 1-19 */
void do_passwd(struct char_data *ch, char *arg, int cmd); /* jdb 2-6 */
void do_fill(struct char_data *ch, char *arg, int cmd); /* jdb 2-9 */
void do_imptest(struct char_data *ch, char *arg, int cmd); /* jdb 2-13 */
void do_silence(struct char_data *ch, char *arg, int cmd); /* smg 4-26 */
void do_teams(struct char_data *ch, char *arg, int cmd); /* smg 5-26 */
void do_auth(struct char_data *ch, char *arg, int cmd); /* jdb 3-1 */
void do_shoot(struct char_data *ch, char *arg, int cmd); /* jdb 3-8 */
void do_swim(struct char_data *ch, char *arg, int cmd); /* jdb 8-4 */
void do_reload(struct char_data *ch, char *arg, int cmd); /* jhh 7-24 */
void do_oset(struct char_data *ch, char *arg, int cmd); /* jfr2 10-15*/
void do_bet(struct char_data *ch, char *arg, int cmd); /* jhh 8-22 */
void do_stay(struct char_data *ch, char *arg, int cmd); /* jhh 8-22 */
void do_peek(struct char_data *ch, char *arg, int cmd); /* jhh 8-22 */
void do_color(struct char_data *ch, char *arg, int cmd); /*jfr 8-28 */
void do_search(struct char_data *ch, char *arg, int cmd); /* jfr2 1-16-93*/
void do_send(struct char_data *ch, char *arg, int cmd);  /* jfr2 12-30 */
void do_spy(struct char_data *ch, char *arg, int cmd);  /*jfr2 1-23-93*/
void do_sign(struct char_data *ch, char *arg, int cmd);
void do_play(struct char_data *ch, char *arg, int cmd); /*jfr2 2-11-93 */
void do_flag(struct char_data *ch, char *arg, int cmd); /*jfr2 1-30-93 */
void do_link(struct char_data *ch, char *arg, int cmd); /* jfr2 1-24-93 */
void do_doorbash(struct char_data *ch, char *arg, int cmd);
void do_springleap(struct char_data *ch, char *arg, int cmd); 
void do_lay_hands(struct char_data *ch, char *arg, int cmd);
void do_quivering_palm(struct char_data *ch, char *arg, int cmd); 
void do_feign_death( struct char_data *ch, char *arg, int cmd);
void do_first_aid( struct char_data *ch, char *arg, int cmd);
void do_channel(struct char_data *ch, char *arg, int cmd); /*jfr2 12-30 */
void do_headbutt(struct char_data *ch, char *arg, int cmd); /*jfr2 9-21 */
void do_log(struct char_data *ch, char *arg, int cmd);  /*jfr2 12/29/92 */
void do_subterfuge(struct char_data *ch, char *arg, int cmd); /* jfr2 10-3 */
void do_throw(struct char_data *ch, char *arg, int cmd); /* jfr2 10-4 */
void do_scribe(struct char_data *ch, char *arg, int cmd); /* jfr2 10-4 */
void do_brew(struct char_data *ch, char *arg, int cmd); /* jfr2 10-15*/
void do_grapple(struct char_data *ch, char *arg, int cmd); /*jfr2 10-16*/

/*
  depth first seach procedure donated by WhiteGold
  */

void do_track(struct char_data *ch, char *arg, int cmd); /* jdb 10-9 */

/*
  These 3 were donated by sequent
  */

void do_attribute(struct char_data *ch, char *arg, int cmd); /* jdb 11-6 */
void do_world(struct char_data *ch, char *arg, int cmd); /* jdb 11-6 */
void do_spells(struct char_data *ch, char *arg, int cmd); /* jdb 11-6 */


void do_action(struct char_data *ch, char *arg, int cmd);
void do_practice(struct char_data *ch, char *arg, int cmd);

/* Hammor commands */
void do_assist(struct char_data *ch, char *arg, int cmd);
void do_fire(struct char_data *ch, char *arg, int cmd);
void do_show(struct char_data *ch, char *arg, int cmd);
void do_bodyslam(struct char_data *ch, char *arg, int cmd);
void do_invis(struct char_data *ch, char *arg, int cmd);
void do_grouptell(struct char_data *ch, char *arg, int cmd);

/* Brutius commands */

void do_report(struct char_data *ch, char *arg, int cmd);
void do_demote(struct char_data *ch, char *arg, int cmd);
void do_split(struct char_data *ch, char *arg, int cmd);
void do_command(struct char_data *ch, char *arg, int cmd);
void do_deathstroke(struct char_data *ch, char *arg, int cmd);



char *command[]=
{ "north", 	     /* 1 */
    "east",
    "south",
    "west",
    "up",
    "down",
    "enter",
    "exits",
    "kiss",
    "get",
    "drink",	     /* 11 */
    "eat",
    "wear",
    "wield",
    "look",
    "score",
    "say",
    "shout",
    "tell",
    "inventory",
    "qui",	       /* 21 */
    "bounce",
    "smile",
    "dance",
    "kill",
    "cackle",
    "laugh",
    "giggle",
    "shake",
    "puke",
    "growl",  	   /* 31 */    
    "scream",
    "insult",
    "comfort",
    "nod",
    "sigh",
    "sulk",
    "help",
    "who",
    "emote",
    "echo",        /* 41 */
    "stand",
    "sit",
    "rest",
    "sleep",
    "wake",
    "force",
    "transfer",
    "hug",
    "snuggle",
    "cuddle",	     /* 51 */
    "nuzzle",
    "cry",
    "news",
    "equipment",
    "buy",
    "sell",
    "value",
    "list",
    "drop",
    "goto",	       /* 61 */
    "weather",
    "read",
    "pour",
    "grab",
    "remove",
    "put",
    "shutdow",
    "save",
    "hit",
    "string",      /* 71 */
    "give",
    "quit",
    "stat",
    "guard",
    "time",
    "load",
    "purge",
    "shutdown",
    "idea",
    "typo",        /* 81 */
    "bug",
    "whisper",
    "cast",
    "at",
    "ask",
    "order",
    "sip",
    "taste",
    "snoop",
    "follow",      /* 91 */
    "rent",
    "bload",
    "poke",
    "advance",
    "accuse",
    "grin",
    "bow",
    "open",
    "close",
    "lock",        /* 101 */
    "unlock",
    "leave",
    "applaud",
    "blush",
    "burp",
    "chuckle",
    "clap",
    "cough",
    "curtsey",
    "fart",        /* 111 */
    "flip",
    "fondle",
    "frown",
    "gasp",
    "glare",
    "groan",
    "grope",
    "hiccup",
    "lick",
    "love",        /* 121 */
    "moan",
    "nibble",
    "pout",
    "purr",
    "ruffle",
    "shiver",
    "shrug",
    "sing",
    "slap",
    "smirk",       /* 131 */
    "snap",
    "sneeze",
    "snicker",
    "sniff",
    "snore",
    "spit",
    "squeeze",
    "stare",
    "strut",
    "thank",       /* 141 */
    "twiddle",
    "wave",
    "whistle",
    "wiggle",
    "wink",
    "yawn",
    "snowball",
    "write",
    "hold",
    "flee",        /* 151 */
    "sneak",
    "hide",
    "backstab",
    "pick",
    "steal",
    "bash",
    "rescue",
    "kick",
    "french",
    "comb",        /* 161 */
    "massage",
    "tickle",
    "practice",
    "pat",
    "examine",
    "take",
    "info",
    "'",
    "practise",
    "curse",       /* 171 */
    "use",
    "where",
    "levels",
    "reroll",
    "pray",
    ",",
    "beg",
    "bleed",
    "cringe",
    "daydream",    /* 181 */
    "fume",
    "grovel",
    "hop",
    "nudge",
    "peer",
    "point",
    "ponder",
    "punch",
    "snarl",
    "spank",       /* 191 */
    "steam",
    "tackle",
    "taunt",
    "wiznet",
    "whine",
    "worship",
    "yodel",
    "brief",
    "wizlist",
    "consider",    /* 201 */
    "group",
    "restore",
    "return",
    "switch",      /* 205 */
    "quaff",
    "recite",
    "users",
    "pose",
    "noshout",
    "wizhelp",   /* 211 */
    "credits",   /* 212 */
    "compact",   /* 213 */
    ":",           /* emote   (jdb - 7/31/91)     */
    "deafen",      /* plr_noshout  (jdb - 7/31)   */  /*215*/
    "slay",        /* instead of "kill" for immorts (8/16) */ /*216*/ 
    "wimpy",        /* 217 */
    "junk",      /* 218 */
    "deposit",      /* 219 */   /* 9 - 4ish */
    "withdraw",     /* 220 */
    "balance",      
    "nohassle",        /* 9 - 6 */
    "system",          /* 9 - 16 */  
    "pull",         
    "stealth",      /* 225 */
    "edit",         /* 226 */
    "@set",         /* 227 */
    "rsave",        /* 228 */
    "rload",        /* 229 */
    "track",        /* 230 */
    "wizlock",      /* 231 */
    "highfive",     /* 232 */
    "title",        /* 233 */
    "whozone",      /* 234 */
    "assist",       /* 235 */
    "attribute",    /* 236 */
    "world",        /* 237 */ 
    "allspells",    /* 238 */
    "fire",       /* 239 */
    "show",         /* 240 */
    "bodyslam",        /* 241 */
    "invisible",    /* 242 */
    "gain",         /* 243 */
    "instazone",    /* 244 */
    "disarm",       /* 245 */
    "think",	    /* 246 */
    "chpwd",        /* 247 */ /* 2-6-92 jdb */
    "fill",         /* 248 */ /* 2-9-92 jdb */
    "imptest",      /* 249 */ /* 2-13-92 jdb */
#if PLAYER_AUTH
    "auth",         /* 250 */ /* 3-1-92 jdb */
    "shoot",        /* 251 */ /* 3-8-92 jdb */
    "silence",      /* 252 */ /* 4-26-92 smg */
    "teams",      /* 253 */ /* 5-26-92 smg */
#else
    "shoot",        /* 250 */ /* 3-8-92 jdb */
    "silence",      /* 251 */ /* 4-26-92 smg */
    "teams",      /* 252 */ /* 5-26-92 smg */
    "gt",  /*253*/  /*jfr2 6/25/92*/
    "send",     /*254*/                     
    "log",    /*255*/                     
    "monitor",      /*256*/                     
    "channel",      /*257*/                    
    "report",    /*258*/
    "demote",   /*259*/
    "atlas",   /*260*/
    "bonk",    /*261*/
    "scold",   /*262*/
    "drool",   /*263*/
    "rip",      /*264*/
    "stretch",     /*265*/ 
    "split",     /*266*/ 
    "command",   /*267*/
    "deathstroke", /*268*/
    "pimp", 	/*269*/
    "reload",	/*270*/
    "belittle", /*271*/
    "piledrive", /*272*/
    "tap",	/*273*/
    "bet",  /*274*/
    "stay", /*275*/
    "peek", /*276*/
    "color",/*277*/  
    "headbutt",
    "subterfuge",
    "throw",
    "oset",
    "scribe",
    "brew",
    "grapple", /*284*/
    "flipoff",
    "moon",
    "pinch",
    "bite",
    "search", /*289*/
    "spy",
    "doorbash",
    "play",
    "flag",
    "quivering palm",
    "feign death",
    "springleap", 
    "first aid",
    "swim",
    "sign",
    "cutlink",
    "lay hands",
    "wiznews",				/* 302 - SG */
    "mail",
    "check",
    "receive",
    "cls",       /* 306 - jfr2 */
    "glance",    /* 307 - jfr2 */
    "terminal",
    "prompt",
    "bamfin",
    "bamfout",
    "board",
    "checklog", /* 313 - SG */
    "loglist",  /* 314 - SG */
    "deathcheck", /* SG */
    "whap", /* 316 Batopr */
        "beam", /* 317 Bat */
        "chortle", /* 318 Bat */


#endif    
    "\n"
    };


char *fill[]=
{ "in",
    "from",
    "with",
    "the",
    "on",
    "at",
    "to",
    "\n"
    };

int search_block(char *arg, char **list, bool exact)
{
  register int i,l;
  
  /* Make into lower case, and get length of string */
  for(l=0; *(arg+l); l++)
    *(arg+l)=LOWER(*(arg+l));
  
  if (exact) {
    for(i=0; **(list+i) != '\n'; i++)
      if (!strcmp(arg, *(list+i)))
	return(i);
  } else {
    if (!l)
      l=1; /* Avoid "" to match the first available string */
    for(i=0; **(list+i) != '\n'; i++)
      if (!strncmp(arg, *(list+i), l))
	return(i);
  }
  
  return(-1);
}


int old_search_block(char *argument,int begin,int length,char **list,int mode)
{
  int guess, found, search;
  
  
  /* If the word contain 0 letters, then a match is already found */
  found = (length < 1);
  
  guess = 0;
  
  /* Search for a match */
  
  if(mode)
    while ( NOT found AND *(list[guess]) != '\n' )
      {
	found=(length==strlen(list[guess]));
	for(search=0;( search < length AND found );search++)
	  found=(*(argument+begin+search)== *(list[guess]+search));
	guess++;
      } else {
	while ( NOT found AND *(list[guess]) != '\n' ) {
	  found=1;
	  for(search=0;( search < length AND found );search++)
	    found=(*(argument+begin+search)== *(list[guess]+search));
	  guess++;
	}
      }
  
  return ( found ? guess : -1 ); 
}

void command_interpreter(struct char_data *ch, char *argument) 
{
  int look_at, cmd, begin;
  char buf[200];
  extern int no_specials;	
  extern struct char_data *board_kludge_char;
  
  REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

  /*
   *  a bug check.
   */
  if (!IS_NPC(ch)) {
    int i, found=FALSE;
    if ((!ch->player.name[0]) || (ch->player.name[0]<' ')) {
	log("Error in character name.  Changed to 'Error'");
	free(ch->player.name);
	ch->player.name = (char *)malloc(6);
	strcpy(ch->player.name, "Error"); 
        SET_BIT(ch->specials.act, PLR_BANISHED);
        char_from_room(ch);
        char_to_room(ch, 2);
	return;
    }
    strcpy(buf, ch->player.name);
    for (i = 0; i< strlen(buf) && !found; i++) {
      if (buf[i]<65) {
	found = TRUE;
      }
    }
    if (found) {
	log("Error in character name.  Changed to 'Error'");
	free(ch->player.name);
	ch->player.name = (char *)malloc(6);
	strcpy(ch->player.name, "Error");
	return;
    }
  }
  
  if (ch == board_kludge_char) {
    /*
     **  what about trans/flee?  board_kludge_board (later)
     */
    board_save_board(FindBoardInRoom(ch->in_room));
    board_kludge_char = 0;
  }
  
  /* Find first non blank */
  for (begin = 0 ; (*(argument + begin ) == ' ' ) ; begin++ );

  if (*(argument + begin) == '\'') {
    look_at = begin+1;
  } else
  /* Find length of first word */
  for (look_at = 0; *(argument + begin + look_at ) > ' ' ; look_at++) {
    /* Make all letters lower case AND find length */
       *(argument + begin + look_at) = LOWER(*(argument + begin + look_at));
  }

  cmd = old_search_block(argument,begin,look_at,command,0);
  
  if (!cmd)
    return;
  
  if ( cmd>0 && GetMaxLevel(ch)<cmd_info[cmd].minimum_level )	{
    send_to_char("Excuse me?\n\r", ch);
    return;
  }
  
  if ( cmd>0 && (cmd_info[cmd].command_pointer != 0))	{
   if (!IS_AFFECTED(ch, AFF_BREWING)) {
    if ((!IS_AFFECTED(ch, AFF_PARALYSIS)) || (cmd_info[cmd].minimum_position <= POSITION_STUNNED)) {
      if( GET_POS(ch) < cmd_info[cmd].minimum_position ) {
	switch(GET_POS(ch))
	  {
	  case POSITION_DEAD:
	    send_to_char("Lie still; you are DEAD!!! :-( \n\r", ch);
	    break;
	  case POSITION_INCAP:
	  case POSITION_MORTALLYW:
	    send_to_char(
			 "You are in a pretty bad shape, unable to do anything!\n\r",
			 ch);
	    break;
	    
	  case POSITION_STUNNED:
	    send_to_char(
			 "All you can do right now, is think about the stars!\n\r", ch);
	    break;
	  case POSITION_SLEEPING:
	    send_to_char("In your dreams, or what?\n\r", ch);
	    break;
	  case POSITION_RESTING:
	    send_to_char("Nah... You feel too relaxed to do that..\n\r",
			 ch);
	    break;
	  case POSITION_SITTING:
	    send_to_char("Maybe you should get on your feet first?\n\r",ch);
	    break;
	  case POSITION_FIGHTING:
	    send_to_char("No way! You are fighting for your life!\n\r", ch);
	    break;
	  }
      } else {
	
	if (!no_specials && special(ch, cmd, argument + begin + look_at))
	  return;  
	
        if ((GetMaxLevel(ch)>=LOW_IMMORTAL) &&
            (cmd > 6) &&
            (GetMaxLevel(ch)<60)) {
          sprintf(buf,"%s:%s",ch->player.name,argument);
          slog(buf);
        }

        if (IS_PC(ch) && IS_SET(ch->specials.act, PLR_LOGGED)) {
           sprintf(buf, "%s %s",ch->player.name,argument);
           slog(buf);
        }

	((*cmd_info[cmd].command_pointer)
	 (ch, argument + begin + look_at, cmd));
      }
      return;
    } else {
      send_to_char("You are paralyzed, you can't do much of anything!\n\r",ch);
      return;
    }
   } else {
     send_to_char("You are brewing, you MUST concentrate on your potion.\n\r", ch);
    return;
   }               
  }
  if ( cmd>0 && (cmd_info[cmd].command_pointer == 0))
    send_to_char(
		 "Sorry, but that command has yet to be implemented...\n\r",
		 ch);
  else 
    send_to_char("Pardon? \n\r", ch);
}

void argument_interpreter(char *argument,char *first_arg,char *second_arg )
{
  int look_at, found, begin;
  
  found = begin = 0;
  
  do
    {
      /* Find first non blank */
      for ( ;*(argument + begin ) == ' ' ; begin++);
      
      /* Find length of first word */
      for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)
	
	/* Make all letters lower case,
	   AND copy them to first_arg */
	*(first_arg + look_at) =
	  LOWER(*(argument + begin + look_at));
      
      *(first_arg + look_at)='\0';
      begin += look_at;
      
    }
  while( fill_word(first_arg));
  
  do
    {
      /* Find first non blank */
      for ( ;*(argument + begin ) == ' ' ; begin++);
      
      /* Find length of first word */
      for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)
	
	/* Make all letters lower case,
	   AND copy them to second_arg */
	*(second_arg + look_at) =
	  LOWER(*(argument + begin + look_at));
      
      *(second_arg + look_at)='\0';
      begin += look_at;
      
    }
  while( fill_word(second_arg));
}

int is_number(char *str)
{
  int look_at;
  
  if(*str=='\0')
    return(0);
  
  for(look_at=0;*(str+look_at) != '\0';look_at++)
    if((*(str+look_at)<'0')||(*(str+look_at)>'9'))
      return(0);
  return(1);
}

/*  Quinn substituted a new one-arg for the old one.. I thought returning a 
    char pointer would be neat, and avoiding the func-calls would save a
    little time... If anyone feels pissed, I'm sorry.. Anyhow, the code is
    snatched from the old one, so it outta work..
    
    void one_argument(char *argument,char *first_arg )
    {
    static char dummy[MAX_STRING_LENGTH];
    
    argument_interpreter(argument,first_arg,dummy);
    }
    
    */


/* find the first sub-argument of a string, return pointer to first char in
   primary argument, following the sub-arg			            */
char *one_argument(char *argument, char *first_arg )
{
  int found, begin, look_at;
  
  found = begin = 0;
  
  do
    {
      /* Find first non blank */
      for ( ;isspace(*(argument + begin)); begin++);
      
      /* Find length of first word */
      for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)
	
	/* Make all letters lower case,
	   AND copy them to first_arg */
	*(first_arg + look_at) =
	  LOWER(*(argument + begin + look_at));
      
      *(first_arg + look_at)='\0';
      begin += look_at;
    }
  while (fill_word(first_arg));
  
  return(argument+begin);
}



void only_argument(char *argument, char *dest)
{
  while (*argument && isspace(*argument))
    argument++;
  strcpy(dest, argument);
}




int fill_word(char *argument)
{
  return ( search_block(argument,fill,TRUE) >= 0);
}





/* determine if a given string is an abbreviation of another */
int is_abbrev(char *arg1, char *arg2)
{
  if (!*arg1)
    return(0);
  
  for (; *arg1; arg1++, arg2++)
    if (LOWER(*arg1) != LOWER(*arg2))
      return(0);
  
  return(1);
}




/* return first 'word' plus trailing substring of input string */
void half_chop(char *string, char *arg1, char *arg2)
{
  for (; isspace(*string); string++);
  
  for (; !isspace(*arg1 = *string) && *string; string++, arg1++);
  
  *arg1 = '\0';
  
  for (; isspace(*string); string++);
  
  for (; *arg2 = *string; string++, arg2++);
}

int special(struct char_data *ch, int cmd, char *arg)
{
  register struct obj_data *i;
  register struct char_data *k;
  int j;

  
  
  if (ch->in_room == NOWHERE) {
    char_to_room(ch, 2999);
    return;
  }
  
  /* special in room? */
  if (real_roomp(ch->in_room)->funct)
    if ((*real_roomp(ch->in_room)->funct)(ch, cmd, arg))
      return(1);
  
  /* special in equipment list? */
  for (j = 0; j <= (MAX_WEAR - 1); j++)
    if (ch->equipment[j] && ch->equipment[j]->item_number>=0)
      if (obj_index[ch->equipment[j]->item_number].func)
	if ((*obj_index[ch->equipment[j]->item_number].func)
	    (ch, cmd, arg, ch->equipment[j]))
	  return(1);
  
  /* special in inventory? */
  for (i = ch->carrying; i; i = i->next_content)
    if (i->item_number>=0)
      if (obj_index[i->item_number].func)
       if ((*obj_index[i->item_number].func)(ch, cmd, arg, i))
	  return(1);
  
  
  /* special in mobile present? */
  for (k = real_roomp(ch->in_room)->people; k; k = k->next_in_room)
    if ( IS_MOB(k) )
      if (mob_index[k->nr].func)
	if ((*mob_index[k->nr].func)(ch, cmd, arg))
	  return(1);
  
  
  /* special in object present? */
  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content)
    if (i->item_number>=0)
      if (obj_index[i->item_number].func)
	if ((*obj_index[i->item_number].func)(ch, cmd, arg, i))
	  return(1);
  
  
  return(0);
}

void assign_command_pointers ( void )
{
  int position;
  
  for (position = 0 ; position < MAX_CMD_LIST; position++)
    cmd_info[position].command_pointer = 0;
  
  COMMANDO(1,POSITION_STANDING,do_move,0);
  COMMANDO(2,POSITION_STANDING,do_move,0);
  COMMANDO(3,POSITION_STANDING,do_move,0);
  COMMANDO(4,POSITION_STANDING,do_move,0);
  COMMANDO(5,POSITION_STANDING,do_move,0);
  COMMANDO(6,POSITION_STANDING,do_move,0);
  COMMANDO(7,POSITION_STANDING,do_enter,0);
  COMMANDO(8,POSITION_RESTING,do_exits,0);
  COMMANDO(9,POSITION_RESTING,do_action,0);
  COMMANDO(10,POSITION_RESTING,do_get,0);
  COMMANDO(11,POSITION_RESTING,do_drink,0);
  COMMANDO(12,POSITION_RESTING,do_eat,0);
  COMMANDO(13,POSITION_RESTING,do_wear,0);
  COMMANDO(14,POSITION_RESTING,do_wield,0);
  COMMANDO(15,POSITION_RESTING,do_look,0);
  COMMANDO(16,POSITION_DEAD,do_score,0);
  COMMANDO(17,POSITION_RESTING,do_say,0);
  COMMANDO(18,POSITION_RESTING,do_shout,5);
  COMMANDO(19,POSITION_RESTING,do_tell,0);
  COMMANDO(20,POSITION_DEAD,do_inventory,0);
  COMMANDO(21,POSITION_DEAD,do_qui,0);
  COMMANDO(22,POSITION_STANDING,do_action,0);
  COMMANDO(23,POSITION_RESTING,do_action,0);
  COMMANDO(24,POSITION_STANDING,do_action,0);
  COMMANDO(25,POSITION_FIGHTING,do_kill,0);
  COMMANDO(26,POSITION_RESTING,do_action,0);
  COMMANDO(27,POSITION_RESTING,do_action,0);
  COMMANDO(28,POSITION_RESTING,do_action,0);
  COMMANDO(29,POSITION_RESTING,do_action,0);
  COMMANDO(30,POSITION_RESTING,do_action,0);
  COMMANDO(31,POSITION_RESTING,do_action,0);
  COMMANDO(32,POSITION_RESTING,do_action,0);
  COMMANDO(33,POSITION_RESTING,do_insult,0);
  COMMANDO(34,POSITION_RESTING,do_action,0);
  COMMANDO(35,POSITION_RESTING,do_action,0);
  COMMANDO(36,POSITION_RESTING,do_action,0);
  COMMANDO(37,POSITION_RESTING,do_action,0);
  COMMANDO(38,POSITION_DEAD,do_help,0);
  COMMANDO(39,POSITION_DEAD,do_who,0);
  COMMANDO(40,POSITION_SLEEPING,do_emote,1);
  COMMANDO(41,POSITION_SLEEPING,do_echo,1);
  COMMANDO(42,POSITION_RESTING,do_stand,0);
  COMMANDO(43,POSITION_RESTING,do_sit,0);
  COMMANDO(44,POSITION_RESTING,do_rest,0);
  COMMANDO(45,POSITION_SLEEPING,do_sleep,0);
  COMMANDO(46,POSITION_SLEEPING,do_wake,0);
  COMMANDO(47,POSITION_SLEEPING,do_force,LESSER_GOD);
  COMMANDO(48,POSITION_SLEEPING,do_trans,DEMIGOD);
  COMMANDO(49,POSITION_RESTING,do_action,0);
  COMMANDO(50,POSITION_RESTING,do_action,0);
  COMMANDO(51,POSITION_RESTING,do_action,0);
  COMMANDO(52,POSITION_RESTING,do_action,0);
  COMMANDO(53,POSITION_RESTING,do_action,0);
  COMMANDO(54,POSITION_SLEEPING,do_news,0);
  COMMANDO(55,POSITION_SLEEPING,do_equipment,0);
  COMMANDO(56,POSITION_STANDING,do_not_here,0);
  COMMANDO(57,POSITION_STANDING,do_not_here,0);
  COMMANDO(58,POSITION_STANDING,do_not_here,0);
  COMMANDO(59,POSITION_STANDING,do_not_here,0);
  COMMANDO(60,POSITION_RESTING,do_drop,0);
  COMMANDO(61,POSITION_SLEEPING,do_goto,LOW_IMMORTAL);
  COMMANDO(62,POSITION_RESTING,do_weather,0);
  COMMANDO(63,POSITION_RESTING,do_read,0);
  COMMANDO(64,POSITION_STANDING,do_pour,0);
  COMMANDO(65,POSITION_RESTING,do_grab,0);
  COMMANDO(66,POSITION_RESTING,do_remove,0);
  COMMANDO(67,POSITION_RESTING,do_put,0);
  COMMANDO(68,POSITION_DEAD,do_shutdow,SILLYLORD);
  COMMANDO(69,POSITION_SLEEPING,do_save,0);
  COMMANDO(70,POSITION_SITTING,do_hit,0);
  COMMANDO(71,POSITION_SLEEPING,do_string,52);
  COMMANDO(72,POSITION_RESTING,do_give,0);
  COMMANDO(73,POSITION_DEAD,do_quit,0);
  COMMANDO(74,POSITION_DEAD,do_stat,CREATOR);
  COMMANDO(75,POSITION_STANDING,do_guard,1);
  COMMANDO(76,POSITION_DEAD,do_time,0);
  COMMANDO(77,POSITION_DEAD,do_load,55);
  COMMANDO(78,POSITION_DEAD,do_purge,LOW_IMMORTAL);
  COMMANDO(79,POSITION_DEAD,do_shutdown,IMPLEMENTOR);
  COMMANDO(80,POSITION_DEAD,do_idea,0);
  COMMANDO(81,POSITION_DEAD,do_typo,0);
  COMMANDO(82,POSITION_DEAD,do_bug,0);
  COMMANDO(83,POSITION_RESTING,do_whisper,0);
  COMMANDO(84,POSITION_SITTING,do_cast,1);
  COMMANDO(85,POSITION_DEAD,do_at,CREATOR);
  COMMANDO(86,POSITION_RESTING,do_ask,0);
  COMMANDO(87,POSITION_RESTING,do_order,1);
  COMMANDO(88,POSITION_RESTING,do_sip,0);
  COMMANDO(89,POSITION_RESTING,do_taste,0);
  COMMANDO(90,POSITION_DEAD,do_snoop,GOD);
  COMMANDO(91,POSITION_RESTING,do_follow,0);
  COMMANDO(92,POSITION_STANDING,do_not_here,1);
  COMMANDO(93,POSITION_FIGHTING,do_bload,1);
  COMMANDO(94,POSITION_RESTING,do_action,0);
  COMMANDO(95,POSITION_DEAD,do_advance,IMPLEMENTOR);
  COMMANDO(96,POSITION_SITTING,do_action,0);
  COMMANDO(97,POSITION_RESTING,do_action,0);
  COMMANDO(98,POSITION_STANDING,do_action,0);
  COMMANDO(99,POSITION_SITTING,do_open,0);
  COMMANDO(100,POSITION_SITTING,do_close,0);
  COMMANDO(101,POSITION_SITTING,do_lock,0);
  COMMANDO(102,POSITION_SITTING,do_unlock,0);
  COMMANDO(103,POSITION_STANDING,do_leave,0);
  COMMANDO(104,POSITION_RESTING,do_action,0);
  COMMANDO(105,POSITION_RESTING,do_action,0);
  COMMANDO(106,POSITION_RESTING,do_action,0);
  COMMANDO(107,POSITION_RESTING,do_action,0);
  COMMANDO(108,POSITION_RESTING,do_action,0);
  COMMANDO(109,POSITION_RESTING,do_action,0);
  COMMANDO(110,POSITION_STANDING,do_action,0);
  COMMANDO(111,POSITION_RESTING,do_action,0);
  COMMANDO(112,POSITION_STANDING,do_action,0);
  COMMANDO(113,POSITION_RESTING,do_action,0);
  COMMANDO(114,POSITION_RESTING,do_action,0);
  COMMANDO(115,POSITION_RESTING,do_action,0);
  COMMANDO(116,POSITION_RESTING,do_action,0);
  COMMANDO(117,POSITION_RESTING,do_action,0);
  COMMANDO(118,POSITION_RESTING,do_action,0);
  COMMANDO(119,POSITION_RESTING,do_action,0);
  COMMANDO(120,POSITION_RESTING,do_action,0);
  COMMANDO(121,POSITION_RESTING,do_action,0);
  COMMANDO(122,POSITION_RESTING,do_action,0);
  COMMANDO(123,POSITION_RESTING,do_action,0);
  COMMANDO(124,POSITION_RESTING,do_action,0);
  COMMANDO(125,POSITION_RESTING,do_action,0);
  COMMANDO(126,POSITION_STANDING,do_action,0);
  COMMANDO(127,POSITION_RESTING,do_action,0);
  COMMANDO(128,POSITION_RESTING,do_action,0);
  COMMANDO(129,POSITION_RESTING,do_action,0);
  COMMANDO(130,POSITION_RESTING,do_action,0);
  COMMANDO(131,POSITION_RESTING,do_action,0);
  COMMANDO(132,POSITION_RESTING,do_action,0);
  COMMANDO(133,POSITION_RESTING,do_action,0);
  COMMANDO(134,POSITION_RESTING,do_action,0);
  COMMANDO(135,POSITION_RESTING,do_action,0);
  COMMANDO(136,POSITION_SLEEPING,do_action,0);
  COMMANDO(137,POSITION_STANDING,do_action,0);
  COMMANDO(138,POSITION_RESTING,do_action,0);
  COMMANDO(139,POSITION_RESTING,do_action,0);
  COMMANDO(140,POSITION_STANDING,do_action,0);
  COMMANDO(141,POSITION_RESTING,do_action,0);
  COMMANDO(142,POSITION_RESTING,do_action,0);
  COMMANDO(143,POSITION_RESTING,do_action,0);
  COMMANDO(144,POSITION_RESTING,do_action,0);
  COMMANDO(145,POSITION_STANDING,do_action,0);
  COMMANDO(146,POSITION_RESTING,do_action,0);
  COMMANDO(147,POSITION_RESTING,do_action,0);
  COMMANDO(148,POSITION_STANDING,do_action,DEMIGOD);
  COMMANDO(149,POSITION_STANDING,do_write,1);
  COMMANDO(150,POSITION_RESTING,do_grab,1);
  COMMANDO(151,POSITION_FIGHTING,do_flee,1);	
  COMMANDO(152,POSITION_STANDING,do_sneak,1);	
  COMMANDO(153,POSITION_RESTING,do_hide,1);	
  COMMANDO(154,POSITION_STANDING,do_backstab,1);	
  COMMANDO(155,POSITION_STANDING,do_pick,1);	
  COMMANDO(156,POSITION_STANDING,do_steal,1);	
  COMMANDO(157,POSITION_FIGHTING,do_bash,1);	
  COMMANDO(158,POSITION_FIGHTING,do_rescue,1);
  COMMANDO(159,POSITION_FIGHTING,do_kick,1);
  COMMANDO(160,POSITION_RESTING,do_action,0);
  COMMANDO(161,POSITION_RESTING,do_action,0);
  COMMANDO(162,POSITION_RESTING,do_action,0);
  COMMANDO(163,POSITION_RESTING,do_action,0);
  COMMANDO(164,POSITION_RESTING,do_practice,1);
  COMMANDO(165,POSITION_RESTING,do_action,0);
  COMMANDO(166,POSITION_SITTING,do_examine,0);
  COMMANDO(167,POSITION_RESTING,do_get,0); /* TAKE */
  COMMANDO(168,POSITION_SLEEPING,do_info,0);
  COMMANDO(169,POSITION_RESTING,do_say,0);
  COMMANDO(170,POSITION_RESTING,do_practice,1);
  COMMANDO(171,POSITION_RESTING,do_action,0);
  COMMANDO(172,POSITION_SITTING,do_use,1);
  COMMANDO(173,POSITION_DEAD,do_where,1);
  COMMANDO(174,POSITION_DEAD,do_levels,0);
  COMMANDO(175,POSITION_DEAD,do_reroll,SILLYLORD);
  COMMANDO(176,POSITION_SITTING,do_pray,0);
  COMMANDO(177,POSITION_SLEEPING,do_emote,1);
  COMMANDO(178,POSITION_RESTING,do_action,0);
  COMMANDO(179,POSITION_RESTING,do_action,0);
  COMMANDO(180,POSITION_RESTING,do_action,0);
  COMMANDO(181,POSITION_SLEEPING,do_action,0);
  COMMANDO(182,POSITION_RESTING,do_action,0);
  COMMANDO(183,POSITION_RESTING,do_action,0);
  COMMANDO(184,POSITION_RESTING,do_action,0);
  COMMANDO(185,POSITION_RESTING,do_action,0);
  COMMANDO(186,POSITION_RESTING,do_action,0);
  COMMANDO(187,POSITION_RESTING,do_action,0);
  COMMANDO(188,POSITION_RESTING,do_action,0);
  COMMANDO(189,POSITION_RESTING,do_action,0);
  COMMANDO(190,POSITION_RESTING,do_action,0);
  COMMANDO(191,POSITION_RESTING,do_action,0);
  COMMANDO(192,POSITION_RESTING,do_action,0);
  COMMANDO(193,POSITION_RESTING,do_action,0);
  COMMANDO(194,POSITION_RESTING,do_action,0);
  COMMANDO(195,POSITION_RESTING,do_commune,LOW_IMMORTAL);
  COMMANDO(196,POSITION_RESTING,do_action,0);
  COMMANDO(197,POSITION_RESTING,do_action,0);
  COMMANDO(198,POSITION_RESTING,do_action,0);
  COMMANDO(199,POSITION_DEAD,do_brief,0);
  COMMANDO(200,POSITION_DEAD,do_wizlist,0);
  COMMANDO(201,POSITION_RESTING,do_consider,0);
  COMMANDO(202,POSITION_RESTING,do_group,1);
  COMMANDO(203,POSITION_DEAD,do_restore,DEMIGOD);
  COMMANDO(204,POSITION_DEAD,do_return,0);
  COMMANDO(205,POSITION_DEAD,do_switch,52);
  COMMANDO(206,POSITION_RESTING,do_quaff,0);
  COMMANDO(207,POSITION_FIGHTING,do_recite,0);
  COMMANDO(208,POSITION_DEAD,do_users,52);
  COMMANDO(209,POSITION_STANDING,do_pose,0);
  COMMANDO(210,POSITION_SLEEPING,do_noshout,51);
  COMMANDO(211,POSITION_SLEEPING,do_wizhelp,51);
  COMMANDO(212,POSITION_DEAD,do_credits,0);
  COMMANDO(213,POSITION_DEAD,do_compact,0);
  COMMANDO(214,POSITION_SLEEPING,do_emote,1);
  COMMANDO(215,POSITION_SLEEPING,do_qui,1);
  COMMANDO(216,POSITION_STANDING,do_kill,SILLYLORD);
  COMMANDO(217,POSITION_DEAD,do_wimpy,0);
  COMMANDO(218,POSITION_RESTING,do_junk,1);
  COMMANDO(219,POSITION_RESTING,do_not_here,1);
  COMMANDO(220,POSITION_RESTING,do_not_here,1);
  COMMANDO(221,POSITION_RESTING,do_not_here,1);
  COMMANDO(222,POSITION_DEAD,do_nohassle,51);
  COMMANDO(223,POSITION_DEAD,do_system,IMPLEMENTOR);
  COMMANDO(224,POSITION_STANDING,do_not_here,1);
  COMMANDO(225,POSITION_DEAD,do_stealth,52);
  COMMANDO(226,POSITION_DEAD,do_edit,1);
  COMMANDO(227,POSITION_DEAD,do_set,SILLYLORD);
  COMMANDO(228,POSITION_DEAD,do_rsave,LOW_IMMORTAL);
  COMMANDO(229,POSITION_DEAD,do_rload,LOW_IMMORTAL);
  COMMANDO(230,POSITION_DEAD,do_track,1);
  COMMANDO(231,POSITION_DEAD,do_wizlock,DEMIGOD);
  COMMANDO(232,POSITION_DEAD,do_highfive,1);
  COMMANDO(233,POSITION_DEAD,do_title,5);
  COMMANDO(234,POSITION_DEAD,do_whozone,0);
  COMMANDO(235,POSITION_FIGHTING,do_assist,0);
  COMMANDO(236,POSITION_DEAD,do_attribute,5);
  COMMANDO(237,POSITION_DEAD,do_world,0);
  COMMANDO(238,POSITION_DEAD,do_spells,0);
  COMMANDO(239,POSITION_FIGHTING,do_fire,0);
  COMMANDO(240,POSITION_DEAD,do_show,CREATOR);
  COMMANDO(241,POSITION_FIGHTING,do_bodyslam,20);
  COMMANDO(242,POSITION_DEAD,do_invis,51);
  COMMANDO(243,POSITION_DEAD,do_gain,1);
  COMMANDO(244,POSITION_DEAD,do_instazone,CREATOR);
  COMMANDO(245,POSITION_FIGHTING,do_disarm,1);
  COMMANDO(246,POSITION_SITTING,do_action,1);
  COMMANDO(247,POSITION_SITTING,do_passwd,BRUTIUS);
  COMMANDO(248,POSITION_SITTING,do_not_here,0);
  COMMANDO(249,POSITION_SITTING,do_imptest,IMPLEMENTOR);
  COMMANDO(250,POSITION_FIGHTING, do_shoot, 1);  
  COMMANDO(251,POSITION_STANDING, do_silence, DEMIGOD);  
  COMMANDO(252,POSITION_STANDING, do_teams, DEMIGOD);
  COMMANDO(253,POSITION_SLEEPING, do_grouptell, 1);
  COMMANDO(254,POSITION_RESTING, do_send, 1);
  COMMANDO(255,POSITION_RESTING, do_log, 58);
  COMMANDO(256,POSITION_SLEEPING, do_monitor,51);
  COMMANDO(257,POSITION_SLEEPING, do_channel, 1);
  COMMANDO(258,POSITION_RESTING, do_report, 1);
  COMMANDO(259,POSITION_DEAD, do_demote, BRUTIUS);
  COMMANDO(260,POSITION_SLEEPING, do_atlas, 1);
  COMMANDO(261,POSITION_RESTING, do_action, 1);
  COMMANDO(262,POSITION_RESTING, do_action, 1);
  COMMANDO(263,POSITION_RESTING, do_action, 1);
  COMMANDO(264,POSITION_RESTING, do_action, 1);
  COMMANDO(265,POSITION_RESTING, do_action, 1);
  COMMANDO(266,POSITION_FIGHTING, do_split, 1);
  COMMANDO(267,POSITION_SLEEPING, do_command, 0);
  COMMANDO(268,POSITION_FIGHTING, do_deathstroke, 60);
  COMMANDO(269,POSITION_STANDING, do_action, 0);
  COMMANDO(270,POSITION_FIGHTING, do_reload, 1);
  COMMANDO(271,POSITION_STANDING, do_action, 1);
  COMMANDO(272,POSITION_STANDING, do_action, 1);
  COMMANDO(273,POSITION_STANDING, do_action, 1);
  COMMANDO(274,POSITION_RESTING, do_bet, 1);
  COMMANDO(275,POSITION_RESTING, do_stay, 1);
  COMMANDO(276,POSITION_RESTING, do_peek, 1);
  COMMANDO(277,POSITION_SLEEPING, do_color, 1);
  COMMANDO(278,POSITION_FIGHTING, do_headbutt, 15);
  COMMANDO(279,POSITION_STANDING, do_subterfuge, 10);
  COMMANDO(280,POSITION_STANDING, do_throw, 15);
  COMMANDO(281,POSITION_DEAD, do_oset, 58);
  COMMANDO(282,POSITION_DEAD, do_scribe, 10);
  COMMANDO(283,POSITION_STANDING, do_brew, 10);
  COMMANDO(284,POSITION_FIGHTING, do_grapple, 20);
  COMMANDO(285,POSITION_FIGHTING, do_action, 1);
  COMMANDO(286,POSITION_STANDING, do_action, 1);
  COMMANDO(287,POSITION_STANDING, do_action, 1);
  COMMANDO(288,POSITION_STANDING, do_action, 1);
  COMMANDO(289,POSITION_STANDING, do_search, 1);
  COMMANDO(290,POSITION_STANDING, do_spy, 1);
  COMMANDO(291,POSITION_STANDING, do_doorbash, 1);
  COMMANDO(292,POSITION_DEAD, do_play, 1);                     /*Russ*/
  COMMANDO(293,POSITION_SLEEPING, do_flag, 52);                /*Russ*/
  COMMANDO(294,POSITION_FIGHTING, do_quivering_palm, 30);
  COMMANDO(295,POSITION_FIGHTING, do_feign_death, 1);
  COMMANDO(296,POSITION_RESTING, do_springleap, 1);
  COMMANDO(297,POSITION_RESTING, do_first_aid, 1);
  COMMANDO(298,POSITION_STANDING, do_swim, 1);
  COMMANDO(299,POSITION_STANDING, do_sign, 1);
  COMMANDO(300,POSITION_STANDING, do_link, 54);
  COMMANDO(301,POSITION_RESTING, do_lay_hands, 1);
  COMMANDO(302,POSITION_SLEEPING, do_wiznews, 52);		/* SG */
  COMMANDO(303,POSITION_STANDING, do_not_here, 1);
  COMMANDO(304,POSITION_STANDING, do_not_here, 1);
  COMMANDO(305,POSITION_STANDING, do_not_here, 1);
  COMMANDO(306,POSITION_DEAD, do_cls, 0);
  COMMANDO(307,POSITION_FIGHTING, do_glance, 1);
  COMMANDO(308,POSITION_DEAD, do_terminal, 0);
  COMMANDO(309,POSITION_DEAD, do_prompt, 0);
  COMMANDO(310,POSITION_DEAD, do_bamfin, 51);
  COMMANDO(311,POSITION_DEAD, do_bamfout, 51);
  COMMANDO(312,POSITION_STANDING, do_not_here, 1);
  COMMANDO(313,POSITION_DEAD, do_checklog, 59);
  COMMANDO(314,POSITION_DEAD, do_loglist, 54);
  COMMANDO(315,POSITION_DEAD, do_deathcheck, 54);
  COMMANDO(316,POSITION_RESTING, do_action, 1);
        COMMANDO(317,POSITION_RESTING, do_action, 1);
        COMMANDO(318,POSITION_RESTING, do_action, 1);


}

/* *************************************************************************
 *  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
 ************************************************************************* */




/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name)
{
  int i;
  
  for (i = 0; i <= top_of_p_table; i++)	{
    if (!str_cmp((player_table + i)->name, name))
      return(i);
  }
  
  return(-1);
}



int _parse_name(char *arg, char *name)
{
  int i;
  
  /* skip whitespaces */
  for (; isspace(*arg); arg++);
  
  for (i = 0; *name = *arg; arg++, i++, name++) 
    if ((*arg <0) || !isalpha(*arg) || i > 15)
      return(1); 
  
  if (!i)
    return(1);
  
  return(0);
}





/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg)
{
  char buf[100], buf2[100], recipient[100], *tmp;
  int player_i, index=0, count=0, oops=FALSE;
  char tmp_name[20];
  struct char_file_u tmp_store;
  struct char_data *tmp_ch;
  struct descriptor_data *k;
  extern struct descriptor_data *descriptor_list;
  extern int WizLock;
  extern int plr_tick_count;
  
  void do_look(struct char_data *ch, char *argument, int cmd);
  void load_char_objs(struct char_data *ch);
  int load_char(char *name, struct char_file_u *char_element);
  
  write(d->descriptor, echo_on, 6);
  
  switch (STATE(d))	{
    
  case CON_QRACE:
    
    for (; isspace(*arg); arg++)  ;
    if (!*arg) {
      SEND_TO_Q("Choose A Race:\n\r", d);
      SEND_TO_Q("D)warf, E)lf, H)uman, G)nome, hoB)bit, O)gre\n\r",d); 
      SEND_TO_Q("For help type '?'. \n\r RACE?:  ", d);
      STATE(d) = CON_QRACE;
    } else {
      switch (*arg)  	{
      case 'o':
      case 'O': {
	GET_RACE(d->character) = RACE_OGRE;
	SEND_TO_Q("What is your sex (M/F) ? ", d);
	STATE(d) = CON_QSEX;
	
      } break;
	
      case 'b':
      case 'B': {
	GET_RACE(d->character) = RACE_HOBBIT;
	SEND_TO_Q("What is your sex (M/F) ? ", d);
	STATE(d) = CON_QSEX;
	
      } break;

      case 'd':
      case 'D': {
        GET_RACE(d->character) = RACE_DWARF;
        SEND_TO_Q("What is your sex (M/F) ? ", d);
        STATE(d) = CON_QSEX;

      } break;

      case 'e':
      case 'E': {
        GET_RACE(d->character) = RACE_ELVEN;
        SEND_TO_Q("What is your sex (M/F) ? ", d);
        STATE(d) = CON_QSEX;

      } break;

      case '?': {
	SEND_TO_Q(RACEHELP, d);
	SEND_TO_Q("Choose A Race:\n\r", d);
	SEND_TO_Q("D)warf, E)lf, H)uman, O)gre, hoB)bit, G)nome\n\r",d); 
	SEND_TO_Q("For help type '?'. \n\r RACE?:  ", d);
	STATE(d) = CON_QRACE;
      } break;
	
      case 'h':
      case 'H': {
	GET_RACE(d->character) = RACE_HUMAN;
	SEND_TO_Q("What is your sex (M/F) ? ", d);
	STATE(d) = CON_QSEX;
	
      } break;

      case 'g':
      case 'G': {
        GET_RACE(d->character) = RACE_GNOME;
        SEND_TO_Q("What is your sex (M/F) ? ", d);
        STATE(d) = CON_QSEX;

      } break;
	
	default : {
	  SEND_TO_Q("\n\rThat's not a race.\n\rRACE?:", d);
	  STATE(d) = CON_QRACE;
	} break;
	
      }
    }
    break;
    
  case CON_NME:		/* wait for input of name	*/
    if (!d->character) {
      CREATE(d->character, struct char_data, 1);
      clear_char(d->character);
      d->character->desc = d;
    }
    
    for (; isspace(*arg); arg++)  ;
    if (!*arg)
      close_socket(d);
    else {
      
      if(_parse_name(arg, tmp_name)) 	{
	SEND_TO_Q("Illegal name, please try another.", d);
	SEND_TO_Q("Name: ", d);
	return;
      }
      
      
      /* Check if already playing */
      for(k=descriptor_list; k; k = k->next) {
	if ((k->character != d->character) && k->character) {
	  if (k->original) {
	    if (GET_NAME(k->original) &&
		(str_cmp(GET_NAME(k->original), tmp_name) == 0))  {
		SEND_TO_Q("Already playing, cannot connect\n\r", d);
		SEND_TO_Q("Name: ", d);
		return;
	      }
	  } else { /* No switch has been made */
	    if (GET_NAME(k->character) &&
		(str_cmp(GET_NAME(k->character), tmp_name) == 0))
	      {
		SEND_TO_Q("Already playing, cannot connect\n\r", d);
		SEND_TO_Q("Name: ", d);
		return;
	      }
	  }
	}
      }
      
      if ((player_i = load_char(tmp_name, &tmp_store)) > -1)  {
	/*
	 *  check for tmp_store.max_corpse;
	 */
	/*
	  if (tmp_store.max_corpse > 3) {
	  SEND_TO_Q("Too many corpses in game, can't connect\n\r", d);
	  sprintf(buf, "%s: too many corpses.",tmp_name);
	  log(buf);
	  STATE(d) = CON_WIZLOCK;
	  break;
	  }
	*/
	store_to_char(&tmp_store, d->character);       	
	strcpy(d->pwd, tmp_store.pwd);
	d->pos = player_table[player_i].nr;
	SEND_TO_Q("Password: ", d);
	write(d->descriptor, echo_off, 4); 
	STATE(d) = CON_PWDNRM;
        d->max_str = 0;	
      } else {
	/* player unknown gotta make a new */
	if (!WizLock) {
	  CREATE(GET_NAME(d->character), char, 
		 strlen(tmp_name) + 1);
	  strcpy(GET_NAME(d->character), CAP(tmp_name));
	  sprintf(buf, "Did I get that right, %s (Y/N)? ",
		  tmp_name);
	  SEND_TO_Q(buf, d);
	  STATE(d) = CON_NMECNF;
	} else {
	  sprintf(buf, "Sorry, %s, no new characters at this time\n\r",
		  GET_NAME(d->character));
	  SEND_TO_Q(buf,d);
	  STATE(d) = CON_WIZLOCK;
	}
      }
    }
    break;

  case CON_NMECNF:	/* wait for conf. of new name	*/
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    
    if (*arg == 'y' || *arg == 'Y')	{
      SEND_TO_Q("New character.\n\r", d);
      
      sprintf(buf, 
	      "Give me a password for %s: ",
	      GET_NAME(d->character));
      SEND_TO_Q(buf,d);
      
  
      write(d->descriptor, echo_off, 4);
      STATE(d) = CON_PWDGET;
    } else 	{
      if (*arg == 'n' || *arg == 'N') {
	SEND_TO_Q("Ok, what IS it, then? ", d);
	free(GET_NAME(d->character));
	STATE(d) = CON_NME;
      } else { /* Please do Y or N */
	SEND_TO_Q("Please type Yes or No? ", d);
      }
    }
    break;
    
  case CON_PWDNRM:	/* get pwd for known player	*/
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    if (!*arg)
      close_socket(d);
    else  {
      if (strncmp(crypt(arg, d->pwd), d->pwd, 10)) 	{
	SEND_TO_Q("Wrong password.\n\r", d);
        if (d->max_str > 3)	{		
           close_socket(d);
	   break;
        }
	SEND_TO_Q("Password: ", d);
        (d->max_str)++;	
	write(d->descriptor, echo_off, 4);
	return;
      }
      d->max_str = 0;
      for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
	if ((!str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch)) &&
	     !tmp_ch->desc && !IS_NPC(tmp_ch)) || 
	    (IS_NPC(tmp_ch) && tmp_ch->orig && 
	     !str_cmp(GET_NAME(d->character), 
		      GET_NAME(tmp_ch->orig)))) {
	  
	  write(d->descriptor, echo_on, 6);
	  SEND_TO_Q("Reconnecting.\n\r", d);
	  
	  free_char(d->character);
	  tmp_ch->desc = d;
	  d->character = tmp_ch;
	  tmp_ch->specials.timer = 0;
          tmp_ch->invis_level = 0;

	  if (tmp_ch->orig) {
	    tmp_ch->desc->original = tmp_ch->orig;
	    tmp_ch->orig = 0;
	  }
	  STATE(d) = CON_PLYNG;

          d->screen_size = 24;
	  
	  act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
	  sprintf(buf, "%s[%s] has reconnected.", GET_NAME(
							   d->character), d->host);
	  log(buf);
	  return;
	}
      
      
      sprintf(buf, "%s[%s] has connected.", GET_NAME(d->character),
	      d->host);
      log(buf);
      if ((IS_SET(d->character->specials.act, PLR_ANSI)) ||
          (IS_SET(d->character->specials.act, PLR_VT100))) {
          SEND_TO_Q(VT_CLENSEQ,d);
      }
      SEND_TO_Q(motd, d);
      SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
      
      STATE(d) = CON_RMOTD;
    }
    break;
    
  case CON_PWDGET:	/* get pwd for new player	*/
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    
    if (!*arg || strlen(arg) > 10) 	{
      
      write(d->descriptor, echo_on, 6);
      SEND_TO_Q("Illegal password.\n\r", d);
      SEND_TO_Q("Password: ", d);
      
      write(d->descriptor, echo_off, 4);
      return;
    }
    
    strncpy(d->pwd, crypt(arg, d->character->player.name), 10);
    *(d->pwd + 10) = '\0';
    write(d->descriptor, echo_on, 6);
    SEND_TO_Q("Please retype password: ", d);
    write(d->descriptor, echo_off, 4);
    STATE(d) = CON_PWDCNF;
    break;
    
  case CON_PWDCNF:	/* get confirmation of new pwd	*/
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    
    if (strncmp(crypt(arg, d->pwd), d->pwd, 10)) {
      write(d->descriptor, echo_on, 6);
      
      SEND_TO_Q("Passwords don't match.\n\r", d);
      SEND_TO_Q("Retype password: ", d);
      STATE(d) = CON_PWDGET;
      write(d->descriptor, echo_off, 4);
      return;
    } else {
      write(d->descriptor, echo_on, 6);
      
      SEND_TO_Q("Choose A Race:\n\r", d);
      SEND_TO_Q("D)warf, E)lf, H)uman, O)gre, hoB)bit, G)nome\n\r",d); 
      SEND_TO_Q("For help type '?'. \n\r RACE:  ", d);
      STATE(d) = CON_QRACE;
    }
    break;
    
  case CON_QSEX:		/* query sex of new user	*/
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    switch (*arg)
      {
      case 'm':
      case 'M':
	/* sex MALE */
	d->character->player.sex = SEX_MALE;
	break;
	
      case 'f':
      case 'F':
	/* sex FEMALE */
	d->character->player.sex = SEX_FEMALE;
	break;
	
      default:
	SEND_TO_Q("That's not a sex..\n\r", d);
	SEND_TO_Q("What IS your sex? :", d);
	return;
	break;
      }
    
   SEND_TO_Q("\n\rNow you get to choose your stat preference.\n\r",d);    
    SEND_TO_Q("Put them in order from highest to lowest.\n\r", d);
    SEND_TO_Q("for example: 'S I W D Co' would put the highest roll in Strength, \n\r",d);
    SEND_TO_Q("next in intelligence, Wisdom, Dex, and Con\n\r",d);
    SEND_TO_Q("Your choices? ",d);
    STATE(d) = CON_STAT_LIST;
    break;
  
  case CON_STAT_LIST:
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
 
    index = 0;
    while (*arg && index < MAX_STAT) {
      if (*arg == 'S' || *arg == 's') 
        d->stat[index++] = 's';
      if (*arg == 'I' || *arg == 'i')
        d->stat[index++] = 'i';
      if (*arg == 'W' || *arg == 'w')
        d->stat[index++] = 'w';
      if (*arg == 'D' || *arg == 'd')
        d->stat[index++] = 'd';
      if (*arg == 'C' || *arg == 'c') {
        arg++;
        if (*arg == 'O' || *arg == 'o') {
          d->stat[index++] = 'o';
        } else {
          SEND_TO_Q("That was an invalid choice.\n\r",d);
          SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);    
          SEND_TO_Q("Seperated by spaces.  don't duplicate letters \n\r", d);
          SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
          SEND_TO_Q("next in intelligence, Wisdom, Dex, Con and lastly Charisma\n\r",d);
          SEND_TO_Q("Your choice? ",d);
          STATE(d) = CON_STAT_LIST;
          break;
        }
      }
      arg++;      
    }



    if (index < MAX_STAT) {
      SEND_TO_Q("You did not enter enough legal stats\n\r", d);
      SEND_TO_Q("That was an invalid choice.\n\r",d);
      SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);    
      SEND_TO_Q("Seperated by spaces, don't duplicate letters \n\r", d);
      SEND_TO_Q("for example: 'S I W D Co' would put the highest roll in Strength, \n\r",d);
      SEND_TO_Q("next in intelligence, Wisdom, Dex and Con.\n\r",d);
      SEND_TO_Q("Your choice? ",d);
      STATE(d) = CON_STAT_LIST;
      break;
    } else {

    SEND_TO_Q("Please pick one of the following combinations for your class.\n\r", d);
    SEND_TO_Q("1. Warrior                    A. Antipaladin\n\r", d);
    SEND_TO_Q("2. Cleric                     B. Paladin\n\r", d);
    SEND_TO_Q("3. Magic-user                 C. Monk\n\r", d);
    SEND_TO_Q("4. Thief                      D. Ranger\n\r", d);
    SEND_TO_Q("5. Warrior/Thief              E. Mage/Cleric\n\r", d);
    SEND_TO_Q("6. Warrior/Cleric             F. Warrior/Cleric/Thief\n\r", d);
    SEND_TO_Q("7. Mage/thief                 G. Mage/Cleric/Thief\n\r", d);
    SEND_TO_Q("8. Mage/Warrior               H. Mage/Cleric/Warrior\n\r", d);
    SEND_TO_Q("9. Cleric/Thief               I. Mage/Thief/Warrior\n\r\n\r",d); 
    SEND_TO_Q("There are advantages and disadvantages to each choice.\n\r",d);
    SEND_TO_Q("Type ? to see a help file telling you these advantages and disadvantages.\n\r",d);
    SEND_TO_Q("Class :", d);
    STATE(d) = CON_QCLASS;
    break;
  }
    
  case CON_QCLASS : {
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    d->character->player.class = 0;
    count=0;
    oops=FALSE;
      switch (*arg)  	{
       case '1' : {
         d->character->player.class = CLASS_WARRIOR;
         STATE(d) = CON_RMOTD;
       }  break;
       case '2' : {
         d->character->player.class = CLASS_CLERIC;
         STATE(d) = CON_RMOTD;
       }  break; 
       case '3' : {
         d->character->player.class = CLASS_MAGIC_USER;
         STATE(d) = CON_RMOTD;
       }  break;
       case '4' : {
         d->character->player.class = CLASS_THIEF;
         STATE(d) = CON_RMOTD;
       }  break;
       case '5' : {
         d->character->player.class = CLASS_WARRIOR + CLASS_THIEF;
         STATE(d) = CON_RMOTD;
       }  break;
       case '6' : {
         d->character->player.class = CLASS_WARRIOR + CLASS_CLERIC;
         STATE(d) = CON_RMOTD;
       }  break;
       case '7' : {
         d->character->player.class = CLASS_MAGIC_USER + CLASS_THIEF;
         STATE(d) = CON_RMOTD;
       }  break;
       case '8' : {
         d->character->player.class = CLASS_MAGIC_USER + CLASS_WARRIOR;
         STATE(d) = CON_RMOTD;
       }  break;
       case '9' : {
         d->character->player.class = CLASS_CLERIC + CLASS_THIEF;
         STATE(d) = CON_RMOTD;
       }  break;
       case 'a' :
       case 'A' : {
         d->character->player.class = CLASS_ANTIPALADIN;
         STATE(d) = CON_RMOTD;
       }  break;
       case 'b' :
       case 'B' : {
         d->character->player.class = CLASS_PALADIN;
         STATE(d) = CON_RMOTD;
       }  break;
       case 'c' :
       case 'C' : {
         d->character->player.class = CLASS_MONK;
         STATE(d) = CON_RMOTD;
       } break;
       case 'd' :
       case 'D' : {
         d->character->player.class = CLASS_RANGER;
         STATE(d) = CON_RMOTD;
       }  break;
       case 'e' :
       case 'E' : {
        d->character->player.class = CLASS_MAGIC_USER+CLASS_CLERIC; 
        STATE(d) = CON_RMOTD;
       } break;
       case 'f' :
       case 'F' : {
        d->character->player.class = CLASS_WARRIOR+CLASS_THIEF+CLASS_CLERIC;
        STATE(d) = CON_RMOTD;
       } break;
       case 'g' :
       case 'G' : {
        d->character->player.class = CLASS_MAGIC_USER+CLASS_CLERIC+CLASS_THIEF;
        STATE(d) = CON_RMOTD;
       } break;
       case 'h' :
       case 'H' : {
       d->character->player.class = CLASS_MAGIC_USER+CLASS_CLERIC+CLASS_WARRIOR;
        STATE(d) = CON_RMOTD;
       } break;
       case 'i' :
       case 'I' : {
       d->character->player.class = CLASS_MAGIC_USER+CLASS_THIEF+CLASS_WARRIOR;
        STATE(d) = CON_RMOTD;
      }  break;
      case '?' : {
        SEND_TO_Q(CLASSHELP,d);
        STATE(d) = CON_QCLASS;
        oops = TRUE;
      } break;
      default:
	SEND_TO_Q("Please enter either (1-9) or (A-I)\n\r", d);
	STATE(d) = CON_QCLASS;
	oops = TRUE;
	break;
      }
#if PLAYER_AUTH
    break;
  }
#else
    if (STATE(d) != CON_QCLASS) {
      sprintf(buf, "%s [%s] new player.", GET_NAME(d->character), d->host);
      log(buf);
       /*
       ** now that classes are set, initialize
       */
      init_char(d->character);
      /* create an entry in the file */
      d->pos = create_entry(GET_NAME(d->character));
      save_char(d->character, AUTO_RENT);
      SEND_TO_Q(motd, d);
      SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
      STATE(d) = CON_RMOTD;
    }
  } break;
#endif

  case CON_RMOTD:		/* read CR after printing motd	*/
   if (IS_SET(d->character->specials.act, PLR_ANSI) ||
       (IS_SET(d->character->specials.act, PLR_VT100))) {
    SEND_TO_Q(VT_CLENSEQ,d);
    sprintf(buf,VT_MARGSET,1,22);
    SEND_TO_Q(buf,d);
   }
    SEND_TO_Q(MENU, d);
    STATE(d) = CON_SLCT;
    if (WizLock) {
      if (GetMaxLevel(d->character) < LOW_IMMORTAL) {
	sprintf(buf, "Sorry, the game is locked up for repair\n\r");
	SEND_TO_Q(buf,d);
	STATE(d) = CON_WIZLOCK;
      }
    }
    break;
    
  case CON_WIZLOCK:
    close_socket(d);
    break;

 case CON_CITY_CHOICE:
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    if (d->character->in_room != NOWHERE) {
      SEND_TO_Q("This choice is only valid when you have been auto-saved\n\r",d);
      STATE(d) = CON_SLCT;
    } else {  
      switch (*arg)  	{
      case '1':

        reset_char(d->character);
        sprintf(buf, "Loading %s's equipment", d->character->player.name);
        log(buf);
        load_char_objs(d->character);
        set_title(d->character);
        save_char(d->character, AUTO_RENT);
        send_to_char(WELC_MESSG, d->character);
        d->character->next = character_list;
        character_list = d->character;
	
      if (IS_SET(d->character->specials.act, PLR_BANISHED)) {
        char_to_room(d->character, 2);
        d->character->player.hometown = 2;
      } else {
	char_to_room(d->character, 2999);
	d->character->player.hometown = 2999;
      }

	d->character->specials.tick = plr_tick_count++;
	if (plr_tick_count == PLR_TICK_WRAP)
	  plr_tick_count=0;
	
          act("$n has entered the game.", 
	      TRUE, d->character, 0, 0, TO_ROOM);
          STATE(d) = CON_PLYNG;
          if (!GetMaxLevel(d->character))
             do_start(d->character);
          do_look(d->character, "",15);
          d->prompt_mode = 1;

	break;
      case '2':

        reset_char(d->character);
        sprintf(buf, "Loading %s's equipment", d->character->player.name);
        log(buf);
        load_char_objs(d->character);
        save_char(d->character, AUTO_RENT);
        send_to_char(WELC_MESSG, d->character);
        d->character->next = character_list;
        character_list = d->character;

      if (IS_SET(d->character->specials.act, PLR_BANISHED)) {
        char_to_room(d->character, 2);
        d->character->player.hometown = 2;
      } else {
	char_to_room(d->character, 1102);
	d->character->player.hometown = 1102;
      }

          d->character->specials.tick = plr_tick_count++;
          if (plr_tick_count == PLR_TICK_WRAP)
	     plr_tick_count=0;
      
          act("$n has entered the game.", 
	      TRUE, d->character, 0, 0, TO_ROOM);
          STATE(d) = CON_PLYNG;
          if (!GetMaxLevel(d->character))
             do_start(d->character);
          do_look(d->character, "",15);
          d->prompt_mode = 1;

	break;
      case '3':
	if (GetMaxLevel(d->character) > 5) {

          reset_char(d->character);
          sprintf(buf, "Loading %s's equipment", d->character->player.name);
          log(buf);
          load_char_objs(d->character);
          save_char(d->character, AUTO_RENT);
          send_to_char(WELC_MESSG, d->character);
          d->character->next = character_list;
          character_list = d->character;

        if (IS_SET(d->character->specials.act, PLR_BANISHED)) {
          char_to_room(d->character, 2);
          d->character->player.hometown = 2;
        } else {
	  char_to_room(d->character, 18221);
	  d->character->player.hometown = 18221;
        }

          d->character->specials.tick = plr_tick_count++;
          if (plr_tick_count == PLR_TICK_WRAP)
	     plr_tick_count=0;
      
          act("$n has entered the game.", 
	      TRUE, d->character, 0, 0, TO_ROOM);
          STATE(d) = CON_PLYNG;
          if (!GetMaxLevel(d->character))
             do_start(d->character);
          do_look(d->character, "",15);
          d->prompt_mode = 1;
	  break;

	} else {
	  SEND_TO_Q("That was an illegal choice.\n\r", d);
	  STATE(d) = CON_SLCT;
	  break;
	}
      case '4':
	if (GetMaxLevel(d->character) > 5) {

          reset_char(d->character);
          sprintf(buf, "Loading %s's equipment", d->character->player.name);
          log(buf);
          load_char_objs(d->character);
          save_char(d->character, AUTO_RENT);
          send_to_char(WELC_MESSG, d->character);
          d->character->next = character_list;
          character_list = d->character;

        if (IS_SET(d->character->specials.act, PLR_BANISHED)) {
          char_to_room(d->character, 2);
          d->character->player.hometown = 2;
        } else {
	  char_to_room(d->character, 3606);
	  d->character->player.hometown = 3606;
        }

          d->character->specials.tick = plr_tick_count++;
          if (plr_tick_count == PLR_TICK_WRAP)
	     plr_tick_count=0;
      
          act("$n has entered the game.", 
	      TRUE, d->character, 0, 0, TO_ROOM);
          STATE(d) = CON_PLYNG;
          if (!GetMaxLevel(d->character))
             do_start(d->character);
          do_look(d->character, "",15);
          d->prompt_mode = 1;
	  break;

	} else {
	  SEND_TO_Q("That was an illegal choice.\n\r", d);
	  STATE(d) = CON_SLCT;
	  break;
	}
      case '5':
	if (GetMaxLevel(d->character) > 5) {

          reset_char(d->character);
          sprintf(buf, "Loading %s's equipment", d->character->player.name);
          log(buf);
          load_char_objs(d->character);
          save_char(d->character, AUTO_RENT);
          send_to_char(WELC_MESSG, d->character);
          d->character->next = character_list;
          character_list = d->character;

       if (IS_SET(d->character->specials.act, PLR_BANISHED)) {
          char_to_room(d->character, 2);
          d->character->player.hometown = 2;
       } else {
	  char_to_room(d->character, 16107);
	  d->character->player.hometown = 16107;
       }

          d->character->specials.tick = plr_tick_count++;
          if (plr_tick_count == PLR_TICK_WRAP)
	     plr_tick_count=0;
      
          act("$n has entered the game.", 
	      TRUE, d->character, 0, 0, TO_ROOM);
          STATE(d) = CON_PLYNG;
          if (!GetMaxLevel(d->character))
             do_start(d->character);
          do_look(d->character, "",15);
          d->prompt_mode = 1;
	  break;

	} else {
	  SEND_TO_Q("That was an illegal choice.\n\r", d);
	  STATE(d) = CON_SLCT;
	  break;
	}
      default:
	SEND_TO_Q("That was an illegal choice.\n\r", d);
	STATE(d) = CON_SLCT;
        break;
      }
    }
    break;

  case CON_SLCT:		/* get selection from main menu	*/
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    switch (*arg)  	{
    case '0':
      close_socket(d);
      break;
      
    case '1':
      reset_char(d->character);
      sprintf(buf, "Loading %s's equipment", d->character->player.name);
      log(buf);
      load_char_objs(d->character);
      save_char(d->character, AUTO_RENT);
      send_to_char(WELC_MESSG, d->character);
      d->character->next = character_list;
      character_list = d->character;
      if (d->character->in_room == NOWHERE ||
	  d->character->in_room == AUTO_RENT) {
        if (IS_SET(d->character->specials.act, PLR_BANISHED)) {
          char_to_room(d->character, 2);
          d->character->player.hometown = 2;
	} else if (GetMaxLevel(d->character) < LOW_IMMORTAL) {
  	  char_to_room(d->character, 2999);
	  d->character->player.hometown = 2999;	
	} else {
	  char_to_room(d->character, 1000);
	  d->character->player.hometown = 1000;
	  blk_read(d->character);  /* Luft */
	}
      } else {
       if (!BANISHED(d->character)) {
	if (real_roomp(d->character->in_room)) {
	  char_to_room(d->character,
		       d->character->in_room);	  
	  d->character->player.hometown = d->character->in_room;
	} else { 
	  char_to_room(d->character, 2999);
	  d->character->player.hometown = 2999;
	}
       } else {
         char_to_room(d->character, 2);
         d->character->player.hometown = 2;
        }
      }

      d->character->specials.tick = plr_tick_count++;
      if (plr_tick_count == PLR_TICK_WRAP)
	plr_tick_count=0;
      
      act("$n has entered the game.", 
	  TRUE, d->character, 0, 0, TO_ROOM);
      STATE(d) = CON_PLYNG;
      if (!GetMaxLevel(d->character))
         do_start(d->character);
      do_look(d->character, "",15);
      d->prompt_mode = 1;
      break;
      
    case '2':
      SEND_TO_Q("Enter a text you'd like others to see when they look at you.\n\r", d);
      SEND_TO_Q("Terminate with a '@'.\n\r", d);
      if (d->character->player.description)	{
	  SEND_TO_Q("Old description :\n\r", d);
	  SEND_TO_Q(d->character->player.description, d);
	  free(d->character->player.description);
	  d->character->player.description = 0;
	}
      d->str = 
	&d->character->player.description;
      d->max_str = 240;
      STATE(d) = CON_EXDSCR;
      break;
      
    case '3':
      SEND_TO_Q(STORY, d);
      STATE(d) = CON_RMOTD;
      break;
    case '4':
      SEND_TO_Q("Enter a new password: ", d);

      write(d->descriptor, echo_off, 4);
      
      STATE(d) = CON_PWDNEW;
      break;

    case '5':
        SEND_TO_Q("Where would you like to enter?\n\r", d);
        SEND_TO_Q("1.    Midgaard\n\r", d);
        SEND_TO_Q("2.    Shire\n\r",    d);
      if (GetMaxLevel(d->character) > 5)
        SEND_TO_Q("3.    Mordilnia\n\r", d);
      if (GetMaxLevel(d->character) > 10)
        SEND_TO_Q("4.    New  Thalos\n\r", d);
      if (GetMaxLevel(d->character) > 20)
        SEND_TO_Q("5.    The Gypsy Village\n\r", d);
      SEND_TO_Q("Your choice? ",d);
      STATE(d) = CON_CITY_CHOICE;
      break;

    default:
      SEND_TO_Q("Wrong option.\n\r", d);
      SEND_TO_Q(MENU, d);
      break;
    }
    break;

  case CON_PWDNEW:
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    
    if (!*arg || strlen(arg) > 10)      {
	write(d->descriptor, echo_on, 6);
	
	SEND_TO_Q("Illegal password.\n\r", d);
	SEND_TO_Q("Password: ", d);

	write(d->descriptor, echo_off, 4);
	
	
	return;
      }
    
    strncpy(d->pwd, crypt(arg, d->character->player.name), 10);
    *(d->pwd + 10) = '\0';
    write(d->descriptor, echo_on, 6);
    
    SEND_TO_Q("Please retype password: ", d);
    
    STATE(d) = CON_PWDNCNF;
    write(d->descriptor, echo_off, 4);
    
    
    break;
  case CON_PWDNCNF:
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    
    if (strncmp(crypt(arg, d->pwd), d->pwd, 10))      {
	  write(d->descriptor, echo_on, 6);
	  SEND_TO_Q("Passwords don't match.\n\r", d);
	  SEND_TO_Q("Retype password: ", d);
	  write(d->descriptor, echo_off, 4);
	  
	  STATE(d) = CON_PWDNEW;
	  return;
	}
    write(d->descriptor, echo_on, 6);
    
    SEND_TO_Q(
	      "\n\rDone. You must enter the game to make the change final\n\r",
	      d);
    SEND_TO_Q(MENU, d);
    STATE(d) = CON_SLCT;
    break;
  default:
    log("Nanny: illegal state of con'ness");
    abort();
    break;
  }
}
