//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "games.h" - All functions and routines related to sneezyMUD games
//
//////////////////////////////////////////////////////////////////////////

#ifndef __GAMES_H
#define __GAMES_H

#include <algorithm>
#include <deque>

#include "sstring.h"

class TBeing;
class TObj;

const int CARD_WATER = 128;
const int CARD_FIRE = 64;
const int CARD_EARTH = 32;
const int CARD_ETHER = 16;

enum cardSuitT {
  MIN_SUIT=0,
  SUIT_WATER=0,
  SUIT_FIRE,
  SUIT_EARTH,
  SUIT_ETHER,
  MAX_SUIT
};


const int CHIP_100    = 2350;
const int CHIP_500    = 2351;
const int CHIP_1000   = 2352;
const int CHIP_5000   = 2353;
const int CHIP_10000  = 2354;
const int CHIP_50000  = 2355;
const int CHIP_100000 = 2356;
const int CHIP_500000 = 2357;
const int CHIP_1000000= 2358;

// these are just for observer reactions
const int GAMBLER_WON           = 0;
const int GAMBLER_LOST          = 1;
const int GAMBLER_BET           = 2;
const int GAMBLER_HILO_BET      = 3;
const int GAMBLER_BLACKJACK_BET = 4;

TObj *find_chip(TBeing *, const int &);
TObj *find_chip(TBeing *, const sstring &);
void payout(TBeing *, int, int chip_vnum=0);
void observerReaction(TBeing *, int);

int cardnumComparAscend(const void *, const void *);
int cardnumComparDescend(const void *, const void *);

extern unsigned char CARD_NUM(unsigned char card);
extern unsigned char CARD_NUM_ACEHI(unsigned char card);

class Craps {
  public: 
    TBeing *m_ch;

  private:
    // intentionally made uncallable
    Craps();
  public:
    Craps(TBeing *ch);

    void checkOnerolls(int);
    void setPoint(int);
    int checkCraps(int);
    int checkSeven(int);
    int checkEleven(int);
    void checkOnerollSeven(int, TBeing *);
    void checkHorn(int, TBeing *);
    void checkField(int, TBeing *);
    void checkHardFour(int);
    void checkHardSix(int);
    void checkHardEight(int);
    void checkHardTen(int);
    void checkHardrolls(int);
    int rollDice();
    void checkTwo(int, TBeing *);
    void checkThree(int, TBeing *);
    void checkTwelve(int, TBeing *);
    void checkOnerollEleven(int, TBeing *);
    void checkOnerollCraps(int, TBeing *);
    void loseDice();
    void clearBets();
    void getDice();
};

class Hand {
  public:
    int both[11];
    int num;

    Hand();
};

typedef struct bj_players {
  bool inuse;
  char name[40];
  int deck[52], hand[12], dealer[12], np, nd;
  int deck_inx;
  int bet;
} BlackJack;

class Card {
 private:
  cardSuitT suit;
  int value;

 public:
  Card();
  Card(cardSuitT, int);
  virtual ~Card(){};

  cardSuitT getSuit() const;

  sstring getName() const;
  int getVal() const{
    return value;
  }
  int getValAceHi() const{
    if(value==1)
      return 14;
    return value;
  }
};

class CardDeck {
 private:
  std::deque <Card *> deck;
 public:
  CardDeck();
  virtual ~CardDeck(){};

  void shuffle();
  const Card *draw();
  const Card *undraw();

};

class CardGame {
  protected:
    int deck[52];
    bool game;
    int bet;
  public:
    CardGame();
    virtual ~CardGame() {}

    virtual void peek(const TBeing *) = 0;

    void take_card_from_hand(int *, int, int);
    void setup_deck();
    const sstring pretty_card_printout(const TBeing *, int) const;
    int same_suit(int, int);
    bool is_heart(int);
    bool is_queen_of_spades(int);
    bool has_suit(int *, int);
    int add_suit(const TBeing *, char *, int) const;
    const sstring suit(const TBeing *, int) const;
    void shuffle();
    void pass(TBeing *, const char *, int);
    void order_high_to_low(int *, int *, int *);
    virtual int index(const TBeing *) const { return false; }
};

class GinGame : public CardGame {
  private:
    char names[2][20];
    int pile[32];
    bool inuse[2];
    bool loser[2];
    bool can_draw;
    int score[2];
  public:
    int hands[2][11];
    int topcard, deck_index, pile_index;

    GinGame();

    int count(int which);
    void deal(TBeing *);
    virtual int index(const TBeing *) const;
    void play(TBeing *, const char *);
    bool check(const TBeing *ch) const;
    virtual void peek(const TBeing *);
    int move_card(TBeing *ch, const char *arg);
    int enter(const TBeing *ch);
    int exitGame(const TBeing *ch);
    int draw(TBeing *ch, const char *arg);
    void clear();
    void clear_hand();
    void win(TBeing *ch);
    void win_hand(TBeing *ch);
    void gin(TBeing *ch);
    void knock(TBeing *ch, int low);
    int total_not_in_book(int *hand, Hand *hs);
    int can_knock_or_gin(TBeing *ch);
    int *find_book(int num, int *hand, int *left);
    int *find_run(int num, int *hand, int *left);
    int recursive_gin_search(TBeing *ch, Hand *hs, int *hand);
    const sstring gin_score();
    int look(TBeing *ch, const char *arg);
};

class BjGame : public CardGame {
  private:
    bool inuse;
    char name[40];
    int hand[12], dealer[12], np, nd;
    int deck_inx;
  public:
    BjGame();
    bool enter(const TBeing *ch);
    virtual int index(const TBeing *) const;
    void Split(TBeing *, const char *, int);
    void Bet(TBeing *ch, const char *arg);
    virtual void peek(const TBeing *ch);
    void stay(TBeing *ch);
    void bj_shuffle(int inx, const TBeing *ch);
    int exitGame(const TBeing *);
    int min_score();
    int best_dealer();
    int best_score();
    void Hit(const TBeing *);
    int check_for_bet() {
      return bet;
    }
    friend int bj_index(TBeing *);
};
  
class HeartsGame : public CardGame {
  private:
    char names[4][20];
    int score[4];
    int iplay;  // index [0..3] counting how many people have played
    int canplay;  // the index of the player now permitted to play
    int led;  // the value of the card that led
    bool broken;
    int round;
    int done_passing;  // how many people are done passing
    int ipass;  // direction of pasing for this round
    bool passing;
    bool canpass[4];
    bool cangetpass[4];
    int passes[4][3];
    bool inuse[4];
    int firstplay;
  public:
    int hands[4][13];
    int pile[4];
    int tricks[13][5];

    HeartsGame();

    const sstring hearts_score();
    void deal(TBeing *);
    void play(TBeing *, const char *);
    virtual void peek(const TBeing *);
    virtual int index(const TBeing *) const;
    bool get_other_players(const TBeing *, TBeing **, TBeing **, TBeing **);
    int enter(const TBeing *ch);
    int exitGame(const TBeing *ch);
    int new_deal();
    int new_round(TBeing *ch, int *pile);
    void pass(TBeing *ch, const char *arg);
    int move_card(TBeing *ch, const char *arg);
    int find_two_of_clubs();
    int look(TBeing *ch, const char *arg);
    int get_pass(TBeing *ch, char *arg);
    int count(int which);
    int LEFT(const TBeing *) const;
    int RIGHT(const TBeing *) const;
    int ACROSS(const TBeing *) const;
};


class HiLoGame : public CardGame {
 private:
  const Card *card;
  bool inuse;
  float win_perc;
  sstring name;
  CardDeck deck;
 public:
  bool enter(const TBeing *ch);
  virtual void peek(const TBeing *);
  void BetHi(TBeing *, const Card *);
  void BetLo(TBeing *, const Card *);
  void Bet(TBeing *ch, const sstring &arg);
  void stay(TBeing *ch);
  int check_for_bet() {
    return bet;
  }
  int exitGame(const TBeing *ch);
  virtual int index(const TBeing *) const;
};

class PokerGame : public CardGame {
 private:
  int card[5];
  bool inuse;
  int deck_inx;
  sstring name;
 public:
  bool enter(const TBeing *ch);
  virtual void peek(const TBeing *);
  void Bet(TBeing *ch, const sstring &arg);
  void poker_shuffle(const TBeing *ch);
  void stay(TBeing *ch);
  void discard(TBeing *ch, sstring arg);
  int check_for_bet() {
    return bet;
  }
  int exitGame(const TBeing *ch);
  virtual int index(const TBeing *) const;
  bool isRoyalFlush();
  bool isStraightFlush();
  bool isFourOfAKind();
  bool isFullHouse();
  bool isFlush();
  bool isStraight();
  bool isThreeOfAKind();
  bool isTwoPair();
  bool isPair();
};


class BaccaratGame : public CardGame {
 private:
  int player[3], dealer[3];
  bool inuse;
  int deck_inx;
  sstring name;
  int bet_type;
 public:
  bool enter(const TBeing *ch);
  virtual void peek(const TBeing *);
  void Bet(TBeing *ch, const sstring &arg);
  void baccarat_shuffle(const TBeing *ch);
  void stay(TBeing *ch);
  void Hit(TBeing *);
  int check_for_bet() {
    return bet;
  }
  int exitGame(const TBeing *ch);
  virtual int index(const TBeing *) const;
};


enum holdemStateT {
  STATE_NONE,
  STATE_DEAL,
  STATE_FLOP,
  STATE_TURN,
  STATE_RIVER
};

class HoldemPlayer {
 public:
  sstring name;
  TBeing *ch;
  const Card *hand[2];
  bool allin;

  HoldemPlayer(const TBeing *ch);
};


const int MAX_HOLDEM_PLAYERS=5;

class HoldemGame : public CardGame {
 private:
  CardDeck deck;
  const Card *community[5];
  HoldemPlayer *players[MAX_HOLDEM_PLAYERS];
  int better, firstbetter, button;
  
  holdemStateT state;
  int last_bet;
  int nraises;
  int nraises_round;
 public:
  HoldemGame(){
    state=STATE_NONE;
  }

  sstring handValToStr(int);
  sstring stateToString();
  HoldemPlayer *getBetter(){return players[better];}
  int getNRaises(){ return nraises;}
  int getLastBet(){return last_bet;}
  holdemStateT getState(){return state;}
  int handValue(HoldemPlayer *);
  void advanceRound(TBeing *ch);
  int nextBetter(int);
  int nextPlayer(int);
  int nextButton();
  int lastPlayer();
  int firstPlayer();
  void linkPlayers();
  int playerCount();
  int playerHandCount();
  bool enter(const TBeing *ch);
  int exitGame(const TBeing *ch);
  virtual void peek(const TBeing *);
  bool isPlaying(const TBeing *) const;
  HoldemPlayer *getPlayer(const sstring &name) const;
  void Bet(TBeing *ch, const sstring &arg);
  void call(TBeing *ch);
  void raise(TBeing *ch, const sstring &arg);
  void fold(TBeing *ch);
  void flop(TBeing *ch);
  void river(TBeing *ch);
  void turn(TBeing *ch);
  void showdown(TBeing *ch);
};

extern GinGame gGin;
extern HeartsGame gHearts;
extern BjGame gBj;
extern HiLoGame gHiLo;
extern PokerGame gPoker;
extern BaccaratGame gBaccarat;
extern HoldemGame gHoldem;

/* craps_options */

const unsigned int COME_OUT=(1<<0);
const unsigned int CRAP_OUT=(1<<1);

/* One roll bets, and bets that can be laid anytime. */

const unsigned int ELEVEN       =(1<<0);
const unsigned int TWELVE       =(1<<1);
const unsigned int HARD_EIGHT   =(1<<2);
const unsigned int HARD_TEN     =(1<<3);
const unsigned int HARD_SIX     =(1<<4);
const unsigned int HARD_FOUR    =(1<<5);
const unsigned int CRAPS        =(1<<6);
const unsigned int HORN_BET     =(1<<7);
const unsigned int FIELD_BET    =(1<<8);
const unsigned int TWO2         =(1<<9);
const unsigned int THREE3       =(1<<10);
const unsigned int SEVEN        =(1<<11);

/* For the act_ptrs that control when you can throw dice */

const int START_BETS    =10;
const int SECOND_CALL   =6;
const int LAST_CALL     =2;
 
extern int recursive_gin_search(TBeing *, Hand *hs, int *hand);
extern char *pretty_card_printout(TBeing *, int);
extern int best_bj_dealer(int);
extern int best_bj_score(int);
extern int min_bj_score(int);
extern int gin_index(TBeing *);
extern void take_card_from_hand(int *, int, int);
extern const char *suit(TBeing *, int card);
extern int hearts_index(TBeing *);
extern int hearts_pass(TBeing *, char *arg);
extern int hearts_get_pass(TBeing *, char *);
extern int get_suit(int);

#endif
