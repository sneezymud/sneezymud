#!/usr/bin/perl  -- Peel

foreach(`rlog *.cc *.h makefile README`){
  $fileName=$1 if /Working file: (.*)/;
  print "$fileName *** $_" if /locked by/;
}
