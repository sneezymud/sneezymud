#!/usr/bin/perl

use DBI;


my $db = DBI->connect("DBI:mysql:sneezyglobal", "sneezy", "")
    or die "Can't connect to $data_source: $dbh->errstr\n"; 

$timenow = time();
$lastday = $timenow - (24 * 60 * 60);
$lastweek = $timenow - (24 * 60 * 60 * 7);
$lastmonth = $timenow - (24 * 60 * 60 * 30);
$lastyear = $timenow - (24 * 60 * 60 * 30);

$sth=$db->prepare("select time,players from usagelogs where port=7900 AND time<=$timenow AND time>=$lastday");
$sth->execute;


local *FH;  
open(FH, ">usage.data");



while(($time, $players)=$sth->fetchrow_array){
  print FH "$time $players\n";
}

close(FH);

open(GP, "| /usr/bin/gnuplot");
print GP "set terminal png color\n";
print GP "set output '24hours.png'\n";
print GP "set width 500\n";
print GP "set height 100\n";
print GP "plot 'usage.data' with lines\n";
print GP "quit\n";
close(GP);

system("chmod 774 *.png");


