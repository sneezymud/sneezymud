#include "process.h"
#include "being.h"
#include "thing.h"
#include "obj.h"
#include "extern.h"

// procCheckForRepo
procCheckForRepo::procCheckForRepo(const int &p)
{
  trigger_pulse=p;
  name="procCheckForRepo";
}

void procCheckForRepo::run(const TPulse &) const 
{
  TBeing *tmp_ch, *temp;

  for (tmp_ch = character_list; tmp_ch; tmp_ch = temp) {
    temp = tmp_ch->next; 
    int i;
    TThing *repot;
    TObj *repoo;
    // check worn equipment
    for (i = MIN_WEAR;i < MAX_WEAR;i++) {
      if (!(repot = tmp_ch->equipment[i]) || !(repoo = dynamic_cast<TObj *>(repot)))
	continue;
      
      repoCheckForRent(tmp_ch, repoo, false);
    }
    // check inventory
    for(StuffIter it=tmp_ch->stuff.begin();it!=tmp_ch->stuff.end();){
      repot=*(it++);
      repoo = dynamic_cast<TObj *>(repot);
      if (!repoo)
	continue;
      
      repoCheckForRent(tmp_ch, repoo, false);
    }
    
  }
}

