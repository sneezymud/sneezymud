<!--
The Grand Grimoire
<a class="nav" href="index.php?page=histories">histories</a> : <a class="nav" href="index.php?page=grimoire">the grand grimoire</a> 
the grand grimoire
-->
<div align="left">
<p align="center"><b>The Grand Grimoire</b></p>

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
		<span style="width:100px;font-size:9;padding:0px;text-align:center"><a href="index.php?page=grimpage&which=<?=$grimpage?>">Volume <?=$grimpage?></a></span>
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

