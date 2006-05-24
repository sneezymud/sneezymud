<?

function runQuery($query, $process) {
         $DBNAME = "sneezy";
         $DBLOGIN = "sneezy";
         $DBPASS = "";
         $DBSERVER = "localhost";

    $link = mysql_connect($DBSERVER, $DBLOGIN, $DBPASS)
//	$link = mysql_connect()
	   or die("Couldn't ".$process." - error 1");
	mysql_select_db("sneezy")
       or die("Couldn't ".$process." - error 2");
   	$result = mysql_query ($query)
       or die("Couldn't ".$process." - error 3");

    mysql_close($link);
    return $result;
}

if (!isset($order)) $order = "priority";

?>

<!doctype html public "-//W3C//DTD HTML 4.0 //EN"> 
<html>
<body bgcolor="#000000" text="#FFFFFF" link="#808080" vlink="#808080" alink="#808080">

<p>&nbsp;</p>
<center>
<p align="center"><b><font size="6">SneezyMUD LOW Task List</font></b></p>
<div align="center">
  <center>
  <table border="1" cellpadding="3" cellspacing="0" width="600">
    <tr>
      <td width="65" align="left"><b><a href="projectstatus.php?order=id">ID</a></b></td>
      <td width="49" align="left"><b><a href="projectstatus.php?order=priority">P</a></b></td>
      <td width="146" align="left"><b><a href="projectstatus.php?order=assigned_to">Assigned To</a></b></td>
      <td width="104" align="left"><b><a href="projectstatus.php?order=status">Status</a></b></td>
      <td width="546" align="left"><b><a href="projectstatus.php?order=task">Task</a></b></td>
    </tr>
      <?
  if (!isset($member)) {
     $result = runQuery("SELECT * FROM lowtasks ORDER BY ".$order, "get tasks");
  } else {
      $result = runQuery("SELECT * FROM lowtasks where assigned_to=\"".$member."\"ORDER BY ".$order, "get tasks");
  }
  for ($count=0; $line = mysql_fetch_array($result); $count++) {
  ?>
    <tr>
      <td align="left"><?=$line["id"]?></td>
      <td align="left"><?=$line["priority"]?></td>
      <td align="left"><a href="projectstatus.php?member=<?=$line["assigned_to"]?>"><?=$line["assigned_to"]?></a></td>
      <td align="left"><?=$line["status"]?></td>
      <td align="left"><?=$line["task"]?></td>
    </tr>
    <tr> 

<?
}
?>

	<td colspan=5 align="right"><?="<i>".$count." tasks displayed.</i>"?></td>
</tr>	

</div>

</body>

</html>
