#!/bin/bash

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
  dash)
    PORT=6996
    ;;
  metrohep)
    PORT=6904
    ;;
  coral)
    PORT=6905
    ;;
  deirdre)
    PORT=6942
    ;;
  brutius)
    PORT=6906
    ;;
  magdalena)
    PORT=6907
    ;;
  macross)
    PORT=6999
    ;;
  vasco)
    PORT=6908
    ;;
  pappy)
    PORT=1337
    ;;
  staffa)
    PORT=6900
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
 echo "Usage: $0 <start|stop|restart> <beta|lite (optional)>";
 exit
fi

if [ "$1" = "" ]
then
  echo "Usage: $0 <start|stop|restart> <beta|lite (optional)>";
  exit
fi

if [[ "$1" = "stop" || "$1" = "restart" ]]
then
  pid=$(pgrep -U $USER -f "sneezy $PORT")
  if [ "$pid" != "" ]
  then
    echo "Killing process $pid."
    kill $pid
  else
    pid=$(pgrep -U $USER -f "sneezy -t $PORT")
    if [ "$pid" != "" ]
    then
      echo "Killing process $pid."
      kill $pid
    else
      echo "Sneezy process not found running on port $PORT."
    fi
  fi
fi

if [ "$1" = "restart" ]
then
  sleep 2
fi

if [[ "$1" = "start" || "$1" = "restart" ]]
then
  pid=$(pgrep -U $USER -f "sneezy $PORT")
  if [ "$pid" == "" ]
  then
    rm -f file
    echo "Booting ./sneezy $PORT."
    ./sneezy $FLAGS $PORT >& file &
    sleep 2
    pid=$(pgrep -U $USER -f "sneezy $PORT")
    echo "Running as process $pid.";
  else
    echo "Sneezy already running on port $PORT."
  fi
fi

