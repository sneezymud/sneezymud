//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// Garble.cc
// Code for changing how players talk
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "garble.h"

// Garble functions - defined here so peeps dont reference them outside of this file
sstring garble_blahblah(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_PG13filter(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_drunk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_sign(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_innuendo(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_excited(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_glubglub(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_ghost(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_wahwah(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);


TGarble GarbleData[GARBLE_MAX] = {
  { "innuendo", "Makes the character end sentences, if you know what I mean", false, GARBLE_SCOPE_EVERYONE, garble_innuendo, SPEECH_FLAG_VERBAL },
  { "excited", "Makes the character end phrases with exclamations!!", false, GARBLE_SCOPE_EVERYONE, garble_excited, SPEECH_FLAG_VERBAL },
  { "sign", "Applies to signing, if the character's skill level < maxed", true, GARBLE_SCOPE_EVERYONE, garble_sign, SPEECH_FLAG_SIGN },
  { "ghost", "Makes the character talk OOooooo scary!", false, GARBLE_SCOPE_EVERYONE, garble_ghost, SPEECH_FLAG_VERBAL },
  { "wahwah", "Makes the character cry like a little baby", false, GARBLE_SCOPE_EVERYONE, garble_wahwah, SPEECH_FLAG_VERBAL },
  { "blahblah", "Interjects blah blah into the player's shouts", false, GARBLE_SCOPE_EVERYONE, garble_blahblah, SPEECH_FLAG_SHOUT },
  { "drunk", "Makes the character talk (and view room) drunkenly", true, GARBLE_SCOPE_EVERYONEANDSELF, garble_drunk, SPEECH_FLAG_VERBAL | SPEECH_FLAG_ROOMDESC },
  { "pg13out", "Makes the character tone down their swear words (to other people)", true, GARBLE_SCOPE_EVERYONE, garble_PG13filter, SPEECH_FLAG_NONWRITE },
  { "pg13in", "Makes the character not get or say any swear words", true, GARBLE_SCOPE_INDIVIDUAL, garble_PG13filter, SPEECH_FLAG_NONWRITE },
  { "glubglub", "Makes the character say glub glub", true, GARBLE_SCOPE_EVERYONE, garble_glubglub, SPEECH_FLAG_VERBAL }
};

// gets the garbles that will apply to this character (adds automatic ones)
int TBeing::getGarbles(TBeing *to) const
{
  int garbleFlags = my_garbleFlags;

  if (getCond(DRUNK) >= 9)
    garbleFlags |= GARBLE_FLAG_DRUNK;

  if (getSkillValue(SKILL_SIGN) < MAX_SKILL_LEARNEDNESS)
    garbleFlags |= GARBLE_FLAG_SIGN;

  if ((roomp->isUnderwaterSector() || hasDisease(DISEASE_DROWNING)) &&
      !isImmortal() && !isShopkeeper() && !isAffected(AFF_WATERBREATH) &&
      (!to || !to->isImmortal()))
  {
    garbleFlags |= GARBLE_FLAG_GLUBGLUB;
  }

  if (desc && IS_SET(desc->autobits, AUTO_PG13))
    garbleFlags |= GARBLE_FLAG_PG13IN;

  if (to && to->desc && IS_SET(to->desc->autobits, AUTO_PG13))
    garbleFlags |= GARBLE_FLAG_PG13OUT;

  if (hasQuestBit(TOG_BLAHBLAH))
    garbleFlags |= GARBLE_FLAG_BLAHBLAH;

  return garbleFlags;
}

// toggles whatever garble on this character - note only blahblah is persisted
// and many are set automatically anyway (sign, drunk, pg13, glubglub)
// adding an automatic will just make it always on for that character
// toggling off an automatic will turn it off, unless they are in a situation where they'd
// get it on automatically (like say they were drunk and you toggled off garble_trunk)
int TBeing::toggleGarble(GARBLETYPE garble)
{
  int garbles = my_garbleFlags;

  if (hasQuestBit(TOG_BLAHBLAH))
    garbles |= GARBLE_FLAG_BLAHBLAH;

  garbles ^= (1<<garble);

  my_garbleFlags = garbles;

  if ((my_garbleFlags & GARBLE_FLAG_PG13OUT) && desc && !IS_SET(desc->autobits, AUTO_PG13))
    SET_BIT(desc->autobits, AUTO_PG13);
  else if (!(my_garbleFlags & GARBLE_FLAG_PG13OUT) && desc && IS_SET(desc->autobits, AUTO_PG13))
    REMOVE_BIT(desc->autobits, AUTO_PG13);

  if ((my_garbleFlags & GARBLE_FLAG_BLAHBLAH) && !hasQuestBit(TOG_BLAHBLAH))
    setQuestBit(TOG_BLAHBLAH);
  else if (!(my_garbleFlags & GARBLE_FLAG_BLAHBLAH) && hasQuestBit(TOG_BLAHBLAH))
    remQuestBit(TOG_BLAHBLAH);

  return my_garbleFlags;
}

// applies all of the proper garbles
sstring TBeing::garble(TBeing *to, const sstring &arg, SPEECHTYPE speechType, GARBLESCOPE garbleScope) const
{
  if (arg.empty())
    return "";

  sstring garbled = arg;
  int garbleFlags = getGarbles(to);

  // no garbles if you are telling to an immortal
  // no garbles applied? just return the string
  if (!garbleFlags || (to && to->isImmortal()))
    return garbled;

  // run all of my applied garbles, in order
  // check if garble applies to this speechtype
  for(int iGarble = 0;iGarble < GARBLE_MAX;iGarble++)
  {
    // skip if this garble isnt included from the flags
    if (!(1<<iGarble & garbleFlags))
      continue;

    // skip this garble if it is not scoped to our target scope
    if (!(garbleScope & GarbleData[iGarble].scope))
      continue;

    // skip if this speechtype doesnt match the given garble's set of speechtypes
    if (!((1<<speechType) & GarbleData[iGarble].speechFlags))
      continue;

    garbled = GarbleData[iGarble].garbleFunction(this, to, garbled, speechType);
  }

  return garbled;
}

// Deal with whiney bitches
sstring garble_blahblah(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  sstring obuf, buf, blah, word;
  unsigned int loc;
  buf=obuf=arg;

  for(int i=0;!arg.word(i).empty();++i){
    word=arg.word(i);

    // remove color codes at the beginning
    while(word.length()>2 && word[0]=='<' && word[2]=='>'){
      word.erase(0, 3);
    }

    // make sure we have something left to play with
    if(word.length()<2)
      continue;

    // find punctuation at the end of the word and remove
    for(loc=word.length()-1;loc>0;--loc){
      if(isalpha(word[loc]) || word[loc]=='>')
  break;
    }
    word.erase(loc, word.length()-loc);

    // make sure we have something left to play with
    if(word.length()<2)
      continue;

    // swap out with a random word sometimes
    if (::number(0, 4)) {
      blah="blah";
    } else {
      blah=word;
    }

    if(isupper(word[0]))
      blah[0]=toupper(blah[0]);

    // replace the original word in obuf with whitespace
    // replace the original word in buf with the new word
    loc=obuf.find(word, 0);
    if(loc != sstring::npos){
      obuf.erase(loc, word.length());
      obuf.insert(loc, blah.length(), ' ');
      buf.erase(loc, word.length());
      buf.insert(loc, blah);
    }
  }
  return buf;
}

// removes swears and bad stuff for the recipient
sstring garble_PG13filter(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  sstring output, word, ilname;

  for(int i=0; !arg.word(i).empty();++i)
  {
    word = arg.word(i);
    for(int j=0; j < 25; j++)
    {
      ilname = illegalnames[j];
      if(ilname[0] == '*')
        ilname.erase(0, 1);
      if(!((int)(word.find(ilname, 0)) >= 0))
        continue;

      // found a bad word, redact it and then break
      for(int k=0; k < (int)word.size(); k++)
      {
        const static char* redact[] = { "!", "@", "#", "$", "%", "^", "&", "*", "|", "?" };
        unsigned int random = ::number(cElements(redact)-1, 0);
        random %= cElements(redact); // not sure why this is needed?
        word.replace(k, 1, redact[random]);
      }
      break;
    }
    output += word;
    output += ' ';
  }
  output.erase((output.length()-1), 1);
  return (output);
}

// Make drunk people garble their words!
sstring garble_drunk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  int chance = from->getCond(DRUNK);
  sstring obuf, buf, latin, word;
  unsigned int loc;
  buf=obuf=arg;

  // first, lets turn things into pig latin, word by word
  for(int i=0;!arg.word(i).empty();++i)
  {
    word=arg.word(i);

    // remove color codes at the beginning
    while(word.length()>2 && word[0]=='<' && word[2]=='>')
      word.erase(0, 3);

    // make sure we have something left to play with
    if(word.length() < 2)
      continue;

    // find punctuation at the end of the word and remove
    for(loc=word.length()-1;loc > 0;--loc)
      if(isalpha(word[loc]) || word[loc]=='>')
        break;
    word.erase(loc, word.length()-loc);

    // make sure we have something left to play with
    if(word.length()<2)
      continue;

    // swap out with a random word sometimes
    if (::number(0, chance) >= 14)
      latin=RandomWord();
    else
      latin=word;

    // pig latinize sometimes
    if (::number(0, chance) >= 10)
    {
      if(isupper(word[0]))
        latin[0]=tolower(word[0]);
      latin = fmt("%s%cay") % latin.substr(1,latin.length()-1) % latin[0];
    }

    if(isupper(word[0]))
      latin[0]=toupper(latin[0]);

    // replace the original word in obuf with whitespace
    // replace the original word in buf with the new word
    loc=obuf.find(word, 0);
    if(loc != sstring::npos)
    {
      obuf.erase(loc, word.length());
      obuf.insert(loc, latin.length(), ' ');
      buf.erase(loc, word.length());
      buf.insert(loc, latin);
    }
  } // end for i
  
  // change some letters randomly
  for(unsigned int i=0;i<buf.length()-1;++i)
  {
    // skip color codes
    if(buf[i-1]=='<' && buf[i+1]=='>')
      continue;

    if (::number(0, chance) >= 18)
    {
      switch (buf[i])
      {
        case 'a':
        case 'e':
        case 'i':
        case 'o':
        case 'u':
        case 'A':
        case 'E':
        case 'I':
        case 'O':
        case 'U':
          break;
        case 'z':
        case 'Z':
          buf[i] = 'y';
          break;
        default:
          if (isalpha(buf[i]))
            (buf[i])++;
          break;
      }
    }
  } // end for i
  return buf;
}

// This garble applied to signing, used to make the sign look worse based on skill
// returns the garbled string
sstring garble_sign(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  sstring word, garble;
  sstring whitespace = " \f\n\r\t\v";
  unsigned int pos = 0;
  TBeing * tFrom = const_cast<TBeing *>(from); // sorry, but we want to check skill in here

  // work through the arg, word by word.  if you fail your
  //  skill roll, the word comes out garbled
  while(pos!=sstring::npos)
  {
    word = arg.substr(pos, arg.find_first_of(whitespace,pos) - pos);
    if (tFrom->bSuccess(tFrom->getSkillValue(SKILL_SIGN) - garble.length(), SKILL_SIGN))
      garble += word;
    else
      garble += RandomWord();

    // skip past the whitespace to the beginning of the next word
    pos = arg.find_first_of(whitespace, pos);
    if (pos != sstring::npos)
      garble += arg.substr(pos, arg.find_first_not_of(whitespace, pos) - pos);
    pos = arg.find_first_not_of(whitespace, pos);
  }

  return garble;
}

// ends sentences with a little innuendo, if you know what I mean.
sstring garble_innuendo(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  sstring buf = arg;
  buf.trim();
  int end = buf.length() - 1;
  if (end < 0)
    return buf;
  char lastChar = buf[end];
  if (lastChar != '!' && lastChar != '?')
    lastChar = '.';

    // remove all trailing punctuation
  string::size_type st = arg.find_last_not_of("?!,./;:'\"]}[{\\|=+-_`~@#$%^&*()");
  if (st != string::npos)
    buf.erase(st+1);

  buf += ", if you know what I mean";
  buf += lastChar;
  return buf;
}

// ends sentences with exclamation points and randomly ALL CAPS
sstring garble_excited(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  sstring buf = arg;
  byte exclamations = 1;
  buf.trim();
  if (!number(0,9))
    buf = buf.upper();

  // remove trailing punctuation
  string::size_type st = arg.find_last_not_of(",./;:'\"]}[{\\|=+-_`~@#$%^&*()");
  if (buf[st] == '?')
    exclamations += 2;
  else if (st != string::npos)
    buf.erase(st+1);

  // add us some exclamations
  exclamations += number(0, 1);
  while (exclamations-- >= 0)
   buf += '!';
  return buf;
}

// used when underwater or drowning - the essential glub glub glub!
sstring garble_glubglub(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  return "Glub glub glub.";
}

// makes you talk like a ghost
sstring garble_ghost(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  sstring buf, word;
  static const sstring whitespace = " \f\n\r\t\v";
  unsigned int pos = 0;

  // work through the arg, word by word and make into ghosty sounds
  while(pos != sstring::npos)
  {
    word = arg.substr(pos, arg.find_first_of(whitespace,pos) - pos);
    for(unsigned int iCh = 0;iCh < word.length();iCh++)
      word[iCh] = (iCh < word.length()/3) ? 'O' : 'o';

    buf += word;

    // skip past the whitespace to the beginning of the next word
    pos = arg.find_first_of(whitespace, pos);
    if (pos != sstring::npos)
      buf += arg.substr(pos, arg.find_first_not_of(whitespace, pos) - pos);
    pos = arg.find_first_not_of(whitespace, pos);
  }
  return buf;
}

// makes you whine like a baby!
sstring garble_wahwah(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  sstring buf, word;
  static const sstring whitespace = " \f\n\r\t\v";
  unsigned int pos = 0;

  // work through the arg, word by word.  Convert to Wa+h
  while(pos != sstring::npos)
  {
    word = arg.substr(pos, arg.find_first_of(whitespace,pos) - pos);
    int iLast = word.length() - 1;
    if (word.length() > 1)
      word[0] = (word[0] == toupper(word[0])) ? 'W' : 'w';

    if (word.length() > 2)
      word[iLast] = (word[iLast] == toupper(word[iLast])) ? 'H' : 'h';
    else
      iLast++;

    for (int iInside = 1;iInside < iLast;iInside++)
      word[iInside] = (word[iInside] == toupper(word[iInside])) ? 'A' : 'a';

    buf += word;

    // skip past the whitespace to the beginning of the next word
    pos = arg.find_first_of(whitespace, pos);
    if (pos != sstring::npos)
      buf += arg.substr(pos, arg.find_first_not_of(whitespace, pos) - pos);
    pos = arg.find_first_not_of(whitespace, pos);
  }
  return buf;
}


const sstring RandomWord()
{
  static sstring str[] =
  {
    "argle",
    "bargle",
    "glop",
    "glyph",
    "hussamah",                 // 5 
    "rodina",
    "mustafah",
    "angina",
    "the",
    "fribble",                  // 10 */
    "fnort",
    "frobozz",
    "zarp",
    "ripple",
    "yrk",                      // 15 */
    "yid",
    "yerf",
    "oork",
    "beavis",
    "butthead",                 // 20 */
    "rod",
    "johnson",
    "tool",
    "ftagn",
    "hastur",                   // 25 */
    "brob",
    "gnort",
    "lram",
    "truck",
    "kill",                     // 30 */
    "cthulhu",
    "huzzah",
    "fish",
    "chicken",
    "summah",                   // 35 */
    "hummah",
    "cookies",
    "stan",
    "will",
    "wadapatang",               // 40 */
    "pterodactyl",
    "frob",
    "yuma",
    "gumma",
    "lo-pan",                   // 45 */
    "sushi",
    "yaya",
    "yoyodine",
    "your",
    "mother"                    // 50 */
  };
  return (str[number(0, (cElements(str)-1))]);
}

const sstring RandomVerb()
{
  // guaranteed insensible
  static sstring str[] =
  {
    "hoist", 
    "pinch", 
    "decorate", 
    "besmirch", 
    "conflagrate",              // 5
    "snook", 
    "pilfer", 
    "parlay", 
    "flummox", 
    "tender",                   // 10
    "archive", 
    "hail", 
    "ploot", 
    "castigate", 
    "micturate",                // 15
    "tuck", 
    "strap", 
    "absolve", 
    "flagellate", 
    "abnegate",                 // 20
    "bowdlerize", 
    "mesmerize", 
    "simonize", 
    "levitate", 
    "orate",                    // 25
    "ration", 
    "finesse", 
    "enrapture", 
    "declaim", 
    "canoodle",                 // 30
    "atomize", 
    "massage", 
    "poeticize", 
    "criticize", 
    "harbor",                   // 35
    "unravel", 
    "demystify", 
    "lobby", 
    "swive", 
    "scrump",                   // 40
    "deride", 
    "remune", 
    "addle", 
    "brindle", 
    "coddle",                   // 45
    "succor", 
    "embarrass", 
    "dominate", 
    "envalorize", 
    "encapitate"                // 50
  };
  return (str[number(0, (cElements(str)-1))]);
}



