#!/usr/local/bin/bash

FLAGS="";

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
  maror)
    PORT=6960
    ;;
esac

if [ "$2" = "beta" ]
then
  PORT=5678;
elif [ "$2" = "lite" ]
then
  FLAGS="-t";
elif [ "$2" ]
then
 echo "Usage: $0 <start|stop> <beta|lite (optional)>";
 exit
fi

if [ "$1" = "" ]
then
  echo "Usage: $0 <start|stop> <beta|lite (optional)>";
elif [ "$1" = "start" ]
then
  pid=$(pgrep -U $USER -f "sneezy $PORT")
  if [ "$pid" == "" ]
  then
    mv -f sneezy.2 sneezy >& /dev/null
    rm -f file
    cp lib/tinyworld.mob lib/tinymob.use
    echo "Booting ./sneezy $PORT."
    ./sneezy $FLAGS $PORT >& file &
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

