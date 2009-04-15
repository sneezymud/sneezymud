//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "games.h"

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
    const sstring score();
    bool  getPlayers(const TBeing *, TBeing **, TBeing **, TBeing **);
    void  deal(TBeing *);
    void  peek(const TBeing *);
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
