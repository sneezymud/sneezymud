#!/bin/sh

SVNROOT="http://svn.sneezymud.com/repos/source/trunk/code"
cd $HOME
svn checkout $SVNROOT sneezy
ln -s /mud/build/lib sneezy/lib
cp sneezy/sneezy_beta.cfg sneezy/sneezy.cfg
echo "Your dev environment is now in $HOME/sneezy."
echo "An SVN tutorial is available: $HOME/sneezy/svntutorial.txt"
