//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: range.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


extern int throwThing(TThing *, dirTypeT, int, TBeing **, int, int, TBeing *);
extern TBeing *get_char_linear(const TBeing *, char *, int *, dirTypeT *);
extern TBeing *get_char_vis_direction(const TBeing *, char *, dirTypeT, unsigned int, bool, unsigned int *count = NULL);
extern TThing *has_range_object(TBeing *ch, int *pos);
extern bool can_see_char_other_room(const TBeing *, TBeing *, TRoom *rp);
