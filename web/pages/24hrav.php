<?php
header ("Content-type: image/png");
//dl("extensions/php_gd.dll");
$im = ImageCreate (525, 150)
    or die ("Cannot Initialize new GD image stream");
$background = ImageColorAllocate ($im, 0, 0, 0);
$textcolor = ImageColorAllocate ($im, 128, 128, 128);
$highcolor = ImageColorAllocate ($im, 25, 155, 25);
$lowcolor = ImageColorAllocate ($im, 155, 25, 25);
$averagecolor = ImageColorAllocate ($im, 100, 100, 255);

$cols = $_REQUEST['cols'];
$port = $_REQUEST['port'];


function runQuery($query, $database) {
         $DBNAME = $database;
         $DBLOGIN = "www";
         $DBPASS = "";
         $DBSERVER = "localhost";

//      $conn = pg_connect("dbname=".$DBNAME." host=".$DBSERVER." user=".$DBLOGIN." password=");

        $connstr = "dbname=".$DBNAME." user=".$DBLOGIN." password=".$DBPASS;
//      echo $connstr;
      $conn = pg_connect($connstr);
        $result = pg_exec($conn, $query);

//    $link = mysql_connect($DBSERVER, $DBLOGIN, $DBPASS)
//         or die("Couldn't connect to mysql - error 1");
//      mysql_select_db($DBNAME)
//       or die("Couldn't connect to database - error 2");
//      $result = mysql_query ($query)
//       or die("Couldn't query failed - error 3");

//    mysql_close($link);
    return $result;
}


if(!isset($port)) $port = 7900;

$hour = 60 * 60;
$day = $hour * 24;
$week = 7 * $day;
$month = 30 * $day;
$year = 365 * $day;
$graphwidth = 500;
$graphheight = 100;
$offset = 25;
$offset2 = 1;

$timenow = time();
$lastday = $timenow - $day;

$days = 1;

if (!isset($cols)) $cols = 125;
$colw = $graphwidth/$cols;
$graphmax = $timenow;
$graphmin = $timenow - ( $days * $day );
$range = $graphmax - $graphmin;

$segment = $range/$cols;

$pmax = 2;

$pmin = 0;
$colwidth = 4;
$k = 0;

        $result = runQuery("select time,players from usagelogs where port=".$port." order by time desc","sneezy");


$ploth = array();
$plotl = array();
$plota = array();
$colmin = array();
$colmax = array();
$colcount = array();
$coltotal = array();

while($line = pg_fetch_array($result)) {
	$col = (($line["time"]-$lastday + $year)%$range) / $segment;
	if (!isset($colcount[$col])) {
		$colcount[$col] = 0;
		$coltotal[$col] = 0;
	}
	$colcount[$col] += 1;
	$coltotal[$col] += $line["players"];
	if (!isset($colmin[$col]) OR $colmin[$col] > $line["players"])
		$colmin[$col] = $line["players"];
	if (!isset($colmax[$col]) OR $colmax[$col] < $line["players"])
                $colmax[$col] = $line["players"];
	if (!isset($pmax) OR $pmax < $line["players"])
                $pmax = $line["players"];

}


for($count=0;$count < $cols;$count++) {
	if (isset($colcount[$count])) {
		$plota[] = $coltotal[$count] / $colcount[$count];
		$plotl[] = $colmin[$count];
		$ploth[] = $colmax[$count];
	} else {
		$plota[] = 0;
		$plotl[] = 0;
		$ploth[] = 0;	
	}
	$k = $cols - 1;
}


//imagefill ($im, 0, 0, $background);
$lasthour = -1;

if ($pmax % 2) $pmax += 1; // make it an even number so we can divide it easy

for ($j = 0; $j < $k; $j++) {


	$x1 = $j * $colw + $offset2 + 2;
	$y1 = $graphheight - (($ploth[$j] * $graphheight) / $pmax) + $offset;

        $x2 = ($j+1) * $colw + $offset2 + 2;
        $y2 = $graphheight - (($ploth[($j+1)] * $graphheight) / $pmax) + $offset;
	

	imageline( $im, $x1, $y1, $x2, $y2, $highcolor);
}

for ($j = 0; $j < $k; $j++) {

        $x1 = $j * $colw  + $offset2 + 2;
        $y1 = $graphheight - (($plotl[$j] * $graphheight) / $pmax) + $offset;

        $x2 = ($j+1) * $colw + $offset2 + 2;
        $y2 = $graphheight - (($plotl[($j+1)] * $graphheight) / $pmax) + $offset;

        imageline( $im, $x1, $y1, $x2, $y2, $lowcolor);
}

for ($j = 0; $j < $k; $j++) {

        $x1 = $j * $colw + $offset2 + 2;
        $y1 = $graphheight - (($plota[$j] * $graphheight) / $pmax) + $offset;

        $x2 = ($j+1) * $colw + $offset2 + 2;
        $y2 = $graphheight - (($plota[($j+1)] * $graphheight) / $pmax) + $offset;

        imageline( $im, $x1, $y1, $x2, $y2, $averagecolor);
        imageline( $im, $x1, $y1+1, $x2, $y2+1, $averagecolor);

        imageline( $im, $x1+1, $y1, $x2+1, $y2, $averagecolor);
        imageline( $im, $x1+1, $y1+1, $x2+1, $y2+1, $averagecolor);

}

imageline( $im, $offset2 - 1, $offset -1, $offset2 - 1, $graphheight+$offset + 2, $textcolor);
imageline( $im, $offset2 + $graphwidth, $offset - 1, $offset2 + $graphwidth, $graphheight+$offset + 2, $textcolor);
imageline( $im, $offset2, $offset -1, $offset2, $graphheight+$offset + 2, $textcolor);
imageline( $im, $offset2 + $graphwidth - 1, $offset - 1, $offset2 + $graphwidth -1, $graphheight+$offset + 2, $textcolor);

imageline( $im, $offset2 -1, $offset+$graphheight +2, $offset2 + $graphwidth, $graphheight+$offset + 2, $textcolor);
imageline( $im, $offset2 -1, $offset+$graphheight +1, $offset2 + $graphwidth, $graphheight+$offset + 1, $textcolor);

$widthhour = $graphwidth / 24.0;

$plott = array(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23, 24);


$nowtime = ($timenow % $day) / $hour;

$nowtime = $nowtime + 16;
if ($nowtime > 24) $nowtime -= 24.0; 

foreach($plott as $hour) {
	$anchor = $hour;
	$hour += $nowtime;
	if ($hour >= 24) $hour -= 24;
	imageline( $im, $graphwidth - ($hour * $widthhour) + $offset2 - 3, $offset + $graphheight - 4,  $graphwidth - ($hour * $widthhour) + $offset2 - 3, $graphheight+$offset + 2, $textcolor);
	imagestring ($im, 0, $graphwidth - ($hour * $widthhour) + $offset2 - 6, $graphheight + $offset + 4, 24 - $anchor, $textcolor);
}

imageline( $im, $graphwidth + $offset2, $offset, $graphwidth + $offset2 + 2, $offset, $textcolor);
imagestring ($im, 0, $graphwidth + $offset2 + 4, $offset - 4, $pmax, $textcolor);

imageline( $im, $graphwidth + $offset2, $offset + ($graphheight/2), $graphwidth + $offset2 + 2, $offset + ($graphheight/2), $textcolor);
imagestring ($im, 0, $graphwidth + $offset2 + 4, $offset - 4  + ($graphheight/2), ($pmax+$pmin)/2, $textcolor);

imageline( $im, $graphwidth + $offset2, $offset + $graphheight, $graphwidth + $offset2 + 2, $offset + $graphheight, $textcolor);
imagestring ($im, 0, $graphwidth + $offset2 + 4, $offset - 4  + $graphheight, $pmin, $textcolor);

imagestringup ($im, 1, $graphwidth + $offset2 +15, $offset*1.5 + $graphheight/2 +5, "players", $textcolor);
imagestring   ($im, 1, $graphwidth/2 + $offset2 - 40, $graphheight + $offset * 1.5 +2, "hour of the day (PST)", $textcolor);

imagestring   ($im, 2, $offset2,5, "SneezyMUD usage over the past 24 hours:", $textcolor);
imagestring   ($im, 1, $offset2+20,19, "average usage", $textcolor);
imagestring   ($im, 1, $offset2+20,29, "low usage", $textcolor);
imagestring   ($im, 1, $offset2+20,39, "high usage", $textcolor);

imageline( $im, $offset2+5, 22, $offset2 + 15, 22, $averagecolor);
imageline( $im, $offset2+5, 23, $offset2 + 15, 23, $averagecolor);
imageline( $im, $offset2+5, 32, $offset2 + 15, 32, $lowcolor);
imageline( $im, $offset2+5, 42, $offset2 + 15, 42, $highcolor);


ImagePng($im);
ImageDestroy($im);

?>
