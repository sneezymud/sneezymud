//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: drawpoker.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


class DrawPokerGame : public CardGame {
  private:
    char names[6][20];
    int  scores[6],     // Total earned for each player.
         playerante[6], // What the player has on the table.  bet wise.
         iplay,
         nextCard,
         nextPlayer,
         initialPlayer,
         totalPlayers,
         anteCosts[2],
         bidCosts[2],
         lastAnte;
    bool game,
         inuse[6],
         silentBets,
         csilentBets,
         discarded[6],
         isbidding,
         usenewbie;
  public:
    int hands[6][5];

    int   LEFT(const TBeing *) const;
    int   LEFT(int) const;
    int   getNextPlayer(const TBeing *);
    int   count(int) const;
    const string score() const;
    const string bets() const;
    bool  getPlayers(const TBeing *, TBeing **, TBeing **,
                     TBeing **, TBeing **, TBeing **) const;
    void  deal(TBeing *, const char *);
    int   averagePlayerLevel() const;
    void  peek(const TBeing *) const;
    int   move_card(TBeing *, const char *);
    int   enter(const TBeing *);
    int   exitGame(const TBeing *);
    int   index(const TBeing *) const;
    int   new_deal();
    void  pass(const TBeing *);
    void  bet(const TBeing *, const char *);
    bool  isBettingClosed() const;
    void  stop(const TBeing *);
    int   look(const TBeing *, const char *);
    void  discard(const TBeing *, const char *);
    int   findWinner(int *, int *, int *, int *, int *, int *, int *);
    bool  isStraight(int) const;
    bool  isFlush(int) const;
    bool  isPair(int, int, int *, bool, int *) const;
    int   getHighCard(int, int) const;
    void  settleUp(const TBeing *, bool);

    DrawPokerGame();
};

extern DrawPokerGame gPoker;
