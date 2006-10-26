#!/bin/bash
#
# SneezyMUD rebooter watchdog script, run via cron
#
# January, 2005 - initial version
# October, 2006 - modified for linux

cd /mud/prod

# Close stdin so that this process can spawn daemons better
exec 0<&-

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin
export PATH

if [ -r rebooter.cfg ]
then
  source ./rebooter.cfg
else
  echo "Error: $(basename $0) can't read in rebooter.cfg"
  exit 1
fi

REBOOTER_WATCHDOG_LOG=${PROD_DIR}/rebooter-watchdog.log
#running_pid=$(pgrep -U sneezy -f "/usr/local/bin/bash .*${REBOOTER}")

if [ -e "${PROD_LOCK}" ]
then
  DATE_STAMP=$(date +%Y%m%d.%H%M)
  echo "${DATE_STAMP}: Lock file ${PROD_LOCK} found, not starting ${REBOOTER}" >> ${REBOOTER_WATCHDOG_LOG}
  exit 0
fi

if [ -s "${REBOOTER_PID}" ]
then
  stored_pid=$(cat "${REBOOTER_PID}")
  verify_pid=$(ps ax | awk '{print $1}' | egrep "^${stored_pid}$")
  if [ "$stored_pid" != "$verify_pid" ]
  then
    # we have a problem, REBOOTER_PID isn't running anymore
    DATE_STAMP=$(date +%Y%m%d.%H%M)
    echo "${DATE_STAMP}: ${REBOOTER}, pid=$stored_pid was dead!  Spawning new rebooter." >> ${REBOOTER_WATCHDOG_LOG}
    exec "${PROD_DIR}/${REBOOTER}"
  fi
else
  # $REBOOTER_PID is not here or empty at this point, we have a problem
  DATE_STAMP=$(date +%Y%m%d.%H%M)
  echo "${DATE_STAMP}: ${REBOOTER} wasn't running!  Spawning new rebooter." >> ${REBOOTER_WATCHDOG_LOG}
  exec "${PROD_DIR}/${REBOOTER}"
fi

exit 0
