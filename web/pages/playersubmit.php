<!--
Player Bio Submission form:
<a class="nav" href="index.php?page=players">players</a> : <a class="nav" href="index.php?page=playersubmit">bio submission form</a>
player bio submission form
-->

<div align="left">
<center><b>SneezyMUD</b> Player Bio Submission Form:</center>

<?
if (("" != $_POST['character']) AND ("" != $_POST['bio'])) {
	
	$result = runQuery("select id from players order by id desc", "sneezyglobal");
	if ($line = pg_fetch_array($result))
		$id = $line['id'] + 1;
	else
		$id = 0;

	runQuery("insert into players (id, character, title, picture, smallpic, bio, realname, email, webpage, immortal, approved) VALUES (".$id.", '".addslashes($_POST['character'])."' , '', '', '', '".addslashes($_POST['bio'])."', '".addslashes($_POST['realname'])."', '".addslashes($_POST['email'])."', '".addslashes($_POST['webpage'])."', FALSE, FALSE)", "sneezyglobal");


echo "Thank you for your submission.  Your player biography will be checked and made public as soon as possible.";

} else {

?>
<form method="POST" action="index.php?page=playersubmit">
  <div align="center">
    <center>
    <table border="0" cellpadding="0" cellspacing="0" width="100%">
      <tr>
        <td width="50%"><B>Character Name(s):</b> <br>
          <span style="font-size:9px;">list primary character first</span></td>
        <td width="50%" valign="top"><input type="text" name="character" size="25"></td>
      </tr>
      <tr>
        <td width="50%"><b>Real Name: </b><br>
          <span style="font-size:9px;">optional</span></td>
        <td width="50%" valign="top"><input type="text" name="realname" size="25"></td>
      </tr>
      <tr>
        <td width="50%"><b>Email: </b><br>
          <span style="font-size:9px;">optional</span></td>
        <td width="50%" valign="top"><input type="text" name="email" size="25"></td>
      </tr>
      <tr>
        <td width="50%"><b>Webpage: </b><br>
          <span style="font-size:9px;">optional</span></td>
        <td width="50%" valign="top"><input type="text" name="webpage" size="25"></td>
      </tr>
    </table>
    </center>
  </div>
  <p><b>Bio/About You:</b><br>
  <span style="font-size:9px;">may contain HTML: links to images, formatting, etc.</span><br>
  <textarea rows="11" name="bio" cols="60"></textarea><br>
  <br>
  Submissions will be checked for acceptable content before being made public on
  the website.&nbsp; No flaming of other groups or individuals will be
  tolerated. </p>
  <p align="center"><input type="submit" value="Submit" name="B1"></p>
</form>
<?
}
?>
</div>

