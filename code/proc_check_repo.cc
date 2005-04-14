#include "stdsneezy.h"
#include "process.h"


// procCheckForRepo
procCheckForRepo::procCheckForRepo(const int &p)
{
  trigger_pulse=p;
  name="procCheckForRepo";
}

void procCheckForRepo::run(int pulse) const 
{
  TBeing *tmp_ch, *temp;

  for (tmp_ch = character_list; tmp_ch; tmp_ch = temp) {
    temp = tmp_ch->next; 
    int i;
    TThing *repot, *repot2;
    TObj *repoo;
    // check worn equipment
    for (i = MIN_WEAR;i < MAX_WEAR;i++) {
      if (!(repot = tmp_ch->equipment[i]) || !(repoo = dynamic_cast<TObj *>(repot)))
	continue;
      
      repoCheckForRent(tmp_ch, repoo, false);
    }
    // check inventory
    for (repot = tmp_ch->getStuff(); repot; repot = repot2) {
      repot2 = repot->nextThing;
      repoo = dynamic_cast<TObj *>(repot);
      if (!repoo)
	continue;
      
      repoCheckForRent(tmp_ch, repoo, false);
    }
    
  }
}

