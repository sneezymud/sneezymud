//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//
//////////////////////////////////////////////////////////////////////////

// body.h
//
//  This is an attempt to make some sense out of the limb system.
#pragma once

#include "limbs.h"

const int MAX_SHEATH = 3;

enum body_t {
  BODY_NONE,
  BODY_HUMANOID,
  BODY_INSECTOID,
  BODY_PIERCER,
  BODY_MOSS,
  BODY_ELEMENTAL,
  BODY_KUOTOA,
  BODY_CRUSTACEAN,
  BODY_DJINN,
  BODY_MERMAID,
  BODY_FROGMAN,
  BODY_MANTICORE,
  BODY_GRIFFON,
  BODY_SHEDU,
  BODY_SPHINX,
  BODY_CENTAUR,
  BODY_LAMIA,
  BODY_LAMMASU,
  BODY_WYVERN,
  BODY_DRAGONNE,
  BODY_HIPPOGRIFF,
  BODY_CHIMERA,
  BODY_DRAGON,
  BODY_FISH,
  BODY_SNAKE,
  BODY_NAGA,
  BODY_SPIDER,
  BODY_CENTIPEDE,
  BODY_OCTOPUS,
  BODY_BIRD,
  BODY_BAT,
  BODY_TREE,
  BODY_PARASITE,
  BODY_SLIME,
  BODY_ORB,
  BODY_VEGGIE,
  BODY_DEMON,
  BODY_LION,
  BODY_FOUR_LEG,
  BODY_PIG,
  BODY_TURTLE,
  BODY_FOUR_HOOF,
  BODY_BAANTA,
  BODY_AMPHIBEAN,
  BODY_FROG,
  BODY_MIMIC,
  BODY_MEDUSA,
  BODY_FELINE,
  BODY_DINOSAUR,
  BODY_REPTILE,
  BODY_ELEPHANT,
  BODY_OTYUGH,
  BODY_OWLBEAR,
  BODY_MINOTAUR,
  BODY_GOLEM,
  BODY_COATL,
  BODY_SIMAL,
  BODY_PEGASUS,
  BODY_ANT,
  BODY_WYVELIN,
  BODY_FISHMAN,

  MAX_BODY_TYPES
};

extern const char* bodyNames[MAX_BODY_TYPES];
