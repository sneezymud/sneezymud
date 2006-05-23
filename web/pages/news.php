<!--
News
<a class="nav" href="index.php?page=news">news</a>
news
-->
<div align="left">
<p align="center">The <b>SneezyMUD</b> News</p>
<p>This section contains a summary of recent announcements concerning SneezyMUD, a listing of code changes, and a listing world changes.</p>
</div>
<div align="center">
  <center>
<p>Recent Code Changes:
  <table border="0" cellpadding="2" cellspacing="0" width="100%">
    <tr>
      <td width="16%" align="right" valign="top"><b>Date&nbsp;&nbsp;</b></td>
      <td width="84%" valign="top"><b>Code Change</b></td>
    </tr>

<?

if (!isset($display)) $display = 3;

if (file_exists("/mud/prod/lib/txt/news")) {
  $fd = fopen ("/mud/prod/lib/txt/news", "r");
  $kill = FALSE;
  $count = 1;
  while (!feof ($fd) AND $count <= $display) {
    $buf = rtrim(fgets($fd, 4096));
    $d1 = 0;
    $d2 = 0;
    $d3 = 0;
    $str = "";
    $vars = sscanf($buf, "%d-%d-%d : %[^[]]", $d1, $d2, $d3, $str);
    if ($d1 != 0 OR $d2 != 0 OR $d3 != 0) {
  ?>
 
 <tr>
    <td width="16%" align="right" valign="top"><b><?=$d1."-".$d2."-".sprintf("%02d",$d3)?>&nbsp;&nbsp;</b></td>
    
    <td>
    <? 
    while (!feof ($fd) AND $str != "") {
      echo rtrim($str)." ";
      $str = rtrim(fgets($fd, 4096));
    }
    ?>
    <br><br></td></tr>
<?
    $count++;
    }
  }

fclose ($fd);
}
?>
<tr><td colspan="2" align="right"><div class="footer">A full listing can be found in the <a class="nav" href="index.php?page=codechanges">code changes</a> section</div></td></tr>
  </table></p>
  </center>
</div>

