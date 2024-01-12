#pragma once

class TBeing;
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
  DISEASE_HEMORRHAGE,
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
  DISEASE_EXTREME_PAIN,
  MAX_DISEASE
};

diseaseTypeT affToDisease(affectedData&);

// The format for a spec_proc is:
//  int disease_name(TBeing **victim, int message, affectedData *af);
//
// spec procs should (for now) always return false from a message call
//
// note, if message > 0, then it represents a "command triggered" message.
// This is similar to mobile/object/room special procedures, in that
// the message # will be the command # that triggered the spec_proc.
//
// the disease affect will be removed following the DISEASE_DONE call

inline constexpr int DISEASE_PULSE = 0;
inline constexpr int DISEASE_DONE = -1;
inline constexpr int DISEASE_BEGUN = -2;

void spread_affect(TBeing* ch, int chance_to_spread, bool race, bool not_race,
  affectedData* af);
int disease_start(TBeing* ch, affectedData* af);

struct DISEASEINFO {
    int (*code)(TBeing*, int, affectedData*);
    char name[40];
    int cure_cost;
};

extern DISEASEINFO DiseaseInfo[MAX_DISEASE];

inline constexpr int DISEASE_PRICE_3 = 1;
inline constexpr int DISEASE_PRICE_6 = 50;
inline constexpr int DISEASE_PRICE_12 = 150;
