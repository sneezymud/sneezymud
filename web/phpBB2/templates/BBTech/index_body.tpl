<table border="0" cellpadding="0" cellspacing="0" width="100%">
  <tr>
   <td><img name="tlc" src="templates/BBTech/images/tlc.gif" width="10" height="15" border="0" alt=""></td> 
   <td width="100%" background="templates/BBTech/images/tm.gif"><img name="tm" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
   <td><img name="trc" src="templates/BBTech/images/trc.gif" width="10" height="15" border="0" alt=""></td>
  </tr>
  <tr>
    <td background="templates/BBTech/images/tlt.gif"><img name="tlt" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
        <td valign="top" bgcolor="#1E1E2A">
<table width="100%" cellspacing="0" cellpadding="2" border="0" align="center">
  <tr> 
	<td align="left" valign="bottom"><span class="gensmall">
	<!-- BEGIN switch_user_logged_in -->
	{LAST_VISIT_DATE}<br />
	<!-- END switch_user_logged_in -->
	{CURRENT_TIME}<br /></span><span class="nav"><a href="{U_INDEX}" class="nav">{L_INDEX}</a></span></td>
	<td align="right" valign="bottom" class="gensmall">
		<!-- BEGIN switch_user_logged_in -->
		<a href="{U_SEARCH_NEW}" class="gensmall">{L_SEARCH_NEW}</a><br /><a href="{U_SEARCH_SELF}" class="gensmall">{L_SEARCH_SELF}</a><br />
		<!-- END switch_user_logged_in -->
		<a href="{U_SEARCH_UNANSWERED}" class="gensmall">{L_SEARCH_UNANSWERED}</a></td>
  </tr>
</table>
</td>
    <td background="templates/BBTech/images/trt.gif"><img name="trt" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
  </tr>
  <tr>
   <td><img name="blc" src="templates/BBTech/images/blc.gif" width="10" height="12" border="0" alt=""></td>
    <td background="templates/BBTech/images/btm.gif"><img name="btm" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
   <td><img name="brc" src="templates/BBTech/images/brc.gif" width="10" height="12" border="0" alt=""></td>
  </tr></table>
  <table border="0" cellpadding="0" cellspacing="0" width="100%">
  <tr>
   <td><img name="tlc" src="templates/BBTech/images/tlc.gif" width="10" height="15" border="0" alt=""></td> 
   <td width="100%" background="templates/BBTech/images/tm.gif"><img name="tm" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
   <td><img name="trc" src="templates/BBTech/images/trc.gif" width="10" height="15" border="0" alt=""></td>
  </tr>
  <tr>
    <td background="templates/BBTech/images/tlt.gif"><img name="tlt" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
        <td valign="top" bgcolor="#1E1E2A">
<table width="100%" cellpadding="2" cellspacing="1" border=0>
  <tr> 
	<th colspan="2" class="1" height="25" nowrap="nowrap">&nbsp;{L_FORUM}&nbsp;</th>
	<th width="50" class="1" nowrap="nowrap">&nbsp;{L_TOPICS}&nbsp;</th>
	<th width="50" class="1" nowrap="nowrap">&nbsp;{L_POSTS}&nbsp;</th>
	<th class="1" nowrap="nowrap">&nbsp;{L_LASTPOST}&nbsp;</th>
  </tr>
  <!-- BEGIN catrow -->
  <tr> 
	<td class="rowpic" colspan="2" height="28"><span class="cattitle"><a href="{catrow.U_VIEWCAT}" class="cattitle">{catrow.CAT_DESC}</a></span></td>
	<td class="rowpic" colspan="3" align="right">&nbsp;</td>
  </tr>
  <!-- BEGIN forumrow -->
  <tr> 
	<td class="row1" align="center" valign="middle" height="50"><img src="{catrow.forumrow.FORUM_FOLDER_IMG}" alt="{catrow.forumrow.L_FORUM_FOLDER_ALT}" title="{catrow.forumrow.L_FORUM_FOLDER_ALT}" /></td>
	<td class="row1" width="100%" height="50"><span class="forumlink"> <a href="{catrow.forumrow.U_VIEWFORUM}" class="forumlink">{catrow.forumrow.FORUM_NAME}</a><br />
	  </span> <span class="genmed">{catrow.forumrow.FORUM_DESC}<br />
	  </span><span class="gensmall">{catrow.forumrow.L_MODERATOR} {catrow.forumrow.MODERATORS}</span></td>
	<td class="row2" align="center" valign="middle" height="50"><span class="gensmall">{catrow.forumrow.TOPICS}</span></td>
	<td class="row2" align="center" valign="middle" height="50"><span class="gensmall">{catrow.forumrow.POSTS}</span></td>
	<td class="row2" align="center" valign="middle" height="50" nowrap="nowrap"> <span class="gensmall">{catrow.forumrow.LAST_POST}</span></td>
  </tr>
  <!-- END forumrow -->
  <!-- END catrow -->
</table>
</td>
    <td background="templates/BBTech/images/trt.gif"><img name="trt" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
  </tr>
  <tr>
   <td><img name="blc" src="templates/BBTech/images/blc.gif" width="10" height="12" border="0" alt=""></td>
    <td background="templates/BBTech/images/btm.gif"><img name="btm" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
   <td><img name="brc" src="templates/BBTech/images/brc.gif" width="10" height="12" border="0" alt=""></td>
  </tr></table>
<table width="100%" cellspacing="0" border="0" align="center" cellpadding="2">
  <tr> 
	<td align="left"><span class="gensmall"><a href="{U_MARK_READ}" class="gensmall">{L_MARK_FORUMS_READ}</a></span></td>
	<td align="right"><span class="gensmall">{S_TIMEZONE}</span></td>
  </tr>
</table>
<table border="0" cellpadding="0" cellspacing="0" width="100%">
  <tr>
   <td><img name="tlc" src="templates/BBTech/images/tlc.gif" width="10" height="15" border="0" alt=""></td> 
   <td width="100%" background="templates/BBTech/images/tm.gif"><img name="tm" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
   <td><img name="trc" src="templates/BBTech/images/trc.gif" width="10" height="15" border="0" alt=""></td>
  </tr>
  <tr>
    <td background="templates/BBTech/images/tlt.gif"><img name="tlt" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
        <td valign="top" bgcolor="#1E1E2A">
<table width="100%" cellpadding="3" cellspacing="1" border="0" class="forumline">
  <tr> 
	<td class="rowpic" colspan="2" height="28"><span class="cattitle"><a href="{U_VIEWONLINE}" class="cattitle">{L_WHO_IS_ONLINE}</a></span></td>
  </tr>
  <tr> 
	<td class="row1" align="center" valign="middle" rowspan="2"><img src="templates/BBTech/images/whosonline.gif" alt="{L_WHO_IS_ONLINE}" /></td>
	<td class="row1" align="left" width="100%"><span class="gensmall">{TOTAL_POSTS}<br />{TOTAL_USERS}<br />{NEWEST_USER}</span>
	</td>
  </tr>
  <tr> 
	<td class="row1" align="left"><span class="gensmall">{TOTAL_USERS_ONLINE} &nbsp; [ {L_WHOSONLINE_ADMIN} ] &nbsp; [ {L_WHOSONLINE_MOD} ]<br />{RECORD_USERS}<br />{LOGGED_IN_USER_LIST}</span></td>
  </tr>
</table>

<table width="100%" cellpadding="1" cellspacing="1" border="0">
<tr>
	<td align="left" valign="top"><span class="gensmall">{L_ONLINE_EXPLAIN}</span></td>
</tr>
</table>

<!-- BEGIN switch_user_logged_out -->
<form method="post" action="{S_LOGIN_ACTION}">
  <table width="100%" cellpadding="3" cellspacing="1" border="0" class="forumline">
	<tr> 
	  <td class="catHead" height="28"><a name="login"></a><span class="cattitle">{L_LOGIN_LOGOUT}</span></td>
	</tr>
	<tr> 
	  <td class="row1" align="center" valign="middle" height="28"><span class="gensmall">{L_USERNAME}: 
		<input class="post" type="text" name="username" size="10" />
		&nbsp;&nbsp;&nbsp;{L_PASSWORD}: 
		<input class="post" type="password" name="password" size="10" maxlength="32" />
		&nbsp;&nbsp; &nbsp;&nbsp;{L_AUTO_LOGIN} 
		<input class="text" type="checkbox" name="autologin" />
		&nbsp;&nbsp;&nbsp; 
		<input type="submit" class="mainoption" name="login" value="{L_LOGIN}" />
		</span> </td>
	</tr>
  </table>
</form>
<!-- END switch_user_logged_out -->

<br clear="all" />

<table cellspacing="3" border="0" align="center" width=100% cellpadding="0">
  <tr> 
	<td width="33.9%" align="center"><img src="templates/BBTech/images/folder_new.gif" alt="{L_NEW_POSTS}"/>&nbsp;&nbsp;<span class="gensmall">{L_NEW_POSTS}</span></td>
	
	<td width="33.9%" align="center"><img src="templates/BBTech/images/folder.gif" alt="{L_NO_NEW_POSTS}" />&nbsp;&nbsp;<span class="gensmall">{L_NO_NEW_POSTS}</span></td>
	
	<td width="33.9%" align="center"><img src="templates/BBTech/images/folder_lock.gif" alt="{L_FORUM_LOCKED}" />&nbsp;&nbsp;<span class="gensmall">{L_FORUM_LOCKED}</span></td>
  </tr>
</table>
</td>
    <td background="templates/BBTech/images/trt.gif"><img name="trt" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
  </tr>
  <tr>
   <td><img name="blc" src="templates/BBTech/images/blc.gif" width="10" height="12" border="0" alt=""></td>
    <td background="templates/BBTech/images/btm.gif"><img name="btm" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
   <td><img name="brc" src="templates/BBTech/images/brc.gif" width="10" height="12" border="0" alt=""></td>
  </tr></table>