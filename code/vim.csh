#/bin/csh -f

if (-f $1.lock) then
echo It appears that file is currently in use.
more $1.lock
exit
endif

if ($USER == batopr) then
setenv TERM ansi
endif

echo "$USER at `date`" > $1.lock
/usr/bin/co -l $1
/usr/bin/vim $1
/usr/bin/ci -u $1
rm -f $1.lock

if ($USER == batopr) then
setenv TERM vt100
endif

