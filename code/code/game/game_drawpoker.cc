/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "drawpoker.cc"
  All functions and routines related to the draw poker card game.

  Created 4/24/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

#include "being.h"
#include "monster.h"
#include "games.h"
#include "extern.h"
#include "handler.h"
#include "game_drawpoker.h"

DrawPokerGame gDrawPoker;

const int ROOM_DRAWPOKER    = 2;

const int DRAWPOKER_MINANTE = 1;
const int DRAWPOKER_MAXANTE = 100;
const int DRAWPOKER_MINBID  = 10;
const int DRAWPOKER_MAXBID  = 50000;

int DrawPokerGame::LEFT(const TBeing *ch) const
{
  return ((index(ch) == 6) ? 0 : index(ch) + 1);
}

int DrawPokerGame::LEFT(int playerNum) const
{
  TBeing *ch;

  for (int playerIndex = (playerNum + 1); playerIndex < 6; playerIndex++)
    if ((ch = get_char_room(names[playerNum], ROOM_DRAWPOKER)))
      return playerIndex;

  for (int playerIndex = 0; playerIndex < playerNum; playerIndex++)
    if ((ch = get_char_room(names[playerNum], ROOM_DRAWPOKER)))
      return playerIndex;

  return -1;
}

int DrawPokerGame::getNextPlayer(const TBeing *tChar)
{
  TBeing *ch[5];
  int    playerNum;

  getPlayers(tChar, &ch[0], &ch[1], &ch[2], &ch[3], &ch[4]);

  if ((playerNum = index(tChar)) < 0) {
    vlogf(LOG_BUG, "DrawPoker::getNextPlayer called by player not at poker table.");
    return 0;
  }

  for (int playerIndex = 0; playerIndex < 5; playerIndex++)
    if (ch[playerIndex] && hands[playerIndex][0] && inuse[playerIndex])
      return index(ch[playerIndex]);

  vlogf(LOG_BUG, "DrawPoker::getNextPlayer called when apparently no next player!");

  return index(tChar);
}

int DrawPokerGame::count(int playerNum) const
{
  if (playerNum < 0 || playerNum > 5)
    return 0;

  return 5;
}

const sstring DrawPokerGame::score() const
{
  TBeing *ch[6];
  char    tString[256];
  sstring  tBuffer("\n\r");

  ch[0] = get_char_room(names[0], ROOM_DRAWPOKER);
  ch[1] = get_char_room(names[1], ROOM_DRAWPOKER);
  ch[2] = get_char_room(names[2], ROOM_DRAWPOKER);
  ch[3] = get_char_room(names[3], ROOM_DRAWPOKER);
  ch[4] = get_char_room(names[4], ROOM_DRAWPOKER);
  ch[5] = get_char_room(names[5], ROOM_DRAWPOKER);

  if (!ch[0] || !ch[1]) {
    vlogf(LOG_BUG, "DrawPoker::score() called without two poker players!");
    return "";
  }

  for (int playerIndex = 0; playerIndex < 6; playerIndex++) {
    if (!ch[playerIndex] ||
        !inuse[playerIndex] ||
        !hands[playerIndex])
      continue;

    sprintf(tString, "   %s[%d]", ch[playerIndex]->getName(), scores[playerIndex]);
    tBuffer += tString;
  }

  tBuffer += "\n\r";

  return tBuffer;
}

const sstring DrawPokerGame::bets() const
{
  TBeing *ch[6];
  char    tString[256];
  sstring  tBuffer("\n\r");

  ch[0] = get_char_room(names[0], ROOM_DRAWPOKER);
  ch[1] = get_char_room(names[1], ROOM_DRAWPOKER);
  ch[2] = get_char_room(names[2], ROOM_DRAWPOKER);
  ch[3] = get_char_room(names[3], ROOM_DRAWPOKER);
  ch[4] = get_char_room(names[4], ROOM_DRAWPOKER);
  ch[5] = get_char_room(names[5], ROOM_DRAWPOKER);

  if (!ch[0] || !ch[1]) {
    vlogf(LOG_BUG, "DrawPoker::bets() called without two poker players!");
    return "";
  }

  for (int playerIndex = 0; playerIndex < 6; playerIndex++) {
    if (!ch[playerIndex] && inuse[playerIndex])
      continue;

    sprintf(tString, "   %s[%d]", ch[playerIndex]->getName(), playerante[playerIndex]);
    tBuffer += tString;
  }

  tBuffer += "\n\r";

  return tBuffer;
}

bool DrawPokerGame::getPlayers(const TBeing *ch, TBeing **ch2, TBeing **ch3,
                               TBeing **ch4, TBeing **ch5, TBeing **ch6) const
{
  int      playerNum = 0,
           playerCount = 0;
  TBeing **tChar[5];

  if ((playerNum = index(ch)) < 0)
    return false;

  tChar[0] = &(*ch2 = NULL);
  tChar[1] = &(*ch3 = NULL);
  tChar[2] = &(*ch4 = NULL);
  tChar[3] = &(*ch5 = NULL);
  tChar[4] = &(*ch6 = NULL);

  for (int playerIndex = (playerNum + 1); playerIndex < 6; playerIndex++)
    if ((*tChar[playerCount] = get_char_room(names[playerIndex], ROOM_DRAWPOKER)))
      playerCount++;

  for (int playerIndex = 0; playerIndex < playerNum; playerIndex++)
    if ((*tChar[playerCount] = get_char_room(names[playerIndex], ROOM_DRAWPOKER)))
      playerCount++;

  if (!*tChar[0])
    return false;

  return true;
}

void DrawPokerGame::deal(TBeing *ch, const char *tArg)
{
  TBeing *tChar[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
  int     pointCard = 0,
          dealerNum = 0,
          anteCost  = 0,
          averageLevel;
  char    tString[256],
          tBuffer[256];
  bool    anteSet = false;

  if (game) {
    ch->sendTo("Redeal while the game is in progress?  Never!!\n\r");
    return;
  }

  if ((dealerNum = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s got into DrawPoker::deal without being at the poker table!") % 
          ch->getName());
    return;
  }

  tChar[0] = ch;

  if (!getPlayers(ch, &tChar[1], &tChar[2], &tChar[3], &tChar[4], &tChar[5])) {
    ch->sendTo("You must have at least 2 players, and there isn't even 2.\n\r");
    return;
  }

  for (; isspace(*tArg); tArg++);

  // ante_min = max(1, levels / 2) =    1,    25
  // ante_max = levels * 2         =    2,   100
  //  bid_min = levels             =    1,    50
  //  bid_max = levels * 1000      = 1000, 50000

  averageLevel = max(1, averagePlayerLevel());
  anteCosts[0] = max(DRAWPOKER_MINANTE, (int)(averageLevel / 2));
  anteCosts[1] = min(DRAWPOKER_MAXANTE, (averageLevel * 2));
  bidCosts[0]  = max(DRAWPOKER_MINBID , averageLevel);
  bidCosts[1]  = min(DRAWPOKER_MAXBID , (averageLevel * 10000));
  anteCost     = anteCosts[0];

  if (*tArg) {
    vlogf(LOG_LAPSOS, format("DrawPoker::deal [%s]") %  tArg);

    do {
      half_chop(tArg, tString, tBuffer);
      tArg = tBuffer;

      if (is_number(tString)) {
        if (!anteSet) {
          anteCost = convertTo<int>(tString);

          if (!in_range(anteCost, anteCosts[0], anteCosts[1])) {
            ch->sendTo(format("No luck slick.  Ante must be between: %d-%d\n\r") %
                       anteCosts[0] % anteCosts[1]);
            return;
          }

          anteSet = true;
        } else {
          int oldMax = bidCosts[1];
          bidCosts[1] = convertTo<int>(tString);

          if (!in_range(bidCosts[1], bidCosts[0], oldMax)) {
            ch->sendTo(format("No luck slick.  Bid max must be between: %d-%d\n\r") %
                       bidCosts[0] % oldMax);
            bidCosts[1] = oldMax;
            return;
          }
        }
      } else if (is_abbrev(tString, "silentbets"))
        csilentBets = silentBets = true;
      else if (is_abbrev(tString, "newbie"))
        usenewbie = true;
      else {
        ch->sendTo("I have no idea what you want.\n\r");
        return;
      }
    } while (*tArg);
  }

  lastAnte = anteCost;

  shuffle();

  ch->sendTo("You shuffle the cards, and deal them.\n\r");
  act("$n shuffles the cards and deals them.",
      FALSE, ch, NULL, NULL, TO_ROOM);

  ch->sendTo("You are dealt:\n\r");

  memset((char *)hands, 0, sizeof(hands));

  silentBets = false;

  for (int playerIndex = 0; playerIndex < 6; playerIndex++)
    if (tChar[playerIndex] && inuse[playerIndex] &&
        tChar[playerIndex]->GetMaxLevel() > MAX_MORT)
      silentBets = true;

  for (int cardIndex = 0; cardIndex < 5; cardIndex++)
    for(int playerIndex = 0; playerIndex < 6; playerIndex++)
      if (tChar[playerIndex] && inuse[playerIndex])
        if ((tChar[playerIndex]->getMoney() < anteCost) && !silentBets) {
          ch->sendTo(format("You can not cover the ante of %d talens, your forced to sit out.\n\r") %
                     anteCost);
          act("$n is forced to sit out this round due to low talens.",
              FALSE, ch, NULL, NULL, TO_ROOM);
        } else {
          hands[playerIndex][cardIndex] = deck[pointCard++];

          if (!silentBets)
            playerante[playerIndex] += anteCost;
        }

  game = true;

  for (int playerIndex = 0; playerIndex < 6; playerIndex++)
    if (tChar[playerIndex] && inuse[playerIndex] && hands[playerIndex][0]) {
      tChar[playerIndex]->sendTo("You are dealt:\n\r");
      tChar[playerIndex]->doPeek();
      discarded[playerIndex] = false;
    }

  iplay         = (silentBets ? 1 : 0);
  isbidding     = false;
  nextCard      = pointCard;
  nextPlayer    = getNextPlayer(ch);
  initialPlayer = index(ch);

  if (usenewbie) {
    TBeing *cNewC;

    if ((cNewC = get_char_room(names[nextPlayer], ROOM_DRAWPOKER)))
      cNewC->sendTo("It is your turn now.\n\r");
  }

  if (silentBets && ch->roomp)
    sendrpf(ch->roomp, "Betting is currently off.\n\r");
}

int DrawPokerGame::averagePlayerLevel() const
{
  int     wholeLevel  = 0,
          playerCount = 0;
  TBeing *ch;

  for (int playerIndex = 0; playerIndex < 6; playerIndex++) {
    if (!hands[playerIndex][0] ||
        !inuse[playerIndex] ||
        !(ch = get_char_room(names[playerIndex], ROOM_DRAWPOKER)))
      continue;

    playerCount++;
    wholeLevel += ch->GetMaxLevel();
  }

  if (!playerCount)
    return 0;

  return ((int)((wholeLevel / playerCount)));
}

void DrawPokerGame::peek(const TBeing *ch)
{
  int    playerNum;
  char   tArg[256];
  sstring tString("");

  if (!game) {
    ch->sendTo("No one has dealt yet, perhaps you should deal the cards yourself.\n\r");
    return;
  }

  if ((playerNum = index(ch)) < 0) {
    ch->sendTo("You are not at the Poker table, you have no cards to peek at.\n\r");
    return;
  }

  if (!hands[playerNum][0]) {
    ch->sendTo("You have no cards, how can you peek at them?\n\r");
    return;
  }

  tString += "You have the following cards:\n\r";
  tString += "-----------------------------\n\r";

  for (int indexCard = 0; indexCard < 5; indexCard++) {
    if (!hands[playerNum][indexCard])
      break;

    sprintf(tArg, "%2d) %-5s | %s\n\r",
            (indexCard + 1), card_names[CARD_NUM(hands[playerNum][indexCard])],
            suit(ch, hands[playerNum][indexCard]).c_str());
    tString += tArg;
  }

  ch->sendTo(tString);
}

int DrawPokerGame::move_card(TBeing *ch, const char *tArg)
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
    ch->sendTo("You are not at the Poker table, you have no cards to move around.\n\r");
    return FALSE;
  }

  if (!hands[playerNum][0]) {
    ch->sendTo("Your not in this round, wait until your actually In the game first.\n\r");
    return FALSE;
  }

  if (sscanf(tArg, "%d %d", &origSlot, &moveSlot) == 2) {
    if ((origSlot < 1) || (origSlot > 32) ||
        (moveSlot < 1) || (moveSlot > 32)) {
      ch->sendTo("Poker Syntax: put <old card slot> <new card slot>\n\r");
      return FALSE;
    }

    if (origSlot == moveSlot) {
      ch->sendTo("You're funny.  You know that?\n\r");
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
    ch->sendTo("Poker Syntax: put <old card slot> <new card slot>\n\r");
    return FALSE;
  }

  return TRUE;
}

bool TBeing::checkDrawPoker(bool inGame) const
{
  if (in_room == ROOM_DRAWPOKER && (inGame || (gDrawPoker.index(this) > -1)))
    return true;
  else
    return false;
}

int DrawPokerGame::enter(const TBeing *ch)
{
  int playerNum = 0;

  if (dynamic_cast<const TMonster *>(ch)) {
    ch->sendTo("Silly monster, Poker is for mortals!\n\r");
    return FALSE;
  }

  if (inuse[0] && inuse[1] && inuse[2] && inuse[3] && inuse[4] && inuse[5])
    ch->sendTo("Denied, there are too many people already.\n\r");
  else if (ch->getPosition() == POSITION_SITTING)
    ch->sendTo("You are already sitting at the table, did you forget?\n\r");
  else {
    for (int indexPlayer = 0; indexPlayer < 6; indexPlayer++)
      if (!inuse[indexPlayer]) {
        playerNum = indexPlayer;
        break;
      }

    ch->sendTo("You sit down at the Poker table.\n\r");
    strcpy(names[playerNum], ch->getName());
    inuse[playerNum]      = true;
    scores[playerNum]     = 0;
    playerante[playerNum] = 0;
    totalPlayers++;
    discarded[playerNum]  = false;

    if (playerNum == 0)
      game = false;

    for (int cardIndex = 0; cardIndex < 5; cardIndex++)
      hands[playerNum][cardIndex] = 0;

    return TRUE;
  }

  return FALSE;
}

int DrawPokerGame::exitGame(const TBeing *ch)
{
  int playerNum;
  TBeing *ch2 = NULL,
         *ch3 = NULL,
         *ch4 = NULL,
         *ch5 = NULL,
         *ch6 = NULL;

  if ((playerNum = index(ch)) < 0) {
    vlogf(LOG_BUG, format("%s left a poker table %s wasn't at!") % 
          ch->getName() % ch->hssh());
    return FALSE;
  }

  ch->sendTo("You leave the poker table.\n\r");
  act("$n leaves the table.",
      FALSE, ch, NULL, NULL, TO_ROOM);

  getPlayers(ch, &ch2, &ch3, &ch4, &ch5, &ch6);

  if (!silentBets)
    settleUp(ch, true);

  playerante[playerNum] = 0;
  totalPlayers--;

  for (int playerIndex = playerNum; playerIndex < 5; playerIndex++) {
    strcpy(names[playerIndex], names[playerIndex + 1]);
    names[playerIndex + 1][0] = '\0';
    inuse[playerIndex]        = inuse[playerIndex + 1];
    inuse[playerIndex + 1]    = false;

    for (int cardIndex = 0; cardIndex < 5; cardIndex++) {
      hands[playerIndex][cardIndex]     = hands[playerIndex + 1][cardIndex];
      hands[playerIndex + 1][cardIndex] = 0;
    }
  }

  return TRUE;
}

int DrawPokerGame::index(const TBeing *ch) const
{
  for (int indexPlayer = 0; indexPlayer < 6; indexPlayer++)
    if (!strcmp(ch->getName(), names[indexPlayer]))
      return indexPlayer;

  return -1;
}

const char DrawPokerHands[][21] =
{
  "Unknown",
  "Royal Straight Flush",
  "Straight Flush",
  "Four of a Kind",
  "Full House",
  "Flush",
  "Straight",
  "Three of a Kind",
  "Two Pairs",
  "One Pair",
  "High Card",
  "Error Hand"
};

int DrawPokerGame::new_deal()
{
  int     tScore        = 0,
          playerNum     = -1,
          winnerList[6] = {-1, -1, -1, -1, -1, -1},
          totalWinners  = 1,
          whType        = 0;
  TBeing *tChar[6];
  char    tString[256],
          tColor        = 'n';

  playerNum = findWinner(&winnerList[0], &winnerList[1], &winnerList[2],
                         &winnerList[3], &winnerList[4], &winnerList[5],
                         &whType);

  tChar[0] = get_char_room(names[nextPlayer], ROOM_DRAWPOKER);

  if ((playerNum & (1 << 31))) {
    sendrpf(tChar[0]->roomp, "A tie occured.\n\r");
    totalWinners = playerNum &= ~(1 << 31);
    playerNum = winnerList[0];
  } else {
    sendrpf(tChar[0]->roomp, "%s won the hand.\n\r", names[playerNum]);

    if (get_suit(hands[playerNum][0]) == CARD_WATER)
      tColor = 'b';
    else if (get_suit(hands[playerNum][0]) == CARD_FIRE)
      tColor = 'r';
    else if (get_suit(hands[playerNum][0]) == CARD_EARTH)
      tColor = 'o';
    else
      tColor = 'p';

    int highCard = getHighCard(playerNum, 0);

    if (CARD_NUM(highCard) == 14)
      highCard = (1 | get_suit(highCard));

    sendrpf(COLOR_OBJECTS, tChar[0]->roomp,  "Winning Hand: <%c>%s%s<z>%s\n\r",
            tColor, DrawPokerHands[whType],
            (whType == 10 ? " " : ""),
            (whType == 10 ? pretty_card_printout(tChar[0], highCard).c_str() : ""));
  }

  if (!silentBets) {
    for (int betIndex = 0; betIndex < 6; betIndex++)
      tScore += playerante[betIndex];

    if (tScore)
      for (int playerIndex = 0; playerIndex < totalWinners; playerIndex++) {
        playerNum              = winnerList[playerIndex];
        playerante[playerNum] -= (tScore / totalWinners);
        scores[playerNum]      = (tScore / totalWinners);
      }
  }

  game              = false;
  isbidding         = false;

  if (!getPlayers(tChar[0], &tChar[1], &tChar[2], &tChar[3], &tChar[4], &tChar[5])) {
    vlogf(LOG_BUG, "DrawPoker::new_deal called with less than 2 players!");
    return FALSE;
  }

  tChar[6] = get_char_room(names[initialPlayer], ROOM_DRAWPOKER);

  initialPlayer = LEFT(initialPlayer);
  sprintf(tString, "%d%s%d%s",
          lastAnte   , (csilentBets ? " silentbets " : " "),
          bidCosts[1], (usenewbie ? " newbie" : ""));
  deal(tChar[6], tString);
  strcpy(tString, score().c_str());

  for (int playerIndex = 0; playerIndex < 6; playerIndex++) {
    if (tChar[playerIndex] && !silentBets) {
      tChar[playerIndex]->sendTo(format("The score is now %s.\n\r") % tString);

      settleUp(tChar[playerIndex], false);
    }

    playerante[playerIndex] = 0;
  }

  return TRUE;
}

void DrawPokerGame::pass(const TBeing *ch)
{
  int     playerNum;
  TBeing *tChar[6];

  if ((playerNum = index(ch)) < 0) {
    vlogf(LOG_BUG, "DrawPoker::pass called by player not in game.");
    return;
  }

  if (!game) {
    ch->sendTo("No game in progress thus no bidding or discarding to pass.\n\r");
    return;
  }

  tChar[0] = get_char_room(names[playerNum], ROOM_DRAWPOKER);

  if (!getPlayers(ch, &tChar[1], &tChar[2], &tChar[3], &tChar[4], &tChar[5])) {
    ch->sendTo("You must have at least 2 players to play poker.\n\r");
    return;
  }

  if (playerNum != nextPlayer) {
    ch->sendTo("It isn't your turn, you can't pass yet.\n\r");
    return;
  }

  if (isbidding && iplay != 1) {
    ch->sendTo("I'm afraid that once bidding has started you cannot pass.\n\r");
    return;
  }

  ch->sendTo(format("You skip %s this time.\n\r") %
             (iplay == 1 ? "discarding" : "betting"));

  if (iplay == 1)
    act("$n skips $s turn at discarding this time.",
        FALSE, ch, NULL, NULL, TO_ROOM);
  else
    act("$n skips $s turn at bidding this time.",
        FALSE, ch, NULL, NULL, TO_ROOM);

  nextPlayer = getNextPlayer(ch);

  if (iplay == 1)
    discarded[playerNum] = true;

  if (usenewbie) {
    TBeing *cNewC;

    if ((cNewC = get_char_room(names[nextPlayer], ROOM_DRAWPOKER)))
      cNewC->sendTo("It is your turn now.\n\r");
  }
}

void DrawPokerGame::bet(const TBeing *ch, const char *tArg)
{
  int     playerNum,
          newBet;
  TBeing *tChar[6];
  char    tBuffer[256];

  if ((playerNum = index(ch)) < 0) {
    vlogf(LOG_BUG, "DrawPoker::bet called by player not in game.\n\r");
    return;
  }

  if (!game) {
    ch->sendTo("No game thus no bidding.\n\r");
    return;
  }

  tChar[0] = get_char_room(names[playerNum], ROOM_DRAWPOKER);

  if (!getPlayers(ch, &tChar[1], &tChar[2], &tChar[3], &tChar[4], &tChar[5])) {
    ch->sendTo("You need at least 2 players to play Poker.\n\r");
    return;
  }

  if (playerNum != nextPlayer) {
    ch->sendTo("Just keep your poker face on, you'll get your chance.\n\r");
    return;
  }

  if (silentBets) {
    ch->sendTo("The game is running with no bets, thus no betting.\n\r");
    return;
  }

  if (iplay == 1) {
    ch->sendTo("There isn't betting at this moment.\n\r");
    return;
  }

  if (!hands[playerNum][0]) {
    ch->sendTo("You don't have any cards, how can you bet?\n\r");
    return;
  }

  for (; isspace(*tArg); tArg++);
  strcpy(tBuffer, tArg);
  if (!*tArg || !is_number(tBuffer)) {
    ch->sendTo(format("Poker Syntax: bet <amount[Limit:%d]>\n\r") % bidCosts[1]);
    return;
  }

  newBet = convertTo<int>(tArg);

  if (ch->getMoney() < newBet) {
    ch->sendTo("You don't have that much to bet, so bugger off.\n\r");
    return;
  }

  if (!in_range(newBet, bidCosts[0], bidCosts[1])) {
    ch->sendTo(format("Funny, Real Funny.  Bugger off.  Bid is limited to: %d-%d\n\r") %
               bidCosts[0] % bidCosts[1]);
    return;
  }

  playerante[playerNum] += newBet;
  sprintf(tBuffer, "$n bets %d\n\r", newBet);
  act(tBuffer, FALSE, ch, NULL, NULL, TO_ROOM);

  if (!isBettingClosed()) {
    nextPlayer = getNextPlayer(ch);
    isbidding = true;
  } else {
    ch->sendTo("Betting is closed.\n\r");
    act("Betting is closed.", FALSE, ch, NULL, NULL, TO_ROOM);
    iplay++;
  }

  ch->sendTo(format("You bet %d.\n\r%s") % newBet %
             (nextPlayer == playerNum ? "Betting is closed." : ""));

  if (nextPlayer == playerNum && iplay == 2)
    if (!new_deal()) {
      game = false;
      return;
    }

  if (usenewbie) {
    TBeing *cNewC;

    if ((cNewC = get_char_room(names[nextPlayer], ROOM_DRAWPOKER)))
      cNewC->sendTo("It is your turn now.\n\r");
  }
}

bool DrawPokerGame::isBettingClosed() const
{
  int betAverage = playerante[0];

  for (int playerIndex = 0; playerIndex < totalPlayers; playerIndex++)
    if (betAverage != playerante[playerIndex])
      return false;

  return true;
}

void DrawPokerGame::stop(const TBeing *ch)
{
  int     playerNum;
  TBeing *tChar[6];
  char    tString[256];

  if ((playerNum = index(ch)) < 0) {
    vlogf(LOG_BUG, "DrawPoker::stop called by player not in game.");
    return;
  }

  if (!game) {
    ch->sendTo("No game thus no exiting from the game.\n\r");
    return;
  }

  tChar[0] = get_char_room(names[playerNum], ROOM_DRAWPOKER);

  if (!getPlayers(ch, &tChar[1], &tChar[2], &tChar[3], &tChar[4], &tChar[5])) {
    ch->sendTo("You have to have at least two to have a game.\n\r");
    return;
  }

  strcpy(tString, sstring(ch->getName()).cap().c_str());
  ch->sendTo("You put down, and leave this hand to the others.\n\r");
  act("$n puts down, deciding not to play this hand.",
      FALSE, ch, NULL, NULL, TO_ROOM);

  if (!silentBets)
    settleUp(ch, false);

  for (int cardIndex = 0; cardIndex < 5; cardIndex++)
    hands[playerNum][cardIndex] = 0;

  int TotalCount = 0;

  for (int playerIndex = 0; playerIndex < totalPlayers; playerIndex++)
    if (hands[playerIndex][0])
      TotalCount++;

  if (TotalCount == 1)
    new_deal();

  if (nextPlayer == playerNum)
    nextPlayer = getNextPlayer(ch);

  if (usenewbie) {
    TBeing *cNewC;

    if ((cNewC = get_char_room(names[nextPlayer], ROOM_DRAWPOKER)))
      cNewC->sendTo("It is your turn now.\n\r");
  }
}

int DrawPokerGame::look(const TBeing *ch, const char *tArg)
{
  int playerNum;

  for (; isspace(*tArg); tArg++);
  if (!*tArg)
    return FALSE;

  playerNum = index(ch);

  if (is_abbrev(tArg, "table")) {
    if (!game)
      ch->sendTo("The game is not yet in progress, nothing to look at.\n\r");
    else {
      if (silentBets) {
        ch->sendTo("Since there is no bidding there isn't any scores to view.\n\r");
      } else {
        ch->sendTo(format("The score is currently:\n\r%s\n\r") % score());
        ch->sendTo(format("The bet is currently:\n\r%s\n\r") % bets());
      }
    }
  } else
    return FALSE;

  return TRUE;
}

void DrawPokerGame::discard(const TBeing *ch, const char *tArg)
{
  int     discardCards[5] = {-1, -1, -1, -1, -1},
          discardIndex,
          playerNum;
  TBeing *ch2,
         *ch3,
         *ch4,
         *ch5,
         *ch6;
  char    tString[256];

  if ((playerNum = index(ch)) < 0) {
    vlogf(LOG_BUG, "DrawPoker::discard called by player not in game.");
    return;
  }

  if (!getPlayers(ch, &ch2, &ch3, &ch4, &ch5, &ch6)) {
    ch->sendTo("You have to have at least 2 players to have a game.\n\r");
    return;
  }

  if (!game) {
    ch->sendTo("No game thus no discarding of cards.\n\r");
    return;
  }

  if (iplay != 1) {
    ch->sendTo("This isn't the round in which you discard cards.\n\r");
    return;
  }

  if (discarded[playerNum]) {
    ch->sendTo("You think you're funny, don't you?\n\r");
    return;
  }

  if (playerNum != nextPlayer) {
    ch->sendTo("Hold your horses, you will get your chance.\n\r");
    return;
  }

  for (; isspace(*tArg); tArg++);
  if (!*tArg) {
    ch->sendTo("Poker Syntax: discard <card> <card> <card> <card> <card>\n\r");
    return;
  }

  discardIndex = sscanf(tArg, "%d %d %d %d %d", &discardCards[0],
                        &discardCards[1], &discardCards[2],
                        &discardCards[3], &discardCards[4]);

  if (discardIndex <= 0) {
    ch->sendTo("Poker Syntax: discard <card> <card> <card> <card> <card>\n\r");
    return;
  }

  for (int cardIndexA = 0; cardIndexA < (discardIndex - 1); cardIndexA++)
    for (int cardIndexB = (cardIndexA + 1); cardIndexB < discardIndex; cardIndexB++) {
      if (discardCards[cardIndexA] == -1 ||
          discardCards[cardIndexB] == -1 ||
          discardCards[cardIndexA] != discardCards[cardIndexB])
        continue;

      if (!in_range(discardCards[cardIndexA], 1, 5) ||
          !in_range(discardCards[cardIndexB], 1, 5)) {
        ch->sendTo("Invalid card.  Range is 1 to 5.\n\r");
        return;
      }

      ch->sendTo("Only specify cards once.\n\r");
      return;
    }

  for (int cardIndex = 0; cardIndex < discardIndex; cardIndex++)
    hands[playerNum][discardCards[cardIndex] - 1] = deck[nextCard++];

  discarded[playerNum] = true;

  ch->sendTo(format("You discard %d cards.\n\r") % discardIndex);
  sprintf(tString, "$n discards %d cards.", discardIndex);
  act(tString, FALSE, ch, NULL, NULL, TO_ROOM);

  ch->doPeek();

  nextPlayer = getNextPlayer(ch);

  for (int playerIndex = 0; playerIndex < totalPlayers; playerIndex++)
    if (!discarded[playerIndex])
      return;

  if (silentBets) {
    if (!new_deal())
      game = false;

    return;
  }

  iplay++;
  ch->sendTo("Bidding begins again.\n\r");
  act("Bidding begins again.",
      FALSE, ch, NULL, NULL, TO_ROOM);

  if (usenewbie) {
    TBeing *cNewC;

    if ((cNewC = get_char_room(names[nextPlayer], ROOM_DRAWPOKER)))
      cNewC->sendTo("It is your turn now.\n\r");
  }
}

int DrawPokerGame::findWinner(int *PlyWin1, int *PlyWin2, int *PlyWin3,
                              int *PlyWin4, int *PlyWin5, int *PlyWin6,
                              int *winningHandType)
{
  int handScores[6] = {11, 11, 11, 11, 11, 11},
      handHighs[6][2] = {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}},
      handWinner;
  int *PlyWinLs[6];

  PlyWinLs[0] = &(*PlyWin1 = -1);
  PlyWinLs[1] = &(*PlyWin2 = -1);
  PlyWinLs[2] = &(*PlyWin3 = -1);
  PlyWinLs[3] = &(*PlyWin4 = -1);
  PlyWinLs[4] = &(*PlyWin5 = -1);
  PlyWinLs[5] = &(*PlyWin6 = -1);

  for (int playerIndex = 0; playerIndex < 6; playerIndex++) {
    if (!inuse[playerIndex] || !hands[playerIndex][0])
      continue;

    bool hasStraight      = isStraight(playerIndex),
         hasFlush         = isFlush(playerIndex),
         hasTwoPair       = isPair(playerIndex, 2, &handHighs[playerIndex][0],
                                   false, &handHighs[playerIndex][1]),
         hasSecondTwoPair = isPair(playerIndex, 2, &handHighs[playerIndex][0],
                                   true , &handHighs[playerIndex][1]),
         hasThreePair     = isPair(playerIndex, 3, &handHighs[playerIndex][0],
                                   false, &handHighs[playerIndex][1]),
         hasFourPair      = isPair(playerIndex, 4, &handHighs[playerIndex][0],
                                   false, &handHighs[playerIndex][1]);

    if (handHighs[playerIndex][0] == -1)
      handHighs[playerIndex][0] = getHighCard(playerIndex, 0);

    vlogf(LOG_LAPSOS, format("DrawPoker::S:%d F:%d 2:%d 2x2:%d 3:%d 4:%d H1:%d H2:%d Ply:%d") % 
          hasStraight % hasFlush % hasTwoPair % hasSecondTwoPair % hasThreePair % hasFourPair %
          handHighs[playerIndex][0] % handHighs[playerIndex][1] % playerIndex);

    if (hasStraight && hasFlush) {
      if (CARD_NUM(handHighs[playerIndex][0]) == 14)
        handScores[playerIndex] = 1;
      else
        handScores[playerIndex] = 2;
    } else if (hasFourPair)
      handScores[playerIndex] = 3;
    else if (hasThreePair && hasTwoPair)
      handScores[playerIndex] = 4;
    else if (hasFlush)
      handScores[playerIndex] = 5;
    else if (hasStraight)
      handScores[playerIndex] = 6;
    else if (hasThreePair)
      handScores[playerIndex] = 7;
    else if (hasSecondTwoPair)
      handScores[playerIndex] = 8;
    else if (hasTwoPair)
      handScores[playerIndex] = 9;
    else
      handScores[playerIndex] = 10;
  }

  handWinner = (min(handScores[0], min(handScores[1], min(handScores[2],
                min(handScores[3], min(handScores[4], handScores[5]))))));

  int winnerNum   = 11,
      winnerPly   = 0,
      winnerCount = 0;

  for (int playerIndex = 0; playerIndex < 6; playerIndex++) {
    if (handScores[playerIndex] > winnerNum ||
        !inuse[playerIndex] || !hands[playerIndex][0])
      continue;

    if (winnerNum == handScores[playerIndex])
      winnerCount++;
    else {
      winnerNum = handScores[playerIndex];
      winnerCount = 1;
    }

    winnerPly = playerIndex;
  }

  if (winnerCount == 0)
    vlogf(LOG_BUG, "Something went very wrong in DrawPoker::findWinner.  No Winner Found.");
  else if (winnerCount > 1) {
    for (int playerIndex = 0; playerIndex < 6; playerIndex++) {
      if (handScores[playerIndex] > winnerNum ||
          playerIndex == winnerPly ||
          !inuse[playerIndex] || !hands[playerIndex][0])
        continue;

      if (handHighs[playerIndex][0] >= handHighs[winnerPly][0]) {
        if (handHighs[playerIndex][0] == handHighs[winnerPly][0]) {
          if (handHighs[playerIndex][1] == handHighs[winnerPly][1]) {
            for (int Runner = 1; Runner < 5; Runner++)
              if (getHighCard(playerIndex, Runner) > getHighCard(winnerPly, Runner))
                winnerPly = playerIndex;

            if (winnerPly != playerIndex && playerIndex != *PlyWinLs[0]) {
              for (int newPlayerIndex = 1; newPlayerIndex < 6; newPlayerIndex++) {
                if (*PlyWinLs[newPlayerIndex] > -1)
                  continue;

                *PlyWinLs[newPlayerIndex] = playerIndex;
                break;
              }
            }
          } else if (handHighs[playerIndex][1] > handHighs[winnerPly][1]) {
            *PlyWinLs[0] = winnerPly = playerIndex;

            for (int newPlayerIndex = 1; newPlayerIndex < 6; newPlayerIndex++)
              *PlyWinLs[newPlayerIndex] = -1;
          }
        } else {
          *PlyWinLs[0] = winnerPly = playerIndex;

          for (int newPlayerIndex = 1; newPlayerIndex < 6; newPlayerIndex++)
            *PlyWinLs[newPlayerIndex] = -1;
        }
      }
    }
  }

  *winningHandType = handScores[winnerPly];

  for (int playerIndex = 5; playerIndex > 0; playerIndex--)
    if (*PlyWinLs[playerIndex] != -1 &&
        inuse[playerIndex] &&
        hands[playerIndex][0])
      return ((1 << 31) | (playerIndex + 1));

  return winnerPly;
}

bool DrawPokerGame::isStraight(int playerNum) const
{
  int forCards[6];

  for (int Runner = 0; Runner < 5; Runner++)
    forCards[Runner] = CARD_NUM(hands[playerNum][Runner]);

  forCards[5] = 0;
  qsort(forCards, 6, 4, cardnumComparDescend);
  forCards[5] = (forCards[4] - 1);

  for (int cardIndex = 0; cardIndex < 5; cardIndex++)
    if (forCards[cardIndex] != (forCards[cardIndex + 1] + 1)) {
      for (int Runner = 0; Runner < 5; Runner++)
        if (forCards[Runner] == 1)
          forCards[Runner] = 14;

      forCards[5] = 0;
      qsort(forCards, 6, 4, cardnumComparDescend);
      forCards[5] = (forCards[4] - 1);

      for (int ocardIndex = 0; ocardIndex < 5; ocardIndex++)
        if (forCards[ocardIndex] != (forCards[ocardIndex + 1] + 1))
          return false;
    }

  return true;
}

bool DrawPokerGame::isFlush(int playerNum) const
{
  int forCards[6];

  for (int Runner = 0; Runner < 5; Runner++)
    forCards[Runner] = get_suit(hands[playerNum][Runner]);

  forCards[5] = forCards[4];

  for (int Runner = 0; Runner < 5; Runner++)
    if (forCards[Runner] != forCards[Runner + 1])
      return false;

  return true;
}

bool DrawPokerGame::isPair(int playerNum, int wantThis, int *highCard,
                           bool doublePair, int *secondHighCard) const
{
  int forCards[6],
      matchCards[5][2] = {{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}};

  for (int cardIndex = 0; cardIndex < 5; cardIndex++)
    if ((forCards[cardIndex] = CARD_NUM(hands[playerNum][cardIndex])) == 1)
      forCards[cardIndex] = 14;

  forCards[5] = 0;

  qsort(forCards, 6, 4, cardnumComparDescend);

  for (int cardIndex = 0; cardIndex < 5; cardIndex++) {
    if (forCards[cardIndex] == 14)
      forCards[cardIndex] = 1;

    for (int matchIndex = 0; matchIndex < 5; matchIndex++) {
      if (matchCards[matchIndex][0] == -1) {
        matchCards[matchIndex][0] = forCards[cardIndex];
        matchCards[matchIndex][1] = 1;

        break;
      }

      if (matchCards[matchIndex][0] != forCards[cardIndex])
        continue;

      matchCards[matchIndex][1]++;
    }
  }

  for (int matchIndex = 0; matchIndex < 5; matchIndex++) {
    if (matchCards[matchIndex][0] == -1)
      return false;

    if (matchCards[matchIndex][1] == wantThis)
      if (doublePair) {
        doublePair = false;
        *secondHighCard = matchCards[matchIndex][0];

        for (int cardIndex = 0; cardIndex < 5; cardIndex++)
          if (matchCards[matchIndex][0] == CARD_NUM(hands[playerNum][cardIndex])) {
            *secondHighCard |= (get_suit(hands[playerNum][cardIndex]));

            break;
          }
      } else {
        *highCard = matchCards[matchIndex][0];

        for (int cardIndex = 0; cardIndex < 5; cardIndex++)
          if (matchCards[matchIndex][0] == CARD_NUM(hands[playerNum][cardIndex])) {
            *highCard |= (get_suit(hands[playerNum][cardIndex]));

            break;
          }

        return true;
      }
  }

  return false;
}

int DrawPokerGame::getHighCard(int playerNum, int highCount) const
{
  int Cards[5];

  if (highCount > 4 || highCount < 0)
    return hands[playerNum][0];

  for (int Runner = 0; Runner < 5; Runner++) {
    Cards[Runner] = hands[playerNum][Runner];

    if (CARD_NUM(Cards[Runner]) == 1)
      Cards[Runner] = (14 | get_suit(Cards[Runner]));
  }

  qsort(Cards, 6, 4, cardnumComparDescend);

  return Cards[highCount];
}

void DrawPokerGame::settleUp(const TBeing *ch, bool doOutputs)
{
  if (silentBets)
    return;

  int playerNum,
      WinLoss;

  if ((playerNum = index(ch)) < 0) {
    vlogf(LOG_BUG, "DrawPoker::settleUp called by player not in the game!");
    return;
  }

  WinLoss = playerante[playerNum];
  TBeing *tChar;
  scores[playerNum] += WinLoss;

  if (doOutputs) {
    if (scores[playerNum] == 0)
      ch->sendTo("You broke even, lucky you.\n\r");
    else if (scores[playerNum] > 0)
      ch->sendTo("You came out ahead, good job.\n\r");
    else
      ch->sendTo("You went into this one, maybe next time.\n\r");
  }

  tChar = get_char_room(names[index(ch)], ROOM_DRAWPOKER);
  if (!tChar) {
    vlogf(LOG_BUG, format("WHOA, lost player in drawpoker [%s][index=%d][name=%s]") %  ch->getName() % index(ch) % names[index(ch)]);
    return;
  }

  tChar->addToMoney(WinLoss, GOLD_GAMBLE);
}

DrawPokerGame::DrawPokerGame() :
  CardGame(),
  iplay(0),
  nextCard(0),
  nextPlayer(0),
  initialPlayer(0),
  totalPlayers(0),
  lastAnte(0),
  game(false),
  silentBets(false),
  csilentBets(false),
  isbidding(false),
  usenewbie(false)
{
  anteCosts[0] = anteCosts[1] = 0;
  bidCosts[0] = bidCosts[1] = 0;

  for (int Runner = 0; Runner < 6; Runner++) {
    *(names[Runner])   = '\0';
    scores[Runner]     = 0;
    inuse[Runner]      = false;
    playerante[Runner] = 0;
    discarded[Runner]  = false;

    for (int cardMark = 0; cardMark < 5; cardMark++)
      hands[Runner][cardMark] = 0;
  }
}
