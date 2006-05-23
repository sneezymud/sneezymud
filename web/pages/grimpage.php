<!--
The Grand Grimoire
<a class="nav" href="index.php?page=histories">histories</a> : <a class="nav" href="index.php?page=grimoire">the grand grimoire</a> 
the grand grimoire
-->
<div align="left">
<p align="center"><b>The Grand Grimoire </b>: Volume <?=$which?></p>
<p align="center">
<span style="width:200px;text-align:left">
<?
if ($which > 1) {
?>
<a href="index.php?page=grimpage&which=<?=$which-1?>">
Previous Volume</a>
<?
}
?>
</span>
<span style="width:200px;text-align:right">
<?
if ($which < 44) {
?>
<a href="index.php?page=grimpage&which=<?=$which+1?>">
Next Volume</a>
<?
}
?>
</span>
</p>
<center>
<div style="text-align:left">
<?
if ($which < 10) $file = "160".$which;
else $file = "16".$which;

//include ("/mud/web/grimoire/".$file);

if (file_exists("/mud/web/grimoire/".$file)) {
  $fd = fopen ("/mud/web/grimoire/".$file, "r");
  $kill = FALSE;
  $count = 1;
  while (!feof ($fd)) {
    $buf = fgets($fd, 4096);
    
    echo $buf."<br>";
    $count++;
    }
  }

fclose ($fd);


?>
</div>
</center>
<center>
<span style="width:200px;text-align:left">
<?
if ($which > 1) {
?>
<a href="index.php?page=grimpage&which=<?=$which-1?>">
Previous Volume</a>
<?
}
?>
</span>
<span style="width:200px;text-align:right">
<?
if ($which < 44) {
?>
<a href="index.php?page=grimpage&which=<?=$which+1?>">
Next Volume</a>
<?
}
?>
</span>


<br>
<br>
<div align="center">
Please select a volume of the Grimoire to view:
  <center>
<br>
<?

$dir=opendir("/mud/web/grimoire/"); 
$count = 0;
while ($file = readdir($dir)) { 
	$grimpage = 0;
	if (sizeof(sscanf($file, "16%d", $grimpage)) == 1 && $grimpage) {
		$count++;
		?>
		<span style="width:100px;font-size:9px;padding:0px;text-align:center"><a href="index.php?page=grimpage&which=<?=$grimpage?>">
<?if ($grimpage == $which) echo "<b>";?>
Volume <?=$grimpage?>
<?if ($grimpage == $which) echo "</b>";?>
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

