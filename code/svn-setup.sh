#!/bin/sh

SVNROOT="file:///mud/source/trunk/code"
cd $HOME
svn checkout $SVNROOT
ln -s /mud/build/lib code/lib
echo "Your dev environment is now in $HOME/code."
echo "An SVN tutorial is available: $HOME/code/svntutorial.txt"
