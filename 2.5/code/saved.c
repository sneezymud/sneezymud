/*
  Sneezymud Save Daemon.
*/
/*
  Regular game spawns this subproc, and opens a socket.

  the current save_objs is here.  The regular save_objs is just a proc
  that collects the data, and writes it to here.  This procedure writes
  the data to file

  this procedure also reads data back to the game.
*/
#include "structs.h"
#include "utils.h"


void get_objs_from_file( char *name)
{
  int i, j;
  bool found = FALSE;
  float timegold;
  struct rental_header rh;
  struct obj_file_u *st;
  extern FILE *obj_file;

  
  /* r+b is for Binary Reading/Writing */
  if (!(fl = fopen(OBJ_SAVE_FILE, "r+b")))  {
    perror("Opening object file for Loading PC's objects");
    exit(1);
  }
  
  
  while (!feof(obj_file) && !found) {
    if (0==fread(&rh, sizeof(rh), 1, obj_file))
      break;
    if (!rh.inuse || 0!=str_cmp(name, rh.owner)) {
      fseek(obj_file, rh.length, 1);
      continue;
    }
    st = (void*)malloc(rh.length);
    fread(st, rh.length, 1, obj_file);
    found = TRUE;
  }
  

  /*
    write st to the socket.
  */


  /*
     and we're done.
  */
}

/**************************************************************************/

void update_file(char *name, struct obj_file_u *st)
{
  struct rental_header	rh;
  int	nlength;

  if (st->nobjects==0)
    return;

  nlength = sizeof(*st) + (st->nobjects-MAX_OBJ_SAVE) * sizeof(*st->objects);
  rewind(obj_file);

  while (1) {
    if (feof(obj_file) || 0==fread(&rh, sizeof(rh), 1, obj_file)) {
      rh.length = nlength;
      break;
    }
    if (rh.inuse) {

#ifdef DEBUG
      printf("block of size %d in use\n",rh.length);
#endif

      fseek(obj_file, rh.length, 1);
      continue;
    }
    if (rh.length >= nlength) {

#ifdef DEBUG
      printf("block of size %d not in use, using for %s (%d)\n",
	     rh.length, name, nlength);
#endif

      fseek(obj_file, -sizeof(rh), 1);
      break;
    }

#ifdef DEBUG
    printf("block of size %d too small\n", rh.length);
#endif

    fseek(obj_file, rh.length, 1);
  }
  rh.inuse = 1;
  strcpy(rh.owner, name);
  fwrite(&rh, sizeof(rh), 1, obj_file);
  fwrite(st, nlength, 1, obj_file);
  fseek(obj_file, 0, 1);
  flush(obj_file);
  return;

}
