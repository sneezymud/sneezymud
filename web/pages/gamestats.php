<!--
Game Stats
<a class="nav" href="index.php?page=home">home</a> : <a class="nav" href="index.php?page=gamestats">game stats</a>
home
-->
<?
$port = $_REQUEST['port'];
?>
<p align="center"><b>SneezyMUD</b> Stats:<br>

<br>
<br>
<!-- the update stuff forces them to reload the image, otherwise it will cache, which we don't want -->
<img src="pages/24hr.php?cols=250update=<?=(int)(time()/60)?><?if (isset($port)) echo "&port=".$port?>" height="150" width="525" border=0><br>
<img src="pages/7day.php?cols=250&update=<?=(int)(time()/60)?><?if (isset($port)) echo "&port=".$port?>" height="150" width="525" border=0><br>
<img src="pages/30day.php?cols=250&update=<?=(int)(time()/60)?><?if (isset($port)) echo "&port=".$port?>" height="150" width="525" border=0><br>
</p>
