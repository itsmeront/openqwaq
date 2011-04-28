<form name="viewOrgForm" method="post" action="<?php echo $submitPage; ?>">
  <input type="hidden" name="id" value="<?php echo $form['id'] ?>">
  <table border="0">
    <tr>
      <td><label class="required">Name:</label></td>
      <td><?php echo $form['name'] ?></td>
    </tr>
    <tr>
      <td><label class="required">Seats:</label></td>
      <td><?php echo $form['seats'] ?></td>
    </tr>
    <tr>
      <td><label class="required">Tier:</label></td>
      <td><?php echo $form['tier'] ?></td>
    </tr>
    <tr>
      <td><label class="required">Status:</label></td>
      <td><?php echo $form['status'] ?></td>
    </tr>
    <tr>
      <td></td>
      <td><input type="submit" value="<?php echo $submitLabel; ?>"></td>
    </tr>
  </table>
</form>
