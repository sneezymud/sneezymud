#!/usr/bin/perl

$hum_m_h=72;
$hum_m_w=173;
$hum_f_h=67;
$hum_f_w=122;

while(<>){
  if(/^maleHt ([^\+]+)\+(.+)d(.+)/){
    $h=$1 + ($2 * (($3 / 2)+0.5));
    $mh=$h;
  } elsif(/^femaleHt ([^\+]+)\+(.+)d(.+)/){
    $h=$1 + ($2 * (($3 / 2)+0.5));
    $fh=$h;
  }
  if(/^maleWt ([^\+]+)\+(.+)d(.+)/){
    $h=$1 + ($2 * (($3 / 2)+0.5));
    $mw=$h;
    print "m: $1 $2 $3 $h\n";
  } elsif(/^femaleWt ([^\+]+)\+(.+)d(.+)/){
    $h=$1 + ($2 * (($3 / 2)+0.5));
    $fw=$h;
    print "f: $1 $2 $3 $h\n";
  }

}



printf("height : [ %.2f, %.2f ],\n", $mh/$hum_m_h, $fh/$hum_m_h);
printf("weight : [ %.2f, %.2f ],\n", $mw/$hum_m_w, $fw/$hum_m_w);


__DATA__
maleHt 62+1d17
maleWt 140+6d10
femaleHt 60+1d12
femaleWt 100+4d10
