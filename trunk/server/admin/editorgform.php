
	<form name="editOrgForm" method="post" action="<?php echo $submitPage; ?>">
		<input type="hidden" name="sfdc" value="<?php echo @$form['sfdc']; ?>" />
		<table border="0">
			<tr>
				<td><b>Required Information:</b></td>
			</tr>
			<tr>
				<td><label class="required">Name:</label></td>
				<td><input type="text" name="name" value="<?php echo $form['name'] ?>" /></td>
			</tr>
			<tr>
				<td><label class="required">Seats:</label></td>
				<td><input type="text" name="seats" value="<?php echo $form['seats'] ?>" /></td>
			</tr>
			<tr>
				<?php 
global $allTiers;
if (count($allTiers) > 1){
  echo "<tr>
        <td><label class='required'>Tier:</label></td>
        <td><select name='tier' title='Tier of Service'>";  
  foreach($allTiers as $tier) {
    echo "<option value='{$tier[0]}'";
    if($form["tier"] == $tier[0]) echo " selected";
    echo ">{$tier[1]}</option>";
  }
  echo  "</select></td>
    </tr>"; 
}
?>
			<tr>
				<td><label class="required">Status:</label></td>
				<td><select name="status" title="Organization status">
						<?php 
  foreach($allOrgStatii as $status) {
    echo "<option value='{$status[0]}'";
    if($form["status"] == $status[0]) echo " selected";
    echo ">{$status[1]}</option>";
  }
?>
					</select></td>
			</tr>
			<tr>
				<td><b>Optional Information:</b></td>
			</tr>
			<tr>
				<td><label class="optional">Comment:</label></td>
				<td><textarea name="comment"><?php echo $form['comment']; ?></textarea></td>
			</tr>
			<tr>
				<td></td>
				<td><input type="submit" value="<?php echo $submitLabel; ?>"></td>
			</tr>
		</table>
	</form>
