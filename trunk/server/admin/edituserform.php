<form name="editUserForm" method="post" onsubmit="return validateForm()" action="<?php echo $submitPage; ?>">
  <input type="hidden" name="sfdc" value="<?php echo $form['sfdc'] ?>" />
  <input type="hidden" name="userid" value="<?php echo @$_GET['id'] ?>" />
  <table border="0">
    <tr><td></td></tr>
    <tr><td><b>Required Information:</b></td></tr>
    <tr>
      <td><label class="required">User ID (login):</label></td>
      <td><input type="text" name="name" value="<?php echo $form['name'] ?>" /> </td>
    </tr>
    <tr>
      <td><label class="required">Password:</label></td>
      <td><input type="password" name="password" value="<?php echo $form['password'] ?>" /> </td>
    </tr>
    <tr>
      <td><label class="required">Repeat password:</label></td>
      <td><input type="password" name="password2" value="<?php echo $form['password'] ?>" /> </td>
    </tr>
    <tr>
      <td><label class="required">Email:</label></td>
      <td><input type="text" name="email" value="<?php echo $form['email'] ?>" /> </td>
    </tr>
<!--
    <tr>
      <td><label class="required">Status:</label></td>
      <td><select name="status" title="User Status">
            <option value="ActiveUser" selected>Active</option>
	    <option value="InvitedUser">Invited</option>
	    <option value="InActiveUser">Inactive</option>
	  </select></td>
    </tr>
-->
    <tr><td><b>Optional Information:</b></td></tr>
    <tr>
      <td><label class="optional">First Name:</label></td>
      <td><input type="text" name="firstName" value="<?php echo $form['firstName'] ?>" /> </td>
    </tr>
    <tr>
      <td><label class="optional">Last Name:</label></td>
      <td><input type="text" name="lastName" value="<?php echo $form['lastName'] ?>" /> </td>
    </tr>
    <tr>
      <td><label class="optional">Company:</label></td>
      <td><input type="text" name="company" value="<?php echo $form['company'] ?>" /> </td>
    </tr>
    <tr>
      <td><label class="optional">Comment:</label></td>
      <td><textarea name="comment"><?php echo $form['comment'] ?></textarea></td>
    </tr>
    <tr>
      <td></td>
      <td><input name="submit" type="submit" value="<?php echo $submitLabel ?>"></td>
    </tr>
  </table>
  <table border="0">
    <tr><td></td></tr>
    <tr><td><b>Send Email:</b></td></tr>
    <tr> 
     <td><input name="submit" type="submit" value="Send Welcome Email"></td>
     <td><input name="submit" type="submit" value="Send Account Reminder"></td>
    </tr>
   </table>
 </form>
      