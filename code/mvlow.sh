#!/usr/local/bin/bash

BUILD_MUD_ROOT="/mud/build"
PROD_MUD_ROOT="/mud/prod"
TOUCHFILE=$BUILD_MUD_ROOT/mvlow.lock

umask 002
if [ -a $TOUCHFILE ]; then
  echo "LOW move aborted at request of Damescena."
  echo "...busy screwing stuff up, please don't move LOW."
  exit
fi

# copy the latest tiny files
echo Copying tinyworld.mob
cp $BUILD_MUD_ROOT/lib/tinyworld.mob $PROD_MUD_ROOT/lib/tinyworld.mob

echo Updating zonefiles data
(
cd $PROD_MUD_ROOT/lib/zonefiles
svn update
)

echo Updating response and rumor data
(
cd $PROD_MUD_ROOT/lib/mobdata
svn update
)

#cp $BUILD_MUD_ROOT/lib/zonefiles/* $PROD_MUD_ROOT/lib/zonefiles
#cd $PROD_MUD_ROOT/lib/zonefiles
#cvs update

# copy latest Crier Response File
#echo Copying town crier response file

#cp $BUILD_MUD_ROOT/lib/mobdata/responses/crier.new $PROD_MUD_ROOT/lib/mobdata/responses/crier


# note that we've removed indexes from sneezyq for speed, so pg_restore 
# will complain about missing constraints
echo '***** Note that "XXX_pkey does not exist" errors are ok - ignore them.'
for i in obj objextra objaffect \
         shop shopproducing shopmaterial shoptype \
         room roomexit roomextra itemtypes;
do 
  echo Copying $i table.
  mysqldump -u sneezy -h db.sneezymud.com sneezybeta $i | mysql -u sneezy -h db.sneezymud.com sneezyq
done


