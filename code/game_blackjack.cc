//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "blackjack.c" - All functions and routines related to blackjack
//      
//      The blackjack table coded by Russ Russell, January 1993, 
//      Changed to c++ October 1994
//      Last revision, October 13th, 1994.
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "games.h"

const unsigned short MAX_BLACKJACK = 1;

BjGame gBj;

bool TBeing::checkBlackjack(bool inGame) const
{
  if (in_room == ROOM_BLACKJACK && (inGame || (gBj.index(this) > -1)))
    return true;
  else
    return false;
}

void BjGame::bj_shuffle(int, const TBeing *ch)
{
  act("The ghostly dealer shuffles the deck.",FALSE, ch, 0, 0, TO_CHAR);
  act("The ghostly dealer shuffles the deck.",FALSE, ch, 0, 0, TO_ROOM);

  shuffle();
  deck_inx = 0;
}

bool BjGame::enter(const TBeing *ch)
{
  int player, inx;

  for (player = 0, inx = -1; player < MAX_BLACKJACK; player++) {
    if (!strcmp(ch->name, name)) {
      inx = player;
      ch->sendTo("The dealer says, 'Ah, you have returned.'\n\r");
    }
    if ((inx < 0) && !inuse)
      inx = player;
  }
  if (inx < 0) {
    ch->sendTo("The table seems to be full.\n\r");
    if (ch->isImmortal())
      ch->sendTo(fmt("%s is at the table.\n\r") % name);
    return FALSE;
  }
  ch->sendTo("You move up to the blackjack table.\n\r");
  inuse = TRUE;
  strcpy(name, ch->name);
  bj_shuffle(inx, ch);
  bet = 0;
  return TRUE;
}

int BjGame::exitGame(const TBeing *ch)
{
  int inx, i;

  if ((inx = index(ch)) < 0) {
    vlogf(LOG_BUG, fmt("%s left a table he was not at!") %  ch->name);
    return FALSE;
  }
  *name = '\0';
  inuse = FALSE;
  deck_inx = 0;
  bet = 0;
  np = 0;
  nd = 0;
  for (i = 0; i < 52; i++) {
    if (i < 12) {
      hand[i] = 0;
      dealer[i] = 0;
    }
    deck[i] = 0;
  }
  setup_deck();
  ch->sendTo("You leave the blackjack table.\n\r");
  return TRUE;
}


void BjGame::Bet(TBeing *ch, const char *arg)
{
  int inx, player;
  char coin_str[20], log_msg[2048];
  sstring buf;
  TObj *chip;

  if (ch->checkBlackjack()) {
    inx = index(ch);
    if (inx < 0) {
      ch->sendTo("You are not sitting at the table yet.\n\r");
      return;
    }
    if (bet > 0) {
      ch->sendTo("You can't change your bet now.\n\r");
      return;
    }
    strcpy(coin_str, arg);
    if (!*coin_str) {
      ch->sendTo("Bet which chip?\n\r");
      return;
    }

    if(!(chip=find_chip(ch, coin_str))){
      ch->sendTo("You don't have that chip!\n\r");
      return;
    }

    bet = chip->obj_flags.cost;
    ch->doSave(SILENT_YES);

    sstring buf;
    buf = fmt("$n bets %s.") % chip->getName();
    act(buf, TRUE, ch, 0, 0, TO_ROOM);
    buf = fmt("You bet %s.") % chip->getName();
    act(buf, TRUE, ch, 0, 0, TO_CHAR);

    (*chip)--;
    delete chip;

    nd = 0;
    np = 0;
    for (player = 0; player < 12; hand[player] = 0, dealer[player] = 0, player++);
    if (deck_inx > 10)
      bj_shuffle(inx, ch);

    hand[np++] = deck[deck_inx++];
    hand[np++] = deck[deck_inx++];
    dealer[nd++] = deck[deck_inx++];
    dealer[nd++] = deck[deck_inx++];

    sprintf(log_msg, "You are dealt:\n\r%s (down)\n\r", pretty_card_printout(ch, hand[0]).c_str());
    sprintf(log_msg + strlen(log_msg), "%s\n\r\n\r", pretty_card_printout(ch, hand[1]).c_str());
    sprintf(log_msg + strlen(log_msg), "The dealer is showing:\n\r%s\n\r", pretty_card_printout(ch, dealer[1]).c_str());
    ch->sendTo(COLOR_BASIC, log_msg);

    sprintf(log_msg, "$n is dealt:\n\r%s (down)\n\r", pretty_card_printout(ch, hand[0]).c_str());
    sprintf(log_msg + strlen(log_msg), "%s\n\r\n\r", pretty_card_printout(ch, hand[1]).c_str());
    sprintf(log_msg + strlen(log_msg), "The dealer is showing:\n\r%s", pretty_card_printout(ch, dealer[1]).c_str());
    act(log_msg, TRUE, ch, 0, 0, TO_ROOM);


    if (((CARD_NUM(hand[0]) == 1) && (CARD_NUM(hand[1]) >= 10)) ||
	((CARD_NUM(hand[1]) == 1) && (CARD_NUM(hand[0]) >= 10))) {
      ch->sendTo("You get a blackjack!\n\r");
      act("$n gets a blackjack!", TRUE, ch, 0, 0, TO_ROOM);
      payout(ch, (int) (bet * 2.5));
      bet = 0;
      observerReaction(ch, GAMBLER_WON);
    }
    if (((CARD_NUM(dealer[0]) == 1) && (CARD_NUM(dealer[1]) >= 10)) ||
	((CARD_NUM(dealer[1]) == 1) && (CARD_NUM(dealer[0]) >= 10))) {
      ch->sendTo("The dealer gets a blackjack!\n\r");
      act("The dealer gets a blackjack!", TRUE, ch, 0, 0, TO_ROOM);
      bet = 0;
      observerReaction(ch, GAMBLER_LOST);
    }

    if(bet>0)
      observerReaction(ch, GAMBLER_BLACKJACK_BET);
  }
}

void TBeing::doStay()
{
  int inx;

  if (checkBlackjack()) {
    if ((inx = gBj.index(this)) < 0) {
      sendTo("You are not sitting at the table yet.\n\r");
      return;
    }
    if (!gBj.check_for_bet()) {
      sendTo("You are not playing a game.\n\r");
      return;
    }
    gBj.stay(this);
  } else if(checkHiLo()){
    if ((inx = gHiLo.index(this)) < 0){
      sendTo("You are not sitting at the table yet.\n\r");
      return;
    }
    
    if(!gHiLo.check_for_bet()) {
      sendTo("You are not playing a game.\n\r");
      return;
    }
    gHiLo.stay(this);
  } else if(checkPoker()){
    if ((inx = gPoker.index(this)) < 0){
      sendTo("You are not sitting at the table yet.\n\r");
      return;
    }
    
    if(!gPoker.check_for_bet()) {
      sendTo("You are not playing a game.\n\r");
      return;
    }
    gPoker.stay(this);
  } else if(checkBaccarat()){
    if ((inx = gBaccarat.index(this)) < 0){
      sendTo("You are not sitting at the table yet.\n\r");
      return;
    }
    
    if(!gBaccarat.check_for_bet()) {
      sendTo("You are not playing a game.\n\r");
      return;
    }
    gBaccarat.stay(this);    
  } else
    sendTo("So you think you are in a casino?\n\r");
}

void BjGame::stay(TBeing *ch)
{
  int pbest, dbest, player;
  char log_msg[2048];

  sprintf(log_msg, "The dealer flips up his down card:\n\r%s\n\r", 
      pretty_card_printout(ch, dealer[0]).c_str());
  ch->sendTo(COLOR_BASIC, log_msg);
  act(log_msg, TRUE, ch, 0, 0, TO_ROOM);

  player = best_dealer();
  while (player < 17) {
    dealer[nd++] = deck[deck_inx++];

    //    if((best_dealer() > 21) && !::number(0,3))
    //      dealer[nd-1] = deck[deck_inx++];

    sprintf(log_msg, "The dealer is dealt the ");
    strcat(log_msg, card_names[dealer[nd - 1] & 0x0f]);
    add_suit(ch, log_msg, dealer[nd - 1]);
    strcat(log_msg, ".\n\r");

    ch->sendTo(COLOR_BASIC, log_msg);
    act(log_msg, TRUE, ch, 0, 0, TO_ROOM);

    player = best_dealer();
    if (player > 21) {
      ch->sendTo(fmt("The dealer busts with %d.\n\r") % player);
      sprintf(log_msg, "The dealer busts with %d.", player);
      act(log_msg, TRUE, ch, 0, 0, TO_ROOM);
      break;
    }
  }
  strcpy(log_msg, "Your final hand:\n\r");
  for (player = 0; player < np; player++) {
    strcat(log_msg, card_names[hand[player] & 0x0F]);
    add_suit(ch, log_msg, hand[player]);
    strcat(log_msg, "\r\n");
  }
  strcat(log_msg, "\n\rThe dealer final hand:\n\r");
  for (player = 0; player < nd; player++) {
    strcat(log_msg, card_names[dealer[player] & 0x0F]);
    add_suit(ch, log_msg, dealer[player]);
    strcat(log_msg, "\r\n");
  }
  ch->sendTo(COLOR_BASIC, log_msg);

  strcpy(log_msg, "$n's final hand:\n\r");
  for (player = 0; player < np; player++) {
    strcat(log_msg, card_names[hand[player] & 0x0F]);
    add_suit(ch, log_msg, hand[player]);
    strcat(log_msg, "\r\n");
  }
  strcat(log_msg, "\n\rThe dealer final hand:\n\r");
  for (player = 0; player < nd; player++) {
    strcat(log_msg, card_names[dealer[player] & 0x0F]);
    add_suit(ch, log_msg, dealer[player]);
    strcat(log_msg, "\r\n");
  }
  act(log_msg, TRUE, ch, 0, 0, TO_ROOM);

  pbest = best_score();
  dbest = best_dealer();

  if (pbest > dbest || dbest > 21) {
    ch->sendTo("You win.\n\r");
    act("$n wins.", TRUE, ch, 0, 0, TO_ROOM);
    payout(ch, (int) (bet * 2));
    observerReaction(ch, GAMBLER_WON);
  } else if (pbest == dbest) {
    ch->sendTo("You push. You retain your original bid.\n\r");
    act("$n pushes.", TRUE, ch, 0, 0, TO_ROOM);
    payout(ch, (int) (bet));
  } else {
    ch->sendTo("You lose.\n\r");
    act("$n loses.", TRUE, ch, 0, 0, TO_ROOM);
    observerReaction(ch, GAMBLER_LOST);
  }

  bet = 0;
}

void BjGame::peek(const TBeing *ch)
{
  int inx, player;
  char log_msg[2048];

  if ((inx = index(ch)) < 0) {
    ch->sendTo("You are not sitting at the table yet.\n\r");
    return;
  }
  if (!bet) {
    ch->sendTo("You are not playing a game.\n\r");
    return;
  }
  strcpy(log_msg, "You peek at your hand:\n\r");
  for (player = 0; player < np; player++) {
    strcat(log_msg, card_names[hand[player] & 0x0F]);
    add_suit(ch, log_msg, hand[player]);
    if (!player)
      strcat(log_msg, " (down)");
    strcat(log_msg, "\r\n");
  }
  strcat(log_msg, "\n\rThe dealer is showing:\n\r");
  strcat(log_msg, card_names[dealer[1] & 0x0f]);
  add_suit(ch, log_msg, dealer[1]);
  strcat(log_msg, "\n\r");
  ch->sendTo(COLOR_BASIC, log_msg);
}

void BjGame::Split(TBeing *ch, const char *, int)
{
  ch->sendTo("Not implemented yet.\n\r");
}

void BjGame::Hit(const TBeing *ch)
{
  int inx;
  char log_msg[2048];

  inx = index(ch);
  if (inx < 0) {
    ch->sendTo("You are not sitting at the table yet.\n\r");
    return;
  }
  if (!bet) {
    ch->sendTo("You are not playing a game.\n\r");
    return;
  }
  hand[np++] = deck[deck_inx++];

  sprintf(log_msg, "You are dealt the ");
  strcat(log_msg, card_names[hand[np - 1] & 0x0f]);
  add_suit(ch, log_msg, hand[np - 1]);
  strcat(log_msg, ".\n\r");

  ch->sendTo(COLOR_BASIC, log_msg);

  sprintf(log_msg, "$n is dealt the ");
  strcat(log_msg, card_names[hand[np - 1] & 0x0f]);
  add_suit(ch, log_msg, hand[np - 1]);
  strcat(log_msg, ".");
  act(log_msg, TRUE, ch, 0, 0, TO_ROOM);


  if (min_score() > 21) {
    ch->sendTo("You have busted!\n\r");
    act("$n has busted!", TRUE, ch, 0, 0, TO_ROOM);
    bet = 0;
  }
}

int BjGame::index(const TBeing *ch) const
{
  int player, inx;

  for (player = 0, inx = -1; inx < 0 && player < MAX_BLACKJACK; player++) {
    if (!strcmp(ch->name, name))
      inx = player;
  }
  return inx;
}


int BjGame::min_score()
{
  int player, player2;

  for (player = 0, player2 = 0; player < np; player++) {
    if (CARD_NUM(hand[player]) > 10)
      player2 += 10;
    else
      player2 += CARD_NUM(hand[player]);
  }
  return player2;
}

int BjGame::best_dealer()
{
  int player, player2, player3;

  for (player = 0, player2 = 0, player3 = 0; player < nd; player++) {
    if (CARD_NUM(dealer[player]) > 10)
      player2 += 10;
    else
      player2 += CARD_NUM(dealer[player]);

    if (CARD_NUM(dealer[player]) == 1)
      player3++;
  }
  for (player = 0; player < player3; player++) {
    if ((21 - player2) >= 10)
      player2 += 10;
  }
  return player2;
}

int BjGame::best_score()
{
  int player = 0, player2 = 0, player3 = 0;

  for (; player < np; player++) {
    if (CARD_NUM(hand[player]) > 10)
      player2 += 10;
    else
      player2 += CARD_NUM(hand[player]);

    if (CARD_NUM(hand[player]) == 1)
      player3++;
  }
  for (player = 0; player < player3; player++) {
    if ((21 - player2) >= 10)
      player2 += 10;
  }
  return player2;
}

BjGame::BjGame() :
  CardGame(),
  inuse(false),
  np(0),
  nd(0),
  deck_inx(0)
{
  *name = '\0';
  memset(&hand, 0, sizeof(hand));
  memset(&dealer, 0, sizeof(dealer));
}
