//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// Garble.cc
// Code for changing how players talk
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "garble.h"

// utility functions defined below
const sstring RandomPhrase(bool allowRecursivePhrase);

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
sstring garble_pirate(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_freshprince(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_fishtalk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_lolcats(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_vampire(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_igor(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_olddrunk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_irish(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_trolltalk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_frogtalk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_birdtalk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_gutter(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);
sstring garble_trogtalk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType);

TGarble GarbleData[GARBLE_MAX] = {
  { "innuendo", "Makes the character end sentences, if you know what I mean", false, GARBLE_SCOPE_EVERYONE, garble_innuendo, SPEECH_FLAG_VERBAL },
  { "excited", "Makes the character end phrases with exclamations!!", false, GARBLE_SCOPE_EVERYONE, garble_excited, SPEECH_FLAG_VERBAL },
  { "sign", "Applies to signing, if the character's skill level < maxed", true, GARBLE_SCOPE_EVERYONE, garble_sign, SPEECH_FLAG_SIGN },
  { "ghost", "Makes the character talk OOooooo scary!", false, GARBLE_SCOPE_EVERYONE, garble_ghost, SPEECH_FLAG_VERBALEM },
  { "wahwah", "Makes the character cry like a little baby", false, GARBLE_SCOPE_EVERYONE, garble_wahwah, SPEECH_FLAG_VERBALEM },
  { "blahblah", "Interjects blah blah into the player's shouts", false, GARBLE_SCOPE_EVERYONE, garble_blahblah, SPEECH_FLAG_SHOUT },
  { "drunk", "Makes the character talk (and view room) drunkenly", true, GARBLE_SCOPE_EVERYONEANDSELF, garble_drunk, SPEECH_FLAG_VERBAL | SPEECH_FLAG_ROOMDESC },
  { "pg13out", "Makes the character tone down their swear words (to other people)", true, GARBLE_SCOPE_INDIVIDUAL, garble_PG13filter, SPEECH_FLAG_NONWRITE },
  { "pg13all", "Makes the character not get or say any swear words", true, GARBLE_SCOPE_EVERYONE, garble_PG13filter, SPEECH_FLAG_NONWRITE },
  { "glubglub", "Makes the character say glub glub", true, GARBLE_SCOPE_EVERYONE, garble_glubglub, SPEECH_FLAG_VERBALEM },
  { "pirate", "Makes the character talk like a pirate", false, GARBLE_SCOPE_EVERYONE, garble_pirate, SPEECH_FLAG_VERBAL },
  { "freshprince", "Makes the character do the fresh prince of Brightmoon rap", false, GARBLE_SCOPE_EVERYONE, garble_freshprince, SPEECH_FLAG_SAY },
  { "fishtalk", "Makes the character talk like they are a fish to non-fish", false, GARBLE_SCOPE_INDIVIDUAL, garble_fishtalk, SPEECH_FLAG_VERBALEM },
  { "lolcats", "Makes the character do a lolcats talk (used for gnolls)", false, GARBLE_SCOPE_INDIVIDUAL, garble_lolcats, SPEECH_FLAG_VERBALEM },
  { "vampire", "Makes the character talk like a vampire", false, GARBLE_SCOPE_EVERYONE, garble_vampire, SPEECH_FLAG_VERBALEM },
  { "igor", "Makes the character talk like an igor", false, GARBLE_SCOPE_EVERYONE, garble_igor, SPEECH_FLAG_VERBALEM },
  { "olddrunk", "Makes the character talk in the old-school drunk code", false, GARBLE_SCOPE_EVERYONEANDSELF, garble_olddrunk, SPEECH_FLAG_VERBAL | SPEECH_FLAG_ROOMDESC },
  { "irish", "Makes the character talk with an irish accent", false, GARBLE_SCOPE_EVERYONE, garble_irish, SPEECH_FLAG_VERBALEM },
  { "trolltalk", "Makes the character talk like a troll to non-trolls", false, GARBLE_SCOPE_INDIVIDUAL, garble_trolltalk, SPEECH_FLAG_VERBALEM },
  { "frogtalk", "Makes the character talk like a frogman to non-frogs", false, GARBLE_SCOPE_INDIVIDUAL, garble_frogtalk, SPEECH_FLAG_VERBALEM },
  { "birdtalk", "Makes the character talk like an aarakocra to non-aarakocra", false, GARBLE_SCOPE_INDIVIDUAL, garble_birdtalk, SPEECH_FLAG_VERBALEM },
  { "gutter", "Makes the character talk a bit cockney (used for goblins and orcs)", false, GARBLE_SCOPE_INDIVIDUAL, garble_gutter, SPEECH_FLAG_VERBALEM },
  { "trogtalk", "Makes the character talk like a troglodyte to non-trogs", false, GARBLE_SCOPE_INDIVIDUAL, garble_trogtalk, SPEECH_FLAG_VERBALEM },
  { "crazyfrog", "Like frogtalk, except it effects rooms as well", false, GARBLE_SCOPE_EVERYONEANDSELF, garble_frogtalk, SPEECH_FLAG_VERBALEM | SPEECH_FLAG_ROOMDESC},
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
      getImmunity(IMMUNE_SUFFOCATION) < 100 && (!to || !to->isImmortal()))
  {
    garbleFlags |= GARBLE_FLAG_GLUBGLUB;
  }

  if (desc && IS_SET(desc->autobits, AUTO_PG13))
    garbleFlags |= GARBLE_FLAG_PG13IN;

  if (to && to->desc && IS_SET(to->desc->autobits, AUTO_PG13))
    garbleFlags |= GARBLE_FLAG_PG13OUT;

  if (hasQuestBit(TOG_BLAHBLAH))
    garbleFlags |= GARBLE_FLAG_BLAHBLAH;

  if (to && to->getMyRace()->getGarbles() != getMyRace()->getGarbles() &&
    to->getStat(STAT_CURRENT, STAT_INT) < 180)
  {
    garbleFlags |= getMyRace()->getGarbles();
  }

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
  if (desc && IS_SET(desc->autobits, AUTO_PG13))
    garbles |= GARBLE_PG13OUT;

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

  for(int i=0;!arg.word(i).empty();++i)
  {
    word=arg.word(i);

    // remove color codes at the beginning
    while(word.length()>2 && word[0]=='<' && word[2]=='>')
      word.erase(0, 3);

    // make sure we have something left to play with
    if(word.length()<2)
      continue;

    // find punctuation at the end of the word and remove
    for(loc=word.length()-1;loc>0;--loc)
    {
      if(isalpha(word[loc]) || word[loc]=='>')
        break;
    }
    word.erase(loc, word.length()-loc);

    // make sure we have something left to play with
    if(word.length()<2)
      continue;

    // swap out with a random word sometimes
    if (::number(0, 4))
      blah="blah";
    else
      blah=word;

    if(isupper(word[0]))
      blah[0]=toupper(blah[0]);

    // replace the original word in obuf with whitespace
    // replace the original word in buf with the new word
    loc=obuf.find(word, 0);
    if(loc != sstring::npos)
    {
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
    for(int j=0; j < 24; j++) // up to but excluding "ass" (otherwise you can't say 'class', 'assist', 'seabass')
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
sstring garble_olddrunk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  int chance = from->getCond(DRUNK);
  sstring obuf, buf, latin, word;
  unsigned int loc;
  buf=obuf=arg;

  // if this garble was applied without us being drunk, assume an average drunkenness
  if (chance < 9)
    chance = 15;

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

// This garble makes the target talk like a pirate
// returns the garbled string
sstring garble_pirate(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  static const sstring pirate_replace[][2] = {
    // escape punctuation
    { ",", " *,* " },
    { ";", " *;* " },
    { ".", " *.* " },
    { "!", " *!* " },
    { "?", " *?* " },

    // first, turn prases that would get replaced into tokens
    { " are not ", " *beno* " },
    { " oh my god ", " *omg* " },
    { " omg ", " *omg* " },
    { " wtf ", " *wtf* " },
    { " wft ", " *wtf* " },
    { " what the fuck ", " *wtf* " },
    { " what's up ", " *whatup* " },
    { " whats up ", " *whatup* " },
    { " what up ", " *whatup* " },
    { " whatup ", " *whatup* " },
    { " it is ", " *itis* " },
    { " its ", " *itis* " },
    { " it's ", " *itis* " },
    { " i mean ", " *sezi* " },
    { " btw ", " *btw* " },

    // phrase fixup
    { " they're ", " they are " },
    { " theyre ", " they are " },
    { " we're ", " we are " },
    { " were ", " we are " },
    { " you're ", " you are " },
    { " youre ", " you are " },
    { " thats ", " that is " },
    { " that's ", " that is " },
    { " well *,* ", " arr *,* " },

    // apply simple word replacements
    { " is ", " be " },
    { " are ", " be " },
    { " am ", " be " },
    { " of ", " o' " },
    { " to ", " t' " },
    { " or ", " 'r " },
    { " and ", " 'n " },
    { " the ", " th' " },
    { "&", " 'n " },
    { " nd ", " 'n " },
    { " that ", " thar " },
    { " there ", " thar " },
    { " theres ", " thars " },
    { " their ", " thar " },
    { " theirs ", " thars " },
    { " ya ", " yar " },
    { " yes ", " aye " },
    { " sure ", " aye " },
    { " ok ", " aye " },
    { " okay ", " aye " },
    { " k ", " aye " },
    { " kk ", " aye " },
    { " mkay ", " yar " },
    { " m'kay ", " yar " },
    { " no ", " nay " },
    { " aren't ", " t'ain't " },
    { " arent ", " t'ain't " },
    { " aint ", " t'ain't " },
    { " ain't ", " t'ain't " },
    { " hey ", " ahoy " },
    { " hi ", " ahoy " },
    { " heya ", " ahoy " },
    { " hello ", " ahoy " },
    { " man ", " mate " },
    { " men ", " hearties " },
    { " friend ", " mate " },
    { " friends ", " hearties " },
    { " guy ", " matey " },
    { " guys ", " mateys " },
    { " she ", " th' lass " },
    { " gal ", " lass " },
    { " gals ", " lasses " },
    { " lady ", " lass " },
    { " ladies ", " lasses " },
    { " boy ", " lad " },
    { " boys ", " laddies " },
    { " girl ", " poppet " },
    { " girls ", " poppets " },
    { " chick ", " poppet " },
    { " chicks ", " poppets " },
    { " woman ", " wench " },
    { " women ", " wenches " },
    { " bud ", " matey " },
    { " buds ", " mateys " },
    { " buddy ", " matey " },
    { " buddies ", " mateys " },
    { " haha ", " bar har har " },
    { " hah ", " har " },
    { "bwa", "bar har " },
    { "haha", " har " },
    { " rofl ", " bar har har " },
    { " hehe ", " yo ho ho " },
    { " heh ", " ho " },
    { "hehe", " ho " },
    { " lolo ", " yo ho " },
    { " lol ", " har " },
    { " ass ", " arse " },
    { " asses ", " arses " },
    { " butt ", " arse " },
    { " butts ", " arses " },
    { " allright ", " all be right " },
    { " clean ", " swab " },
    { " cleaned ", " swabbed " },
    { " cleaning ", " swabbing " },
    { " wash ", " swab " },
    { " washed ", " swabbed " },
    { " washing ", " swabbing " },
    { " beer ", " rum " },
    { " booze ", " rum " },
    { " beer ", " rum " },
    { " alcahol ", " rum " },
    { " alcohol ", " rum " },
    { " alcaholic ", " rumsey " },
    { " alcoholic ", " rumsey " },
    { " wine ", " rum " },
    { " mead ", " rum " },
    { " lager ", " rum " },
    { " ale ", " rum " },
    { " spirits ", " rum " },
    { " liquor ", " rum " },
    { " vodka ", " grog " },
    { " whiskey ", " grog " },
    { " tequila ", " grog " },
    { " burbon ", " grog " },
    { " gin ", " grog " },
    { " treasure ", " booty " },
    { " money ", " booty " },
    { " cash ", " booty " },
    { " talen ", " dubloon " },
    { " talens ", " dubloons " },
    { " coin ", " dubloon " },
    { " coins ", " dubloons " },
    { " left ", " port " },
    { " forward ", " fore " },
    { " back ", " stern " },
    { " rear ", " aft " },
    { " nose ", " prow " },
    { " disease ", " scurvy " },
    { " intoxicated ", " squiffy " },
    { " drunk ", " squiffy " },
    { " drink ", " draught " },
    { " uhh ", " arr " },
    { " uh ", " arr " },
    { " oh ", " arr " },
    { " ohh ", " arr " },
    { " idea ", " idear " },
    { " boat ", " ship " },
    { " stand ", " stands " },

    // apply prefix and suffix replacements
    { "ing ", "in' " },
    { " you", " ye" },
    { " jerk", " lubber" },
    { " dork", " lubber" },
    { " loser", " lubber" },
    { " them", " 'em" },
    { " our", " 'r" },
    { " my", " me" },

    // un-tokenize the tokens for our pirate phrases
    { " *beno* ", " be no " },
    { " *omg* ", " shiver me timbers " },
    { " *wtf* ", " blow me down " },
    { " *whatup* ", " yar matey " },
    { " *itis* ", " 'tis " },
    { " *sezi* ", " says I " },
    { " *btw* ", "  " },

    // replace punctuation we escaped
    { " *,* ", "," },
    { " *;* ", ";" },
    { " *.* ", "." },
    { " *!* ", "!" },
    { " *?* ", "?" },

    // pretty us up a bit
    { " i ", " I " },
    };
  sstring garble = " ";
  garble += arg.lower();
  garble += " ";

  // do all of the string replacements, yar!
  string::size_type start = 0;
  for(unsigned int iReplace = 0; iReplace < cElements(pirate_replace);)
  {
    start = garble.find(pirate_replace[iReplace][0], start);

    // nothing found? go to the next word
    if (start == string::npos)
    {
      iReplace++;
      start = 0;
      continue;
    }

    // if we need more buffer, lets resize
    if (pirate_replace[iReplace][0].length() < pirate_replace[iReplace][1].length())
      garble.resize(garble.size() + pirate_replace[iReplace][1].length() - pirate_replace[iReplace][0].length(), ' ');

    // replace it, and step forward
    garble.replace(start, pirate_replace[iReplace][0].length(), pirate_replace[iReplace][1]);
    start += pirate_replace[iReplace][1].length();

    // if we end the replace with a space, backup one so that we can overlap word replacements
    if (pirate_replace[iReplace][1][pirate_replace[iReplace][1].length()-1] == ' ')
      start--;
  }

  // perhaps here, add in some pirate exclamations

  // now caps the first char and first of each sentence and trim
  return garble.trim().matchCase(arg);
}


// no garble is complete without a fresh prince parody (Brightmoon)
sstring garble_freshprince(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  static const sstring lyrics[] =
  {
    "Now this is the story all about how,",
    "My life got flipped, turned upside down.",
    "And I'd like to take a minute won't go past noon,",
    "I'll tell you how I became the prince of a town called Brightmoon.",

    "In low-rent Grimhaven, born and raised.",
    "On the parklands is where I spent most of my days.",
    "Chilling out, maxing, relaxing all cool,",
    "And all shooting some large rats outside of the school,",
    "When a couple of guys who were up to no good,",
    "Started making trouble in my neighborhood.",
    "I got in one little fight but my mom hates goons,",
    "And said 'you're moving with your aunte and uncle in Brightmoon'.",

    "I whistled for a trolley and when it arrived, the",
    "Horses name was 'fresh' and had a gnome for a driver.",
    "If anything I could say that this cart was doomed,",
    "But I thought naw forget it, yo holmes to Brightmoon!",

    "I pulled up to the house about seven or eight.",
    "And I yelled to the trolley 'yo, holmes smell you later'!",
    "Looked at my kingdom I'll be on top soon,",
    "To settle my throne as the prince of Brightmoon."
  };

  // this function means its global, so 2 people can have the same garble and end up rhyming together
  static unsigned int stringIndex = 0;
  if (stringIndex >= cElements(lyrics))
    stringIndex = 0;
  return lyrics[stringIndex++];
}

// makes the character talk like a fish - used for the fishman/fishmen race
// compiled by watching countless hours of He-Man clips for Mer-man speaking parts.
// Heres a spoiler: there aren't many.
sstring garble_fishtalk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  static const sstring watery[] =
  {
    "glug", "glub", "gug", "glurg",
    "gurgle", "guggle", "glubble", "glurble",
    "blug", "bluggle", "blurgle", "rgrle",
    "rglurg", "aahhrrgl", "glglrraa", "bloop",
    "gloop", "goop", "grloop",
  };
  static const sstring replace[][2] = {
    { "R", "Rr", }, // roll r's
    { "G", "Gr", }, // g sounds are growled (add r's)
    { "L", "Gl", },// l sounds are gurgled
    { "Th", "Wr", }, // th sounds are 'wr'
    { "TH", "WR", },// th sounds are 'wr'
    { "T", "W", },// t sounds are 'w'
  };
  sstring out;
  int iWord = 0;
  sstring word = arg.word(iWord);
  int chance = from ? 100 - from->plotStat(STAT_CURRENT, STAT_INT, 0, 100, 50) : 25;

  for(; !word.empty(); word = arg.word(++iWord))
  {
    if (!out.empty())
      out += ' ';
    // replace randomly a word
    if (chance/2 > number(0, 100))
    {
      char punct = word[word.length()-1];
      word = watery[number(0, cElements(watery)-1)].matchCase(word);
      // sad hack to keep trailing , ? ! . on this word
      if (ispunct(punct))
        word += punct;
    }
    else if (chance*1.3 > number(0, 100))
    {
      for(int i=0;i < (int)cElements(replace);i++)
      {
        // assume replace strings are in upper case
        word.inlineReplaceString(replace[i][0], replace[i][1]);
        word.inlineReplaceString(replace[i][0].lower(), replace[i][1].lower());
      }
    }
    out += word;
  }

  return out;
}

// submitted by Vasco - used by the troll race as its base talk
sstring garble_lolcats(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  static const sstring lolcats_replace[][2] = {
    { ",", " *,* " },
    { ";", " *;* " },
    { ".", " *.* " },
    { "!", " *!* " },
    { "?", " *?* " },

    { " am ", " iz " },
    { " are ", " iz " },
    { " am ", " iz " },
    { " is ", " iz " },
    { " you ", " u " },
    { " you're ", " ur " },
    { " youre ", " ur " },
    { " your ", " ur " },
    { " have ", " haz " },
    { " my ", " mai " },
    { " the ", " da " },
    { " this ", " diz " },
    { " that ", " dat " },
    { " them ", " dem " },
    { " doesnt ", " dont " },
    { " dont ", " dun " },
    { " doesn't ", " dont " },
    { " don't ", " dun " },
    { " myself ", " me " },

    { "tion ", "shun " },
    { "tial ", "shal " },
    { "ight ", "ite " },
    { "ever ", "evar " },
    { "pped ", "opt " },

    { "ude ", "ewd " },
    { "ith ", "if " },
    { "ies ", "eez " },
    { "ood ", "ud " },
    { "tia ", "shiay " },
    { "ger ", "guh " },
    { "ove ", "uv " },
    { "cks ", "x " },
    { "ear ", "eer " },
    { "ets ", "itz " },
    { "ing ", "in " },

    { "ty ", "teh " },
    { " qu", " kw" },
    { "ow ", "ao " },
    { "en ", "eh " },
    { "es ", "z " },
    { "ck ", "kk " },
    { "le ", "el " },
    { "ed ", "d " },
    { "x ", "ks " },
    { "s ", "z " },

    { "c", "k" },
    { "wr", "r" },

    { " *,* ", "," },
    { " *;* ", ";" },
    { " *.* ", "." },
    { " *!* ", "!" },
    { " *?* ", "?" },
  };

  sstring out;
  int iWord = 0;
  sstring word = arg.word(iWord);
  int chance = from ? 100 - from->plotStat(STAT_CURRENT, STAT_INT, 0, 100, 50) : 25;

  for(; !word.empty(); word = arg.word(++iWord))
  {
    if (!out.empty())
      out += ' ';

    if (chance > number(0, 100))
    {
      sstring munged = " ";
      munged += word.lower();
      munged += ' ';

      for(int iReplace = 0; iReplace < (int)cElements(lolcats_replace); iReplace++)
        munged.inlineReplaceString(lolcats_replace[iReplace][0], lolcats_replace[iReplace][1]);

      out += munged.trim().matchCase(word);
    }
    else
      out += word;
  }

  return out;
}

// talk like the sesamestreet count vampire
sstring garble_vampire(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  sstring out = " ";
  out += arg.lower();
  out += " ";

  out.inlineReplaceString(" wha", " vhu");
  out.inlineReplaceString(" w", " v");

  return out.trim().matchCase(arg);
}

// talk like an igor henchman : Replace every s and ss with th
sstring garble_igor(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  sstring out = arg.lower();

  out.inlineReplaceString("ss", "th");
  out.inlineReplaceString("s", "th");

  return out.matchCase(arg);
}

// makes you talk drunk
// 9 barely drunk
// 15 really drunk
// 20 super drunk
// 25 wasted
sstring garble_drunk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  static const sstring replace[][2] = {
    { "ss", "\255s" },
    { "s", "sh" },
    { "ce", "she" },
    { "\255", "s" },
  };

  sstring out = arg;
  int iWord = 0;
  sstring word = out.word(iWord);
  int chance = from->getCond(DRUNK);
  TBeing * tFrom = const_cast<TBeing *>(from);
  bool emoted = false;
  unsigned int iReplace = 0;

  // if this garble was applied without us being drunk, assume an average drunkenness
  if (chance < 9)
    chance = 15;

  // chance of total random phrase during speech (not room descriptions)
  if (speechType != SPEECH_ROOMDESC && ::number(15 - chance, 14) < 0)
  {
    out = RandomPhrase(true);
    for(int iReplace = 0; iReplace < (int)cElements(replace); iReplace++)
      out.inlineReplaceString(replace[iReplace][0], replace[iReplace][1]);
    return out;
  }

  for(; !word.empty(); word = arg.word(++iWord))
  {
    // set this is we expect word to be modified
    bool modified = false;
    sstring original = word;
    sstring head = "", tail;
    int loc;

    // preserve color codes at the beginning
    while(word.length() > 2 && word[0] == '<' && word[2]=='>')
    {
      head += word.substr(0, 3);
      word.erase(0, 3);
    }

    while(word.length() > 1 && word[0] == '<' && word[2]=='>')
    {
      head += word.substr(0, 3);
      word.erase(0, 3);
    }

    // find punctuation at the end of the word and preserve
    for(loc = word.length()-1; loc > 0 && (!isalpha(word[loc]) || word[loc-1] == '<');loc--)
      tail = word[loc] + tail;
    if (tail.length() > 0)
      word.erase(loc+1, tail.length());

    // 10% chance of random word
    if (chance > 14 && word.length() > 2 && !::number(0, 9))
      word = RandomWord().matchCase(word);

    // 10% chance of just belching or something
    if (!emoted && speechType != SPEECH_ROOMDESC && chance > 20 && !::number(0, 19))
    {
      word = "*burp*";
      tFrom->doAction("", CMD_BURP);
      modified = emoted = true;
    }
    else if (!emoted && speechType != SPEECH_ROOMDESC && chance > 20 && !::number(0, 19))
    {
      word = "*hic*";
      tFrom->doAction("", CMD_HICCUP);
      modified = emoted = true;
    }

    // slurred speech always on for really drunk, 25% for slightly drunk
    if (chance > 14 || !::number(0, 3))
    {
      sstring munged = " ";
      munged += word.lower();
      munged += ' ';

      for(int iReplace = 0; iReplace < (int)cElements(replace); iReplace++)
        munged.inlineReplaceString(replace[iReplace][0], replace[iReplace][1]);

      word = munged.trim().matchCase(word);
      modified = true;
    }

    // trunc long words 20% of the time
    if (word.length() > 8 && (!::number(0, 4) || chance > 14))
    {
      if (!::number(0, 1))
        word = sstring("...") + word.substr(word.length() - ::number(3, 4), word.length()-1);
      else
        word = word.substr(0, word.length() - ::number(4, 5)) + sstring("...");
      modified = true;
    }

    // any word which has repeating character subsets will have a chance of repeating
    // the subset agian 1-3 times.  "Banana" has a repeating "an" in it, so if it repeats
    // 1 time, itll go "Bananana".  You do this: take the length, divide by two -> n.
    // From n down to 2 in length, look for substrings in the string of that length which repeat.
    // If so, find the end of the repeating substring and repeat it.
    int iStart = 0, cSub = 0;
    bool match = false;
    for(cSub = (int)word.length()/2; !match && cSub > 1;cSub--)
      for(iStart = 0; !match && iStart+cSub+cSub <= (int)word.length(); iStart++)
      {
        match = true;
        for (int iScan = 0; match && iScan < cSub; iScan++)
          match = word[iStart+iScan] == word[iStart+cSub+iScan];
      }
    if (match)
    {
      cSub++;
      iStart--;
      sstring munged = word.substr(0, iStart);
      munged += word.substr(iStart, cSub);
      munged += word.substr(iStart, cSub);
      if (::number(0,1))
        munged += word.substr(iStart, cSub);
      munged += word.substr(iStart+cSub, word.length());
      word = munged;
      modified = true;
    }

    // maintain all original formatting
    if (modified)
    {
      word = head + word;
      word += tail;

      // replace original with new word
      unsigned int iFound = out.find(original, iReplace);
      if(iFound != sstring::npos)
      {
        out.erase(iFound, original.length());
        out.insert(iFound, word);
        iReplace += word.length();
      }
    }
  }

  return out;
}

// Irish accent!
sstring garble_irish(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  static const sstring replace[][2] = {
    { ",", " *,* " },
    { ";", " *;* " },
    { ".", " *.* " },
    { "!", " *!* " },
    { "?", " *?* " },

    { "or", "ar"},
    { "er", "ar"},
    { " o", " oo"},
    { " ha", " 'a"},
    { " he", " 'e"},
    { " hi", " 'i"},
    { " ho", " 'o"},
    { " hu", " 'u"},
    { " hy", " 'y"},
    { " u ", " ye "},
    { " my", " me"},
    { " out", " oot"},
    { "out ", "oot "},
    { " my ", " me "},
    { " no ", " nay "},
    { " to ", " t' "},
    { "your", "yur"},
    { "ever", "e'er"},
    { " of ", " o' "},
    { " be ", " b' "},
    { " ack", " och"},
    { " ack ", " och "},
    { " you ", " ye "},
    { " your", " yur"},
    { " its ", " tis "},
    { " it's ", " 'tis "},
    { " yes ", " aye "},
    { " nod ", " aye "},
    { " not ", " na "},
    { " the ", " th' "},
    { " out ", " oot "},
    { " sir ", " sirrah "},
    { " and ", " an' "},
    { " boy ", " lad "},
    { " with", " wi'"},
    { " your ", " yur "},
    { " girl ", " lass "},
    { " very ", " ver' "},
    { " yeah ", " aye "},
    { " ever ", " e'er "},
    { " even ", " e'en "},
    { " auld ", " auld "},
    { " with ", " wi' "},
    { " isn't ", " is na "},
    { " isnt ", " is na "},
    { " it is ", " 'tis "},
    { " can't ", " canna "},
    { " don't ", " dunna "},
    { " cant ", " canna "},
    { " dont ", " dunna "},
    { " maybe ", " may'ap "},
    { " about ", " aboot "},
    { " won't ", " will na "},
    { " wont ", " will na "},
    { " today ", " t'day "},
    { " fairy ", " faerie "},
    { " aren't ", " are na "},
    { " arent ", " are na "},
    { " cannot ", " canna "},
    { " little ", " wee "},
    { " at all ", " a'tall "},
    { " do not ", " dunna "},
    { " didn't ", " dinna "},
    { " didnt ", " dinna "},
    { " myself ", " meself "},
    { " milord ", " m'lord "},
    { "perfect", "parfect"},
    { " before ", " afore "},
    { " it was ", " twas "},
    { " i think ", " methinks "},
    { " can not ", " canna "},
    { " did not ", " dinna "},
    { " perfect ", " parfect "},
    { " my lord ", " m'lord "},
    { " morning ", " marnin' "},
    { " between ", " atwixt "},
    { " ireland ", " eire "},
    { " it would ", " twood "},
    { " tomorrow ", " t'morrow "},
    { " thank you", " thankee"},
    { " thanks ", " thankee "},

    { " *,* ", "," },
    { " *;* ", ";" },
    { " *.* ", "." },
    { " *!* ", "!" },
    { " *?* ", "?" },
  };

  sstring out = " ";
  out += arg.lower();
  out += " ";

  for(int i=0;i < (int)cElements(replace);i++)
  {
    out.inlineReplaceString(replace[i][0], replace[i][1]);
  }

  return out.trim().matchCase(arg);
}

// a sad atempt to make trolls sound like klingons:
// no soft 'h' -> gh or g' like a gargle
// all q's are a hard 'K' at back of throat
// klingons like apostrophes
// no 'c' soft -> 'k'
// no 'ss' or 's' soft
// regular z's, are kz
// no f's or f sounds
// no th sound
sstring garble_trolltalk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  static const sstring replace[][2] = {
    { "ph", "'" },
    { "ch", "\254\255" },
    { "h", "g'" },
    { "z", "kz" },
    { "qu", "kw" },
    { "q", "k" },
    { "ck", "k" },
    { "c", "k" },
    { "\254\255", "ch" },
    { " s", " ch" },
    { "ss", "auch" },
    { "es ", "'k " },
    { "s ", "'k " },
    { "s", "'" },
    { "fr", "r" },
    { "fl", "l" },
    { "of", "uv" },
    { "f", "'" },
    { "'''", "'" },
    { "''", "'" },
  };

  sstring out = " ";
  out += arg.lower();
  out += " ";
  int chance = from ? 100 - from->plotStat(STAT_CURRENT, STAT_INT, 0, 100, 50) : 25;

  for(int i=0;i < (int)cElements(replace);i++)
  {
    if (!(chance*1.3 > number(0, 100)))
      continue;
    out.inlineReplaceString(replace[i][0], replace[i][1]);
  }

  return out.trim().matchCase(arg);
}

// frogs have a soft pallate, and talk as if they have 2 fingers holding their tounge down
sstring garble_frogtalk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  static const sstring replace[][2] = {
    { "ll", "y" },
    { "l", "y" },
    { "rr", "y" },
    { "r", "y" },
    { "cc", "k" },
    { "ck", "k" },
    { "ch", "sh" },
    { "c", "s" },
    { "st", "ss" },
    { "ts", "ss" },
    { "tt", "sh" },
    { "th", "ph" },
    { "tt", "sh" },
    { "t", "s" },
  };

  sstring out = " ";
  out += arg.lower();
  out += " ";
  int chance = from ? 100 - from->plotStat(STAT_CURRENT, STAT_INT, 0, 100, 50) : 75;

  // non-native frogtalkers must have gotten this via some drug or enchantment.  100% messup chance
  if (from && !(from->getMyRace()->getGarbles() & GARBLE_FLAG_FROGTALK))
    chance = 100;

  for(int i=0;i < (int)cElements(replace);i++)
  {
    if (!(chance*1.3 > number(0, 100)))
      continue;
    out.inlineReplaceString(replace[i][0], replace[i][1]);
  }

  return out.trim().matchCase(arg);
}


sstring garble_birdtalk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  static const sstring birdsquak_prefix[] = { "mwr","bwr","wr","buk","pwr","squ" };
  static const sstring birdsquak_suffix[] = { "awk","aah","awr","awrk","ak" };
  static const sstring replace[][2] = {
    { "gh", "k", },
    { "Gh", "K", },
    { "ww", "wr", },
    { "th", "t'", },
    { "Th", "T'", },
    { "sh", "s'", },
    { "Sh", "S'", },
    { "st", "'t", },
    { "St", "'T", },
    { "''", "'", },
    { "'s", "'", },
    { "ts", "t'", },
    { "sp", "s'", },
    { "Sp", "S'", },
    { "ng", "'", },
    { "wh", "w'", },
    { "Wh", "W'", },
    { "'''", "'", },
    { "''", "'", },
  };

  sstring out;
  int iWord = 0;
  sstring word = arg.word(iWord);
  int chance = from ? 100 - from->plotStat(STAT_CURRENT, STAT_INT, 0, 100, 50) : 25;

  for(; !word.empty(); word = arg.word(++iWord))
  {
    if (!out.empty())
      out += ' ';
    // replace randomly a word
    if (chance/2 > number(0, 100))
    {
      char punct = word[word.length()-1];
      word = sstring(birdsquak_prefix[number(0, cElements(birdsquak_prefix)-1)] +
              birdsquak_suffix[number(0, cElements(birdsquak_suffix)-1)]).matchCase(word);
      // sad hack to keep trailing , ? ! . on this word
      if (ispunct(punct))
        word += punct;
    }
    else if (chance*1.3 > number(0, 100))
    {
      for(int i=0;i < (int)cElements(replace);i++)
      {
        // assume replace strings are in lower case
        word.inlineReplaceString(replace[i][0], replace[i][1]);
        word.inlineReplaceString(replace[i][0].upper(), replace[i][1].upper());
      }
    }

    out += word;
  }

  return out;
}


sstring garble_gutter(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{
  static const sstring replace[][2] = {
    { " this ", " dis " },
    { " that", " dats" },
    { " their ", " deys " },
    { " theirs ", " deys " },
    { " they", " deys" },
    { " them", " dems" },
    { " the ", " dah " },
    { " there ", " ders " },
    { " are ", " is " },
    { "yre ", "'s " },
    { "'re ", "'s " },
    { "d'nt ", "n' " },
    { "'nt ", "n' " },
    { " wha", " who" },
    { " th", " f" },
    { "th ", "f " },
    { "th", "v" },
    { " its ", " s'/* " },
    { " it's ", " s'/* " },
    { "nd", "n'" },
    { " h", " '" },
    { "ing ", "in' " },
    { "ool ", "oo' " },
    { "oll ", "oe " },
    { "ol ", "o' " },
    { "al ", "ow " },
    { "all ", "aoe " },
    { "ill ", "iw " },
    { "il ", "ew " },
    { "le ", "ow " },
    { " all", " au" },
    { " al", " au" },
    { " oll", " ow" },
    { " ol", " ow" },
    { " ill", " ew" },
    { " il", " ew" },
    { "tt", "h'" },
    { "te ", "' " },
    { "it ", "ih' " },
    { "ot ", "oh' " },
    { "at ", "ah' " },
    { "et ", "eh' " },
    { "er ", "ah " },
    { "'''", "'" },
  };
  sstring out = " ";
  out += arg.lower();
  out += " ";
  int chance = from ? 100 - from->plotStat(STAT_CURRENT, STAT_INT, 0, 100, 50) : 25;

  for(int i=0;i < (int)cElements(replace);i++)
  {
    if (i > 9 && !(chance*1.3 > number(0, 100)))
      continue;
    out.inlineReplaceString(replace[i][0], replace[i][1]);
  }
  out.inlineReplaceString("/* ", "");

  return out.trim().matchCase(arg);
}


sstring garble_trogtalk(const TBeing *from, const TBeing *to, const sstring &arg, SPEECHTYPE speechType)
{

  return arg;
}


const sstring RandomWord()
{
  static const sstring str[] =
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

const sstring RandomNoun()
{
  static const sstring str[] =
  {
    // silly words
    "glop",
    "glyph",
    "fribble",
    "fjord",
    "regina",
    "angina",
    "pterodactyl",
    "lo-pan",
    "tangibles",
    "hootenanny",
    "pocket",

    // suggestive words
    "thingy",
    "whazzit",
    "whatchyamacallit",
    "johnson",
    "tool",
    "claymore",
    "helmet",
    "longstaff",
    "poodle",
    "pussycat",

    // foods
    "fish",
    "chicken",
    "beef",
    "meat",
    "pork",
    "vittles",
    "muffin",
    "cookies",
    "sushi",
    "tacos",

    // people/beings
    "gimp",
    "brother",
    "sister",
    "cousin",
    "father",
    "mother",
    "uncle",
    "aunt",
    "dude",
    "lady",
    "monster",
    "beast",
  };
  return (str[number(0, (cElements(str)-1))]);
}

const sstring RandomAdjective()
{
  static const sstring str[] =
  {
    // synonyms/antonyms
    "hot",
    "cold",
    "warm",
    "cool",
    "hard",
    "soft",
    "firm",
    "stiff",
    "flacid",
    "limp",
    "strong",
    "weak",
    "wimpy",
    "tender",
    "pathetic",
    "powerful",
    "quiet",
    "loud",
    "noisy",
    "silent",
    "beautiful",
    "ugly",
    "cute",
    "revolting",
    "plain",
    "scary",
    "comforting",
    "stupid",
    "smart",
    "stupendous",
    "ordinary",
    "obstreperous",
    "accommodating",
    "malodorous",
    "aromatic",
    "honorable",
    "nefarious",
    "chaste",
    "perverted",

    // eq adjectives
    "glowing",
    "pulsing",
    "shadowy",
    "humming",
    "charmed",
    "charred",
    "sparkling",
  };
  return (str[number(0, (cElements(str)-1))]);
}


const sstring RandomVerb()
{
  // guaranteed insensible
  static const sstring str[] =
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
    "encapitate",
    "kill",
    "taste",
    "tidy",
    "polish",
  };
  return (str[number(0, (cElements(str)-1))]);
}

const sstring RandomPhrase(bool allowPhrase)
{
  // random drunk phrase
  static const sstring str[] =
  {
    // terminating phrases
    "Party!",
    "What?",
    "Woohoo!",
    "Remember that time?",
    "Listen to this!",
    "How you doin'?",
    "I am sooooo drunk.",
    "I love you guys.",
    "I hate you guys.",

    // piecemeal
    "Hey where's the {noun}?",
    "Could someone {verb} my {adj} {noun}?",
    "Hey!  Who here wants to {verb} the {adj} {noun}?",
    "Who left their {noun} here?",
    "Okay {verb} me a {noun}.",
    "Don't make me {verb} you in the {noun}!",
    "Is it because you {verb} {noun} that you are so {adj}?",
    "My {noun} {verb}s in a {adj} way.",
    "I think I'll {verb} your {noun}",
    "My {noun} is more {adj} than your {noun}",
    "I can {verb} that {adj} {noun} way better then you.",
    "Can anyone here {verb} a {adj} {noun}?",

    // conjunctives
    "Wait...  {phrase}",
    "Oh man...  {phrase}",
    "Yo!  {phrase}",
    "Oh!  {phrase}",
    "Hey, {phrase}",
    "Uh Oh.  {phrase}",
    "Hehehe.  You said \"{phrase}\"",
    "At first I was like, \"{phrase}\" but now I'm like \"{phrase2}\"",
    "If you say \"{phrase}\" one more time, I swear I'm going to lose it!",
  };
  int cSize = allowPhrase ? (cElements(str)-1) : (cElements(str)-10);
  sstring out = str[number(0, cSize)];
  out.inlineReplaceString("{verb}", RandomVerb());
  out.inlineReplaceString("{adj}", RandomAdjective());
  out.inlineReplaceString("{noun}", RandomNoun());
  if (allowPhrase)
  {
    out.inlineReplaceString("{phrase}", RandomPhrase(false));
    out.inlineReplaceString("{phrase2}", RandomPhrase(false));
  }
  if (out.length() > 250)
    return RandomPhrase(allowPhrase);
  return out;
}

