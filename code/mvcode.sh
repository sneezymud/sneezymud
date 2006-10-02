#!/bin/bash

CODE_MUD_ROOT="/mud/build"
PROD_MUD_ROOT="/mud/prod"

if false; then
echo "code move aborted at request of Lapsos"
echo "Making changes to the show system, not usable atm."
exit
fi

if false; then
echo "Please don't move code, working on some things, Brutius"
exit
fi

if false; then
echo "The v5.0 mud is in a frozen state."
echo "moving into beta v5.1 environment."
set PROD_MUD_ROOT = "/mud/5.1"
fi

if false; then
echo "code move aborted at request of Batopr"
echo untested code
echo Messing with pet definitions
exit
fi

if false; then
echo "code move aborted at request of Cosmo"
echo "Just need to test some of my code. "
echo "My code should not cause a problem so move despite this block."
echo "in case of emergency"
exit
fi

if false; then
echo "code move aborted at request of Peel"
echo "need to test after compile"
echo "probably crashy"
exit
fi

if true; then
echo "Moving code to production."

# copy the latest executable
echo "Copying binaries."
mv -fv $CODE_MUD_ROOT/sneezy.2 $CODE_MUD_ROOT/sneezy
cp -fv $CODE_MUD_ROOT/sneezy $PROD_MUD_ROOT/sneezy.2

echo "Copying shared object files."
cp -fv $CODE_MUD_ROOT/objs/*.so $PROD_MUD_ROOT/objs.2

echo "Copying news files."
# move over text files  (preserve modification time)
rm -f $PROD_MUD_ROOT/lib/txt/news.new
cp -fp $CODE_MUD_ROOT/lib/txt/news.new $PROD_MUD_ROOT/lib/txt/news.new
rm -f $PROD_MUD_ROOT/lib/txt/wiznews.new
cp -fp $CODE_MUD_ROOT/lib/txt/wiznews.new $PROD_MUD_ROOT/lib/txt/wiznews.new
rm -f $PROD_MUD_ROOT/lib/txt/version.new
cp -fp $CODE_MUD_ROOT/lib/txt/version.new $PROD_MUD_ROOT/lib/txt/version.new

echo "Finished."

fi
