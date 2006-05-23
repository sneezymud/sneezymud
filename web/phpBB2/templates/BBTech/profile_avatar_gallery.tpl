<table border="0" cellpadding="0" cellspacing="0" width="100%">
  <tr>
   <td><img name="tlc" src="templates/BBTech/images/tlc.gif" width="10" height="15" border="0" alt=""></td> 
   <td width="100%" background="templates/BBTech/images/tm.gif"><img name="tm" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
   <td><img name="trc" src="templates/BBTech/images/trc.gif" width="10" height="15" border="0" alt=""></td>
  </tr>
  <tr>
    <td background="templates/BBTech/images/tlt.gif"><img name="tlt" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
        <td valign="top" bgcolor="#1E1E2A">
<form action="{S_PROFILE_ACTION}" method="post">
<table width="100%" cellspacing="2" cellpadding="2" border="0" align="center">
  <tr> 
	<td align="left"><span class="nav"><a href="{U_INDEX}" class="nav">{L_INDEX}</a></span></td>
  </tr>
</table>

<table border="0" cellpadding="3" cellspacing="1" width="100%" class="forumline">
	<tr> 
	  <th class="thHead" colspan="{S_COLSPAN}" height="25" valign="middle">{L_AVATAR_GALLERY}</th>
	</tr>
	<tr> 
	  <td class="catBottom" align="center" valign="middle" colspan="6" height="28"><span class="genmed">{L_CATEGORY}:&nbsp;{S_CATEGORY_SELECT}&nbsp;<input type="submit" class="liteoption" value="{L_GO}" name="avatargallery" /></span></td>
	</tr>
	<!-- BEGIN avatar_row -->
	<tr> 
	<!-- BEGIN avatar_column -->
		<td class="row1" align="center"><img src="{avatar_row.avatar_column.AVATAR_IMAGE}" alt="{avatar_row.avatar_column.AVATAR_NAME}" title="{avatar_row.avatar_column.AVATAR_NAME}" /></td>
	<!-- END avatar_column -->
	</tr>
	<tr>
	<!-- BEGIN avatar_option_column -->
		<td class="row2" align="center"><input type="radio" name="avatarselect" value="{avatar_row.avatar_option_column.S_OPTIONS_AVATAR}" /></td>
	<!-- END avatar_option_column -->
	</tr>

	<!-- END avatar_row -->
	<tr> 
	  <td class="catBottom" colspan="{S_COLSPAN}" align="center" height="28">{S_HIDDEN_FIELDS} 
		<input type="submit" name="submitavatar" value="{L_SELECT_AVATAR}" class="mainoption" />
		&nbsp;&nbsp; 
		<input type="submit" name="cancelavatar" value="{L_RETURN_PROFILE}" class="liteoption" />
	  </td>
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