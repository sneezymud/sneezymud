#!/usr/local/bin/bash

case $USER in
  peel)
    PORT=6968
    ;;
  angus)
    PORT=6975
    ;;
  jesus)
    PORT=7000
    ;;
  damescena)
    PORT=6969
    ;;
esac

if [ "$1" = "" ]
then
  echo "Usage: $0 <start|stop>";
elif [ "$1" = "start" ]
then
  pid=$(pgrep -U $USER -f "sneezy $PORT")
  if [ "$pid" == "" ]
  then
    mv -f sneezy.2 sneezy >& /dev/null
    rm -f file
    echo "Booting ./sneezy $PORT."
    ./sneezy $PORT >& file &
    pid=$(pgrep -U $USER -f "sneezy $PORT")
    echo "Running as process $pid.";
  else
    echo "Sneezy already running on port $PORT."
  fi
elif [ "$1" = "stop" ]
then
  pid=$(pgrep -U $USER -f "sneezy $PORT")
  if [ "$pid" != "" ]
  then
    echo "Killing process $pid."
    kill $pid
  else
    echo "Sneezy process not found running on port $PORT."
  fi
fi

