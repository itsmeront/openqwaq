<form name="viewUserForm" method="post" action="<?php echo $submitPage; ?>">
  <input type="hidden" name="id" value="<?php echo $form['id'] ?>">
  <table border="0">
    <tr>
      <td><label class="required">User ID (login):</label></td>
      <td><?php echo $form['name'] ?></td>
    </tr>
    <tr>
      <td><label class="required">Email:</label></td>
      <td><?php echo $form['email'] ?></td>
    </tr>
    <tr>
      <td><label class="optional">First Name:</label></td>
      <td><?php echo $form['firstName'] ?></td>
    </tr>
    <tr>
      <td><label class="optional">Last Name:</label></td>
      <td><?php echo $form['lastName'] ?></td>
    </tr>
    <tr>
      <td><label class="optional">Company:</label></td>
      <td><?php echo $form['company'] ?></td>
    </tr>
    <tr>
      <td></td>
      <td><input type="submit" value="<?php echo $submitLabel ?>"></td>
    </tr>
  </table>
</form>
