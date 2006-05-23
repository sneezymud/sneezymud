#!/usr/bin/perl
#-*- Perl -*-

use CGI::Carp qw(fatalsToBrowser);

$current_log="/mud/prod/lib/logs/logcurrent";

print "Content-type: text/html\n\n";

open(IN, "$current_log") || die $!;

while(<IN>){
  next if !/:: L.O.W. Error:/;

  chomp;
  s/\<.\>//g;
  s/\</&lt;/g;
  s/\>/&gt;/g;
  s/.*:: L.O.W. Error: (.*)/$1/;

  if(!$lowerrors{$_}){
    print "$_<br>\n";
    $count++;
  }

  $lowerrors{$_}++;
}

print "$count low errors\n";

