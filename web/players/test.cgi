#!/usr/local/bin/perl -w
# for listing limbs for limb quest with cagrane
use strict;
use CGI;
use CGI::Carp qw(fatalsToBrowser);

my $pg = new CGI;
print $pg->header(),  $pg->start_html(-title=>'Limb Fest June 2004');
print "Disabled<BR>\n";
print $pg->end_html();

