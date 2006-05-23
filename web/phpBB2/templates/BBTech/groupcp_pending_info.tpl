<table border="0" cellpadding="0" cellspacing="0" width="100%">
  <tr>
   <td><img name="tlc" src="templates/BBTech/images/tlc.gif" width="10" height="15" border="0" alt=""></td> 
   <td width="100%" background="templates/BBTech/images/tm.gif"><img name="tm" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
   <td><img name="trc" src="templates/BBTech/images/trc.gif" width="10" height="15" border="0" alt=""></td>
  </tr>
  <tr>
    <td background="templates/BBTech/images/tlt.gif"><img name="tlt" src="templates/BBTech/images/spacer.gif" width="1" height="1" border="0" alt=""></td>
        <td valign="top" bgcolor="#1E1E2A">
<br clear="all" />

<table width="100%" cellpadding="4" cellspacing="1" border="0" class="forumline">
	<tr> 
	  <th class="thCornerL" height="25">{L_PM}</th>
	  <th class="thTop">{L_USERNAME}</th>
	  <th class="thTop">{L_POSTS}</th>
	  <th class="thTop">{L_FROM}</th>
	  <th class="thTop">{L_EMAIL}</th>
	  <th class="thTop">{L_WEBSITE}</th>
	  <th class="thCornerR">{L_SELECT}</th>
	</tr>
	<tr> 
	  <td class="catSides" colspan="8" height="28"><span class="cattitle">{L_PENDING_MEMBERS}</span></td>
	</tr>
	<!-- BEGIN pending_members_row -->
	<tr> 
	  <td class="{pending_members_row.ROW_CLASS}" align="center"> {pending_members_row.PM_IMG} 
	  </td>
	  <td class="{pending_members_row.ROW_CLASS}" align="center"><span class="gen"><a href="{pending_members_row.U_VIEWPROFILE}" class="gen">{pending_members_row.USERNAME}</a></span></td>
	  <td class="{pending_members_row.ROW_CLASS}" align="center"><span class="gen">{pending_members_row.POSTS}</span></td>
	  <td class="{pending_members_row.ROW_CLASS}" align="center"><span class="gen">{pending_members_row.FROM}</span></td>
	  <td class="{pending_members_row.ROW_CLASS}" align="center"><span class="gen">{pending_members_row.EMAIL_IMG}</span></td>
	  <td class="{pending_members_row.ROW_CLASS}" align="center"><span class="gen">{pending_members_row.WWW_IMG}</span></td>
	  <td class="{pending_members_row.ROW_CLASS}" align="center"><span class="gensmall"> <input type="checkbox" name="pending_members[]" value="{pending_members_row.USER_ID}" checked="checked" /></span></td>
	</tr>
	<!-- END pending_members_row -->
	<tr> 
	  <td class="cat" colspan="8" align="right"><span class="cattitle"> 
		<input type="submit" name="approve" value="{L_APPROVE_SELECTED}" class="mainoption" />
		&nbsp; 
		<input type="submit" name="deny" value="{L_DENY_SELECTED}" class="liteoption" />
		</span></td>
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