/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "game_crazyeights.cc"
  All functions and routines related to the crazy eights card game.

  Created 4/24/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

#include "being.h"
#include "monster.h"
#include "games.h"
#include "extern.h"
#include "handler.h"
#include "game_crazyeights.h"

CrazyEightsGame gEights;

const int ROOM_CRAZYEIGHTS = 1;

int CrazyEightsGame::LEFT(const TBeing *ch) const
{
  return ((index(ch) == 3) ? 0 : index(ch) + 1);
}

int CrazyEightsGame::RIGHT(const TBeing *ch) const
{
  return (!index(ch) ? 3 : index(ch) - 1);
}

int CrazyEightsGame::ACROSS(const TBeing *ch) const
{
  return (index(ch) + ((index(ch) < 2) ? 2 : -2));
}

int CrazyEightsGame::count(int playerNum)
{
  if (playerNum < 0 || playerNum > 4)
    return 0;

  for (int cardIndex = 0; cardIndex < 32; cardIndex++)
    if (!hands[playerNum][cardIndex])
      return cardIndex;

  return 32;
}

const sstring CrazyEightsGame::score()
{
  TBeing *ch1,
         *ch2,
         *ch3,
         *ch4;
  char    tString[256];

  ch1 = get_char_room(names[0], ROOM_CRAZYEIGHTS);
  ch2 = get_char_room(names[1], ROOM_CRAZYEIGHTS);
  ch3 = get_char_room(names[2], ROOM_CRAZYEIGHTS);
  ch4 = get_char_room(names[3], ROOM_CRAZYEIGHTS);

  if (!ch1 || !ch2 || !ch3 || !ch4) {
    vlogf(LOG_BUG, "CrazyEights::score() called without four eights players!");
    return "";
  }

  sprintf(tString, "\n\r%s[%d] %s[%d] %s[%d] %s[%d]\n\r",
          ch1->getName(), scores[0], ch2->getName(), scores[1],
          ch3->getName(), scores[2], ch4->getName(), scores[3]);

  return tString;
}

bool CrazyEightsGame::getPlayers(const TBeing *ch, TBeing **ch2, TBeing **ch3, TBeing **ch4)
{
  if (index(ch) < 0)
    return false;

  *ch2 = get_char_room(names[LEFT(ch)  ], ROOM_CRAZYEIGHTS);
  *ch3 = get_char_room(names[ACROSS(ch)], ROOM_CRAZYEIGHTS);
  *ch4 = get_char_room(names[RIGHT(ch) ], ROOM_CRAZYEIGHTS);

  if (!*ch2 || !*ch3 || !*ch4)
    return false;

  return true;
}

void CrazyEightsGame::deal(TBeing *ch)
{
  TBeing *ch2 = NULL,
         *ch3 = NULL,
         *ch4 = NULL;
  int     pointCard = 0,
          dealerNum = 0;
  char    tString[256];

  if (game) {
    ch->sendTo("Redeal while the game is in progress?  Never!!\n\r");
    return;
  }

  if ((dealerNum = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s got into CrazyEights::deal without being at the eights table!") % 
          ch->getName());
    return;
  }

  if (!getPlayers(ch, &ch2, &ch3, &ch4)) {
    ch->sendTo("You must have 4 players, and there isn't 4.\n\r");
    return;
  }

  shuffle();

  ch->sendTo("You shuffle the cards, and deal them.\n\r");
  act("$n shuffles the cards and deals them.",
      FALSE, ch, NULL, NULL, TO_ROOM);

  ch->sendTo("You are dealt:\n\r");
  ch2->sendTo("You are dealt:\n\r");
  ch3->sendTo("You are dealt:\n\r");
  ch4->sendTo("You are dealt:\n\r");

  memset((char *)hands, 0, sizeof(hands));

  for (int indexCard = 0; indexCard < 5; indexCard++) {
    hands[dealerNum ][indexCard] = deck[pointCard++];
    hands[LEFT(ch)  ][indexCard] = deck[pointCard++];
    hands[ACROSS(ch)][indexCard] = deck[pointCard++];
    hands[RIGHT(ch) ][indexCard] = deck[pointCard++];
  }

  game = true;
  iplay = 0;
  ch->doPeek();
  ch2->doPeek();
  ch3->doPeek();
  ch4->doPeek();
  starterCard  = deck[pointCard++];
  nextCard     = pointCard;
  nextPlayer   = LEFT(ch);
  initialPlayer = index(ch);

  strcpy(tString, pretty_card_printout(ch, starterCard).c_str());

  ch->sendTo(format("You turn the %s over as the starting card.\n\r") % tString);
  ch2->sendTo(format("%s turns the %s over as the starting card.\n\r") %
              sstring(ch->getName()).cap() % tString);
  ch3->sendTo(format("%s turns the %s over as the starting card.\n\r") %
              sstring(ch->getName()).cap() % tString);
  ch4->sendTo(format("%s turns the %s over as the starting card.\n\r") %
              sstring(ch->getName()).cap() % tString);
}

void CrazyEightsGame::peek(const TBeing *ch)
{
  int    playerNum;
  char   tArg[256];
  sstring tString("");

  if (!game) {
    ch->sendTo("No one has dealt yet, perhaps you should deal the cards yourself.\n\r");
    return;
  }

  if ((playerNum = index(ch)) < 0) {
    ch->sendTo("You are not at the Crazy Eights table, you have no cards to peek at.\n\r");
    return;
  }

  tString += "You have the following cards:\n\r";
  tString += "-----------------------------\n\r";

  for (int indexCard = 0; indexCard < 32; indexCard++) {
    if (!hands[playerNum][indexCard])
      break;

    sprintf(tArg, "%2d) %-5s | %s\n\r",
            (indexCard + 1), card_names[CARD_NUM(hands[playerNum][indexCard])],
            suit(ch, hands[playerNum][indexCard]).c_str());
    tString += tArg;
  }

  ch->desc->page_string(tString);
}

int CrazyEightsGame::move_card(TBeing *ch, const char *tArg)
{
  int playerNum,
      origSlot,
      moveSlot,
      tempCard;

  if (!game) {
    ch->sendTo("The game is not in progress, you have no cards to move.\n\r");
    return FALSE;
  }

  if ((playerNum = index(ch)) < 0) {
    ch->sendTo("You are not at the Crazy Eights table, you have no cards to move around.\n\r");
    return FALSE;
  }

  if (sscanf(tArg, "%d %d", &origSlot, &moveSlot) == 2) {
    if ((origSlot < 1) || (origSlot > 32) ||
        (moveSlot < 1) || (moveSlot > 32)) {
      ch->sendTo("Crazy Eights Syntax: put <old card slot> <new card slot>\n\r");
      return FALSE;
    }

    if (origSlot == moveSlot) {
      ch->sendTo("You're funny,  You know that?\n\r");
      return FALSE;
    }

    origSlot--;
    moveSlot--;

    if (!hands[playerNum][origSlot] || !hands[playerNum][moveSlot]) {
      ch->sendTo("There is no card in that slot.\n\r");
      return FALSE;
    }

    tempCard = hands[playerNum][origSlot];

    if (origSlot < moveSlot) {
      for (int indexCard = origSlot; indexCard < moveSlot; indexCard++)
        hands[playerNum][indexCard] = hands[playerNum][indexCard + 1];
    } else {
      for (int indexCard = origSlot; indexCard > moveSlot; indexCard--)
        hands[playerNum][indexCard] = hands[playerNum][indexCard - 1];
    }

    hands[playerNum][moveSlot] = tempCard;
    ch->sendTo(format("You move the card %d to slot %d.\n\r") %
               (origSlot + 1) % (moveSlot + 1));
  } else {
    ch->sendTo("Crazy Eights Syntax: put <old card slot> <new card slot>\n\r");
    return FALSE;
  }

  return TRUE;
}

bool TBeing::checkCrazyEights(bool inGame) const
{
  if (in_room == ROOM_CRAZYEIGHTS && (inGame || (gEights.index(this) > -1)))
    return true;
  else
    return false;
}

int CrazyEightsGame::enter(const TBeing *ch)
{
  int playerNum = 0;

  if (dynamic_cast<const TMonster *>(ch)) {
    ch->sendTo("Silly monster, Crazy Eights are for mortals!\n\r");
    return FALSE;
  }

  if (inuse[0] && inuse[1] && inuse[2] && inuse[3])
    ch->sendTo("Denied, there are too many people already.\n\r");
  else if (ch->getPosition() == POSITION_SITTING)
    ch->sendTo("You are already sitting at the table, did you forget?\n\r");
  else {
    for (int indexPlayer = 0; indexPlayer < 4; indexPlayer++)
      if (!inuse[indexPlayer]) {
        playerNum = indexPlayer;
        break;
      }

    ch->sendTo("You sit down at the Crazy Eights table.\n\r");
    strcpy(names[playerNum], ch->getName());
    inuse[playerNum] = true;
    scores[playerNum] = 0;
    game = false;

    return TRUE;
  }

  return FALSE;
}

int CrazyEightsGame::exitGame(const TBeing *ch)
{
  int playerNum;
  TBeing *ch2 = NULL,
         *ch3 = NULL,
         *ch4 = NULL;

  if ((playerNum = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s left a crazy eights table %s wasn't at!") % 
          ch->getName() % ch->hssh());
    return FALSE;
  }

  ch->sendTo("You leave the crazy eights table.\n\r");
  act("$n leaves the table, killing the game.",
      FALSE, ch, NULL, NULL, TO_ROOM);

  getPlayers(ch, &ch2, &ch3, &ch4);

  if (ch2) {
    ch2->sendTo("You stand up as well.\n\r");
    ch2->setPosition(POSITION_STANDING);
  }

  if (ch3) {
    ch3->sendTo("You stand up as well.\n\r");
    ch3->setPosition(POSITION_STANDING);
  }

  if (ch4) {
    ch4->sendTo("You stand up as well.\n\r");
    ch4->setPosition(POSITION_STANDING);
  }

  *(names[0]) = '\0';
  *(names[1]) = '\0';
  *(names[2]) = '\0';
  *(names[3]) = '\0';
  game = false;
  inuse[0] = false;
  inuse[1] = false;
  inuse[2] = false;
  inuse[3] = false;
  setup_deck();

  return TRUE;
}

int CrazyEightsGame::index(const TBeing *ch) const
{
  for (int indexPlayer = 0; indexPlayer < 4; indexPlayer++)
    if (!strcmp(ch->getName(), names[indexPlayer]))
      return indexPlayer;

  return -1;
}

int CrazyEightsGame::new_deal()
{
  int     tScore = 0,
          playerIndex,
          cardIndex,
          tCard,
          playerNum = -1;
  TBeing *ch1,
         *ch2,
         *ch3,
         *ch4,
         *ch5;

  for (playerIndex = 0; playerIndex < 4; playerIndex++)
    for (cardIndex = 0; cardIndex < 32; cardIndex++) {
      if (!hands[playerIndex][cardIndex]) {
        if (cardIndex == 0)
          playerNum = playerIndex;

        break;
      }

      tCard = CARD_NUM(hands[playerIndex][cardIndex]);

      if (tCard == 8)
        tScore += 50;
      else if (tCard >= 10)
        tScore += 10;
      else
        tScore += tCard;

      hands[playerIndex][cardIndex] = 0;
    }

  if (playerNum == -1) {
    vlogf(LOG_BUG, "CrazyEights::new_deal() called when people still had cards.");
    return FALSE;
  }

  tScore += scores[playerNum];
  scores[playerNum] = tScore;
  game = false;

  ch1 = get_char_room(names[0], ROOM_CRAZYEIGHTS);
  ch2 = get_char_room(names[1], ROOM_CRAZYEIGHTS);
  ch3 = get_char_room(names[2], ROOM_CRAZYEIGHTS);
  ch4 = get_char_room(names[3], ROOM_CRAZYEIGHTS);
  ch5 = get_char_room(names[initialPlayer], ROOM_CRAZYEIGHTS);

  if (!ch1 || !ch2 || !ch3 || !ch4) {
    vlogf(LOG_BUG, "CrazyEights::new_deal called with less than 4 players!");
    return FALSE;
  }

  if (tScore >= 100) {
    ch1->sendTo(format("%s won the game.\n\r") % sstring(names[playerNum]).cap());
    ch2->sendTo(format("%s won the game.\n\r") % sstring(names[playerNum]).cap());
    ch3->sendTo(format("%s won the game.\n\r") % sstring(names[playerNum]).cap());
    ch4->sendTo(format("%s won the game.\n\r") % sstring(names[playerNum]).cap());

    scores[0] = scores[1] = scores[2] = scores[3] = 0;
  }

  initialPlayer = LEFT(ch5);
  deal(ch5);

  ch1->sendTo(format("The score is now %s.\n\r") % score());
  ch2->sendTo(format("The score is now %s.\n\r") % score());
  ch3->sendTo(format("The score is now %s.\n\r") % score());
  ch4->sendTo(format("The score is now %s.\n\r") % score());

  return TRUE;
}

void CrazyEightsGame::pass(const TBeing *ch)
{
  int     cardIndex,
          playerNum,
          curCard;
  bool    hasMatch = false,
          canDraw  = false;
  char    tString[256];
  TBeing *ch2 = NULL,
         *ch3 = NULL,
         *ch4 = NULL;

  if (!game) {
    ch->sendTo("No game in progress.\n\r");
    return;
  }

  if ((playerNum = index(ch)) < 0) {
    vlogf(LOG_BUG, "CrazyEights::pass called by player not in the game!");
    return;
  }

  if (!getPlayers(ch, &ch2, &ch3, &ch4)) {
    vlogf(LOG_BUG, "CrazyEights::pass called without a full table!");
    return;
  }

  if (playerNum != nextPlayer) {
    ch->sendTo("It isn't your turn, so you can not pass yet.\n\r");
    return;
  }

  for (cardIndex = 0; cardIndex < 32; cardIndex++) {
    if (!hands[playerNum][cardIndex])
      break;

    curCard = hands[playerNum][cardIndex];

    if ((CARD_NUM(curCard) == 8) ||
        (CARD_NUM(curCard) == CARD_NUM(starterCard)) ||
        (same_suit(curCard, starterCard))) {
      hasMatch = true;
      break;
    }
  }

  if (nextCard < 51)
    canDraw = true;

  if (hasMatch) {
    ch->sendTo("At least one of your cards is playable, you cannot pass.\n\r");
    return;
  } else if (canDraw) {
    ch->sendTo("You can still draw from the pile, you cannot pass.\n\r");
    return;
  } else {
    sprintf(tString, "%s has opted to pass %s turn.\n\r", ch->getName(), ch->hshr());
    ch->sendTo("You pass your turn.\n\r");
    ch2->sendTo(tString);
    ch3->sendTo(tString);
    ch4->sendTo(tString);
    nextPlayer = LEFT(ch);
  }
}

void CrazyEightsGame::play(const TBeing *ch, const char *tArg)
{
  TBeing *ch2,
         *ch3,
         *ch4;
  int     playerNum,
          cardPlayed,
          newSuit = 0;
  char    tString[256],
          tBuffer[256];
  bool    isPlayable = false;

  if (!game) {
    ch->sendTo("No game in progress, you have no cards to play.\n\r");
    return;
  }

  if ((playerNum = index(ch)) < 0) {
    vlogf(LOG_BUG, "CrazyEights::play called by player not in game!");
    return;
  }

  if (!getPlayers(ch, &ch2, &ch3, &ch4)) {
    ch->sendTo("There isn't 4 players, thus the game cannot be in progress yet.\n\r");
    return;
  }

  if (playerNum != nextPlayer) {
    ch->sendTo("It isn't your turn, so you can not play yet.\n\r");
    return;
  }

  for (; isspace(*tArg); tArg++);
  if (!*tArg) {
    ch->sendTo("CrazyEights Syntax: play <card#> <suit-if-card-is-eight>\n\r");
    return;
  }

  tArg = one_argument(tArg, tString, cElements(tString));
  cardPlayed = convertTo<int>(tString);

  for (; isspace(*tArg); tArg++);
  if (*tArg)
    tArg = one_argument(tArg, tString, cElements(tString));
  else
    tString[0] = '\0';

  if (!in_range(cardPlayed, 1, 33)) {
    ch->sendTo("Incorrect card number.\n\r");
    return;
  }

  cardPlayed--;

  if (!hands[playerNum][cardPlayed]) {
    ch->sendTo("You do not have that card, thus you cannot play it.\n\r");
    return;
  }

  if (CARD_NUM(hands[playerNum][cardPlayed]) == 8) {
    if (!*tString) {
      ch->sendTo("You must specify a suit when playing an eight: Water, Fire, Earth, Ether\n\r");
      return;
    }

    if (is_abbrev(tString, "water"))
      newSuit = CARD_WATER;
    else if (is_abbrev(tString, "fire"))
      newSuit = CARD_FIRE;
    else if (is_abbrev(tString, "earth"))
      newSuit = CARD_EARTH;
    else if (is_abbrev(tString, "ether"))
      newSuit = CARD_ETHER;
    else {
      ch->sendTo("Incorrect Suit.  Valid: Water, Fire, Earth, Ether\n\r");
      return;
    }
  }

  if (CARD_NUM(hands[playerNum][cardPlayed]) == 8)
    isPlayable = true;

  if (CARD_NUM(hands[playerNum][cardPlayed]) == CARD_NUM(starterCard))
    isPlayable = true;

  if (same_suit(hands[playerNum][cardPlayed], starterCard))
    isPlayable = true;

  if (!isPlayable) {
    ch->sendTo("That card is out of order.  You must play one of:\n\r");
    ch->sendTo("1) Same Suit\n\r");
    ch->sendTo("2) Same Value (2 for 2, Jack for Jack)\n\r");
    ch->sendTo("3) An Eight\n\r");
    return;
  }

  starterCard = hands[playerNum][cardPlayed];

  ch->sendTo(format("You play the %s.\n\r") % pretty_card_printout(ch, starterCard));
  sprintf(tBuffer, "%s plays the %s.\n\r",
          sstring(ch->getName()).cap().c_str(),
          pretty_card_printout(ch, starterCard).c_str());
  ch2->sendTo(tBuffer);
  ch3->sendTo(tBuffer);
  ch4->sendTo(tBuffer);

  if ((CARD_NUM(starterCard) == 8) && !(starterCard & newSuit)) {
    strcpy(tString, pretty_card_printout(ch, starterCard).c_str());

    starterCard &= ~(CARD_WATER | CARD_FIRE | CARD_EARTH | CARD_ETHER);
    starterCard |= newSuit;

    ch->sendTo(format("You change the %s into an %s.\n\r") % tString %
               pretty_card_printout(ch, starterCard));
    sprintf(tBuffer, "%s waves his hand and changes the %s into an %s.\n\r",
            sstring(ch->getName()).cap().c_str(), tString,
            pretty_card_printout(ch, starterCard).c_str());
    ch2->sendTo(tBuffer);
    ch3->sendTo(tBuffer);
    ch4->sendTo(tBuffer);
  }

  for (int cardIndex = cardPlayed; cardIndex < 32; cardIndex++) {
    if (!hands[playerNum][cardIndex])
      break;

    if (cardIndex == 31) {
      hands[playerNum][cardIndex] = 0;
      break;
    }

    hands[playerNum][cardIndex] = hands[playerNum][cardIndex + 1];
  }

  if (!hands[playerNum][0]) {
    new_deal();
    initialPlayer = playerNum;
    return;
  }

  nextPlayer = LEFT(ch);
}

int CrazyEightsGame::get(const TBeing *ch, const char *tArg)
{
  if (!*tArg || !is_abbrev(tArg, "pile"))
    return FALSE;

  int     playerNum;
  TBeing *ch2,
         *ch3,
         *ch4;
  char    tString[256];

  if (!game) {
    ch->sendTo("No game in progress, thus no pile to draw from.\n\r");
    return TRUE;
  }

  if ((playerNum = index(ch)) < 0) {
    vlogf(LOG_BUG, "CrazyEights::get called by player not in game!");
    return FALSE;
  }

  if (!getPlayers(ch, &ch2, &ch3, &ch4)) {
    ch->sendTo("There isn't 4 players in the game, thus you cannot draw.\n\r");
    return FALSE;
  }

  if (playerNum != nextPlayer) {
    ch->sendTo("It isn't your turn, so you can not draw yet.\n\r");
    return FALSE;
  }

  if (nextCard == 51) {
    ch->sendTo("The deck is gone.  You must either play or pass.\n\r");
    return FALSE;
  }

  for (int cardIndex = 0; cardIndex < 32; cardIndex++) {
    if (hands[playerNum][cardIndex])
      continue;

    hands[playerNum][cardIndex] = deck[nextCard++];
    sprintf(tString, "You draw the %s.\n\r",
            pretty_card_printout(ch, hands[playerNum][cardIndex]).c_str());
    ch->sendTo(tString);
    sprintf(tString, "%s draws from the deck.\n\r", sstring(ch->getName()).cap().c_str());
    ch2->sendTo(tString);
    ch3->sendTo(tString);
    ch4->sendTo(tString);
    return TRUE;
  }

  ch->sendTo("How odd.  You can't find a place in your hand to Put a new card.\n\r");
  vlogf(LOG_BUG, "CrazyEights::get called while player had 0 free slots and some deck left.");

  return TRUE;
}

int CrazyEightsGame::look(const TBeing *ch, const char *tArg)
{
  int  playerNum;
  bool showFull = false;

  for (; isspace(*tArg); tArg++);
  if (!*tArg)
    return FALSE;

  playerNum = index(ch);

  showFull = is_abbrev(tArg, "table");

  if (showFull || is_abbrev(tArg, "deck") || is_abbrev(tArg, "pile")) {
    if (!game)
      ch->sendTo("The game is not yet in progress, nothing to look at.\n\r");
    else {
      if (showFull) {
        ch->sendTo(format("To your left sits %s.%s\n\r") %
                   names[LEFT  (ch)]% ( LEFT  (ch) == nextPlayer ? " (current turn)" : ""));
        ch->sendTo(format("Across from you sits %s.%s\n\r") %
                   names[ACROSS(ch)] % ( ACROSS(ch) == nextPlayer ? " (current turn)" : ""));
        ch->sendTo(format("To your right sits %s.%s\n\r") %
                   names[RIGHT (ch)] % ( RIGHT (ch) == nextPlayer ? " (current turn)" : ""));

        if (nextPlayer == playerNum)
          ch->sendTo("It is currently your turn.\n\r");
      }

      if (is_abbrev(tArg, "pile") || showFull)
        ch->sendTo(format("Current card in play: %s\n\r") %
                   pretty_card_printout(ch, starterCard));

      if (is_abbrev(tArg, "deck") || showFull) {
        if (nextCard < 30)
          ch->sendTo("There are a number of cards left in the deck.\n\r");
        else if (nextCard < 40)
          ch->sendTo("The deck is starting to thin out.\n\r");
        else if (nextCard < 50)
          ch->sendTo("Not much of the deck remains.\n\r");
        else if (nextCard == 51)
          ch->sendTo("The deck is gone, all used up.\n\r");
        else
          ch->sendTo("An itty bitty little part of the deck remains.\n\r");
      }

      ch->sendTo(format("The score is currently %s.\n\r") % score());
    }
  } else
    return FALSE;

  return TRUE;
}

CrazyEightsGame::CrazyEightsGame() :
  CardGame(),
  iplay(0),
  starterCard(0),
  nextCard(0),
  nextPlayer(0),
  game(false)
{
  for (int Runner = 0; Runner < 4; Runner++) {
    *(names[Runner]) = '\0';
    scores[Runner] = 0;
    inuse[Runner] = false;

    for (int cardMark = 0; cardMark < 32; cardMark++)
      hands[Runner][cardMark] = 0;
  }
}
