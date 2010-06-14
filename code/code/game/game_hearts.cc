/*************************************************************************

      SneezyMUD++ - All rights reserved, SneezyMUD Coding Team
      "hearts.cc" - All functions and routines related to the hearts game
      
      The SneezyMUD hearts table was coded by Russ Russell, April 1994.
      Changed to c++ October 1994
      Last revision, October 26th, 1994.

*************************************************************************/

#include "room.h"
#include "being.h"
#include "low.h"
#include "monster.h"
#include "games.h"
#include "extern.h"
#include "handler.h"

HeartsGame gHearts;

const int PASS_LEFT    = 0;
const int PASS_RIGHT   = 1;
const int PASS_ACROSS  = 2;
const int PASS_NONE    = 3;

const char *which_pass[4] =
{
  "left",
  "right",
  "across",
  "no pass"
};

int HeartsGame::LEFT(const TBeing *ch) const
{
  return ((index(ch) == 3) ? 0 : index(ch) + 1);
}

int HeartsGame::RIGHT(const TBeing *ch) const
{
  return (!index(ch) ? 3 : index(ch) - 1);
}

int HeartsGame::ACROSS(const TBeing *ch) const
{
  return (index(ch) + ((index(ch) < 2) ? 2 : -2));
}

const sstring HeartsGame::hearts_score()
{
  TBeing *ch1, *ch2, *ch3, *ch4;
  char buf[256];
  int score1, score2, score3, score4;

  ch1 = get_char_room(names[0], Room::HEARTS);
  ch2 = get_char_room(names[1], Room::HEARTS);
  ch3 = get_char_room(names[2], Room::HEARTS);
  ch4 = get_char_room(names[3], Room::HEARTS);

  if (!ch1 || !ch2 || !ch3 || !ch4) {
    vlogf(LOG_BUG, "hearts_score() called without four hearts players!");
    return "";
  }
  score1 = score[0];
  score2 = score[1];
  score3 = score[2];
  score4 = score[3];

  sprintf(buf, "\n\r%s : %d %s : %d %s : %d %s : %d\n\r",
          ch1->getName(), score1, ch2->getName(), score2,
          ch3->getName(), score3, ch4->getName(), score4);

  return buf;
}

bool HeartsGame::get_other_players(const TBeing *ch, TBeing **left, TBeing **across, TBeing **right)
{
  int which;

  if ((which = index(ch)) < 0) 
    return FALSE;
  
  *left = get_char_room(names[LEFT(ch)], Room::HEARTS);
  *across = get_char_room(names[ACROSS(ch)], Room::HEARTS);
  *right = get_char_room(names[RIGHT(ch)], Room::HEARTS);

  if (!*left || !*right || !*across)
    return FALSE;

  return TRUE;
}

void HeartsGame::deal(TBeing *ch)
{
  int i, j = 0, which;
  TBeing *left = NULL, *across = NULL, *right = NULL;

  if ((which = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s got into HeartsGame::deal without being at the hearts table!") %  ch->getName());
    return;
  }
  if (game) {
    ch->sendTo("Redeal while the game is in progress? Never!!\n\r");
    return;
  }
  if (get_other_players(ch, &left, &across, &right) == FALSE) {
    ch->sendTo("There aren't 4 players at the table. You can't deal!\n\r");
    return;
  }
  shuffle();

  ch->sendTo("You shuffle the cards, and deal them.\n\r");
  act("$n shuffles the cards and deals them.", FALSE, ch, NULL, NULL, TO_ROOM);

  ch->sendTo("You are dealt:\n\r");
  left->sendTo("You are dealt:\n\r");
  across->sendTo("You are dealt:\n\r");
  right->sendTo("You are dealt:\n\r");

  for (i = 0; i < 13; i++) {
    hands[which][i] = deck[j++];
    hands[LEFT(ch)][i] = deck[j++];
    hands[ACROSS(ch)][i] = deck[j++];
    hands[RIGHT(ch)][i] = deck[j++];
  }
  game = TRUE;
  passing = (ipass != PASS_NONE);
  done_passing = !passing;
  if (done_passing)
    canplay = find_two_of_clubs();

  round = 0;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 3; j++)
      passes[i][j] = 0;

    canpass[i] = TRUE;
    cangetpass[i] = TRUE;
  }
  iplay = 0;

  ch->doPeek();
  left->doPeek();
  across->doPeek();
  right->doPeek();

  ch->sendTo(format("The pass for this hand is %s.\n\r") % which_pass[ipass]);
  left->sendTo(format("The pass for this hand is %s.\n\r") % which_pass[ipass]);
  right->sendTo(format("The pass for this hand is %s.\n\r") % which_pass[ipass]);
  across->sendTo(format("The pass for this hand is %s.\n\r") % which_pass[ipass]);
}

void HeartsGame::peek(const TBeing *ch)
{
  int i, which;

  if ((which = index(ch)) < 0) {
    ch->sendTo("You aren't at the hearts table to peek at any cards.\n\r");
    return;
  }
  if (!game) {
    ch->sendTo("You have no cards, there is currently no game!\n\r");
    return;
  }
  ch->sendTo("You have the following cards:\n\r");
  ch->sendTo("-----------------------------\n\r");

  for (i = 0; i < 13; i++) {
    if (hands[which][i])
      ch->sendTo(format("%2d) %-5s | %s\n\r") % (i+1) % card_names[CARD_NUM(hands[which][i])] %
	    suit(ch, hands[which][i]));
  }
  return;
}

int HeartsGame::move_card(TBeing *ch, const char *arg)
{
  int i, orig, n, which, tmp;

  if ((which = index(ch)) < 0) {
    ch->sendTo("You aren't at a hearts table to move any cards around!\n\r");
    return FALSE;
  }
  if (sscanf(arg, "%d %d", &orig, &n) == 2) {
    if ((orig < 1) || (orig > 13) || (n < 1) || (n > 13)) {
      ch->sendTo("Hearts table syntax : put <original card place number>< new place number>\n\r");
      return FALSE;
    }
    if (orig == n) {
      ch->sendTo(format("The number %d card is already in the number %d slot!\n\r") % orig % n);
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
  } else {
    ch->sendTo("Hearts table syntax : put <original card place number> <new place number>\n\r");
    return FALSE;
  }
  return TRUE;
}

bool TBeing::checkHearts(bool inGame) const
{
  if (in_room == Room::HEARTS && (inGame || (gHearts.index(this) > -1)))
    return TRUE;
  else
    return FALSE;
}

int HeartsGame::enter(const TBeing *ch)
{
  int which = 0, i;

  if (dynamic_cast<const TMonster *>(ch)) {
    ch->sendTo("Dumb monsters can't play hearts!\n\r");
    return FALSE;
  }
  if (inuse[0] && inuse[1] && inuse[2] && inuse[3]) {
    ch->sendTo("There are already four players at the hearts table!\n\r");
    return FALSE;
  } else if (ch->getPosition() == POSITION_SITTING) {
    ch->sendTo("You are already sitting at the hearts table.\n\r");
    return FALSE;
  } else {
    for (i = 0; i < 4; i++) {
      if (!inuse[i]) {
	which = i;
        break;
      }
    }
    ch->sendTo("You sit down at the hearts table.\n\r");
    strcpy(names[which], ch->getName());
    inuse[which] = TRUE;
    score[which] = 0;
    game = FALSE;
    ipass = PASS_LEFT;
    return TRUE;
  }
}

int HeartsGame::exitGame(const TBeing *ch)
{
  int which;
  TBeing *left = NULL, *across = NULL, *right = NULL;

  if ((which = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s left a hearts table %s wasn't at!") %  ch->getName() % ch->hssh());
    return FALSE;
  }
  ch->sendTo("You leave the hearts table.\n\r");
  act("$n stands up and leaves the hearts table, totally mooting the game.", FALSE, ch, NULL, NULL, TO_ROOM);

  get_other_players(ch, &left, &across, &right);

  if (left) {
    left->sendTo("You stand up and leave the table as well.\n\r");
    left->setPosition(POSITION_STANDING);
  }
  if (across) {
    across->sendTo("You stand up and leave the table as well.\n\r");
    across->setPosition(POSITION_STANDING);
  }
  if (right) {
    right->sendTo("You stand up and leave the table as well.\n\r");
    right->setPosition(POSITION_STANDING);
  }
  // Clear out and zero all necessary gHearts variables. 
  *(names[0]) = '\0';
  *(names[1]) = '\0';
  *(names[2]) = '\0';
  *(names[3]) = '\0';
  game = FALSE;
  inuse[0] = FALSE;
  inuse[1] = FALSE;
  inuse[2] = FALSE;
  inuse[3] = FALSE;
  setup_deck();
  return TRUE;
}

int HeartsGame::index(const TBeing *ch) const
{
  int i;

  for (i = 0; i < 4; i++) {
    if (!strcmp(ch->getName(), names[i]))
      return i;
  }
  return -1;
}

int game_over()
{
  return FALSE;
}

int HeartsGame::new_deal()
{
  int scores[4];
  int i, j, countx = 0;
  TBeing *ch1, *ch2, *ch3, *ch4;

  scores[0] = scores[1] = scores[2] = scores[3] = 0;

  for (i = 0; i < 13; i++) {
    for (j = 0; j < 4; j++) {
      if (is_heart(tricks[i][j])) 
        countx += 1;
      if (is_queen_of_spades(tricks[i][j]))
        countx += 13;
    }
    if (!in_range(tricks[i][4], 0, 3)) {
       vlogf(LOG_BUG, format("Bad number in HeartsGame::new_deal for winner of trick! (%d)") %  tricks[i][4]);
       return FALSE;
    }
    scores[tricks[i][4]] += countx;
    countx = 0;
  }
  score[0] += (scores[0] == 26) ? (-26) : scores[0];
  score[1] += (scores[1] == 26) ? (-26) : scores[1];
  score[2] += (scores[2] == 26) ? (-26) : scores[2];
  score[3] += (scores[3] == 26) ? (-26) : scores[3];

  if (game_over())
    return TRUE;
  else {
    broken = FALSE;
    memset((char *) pile, 0, sizeof(pile));
    memset((char *) tricks, 0, sizeof(pile));
    game = FALSE;
    led = 0;
    iplay = 0;
    
    ch1 = get_char_room(names[0], Room::HEARTS);
    ch2 = get_char_room(names[1], Room::HEARTS);
    ch3 = get_char_room(names[2], Room::HEARTS);
    ch4 = get_char_room(names[3], Room::HEARTS);
  
    if (!ch1 || !ch2 || !ch3 || !ch4) {
      vlogf(LOG_BUG, "HeartsGame::new_deal called without four hearts players!");
      return FALSE;
    }
    ipass = (ipass == PASS_NONE) ? PASS_LEFT : ipass + 1;
    deal(ch1);
    ch1->sendTo(format("The score is now %s.\n\r") % hearts_score());
    ch2->sendTo(format("The score is now %s.\n\r") % hearts_score());
    ch3->sendTo(format("The score is now %s.\n\r") % hearts_score());
    ch4->sendTo(format("The score is now %s.\n\r") % hearts_score());
    if (ipass == PASS_NONE) {
      passing = FALSE;
      canplay = find_two_of_clubs();
      firstplay = TRUE;
    }
  }
  return TRUE;
}


int HeartsGame::new_round(TBeing *ch, int *pilex)
{
  TBeing *won, *left, *right, *across;
  int i, which, winner = 0, wincard = 0, high = 0;

  if ((which = index(ch)) < 0) {
    vlogf(LOG_BUG, "HeartsGame::new_round called with ch not at hearts table!");
    return FALSE;
  }
  for (i = 0; i < 4; i++) {
    if (same_suit(pilex[i], led)) {
      if ((CARD_NUM(pilex[i]) == 1) || (CARD_NUM(pilex[i]) > high)) {
        high = (CARD_NUM(pilex[i]) == 1) ? 14 : CARD_NUM(pilex[i]);
	wincard = i;
      }
    }
  }
  get_other_players(ch, &left, &across, &right);

  switch (which) {
    case 0:
      winner = ((wincard == 3) ? 0 : (wincard + 1));
      break;
    case 1:
      winner = (wincard + ((wincard < 2) ? 2 : -2));
      break;
    case 2:
      winner = (!wincard ? 3 : (wincard - 1));
      break;
    case 3:
      winner = wincard;
      break;
  }

  if (!(won = get_char_room(names[winner], Room::HEARTS))) {
    vlogf(LOG_BUG, "Null character for won in HeartsGame::new_round()");
    return FALSE;
  }
  sendrpf(won->roomp, "%s takes the trick with the %s.\n\r", won->getName(), pretty_card_printout(NULL, pilex[wincard]).c_str());
  won->sendTo(format("You take the trick with the %s.\n\r") % pretty_card_printout(ch, pilex[wincard]));

  tricks[round][4] = winner;

  for (i = 0; i < 4; i++) {
    tricks[round][i] = pilex[i];
    pilex[i] = 0;
  }
  if (++round == 13) 
    new_deal();
  else {
    led = 0;
    iplay = 0;
    canplay = winner;
  }
  return TRUE;
}


void HeartsGame::play(TBeing *ch, const char *arg)
{
  int which, card;

  for (; isspace(*arg); arg++);

  if ((which = index(ch)) < 0) {
    ch->sendTo("You can't play a card when not at the hearts table!\n\r");
    return;
  }
  if (passing) {
    ch->sendTo("You can't play until all passing has been done!\n\r");
    return;
  }
  if (canplay != which) {
    ch->sendTo("It isn't your turn to play!\n\r");
    return;
  }
  if (sscanf(arg, "%d", &card) != 1) {
    ch->sendTo("Hearts table syntax : play <card number>\n\r");
    return;
  }
  card--;

  if (card < 0 || card > 12) {
    ch->sendTo("Hearts table syntax : play <card number>\n\r");
    return;
  }
  if (hands[which][card]) {
    if (firstplay) {
      if ((CARD_NUM(hands[which][card]) !=2) || !(hands[which][card] & CARD_WATER)) {
        ch->sendTo("You must play the 2 of WATER on the first play of the round.\n\r");
        ch->sendTo("Consult the hearts help file for more information.\n\r");
        return;
      }
    }
    if (!iplay) {
      if (is_heart(hands[which][card]) && !broken &&
          // don't allow it to lead, UNLESS, it is the only suit i have
          (has_suit(hands[which], CARD_WATER) ||
           has_suit(hands[which], CARD_EARTH) ||
           has_suit(hands[which], CARD_ETHER))) {
	ch->sendTo("Hearts must be broken before they are led.\n\r");
	ch->sendTo("Consult the hearts help file for more information.\n\r");
	return;
      }
      led = hands[which][card];
    } else if (led && !same_suit(hands[which][card], led) && has_suit(hands[which], get_suit(led))) {
      ch->sendTo("You must play the led suit if you have it.\n\r");
      ch->sendTo("Consult the hearts help file for more information.\n\r");
      return;
    }
    pile[iplay] = hands[which][card];
    broken |= is_heart(pile[iplay]);
    firstplay = FALSE;
    take_card_from_hand(hands[which], card, 12);
    ch->sendTo(format("You play the %s.\n\r") % pretty_card_printout(ch, pile[iplay]));

    for(StuffIter it= ch->roomp->stuff.begin();it!= ch->roomp->stuff.end();++it) {
      TBeing *tbt = dynamic_cast<TBeing *>(*it);
      if (tbt && (tbt != ch))
        tbt->sendTo(COLOR_MOBS, format("%s plays the %s.\n\r") % ch->getName() % pretty_card_printout(tbt, pile[iplay]));
    }
    if (++iplay == 4)
      new_round(ch, pile);
    else
      canplay = LEFT(ch);

    return;
  } else {
    ch->sendTo("You don't have a card in that slot to play!\n\r");
    return;
  }
}

void HeartsGame::pass(TBeing *ch, const char *arg)
{
  int which, pass1, pass2, pass3;

  for (; isspace(*arg); arg++);

  if ((which = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s got into hearts_pass without being at the hearts table!\n\r") %  ch->getName());
    return;
  }
  if (!passing || !canpass[which]) {
    ch->sendTo("Passing is done at the beginning of each round.\n\r");
    return;
  }
  if (!hands[which][12]) {
    ch->sendTo("You can't pass without all your cards!\n\r");
    return;
  }
  if (sscanf(arg, "%d %d %d", &pass1, &pass2, &pass3) != 3) {
    ch->sendTo("Hearts table syntax : pass <cardnum> <cardnum> <cardnum>\n\r");
    return;
  }
  if (!in_range(pass1--, 1, 13) || 
      !in_range(pass2--, 1, 13) ||
      !in_range(pass3--, 1, 13) ||
      (pass1 == pass2) || (pass2 == pass3) || (pass1 == pass3)) {
    ch->sendTo("Hearts table syntax : pass <cardnum> <cardnum> <cardnum>\n\r");
    return;
  }
  passes[which][0] = hands[which][pass1];
  passes[which][1] = hands[which][pass2];
  passes[which][2] = hands[which][pass3];
  order_high_to_low(&pass1, &pass2, &pass3);
  take_card_from_hand(hands[which], pass1, 12);
  take_card_from_hand(hands[which], pass2, 12);
  take_card_from_hand(hands[which], pass3, 12);
  act("$n places $s three card pass in front of $m.", FALSE, ch, NULL, NULL, TO_ROOM);
  ch->sendTo("You place your pass down in front of you.\n\r");
  canpass[which] = FALSE;
  return;
}

// returns the index of the owner of the 2 of water
int HeartsGame::find_two_of_clubs()
{
  int i, j;

  for (i = 0; i < 13; i++) {
    for (j = 0; j < 4; j++) {
      if ((CARD_NUM(hands[j][i]) == 2) && (hands[j][i] & CARD_WATER))
        return j;
    }
  }
  return -1;
}


int HeartsGame::get_pass(TBeing *ch, char *arg)
{
  int which;
  int passed_from = 0;
  char buf1[80], buf2[80], buf3[80];

  for (; isspace(*arg); arg++);

  if ((which = index(ch)) < 0)
    return FALSE;
  
  if (is_abbrev(arg, "pass")) {
    if (hands[which][12]) {
      ch->sendTo("You can't get your pass with all your cards!\n\r");
      return TRUE;
    }
    if (!passing || !cangetpass[which]) {
      ch->sendTo("You can't do that now, sorry.\n\r");
      return TRUE;
    }
    switch (ipass) {
      case PASS_LEFT:
        passed_from = RIGHT(ch);
        break;
      case PASS_RIGHT:
        passed_from = LEFT(ch);
        break;
      case PASS_ACROSS:
        passed_from = ACROSS(ch);
        break;
    }
    if (!passes[passed_from][0]) {
      ch->sendTo("You haven't been passed your cards yet!\n\r");
      return TRUE;
    }
    hands[which][10] = passes[passed_from][0];
    hands[which][11] = passes[passed_from][1];
    hands[which][12] = passes[passed_from][2];
  
    /* Since pretty_card_printout returns a static, I have to */
    /* put them into temporary buffers - Russ                 */
  
    strcpy(buf1, pretty_card_printout(ch, passes[passed_from][0]).c_str());
    strcpy(buf2, pretty_card_printout(ch, passes[passed_from][1]).c_str());
    strcpy(buf3, pretty_card_printout(ch, passes[passed_from][2]).c_str());
  
    ch->sendTo(format("You pick up the pile and get the %s, %s, and %s.\n\r") % buf1 % buf2 % buf3);
  
    cangetpass[which] = FALSE;
    if (++done_passing == 4) {
      passing = FALSE;
      canplay = find_two_of_clubs();
      firstplay = TRUE;
    }
    return TRUE;
  }
  return FALSE;
}

int HeartsGame::look(TBeing *ch, const char *arg)
{
  TBeing *left = NULL, *across = NULL, *right = NULL;
  for (; isspace(*arg); arg++);

  if (is_abbrev(arg, "table")) {
    get_other_players(ch, &left, &across, &right);
    if (game)
      ch->sendTo(format("You see the score on a piece of paper on the corner of the table : %s\n\r") % hearts_score());

    if (!pile[0])
      ch->sendTo("No cards are on the table.\n\r");
    else {
      ch->sendTo("You see the following cards:\n\r");
      for (int i = 0; i < 4; i++) {
        if (pile[i])
          ch->sendTo(format("%s\n\r") % pretty_card_printout(ch, pile[i]));
      }
    }
    ch->sendTo(format("%s sits to your left, %s across from you, and %s to your right.\n\r") %
	       (left ? left->getName() : "No one") %
	       (across ? across->getName() : "no one") %
	       (right ? right->getName() : "no one"));

    return TRUE;
  }
  return FALSE;
}

int HeartsGame::count(int which)
{
  int num = 0;

  while (hands[which][num++]) {
    if (num == 13)
      return num;
  }
  return num - 1;
}

HeartsGame::HeartsGame() :
  CardGame(),
  iplay(0),
  canplay(0),
  led(0),
  broken(false),
  round(0),
  done_passing(0),
  ipass(0),
  passing(0),
  firstplay(0)
{
  int i, j;
  for (i = 0; i < 4; i++) {
    *(names[i]) = '\0';
    score[i] = 0;
    canpass[i] = false;
    cangetpass[i] = false;
    inuse[i] = false;
    pile[i] = 0;

    for (j = 0; j < 3; j++)
      passes[i][j] = 0;

    for (j = 0; j < 13; j++)
      hands[i][j] = 0;
  }
  for (i = 0; i < 13; i++)
    for (j = 0; j < 5; j++)
      tricks[i][j] = 0;
}

