#!/usr/local/bin/perl -wT
# For display of builder help information
# directory structure is non-dynamic - the program will require updating
#  if it is moved
#
# Maror, Oct. 29 2003
#
use strict;
use CGI;
use CGI::Carp qw(fatalsToBrowser);

my $pg = new CGI;
my %books = ( "1450" => "Redit (rooms)",
                "1451" => "Oedit (objects)",
                "1452" => "Medit (mobs)",
                "1453" => "Beginner Building Guide",
                "1454" => "Zonefile Guide",
                "1457" => "The Meaning of Time",
                "1461" => "Response Quests",
                "edit" => "edit help doc");
my @booklist;
                
foreach my $book (sort keys %books) {
  push @booklist, "<A HREF=\"http://sneezy.saw.net/builderbooks.cgi?book=$book;chap=0\">".
    "$books{$book}</A><BR>\n";
}
my $booklist = join "", @booklist;

my $book = $pg->param('book');
if (!defined $book) { $book = 0 }
my $chap = $pg->param('chap');
if (!defined $chap) { $chap = 0 }

my $untainted = 0;
if ($book != 0) {
  foreach my $key (keys %books) {
    if ($book =~ /$key/) {
      $book = $key;
      $untainted = 1;
      last;
    }
  }  
} else { $untainted = 1 }
if ($untainted == 0) { $book = 0 }
if ($chap =~ /(\d+)/) {
  $chap = $1;
} else { $chap = 0 }

my $text;
my $bookchap = $book;
if ($chap > 0) { $bookchap = "$book.$chap" }
if ($book eq '0') {
  $text = '';
} else {  
  open BOOK, "/mud/web/builders/$bookchap" 
    or die "That book entry cannot be found ($bookchap): $!";
  my @text;
  while(<BOOK>) {
    chomp;
    s/</&lt;/g;
    s/>/&gt;/g;
    push @text, "$_<BR>\n"; 
  }
  close BOOK;
  $text = join "", @text;
}
#convert spaces to preserve spacing
$text =~ s/ /&nbsp;/g;

my $prevchap;
if ($chap > 0) {
  $prevchap = $chap - 1;
} else { $prevchap = 0 }

my $nextchap = $chap + 1; 
if (open TEST, "/mud/web/builders/$book.$nextchap") {
  close TEST;
} else { $nextchap = 0 }

my $title2 = 
  $pg->a({-href=>"http://sneezy.saw.net/builderbooks.cgi?book=$book;chap=$prevchap"}, '<< prev').
  " ---- $books{$book} -- chapter $chap ---- ".
  $pg->a({-href=>"http://sneezy.saw.net/builderbooks.cgi?book=$book;chap=$nextchap"}, 'next >>');

print $pg->header(),
  $pg->start_html(-title=>'SneezyMUD Builder Guides'),
#  $pg->center($pg->h1('SneezyMUD Builder Guides')),
  $pg->table({-border=>'2', -cellpadding=>'3', -cellspacing=>'0'},
    $pg->Tr({-align=>'left', valign=>'top'},
    [
      $pg->td([$pg->h3($pg->center('Books')), 
        $pg->h3($pg->center($title2))]),
      $pg->td([ $booklist, $pg->tt($text) ])
    ]
    )
  ),
  $pg->end_html();



