#include "stdsneezy.h"
#include "process.h"
#include "mail.h"

// procCheckMail
procCheckMail::procCheckMail(const int &p)
{
  trigger_pulse=p;
  name="procCheckMail";
}

void procCheckMail::run(int pulse) const
{
  Descriptor *d;

  if (gamePort == BUILDER_GAMEPORT)
    return;
  
  for (d = descriptor_list; d; d = d->next) {
    TBeing *ch = d->character;
    if (!no_mail && !d->connected && ch) {
      char recipient[100], *tmp;

      _parse_name(ch->getName(), recipient);
      for (tmp = recipient; *tmp; tmp++)
        if (isupper(*tmp))
          *tmp = tolower(*tmp);
      if (has_mail(recipient))
        ch->sendTo(fmt("You have %sMAIL!%s\n\r") % ch->cyan() % ch->norm());
    }
  }
}

