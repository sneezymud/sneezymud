#!/usr/local/bin/bash

whoami=`whoami`

if [ "$1" = "" ]
then
  echo "Usage: $0 <start|stop>";
elif [ "$1" = "start" ]
then
  pid=`pgrep -U $whoami -f "gmake"`
  if [ "$pid" == "" ]
  then
    echo "Starting make, output to file.mak.";
# the second gmake is to make sure the linking is done properly; sometimes
# gmake gets confused with multiple processes.  The && makes sure that it
# only runs if the first build completed successfully, so we don't rebuild
# files that errored out
    (gmake -k -j 2 -l 3 && gmake) >& file.mak &
  else
    echo "You are already running make."
  fi
elif [ "$1" = "stop" ]
then
  pid=`pgrep -U $whoami -f "gmake"`
  if [ "$pid" != "" ]
  then
    echo "Killing process $pid."
    kill $pid
  else
    echo "Make process not found."
  fi
elif [ "$1" = "status" ]
then
  pid=`pgrep -U $whoami -f "gmake"`
  if [ "$pid" != "" ]
  then
    echo "Make process found, pid $pid."
  else
    echo "Make process not found."
  fi
fi

