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
    gmake -k -j 2 -l 3 >& file.mak &
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

