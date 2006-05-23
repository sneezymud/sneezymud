<!--
Online Help Files
<a class="nav" href="index.php?page=resources">resources</a> : <a class="nav" href="index.php?page=help">online help files</a> 
online help files
-->
<?

$keyword = $_REQUEST['keyword'];
?>
<div align="left">
<p align="center"><b>SneezyMUD</b> Online Help Files:

<div style="text-align:center; font-size:9">Skills and spells displayed in <b>bold</b></div>
</p>

<div align="center">
<FORM action="index.php?page=help" method="post">
Search the online help files:<br>

        <input type="text" name="keyword" value="<?=$keyword?>" size=25>
	<input type=SUBMIT value="Search">
</form>
</div>
<?

if (isset($keyword) AND $keyword != "") {
$listing = array();

$dir=opendir("/mud/prod/lib/help/");
while ($file = readdir($dir)) {
         if (strstr($file, $keyword) AND !is_dir($file) AND !(strstr($file,"_") OR strstr($file,".") OR strstr($file,"~"))) {
                $count++;
//              echo "<a href=\"index.php?page=help&which=".$file."\">".$file."</a>, ";
                $listing[$file] = $file;
        }
}
closedir($dir);
$dir=opendir("/mud/prod/lib/help/_skills");
while ($file = readdir($dir)) {
         if (strstr($file, $keyword) AND !is_dir($file) AND !(strstr($file,"_") OR strstr($file,".") OR strstr($file,"~"))) {
                $count++;
//              echo "<a href=\"index.php?page=help&which=".$file."\">".$file."</a>, ";
                $listing["skill:".$file] = $file;
        }
}
closedir($dir);
$dir=opendir("/mud/prod/lib/help/_spells");
while ($file = readdir($dir)) {
        if (strstr($file, $keyword) AND !is_dir($file) AND !(strstr($file,"_") OR strstr($file,".") OR strstr($file,"~"))) {
                $count++;
//              echo "<a href=\"index.php?page=help&which=".$file."\">".$file."</a>, ";
                $listing["spell:".$file] = $file;
        }
}
closedir($dir);
echo "<center>";
if ($count == 0) {
	echo "Your search returned no results - please try again.<br></center>\n\r";
} else if ($count == 1) {
	echo "Your search returned 1 result:<br></center>\n\r";
	while (list ($key, $val) = each ($listing)) {
		$which = $key;
	}
} else {
	echo "Your search returned ".$count." results, please choose one:<br></center>\n\r";
	asort($listing);
while (list ($key, $val) = each ($listing)) {

        if (strstr($key, "skill") OR strstr($key, "spell")) echo "<b>";
        echo "<a href=\"index.php?page=help&which=".$key."\">".$val."</a><br>";
        if (strstr($key, "skill") OR strstr($key, "spell")) echo "</b>";


    }
}
	

}

if (isset($which) AND $which != "") {

	$bits = explode(":", $which);
	if (sizeof($bits) == 1) {
		$head = strtoupper($bits[0]);
		$file = $bits[0];
	} else {
		$head = strtoupper($bits[0]."s: ".$bits[1]);
		$file = "_".$bits[0]."s/".$bits[1];
	}

echo "<b>".$head."</b><br>";	
if (file_exists("/mud/prod/lib/help/".$file)) {
  $fd = fopen ("/mud/prod/lib/help/".$file, "r");
  $kill = FALSE;
  $count = 1;
  while (!feof ($fd)) {
    $buf = fgets($fd, 4096);
	
    $buf = str_replace("<h>", "SneezyMUD 5.2", $buf);

    $buf = str_replace("<", "&lt;", $buf);
    $buf = str_replace(">", "&gt;", $buf);
    if (strstr(strtolower($buf), "see also") OR strstr(strtolower($buf), "related topics")) {
	$stuff = explode(":", $buf);
	$list = explode(",", $stuff[1]);
	$size = sizeof($list);
	echo $stuff[0].": ";
	$count2 = 1;
	while (list ($key, $val) = each ($list)) {
		echo "<a href=\"index.php?page=help&keyword=".strtolower(trim($val))."\">".$val."</a>";
		if ($count2 < $size) echo ", ";
		$count2 += 1;
	}
	echo "<br>";
    } else {
    	echo $buf."<br>";
    }
    $count++;
    
  }

fclose ($fd);
}
}
$listing = array();

$dir=opendir("/mud/prod/lib/help/"); 
$count = 0;
while ($file = readdir($dir)) { 
	$grimpage = 0;
	if (!is_dir($file) AND !(strstr($file,"_") OR strstr($file,".") OR strstr($file,"~"))) {
		$count++;
//		echo "<a href=\"index.php?page=help&which=".$file."\">".$file."</a>, ";
		$listing[$file] = $file;
	}
} 
closedir($dir); 
$dir=opendir("/mud/prod/lib/help/_skills");
$count = 0;
while ($file = readdir($dir)) {
        $grimpage = 0;
        if (!is_dir($file) AND !(strstr($file,"_") OR strstr($file,".") OR strstr($file,"~"))) {
                $count++;
//              echo "<a href=\"index.php?page=help&which=".$file."\">".$file."</a>, ";
                $listing["skill:".$file] = $file;
        }
}
closedir($dir);
$dir=opendir("/mud/prod/lib/help/_spells");
$count = 0;
while ($file = readdir($dir)) {
        $grimpage = 0;
        if (!is_dir($file) AND !(strstr($file,"_") OR strstr($file,".") OR strstr($file,"~"))) {
                $count++;
//              echo "<a href=\"index.php?page=help&which=".$file."\">".$file."</a>, ";
                $listing["spell:".$file] = $file;
        }
}
closedir($dir);

?>
<center>
<hr>
Alphabetical Listing:<br>
<?
$alpha = $_REQUEST['alpha'];

asort($listing);
for($char = 0; $char <= 25; $char += 1) {
	$ch = chr(97+$char);
	echo "<a href=\"index.php?page=help&alpha=".$ch."\">";
	if ($ch == $alpha) echo "<b>";
	echo $ch;
	if ($ch == $alpha) echo "</b>";	
	echo "</a> ";
}
echo "<hr></center>";
$firstchar = "";
while (list ($key, $val) = each ($listing)) {
    if ($firstchar != $val[0]) {
	$firstchar = $val[0];
    } else if ($alpha == $firstchar) {
	echo ", ";
    }
    if ($alpha == $firstchar) {
	if (strstr($key, "skill") OR strstr($key, "spell")) echo "<b>";
	echo "<a href=\"index.php?page=help&which=".$key."\">".$val."</a>";
	if (strstr($key, "skill") OR strstr($key, "spell")) echo "</b>";


    }
}


?>

</div>
</center>
</p>
</div>

