//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disease.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


/**************************************************************************

      SneezyMUD - All rights reserved, SneezyMUD Coding Team
      "disease.h" - interface to disease.c

 A note on diseases in SneezyMUD ...  Diseases are simply periodically
 activated affects.  I.e., every so often, if a mob/player is affected by
 a disease then a special procedure related to that specific disease is
 called.   There may be other places where affects are checked for and
 called.  These will vary from disease to disease.  The important thing
 to remember when creating/thinking of diseases is that the goal is not
 to burden the player but to enrich playing.  That wount stop me from 
 writing disease_herpes, however.
                                            - Stargazer   9/93

**************************************************************************/

#ifndef __DISEASE_H
#define __DISEASE_H

/* tech note:  Mob_Affect fields for diseases are defined as follows ...
   short type;         <-- should always be AFFECT_DISEASE
   sbyte level;        <-- state data for the disease
   sh_int duration;    <-- updates until disease wears off
   sbyte modifier;     <-- type of disease (note this is signed, but only
                           use 0 -> 127.  I think 127 diseases is plenty.) 
   byte location;      <-- this should be APPLY_NONE
   long bitvector;     <-- this should be 0
*/

const sbyte DISEASE_NULL      =0; /* a do-nothing disease which logs warning */
const sbyte DISEASE_COLD      =1;
const sbyte DISEASE_FLU       =2;
const sbyte DISEASE_FROSTBITE =3;
const sbyte DISEASE_BLEEDING  =4;
const sbyte DISEASE_INFECTION =5;
const sbyte DISEASE_HERPES    =6;
const sbyte DISEASE_BROKEN_BONE =7;
const sbyte DISEASE_NUMB      =8;
const sbyte DISEASE_VOICEBOX  =9;
const sbyte DISEASE_EYEBALL   =10;
const sbyte DISEASE_LUNG      =11;
const sbyte DISEASE_STOMACH   =12;
const sbyte DISEASE_HEMORRAGE =13;
const sbyte DISEASE_LEPROSY   =14;
const sbyte DISEASE_PLAGUE    =15;
const sbyte DISEASE_SUFFOCATE =16;
const sbyte DISEASE_FOODPOISON= 17;
const sbyte DISEASE_DROWNING  =18;
const sbyte DISEASE_GARROTTE  =19;
const sbyte DISEASE_POISON    =20;

const int MAX_DISEASE       =21;    /* highest disease # that is valid */

sbyte DISEASE_INDEX(sbyte d);

                    /* DISEASE SPEC_PROC MESSAGES */

/* The format for a spec_proc is:
    int disease_name(TBeing **victim, int message, affectedData *af);        */

/* spec procs should (for now) always return FALSE from a message call   */

/* note, if message > 0, then it represents a "command triggered" message.
   This is similar to mobile/object/room special procedures, in that 
   the message # will be the command # that triggered the spec_proc.     */
const int DISEASE_PULSE  =0;    /* spec_proc is getting called for its periodic
                               update.  Update time is defined in comm.c */
const int DISEASE_DONE  =-1;   /* spec_proc is getting called for the last
                               time on the victim .  The affect will be 
                               removed following spec_proc execution.i
                               Use stop_disease(). */
const int DISEASE_BEGUN =-2;   /* spec_proc is getting called for the first 
                              time on the victim.   Use start_disease(). */

/* disease function prototypes */

void spread_affect(TBeing *ch, int chance_to_spread, bool race, bool not_race, affectedData *af);
 int disease_start(TBeing *ch, affectedData *af);

typedef struct {
  int (*code)(TBeing *, int, affectedData *);
  char name[40];
  int cure_cost;  
} DISEASEINFO;

extern DISEASEINFO DiseaseInfo[MAX_DISEASE];

const int DISEASE_PRICE_3       =1;
const int DISEASE_PRICE_6       =50;
const int DISEASE_PRICE_12      =150;

#endif
