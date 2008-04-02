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

// all the different ways you can communicate with someone
enum SPEECHTYPE
{
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

// combined to designate multiple types of speech (say to detemine which are valid to a garble)
#define SPEECH_FLAG_SAY       (1<<SPEECH_SAY)
#define SPEECH_FLAG_ASK       (1<<SPEECH_ASK)
#define SPEECH_FLAG_WHISPER   (1<<SPEECH_WHISPER)
#define SPEECH_FLAG_SHOUT     (1<<SPEECH_SHOUT)
#define SPEECH_FLAG_TELL      (1<<SPEECH_TELL)
#define SPEECH_FLAG_GROUPTELL (1<<SPEECH_GROUPTELL)
#define SPEECH_FLAG_COMMUNE   (1<<SPEECH_COMMUNE)
#define SPEECH_FLAG_SIGN      (1<<SPEECH_SIGN)
#define SPEECH_FLAG_WRITE     (1<<SPEECH_WRITE)
#define SPEECH_FLAG_WIZNET    (1<<SPEECH_WIZNET)
#define SPEECH_FLAG_ROOMDESC  (1<<SPEECH_ROOMDESC)
#define SPEECH_FLAG_EMOTE     (1<<SPEECH_EMOTE)

#define SPEECH_FLAG_VERBAL    (SPEECH_FLAG_SAY|SPEECH_FLAG_ASK|SPEECH_FLAG_WHISPER|SPEECH_FLAG_TELL|SPEECH_FLAG_GROUPTELL|SPEECH_FLAG_SHOUT)
#define SPEECH_FLAG_VERBALEM  (SPEECH_FLAG_VERBAL|SPEECH_FLAG_EMOTE)
#define SPEECH_FLAG_NONVERBAL (SPEECH_FLAG_COMMUNE|SPEECH_FLAG_SIGN|SPEECH_FLAG_WRITE|SPEECH_FLAG_WIZNET|SPEECH_FLAG_EMOTE)
#define SPEECH_FLAG_LOCAL     (SPEECH_FLAG_SAY|SPEECH_FLAG_ASK|SPEECH_FLAG_WHISPER|SPEECH_FLAG_SIGN|SPEECH_FLAG_EMOTE)
#define SPEECH_FLAG_DIRECTED  (SPEECH_FLAG_TELL|SPEECH_FLAG_ASK|SPEECH_FLAG_WHISPER)
#define SPEECH_FLAG_NONWRITE  (SPEECH_FLAG_SAY|SPEECH_FLAG_ASK|SPEECH_FLAG_WHISPER|SPEECH_FLAG_TELL|SPEECH_FLAG_GROUPTELL|SPEECH_FLAG_COMMUNE|SPEECH_FLAG_SIGN|SPEECH_FLAG_SHOUT|SPEECH_FLAG_WIZNET|SPEECH_FLAG_EMOTE)

// The different types of garbles
// Order here matters - they are ordered by execution.
// from 0 (first run) to MAX (last run).  So for instance PG13 & glubglub should probably
// always be last.
enum GARBLETYPE
{
  GARBLE_INNUENDO = 0,
  GARBLE_EXCITED,
  GARBLE_SIGN,
  GARBLE_GHOST,
  GARBLE_WAHWAH,
  GARBLE_BLAHBLAH,
  GARBLE_DRUNK,
  GARBLE_PG13OUT,
  GARBLE_PG13IN,
  GARBLE_GLUBGLUB,
  GARBLE_PIRATE,
  GARBLE_FRESH,
  GARBLE_FISHTALK,
  GARBLE_LOLCATS,
  GARBLE_VAMPIRE,
  GARBLE_IGOR,
  GARBLE_OLDDRUNK,
  GARBLE_IRISH,
  GARBLE_TROLLTALK,
  GARBLE_FROGTALK,
  GARBLE_BIRDTALK,
  GARBLE_GUTTER,
  GARBLE_TROGTALK,
  GARBLE_CRAZYFROG,
  // Add new GARBLES here, then make a GARBLE_FLAG, and add to GarbleData array

  GARBLE_MAX
};

// flags for passing which garbles should be applied
#define GARBLE_FLAG_INNUENDO  (1<<GARBLE_INNUENDO)
#define GARBLE_FLAG_EXCITED   (1<<GARBLE_EXCITED)
#define GARBLE_FLAG_SIGN      (1<<GARBLE_SIGN)
#define GARBLE_FLAG_GHOST     (1<<GARBLE_GHOST)
#define GARBLE_FLAG_WAHWAH    (1<<GARBLE_WAHWAH)
#define GARBLE_FLAG_BLAHBLAH  (1<<GARBLE_BLAHBLAH)
#define GARBLE_FLAG_DRUNK     (1<<GARBLE_DRUNK)
#define GARBLE_FLAG_PG13OUT   (1<<GARBLE_PG13OUT)
#define GARBLE_FLAG_PG13IN    (1<<GARBLE_PG13IN)
#define GARBLE_FLAG_GLUBGLUB  (1<<GARBLE_GLUBGLUB)
#define GARBLE_FLAG_PIRATE    (1<<GARBLE_PIRATE)
#define GARBLE_FLAG_FRESH     (1<<GARBLE_FRESH)
#define GARBLE_FLAG_FISHTALK  (1<<GARBLE_FISHTALK)
#define GARBLE_FLAG_LOLCATS   (1<<GARBLE_LOLCATS)
#define GARBLE_FLAG_VAMPIRE   (1<<GARBLE_VAMPIRE)
#define GARBLE_FLAG_IGOR      (1<<GARBLE_IGOR)
#define GARBLE_FLAG_OLDDRUNK  (1<<GARBLE_OLDDRUNK)
#define GARBLE_FLAG_IRISH     (1<<GARBLE_IRISH)
#define GARBLE_FLAG_TROLLTALK (1<<GARBLE_TROLLTALK)
#define GARBLE_FLAG_FROGTALK  (1<<GARBLE_FROGTALK)
#define GARBLE_FLAG_BIRDTALK  (1<<GARBLE_BIRDTALK)
#define GARBLE_FLAG_GUTTER    (1<<GARBLE_GUTTER)
#define GARBLE_FLAG_TROGTALK  (1<<GARBLE_TROGTALK)
#define GARBLE_FLAG_CRAZYFROG (1<<GARBLE_CRAZYFROG)

#define GARBLE_FLAG_MAX       (1<<GARBLE_MAX)

// flags for a garble's scope
// consider a scope for when a garble should activate, depending on who the garble is intended for
// for instance: if I am sending to a particlar person vs. all people
// I want the drunk garble to be applied when i garbling for everyone,
// but the pg13 garble should only apply per individual
// this way I can run the garble operation once for getting the drunk setting, and a second time for each person
// depending on their settings
enum GARBLESCOPE
{
  GARBLE_SCOPE_EVERYONE = 0x1,
  GARBLE_SCOPE_INDIVIDUAL = 0x2,
  GARBLE_SCOPE_SELF = 0x4,
  GARBLE_SCOPE_EVERYONEANDSELF = 0x5,
  GARBLE_SCOPE_ALL = 0xFFFFFFFF
};

// the struct used to hold the particular garble information
typedef struct _TGarble 
{
  const char * const name; // used by egotrip
  const char * const description; // used by egotrip
  const bool automatic; // used by egotrip
  const GARBLESCOPE scope;
  sstring (*garbleFunction)(const TBeing *, const TBeing *, const sstring &, SPEECHTYPE);
  const int speechFlags;
} TGarble;

extern TGarble GarbleData[GARBLE_MAX];

// utility functions
extern const sstring RandomWord();
extern const sstring RandomVerb();



#endif // __GARBLE_H
