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
  if ("$dayTime" == "01") then
    set dayTime = " 1"
  else if ("$dayTime" == "02") then
    set dayTime = " 2"
  else if ("$dayTime" == "03") then
    set dayTime = " 3"
  else if ("$dayTime" == "04") then
    set dayTime = " 4"
  else if ("$dayTime" == "05") then
    set dayTime = " 5"
  else if ("$dayTime" == "06") then
    set dayTime = " 6"
  else if ("$dayTime" == "07") then
    set dayTime = " 7"
  else if ("$dayTime" == "08") then
    set dayTime = " 8"
  else if ("$dayTime" == "09") then
    set dayTime = " 9"
  endif
  set dateTime = "$monthTime $dayTime"
endif
echo "Reviewing for: '$dateTime'"

foreach file (*.cc *.h makefile README)
  set revLine = `rlog -N -d"$dateTime<" $file | grep "selected revisions"`
  set revNum = $revLine[6]
  if ($revNum > 0) then
    rlog -N -d"$dateTime<" $file
    echo "<### Hit Return ###>"
    set input = $<
  endif
end
