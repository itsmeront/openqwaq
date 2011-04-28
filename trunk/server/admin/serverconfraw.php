<?php

include("serverapi.php");

$server = $_GET['server'];
$config = getServerConfigFile($server);

?>

<?php include('header.php'); ?>
<body id="edit_user" class="users">
<?php make_navbar('Servers'); ?>
<div id="body" class="wrap">

  <b>Warning: Incorrect modifications to this file can harm your server</b><p>

<?php
  echo "<form method=\"POST\" action=\"serverupdateraw.php?server=$server\">";
?>
<textarea name="config" rows="30" cols="80">
<?php echo htmlspecialchars($config, ENT_QUOTES); ?>
</textarea>
<p>
<input type="submit" name="action" value="Update">
</form>

</div>
<?php include('footer.php') ?>
</body>
</html>


