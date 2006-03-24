/**************************************************************************
*  file: syntax_check.c , Check syntax of all files       Part of DIKUMUD *
*  Usage: syntax_check <file prefix> <start room>                         *
*  Heavily modified by Benedict (bls@sector7g.eng.sun.com)                *
*  compile with -DOLD_DIKU if you're using the original diku file format  *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */
FILE *mob_f,                     /* file containing mob prototypes  */
*obj_f,                          /* obj prototypes                  */
*wld_f,                          /* World file                      */
*zon_f;

struct edge {
  bool exists;
  int dest;
};

struct room_graph {
  int vnum;
  struct edge way_out[6];
  bool flag;
  bool processed;
};

struct room_graph *world;

struct index_data *mob_index;         /* index table for mobile file     */
struct index_data *obj_index;         /* index table for object file     */
struct index_data *wld_index;

int top_of_mobt = 0;                  /* top of mobile index table       */
int top_of_objt = 0;                  /* top of object index table       */
int top_of_wldt = 0;
int firstroom;

struct time_data time_info;    /* the infomation about the time   */

/* local procedures */
void setup_dir(FILE *fl, int room, int dir, int rnum);
struct index_data *generate_indices(FILE *fl, int *top);

void assume(int faktisk, int antal, int place, char *errmsg)
{
  if (antal != faktisk) {
    printf("Error has occured at #%d.\n", place);
    printf("Message is : %s\n", errmsg);
    printf("Actual number read is %d\n", faktisk);
    exit(1);
  }
}


/* generate index table for object, monster or world file*/
struct index_data *generate_indices(FILE *fl, int *top)
{
  int i = 0, antal;
  struct index_data *index;
  long pos;
  char buf[82];

  rewind(fl);

  for (;;) {
    if (fgets(buf, 81, fl)) {
      if (*buf == '#') {
        if (i == 0)
          CREATE(index, struct index_data, 1);
        else if ((index = (struct index_data*) realloc(index,
                 (i + 1) * sizeof(struct index_data))) == 0) {
          printf("load indices");
          exit(1);
        }
        antal = sscanf(buf, "#%d", &index[i].virtual);
        assume(antal, 1, index[i].virtual, "Next string with E/A/$");

        index[i].pos = ftell(fl);
        index[i].number = index[i].virtual;
        index[i].func = 0;
        i++;
      } else if (*buf == '$')  /* EOF */
        break;
    } else {
      printf("Error when generating index, based upon #xxxx numbers.\n");
      printf("   Probably error at end of file.\n");

      exit(1);
    }
  }
  index[i-1].number = -1;
  *top = i - 1;
  return(index);
}

int exist_index(char *type, struct index_data *index_list, int top, int num)
{
  int i, found;

  for (found = FALSE, i = 0; i <= top && !found; i++)
    if (index_list[i].number == num)
      found = TRUE;

  if (!found) {
    printf("Lack of existence of %s #%d\n", type, num);
  }

  return (found);
}


int rnum_from_vnum(struct index_data *p, int last, int vnum)
{
  int bot = 0;
  int top = last;
  int current;

  for (;;) {
    current = (top+bot)/2;
    if (p[current].number == vnum) return current;
    else if (bot >= top) return -1;
    else if (p[current].number > vnum) top = current - 1;
    else                               bot = current + 1;
  }
}



void
init_node(int room_nr, int vnum)
{
  int i;

  for (i = 0; i < 6; i++)
    world[room_nr].way_out[i].exists = FALSE;
  world[room_nr].vnum = vnum;
}

  
void check_world(FILE *fl)
{
  int room_nr = 0, zone = 0, dir_nr, virtual_nr, flag, tmp, old_virtual;
  char *temp, chk[50];
  struct extra_descr_data *new_descr;

  int antal;
  char *temp2;

  rewind(fl);

  old_virtual = -1;

  do {
    antal = fscanf(fl, " #%d\n", &virtual_nr);
    init_node(room_nr, virtual_nr);

    if (old_virtual >= virtual_nr)
      printf("Rooms not in order around %d.\n", virtual_nr);
    old_virtual = virtual_nr;

    temp = fread_string(fl);
    if (flag = (*temp != '$'))  /* a new record to be read */
    {
      temp2 = fread_string(fl);

      antal = fscanf(fl, " %d ", &tmp);
      assume(antal,1, virtual_nr, "In room basic 3 numbers");

#ifndef OLD_DIKU
      antal = fscanf(fl, " %x ", &tmp);
#else
      antal = fscanf(fl, " %d ", &tmp);
#endif
      assume(antal,1, virtual_nr, "In room basic 3 numbers");

      antal = fscanf(fl, " %d ", &tmp);
      assume(antal,1, virtual_nr, "In room basic 3 numbers");

      for (;;) {
        antal = fscanf(fl, " %s \n", chk);
        assume(antal,1, virtual_nr, "Reading D/E/S string");

        if (*chk == 'D')  /* direction field */
          setup_dir(fl, virtual_nr, atoi(chk + 1), room_nr);
        else if (*chk == 'E')  /* extra description field */
        {
          temp2 = fread_string(fl); /* Description */
          temp2 = fread_string(fl); /* Keywords    */
        }
        else if (*chk == 'S')  /* end of current room */
          break;
        else
          assume(FALSE, 0, virtual_nr, "MISSING D/E or S");
      }
    }
    room_nr++;
  } while (flag);
}




/* read direction data */
void setup_dir(FILE *fl, int room, int dir, int rnum)
{
  int dest, key, antal;
  char *temp;

  temp = fread_string(fl);
  temp = fread_string(fl);

  antal = fscanf(fl, " %d ", &key);
  assume(antal,1, room, "Lockable door");
  if (key < 0 || key > 2)
    printf("Closability flag is wrong (room %d, direction %d)\n", room, dir);
  antal = fscanf(fl, " %d ", &key);
  assume(antal,1, room, "Key number for exit");
  if (key > 0 && !exist_index("key for exit, key", obj_index, top_of_objt, key))
    printf("That's for room %d, exit %d\n", room, dir);
  antal = fscanf(fl, " %d ", &dest);
  assume(antal,1, room, "Destination room for exit");
  if (!exist_index("exit in world", wld_index, top_of_wldt, dest))
    printf("Room %d is buggered.\n", room);

  if (dir < 0 || dir > 5) {
    printf("Illegal direction number (%d) for exit from room %d.\n", dir, room);
  } else if (world[rnum].way_out[dir].exists) {
    printf("Room %d has more than one exit in direction %d.\n", room, dir);
  } else if (dest == -1) {
    ; /* deliberate (we assume) exit to room -1 */
  } else {
    world[rnum].way_out[dir].exists = TRUE;
    world[rnum].way_out[dir].dest = dest;
  }
}


/* load the zone table and command tables */
void check_zones(FILE *fl)
{
  int line_no;
  int antal, tmp1, tmp2, tmp3, tmp4;
  int zon = 0, cmd_no = 0, ch, expand;
  char *check, buf[81];
  char cmd_type;
  int index;

  rewind(fl);
  line_no=1;

  for (;;)
  {
    antal = fscanf(fl, " #%*d\n");
    assume(antal, 0, line_no++, "Zone number not found");

    check = fread_string(fl);
    line_no++;

    if (*check == '$')
      break;    /* end of file */

    /* alloc a new zone */

    antal = fscanf(fl, " %d ", &zon);
    assume(antal, 1, line_no, "Zone Room < number not found");

    antal = fscanf(fl, " %d ", &zon);
    assume(antal, 1, line_no, "Life Span");

    antal = fscanf(fl, " %d ", &zon);
    assume(antal, 1, line_no, "Reset Mode");

#ifndef OLD_DIKU
    antal = fscanf(fl, " %d ", &zon);
    assume(antal, 1, line_no, "Teleport Target");
    exist_index("teleport target, room", wld_index, top_of_wldt, zon);
#endif
    line_no++;

    /* read the command table */

    cmd_no = 0;

    for (expand = 1;;)
    {
      fscanf(fl, " "); /* skip blanks */
      antal = fscanf(fl, "%c", &cmd_type);
      assume(antal, 1, line_no, "Command type M/*/O/G/E/S missing");

      if (cmd_type == 'S') {
        line_no++;
        break;
      }

      if (cmd_type == '*')
      {
        expand = 0;
        fgets(buf, 80, fl); /* skip command */
        line_no++;
        continue;
      }

      antal = fscanf(fl, " %d %d %d", &tmp1, &tmp2, &tmp3);
      sprintf(buf, "Three numbers after %c command missing -- we have %d %d %d", cmd_type, tmp1, tmp2, tmp3);
      assume(antal, 3, line_no, buf);


      if (cmd_type == 'M' || cmd_type == 'O' ||
          cmd_type == 'D' || cmd_type == 'P'
#ifndef OLD_DIKU
          || cmd_type == 'R'
#endif
        ) {
        antal = fscanf(fl, " %d", &tmp4);
        sprintf(buf, "Fourth number after %c command missing -- we have %d %d %d", cmd_type, tmp1, tmp2, tmp3);
        assume(antal, 1, line_no, buf);
      }

      switch (cmd_type) {
      case 'M' :
        exist_index("mob in zone M", mob_index, top_of_mobt, tmp2);
        exist_index("wld in zone M", wld_index, top_of_wldt, tmp4);
        break;
      case 'O' :
        exist_index("obj in zone O", obj_index, top_of_objt, tmp2);
        exist_index("wld in zone O", wld_index, top_of_wldt, tmp4);
        break;
      case 'G' :
        exist_index("obj in zone G", obj_index, top_of_objt, tmp2);
        break;
      case 'E' :
        exist_index("obj in zone E", obj_index, top_of_objt, tmp2);
        break;
      case 'P' :
        exist_index("obj in zone P", obj_index, top_of_objt, tmp2);
        exist_index("obj in zone P", obj_index, top_of_objt, tmp4);
        break;
      case 'D' :
        exist_index("wld in zone D", wld_index, top_of_wldt, tmp2);
        if ( (index = rnum_from_vnum(wld_index, top_of_wldt, tmp2)) == -1)
          printf("No room #%d -- zone trouble\n", tmp2);
        else if (tmp3 < 0 || tmp3 > 5)
          printf("Illegal direction (%d) for D command\n", tmp3);
        else if (!world[index].way_out[tmp3].exists) 
          printf("Bad D command -- no exit %d from room %d\n", tmp3, tmp2);
        break;
#ifndef OLD_DIKU
      case 'R' :
        exist_index("obj in zone R", obj_index, top_of_objt, tmp2);
        exist_index("wld in zone R", wld_index, top_of_wldt, tmp4);
        break;
#endif
      case '*' :
        break;
      default:
        printf("Illegal command type");
        exit(1);
        break;
      }


      fgets(buf, 80, fl);  /* read comment */
      line_no++;
    }
  }
}





/*************************************************************************
*  procedures for resetting, both play-time and boot-time      *
*********************************************************************** */


/* read a mobile from MOB_FILE */
void check_mobile(FILE *fl)
{
  int virtual_nr, old_virtual, antal, flag;
  char *temp;
  char bogst;

  int i, skill_nr;
  long tmp, tmp2, tmp3;
  struct char_data *mob;
  char buf[100];
  char chk[10];


  old_virtual = -1;

  rewind(fl);

  do {
    antal = fscanf(fl, " #%d\n", &virtual_nr);
    assume(antal,1, virtual_nr, "Reading #xxx");

    if (old_virtual >= virtual_nr)
      printf("Mobs not in order around %d.\n", virtual_nr);

    old_virtual = virtual_nr;

    temp = fread_string(fl);  /* Namelist */
    if (flag = (*temp != '$')) {  /* a new record to be read */

      /***** String data *** */
      /* Name already read mob->player.name = fread_string(fl); */
      temp = fread_string(fl);  /* short description  */
      temp = fread_string(fl);  /*long_description    */
      temp = fread_string(fl);  /* player.description */

      /* *** Numeric data *** */

#ifndef OLD_DIKU
      antal = fscanf(fl, "%x ", &tmp);
      assume(antal, 1, virtual_nr, "ACT error");

      antal = fscanf(fl, "%s", buf);
      assume(antal, 1, virtual_nr, "affected_by error");
      for (i = 0; buf[i]; i++)
        if ( (buf[i] >= '0' && buf[i] <= '9') ||
             (buf[i] >= 'a' && buf[i] <= 'f') ||
             (buf[i] >= 'A' && buf[i] <= 'F') )
          ;
        else
         printf("Error in affected_by: mob %d (bad digit \"%c\")\n",
                virtual_nr, buf[i]);
#else
      antal = fscanf(fl, "%d ", &tmp);
      assume(antal, 1, virtual_nr, "ACT error");

      antal = fscanf(fl, "%d", &tmp);
      assume(antal, 1, virtual_nr, "affected_by error");
#endif

      antal = fscanf(fl, " %d ", &tmp);
      assume(antal, 1, virtual_nr, "Monster Alignment Error");

      antal = fscanf(fl, " %c \n", &bogst);
      assume(antal, 1, virtual_nr, "Simple/Detailed error");

      if (bogst!='S')
        printf("%c %d\n", bogst, bogst);

      if (bogst == 'S') {
        /* The new easy monsters */

        antal = fscanf(fl, " %D ", &tmp);
        assume(antal, 1, virtual_nr, "Level error");

        antal = fscanf(fl, " %D ", &tmp);
        assume(antal, 1, virtual_nr, "THAC0 error");

        antal = fscanf(fl, " %D ", &tmp);
        assume(antal, 1, virtual_nr, "AC error");

        antal = fscanf(fl, " %Dd%D+%D ", &tmp, &tmp2, &tmp3);
        assume(antal, 3, virtual_nr, "Hitpoints");

        antal = fscanf(fl, " %Dd%D+%D \n", &tmp, &tmp2, &tmp3);
        assume(antal, 3, virtual_nr, "Damage error");

        antal = fscanf(fl, " %D ", &tmp);
        assume(antal, 1, virtual_nr, "GOLD error");

        antal = fscanf(fl, " %D \n", &tmp);
        assume(antal, 1, virtual_nr, "XP error");

        antal = fscanf(fl, " %D ", &tmp);
        assume(antal, 1, virtual_nr, "POSITION error");

        antal = fscanf(fl, " %D ", &tmp);
        assume(antal, 1, virtual_nr, "DEFAULT POS error");

        antal = fscanf(fl, " %D \n", &tmp);
        assume(antal, 1, virtual_nr, "SEXY error");

      } else {  /* The old monsters are down below here */

        printf("Detailed monsters can't be syntax-checked (yet).\n");
        assume(0,1,virtual_nr, "DETAIL ERROR");

        exit(1);
        /*   ***************************
      fscanf(fl, " %D ", &tmp);
      mob->abilities.str = tmp;

      fscanf(fl, " %D ", &tmp);
      mob->abilities.intel = tmp;

      fscanf(fl, " %D ", &tmp);
      mob->abilities.wis = tmp;

      fscanf(fl, " %D ", &tmp);
      mob->abilities.dex = tmp;

      fscanf(fl, " %D \n", &tmp);
      mob->abilities.con = tmp;

      fscanf(fl, " %D ", &tmp);
      fscanf(fl, " %D ", &tmp2);

      mob->points.max_hit = 0;
      mob->points.hit = mob->points.max_hit;

      fscanf(fl, " %D ", &tmp);
      mob->points.armor = tmp;

      fscanf(fl, " %D ", &tmp);
      mob->points.mana = tmp;
      mob->points.max_mana = tmp;

      fscanf(fl, " %D ", &tmp);
      mob->points.move = tmp;
      mob->points.max_move = tmp;

      fscanf(fl, " %D ", &tmp);
      mob->points.gold = tmp;

      fscanf(fl, " %D \n", &tmp);
      GET_EXP(mob) = tmp;

      fscanf(fl, " %D ", &tmp);
      mob->specials.position = tmp;

      fscanf(fl, " %D ", &tmp);
      mob->specials.default_pos = tmp;

      fscanf(fl, " %D ", &tmp);
      mob->player.sex = tmp;

      fscanf(fl, " %D ", &tmp);
      mob->player.class = tmp;

      fscanf(fl, " %D ", &tmp);
      GET_LEVEL(mob) = tmp;

      fscanf(fl, " %D ", &tmp);
      mob->player.birth.hours = time_info.hours;
      mob->player.birth.day  = time_info.day;
      mob->player.birth.month = time_info.month;
      mob->player.birth.year  = time_info.year - tmp;

      fscanf(fl, " %D ", &tmp);
      mob->player.weight = tmp;

      fscanf(fl, " %D \n", &tmp);
      mob->player.height = tmp;

      for (i = 0; i < 3; i++)
      {
        fscanf(fl, " %D ", &tmp);
        GET_COND(mob, i) = tmp;
      }
      fscanf(fl, " \n ");

      for (i = 0; i < 5; i++)
      {
        fscanf(fl, " %D ", &tmp);
        mob->specials.apply_saving_throw[i] = tmp;
      }

      fscanf(fl, " \n ");
      mob->points.damroll = 0;
      mob->specials.damnodice = 1;
      mob->specials.damsizedice = 6;

      mob->points.hitroll = 0;
      ************************************* */
      }

    }
  }  while (flag);

}


/* read an object from OBJ_FILE */
void check_objects(FILE *fl)
{
  int virtual_nr, old_virtual, antal, flag;
  char *temp;

  struct obj_data *obj;
  int tmp, i;
  char chk[256];
  struct extra_descr_data *new_descr;

  old_virtual = -1;

  rewind(fl);

  antal = fscanf(fl, " %s \n", chk);
  assume(antal, 1, virtual_nr, "First #xxx number");

  do {
    antal = sscanf(chk, " #%d\n", &virtual_nr);
    assume(antal,1, virtual_nr, "Reading #xxx");

    if (old_virtual >= virtual_nr)
      printf("Objects not in order around %d.\n", virtual_nr);

    old_virtual = virtual_nr;

    temp = fread_string(fl);  /* Namelist */
    if (flag = (*temp != '$')) {  /* a new record to be read */

      /* *** string data *** */

      /* temp = fread_string(fl);  name has been read above */
      temp = fread_string(fl); /* short */
      temp = fread_string(fl); /* descr */
      temp = fread_string(fl); /* action */

      /* *** numeric data *** */

      antal = fscanf(fl, " %d ", &tmp);
      assume(antal, 1, virtual_nr, "Error reading type flag");

#ifndef OLD_DIKU
      antal = fscanf(fl, " %x ", &tmp);
#else
      antal = fscanf(fl, " %d ", &tmp);
#endif
      assume(antal, 1, virtual_nr, "Extra Flag");

#ifndef OLD_DIKU
      antal = fscanf(fl, " %x ", &tmp);
#else
      antal = fscanf(fl, " %d ", &tmp);
#endif
      assume(antal, 1, virtual_nr, "wear_flags");

      antal = fscanf(fl, " %d ", &tmp);
      assume(antal, 1, virtual_nr, "value[0]");

      antal = fscanf(fl, " %d ", &tmp);
      assume(antal, 1, virtual_nr, "value[1]");

      antal = fscanf(fl, " %d ", &tmp);
      assume(antal, 1, virtual_nr, "value[2]");

      antal = fscanf(fl, " %d ", &tmp);
      assume(antal, 1, virtual_nr, "value[3]");

      antal = fscanf(fl, " %d ", &tmp);
      assume(antal, 1, virtual_nr, "Weight");

      antal = fscanf(fl, " %d \n", &tmp);
      assume(antal, 1, virtual_nr, "Cost");

      antal = fscanf(fl, " %d \n", &tmp);
      assume(antal, 1, virtual_nr, "Cost Per Day");

      /* *** extra descriptions *** */

      while (fscanf(fl, " %s \n", chk), *chk == 'E')
      {

        temp = fread_string(fl);
        temp = fread_string(fl);
      }

      for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++)
      {
        antal = fscanf(fl, " %d ", &tmp);
        assume(antal, 1, virtual_nr, "affected location");

        antal = fscanf(fl, " %d \n", &tmp);
        assume(antal, 1, virtual_nr, "Modifier");

        antal = fscanf(fl, " %s \n", chk);
        assume(antal, 1, virtual_nr, "Next string with E/A/$");

      }
    }
  }  while (flag);
}




/************************************************************************
*  procs of a (more or less) general utility nature      *
********************************************************************** */


/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl)
{
  static char buf[MAX_STRING_LENGTH], tmp[MAX_STRING_LENGTH];
  char *rslt;
  register char *point;
  int flag;

  bzero(buf, MAX_STRING_LENGTH);

  do
  {
    if (!fgets(tmp, MAX_STRING_LENGTH, fl))
    {
      printf("fread_str");
      exit(1);
    }

    if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH-1)
    {
      printf("fread_string: string too large (db.c, fread_string)");
      exit(1);
    }
    else
      strcat(buf, tmp);

    for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
        point--);
    if (flag = (*point == '~'))
      if (*(buf + strlen(buf) - 3) == '\n')
      {
        *(buf + strlen(buf) - 2) = '\r';
        *(buf + strlen(buf) - 1) = '\0';
      }
      else
        *(buf + strlen(buf) -2) = '\0';
    else
    {
      *(buf + strlen(buf) + 1) = '\0';
      *(buf + strlen(buf)) = '\r';
    }
  }  while (!flag);

  return(buf);
}

void
check_reach(void)
{
  int i, j;
  int dest;
  int first;
  bool changed;

  for (i = 0; i <= top_of_wldt; i++) {
    world[i].flag = world[i].processed = FALSE;
    for (j = 0; j < 6; j++)
      if (world[i].way_out[j].exists)
        world[i].way_out[j].dest =
          rnum_from_vnum(wld_index, top_of_wldt, world[i].way_out[j].dest);
  }
  if ( (first = rnum_from_vnum(wld_index, top_of_wldt, firstroom)) == -1) {
    printf("Your specified first room (%d) is not reachable!\n", firstroom);
    return;
  }

  /* for first pass, flag means "we can get here from firstroom" */
  world[first].flag = TRUE;
  changed = TRUE;
  while (changed) {
    changed = FALSE;
    for (i = 0; i < top_of_wldt; i++) {
      if (world[i].flag && !world[i].processed) {
        world[i].processed = TRUE;
        for (j = 0; j < 6; j++) {
          if (world[i].way_out[j].exists) {
            dest = world[i].way_out[j].dest;
            if (!world[dest].flag) {
              world[dest].flag = TRUE;
              changed = TRUE;
            }
          }
        }
      }
    }
  }

  printf("Unreachables:\n");
  for (i = 0; i < top_of_wldt; i++)
    if (!world[i].flag)
      printf("%d\n", world[i].vnum);
  printf("\n");

  for (i = 0; i < top_of_wldt; i++) world[i].flag = world[i].processed = FALSE;

  /* flag now means "we can reach first room from here" */
  world[first].flag = TRUE;
  changed = TRUE;
  while (changed) {
    changed = FALSE;
    for (i = 0; i < top_of_wldt; i++) {
      if (!world[i].flag)
        for (j = 0; j < 6; j++)
          if (world[i].way_out[j].exists) {
            dest = world[i].way_out[j].dest;
            if (world[dest].flag) {
              world[i].flag = TRUE;
              changed = TRUE;
              break;
            }
          }
      }
  }


  printf("Inescapables:\n");
  for (i = 0; i < top_of_wldt; i++)
    if (!world[i].flag)
      printf("%d\n", world[i].vnum);
  printf("\n");
}

void
check_one_way(void)
{
  int i, j;
  int other_way, dest;
  static int opposites[] = {2, 3, 0, 1, 5, 4};
  static char *names = "neswud";

  printf("One-ways:\n");
  for (i = 0; i < top_of_wldt; i++)
    for (j = 0; j < 6; j++)
      if (world[i].way_out[j].exists) {
        dest = world[i].way_out[j].dest;
        other_way = opposites[j];
        if (world[dest].way_out[other_way].exists &&
            world[dest].way_out[other_way].dest == i)
          ; /* do nothing */
        else
          printf("%d (dir %d(%c))\n", world[i].vnum, j, names[j]);
      }
}
    

int main(int argc, char *argv[])
{
  char name[256];

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <basefilename> <firstroom>\n", argv[0]);
    fprintf(stderr, "Your section files must be of the form X.wld, X.obj, X.mob and X.zon.\n");
    fprintf(stderr, "The \"basefilename\" replaces \"X\" in the above.\n\n");
    fprintf(stderr, "\"firstroom\" is the initial room in the section (used for reachability tests).\n");
    fprintf(stderr, "The normal invocation is \"%s lib/tinyworld 3001\"\n", argv[0]);
    fprintf(stderr, "\n\"Unreachable\" rooms are ones that you can't get to from the firstroom.\n");
    fprintf(stderr, "\"Inescapable\" rooms are ones that you can't get to the firstroom from.\n");
    exit(0);
  }

  firstroom = atoi(argv[2]);
 
  strcpy(name, argv[1]);
  strcat(name, ".wld");

  if (!(wld_f = fopen(name, "r")))
  {
    fprintf(stderr, "Could not open world file.\n");
    exit(1);
  }
  strcpy(name, argv[1]);
  strcat(name, ".mob");
  if (!(mob_f = fopen(name, "r")))
  {
    fprintf(stderr, "Could not open mobile file.\n");
    exit(1);
  }
  strcpy(name, argv[1]);
  strcat(name, ".obj");
  if (!(obj_f = fopen(name, "r")))
  {
    fprintf(stderr, "Could not open object file.\n");
    exit(1);
  }
  strcpy(name, argv[1]);
  strcat(name, ".zon");
  if (!(zon_f = fopen(name, "r")))
  {
    fprintf(stderr, "Could not open zone file.\n");
    exit(1);
  }


  fprintf(stderr, "Generating world file indexes.\n");
  wld_index = generate_indices(wld_f, &top_of_wldt);
  CREATE(world, struct room_graph, top_of_wldt);

  fprintf(stderr, "Generating mobile file indexes.\n");
  mob_index = generate_indices(mob_f, &top_of_mobt);

  fprintf(stderr, "Generating object file indexes.\n");
  obj_index = generate_indices(obj_f, &top_of_objt);

  fprintf(stderr, "Checking World File.\n");
  check_world(wld_f);

  fprintf(stderr, "Checking Zone File.\n");
  check_zones(zon_f);

  fclose(zon_f);
  fclose(wld_f);
  fclose(mob_f);
  fclose(obj_f);

  fprintf(stderr, "Checking reachability.\n");
  check_reach();

  fprintf(stderr, "Checking for one-way exits.\n");
  check_one_way();

  printf("Finished all checking.\n");
}
