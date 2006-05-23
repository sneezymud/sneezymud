<!--
The Way of Logrus
<a class="nav" href="index.php?page=histories">histories</a> : <a class="nav" href="index.php?page=factionhistories">faction histories</a> : <a class="nav" href="index.php?page=culthist">the way of logrus</a>
the way of logrus
-->
<? if ($which == 0) $which = "";?>
<div align="left">
<p align="center"><b>The Way of Logrus</b> :
<?if ($which == "") echo "Preface/Table of Contents";?>
<?if ($which != "") echo "Chaper ".$which;?>
<br>The Historical Text of the Cult of Logrus<br>

</p>

<p align="center">
<span style="width:200px;text-align:left">
<?
if ($which > 0) {
?>
<a href="index.php?page=culthist&which=<?=$which-1?>">
Previous Chapter</a>
<?
}
?>
</span>
<span style="width:200px;text-align:right">
<?
if ($which < 5) {
?>
<a href="index.php?page=culthist&which=<?=$which+1?>">
Next Chapter</a>
<?
}
?>
</span>
</p>
<center>
<div style="text-align:left">
<?
if ($which != "") $whichbook = ".".$which;
$file = "29913".$whichbook;


//include ("/mud/prod/lib/objdata/books/".$file);

if (file_exists("/mud/prod/lib/objdata/books/".$file)) {
  $fd = fopen ("/mud/prod/lib/objdata/books/".$file, "r");
  $kill = FALSE;
  $count = 1;
  while (!feof ($fd)) {
    $buf = fgets($fd, 4096);
    $buf = str_replace("<1>", "", $buf);
    $buf = str_replace("<i>", "", $buf);
    echo $buf."<br>";
    $count++;
    }
  }

fclose ($fd);



?>
</div>

<p align="center">
<span style="width:200px;text-align:left">
<?
if ($which > 0) {
?>
<a href="index.php?page=culthist&which=<?=$which-1?>">
Previous Chapter</a>
<?
}
?>
</span>
<span style="width:200px;text-align:right">
<?
if ($which < 5) {
?>
<a href="index.php?page=culthist&which=<?=$which+1?>">
Next Chapter</a>
<?
}
?>
</span>



<br><br>
<div align="center">
Please select a chapter of The Way of Logrus to view:
  <center>
<br>
<?

$dir=opendir("/mud/prod/lib/objdata/books/"); 
$count = 0;
while ($file = readdir($dir)) { 
	$grimpage = 0;
	if (strstr($file, "29913")) {
		if (sizeof(sscanf($file, "29913.%d", $whichpage)) != 1) $whichpage = 0; 
		$count++;
		?>
		<span style="width:100px;font-size:9;padding:0px;text-align:center"><a href="index.php?page=culthist&which=<?=$whichpage?>">
<?if ($whichpage == $which) echo "<b>";?>
<? 
if ($whichpage > 0) {
?>
Chapter <?=$whichpage?>
<? 
} else {
?>
Preface
<?
}
?> 
<?if ($whichpage == $which) echo "</b>";?>

</a></span>
		<?
		if ($count % 10 == 0) echo "<br>\n\r";
	}
} 
closedir($dir); 


?>

</div>
</center>
</p>
</div>

