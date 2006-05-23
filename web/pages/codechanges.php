<!--
Code Changes
<a class="nav" href="index.php?page=news">news</a> : <a class="nav" href="index.php?page=codechanges">code changes</a>
news
-->
<div align="left">
<p align="center"><b>The SneezyMUD News</b>: Code Changes</p>
<p>Any changes made in the code will be put here with date the change was made.  
Changes of a textual (spelling/grammar errors, etc) will not be reported.
Changes to infrastructure (improved efficiency of code base, etc) will not
be reported provided the change is thought to have no impact on game play.
Changes that merely provide extra, or reorganize information will also not
be reported (self-evident).</p>
</div>
<div align="center">
  <center>
  <table border="0" cellpadding="2" cellspacing="0" width="100%">
    <tr>
      <td width="16%" align="right" valign="top"><b>Date&nbsp;&nbsp;</b></td>
      <td width="84%" valign="top"><b>Code Change</b></td>
    </tr>

<?

if (!isset($display)) $display = 100;

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

  </table>
  </center>
</div>

