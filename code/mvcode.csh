#!/bin/csh

set CODE_MUD_ROOT = "/mud/build"
set PROD_MUD_ROOT = "/mud/prod"

if (0) then
echo "code move aborted at request of Lapsos"
echo "Making changes to the show system, not usable atm."
exit
endif

if (0) then
echo "Please don't move code, working on some things, Brutius"
exit
endif

if (0) then
echo "The v5.0 mud is in a frozen state."
echo "moving into beta v5.1 environment."
set PROD_MUD_ROOT = "/mud/5.1"
endif

if (0) then
echo "code move aborted at request of Batopr"
echo untested code
echo Messing with pet definitions
exit
endif

if (0) then
echo "code move aborted at request of Cosmo"
echo "Just need to test some of my code. "
echo "My code should not cause a problem so move despite this block.
echo "in case of emergency"
exit
endif

if (0) then
echo "code move aborted at request of Peel"
echo "need to test after compile"
echo "probably crashy"
exit
endif

if (1) then
echo "moving code to production"

# copy the latest executable
mv $CODE_MUD_ROOT/sneezy.2 $CODE_MUD_ROOT/sneezy
cp $CODE_MUD_ROOT/sneezy $PROD_MUD_ROOT/sneezy.2

cp $CODE_MUD_ROOT/objs/*.so $PROD_MUD_ROOT/objs.2

# move over text files  (preserve modification time)
rm $PROD_MUD_ROOT/lib/txt/news.new
cp -p $CODE_MUD_ROOT/lib/txt/news.new $PROD_MUD_ROOT/lib/txt/news.new
rm $PROD_MUD_ROOT/lib/txt/wiznews.new
cp -p $CODE_MUD_ROOT/lib/txt/wiznews.new $PROD_MUD_ROOT/lib/txt/wiznews.new
rm $PROD_MUD_ROOT/lib/txt/version.new
cp -p $CODE_MUD_ROOT/lib/txt/version.new $PROD_MUD_ROOT/lib/txt/version.new

endif
