//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "cards.cc" - All functions and routines related to general card games 
//
//      The SneezyMUD card games were coded by Russ Russell, April 1994, 
//      Last revision, June 1996
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "games.h"


cardSuitT & operator++(cardSuitT &c, int)
{
  return c = (c == MAX_SUIT) ? MIN_SUIT : cardSuitT(c+1);
}

cardSuitT Card::getSuit() const {
  return suit;
}

sstring Card::getName() const{
  sstring buf;

  buf = card_names[value];

  switch(suit){
    case SUIT_WATER:
      buf += " of <b>Blue Water<z>";
      break;
    case SUIT_FIRE:
      buf += " of <r>Red Fire<z>";
      break;
    case SUIT_EARTH:
      buf += " of <g>Green Earth<z>";
      break;
    case SUIT_ETHER:
      buf += " of <p>Purple Ether<z>";
      break;
    default:
      buf += " of <k>an unknown suit<z>";
  }

  return buf;
}


Card::Card(cardSuitT suit, int value)
{
  this->suit=suit;
  this->value=value;
}


const Card *CardDeck::draw()
{
  Card *tmp;
  tmp=deck.front();
  deck.pop_front();
  deck.push_back(tmp);

  return tmp;
}

const Card *CardDeck::undraw()
{
  Card *tmp;
  tmp=deck.back();
  deck.pop_back();
  deck.push_front(tmp);

  return tmp;
}


void CardDeck::shuffle()
{
  std::random_shuffle( deck.begin( ), deck.end( ) );
}


CardDeck::CardDeck()
{
  for(cardSuitT suit=MIN_SUIT;suit<MAX_SUIT;suit++){
    for(int card=1;card<=13;++card){
      deck.push_back(new Card(suit, card));
    }
  }
  
  shuffle();
}







unsigned char CARD_NUM(unsigned char card)
{
  return (card & 0x0f);
}

unsigned char CARD_NUM_ACEHI(unsigned char card)
{
  int c = (card & 0x0f);
  
  if(c==1)
    return 14;
  else
    return c;
}




void CardGame::setup_deck()
{
  int i, j, k = 0;

  for (i = 0; i < 4; i++) {
    for (j = 1; j <= 13; j++) {
      deck[k] = j;
      switch (i) {
	case 0:
	  deck[k] |= CARD_WATER;
	  break;
	case 1:
	  deck[k] |= CARD_FIRE;
	  break;
	case 2:
	  deck[k] |= CARD_EARTH;
	  break;
	case 3:
	  deck[k] |= CARD_ETHER;
	  break;
      }
      k++;
    }
  }
}

const sstring CardGame::pretty_card_printout(const TBeing *ch, int card) const
{
  char buf[80];

  strcpy(buf, card_names[card & 0x0f]);
  add_suit(ch, buf, card);

  return buf;
}

int CardGame::same_suit(int card1, int card2)
{
  if (((card1 & CARD_WATER) && (card2 & CARD_WATER)) ||
      ((card1 & CARD_EARTH) && (card2 & CARD_EARTH)) ||
      ((card1 & CARD_FIRE) && (card2 & CARD_FIRE)) ||
      ((card1 & CARD_ETHER) && (card2 & CARD_ETHER)))
    return TRUE;

  return FALSE;
}

bool CardGame::is_heart(int card)
{
  return (card & CARD_FIRE);
}

bool CardGame::is_queen_of_spades(int card) 
{
  return ((card & CARD_ETHER) && (CARD_NUM(card) == 12));
}

bool CardGame::has_suit(int *hand, int suitnum)
{
  int i;

  for (i = 0; hand[i] && i < 13; i++) {
    if (hand[i] & suitnum)
      return TRUE;
  }
  return FALSE;
}

int CardGame::add_suit(const TBeing *ch, char *cat_msg, int card) const
{
  if (ch) {
    if (card & CARD_WATER)
      sprintf(cat_msg + strlen(cat_msg), " of <b>Blue Water<z>");
    if (card & CARD_FIRE)
      sprintf(cat_msg + strlen(cat_msg), " of <r>Red Fire<z>");
    if (card & CARD_EARTH)
      sprintf(cat_msg + strlen(cat_msg), " of <g>Green Earth<z>");
    if (card & CARD_ETHER)
      sprintf(cat_msg + strlen(cat_msg), " of <p>Purple Ether<z>");
    /*
    if (card & CARD_WATER)
      sprintf(cat_msg + strlen(cat_msg), " of %sBlue Water%s", ch->blue(), ch->norm());
    if (card & CARD_FIRE)
      sprintf(cat_msg + strlen(cat_msg), " of %sRed Fire%s", ch->red(), ch->norm());
    if (card & CARD_EARTH)
      sprintf(cat_msg + strlen(cat_msg), " of %sGreen Earth%s", ch->green(), ch->norm());
    if (card & CARD_ETHER)
      sprintf(cat_msg + strlen(cat_msg), " of %sPurple Ether%s", ch->purple(), ch->norm());
    */
  } else {
    if (card & CARD_WATER)
      strcat(cat_msg, " of Blue Water");
    if (card & CARD_FIRE)
      strcat(cat_msg, " of Red Fire");
    if (card & CARD_EARTH)
      strcat(cat_msg, " of Green Earth");
    if (card & CARD_ETHER)
      strcat(cat_msg, " of Purple Ether");
  }
  return TRUE;
}

const sstring CardGame::suit(const TBeing *ch, int card) const
{
  char buf[80];

  if (ch) {
    if (card & CARD_WATER)
      sprintf(buf, "%sBlue Water%s", ch->blue(), ch->norm());
    if (card & CARD_FIRE)
      sprintf(buf, "%sRed Fire%s", ch->red(), ch->norm());
    if (card & CARD_EARTH)
      sprintf(buf, "%sGreen Earth%s", ch->green(), ch->norm());
    if (card & CARD_ETHER)
      sprintf(buf, "%sPurple Ether%s", ch->purple(), ch->norm());

    return buf;
  } else {
    if (card & CARD_WATER)
      return "Blue Water";
    if (card & CARD_FIRE)
      return "Red Fire";
    if (card & CARD_EARTH)
      return "Green Earth";
    if (card & CARD_ETHER)
      return "Purple Ether";
  }
  return "";
}


void TBeing::doCall(const sstring &)
{
  if (checkHoldem())
    gHoldem.call(this);
  else
    sendTo("Call is used for casino games.\n\r");
}

void TBeing::doFold(const sstring &)
{
  if (checkHoldem())
    gHoldem.fold(this);
  else
    sendTo("Fold is used for casino games.\n\r");
}


void TBeing::doPeek() const
{
  if (checkBlackjack())
    gBj.peek(this);
  else if (gGin.check(this))
    gGin.peek(this);
  else if (checkHearts())
    gHearts.peek(this);
  else if (checkCrazyEights())
    gEights.peek(this);
  else if (checkDrawPoker())
    gDrawPoker.peek(this);
  else if (checkPoker())
    gPoker.peek(this);
  else if (checkBaccarat())
    gBaccarat.peek(this);
  else if (checkHoldem(true))
    gHoldem.peek(this);
  else
    sendTo("So you think you are at a card table?\n\r");
}

void TBeing::doDeal(const char *tArg)
{
  if (gGin.check(this)) 
    gGin.deal(this);
  else if (checkHearts())
    gHearts.deal(this);
  else if (checkCrazyEights())
    gEights.deal(this);
  else if (checkDrawPoker())
    gDrawPoker.deal(this, tArg);
  else {
    sendTo("Does this look like a card table?\n\r");
    return;
  }
}


void CardGame::shuffle()
{
  int i, num, num2, tmp;

  for (num = 0; num < 500; num++) {
    for (i = 0; i < 52; i++) {
      num2 = number(0, 51);
      tmp = deck[num2];
      deck[num2] = deck[i];
      deck[i] = tmp;
    }
  }
}

void CardGame::take_card_from_hand(int *hand, int which, int max_num)
{
  int i;

  for (i = which; i < max_num; i++)
    hand[i] = hand[i + 1];

  hand[max_num] = 0;
}

void TBeing::doPass(const char *arg)
{
  if (checkHearts())
    gHearts.pass(this, arg);

  if (checkCrazyEights())
    gEights.pass(this);

  if (checkDrawPoker())
    gDrawPoker.pass(this);

  return;
}


void CardGame::order_high_to_low(int *num1, int *num2, int *num3)
{
  int high = -1, mid = -1, low = -1;

  if ((*num1 > *num2) && (*num1 > *num3))
    high = *num1;
  else if ((*num1 > *num2) || (*num1 > *num3))
    mid = *num1;
  else
    low = *num1;

  if (high >= 0) {
    mid = max(*num2, *num3);
    low = min(*num2, *num3);
  } else if (mid >= 0) {
    high = max(*num2, *num3);
    low = min(*num2, *num3);
  } else {
    high = max(*num2, *num3);
    mid = min(*num2, *num3);
  }
  *num1 = high;
  *num2 = mid;
  *num3 = low;
}

int get_suit(int card)
{
  if (card & CARD_WATER)
    return CARD_WATER;
  if (card & CARD_FIRE)
    return CARD_FIRE;
  if (card & CARD_EARTH)
    return CARD_EARTH;
  if (card & CARD_ETHER)
    return CARD_ETHER;

  return -1;
}

int cardnumComparAscend(const void *card1, const void *card2)
{
  int temp = *(const int *)card1 - *(const int *)card2;
  if (temp > 0)
    return 1;
  else if (temp < 0)
    return -1;
  else
    return 0;
}

int cardnumComparDescend(const void *card1, const void *card2)
{
  return (((*(const int *)card1) > (*(const int *)card2)) ? -1 : 1);
}


void TBeing::doSort(const char *arg) const
{
  int index_num, cnt = 0, i;

  for (;isspace(*arg);arg++);

  if (!*arg) {
    sendTo("Sort : Syntax (sort <ascending | descending>)\n\r");
    return;
  }
  if ((index_num = gGin.index(this)) > -1) {
    cnt = gGin.count(index_num);
    if (is_abbrev(arg, "ascending")) {
      if (cnt % 2) {
        // Kludges rule
        int tmp[12];
        for (i = 0; i < 11; tmp[i] = gGin.hands[index_num][i], i++);
        tmp[11] = 999999;
        qsort(tmp, 12, 4, cardnumComparAscend);
        for (i = 0; i < 11; gGin.hands[index_num][i] = tmp[i], i++); 
      } else
        qsort(gGin.hands[index_num], gGin.count(index_num), 4, cardnumComparAscend);
    } else {
      if (cnt % 2) {
        // Kludges rule
        int tmp2[12];
        for (i = 0; i < 11; tmp2[i] = gGin.hands[index_num][i], i++);
        tmp2[11] = 0;
        qsort(tmp2, 12, 4, cardnumComparDescend);
        for (i = 0; i < 11; gGin.hands[index_num][i] = tmp2[i], i++);
      } else
        qsort(gGin.hands[index_num], gGin.count(index_num), 4, cardnumComparDescend);
    }
    doPeek();
    return;
  } else if ((index_num = gDrawPoker.index(this)) > -1) {
    int tmp3[6];
    for (i = 0; i < 5; tmp3[i] = gDrawPoker.hands[index_num][i], i++);

    if (is_abbrev(arg, "ascending")) {
      tmp3[5] = 999999;
      qsort(tmp3, 6, 4, cardnumComparAscend);
    } else {
      tmp3[5] = 0;
      qsort(tmp3, 6, 4, cardnumComparDescend);
    }

    for (i = 0; i < 5; gDrawPoker.hands[index_num][i] = tmp3[i], i++);

    doPeek();
    return;
  } else if ((index_num = gEights.index(this)) > -1) {
    cnt = gEights.count(index_num);
    if (is_abbrev(arg, "ascending")) {
      if (cnt % 2) {
        int tmp3[cnt + 1];
        for (i = 0; i < cnt; tmp3[i] = gHearts.hands[index_num][i], i++);
        tmp3[cnt] = 999999;
        qsort(tmp3, cnt + 1, 4, cardnumComparAscend);
        for (i = 0; i < cnt; gHearts.hands[index_num][i] = tmp3[i], i++);
      } else
        qsort(gHearts.hands[index_num], cnt, 4, cardnumComparAscend);
    } else {
      if (cnt % 2) {
        int tmp4[cnt + 1];
        for (i = 0; i < cnt; tmp4[i] = gHearts.hands[index_num][i], i++);
        tmp4[cnt] = 0;
        qsort(tmp4, cnt + 1, 4, cardnumComparDescend);
        for (i = 0; i < cnt; gHearts.hands[index_num][i] = tmp4[i], i++);
      } else
        qsort(gHearts.hands[index_num], cnt, 4, cardnumComparDescend);
    }

    doPeek();
    return;
  } else if ((index_num = gHearts.index(this)) > -1) {
    cnt = gHearts.count(index_num);
    if (is_abbrev(arg, "ascending")) {
      if (cnt % 2) {
        // Kludges rule
        int tmp3[cnt + 1];
        for (i = 0; i < cnt; tmp3[i] = gHearts.hands[index_num][i], i++);
        tmp3[cnt] = 999999;
        qsort(tmp3, cnt + 1, 4, cardnumComparAscend);
        for (i = 0; i < cnt; gHearts.hands[index_num][i] = tmp3[i], i++);
      } else
        qsort(gHearts.hands[index_num], cnt, 4, cardnumComparAscend);
    } else {
      if (cnt % 2) {
        // Kludges rule
        int tmp4[cnt + 1];
        for (i = 0; i < cnt; tmp4[i] = gHearts.hands[index_num][i], i++);
        tmp4[cnt] = 0;
        qsort(tmp4, cnt + 1, 4, cardnumComparDescend);
        for (i = 0; i < cnt; gHearts.hands[index_num][i] = tmp4[i], i++);
      } else
        qsort(gHearts.hands[index_num], gHearts.count(index_num), 4,cardnumComparDescend);
    }
    doPeek();
    return;
  }
  sendTo("You must be playing a card game to use this command!\n\r");
  return;
}
  
CardGame::CardGame() :
  game(false),
  bet(0)
{
  setup_deck();
}

