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
  # This routine figures our who is on the system, creates a list of
  # everyone EXCEPT myself, and uses those names as the logins to review
  # ergo, we will be reviewing changes for everyone except yourself
  set rlogOpts =
  set curDir = `pwd`
  cd /home
  foreach file (*)
    if ("$file" != "`whoami`") then
      if ("$rlogOpts" == "") then
        set rlogOpts = "$file"
      else
        set rlogOpts = "$rlogOpts"",""$file"
      endif
    endif
  end
  if ("$rlogOpts" != "") then
    set rlogOpts = "-w""$rlogOpts"
  endif
  cd $curDir
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
