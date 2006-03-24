struct Extra {
  struct char_data *target;
  int state; 
};


#define TH_WANDERING 0

int Thief(struct char_data *ch, char *arg, int cmd)
{
   struct obj_data *best, *o;

   if (cmd || !AWAKE(ch)) return(FALSE);

   if (!ch->act_ptr) {
       /* try to find a victim */
       if ((vict = FindVictim(ch)) != NULL) {
	 do_look(ch, GET_NAME(vict), 0);  /* now we have their stuff */
	 ch->act_ptr = (struct Extra *) malloc(sizeof(struct Extra));
	 ch->act_ptr->target = vict;  /* if the vict. dies we're screwed */
	                              /* A strong arg. for c++           */
	 TH_STATE(ch) = TH_SWIPING;  /* swiping things n stuff */
       }     
   } else {
     switch(TH_STATE(ch)) {
       case TH_SWIPING: {  /* choose something on their person */
	 /* find the most valuable item */
	 best
	 for (o = TH_TARG(ch)->carrying; o; o=o->next_content) {
	   if (best) {
	      if (o->obj_flags.cost > best->obj_flags.cost) {
	        best = o;
	      }
	   } else {
	     best = o;
	   }
	 }
	 if (best) { /* try to steal it */
	 }
       }
     }
   }
}     

 

FindABetterWeapon(struct char_data *mob)
{
  struct obj_data *o, *best;
  /*
    pick up and wield weapons
    Similar code for armor, etc.
    */
  
  if (!real_roomp(mob->in_room)) return(FALSE);
  
  /* check room */
  best = 0;
  for (o = real_roomp(mob->in_room)->contents; o; o = o->next_content) {
    if (best) {
	if (GetDamage(o,ch) > GetDamage(best,ch)) {
	  best = o;
    } else {
      if (IS_WEAPON(o)) {
	best = o;
      }
    }
    /* check inv */
    for (o = mob->carrying; o; o=o->next_content) {
      if (best) {
	if (GetDamage(o,ch) > GetDamage(best,ch)) {
	  best = o;
	}
      } else {
	if (IS_WEAPON(o)) {
	  best = o;
	}
      }
    }
    if (best) {  /* compare this best weapon in inventory with eq */
      if (GetDamage(ch->equipment[WIELD],ch) >= GetDamage(best),ch) {
	best = ch->equipment[WIELD];
      } else {
	/*
	  out with the old, in with the new
	*/
	do_remove(ch, fname(ch->equipment[WIELD]->name), 0);
	do_wield(ch, fname(best->name), 0);
      }
    }
  }
}

int GetDamage(struct obj_data *w, struct char_data *ch) 
{
   float ave;
   int num, size, iave;
/*
  return the average damage of the weapon, with plusses.
*/
    num  = w->obj_flags.value[2];
    size = w->obj_flags.value[3];

    ave = size/2.0 + 0.5

    ave *= num;

    ave += GetDamBonus(w);
/*
  check for immunity:
*/
     iave = ave;
    if (ch->specials.fighting) {
       iave = PreProcDam(ch->specials.fighting, ITEM_TYPE(w), iave);
       iave = WeaponCheck(ch, ch->specials.fighting, ITEM_TYPE(w), iave);
    }
    return(iave);
}

int GetDamBonus(struct obj_data *w)
{
  int j, tot=0
/* return the damage bonus from a weapon */
      for(j=0; j<MAX_OBJ_AFFECT; j++) {
	if (w->equipment[WIELD]->affected[j].location ==
	    APPLY_DAMROLL || APPLY_HITNDAM) {
	  tot += w->equipment[WIELD]->affected[j].modifier;

  return(tot);
}
