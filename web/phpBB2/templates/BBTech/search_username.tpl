
<script language="javascript" type="text/javascript">
<!--
function refresh_username(selected_username)
{
	opener.document.forms['post'].username.value = selected_username;
	opener.focus();
	window.close();
}
//-->
</script>

<form method="post" name="search" action="{S_SEARCH_ACTION}">
<table border="0" cellpadding="0" cellspacing="0" width="100%">
  <tr>
   <td><img name="tlc" src="templates/BBTech/images/tlc.gif" width="10" height="15" border="0" alt=""></td> 
   <td width="100%" background="templates/BBTech/images/tm.gif"><img name="tm" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
   <td><img name="trc" src="templates/BBTech/images/trc.gif" width="10" height="15" border="0" alt=""></td>
  </tr>
  <tr>
    <td background="templates/BBTech/images/tlt.gif"><img name="tlt" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
        <td valign="top" bgcolor="#1E1E2A">
<table width="100%" border="0" cellspacing="0" cellpadding="10">
	<tr>
		<td><table width="100%" class="forumline" cellpadding="4" cellspacing="1" border="0">
			<tr> 
				<th class="thHead" height="25">{L_SEARCH_USERNAME}</th>
			</tr>
			<tr> 
				<td valign="top" class="row1"><span class="genmed"><br /><input type="text" name="search_username" value="{USERNAME}" class="post" />&nbsp; <input type="submit" name="search" value="{L_SEARCH}" class="liteoption" /></span><br /><span class="gensmall">{L_SEARCH_EXPLAIN}</span><br />
				<!-- BEGIN switch_select_name -->
				<span class="genmed">{L_UPDATE_USERNAME}<br /><select name="username_list">{S_USERNAME_OPTIONS}</select>&nbsp; <input type="submit" class="liteoption" onClick="refresh_username(this.form.username_list.options[this.form.username_list.selectedIndex].value);return false;" name="use" value="{L_SELECT}" /></span><br />
				<!-- END switch_select_name -->
				<br /><span class="genmed"><a href="javascript:window.close();" class="genmed">{L_CLOSE_WINDOW}</a></span></td>
			</tr>
		</table></td>
	</tr>
</table>
</form>
</td>
    <td background="templates/BBTech/images/trt.gif"><img name="trt" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
  </tr>
  <tr>
   <td><img name="blc" src="templates/BBTech/images/blc.gif" width="10" height="12" border="0" alt=""></td>
    <td background="templates/BBTech/images/btm.gif"><img name="btm" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
   <td><img name="brc" src="templates/BBTech/images/brc.gif" width="10" height="12" border="0" alt=""></td>
  </tr></table>