//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// Usage: Various routines for moving about objects/players               
//
//////////////////////////////////////////////////////////////////////////

#ifndef __HANDLER_H
#define __HANDLER_H

#include "obj_money.h"

TMoney *create_money( int amount );

/* ******** objects *********** */

void thing_to_room(TThing *ch, int room);

TThing *get_thing_in_list_getable(TBeing *ch, const char *name, TThing *list);
TThing *get_thing_in_list_num(int num, TThing *list);
TThing *get_thing_on_list_getable(TBeing *ch, const char *name, TThing *list);
TThing *get_thing_on_list(const char *name, TThing *list);
TThing *get_thing_on_list_num(int num, TThing *list);
TThing *get_thing_on_list_vis(TBeing *ch, const char *name, TThing *list);
TThing *get_thing_in_equip(TBeing *, const char *, equipmentData equipment, wearSlotT *j, bool, int*count);
TThing *get_thing_stuck_in_vis(TBeing *, const char *, wearSlotT *j, int *count, TBeing *);
TThing *get_thing_char_using(TBeing *, const char *, int, bool, bool);
TObj *get_obj(const char *name, exactTypeT);
TObj *get_obj_num(int nr);
TObj *get_num_obj_in_list(TBeing *ch, int num, TThing *list, int shop_nr);
TObj *get_obj_vis(TBeing *ch, const char *name, int *count, exactTypeT exact);
TObj *get_obj_vis_world(TBeing *ch, const char *name, int *count, exactTypeT exact);
TObj *get_obj_vis_accessible(TBeing *ch, const sstring &name);

/* ******* characters ********* */

TBeing *get_char_room(const sstring &name, int room, int *count = NULL);
TBeing *get_char_num(int nr);
TBeing *get_char(const char *name, exactTypeT exact);

/* find if character can see */
TBeing *get_char_room_vis(const TBeing *ch, const sstring &name, int *count = NULL, exactTypeT exact = EXACT_NO, infraTypeT = INFRA_NO);
TBeing *get_char_vis_world(const TBeing *ch, const sstring &name, int *count, exactTypeT exact, infraTypeT = INFRA_NO);
TBeing *get_char_vis(const TBeing *ch, const sstring &name, int *count, infraTypeT = INFRA_NO);
TBeing *get_pc_world(const TBeing *ch, const sstring &name, exactTypeT exact, infraTypeT = INFRA_NO, bool visible = TRUE);
TBeing * get_best_char_room(const TBeing *, const char *, visibleTypeT vis = VISIBLE_YES, infraTypeT inf = INFRA_NO);

TObj *generic_find_obj(sstring, int, TBeing *);
TBeing *generic_find_being(sstring, int, TBeing *);
int generic_find(const char *, int, TBeing *, TBeing**, TObj**);

const unsigned int FIND_CHAR_ROOM     =(1<<0);
const unsigned int FIND_CHAR_WORLD    =(1<<1);
const unsigned int FIND_OBJ_INV       =(1<<2);
const unsigned int FIND_OBJ_ROOM      =(1<<3);
const unsigned int FIND_OBJ_WORLD     =(1<<4);
const unsigned int FIND_OBJ_EQUIP     =(1<<5);
const unsigned int FIND_OBJ_COMM      =(1<<6);
const unsigned int FIND_ROOM_EXTRA    =(1<<7);

extern TThing *searchLinkedListVis(const TBeing *ch, sstring name, TThing *list, int *num = NULL, thingTypeT x = TYPETHING);

extern TThing *searchLinkedList(const char * name, TThing *list, thingTypeT x = TYPETHING);
extern TThing *searchLinkedList(const sstring & name, TThing *list, thingTypeT x = TYPETHING);

#endif
