#!/bin/csh -f

foreach file (*.cc *.h makefile README)
  if ("`rlog $file | grep 'locked by'`" != "") then
    echo "$file *** `rlog $file | grep 'locked by'`"
  endif
#  set revLine = `rlog $file | grep "locked by"`
#  if ("$revLine" != "") then
#    echo "$file *** $revLine"
#  endif

  set branchLine = `rlog $file | grep "branch:"`
  if ("$branchLine" != "branch:") then
    if ("$branchLine[2]" != "5.1.1") then
      echo "$file *** $branchLine"
    endif
  endif

  set headLine = `rlog $file | grep "head:"`
  if ("$headLine" != "head:") then
    if ("$headLine[2]" != "5.1") then
      echo "$file *** $headLine"
    endif
  endif

end
