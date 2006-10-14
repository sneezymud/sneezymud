#!/bin/bash

whoami=`whoami`

if [ "$1" = "start" ]
then
  pid=`pgrep -U $whoami -f "gmake"`
  if [ "$pid" == "" ]
  then
    echo "Starting make, output to file.mak.";
    (gmake -k -j 3 -l 3 libs && gmake exe) >& file.mak &
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
else
  echo "Usage: $0 <start|stop>";
fi

