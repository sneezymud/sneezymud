#!/bin/bash

whoami=`whoami`

if [ "$1" = "start" ]
then
  pid=`pgrep -U $whoami -f "scons"`
  if [ "$pid" == "" ]
  then
    echo "Starting scons, output to file.mak.";
    (scons -k --no-progress) >& file.mak &
  else
    echo "You are already running scons."
  fi
elif [ "$1" = "stop" ]
then
  pid=`pgrep -U $whoami -f "scons"`
  if [ "$pid" != "" ]
  then
    echo "Killing process $pid."
    kill $pid
  else
    echo "Make process not found."
  fi
elif [ "$1" = "status" ]
then
  pid=`pgrep -U $whoami -f "scons"`
  if [ "$pid" != "" ]
  then
    echo "scons process found, pid $pid."
  else
    echo "scons process not found."
  fi
else
  echo "Usage: $0 <start|stop>";
fi

