#pragma once

/* tech note:  Mob_Affect fields for diseases are defined as follows ...
   short type;         <-- should always be AFFECT_DISEASE
   sbyte level;        <-- state data for the disease
   short duration;    <-- updates until disease wears off
   sbyte modifier;     <-- type of disease (note this is signed, but only
                           use 0 -> 127.  I think 127 diseases is plenty.) 
   byte location;      <-- this should be APPLY_NONE
   long bitvector;     <-- this should be 0
*/

class affectedData;

enum diseaseTypeT {
     DISEASE_NULL,
     DISEASE_COLD,
     DISEASE_FLU,
     DISEASE_FROSTBITE,
     DISEASE_BLEEDING,
     DISEASE_INFECTION,
     DISEASE_HERPES,
     DISEASE_BROKEN_BONE,
     DISEASE_NUMB,
     DISEASE_VOICEBOX,
     DISEASE_EYEBALL,
     DISEASE_LUNG,
     DISEASE_STOMACH,
     DISEASE_HEMORRAGE,
     DISEASE_LEPROSY,
     DISEASE_PLAGUE,
     DISEASE_SUFFOCATE,
     DISEASE_FOODPOISON,
     DISEASE_DROWNING,
     DISEASE_GARROTTE,
     DISEASE_POISON,
     DISEASE_SYPHILIS,
     DISEASE_BRUISED,
     DISEASE_SCURVY,
     DISEASE_DYSENTERY,
     DISEASE_PNEUMONIA,
     DISEASE_GANGRENE,
     MAX_DISEASE
};

diseaseTypeT affToDisease(affectedData &);

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

