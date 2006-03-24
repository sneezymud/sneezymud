/***************************************************************

This module was added by will hold all the special routines for the
crap table.

R. Russell

***************************************************************/

#include <string.h>
#include "structs.h"
#include "spells.h"
#include "utils.h"

#define CRAP_TABLE 8405
#define MAX_CRAPS 10

struct crap_players {
       char inuse;
       char name[40];
       int bet;
};

static struct crap_players crap_data[MAX_CRAPS];      

int check_craps( struct char_data *ch )
{
   
    if ( ch->in_room == CRAPS )
       return 1;
    else 
       return 0;

}

int do_craps_enter( struct char_data *ch )
{

  int l1, l2, inx;
  extern struct time_info_data time_info;

  for (l1=0,inx=-1;l1<MAX_CRAPS;l1++)
  {
      if( !strcmp(ch->player.name, crap_data[l1].name) )
      {
         inx=l1;
         send_to_char ( "The tableman say, "Ah, you have returned.'\n\r",ch);
      }
      if ( inx < 0 && !crap_data[l1].inuse )
             inx=l1;
  }
  if ( inx < 0 )
  {
      send_to_char( "The table seems to be full.\n\r", ch);
      return 0;
  }

  srand ((time_info.hours * time_info.day) % (int)(ch));
  send_to_char ( "You move up to the craps table.\n\r", ch);
  crap_data[inx].inuse = 1;
  strcpy ( crap_data[inx].name, ch->player.name );
  crap_data[inx].bet = 0;

  return 1;
}

int do_craps_exit ( struct char_data *ch )
{
      char log_msg[80];
      int inx;

      inx = crap_index ( ch );
      if ( inx < 0 )
      {
           sprintf ( log_msg "%s has left a table he was not at!",
                    ch->player.name );
           log (log_msg);
           return 0;
      }

      crap_data[inx].name[0]=0;
      crap_data[inx].inuse=0

      send_to_char ( "You leave the craps table.\n\r", ch);
}


