//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


extern int throwThing(TThing *, dirTypeT, int, TBeing **, int, int, TBeing *);
extern TBeing *get_char_linear(const TBeing *, char *, int *, dirTypeT *);
extern TBeing *get_char_vis_direction(const TBeing *, char *, dirTypeT, unsigned int, bool, unsigned int *count = NULL);
extern TThing *has_range_object(TBeing *ch, int *pos);
extern bool can_see_char_other_room(const TBeing *, TBeing *, TRoom *rp);
