// body.cc
//
//  Lets see if I can make some magic.

#include "stdsneezy.h"
#include "body.h"
#include "race.h"

const char *bodyNames[MAX_BODY_TYPES]={"BODY_NONE","HUMANOID",
    "INSECTOID", "PIERCER", "MOSS", "ELEMENTAL", "KUOTOA",
    "CRUSTACEAN", "DJINN", "MERMAID",
    "FROGMAN", "MANTICORE", "GRIFFON", "SHEDU", "SPHINX",
    "CENTAUR", "LAMIA", "LAMMASU", "WYVERN", "DRAGONNE",
    "HIPPOGRIFF", "CHIMERA", "DRAGON", "FISH", "SNAKE",
    "NAGA", "SPIDER", "CENTIPEDE", "OCTOPUS", "BIRD",
    "BAT", "TREE", "PARASITE", "SLIME", "ORB",
    "VEGGIE", "DEMON", "LION", "FOUR_LEG", "PIG",
    "TURTLE", "FOUR_HOOF", "BAANTA", "AMPHIBEAN", "FROG",
    "MIMIC", "MEDUSA", "FELINE", "DINOSAUR", "REPTILE",
    "ELEPHANT", "OTYUGH", "OWLBEAR", "MINOTAUR", "GOLEM",
    "COATL", "SIMAL", "PEGASUS", "ANT",
};

// Constructor.
// Given the body_type, build a body.  You want to pass the owner's max
// hitpoints.

Body::Body(body_t body_type, int hitpoints)
  : Limb(),
  bodyType(body_type),
  attack(NULL),
  hit(hitpoints)
{

  for(int mainLimb=LIMB_HEAD; mainLimb < MAX_MAIN_LIMBS; mainLimb++) {
    body[mainLimb]=NULL;
  }

  for(int i=0;i<MAX_SHEATH;i++) {
    sheathSlots[i] = NULL;
  }

  loadBody();
}

// Default constructor.

Body::Body()
  : Limb(),
  bodyType(BODY_NONE),
  attack(NULL),
  hit(0)
{

  for(int mainLimb=LIMB_HEAD; mainLimb < MAX_MAIN_LIMBS; mainLimb++) {
    body[mainLimb]=NULL;
  }

  for(int i=0;i<MAX_SHEATH;i++) {
    sheathSlots[i] = NULL;
  }
}

// Destructor.  Sets the attack limb list to NULL before it proceeds to
// delete all of the limbs.  Top is just the top of the list.  It also relies
// a bit on the Limb destructor to take care of its own sublimbs.

Body::~Body() {

  vlogf(LOG_MISC, "Destroying the Body.");
  attack = NULL;

  for(int mainLimb=LIMB_HEAD; mainLimb < MAX_MAIN_LIMBS; mainLimb++) {
    Limb *top = body[mainLimb];
    while (top) {
      Limb *tmp = top;
      top = top->next;
      delete tmp;
    }
  }

  for(int i=0;i<MAX_SHEATH;i++)
    delete sheathSlots[i];

}

// loadBody() takes the bodyType and creates a Body based on a file in
// /mud/code/lib/bodytypes.

void Body::loadBody()
{
  const sstring bodyLib = "bodytypes/";

  sstring limbtype, connector, description, aFilename;
  ifstream bodyFile;

  aFilename = bodyLib + bodyNames[bodyType];

  bodyFile.open(aFilename.c_str(), ios::in);
  if (!bodyFile.is_open()) {
    aFilename = bodyLib + "HUMANOID";
    bodyFile.open(aFilename.c_str(), ios::in);
  }

  // Go through each line of the bodytype file and add each limb as specified.
  // The order in which the limbs are added is important since addLimb()
  // does some rudimentary error checking (make sure it has the proper connects
  // to limb available).  Ignore comments.
  while (bodyFile >> limbtype) {
    if(limbtype.find("#") || !(limbtype.length()))
      continue;
    bodyFile >> connector;
    getline(bodyFile, description);
    Limb *nextLimb = new Limb(limbtype, connector, description);
    addLimb(nextLimb);
  }
}

//  addLimb() checks to see if the limb connects directly to the body.
//  if it does, it connect it.  If not, search through each major limb
//  (basically, each limb connected directly to body) for the appropriate
//  connector.  If you find the right limb, connect them, otherwise print
//  an error statement.

int Body::addLimb(Limb *newLimb) {
  int target = newLimb->connectsTo;

  if (target == LIMB_BODY) 
    return join(newLimb);

  Limb *connector = search(target, LIMB_NOSUBLIMB);
  if(connector) {
    connector->join(newLimb);
    return TRUE;
  }
  return FALSE;
}

Limb *Body::search(int target, int status)
{
  Limb *found = NULL;

  for(int mainLimb=LIMB_HEAD; mainLimb < MAX_MAIN_LIMBS; mainLimb++) {
    Limb *notFound=body[mainLimb];
    while(notFound) {
      found=notFound->search(target,status);
      if(found)
	return found;
      else
	notFound = notFound->next;
    }
  }
  return found;
}

int Body::join(Limb *newLimb) {
  Limb **joinTo;

  if (newLimb->limbType == LIMB_NONE) {
    vlogf(LOG_LOW, " this limb not initialized properly.");
    return 0;
  }

  joinTo = &(body[newLimb->limbType]);
  while(*joinTo)
    joinTo = &(*joinTo)->next;
  (*joinTo) = newLimb;
  return 1;
}

void Body::showBody(TBeing *caller) {
  Limb *current;
  for(int mainLimb=LIMB_HEAD; mainLimb < MAX_MAIN_LIMBS; mainLimb++) {
    current = body[mainLimb];
    while(current) {
      current->showLimb(caller);
      current=current->next;
    }
  }
}

// all references to this need to go through the mapping function
// since the slot order has changed.  -- bat 06-27-97
const ubyte slot_chance[MAX_BODY_TYPES][MAX_WEAR] =
{
// unused, fing, fing, neck, body, head, leg, leg, foot, foot, hand, hand, arm
// arm, back, waist, wrist, wrist, hold, hold, leg, leg, foot, foot

  {0, 1, 1, 4, 26, 7, 3, 3, 2, 2, 3, 3, 5,
   5, 10, 5, 3, 3, 7, 7, 0, 0, 0, 0},       // BODY_NONE
  {0, 1, 1, 4, 26, 7, 3, 3, 2, 2, 3, 3, 5,
   5, 10, 5, 3, 3, 7, 7, 0, 0, 0, 0},       // BODY_HUMANOID
  {0, 0, 0, 0, 15, 25, 5, 5, 0, 0, 0, 0, 0,
   0, 5, 25, 0, 0, 0, 0, 5, 5, 5, 5},        // BODY_INSECTOID
  {0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},        // BODY_PIERCER
  {0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},        // BODY_MOSS
  {0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},        // BODY_ELEMENTAL
  {0, 0, 0, 4, 26, 7, 3, 3, 2, 2, 3, 3, 5,
   5, 10, 5, 0, 0, 7, 7, 0, 0, 0, 0},       // BODY_KUOTOA
  {0, 0, 0, 0, 40, 0, 5, 5, 0, 0, 6, 6, 6,
   6, 0, 0, 0, 0, 6, 6, 5, 5, 0, 0},        // BODY_CRUSTACEAN
  {0, 1, 1, 4, 31, 7, 3, 0, 2, 0, 3, 3, 5,
   5, 10, 5, 3, 3, 7, 7, 0, 0, 0, 0},       // BODY_DJINN
  {0, 1, 1, 4, 31, 7, 3, 0, 2, 0, 3, 3, 5,
   5, 10, 5, 3, 3, 7, 7, 0, 0, 0, 0},       // BODY_MERMAID
  {0, 1, 1, 4, 26, 7, 3, 3, 2, 2, 3, 3, 5,
   5, 10, 5, 3, 3, 7, 7, 0, 0, 0, 0},       // BODY_FROGMAN
  {0, 0, 0, 5, 30, 7, 5, 5, 3, 3, 0, 0, 5,
   5, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_MANTICORE
  {0, 0, 0, 5, 30, 7, 5, 5, 3, 3, 0, 0, 5,
   5, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_GRIFFON
  {0, 0, 0, 5, 30, 7, 5, 5, 3, 3, 0, 0, 5,
   5, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_SHEDU
  {0, 0, 0, 5, 30, 7, 5, 5, 3, 3, 0, 0, 5,
   5, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_SPHINX
  {0, 1, 1, 4, 26, 7, 3, 3, 2, 2, 3, 3, 5,
   5, 12, 4, 3, 3, 5, 5, 3, 3, 2, 2},       // BODY_CENTAUR
  {0, 1, 1, 4, 26, 7, 3, 3, 2, 2, 3, 3, 5,
   5, 12, 4, 3, 3, 5, 5, 3, 3, 2, 2},       // BODY_LAMIA
  {0, 0, 0, 5, 30, 7, 5, 5, 3, 3, 0, 0, 5,
   5, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_LAMMASU
  {0, 0, 0, 5, 30, 7, 5, 5, 3, 3, 3, 3, 5,
   5, 7, 5, 0, 0, 2, 2, 0, 0, 0, 0},        // BODY_WYVERN
  {0, 0, 0, 5, 30, 7, 5, 5, 3, 3, 0, 0, 5,
   5, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_DRAGONNE
  {0, 0, 0, 5, 30, 7, 5, 5, 3, 3, 0, 0, 5,
   5, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_HIPPOGRIFF
  {0, 0, 0, 5, 30, 7, 5, 5, 3, 3, 0, 0, 5,
   5, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_CHIMERA
  {0, 0, 0, 5, 30, 7, 5, 5, 3, 3, 0, 0, 5,
   5, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_DRAGON
  {0, 0, 0, 0, 75, 25, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},        // BODY_FISH
  {0, 0, 0, 0, 80, 20, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},        // BODY_SNAKE
  {0, 0, 0, 0, 80, 20, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},        // BODY_NAGA
  {0, 0, 0, 0, 47, 8, 4, 4, 4, 4, 0, 0, 0,
   0, 5, 0, 0, 0, 4, 4, 4, 4, 4, 4},        // BODY_SPIDER
  {0, 0, 0, 0, 47, 8, 4, 4, 4, 4, 0, 0, 0,
   0, 5, 0, 0, 0, 4, 4, 4, 4, 4, 4},        // BODY_CENTIPEDE
  {0, 0, 0, 0, 40, 10, 5, 5, 5, 5, 0, 0, 5,
   5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0},        // BODY_OCTOPUS
  {0, 0, 0, 8, 42, 9, 5, 5, 0, 0, 0, 0, 7,
   7, 0, 7, 0, 0, 5, 5, 0, 0, 0, 0},        // BODY_BIRD
  {0, 0, 0, 8, 42, 9, 5, 5, 0, 0, 0, 0, 7,
   7, 0, 7, 0, 0, 5, 5, 0, 0, 0, 0},        // BODY_BAT
  {0, 0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 0, 10,
   10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},       // BODY_TREE
  {0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},        // BODY_PARASITE
  {0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},        // BODY_SLIME
  {0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},        // BODY_ORB
  {0, 0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 0, 10,
   10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},       // BODY_VEGGIE
  {0, 0, 0, 2, 50, 3, 5, 5, 3, 3, 5, 5, 5,
   5, 5, 0, 2, 2, 0, 0, 5, 5, 0, 0},        // BODY_DEMON
  {0, 0, 0, 6, 38, 8, 5, 5, 3, 3, 0, 0, 0,
   0, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_LION
  {0, 0, 0, 6, 38, 8, 5, 5, 3, 3, 0, 0, 0,
   0, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_FOUR_LEG
  {0, 0, 0, 6, 38, 8, 5, 5, 3, 3, 0, 0, 0,
   0, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_PIG
  {0, 0, 0, 6, 38, 8, 5, 5, 3, 3, 0, 0, 0,
   0, 7, 0, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_TURTLE
  {0, 0, 0, 6, 38, 8, 5, 5, 3, 3, 0, 0, 0,
   0, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_FOUR_HOOF
  {0, 0, 0, 4, 26, 7, 3, 3, 2, 2, 3, 3, 5,
   5, 10, 5, 0, 0, 7, 7, 0, 0, 0, 0},       // BODY_BAANTA
// unused, fing, fing, neck, body, head, leg, leg, foot, foot, hand, hand, arm
// arm, back, waist, wrist, wrist, hold, hold, leg, leg, foot, foot
  {0, 0, 0, 6, 38, 8, 5, 5, 3, 3, 0, 0, 0,
   0, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_AMPHIBEAN
  {0, 0, 0, 6, 38, 8, 5, 5, 3, 3, 0, 0, 0,
   0, 7, 0, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_FROG
  {0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},        // BODY_MIMIC
  {0, 1, 1, 2, 50, 3, 5, 5, 3, 3, 5, 5, 5,
   5, 5, 5, 2, 2, 7, 7, 0, 0, 0, 0},        // BODY_MEDUSA
  {0, 0, 0, 6, 38, 8, 5, 5, 3, 3, 0, 0, 0,
   0, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_FELINE
  {0, 0, 0, 6, 38, 8, 5, 5, 3, 3, 0, 0, 0,
   0, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_DINOSAUR
  {0, 0, 0, 6, 38, 8, 5, 5, 3, 3, 0, 0, 0,
   0, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_REPTILE
  {0, 0, 0, 6, 38, 8, 5, 5, 3, 3, 0, 0, 0,
   0, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_ELEPHANT
  {0, 0, 0, 0, 37, 9, 7, 7, 4, 4, 0, 0, 7,
   7, 7, 0, 0, 0, 0, 0, 7, 0, 4, 0},        // BODY_OTYUGH
  {0, 1, 1, 4, 26, 7, 3, 3, 2, 2, 3, 3, 5,
   5, 10, 5, 3, 3, 7, 7, 0, 0, 0, 0},       // BODY_OWLBEAR
  {0, 1, 1, 4, 26, 7, 3, 3, 2, 2, 3, 3, 5,
   5, 10, 5, 3, 3, 7, 7, 0, 0, 0, 0},       // BODY_MINOTAUR
  {0, 0, 0, 2, 50, 3, 5, 5, 3, 3, 5, 5, 5,
   5, 5, 0, 2, 2, 0, 0, 0, 0, 0, 0},        // BODY_GOLEM
  {0, 0, 0, 0, 65, 19, 0, 0, 0, 0, 0, 0, 8,
   8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},        // BODY_COATL
  {0, 1, 1, 4, 26, 7, 3, 3, 2, 2, 3, 3, 5,
   5, 12, 4, 3, 3, 5, 5, 3, 3, 2, 2},       // BODY_SIMAL
  {0, 0, 0, 6, 38, 8, 5, 5, 3, 3, 0, 0, 7,
   7, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_PEGASUS
  {0, 0, 0, 0, 15, 25, 5, 5, 0, 0, 0, 0, 0,
   0, 5, 25, 0, 0, 0, 0, 5, 5, 5, 5},       // BODY_ANT
  {0, 0, 0, 6, 38, 8, 5, 5, 3, 3, 0, 0, 7,
   7, 7, 5, 0, 0, 2, 2, 5, 5, 3, 3},        // BODY_WYVELIN
};
// unused, fing, fing, neck, body, head, leg, leg, foot, foot, hand, hand, arm
// arm, back, waist, wrist, wrist, hold, hold, leg, leg, foot, foot
