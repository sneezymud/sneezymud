//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: game_crazyeights.h,v $
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


class CrazyEightsGame : public CardGame {
  private:
    char names[4][20];
    int  scores[4],
         iplay,
         starterCard,
         nextCard,
         nextPlayer,
         initialPlayer;
    bool game,
         inuse[4];
  public:
    int hands[4][32];

    int   LEFT(const TBeing *) const;
    int   RIGHT(const TBeing *) const;
    int   ACROSS(const TBeing *) const;
    int   count(int);
    const string score();
    bool  getPlayers(const TBeing *, TBeing **, TBeing **, TBeing **);
    void  deal(TBeing *);
    void  peek(const TBeing *) const;
    int   move_card(TBeing *, const char *);
    int   enter(const TBeing *);
    int   exitGame(const TBeing *);
    int   index(const TBeing *) const;
    int   new_deal();
    void  pass(const TBeing *);
    void  play(const TBeing *, const char *);
    int   get(const TBeing *, const char *);
    int   look(const TBeing *, const char *);

    CrazyEightsGame();
};

extern CrazyEightsGame gEights;
