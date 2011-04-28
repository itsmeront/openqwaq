<form name="viewAccountForm" method="post" action="<?php echo $submitPage; ?>">
  <input type="hidden" name="id" value="<?php echo $form['id'] ?>">
  <table border="0">
    <tr>
      <td><label class="required">User ID (login):</label></td>
      <td><?php echo $form['userName'] ?></td>
    </tr>
    <tr>
      <td><label class="required">Organization:</label></td>
      <td><?php echo $form['orgName'] ?></td>
    </tr>
    <tr>
      <td><label class="required">Role:</label></td>
      <td><?php echo $form['role'] ?></td>
    </tr>
    <tr>
      <td></td>
      <td><input type="submit" value="<?php echo $submitLabel ?>"></td>
    </tr>
  </table>
</form>
