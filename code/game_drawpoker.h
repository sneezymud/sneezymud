//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: game_drawpoker.h,v $
// Revision 5.4  2003/09/16 00:08:46  peel
// removed const from peek()
// added undraw to CardDeck
// additional additional debug info to peek
//
// Revision 5.3  2003/04/28 02:04:39  peel
// added poker game (video poker)
//
// Revision 5.2  2003/03/13 22:40:53  peel
// added sstring class, same as string but takes NULL as an empty string
// replaced all uses of string to sstring
//
// Revision 5.1  2001/07/13 05:32:20  peel
// renamed a bunch of source files
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
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
    const sstring score() const;
    const sstring bets() const;
    bool  getPlayers(const TBeing *, TBeing **, TBeing **,
                     TBeing **, TBeing **, TBeing **) const;
    void  deal(TBeing *, const char *);
    int   averagePlayerLevel() const;
    void  peek(const TBeing *);
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

extern DrawPokerGame gDrawPoker;
