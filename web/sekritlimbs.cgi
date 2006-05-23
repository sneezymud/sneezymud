#!/usr/local/bin/perl -w
# for listing limbs for limb quest with cagrane
use strict;
use CGI;
use CGI::Carp qw(fatalsToBrowser);

my $pg = new CGI;
print $pg->header(),  $pg->start_html(-title=>'Limb Fest June 2004');
#print "<B><H2>Please note:  the contest has not started.  We are just testing things out.</H2></B><P>";
print "<B><H2>The limbs collection phase of the contest has ended.</H2></B><P>";
#print "Disabled<BR>\n";
#print $pg->end_html();
#exit(0);
my @limblist = ( "head", "eyeballs",  "heart", "right arm", "left arm",
  "right hand", "left hand", "right finger", "left finger", "genitalia",
  "right foot", "left foot", "tooth" );

my @teams; # array of arrays of hashes

$teams[0]{"members"} = [ qw ( Beloc Peewee Crazed Merkaba Sephie Shakudo
  Thelonious Baffleflap Raven Xalami Fitz Releep Ashcroft Blum Weps Jiraiya
  Antonius ) ];
$teams[0]{"limbs"} = [];
$teams[1]{"members"} = [ qw ( Apollyon Laren Slab Arkane IRS Selch Ferry
  Intuition Frobozz Stin Legato Trina Aiolos Fedaykin Kelranth
  Trask Amberbock ) ];
$teams[1]{"limbs"} = [];

system("/mud/web/limbfest/limb_log_gen.py") or die $!;
#my @logs_summaries = `ls -t /mud/web/limbfest/logs/log*`;
my @logs_summaries = `ls -t /mud/web/limbfest/logs/logcurrent* /mud/web/limbfest/logs/log.070104.0539 /mud/web/limbfest/logs/log.063004.0348 /mud/web/limbfest/logs/log.062704.1453 /mud/web/limbfest/logs/log.062704.1355 /mud/web/limbfest/logs/log.062704.0952 /mud/web/limbfest/logs/log.062504.1559 /mud/web/limbfest/logs/log.062504.1528 /mud/web/limbfest/logs/log.062504.0216 /mud/web/limbfest/logs/log.062404.1849 /mud/web/limbfest/logs/log.062404.1238 /mud/web/limbfest/logs/log.062404.0838`;
  
foreach my $log (@logs_summaries) {
# ANGUS:
  last if $log eq "/mud/web/limbfest/logs/log.062304.2142\n";
  open LOG, " $log";
  my @log = <LOG>;
  close(LOG);
  foreach my $ind (0..$#teams) {
    foreach my $pc (@{$teams[$ind]{"members"}}) {
      #my @newlimbs = grep /$pc/i, @log;
      my @newlimbs = grep / $pc /i, @log;
      if (!defined($teams[$ind]{"limbs"})) {
        $teams[$ind]{"limbs"} = [ @newlimbs ];
      } else {
        $teams[$ind]{"limbs"} = [ @{$teams[$ind]{"limbs"}}, @newlimbs ];
      }
    }
  }
}

foreach my $ind (0..$#teams) {
  my @limbs = @{$teams[$ind]{"limbs"}};
  foreach my $limb (@limbs) {
    if ($limb =~ /Maror: (.* chopped by \w+)/) {
      my $tmp = $1;
      $tmp =~ s/<.>//g;
      $tmp =~ s/bloody, mangled //g;
      foreach my $limbref (@limblist) {
        if ((($limbref eq "heart" or $limbref eq "eyeballs") and
            $limb =~ /$limbref/) or $limb =~ /($limbref\s+of)/
            and !($limb =~ /of the $limbref/)) {
          if (defined $teams[$ind]{$limbref}) {
            $teams[$ind]{$limbref} = [ @{$teams[$ind]{$limbref}}, $tmp ];
          } else {
            $teams[$ind]{$limbref} = [ $tmp ];
          }
        }
      }
    }
  }
}
print "<SMALL>";
print "<TABLE border=2 cellpadding=3 cellspacing=0 width = 100%><TR>\n";

my @sets;
foreach my $ind (0..$#teams) {
  foreach my $key (@limblist) {
    my $sz;
    
    if (!defined $teams[$ind]{$key}) {
      $teams[$ind]{$key} = [ "none" ];
      $sz = 0;
    } else {
      $sz = scalar @{$teams[$ind]{$key}};
      $teams[$ind]{"str"} = $teams[$ind]{"str"}."<B>$key ($sz)</B><BR>\n";
      foreach my $list (@{$teams[$ind]{$key}}) {
        $teams[$ind]{"str"} = $teams[$ind]{"str"}.$list."<BR>\n";
      }
    }
    #if ($key =~ /tooth/){
    #  $sz /= 4; # 4 teeth to make a mob
    #}
    if (not defined $sets[$ind] or $sz < $sets[$ind]) {
      $sets[$ind] = $sz;
    }
  }
}

foreach my $ind (0..$#teams) {
  my $num = $ind + 1;
  print "<TD valign=top>";
  print "<B>TEAM $num : $sets[$ind] complete sets</B><BR><SMALL>\n";
  print $teams[$ind]{"str"}."</TD>";
}

print "</TR><TR>\n";    
foreach my $ind (0..$#teams) {
  print "<TD valign=top>\n";
  print "<B>TEAM MEMBERS</B><BR><SMALL>\n";
  print join("<BR>\n", sort @{$teams[$ind]{"members"}});
  print "</SMALL></TD>\n";
}

print  "</TR></TABLE>",$pg->end_html();


