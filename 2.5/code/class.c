#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"
#include "limits.h"
#include "opinion.h"
#include "race.h"

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct dex_app_type dex_app[];



int GetClassLevel(struct char_data *ch, int class)
{

  if (IS_SET(ch->player.class, class)) {
    return(GET_LEVEL(ch, CountBits(class)-1));
  }
  return(0);
}

int CountBits(int class)
{

  if (class == 1) return(1);
  if (class == 2) return(2);
  if (class == 4) return(3);
  if (class == 8) return(4);

}

int OnlyClass( struct char_data *ch, int class)
{
  int i;

  for (i=1;i<=8; i*=2) {
    if (GetClassLevel(ch, i) != 0)
      if (i != class)
	return(FALSE);
  }
  return(TRUE);

}


int HasClass(struct char_data *ch, int class)
{

#if 0
  if (IS_SET(class, ch->player.class))
     return(TRUE);
#endif

  if (class == ch->player.class)
    return(TRUE);

  return FALSE;
}
