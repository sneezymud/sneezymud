///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//      SneezyMUD++ - All rights reserved, SneezyMUD Coding Team         //
//      "gin.cc." - All functions and routines related to the gin game       //
//                                                                           //
//      The SneezyMUD gin table was coded by Russ Russell, April 1994.       //
//      Changed to c++ October 1994                                          //
//      Last revision, October 13th, 1994.                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "monster.h"
#include "games.h"
#include "extern.h"
#include "handler.h"

GinGame gGin;

const int GIN_TABLE =  8416;

bool GinGame::check(const TBeing *ch) const
{
  if (ch->inRoom() == GIN_TABLE)
    return TRUE;
  else
    return FALSE;
}

void GinGame::deal(TBeing *ch)
{
  int i, j = 0, which;
  TBeing *ch1, *ch2;

  if ((which = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s got into GinGame::deal without being at the gin table!\n\r") %  ch->getName());
    return;
  }
  if (game) {
    ch->sendTo("Redeal while the game is in progress? Never!!\n\r");
    return;
  }
  ch1 = get_char_room(names[!which], GIN_TABLE);
  ch2 = get_char_room(names[which], GIN_TABLE);

  if (!ch1 || !ch2) {
    vlogf(LOG_BUG, "GinGame::deal() called without two players!");
    return;
  }
  shuffle();

  ch2->sendTo("You shuffle the cards, and deal them.\n\r");
  act("$n shuffles the cards and deals them.", FALSE, ch2, NULL, NULL, TO_ROOM);

  ch1->sendTo("You are dealt:\n\r");
  ch2->sendTo("You are dealt:\n\r");

  for (i = 0; i < 10; i++) {
    hands[!which][i] = deck[j++];
    hands[which][i] = deck[j++];
  }
  hands[!which][10] = deck[j];

  for (i = 0; i < 32; i++)
    pile[i] = 0;

  deck_index = 21;
  pile_index = -1;
  loser[0] = FALSE;
  loser[1] = FALSE;
  can_draw = !which;
  game = TRUE;

  peek(ch1);
  peek(ch2);
}

void GinGame::peek(const TBeing *ch)
{
  int i, which;

  if ((which = index(ch)) < 0) {
    ch->sendTo("You aren't at the gin table to peek at any cards.\n\r");
    return;
  }
  if (!game) {
    ch->sendTo("You have no cards, there is currently no game going on!\n\r");
    return;
  }
  ch->sendTo("You have the following cards:\n\r");
  ch->sendTo("-----------------------------\n\r");

  for (i = 0; i < 11; i++) {
    if (hands[which][i])
      ch->sendTo(format("%2d) %-5s | %s\n\r") % (i+1) %		 card_names[CARD_NUM(hands[which][i])] %
		 suit(ch, hands[which][i]));
  }
  return;
}

int GinGame::move_card(TBeing *ch, const char *arg)
{
  int i, orig, n, which, tmp;

  if ((which = index(ch)) < 0) {
    ch->sendTo("You aren't at a gin table to move any cards around!\n\r");
    return FALSE;
  }
  if (sscanf(arg, "%d %d", &orig, &n) == 2) {
    if (!in_range(orig, 1, 11) || !in_range(n, 1, 11)) {
      ch->sendTo("Gin table syntax : put <original card place number> <new place number>\n\r");
      return FALSE;
    }
    if (orig == n) {
      ch->sendTo(format("The number %d card is already in the number %d slot!\n\r") % orig %
n);
      return FALSE;
    }
    orig--;
    n--;

    if (!hands[which][orig] || !hands[which][n]) {
      ch->sendTo("You don't have a card in that slot!\n\r");
      return FALSE;
    }
    tmp = hands[which][orig];

    if (orig < n) {
      for (i = orig; i < n; i++)
	hands[which][i] = hands[which][i + 1];
    } else {
      for (i = orig; i > n; i--)
	hands[which][i] = hands[which][i - 1];
    }
    hands[which][n] = tmp;
    ch->sendTo(format("You move card number %d to slot %d.\n\r") % (orig + 1) % (n + 1));
    return TRUE;
  }
  ch->sendTo("Gin table syntax : put <original card place number> <new place number>\n\r");
  return FALSE;
}

int GinGame::enter(const TBeing *ch)
{
  int which;

  if (dynamic_cast<const TMonster *>(ch)) {
    ch->sendTo("Dumb monsters can't play gin!\n\r");
    return FALSE;
  }
  if (inuse[0] && inuse[1]) {
    ch->sendTo("There are already two players at the gin table!\n\r");
    return FALSE;
  } else if (ch->getPosition() == POSITION_SITTING) {
    ch->sendTo("You are already sitting at the gin table.\n\r");
    return FALSE;
  } else {
    if (!inuse[0])
      which = 0;
    else {
      game = FALSE;
      which = 1;
    }
    ch->sendTo(format("You sit down at the gin table. %s sits across from you at the table.\n\r") % (inuse[!which] ? names[!which] : "No one"));
    strcpy(names[which], ch->getName());
    inuse[which] = TRUE;
    loser[which] = TRUE;
    return TRUE;
  }
}

int GinGame::exitGame(const TBeing *ch)
{
  TBeing *other;
  int which, i;

  if ((which = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s left a gin table he wasn't at!") % ch->getName());
    return FALSE;
  }
  other = get_char_room(names[!which], GIN_TABLE);

  ch->sendTo("You leave the gin table.\n\r");
  act("$n stands up and leaves the gin table, totally mooting the game.", FALSE,
ch, NULL, NULL, TO_ROOM);
  if (other) {
    other->sendTo("You stand up and leave the table as well.\n\r");
    other->setPosition(POSITION_STANDING);
  }
  // Clear out and zero all necessary gGin variables. 
  *(names[0]) = '\0';
  *(names[1]) = '\0';
  topcard = 0;
  deck_index = 0;
  pile_index = -1;
  game = FALSE;
  can_draw = 0;
  inuse[0] = FALSE;
  inuse[1] = FALSE;
  loser[0] = FALSE;
  loser[1] = FALSE;
  score[0] = 0;
  score[1] = 0;

  for (i = 0; i < 52; i++) {
    deck[i] = 0;
    if (i < 32)
      pile[i] = 0;
    if (i < 11) {
      hands[0][i] = 0;
      hands[1][i] = 0;
    }
  }
  setup_deck();
  return TRUE;
}

int GinGame::index(const TBeing *ch) const
{
  if (!strcmp(ch->getName(), names[0]))
    return 0;
  else if (!strcmp(ch->getName(), names[1]))
    return 1;
  else
    return -1;
}

int GinGame::draw(TBeing *ch, const char *arg)
{
  int which;

  for (; isspace(*arg); arg++);

  if ((which = index(ch)) < 0) {
    ch->sendTo("You aren't playing a gin game to draw cards from!\n\r");
    return FALSE;
  }
  if (can_draw != which) {
    ch->sendTo("You can't draw at the current time.\n\r");
    return FALSE;
  }
  if (is_abbrev(arg, "deck")) {
    hands[which][10] = deck[deck_index++];
    ch->sendTo(format("You draw the %s.\n\r") % pretty_card_printout(ch,hands[which][10]));
    act("$n draws a card from the deck.", FALSE, ch, NULL, NULL, TO_ROOM);
    return TRUE;
  } else if (is_abbrev(arg, "pile")) {
    hands[which][10] = pile[pile_index];
    pile[pile_index--] = 0;
    ch->sendTo(format("You draw the %s off of the pile top.\n\r") %
	       pretty_card_printout(ch, hands[which][10]));
    act("$n takes the top card from the pile.", FALSE, ch, NULL, NULL, TO_ROOM);
    return TRUE;
  } else {
    ch->sendTo("Gin table syntax : pick <deck | pile>\n\r");
    return FALSE;
  }
}

void GinGame::clear()
{
  int i;

  for (i = 0; i < 52; i++) {
    deck[i] = 0;
    if (i < 32)
      pile[1] = 0;
    if (i < 11) {
      hands[0][i] = 0;
      hands[1][i] = 0;
    }
  }
  deck_index = 0;
  pile_index = -1;
  game = FALSE;
  score[0] = 0;
  score[1] = 0;
  bet = 0;
  setup_deck();
}

void GinGame::clear_hand()
{
  int i;

  for (i = 0; i < 52; i++) {
    deck[i] = 0;
    if (i < 32)
      pile[1] = 0;
    if (i < 11) {
      hands[0][i] = 0;
      hands[1][i] = 0;
    }
  }
  deck_index = 0;
  pile_index = -1;
  game = FALSE;
  setup_deck();
}

void GinGame::win(TBeing *ch)
{
  int which;
  TBeing *other;
  char buf[256];

  if ((which = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s got into GinGame::win() while not at a gin table!") %  ch->getName());
    return;
  }
  if (!(other = get_char_room(names[!which], GIN_TABLE))) {
    vlogf(LOG_BUG, "GinGame::win() called without two players!");
    return;
  }
  ch->sendTo(format("You win the game! The score is : %s.\n\r") % gin_score());
  sprintf(buf, "$n wins the game! The score is : %s", gin_score().c_str());
  act(buf, TRUE, ch, 0, 0, TO_ROOM);

  loser[which] = FALSE;
  loser[!which] = TRUE;

  clear();
}

void GinGame::win_hand(TBeing *ch)
{
  int which;
  TBeing *other;
  char buf[256];

  if ((which = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s got into GinGame::win() while not at a gin table!") %  ch->getName());
    return;
  }
  if (!(other = get_char_room(names[!which], GIN_TABLE))) {
    vlogf(LOG_BUG, "GinGame::win_hand() called without two players!");
    return;
  }
  ch->sendTo(format("You win the hand! The score is : %s.\n\r") % gin_score());
  sprintf(buf, "$n wins the hand! The score is : %s", gin_score().c_str());
  act(buf, TRUE, ch, 0, 0, TO_ROOM);

  loser[which] = FALSE;
  loser[!which] = TRUE;

  clear_hand();
}

void GinGame::gin(TBeing *ch)
{
  int which, low;
  TBeing *other;
  char buf[256];

  if ((which = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s got into gin() while not at a gin table!") %  ch->getName());
    return;
  }
  if (!(other = get_char_room(names[!which], GIN_TABLE))) {
    vlogf(LOG_BUG, "gin() called without two players!");
    return;
  }
  if ((low = can_knock_or_gin(other)) == -1)
    return;

  ch->sendTo("You call a gin hand!\n\r");
  act("$n calls a gin hand!", TRUE, ch, NULL, NULL, TO_ROOM);
  ch->sendTo(COLOR_MOBS, format("%s had %d points in %s hand!\n\r") % other->getName() % low % other->hshr());
  other->sendTo(format("You have %d points in your hand!\n\r") % low);
  sprintf(buf, "$n gins, and $N had %d points in $S hand.", low);
  act(buf, FALSE, ch, NULL, other, TO_NOTVICT);
  other->sendTo("Here is your opponents winning hand.\n\r");
  ch->sendTo("Here is your opponents losing hand.\n\r");
  for (int i = 0; i < 11; i++) {
    if (hands[which][i])
      other->sendTo(format("%2d) %-5s | %s\n\r") % (i+1) % card_names[CARD_NUM(hands[which][i])] %
            suit(ch, hands[which][i]));
    if (hands[!which][i])
      ch->sendTo(format("%2d) %-5s | %s\n\r") % (i+1) % card_names[CARD_NUM(hands[!which][i])] %
            suit(ch, hands[!which][i]));
  }
  score[which] += (25 + low);
  if (score[which] >= 100)
    win(ch);
  else
    win_hand(ch);
}

void GinGame::knock(TBeing *ch, int low)
{
  int which, other_low;
  TBeing *other;
  char buf[256];

  if ((which = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s got into () while not at a gin table!") %  ch->getName());
    return;
  }
  if (!(other = get_char_room(names[!which], GIN_TABLE))) {
    vlogf(LOG_BUG, "knock() called without two players!");
    return;
  }
  if ((other_low = can_knock_or_gin(other)) == -1)
    return;

  ch->sendTo(format("You knock with %d point%s!\n\r") % low % (low == 1 ? "" : "s"));
  sprintf(buf, "$n calls a knock hand with %d point%s!", low, low == 1 ? "" : "s");
  act(buf, TRUE, ch, NULL, NULL, TO_ROOM);
  ch->sendTo(COLOR_MOBS, format("%s had %d points in %s hand!\n\r") % other->getName() % other_low %
other->hshr());
  other->sendTo(format("You have %d points in your hand!\n\r") % other_low);

  if (low < other_low) {
    ch->sendTo("You win your knock!\n\r");
    other->sendTo("Here is your opponents winning hand.\n\r");
    ch->sendTo("Here is your opponents losing hand.\n\r");
    for (int i = 0; i < 11; i++) {
      if (hands[which][i])
        other->sendTo(format("%2d) %-5s | %s\n\r") % (i+1) % card_names[CARD_NUM(hands[which][i])] %
              suit(ch, hands[which][i]));
      if (hands[!which][i])
        ch->sendTo(format("%2d) %-5s | %s\n\r") % (i+1) % card_names[CARD_NUM(hands[!which][i])] %
              suit(ch, hands[!which][i]));
    }
    score[which] += (other_low - low);
    if (score[which] >= 100)
      win(ch);
    else
      win_hand(ch);
  } else {
    ch->sendTo("Your knock is busted!\n\r");
    other->sendTo(format("You bust %s knock!\n\r") % ch->hshr());
    other->sendTo("Here is your opponents losing hand.\n\r");
    ch->sendTo("Here is your opponents winning hand.\n\r");
    for (int i = 0; i < 11; i++) {
      if (hands[which][i])
        other->sendTo(format("%2d) %-5s | %s\n\r") % (i+1) % card_names[CARD_NUM(hands[which][i])] %
              suit(ch, hands[which][i]));
      if (hands[!which][i])
        ch->sendTo(format("%2d) %-5s | %s\n\r") % (i+1) % card_names[CARD_NUM(hands[!which][i])] %
              suit(ch, hands[!which][i]));
    }
    score[!which] += (10 + (low - other_low));
    if (score[!which] >= 100)
      win(other);
    else
      win_hand(other);
  }
}

void GinGame::play(TBeing *ch, const char *arg)
{
  int i, which, card;
  char buf[256];
  char arg1[132];
  int discard, low;

  for (; isspace(*arg); arg++);

  if ((which = index(ch)) < 0) {
    ch->sendTo("You can't play a card when not at the gin table!\n\r");
    return;
  }
  if (!hands[which][0]) {
    ch->sendTo("You need to deal before you can discard.\n\r");
    return;
  } else if (!hands[which][10]) {
    ch->sendTo("You only have 10 cards, you can't discard one now!\n\r");
    return;
  }
  if (sscanf(arg, "%s %d", arg1, &discard) == 2) {
    if (is_abbrev(arg1, "gin")) {
      if (!in_range(discard, 1, 11)) {
	ch->sendTo("Gin/Knock syntax : play \"gin\" <card number>\n\r");
	return;
      }
      discard--;

      card = hands[which][discard];

      hands[which][discard] = 0;
      take_card_from_hand(hands[which], discard, 10);

      if ((low = can_knock_or_gin(ch)) == -1)
	return;

      if (!low) {
	gin(ch);
	return;
      } else if (low <= 10) {
	knock(ch, low);
	return;
      } else {
	ch->sendTo(format("You can't gin, you have too many points (%d).\n\r") % low);
	for (i = 10; i > discard; i--)
	  hands[which][i] = hands[which][i - 1];

	hands[which][discard] = card;
	return;
      }
    } else {
      ch->sendTo("Gin/Knock syntax : play \"gin\" <card number>\n\r");
      return;
    }
  } else if (sscanf(arg, "%d", &card) != 1) {
    ch->sendTo("Gin table syntax : play <card number>\n\r");
    return;
  }
  card--;

  if (card < 0 || card > 10) {
    ch->sendTo("Gin table syntax : play <card number>\n\r");
    return;
  }
  if (hands[which][card]) {
    // Put card onto the pile 
    pile[++pile_index] = hands[which][card];

    // Rearrange players hand so he doesn't have a "hole" where the card was 
    take_card_from_hand(hands[which], card, 10);

    can_draw = !which;
    ch->sendTo(format("You place the %s on the card pile.\n\r") % pretty_card_printout(ch, pile[pile_index]));
    sprintf(buf, "$n places the %s on the card pile.", pretty_card_printout(ch, pile[pile_index]).c_str());
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);
    return;
  } else {
    ch->sendTo("You don't have a card in that slot to play!\n\r");
    return;
  }
}

// Figuring out the lowest gin hand can be confusing. There are 
// a number of possibilities for each card. A card can fit in a 
// book, it can fit in a run, or it can fit in both. What we end 
// up with are examples like these :                            
                                                            
//    1) A book of three 8's that aren't in any run.           
//    2) A book of three 8's where one 8 is in a 8, 9, 10 run.  
//         In this case we have to figure out which is lower    
//    3) A book of three 8's with more than one 8 in a run.     
//         In this case we have to figure out which is lower    
//    4) A book of four 8's that aren't in a run.               
//    5) A book of four 8's where one eight is in a run.        
//         In this case we don't have to figure anything out    
//         because the run and the remaining 3 8's play.        
//    6) A book of four 8's with more than one 8 in a run.      
//         In this case we have to figure out which is lower    


// This function takes a hand,and returns whether or not has any 
// possibility of being a part of a book of 3 or a run of three. 
// First thing we do in lowest function is total all such cards 
// to see if they even have a shot at any sort of knock of gin  

int GinGame::total_not_in_book(int *hand, Hand *hs)
{
  int i, j, total = 0;

  hs->num = 0;
  memset((char *) hs->both, 0, sizeof(hs->both));

  for (i = 0; i < 10; i++) {
    if (!hand[i])
      continue;

    int book_needed = 2;
    int mid_run_needed = 2;
    int up_run_needed = 2;
    int down_run_needed = 2;

    for (j = 0; j < 10; j++) {
      if (j == i)
	continue;

      if (CARD_NUM(hand[i]) == CARD_NUM(hand[j]))
	book_needed--;

      if (same_suit(hand[i], hand[j])) {
	if ((CARD_NUM(hand[i]) == (CARD_NUM(hand[j]) + 1)) &&
            (CARD_NUM(hand[i] != 13))) {
	  mid_run_needed--;
	  up_run_needed--;
	} else if ((CARD_NUM(hand[i]) == (CARD_NUM(hand[j]) - 1)) &&
                   (CARD_NUM(hand[i] != 1))) {
	  mid_run_needed--;
	  down_run_needed--;
	} else if (CARD_NUM(hand[i]) == (CARD_NUM(hand[j]) + 2))
	  up_run_needed--;
	else if (CARD_NUM(hand[i]) == (CARD_NUM(hand[j]) - 2))
	  down_run_needed--;
      }
    }
    book_needed = max(0, book_needed);
    mid_run_needed = max(0, mid_run_needed);
    up_run_needed = max(0, up_run_needed);
    down_run_needed = max(0, down_run_needed);

    if (!book_needed && (!mid_run_needed || !up_run_needed || !down_run_needed))
{
      hs->both[i] = TRUE;
      hs->num++;
    } else {
      hs->both[i] = FALSE;
      if (book_needed && mid_run_needed && up_run_needed && down_run_needed)
	total += min((unsigned char) 10, CARD_NUM(hand[i]));
    }
  }
  return total;
}

// returns the number of points in a person's hand
// that is returns 0 means will be able to gin,
// returning < 10 means knocking allowed, etc.
int GinGame::can_knock_or_gin(TBeing *ch)
{
  int total, which;
  Hand hand;

  if ((which = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s got into can_knock_or_gin without being at the gin table!") %  ch->getName());
    return -1;
  }
  total = total_not_in_book(hands[which], &hand);

  // if any cards were part of both a book and a run, then we have
  // to calculate the score the hard way
  if (hand.num)
    total = recursive_gin_search(ch, &hand, hands[which]);

  return total;
}

int *GinGame::find_book(int num, int *hand, int *left)
{
  int card, i, k;

  card = hand[num];

  for (i = 0, k = -1; i < 10; i++) {
    if (i == num)
      continue;

    if (CARD_NUM(card) != CARD_NUM(hand[i]))
      left[++k] = hand[i];
  }
  return left;
}

int *GinGame::find_run(int num, int *hand, int *left)
{
  int card, i, j, l = -1, inc = 1, tmp[10], same[10];
  bool updone = FALSE, downdone = FALSE;
  bool upfound, downfound;

  memset((char *) tmp, 1, sizeof(tmp));
  memset((char *) same, 0, sizeof(same));

  card = hand[num];

  for (i = 0, j = -1; i < 10; i++) {
    if (num == i)
      continue;
    if (same_suit(card, hand[i]))
      same[++j] = hand[i];
    else
      left[++l] = hand[i];
  }
  while (!updone || !downdone) {
    upfound = downfound = FALSE;
    for (i = 0; i <= j; i++) {
      if (!updone) {
	if (CARD_NUM(card) == (CARD_NUM(same[i]) - inc)) {
	  tmp[i] = 0;
	  upfound = TRUE;
	}
      }
      if (!downdone) {
	if (CARD_NUM(card) == (CARD_NUM(same[i]) + inc)) {
	  tmp[i] = 0;
	  downfound = TRUE;
	}
      }
    }
    if (!upfound)
      updone = TRUE;
    if (!downfound)
      downdone = TRUE;
    inc++;
  }
  for (i = 0; i <= j; i++) {
    if (tmp[i])
      left[++l] = same[i];
  }
  return left;
}

int GinGame::recursive_gin_search(TBeing *ch, Hand *hs, int *hand)
{
  bool run = FALSE;
  int i, *left, *tmp_left, total = 100;
  int tmp_total;
  Hand new_hs;

  left = new int[10];
  tmp_left = new int[10];

  for (i = 0; i < 10;) {
    if (!hs->both[i]) {
      i++;
      continue;
    }
    // We now are at a card that is both in a run, and a book. What I'm  
    // gonna do here is rip off the book, then the run, figuring         
    // out each time how many points are left. We will do this double    
    // maneuver for each both spot. We will recursively call the function 
    // with the remaining cards each time to make sure we get all of it  
    // This might not be the best way to do this, but after much thought 
    // it is the best way I came up with to do it - Russ                 

    if (!run)
      left = find_book(i, hand, left);
    else
      left = find_run(i, hand, left);

    tmp_total = total_not_in_book(left, &new_hs);
    if (!new_hs.num)
      total = min(total, tmp_total);
    else {
      memcpy(tmp_left, left, sizeof(left));
      total = min(total, recursive_gin_search(ch, &new_hs, tmp_left));
    }
    if (!(run = !run))
      i++;
  }
  delete [] left;
  delete [] tmp_left;
  return total;
}

const sstring GinGame::gin_score()
{
  TBeing *ch1, *ch2;
  char buf[80];
  int score1, score2;

  ch1 = get_char_room(names[0], GIN_TABLE);
  ch2 = get_char_room(names[1], GIN_TABLE);

  if (!ch1 || !ch2) {
    vlogf(LOG_BUG, "gin_score() called without two gin players!");
    return "";
  }
  score1 = score[0];
  score2 = score[1];

  if (score1 >= score2)
    sprintf(buf, "%s : %d to %s : %d", ch1->getName(), score1, ch2->getName(), score2);
  else
    sprintf(buf, "%s : %d to %s : %d", ch2->getName(), score2, ch1->getName(), score1);

  return buf;
}


int GinGame::look(TBeing *ch, const char *arg)
{
  for (; isspace(*arg); arg++);

  if (is_abbrev(arg, "pile")) {
    if (pile_index == -1) {
      ch->sendTo("The pile has no cards in it currently.\n\r");
      return TRUE;
    }
    ch->sendTo(format("The top card on the pile is the %s.\n\r") % pretty_card_printout(ch, pile[pile_index]));
    return TRUE;
  }
  if (is_abbrev(arg, "table")) {
    look(ch, "pile");
    if (game)
      ch->sendTo(format("You see the score on a piece of paper on the corner of the table :\n\r\t%s\n\r") % gin_score());

    return TRUE;
  }
  return FALSE;
}

int GinGame::count(int which)
{
  int num = 0;

  while (hands[which][num++]);

  return (num - 1);
}

GinGame::GinGame() : 
  CardGame(),
  can_draw(false),
  topcard(0),
  deck_index(0),
  pile_index(-1)
{
  int i;
  for (i = 0; i < 2; i++) {
    *(names[i]) = '\0';
    inuse[i] = FALSE;
    loser[i] = FALSE;
    score[i] = 0;

    int j;
    for (j = 0; j < 11; j++)
      hands[i][j] = 0;
  }
  for (i = 0; i < 32; i++)
    pile[i] = 0;
}

Hand::Hand() :
  num(0)
{
  memset((char *) &both, 0, sizeof(both));
}
