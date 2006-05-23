#!/usr/bin/perl

use XML::Parser::Lite::Tree;
use Flickr::API;
use CGI;
$cgi=new CGI;


$photoset_id=72057594105129945;
$cols=5;

###


if(!$cgi->param('photoset_id')){
  print "Content-type: text/html\n\n";
  print "<html>";
  print "<form method=post action=flickr_squares.cgi>\n";
  print "photoset_id: <input type=text name=photoset_id><br>\n";
  print "columns: <input type=text name=cols><br>\n";
  print "exclude: <input type=text name=exclude><br>\n";
  print "<input type=submit>";
  print "</form>";
  print "photoset_id is the number of the photoset you want to get thumbnails for.  If you have a url like http://www.flickr.com/photos/travis_nelson/sets/72057594105129945/, then 72057594105129945 is your photoset_id<p>\n";
  print "columns is the number of columns you want the thumbnails in<p>\n";
  print "exclude is a comma seperated list of photo ids to exclude (eg, 127357762,127356252).  If you have a photo page url like http://www.flickr.com/photos/travis_nelson/127357762/, then 127357762 is the photo id for that photo.<p>\n";

  print "</html>";
  exit;
}

print "Content-type: text/plain\n\n";

$photoset_id=$cgi->param('photoset_id');
$cols=$cgi->param('cols')||5;
foreach(split(/,/, $cgi->param('exclude'))){
  $_=~s/\s//g;
  $exclude{$_}=1;
}

my $api = new Flickr::API({'key'    => 'ff30e5e801d7b21a10aadc2b0c81c74b',
			   'secret' => '17f3f16c3d9370ab'});


my $response = $api->execute_method('flickr.photosets.getPhotos', {
  'photoset_id'=>$photoset_id
    });
  

#print $response->{success};

my $tree=$response->{tree};

$ref=$tree->{children};
$photoset=@$ref[1];

$ref=$photoset->{children};

$count=0;

foreach $photo (@$ref){
  if($photo->{attributes}->{title}){
    $server_id=$photo->{attributes}->{server};
    $photo_id=$photo->{attributes}->{id};
    $secret=$photo->{attributes}->{secret};
    $title=$photo->{attributes}->{title};

    if($exclude{$photo_id}){
      next;
    }

    my $resp = $api->execute_method('flickr.photos.getInfo', {
      'photo_id'=>$photo_id
	});
    
    $url=${${${${$resp->{tree}->{children}}[1]->{children}}[19]->{children}}[1]->{children}}[0]->{content};
  
    if($count >= $cols){
      print "\n";
      $count=0;
    }

    print "<a href=\"$url\" title=\"Photo Sharing\"><img border=0 src=\"http://static.flickr.com/$server_id/$photo_id\_$secret\_s.jpg\" width=\"75\" height=\"75\" alt=\"$title\" /></a> ";
    ++$count;
  }
}
