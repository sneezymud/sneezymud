<!--
Who's Playing?
<a class="nav" href="index.php?page=home">home</a> : <a class="nav" href="index.php?page=whosplaying">who's playing?</a>
home
-->

<p align="center">Who's playing <B>SneezyMUD</b> right now?</p>
<p align="left">The following players are currently logged onto SneezyMUD:</p>
<center>
<?
	if (!isset($port)) $port = 7900;

$result = runQuery("select * from wholist where port=".$port." order by name", "sneezy");

function parseTitle($name, $title) {
   $title = str_replace(" ", "&nbsp;", $title);
   $title = str_replace("<n>", $name, $title);
   $title = str_replace("<N>", $name, $title);
   $title = str_replace("<r>", "</span><span style=\"color:red\">", $title);
   $title = str_replace("<R>", "</span><span style=\"color:red;font-weight:bold\">", $title);
   $title = str_replace("<b>", "</span><span style=\"color:blue\">", $title);
   $title = str_replace("<B>", "</span><span style=\"color:blue;font-weight:bold\">", $title);
   $title = str_replace("<g>", "</span><span style=\"color:green\">", $title);
   $title = str_replace("<G>", "</span><span style=\"color:green;font-weight:bold\">", $title);
   $title = str_replace("<c>", "</span><span style=\"color:cyan\">", $title);
   $title = str_replace("<C>", "</span><span style=\"color:cyan;font-weight:bold\">", $title);
   $title = str_replace("<p>", "</span><span style=\"color:purple\">", $title);
   $title = str_replace("<P>", "</span><span style=\"color:purple;font-weight:bold\">", $title);
   $title = str_replace("<o>", "</span><span style=\"color:orange\">", $title);
   $title = str_replace("<O>", "</span><span style=\"color:orange;font-weight:bold\">", $title);
   $title = str_replace("<y>", "</span><span style=\"color:yellow\">", $title);
   $title = str_replace("<Y>", "</span><span style=\"color:yellow;font-weight:bold\">", $title);
   $title = str_replace("<k>", "</span><span style=\"color:gray\">", $title);
   $title = str_replace("<K>", "</span><span style=\"color:gray;font-weight:bold\">", $title);
   $title = str_replace("<w>", "</span><span style=\"color:white\">", $title);
   $title = str_replace("<W>", "</span><span style=\"color:white;font-weight:bold\">", $title);
   $title = str_replace("<Z>", "</span><span>", $title);
   $title = str_replace("<z>", "</span><span>", $title);
   $title = str_replace("<1>", "</span><span>", $title);
 
return $title;
}

?>
<table border="0" cellpadding="0" cellspacing="0" width="90%">
<?
$count=0;
while ($line = mysql_fetch_row($result)){
$count++;
  ?>
    <tr>
      <td align="left"><span><font face="courier"><?=parseTitle($line[0],$line[1])?></font></span></td>
    </tr>
<?
}
?>
</center>
<tr><td class="footer" align="right"><i><?=$count?> players displayed</i></td></tr></table>
<p align="left">The list updates every five seconds, so changes may not take place instantaneously.</p>    




