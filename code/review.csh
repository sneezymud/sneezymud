#!/bin/csh

# Useage:
#   "review" 
#      - Will cause it to show the RCS logs for all code-related files
#      modified today
#   "review Oct 25" 
#      - Will cause it to show the RCS logs for all code-related files
#      last modified since Oct 25

if ("$1" != "") then
  if ("$2" == "") then
    echo "You need to enter a month and a day"
    exit
  endif

  set dateTime = "$1 $2"
else
  echo "Using today's date"
  set monthTime = `date +"%h"`
  set dayTime = `date +"%d"`
  set dateTime = "$monthTime $dayTime"
endif
echo "Reviewing for changes since: '$dateTime'"

if ("`whoami`" == "batopr") then
  set rlogOpts = "-wbrutius,cosmo,damescena,dolgan,gringar,hylidan,kriebly,lapsos,lothar,marsh,merc,mithros,moath,omen,peel,rixanne,satan,sidartha,sneezy,spawn,speef,stone"
else
  set rlogOpts =
endif

foreach file (*.cc *.h makefile README)
  set revLine = `rlog -N -d"$dateTime<" $rlogOpts $file | grep "selected revisions"`
  set revNum = $revLine[6]
  if ($revNum > 0) then
    rlog -N -d"$dateTime<" $rlogOpts $file
    echo "<### Hit Return ###>"
    set input = $<
  endif
end
