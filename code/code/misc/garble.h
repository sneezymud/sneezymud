//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// Garble.h
// Code for changing how players talk
//////////////////////////////////////////////////////////////////////////

#ifndef __GARBLE_H
#define __GARBLE_H

#include "sstring.h"

#ifndef cElements
#define cElements(x) (sizeof(x)/sizeof(x[0]))
#endif



// flags for a garble's scope
// consider a scope for when a garble should activate, depending on
// who the garble is intended for
// for instance: if I am sending to a particlar person vs. all people
// I want the drunk garble to be applied when i garbling for everyone,
// but the pg13 garble should only apply per individual this way I can
// run the garble operation once for getting the drunk setting, and a
// second time for each person depending on their settings

// the struct used to hold the particular garble information
class Garble {
 public:
  // combined to designate multiple types of speech (say to detemine
  // which are valid to a garble)
  static const unsigned int SPEECH_FLAG_SAY;
  static const unsigned int SPEECH_FLAG_ASK;
  static const unsigned int SPEECH_FLAG_WHISPER;
  static const unsigned int SPEECH_FLAG_SHOUT;
  static const unsigned int SPEECH_FLAG_TELL;
  static const unsigned int SPEECH_FLAG_GROUPTELL;
  static const unsigned int SPEECH_FLAG_COMMUNE;
  static const unsigned int SPEECH_FLAG_SIGN;
  static const unsigned int SPEECH_FLAG_WRITE;
  static const unsigned int SPEECH_FLAG_WIZNET;
  static const unsigned int SPEECH_FLAG_ROOMDESC;
  static const unsigned int SPEECH_FLAG_EMOTE;
  static const unsigned int SPEECH_FLAG_VERBAL;
  static const unsigned int SPEECH_FLAG_VERBALEM;
  static const unsigned int SPEECH_FLAG_NONVERBAL;
  static const unsigned int SPEECH_FLAG_LOCAL;
  static const unsigned int SPEECH_FLAG_DIRECTED;
  static const unsigned int SPEECH_FLAG_NONWRITE;
  
  // flags for passing which garbles should be applied
  static const unsigned int TYPE_FLAG_INNUENDO;
  static const unsigned int TYPE_FLAG_EXCITED;
  static const unsigned int TYPE_FLAG_SIGN;
  static const unsigned int TYPE_FLAG_GHOST;
  static const unsigned int TYPE_FLAG_WAHWAH;
  static const unsigned int TYPE_FLAG_BLAHBLAH;
  static const unsigned int TYPE_FLAG_DRUNK;
  static const unsigned int TYPE_FLAG_PG13OUT;
  static const unsigned int TYPE_FLAG_PG13IN;
  static const unsigned int TYPE_FLAG_GLUBGLUB;
  static const unsigned int TYPE_FLAG_PIRATE;
  static const unsigned int TYPE_FLAG_FRESH;
  static const unsigned int TYPE_FLAG_FISHTALK;
  static const unsigned int TYPE_FLAG_LOLCATS;
  static const unsigned int TYPE_FLAG_VAMPIRE;
  static const unsigned int TYPE_FLAG_IGOR;
  static const unsigned int TYPE_FLAG_OLDDRUNK;
  static const unsigned int TYPE_FLAG_IRISH;
  static const unsigned int TYPE_FLAG_TROLLTALK;
  static const unsigned int TYPE_FLAG_FROGTALK;
  static const unsigned int TYPE_FLAG_BIRDTALK;
  static const unsigned int TYPE_FLAG_GUTTER;
  static const unsigned int TYPE_FLAG_TROGTALK;
  static const unsigned int TYPE_FLAG_CRAZYFROG;
  static const unsigned int TYPE_FLAG_MAX;


  // all the different ways you can communicate with someone
  enum SPEECHTYPE {
    SPEECH_SAY,
    SPEECH_ASK,
    SPEECH_WHISPER,
    SPEECH_SHOUT,
    SPEECH_TELL,
    SPEECH_TELEPATHY,
    SPEECH_GROUPTELL,
    SPEECH_COMMUNE,
    SPEECH_SIGN,
    SPEECH_WRITE,
    SPEECH_WIZNET,
    SPEECH_ROOMDESC,
    SPEECH_EMOTE,
    
    SPEECH_MAX
  };

  enum SCOPE {
    SCOPE_EVERYONE = 0x1,
    SCOPE_INDIVIDUAL = 0x2,
    SCOPE_SELF = 0x4,
    SCOPE_EVERYONEANDSELF = 0x5,
    SCOPE_ALL = 0xFFFFFFFF
  };


  // The different types of garbles Order here matters - they are
  // ordered by execution.  from 0 (first run) to MAX (last run).  So
  // for instance PG13 & glubglub should probably always be last.
  enum TYPE {
    TYPE_INNUENDO = 0,
    TYPE_EXCITED,
    TYPE_SIGN,
    TYPE_GHOST,
    TYPE_WAHWAH,
    TYPE_BLAHBLAH,
    TYPE_DRUNK,
    TYPE_PG13OUT,
    TYPE_PG13IN,
    TYPE_GLUBGLUB,
    TYPE_PIRATE,
    TYPE_FRESH,
    TYPE_FISHTALK,
    TYPE_LOLCATS,
    TYPE_VAMPIRE,
    TYPE_IGOR,
    TYPE_OLDDRUNK,
    TYPE_IRISH,
    TYPE_TROLLTALK,
    TYPE_FROGTALK,
    TYPE_BIRDTALK,
    TYPE_GUTTER,
    TYPE_TROGTALK,
    TYPE_CRAZYFROG,
    // Add new GARBLES here, then make a TYPE_FLAG, and add to
    // GarbleData array
    TYPE_MAX
  };


  const char * const name; // used by egotrip
  const char * const description; // used by egotrip
  const bool automatic; // used by egotrip
  const SCOPE scope;
  sstring (*garbleFunction)(const TBeing *, TBeing *, const sstring &, Garble::SPEECHTYPE);
  const int speechFlags;
};

extern Garble GarbleData[Garble::TYPE_MAX];

// utility functions
extern const sstring RandomWord();
extern const sstring RandomVerb();


// Make drunk people garble their words!
extern sstring garble_olddrunk(const TBeing *, const TBeing *, const sstring &, Garble::SPEECHTYPE);

#endif // __GARBLE_H
