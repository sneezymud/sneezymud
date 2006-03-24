
/* ************************************************************************
*  file: comm.c , Communication module.                   Part of DIKUMUD *
*  Usage: Communication, central game loop.                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
*  Using *any* part of DikuMud without having read license.doc is         *
*  violating our copyright.                                               *
************************************************************************* */

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"

#define DFLT_PORT 4000        /* default port */
#define PACKET_BUFFER_SIZE 40960
#define MAX_NAME_LENGTH 15
#define MAX_HOSTNAME   256
#define OPT_USEC 250000       /* time delay corresponding to 4 passes/sec */

#define STATE(d) ((d)->connected)

extern int errno;    /* Why isn't this done in errno.h on alfa??? */

/* externs */

/* extern struct char_data *character_list; */
#if HASH
extern struct hash_header room_db;    /* In db.c */
#else
extern struct room_data *room_db;     /* In db.c */
#endif

extern int top_of_world;            /* In db.c */
extern struct time_info_data time_info;  /* In db.c */
extern char help[];
extern struct char_data *character_list;

/* local globals */

struct descriptor_data *descriptor_list, *next_to_process;

int lawful = 0;      /* work like the game regulator */
int slow_death = 0;     /* Shut her down, Martha, she's sucking mud */
int Shutdown = 0;       /* clean shutdown */
int rebootmud = 0;         /* reboot the game after a shutdown */
int no_specials = 0;    /* Suppress ass. of special routines */
long Uptime;            /* time that the game has been up */

#if SITELOCK
char hostlist[MAX_BAN_HOSTS][30];  /* list of sites to ban           */
int numberhosts;
#endif

int maxdesc, avail_descs;
int tics = 0;        /* for extern checkpointing */

int get_from_q(struct txt_q *queue, char *dest);
/* write_to_q is in comm.h for the macro */
int run_the_game(int port);
int game_loop(int s);
int init_socket(int port);
int new_connection(int s);
int new_descriptor(int s);
int process_output(struct descriptor_data *t);
int process_input(struct descriptor_data *t);
void close_sockets(int s);
void close_socket(struct descriptor_data *d);
struct timeval timediff(struct timeval *a, struct timeval *b);
void flush_queues(struct descriptor_data *d);
void nonblock(int s);
void parse_name(struct descriptor_data *desc, char *arg);
void save_all();

/* extern fcnts */

struct char_data *make_char(char *name, struct descriptor_data *desc);
void boot_db(void);
void zone_update(void);
void affect_update( int pulse ); /* In spell_parser.c */
void free_char(struct char_data *ch);
void vlog(char *str);
void check_mobile_activity(int pulse);
void string_add(struct descriptor_data *d, char *str);
void perform_violence(int pulse);
void stop_fighting(struct char_data *ch);
void show_string(struct descriptor_data *d, char *input);
void gr(int s);
void station( void);
void down_river( int pulse );
void Teleport( int pulse );
void MakeSound();
void TeleportPulseStuff(int pulse);
void RiverPulseStuff(int pulse);

void check_reboot(void);

/* *********************************************************************
*                     main game loop and related stuff                 *
********************************************************************* */

int __main ()
{
  return(1);
}

int main (int argc, char **argv)
{
  int port, a, pos=1;
  char buf[512], *dir;

  extern int WizLock;
  struct rlimit rl;
  int res;
  
  port = DFLT_PORT;
  dir = DFLT_DIR;
/*
**  this block sets the max # of connections.  
**  # of files = 128, so #of players approx = 110
*/

#if DEBUG  
  malloc_debug(0);
#endif

  while ((pos < argc) && (*(argv[pos]) == '-')) {
     switch (*(argv[pos] + 1))  {
     case 'l':
        lawful = 1;
        vlog("Lawful mode selected.");
        break;
     case 'd':
        if (*(argv[pos] + 2))
           dir = argv[pos] + 2;
        else if (++pos < argc)
          dir = argv[pos];
      else     {
   vlog("Directory arg expected after option -d.");
   exit(0);
      }
      break;
    case 's':
      no_specials = 1;
      vlog("Suppressing assignment of special routines.");
      break;
    default:
      sprintf(buf, "Unknown option -% in argument string.",
         *(argv[pos] + 1));
      vlog(buf);
      break;
    }
    pos++;
  }
  
  if (pos < argc)
    if (!isdigit(*argv[pos]))       {
      fprintf(stderr, "Usage: %s [-l] [-s] [-d pathname] [ port # ]\n", 
         argv[0]);
      exit(0);
    }  else if ((port = atoi(argv[pos])) <= 1024)  {
      printf("Illegal port #\n");
      exit(0);
    }
  
  Uptime = time(0);
  
  sprintf(buf, "Running game on port %d.", port);
  vlog(buf);
  
  if (chdir(dir) < 0)   {
    perror("chdir");
    exit(0);
  }
  
  sprintf(buf, "Using %s as data directory.", dir);
  vlog(buf);
  
  srandom(time(0));
  WizLock = FALSE;

#if SITELOCK
  vlog("Blanking denied hosts.");
  for(a = 0 ; a<= MAX_BAN_HOSTS ; a++) 
    strcpy(hostlist[a]," \0\0\0\0");
  numberhosts = 0;
  strcpy( hostlist[0], "cerdm2.difi.unipi.it");
  numberhosts = 1;
  strcpy( hostlist[1], "iup.edu");
  numberhosts = 2;
#endif

  run_the_game(port);
  return(0);
}



#define PROFILE(x)


/* Init sockets, run game, and cleanup sockets */
int run_the_game(int port)
{
  int s; 
  PROFILE(extern etext();)
    
    void signal_setup(void);
  int load(void);
  void coma(int s);
  
  PROFILE(monstartup((int) 2, etext);)
    
    descriptor_list = NULL;
  
  vlog("Signal trapping.");
  signal_setup();
  
  vlog("Opening mother connection.");
  s = init_socket(port);
  
  if (lawful && load() >= 6)
    {
      vlog("System load too high at startup.");
      coma(s);
    }
  
  boot_db();
  
  vlog("Entering game loop.");
  
  game_loop(s);
  
  close_sockets(s); 
  
  PROFILE(monitor(0);)
    
    if (rebootmud)  {
   vlog("Rebooting.");
   exit(52);           /* what's so great about HHGTTG, anyhow? */
      }
  
  vlog("Normal termination of game.");
}






/* Accept new connects, relay commands, and call 'heartbeat-functs' */
int game_loop(int s)
{
  fd_set input_set, output_set, exc_set;
  struct timeval last_time, now, timespent, timeout, null_time;
  static struct timeval opt_time;
  char comm[MAX_INPUT_LENGTH];
  char promptbuf[80], movebuf[80];
  char buf[80], tempbuf[80], buf2[80];
  char hitscolor[10],manacolor[10],movescolor[10];
  struct descriptor_data *point, *next_point;
  int i,pulse = 0, mask, prompt_per;
  int current_hit,current_mana,current_moves;
  int missing_hit,missing_mana,missing_moves;
  struct room_data *rm;
  
  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  
  opt_time.tv_usec = OPT_USEC;  /* Init time values */
  opt_time.tv_sec = 0;
  gettimeofday(&last_time, (struct timezone *) 0);
  
  maxdesc = s;
  /* !! Change if more needed !! */
  avail_descs = 250;   /* urk a damn constant */
  
  mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
    sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
      sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP);
  
  /* Main loop */
  while (!Shutdown)  {
     /* Check what's happening out there */
     FD_ZERO(&input_set);
     FD_ZERO(&output_set);
     FD_ZERO(&exc_set);
     FD_SET(s, &input_set);
     for (point = descriptor_list; point; point = point->next)  {
         FD_SET(point->descriptor, &input_set);
         FD_SET(point->descriptor, &exc_set);
         FD_SET(point->descriptor, &output_set);
     }
    
     /* check out the time */
     gettimeofday(&now, (struct timezone *) 0);
     timespent = timediff(&now, &last_time);
     timeout = timediff(&opt_time, &timespent);
     last_time.tv_sec = now.tv_sec + timeout.tv_sec;
     last_time.tv_usec = now.tv_usec + timeout.tv_usec;
     if (last_time.tv_usec >= 1000000) {
        last_time.tv_usec -= 1000000;
        last_time.tv_sec++;
     }
    
     sigsetmask(mask);
    
     if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
        perror("Select poll");
        return(-1);
     }
    
     if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
        perror("Select sleep");
        /*exit(1);*/
     }
    
     sigsetmask(0);
    
     /* Respond to whatever might be happening */
    
     /* New connection? */
     if (FD_ISSET(s, &input_set))
        if (new_descriptor(s) < 0)
           perror("New connection");
    
    /* kick out the freaky folks */
    for (point = descriptor_list; point; point = next_point)  {
        next_point = point->next;   
        if (FD_ISSET(point->descriptor, &exc_set))  {
           FD_CLR(point->descriptor, &input_set);
           FD_CLR(point->descriptor, &output_set);
           close_socket(point);
        }
    }
    
    for (point = descriptor_list; point; point = next_point)  {
        next_point = point->next;
        if (FD_ISSET(point->descriptor, &input_set))
           if (process_input(point) < 0) 
              close_socket(point);
    }
    
    /* process_commands; */
    for (point = descriptor_list; point; point = next_to_process){
        next_to_process = point->next;
        if ((--(point->wait) <= 0) && get_from_q(&point->input, comm)) {
           if (point->character && point->connected == CON_PLYNG &&
                   point->character->specials.was_in_room != NOWHERE)  {
              char_from_room(point->character);
            if (point->character->specials.was_in_room == 3) 
              char_to_room(point->character,3001);
            else 
              char_to_room(point->character,point->character->specials.was_in_room);
              point->character->specials.was_in_room = NOWHERE;
              act("$n has returned.", TRUE, point->character, 0, 0, TO_ROOM);
           }
     
           point->wait = 1;
           if (point->character)
              point->character->specials.timer = 0;
           point->prompt_mode = 1;
     
           if (point->str)
              string_add(point, comm);
           else if (!point->connected) {
              if (point->showstr_point)
                 show_string(point, comm);
	      else if (point->pagedfile) 
	         page_file(point, comm);
              else
                 command_interpreter(point->character, comm);
           } else 
              nanny(point, comm); 
           if (point->position < 0) {    /* done with page_file output */
              if (point->pagedfile) {
                 free(point->pagedfile);
                 point->pagedfile = NULL;
              }
              point->position = 0;
           }
        }
    }
    
    /* give the people some prompts  */
    for (point = descriptor_list; point; point = point->next) 
      if ((FD_ISSET(point->descriptor, &output_set) && point->output.head) || (point->prompt_mode)) {
         if (!point->connected && point->character && !(IS_NPC(point->character)) &&  !(IS_SET(point->character->specials.act, PLR_COMPACT)))
             write_to_q("\n\r", &point->output); 
         if (point->str)
            write_to_q("] ", &point->output);
         else if (!point->connected)
            if (point->showstr_point || point->pagedfile)
               write_to_q("*** Press return ***", &point->output);
            else if ((IS_SET(point->character->specials.act, PLR_VT100)) ||
                     (IS_SET(point->character->specials.act, PLR_ANSI))) {
              SEND_TO_Q(VT_CURSAVE, point);
              sprintf(buf, VT_CURSPOS, point->screen_size-1, 1);
              SEND_TO_Q(buf,point);
              SEND_TO_Q("__________________________________________________________________________",point);
              sprintf(buf, VT_CURSPOS, point->screen_size, 1);
              SEND_TO_Q(buf, point);

    current_hit = 10*( (float) GET_HIT(point->character)/
                       (float) hit_limit(point->character));
    current_mana = 10*( (float) GET_MANA(point->character)/
                        (float) GET_MAX_MANA(point->character));
    current_moves = 10*( (float) GET_MOVE(point->character)/
                         (float) GET_MAX_MOVE(point->character));
    if (current_hit < 0) current_hit = 0;
    if (current_mana < 0) current_mana = 0;
    if (current_moves < 0) current_moves = 0;
    if (current_hit > 10) current_hit = 10;
    if (current_mana > 10) current_mana = 10;
    if (current_moves > 10) current_moves = 10;
    missing_hit = 10-current_hit;
    missing_mana = 10-current_mana;
    missing_moves = 10-current_moves;


           if ((current_hit <= 2)  &&
               IS_SET(point->character->specials.act, PLR_ANSI))
           SEND_TO_Q(ANSI_RED,point);
           else
           SEND_TO_Q(VT_BOLDTEX,point);
           if (IS_SET(point->character->specials.act, PLR_VT100))
           sprintf(promptbuf,"Hit Points:%-10d",point->character->points.hit);
           else
           sprintf(promptbuf,"Hits:%-6d",point->character->points.hit);
            SEND_TO_Q(promptbuf,point);
           if (IS_SET(point->character->specials.act, PLR_ANSI)) {
             SEND_TO_Q(ANSI_BLUE,point);
             for (i=1;i<=current_hit;i++) {
              SEND_TO_Q(VT_GRAPHBR,point);
              }
           }
           if (IS_SET(point->character->specials.act, PLR_ANSI)) {
              SEND_TO_Q(ANSI_CYAN,point);
              for (i=1;i<=missing_hit;i++) {
                 SEND_TO_Q(VT_GRAPHBR,point);
              }
            }
            if ((current_mana <= 2) &&
               IS_SET(point->character->specials.act, PLR_ANSI))
            SEND_TO_Q(ANSI_RED,point);
            else
            SEND_TO_Q(VT_BOLDTEX,point);
            if (IS_SET(point->character->specials.act, PLR_ANSI))
            sprintf(buf2, "  Mana:%-6d", point->character->points.mana);
            else
            sprintf(buf2,"  Mana Points:%-10d", point->character->points.mana);
            SEND_TO_Q(buf2,point);
            if (IS_SET(point->character->specials.act, PLR_ANSI))  { 
             SEND_TO_Q(ANSI_BLUE,point);
             for (i=1;i<=current_mana;i++) {
              SEND_TO_Q(VT_GRAPHBR,point);
             }
            }
            if (IS_SET(point->character->specials.act, PLR_ANSI)) {
             SEND_TO_Q(ANSI_CYAN,point);
             for (i=1;i<=missing_mana;i++) {
               SEND_TO_Q(VT_GRAPHBR,point);
             }
            }
            SEND_TO_Q(VT_BOLDTEX,point);
            sprintf(movebuf, "  Vitality:%-6d",point->character->points.move);
            SEND_TO_Q(movebuf,point);
            if (IS_SET(point->character->specials.act, PLR_ANSI)) {
             SEND_TO_Q(ANSI_BLUE,point);
             for (i=1;i<=current_moves;i++) {
              SEND_TO_Q(VT_GRAPHBR,point);
             }
            }
            if (IS_SET(point->character->specials.act, PLR_ANSI)) {
             SEND_TO_Q(ANSI_CYAN,point);
             for (i=1;i<=missing_moves;i++) {
               SEND_TO_Q(VT_GRAPHBR,point);
             }
            }
              SEND_TO_Q(VT_NORMALT, point);
              SEND_TO_Q(VT_CURREST, point);
           if (point->prompt) {
              sprintf(buf, point->prompt);
              strcat(buf, " > ");
           } else
              sprintf(buf, "> ");
              SEND_TO_Q(buf,point);
            } else { 
               if ( IS_SET ( point->character->specials.act, PLR_COLOR ) ) {
                  if (IS_IMMORTAL(point->character)) {
                     rm = real_roomp(point->character->in_room);
                     sprintf(promptbuf,"%sH:%d %sR:%d>%s ",
                     ANSI_CYAN,
                     point->character->points.hit,
                     ANSI_VIOLET,
                     rm->number,
                     ANSI_WHITE );
                     write_to_q(promptbuf, &point->output);
                  } else if (HasClass(point->character, CLASS_MAGIC_USER) || 
                             HasClass(point->character, CLASS_ANTIPALADIN) ||
                             HasClass(point->character, CLASS_PALADIN) ||
                             HasClass(point->character, CLASS_RANGER) ||
                             HasClass(point->character, CLASS_CLERIC))  {
                      prompt_per = point->character->points.hit * 100 /
                              point->character->points.max_hit;
                      if ( prompt_per < 20 ) {
                         strcpy ( hitscolor, ANSI_RED );
                      } else {
                         strcpy ( hitscolor, ANSI_GREEN );
                      }
                      prompt_per = point->character->points.mana * 100 /
                      GET_MAX_MANA(point->character);
                      if ( prompt_per < 20 ) {
                         strcpy ( manacolor, ANSI_RED );
                      } else {
                         strcpy ( manacolor, ANSI_VIOLET );
                      }
                      prompt_per = point->character->points.move * 100 /
                      GET_MAX_MOVE(point->character);
                      if ( prompt_per < 20 ) {
                         strcpy ( movescolor, ANSI_RED );
                      } else {
                        strcpy ( movescolor, ANSI_CYAN );
                      }
                      sprintf(promptbuf,"H:%s%d %sM:%s%d %sV:%s%d%s>%s ",
                              hitscolor,
                              point->character->points.hit,
                              ANSI_WHITE,
                              manacolor,
                              point->character->points.mana,
                              ANSI_WHITE,
                              movescolor,
                              point->character->points.move,
                              ANSI_WHITE,
                              ANSI_NORMAL );
                      write_to_q(promptbuf, &point->output);
                  } else if (HasClass(point->character,CLASS_THIEF) || 
                             HasClass(point->character,CLASS_WARRIOR) || 
                             HasClass(point->character,CLASS_MONK)) {
                      prompt_per = point->character->points.hit * 100 /
                                   point->character->points.max_hit;
                      if ( prompt_per < 20 ) {
                         strcpy ( hitscolor, ANSI_RED );
                      } else {
                         strcpy ( hitscolor, ANSI_GREEN );
                      }
                      prompt_per = point->character->points.move * 100 /
                      GET_MAX_MOVE(point->character);
                      if ( prompt_per < 20 ) {
                         strcpy ( movescolor, ANSI_RED );
                      } else {
                         strcpy ( movescolor, ANSI_CYAN );
                      }
                      sprintf(promptbuf,"H:%s%d %sV:%s%d%s>%s ",
                              hitscolor,
                              point->character->points.hit,
                              ANSI_WHITE,
                              movescolor,
                              point->character->points.move,
                              ANSI_WHITE,
                              ANSI_NORMAL );
                      write_to_q(promptbuf, &point->output);
                  } else {
                      prompt_per = point->character->points.hit * 100 /
                                   point->character->points.max_hit;
                      if ( prompt_per < 20 ) {
                         strcpy ( hitscolor, ANSI_RED );
                      } else {
                         strcpy ( hitscolor, ANSI_GREEN );
                      }
                      prompt_per = point->character->points.move * 100 /
                                   point->character->points.max_move;
                      if ( prompt_per < 20 ) {
                         strcpy ( movescolor, ANSI_RED );
                      } else {
                         strcpy ( movescolor, ANSI_CYAN );
                      }
                      sprintf(promptbuf,"*H:%s%d %sV:%s%d%s>%s ",
                              hitscolor,
                              point->character->points.hit,
                              ANSI_WHITE,
                              movescolor,
                              point->character->points.move,
                              ANSI_WHITE,
                              ANSI_NORMAL );
                      write_to_q(promptbuf, &point->output);
                 }
              } else {
       if (GetMaxLevel(point->character) > 50) {
         rm = real_roomp(point->character->in_room);
         sprintf(promptbuf,"H:%d R:%d> ",
            point->character->points.hit,
            rm->number);
         write_to_q(promptbuf, &point->output);
       } else if (HasClass(point->character, CLASS_MAGIC_USER) || 
                       HasClass(point->character, CLASS_ANTIPALADIN) ||
                       HasClass(point->character, CLASS_PALADIN) ||
                       HasClass(point->character, CLASS_RANGER) ||
             HasClass(point->character, CLASS_CLERIC))  {
         sprintf(promptbuf,"H:%d M:%d V:%d> ",
            point->character->points.hit,
            point->character->points.mana,
            point->character->points.move);
              write_to_q(promptbuf, &point->output);
       } else if (HasClass(point->character,CLASS_THIEF) || 
             HasClass(point->character,CLASS_WARRIOR) ||
                       HasClass(point->character,CLASS_MONK)) {
         sprintf(promptbuf,"H:%d V:%d> ",
            point->character->points.hit,
            point->character->points.move);
         write_to_q(promptbuf, &point->output);
       } else {
         sprintf(promptbuf,"*H:%d V:%d> ",
            point->character->points.hit,
            point->character->points.move);
              write_to_q(promptbuf, &point->output);
       }
      }
     }
      }  
      for (point = descriptor_list; point; point = next_point) {
          next_point = point->next;
          if (FD_ISSET(point->descriptor, &output_set) && point->output.head)
             if (process_output(point) < 0)
                close_socket(point);
          point->prompt_mode = 0;
      }
    
    /* handle heartbeat stuff */
    /* Note: pulse now changes every 1/4 sec  */
    
    pulse++;
    
    if (!(pulse % PULSE_ZONE))  {
      zone_update();
      if (lawful)
         gr(s);
    }
    
    
    if (!(pulse % PULSE_RIVER)) {
      RiverPulseStuff(pulse);
    }
    
    if (!(pulse % PULSE_TELEPORT)) {
      TeleportPulseStuff(pulse);
    }
    
    if (!(pulse % PULSE_VIOLENCE))
      perform_violence( pulse );
    
    /*
      if (!(pulse % PULSE_FALL))
      all_fall_down();
      
      if (!(pulse % PULSE_DROWN))
      glug_glug_glug();
    */
    
    if (!(pulse % (SECS_PER_MUD_HOUR*4))){
      weather_and_time(1);
      affect_update(pulse);  /* things have been sped up by combining */
      if ( time_info.hours == 1 )
         update_time();
         station();

    }
    
    if (pulse >= 2400) {
      pulse = 0;
      if (lawful)
   night_watchman();
      check_reboot();
    }
    
    tics++;        /* tics since last checkpoint signal */
  }
}






/* ******************************************************************
*  general utility stuff (for local use)                           *
****************************************************************** */




int get_from_q(struct txt_q *queue, char *dest)
{
   struct txt_block *tmp;

   /* Q empty? */
   if (!queue->head)
      return(0);

   tmp = queue->head;
   strcpy(dest, queue->head->text);
   queue->head = queue->head->next;

   free(tmp->text);
   free(tmp);

   return(1);
}




void write_to_q(char *txt, struct txt_q *queue)
{
   struct txt_block *new;

        if (!queue) {
     vlog("Output message to non-existant queue");
     return;
   }

   CREATE(new, struct txt_block, 1);
   CREATE(new->text, char, strlen(txt) + 1);

   strcpy(new->text, txt);

         new->next = NULL;

   /* Q empty? */
   if (!queue->head)  {
      queue->head = queue->tail = new;
   } else   {
      queue->tail->next = new;
      queue->tail = new;
   }
}
      






struct timeval timediff(struct timeval *a, struct timeval *b)
{
   struct timeval rslt, tmp;

   tmp = *a;

   if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0)
   {
      rslt.tv_usec += 1000000;
      --(tmp.tv_sec);
   }
   if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0)
   {
      rslt.tv_usec = 0;
      rslt.tv_sec =0;
   }
   return(rslt);
}






/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
   char dummy[MAX_STRING_LENGTH];

   while (get_from_q(&d->output, dummy));
   while (get_from_q(&d->input, dummy));
}






/* ******************************************************************
*  socket handling                      *
****************************************************************** */




int init_socket(int port)
{
   int s;
   char *opt;
   char hostname[MAX_HOSTNAME+1];
   struct sockaddr_in sa;
   struct hostent *hp;
   struct linger ld;

   bzero(&sa, sizeof(struct sockaddr_in));
   gethostname(hostname, MAX_HOSTNAME);
   hp = gethostbyname(hostname);
   if (hp == NULL)
   {
      perror("gethostbyname");
      exit(1);
   }
   sa.sin_family = hp->h_addrtype;
   sa.sin_port = htons(port);
   s = socket(AF_INET, SOCK_STREAM, 0);
   if (s < 0) 
   {
      perror("Init-socket");
      exit(1);
   }
   if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR,
      (char *) &opt, sizeof (opt)) < 0) 
   {
      perror ("setsockopt REUSEADDR");
      exit (1);
   }

   ld.l_onoff = 1;
   ld.l_linger = 1000;
   if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0)
   {
      perror("setsockopt LINGER");
      exit(1);
   }
   if (bind(s, &sa, sizeof(sa)) < 0)
   {
      perror("bind");
      close(s);
      exit(1);
   }
   listen(s, 5);
   return(s);
}





int new_connection(int s)
{
   struct sockaddr_in isa;
   /* struct sockaddr peer; */
   int i;
   int t;
   char buf[100];

   i = sizeof(isa);
   getsockname(s, &isa, &i);


   if ((t = accept(s, &isa, &i)) < 0)
   {
      perror("Accept");
      return(-1);
   }
   nonblock(t);

   /*

   i = sizeof(peer);
   if (!getpeername(t, &peer, &i))
   {
      *(peer.sa_data + 49) = '\0';
      sprintf(buf, "New connection from addr %s.\n", peer.sa_data);
      vlog(buf);
   }

   */

   return(t);
}



/* print an internet host address prettily */
static void printhost(addr, buf)
     struct in_addr  *addr;
     char   *buf;
{
  struct hostent  *h;
  char   *s;
  int n1, n2,n3,n4;

  h = gethostbyaddr((const char *) addr, sizeof(*addr),AF_INET);
  s = (h==NULL) ? NULL : h->h_name;

  if (s) {
    strcpy(buf, s);
  } else {
    n1 = addr->s_addr >> 24;     
    n2 = (addr->s_addr >> 16) - (n1 * 256);
    n3 = (addr->s_addr >> 8) - (n1 * 65536) - (n2 * 256);
    n4 = (addr->s_addr) % 256;
    
    sprintf(buf, "%d.%d.%d.%d", n1, n2,n3,n4);
  }
}


/* print an internet host address prettily */
static void printhostaddr(addr, buf)
     struct in_addr  *addr;
     char   *buf;
{
  struct hostent  *h;
  char   *s;
  int n1,n2,n3,n4;

  h = gethostbyaddr((const char *) addr, sizeof(*addr),AF_INET);
  s = (h==NULL) ? NULL : h->h_name;
  n1 = addr->s_addr >> 24;           
  n2 = (addr->s_addr >> 16) - (n1 * 256);
  n3 = (addr->s_addr >> 8) - (n1 * 65536) - (n2 * 256); 
  n4 = (addr->s_addr) % 256;

  sprintf(buf, "%d.%d.%d.%d", n1, n2,n3,n4);
}

int new_descriptor(int s)
{
   int desc, a, size;
   struct descriptor_data *newd;
   struct sockaddr_in sock;
   struct hostent *from;
   char buf[100],tempbuf[255];
   char *temphost[255], *temphostaddr[255];

   if ((desc = new_connection(s)) < 0)
      return (-1);

   if ((maxdesc + 1) >= avail_descs)   {
     sprintf(tempbuf,"Debug Info: maxdesc=%d,avail descs=%d,desc=%d\n\r",maxdesc,avail_descs,desc);
     write_to_descriptor(desc,tempbuf);
     write_to_descriptor(desc, "Sorry.. The game is full...\n\r");
     close(desc);
     return(0);
   }
   else
     if (desc > maxdesc)
       maxdesc = desc;
   
   CREATE(newd, struct descriptor_data, 1);
   
   /* find info */
   size = sizeof(sock);
   if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0)   {
     perror("getpeername");
      *newd->host = '\0';
   } else {
      printhost(&sock.sin_addr, newd->host);
#if SITELOCK
      printhost(&sock.sin_addr, temphost);
      printhostaddr(&sock.sin_addr, temphostaddr);
      if (numberhosts != 0)
        for(a = 0 ; a <= numberhosts-1 ; a++) {
          fprintf(stderr,"*HOST*|%s|%s|\n",temphost,hostlist[a]);
          if (isdigit(hostlist[a][0])) {        
                 if (strstr((const char *) temphostaddr,hostlist[a]) != NULL) {
         write_to_descriptor(desc,"Sorry, the game is locked.");
         close(desc);
         return(0);
       }
          } else {
       if (strcasestr(temphost, hostlist[a])) {
         write_to_descriptor(desc,"Sorry, the game is locked.");
         close(desc);
         return(0);
       }
          } 
        }
#endif
      
    } 
   
   /* init desc data */
   newd->descriptor = desc;
   newd->connected  = 1;
   newd->wait = 1;
   newd->prompt_mode = 0;
   *newd->buf = '\0';
   newd->str = 0;
   newd->showstr_head = 0;
   newd->showstr_point = 0;
   newd->pagedfile = NULL;
   newd->position = 0;
   *newd->last_input= '\0';
   newd->output.head = NULL;
   newd->input.head = NULL;
   newd->next = descriptor_list;
   newd->character = 0;
   newd->original = 0;
   newd->snoop.snooping = 0;
   newd->snoop.snoop_by = 0;

   /* prepend to list */

   descriptor_list = newd;

    SEND_TO_Q(GREETINGS,newd);
    SEND_TO_Q("\n\rWhat is your name? :", newd);

   return(0);
}
   




int process_output(struct descriptor_data *t)
{       
   char i[MAX_STRING_LENGTH + 1];
        static char buffer[PACKET_BUFFER_SIZE];
        char *end_buf;
        int length;

        end_buf = buffer;
        
   if (!(t->prompt_mode) && !(t->connected)) {
           memcpy(end_buf, "\n\r", strlen("\n\r"));
            end_buf += strlen("\n\r");  
        }

   while (get_from_q(&t->output, i))   {  
      if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)) {
         write_to_q("% ",&t->snoop.snoop_by->desc->output);
         write_to_q(i,&t->snoop.snoop_by->desc->output);
      }
           length = strlen(i);
           if ((length + end_buf + 1) > (buffer + PACKET_BUFFER_SIZE)) {
              *end_buf = '\0';   
         if (write_to_descriptor(t->descriptor, buffer))
                 return (-1); 
              end_buf = buffer;  
           }
           memcpy(end_buf, i, length);
           end_buf += length;
   }
   
        *end_buf = '\0';      
        if (end_buf != buffer)      
           if (write_to_descriptor(t->descriptor, buffer) < 0)
         return(-1);

   return 1;      
}


int write_to_descriptor(int desc, char *txt)
{
  int sofar, thisround, total;
  
  total = strlen(txt);
  sofar = 0;
  
  do
    {
      thisround = write(desc, txt + sofar, total - sofar);
      if (thisround < 0)
   {
     if (errno == EWOULDBLOCK)
       break;
     perror("Write to socket");
     return(-1);
   }
      sofar += thisround;
    } 
  while (sofar < total);
  
  return(0);
}





int process_input(struct descriptor_data *t)
{
  int sofar, thisround, begin, squelch, i, k, flag;
  char tmp[MAX_INPUT_LENGTH+2], buffer[MAX_INPUT_LENGTH + 60];
  
  sofar = 0;
  flag = 0;
  begin = strlen(t->buf);
  
  /* Read in some stuff */
  do  {
    if ((thisround = read(t->descriptor, t->buf + begin + sofar, 
           MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0) {
      sofar += thisround;
    } else {
      if (thisround < 0) {
   if (errno != EWOULDBLOCK) {
     perror("Read1 - ERROR");
     return(-1);
   } else {
     break;
   }
      } else {
   vlog("EOF encountered on socket read.");
   return(-1);
      }
    }
  } while (!ISNEWL(*(t->buf + begin + sofar - 1)));   
  
  *(t->buf + begin + sofar) = 0;
  
  /* if no newline is contained in input, return without proc'ing */
  for (i = begin; !ISNEWL(*(t->buf + i)); i++)
    if (!*(t->buf + i))
      return(0);
  
  /* input contains 1 or more newlines; process the stuff */
  for (i = 0, k = 0; *(t->buf + i);)   {
    if (!ISNEWL(*(t->buf + i)) && !(flag=(k>=(MAX_INPUT_LENGTH - 2))))
      if (*(t->buf + i) == '\b') {   /* backspace */
   if (k) { /* more than one char ? */
     if (*(tmp + --k) == '$')
       k--;          
     i++;
   } else {
     i++;  /* no or just one char.. Skip backsp */
   }
      } else {
   if (isascii(*(t->buf + i)) && isprint(*(t->buf + i))) {
     /* 
       trans char, double for '$' (printf)   
       */
     if ((*(tmp + k) = *(t->buf + i)) == '$')
       *(tmp + ++k) = '$';
     k++;
     i++;
   } else {
     i++;
   }
      } else   {
   *(tmp + k) = 0;
   if(*tmp == '!')
     strcpy(tmp,t->last_input);
   else
     strcpy(t->last_input,tmp);
   
   write_to_q(tmp, &t->input);
   
   if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)){
     write_to_q("% ",&t->snoop.snoop_by->desc->output);
     write_to_q(tmp,&t->snoop.snoop_by->desc->output);
     write_to_q("\n\r",&t->snoop.snoop_by->desc->output);
   }
   
   if (flag) {
     sprintf(buffer, 
        "Line too long. Truncated to:\n\r%s\n\r", tmp);
     if (write_to_descriptor(t->descriptor, buffer) < 0)
       return(-1);
     
     /* skip the rest of the line */
     for (; !ISNEWL(*(t->buf + i)); i++);
   }
   
   /* find end of entry */
   for (; ISNEWL(*(t->buf + i)); i++);
   
   /* squelch the entry from the buffer */
   for (squelch = 0;; squelch++)
     if ((*(t->buf + squelch) = 
          *(t->buf + i + squelch)) == '\0')
       break;
   k = 0;
   i = 0;
      }
  }
  return(1);
}




void close_sockets(int s)
{
   vlog("Closing all sockets.");

   while (descriptor_list)
      close_socket(descriptor_list);

   close(s);
}





void close_socket(struct descriptor_data *d)
{
  struct descriptor_data *tmp;
  char buf[100];

  void do_save(struct char_data *ch, char *argument, int cmd);
  
  if (!d) return;
  
  close(d->descriptor);
  flush_queues(d);
  if (d->descriptor == maxdesc)
    --maxdesc;
  
  /* Forget snooping */
  if (d->snoop.snooping)
    d->snoop.snooping->desc->snoop.snoop_by = 0;
  
  if (d->snoop.snoop_by)      {
    send_to_char("Your victim is no longer among us.\n\r",d->snoop.snoop_by);
    d->snoop.snoop_by->desc->snoop.snooping = 0;
  }
  
  if (d->character)
    if (d->connected == CON_PLYNG)  {
       do_save(d->character, "", 0);
      act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
      sprintf(buf, "Closing link to: %s.", GET_NAME(d->character));
      vlog(buf);
      if (IS_NPC(d->character)) {
   if (d->character->desc)
     d->character->orig = d->character->desc->original;
      }
      d->character->desc = 0;
      d->character->invis_level = LOW_IMMORTAL;  /* set the invis level to LOW_IMMORTAL */
    } else {
    //  if (GET_NAME(d->character)) {
   //sprintf(buf, "Losing player: %s [%s].",GET_NAME(d->character),d->host);
   //log(buf);
      //}
      free_char(d->character);
    }
  
  
  if (next_to_process == d)      /* to avoid crashing the process loop */
    next_to_process = next_to_process->next;   
  
  if (d == descriptor_list) /* this is the head of the list */
    descriptor_list = descriptor_list->next;
  else  /* This is somewhere inside the list */
    {
      /* Locate the previous element */
      for (tmp = descriptor_list; (tmp->next != d) && tmp; 
      tmp = tmp->next);
      
      tmp->next = d->next;
    }
  if (d->showstr_head)
    free(d->showstr_head);
  if (d->pagedfile)
    free(d->pagedfile);
  free(d);
}





void nonblock(int s)
{
   if (fcntl(s, F_SETFL, FNDELAY) == -1)
   {
      perror("Noblock");
      exit(1);
   }
}




#define COMA_SIGN \
"\n\r\
DikuMUD is currently inactive due to excessive load on the host machine.\n\r\
Please try again later.\n\r\n\
\n\r\
   Sadly,\n\r\
\n\r\
    the DikuMUD system operators\n\r\n\r"


/* sleep while the load is too high */
void coma(int s)
{
   fd_set input_set;
   static struct timeval timeout =
   {
      60, 
      0
   };
   int conn;

   int workhours(void);
   int load(void);

   vlog("Entering comatose state.");

   sigsetmask(sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
      sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
      sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP));


   while (descriptor_list)
      close_socket(descriptor_list);

   FD_ZERO(&input_set);
   do
   {
      FD_SET(s, &input_set);
      if (select(64, &input_set, 0, 0, &timeout) < 0)
      {
         perror("coma select");
         exit(1);
      }
      if (FD_ISSET(s, &input_set))
      {
         if (load() < 6)
         {
            vlog("Leaving coma with visitor.");
            sigsetmask(0);
            return;
         }
         if ((conn = new_connection(s)) >= 0)
         {
            write_to_descriptor(conn, COMA_SIGN);
            sleep(2);
            close(conn);
         }
      }        

      tics = 1;
      if (workhours())
      {
         vlog("Working hours collision during coma. Exit.");
         exit(0);
      }
   }
   while (load() >= 6);

   vlog("Leaving coma.");
   sigsetmask(0);
}



/* ****************************************************************
*  Public routines for system-to-player-communication   *
**************************************************************** */



void send_to_char(char *messg, struct char_data *ch)
{
  if (ch)
     if (ch->desc && messg) {
         write_to_q(messg, &ch->desc->output);
     }
}


void save_all()
{
   struct descriptor_data *i;

         for (i = descriptor_list; i; i = i->next)
     if (i->character)
             save_char(i->character,AUTO_RENT);
}

void send_to_all(char *messg)
{
   struct descriptor_data *i;

   if (messg)
      for (i = descriptor_list; i; i = i->next)
         if (!i->connected)
            write_to_q(messg, &i->output);
}


void send_to_outdoor(char *messg)
{
   struct descriptor_data *i;

   if (messg)
      for (i = descriptor_list; i; i = i->next)
         if (!i->connected)
            if (OUTSIDE(i->character))
               write_to_q(messg, &i->output);
}


void send_to_except(char *messg, struct char_data *ch)
{
   struct descriptor_data *i;

   if (messg)
      for (i = descriptor_list; i; i = i->next)
         if (ch->desc != i && !i->connected)
            write_to_q(messg, &i->output);
}



void send_to_room(char *messg, int room)
{
   struct char_data *i;
   
   if (messg)
     for (i = real_roomp(room)->people; i; i = i->next_in_room)
       if (i->desc)
         write_to_q(messg, &i->desc->output);
      }




void send_to_room_except(char *messg, int room, struct char_data *ch)
{
  struct char_data *i;
  
  if (messg)
    for (i = real_roomp(room)->people; i; i = i->next_in_room)
      if (i != ch && i->desc)
   write_to_q(messg, &i->desc->output);
}

void send_to_room_except_two
  (char *messg, int room, struct char_data *ch1, struct char_data *ch2)
{
  struct char_data *i;
  
  if (messg)
    for (i = real_roomp(room)->people; i; i = i->next_in_room)
      if (i != ch1 && i != ch2 && i->desc)
   write_to_q(messg, &i->desc->output);
}



/* higher-level communication */


void act(char *str, int hide_invisible, struct char_data *ch,
    struct obj_data *obj, void *vict_obj, int type)
{
  register char *strp, *point, *i;
  struct char_data *to, *tmp_victim, *temp;
  char buf[MAX_STRING_LENGTH];
  
  if (!str)
    return;
  if (!*str)
    return;
  
  if (ch->in_room <= -1)
    return;  /* can't do it. in room -1 */
  
  if (type == TO_VICT)
    to = (struct char_data *) vict_obj;
  else if (type == TO_CHAR)
    to = ch;
  else 
    to = real_roomp(ch->in_room)->people;
  
  for (; to; to = to->next_in_room)     {
    if (to->desc && ((to != ch) || (type == TO_CHAR)) &&  
        (CAN_SEE(to, ch) || !hide_invisible) && 
        !((type == TO_NOTVICT)&&(to==(struct char_data *) vict_obj))){
      for (strp = str, point = buf;;)
   if (*strp == '$') {
     switch (*(++strp)) {
     case 'n': i = PERS(ch, to); 
       break;
     case 'N': i = PERS((struct char_data *) vict_obj, to); 
       break;
     case 'm': i = HMHR(ch); 
       break;
     case 'M': i = HMHR((struct char_data *) vict_obj); 
       break;
     case 's': i = HSHR(ch); 
       break;
     case 'S': i = HSHR((struct char_data *) vict_obj); 
       break;
     case 'e': i = HSSH(ch); 
       break;
     case 'E': i = HSSH((struct char_data *) vict_obj); 
       break;
     case 'o': i = OBJN(obj, to); 
       break;
     case 'O': i = OBJN((struct obj_data *) vict_obj, to); 
       break;
     case 'p': i = OBJS(obj, to); 
       break;
     case 'P': i = OBJS((struct obj_data *) vict_obj, to); 
       break;
     case 'a': i = SANA(obj); 
       break;
     case 'A': i = SANA((struct obj_data *) vict_obj); 
       break;
     case 'T': i = (char *) vict_obj; 
       break;
     case 'F': i = fname((char *) vict_obj); 
       break;
     case '$': i = "$"; 
       break;
     default:
       vlog("Illegal $-code to act():");
       vlog(str);
       break;
     }
     
     while (*point = *(i++))
       ++point;
     
     ++strp;
     
   }  else if (!(*(point++) = *(strp++)))
     break;
      
      *(--point) = '\n';
      *(++point) = '\r';
      *(++point) = '\0';
      
      write_to_q(CAP(buf), &to->desc->output);
    }
    if ((type == TO_VICT) || (type == TO_CHAR))
      return;
  }
}
