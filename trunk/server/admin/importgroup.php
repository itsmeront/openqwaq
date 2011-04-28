<?php
include('propsapi.php');

function makeOrgsDropDown() {
  $matches = findOrgsByMatch('', 100, 1);
  $count = $matches["count"];
  $total = $matches["total"];
  echo "<select name=\"orgid\">";
  for($index=0; $index < $count; $index+=1) {
    $form = $matches[$index];
    echo "<option value=\"{$form['id']}\">{$form['name']}</option>\n";
  }
  echo "</select>";
}
?>

<?php include('header.php'); ?>
<body id="find_group" class="groups">
<?php make_navbar('Groups'); ?>
<div id="body" class="wrap">

<fieldset><legend>Import from Active Directory</legend>
<form name="importadform" method="post" action="adimport.php">
<table style="border: 0px none ;">
<tr>
  <td style="background: white ;">OpenQwaq Organization</td>
  <td style="background: white ;"><?php makeOrgsDropDown(); ?></td>
</tr>
<tr>
  <td style="background: white ;">Active Directory Group</td>
  <td style="background: white ;"><input name="group" type="text" size="40"></td>
</tr>
<tr>
  <td style="background: white ;">Default Role</td>
  <td style="background: white ;"><select name="role" title="User Role">
    <option value="Admin">Administrator</option>
    <option value="User" selected>Member</option>
    <option value="Guest">Visitor</option>
    </select>
  </td>
</tr>

<tr>
  <td style="background: white;">User Name (for import)</td>
  <td style="background: white;"><input name="user" type="text" size="40"></td>
</tr>

<tr>
  <td style="background: white;">Password (for import)</td>
  <td style="background: white;"><input name="password" type="password" size="40"></td>
</tr>

<tr> 
  <td style="background: white ;"><input name="submit" type="submit" value="Import Group"></td>
</tr>
</table>
</fieldset>

</div>
<?php include('footer.php') ?>
</body>
</html>
