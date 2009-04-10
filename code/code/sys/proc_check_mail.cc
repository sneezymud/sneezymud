#include "stdsneezy.h"
#include "person.h"
#include "configuration.h"
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
    TBeing *ch = d->original ? d->original : d->character;
    if (!no_mail && !d->connected && ch) {
      sstring recipient;

      if (parse_name_sstring(ch->getName(), recipient)) {
        continue;
      }
      if (has_mail(recipient.lower()))
        ch->sendTo(format("You have %sMAIL!%s\n\r") % ch->cyan() % ch->norm());
    }
  }
}

