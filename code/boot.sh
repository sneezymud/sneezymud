#!/usr/local/bin/bash

if   [ $USER = "peel"   ]; then PORT=6968
elif [ $USER = "sneezy" ]; then PORT=7900
fi

if [ "$1" = "" ]
then
  echo "Usage: $0 <start|stop>";
elif [ "$1" = "start" ]
then
  pid=`pgrep -U peel -f "sneezy 6968"`
  if [ "$pid" == "" ]
  then
    mv -f sneezy.2 sneezy >& /dev/null
    rm -f file
    echo "Booting."
    ./sneezy $PORT >& file &
  else
    echo "Sneezy already running on port $PORT."
  fi
elif [ "$1" = "stop" ]
then
  pid=`pgrep -U peel -f "sneezy 6968"`
  if [ "$pid" != "" ]
  then
    echo "Killing process $pid."
    kill $pid
  else
    echo "Sneezy process not found running on port $PORT."
  fi
fi

