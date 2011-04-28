<?php
/* This is a simple test page to test AD authentication */
include("serverapi.php");


if(!isset($_POST['login'])) {
  $login = '';
  $password = '';
  $response = "\n";
} else {

  $login=$_POST["login"];
  $password=$_POST["password"];

  $props = array("OpenQwaq.AD.Enabled", "OpenQwaq.AD.ServerName", 
		 "OpenQwaq.AD.BaseDN", "OpenQwaq.AD.AuthScript");
  $props = getServerConfigProperties("localhost", $props);

  $enabled = $props['OpenQwaq.AD.Enabled'];
  $server = $props['OpenQwaq.AD.ServerName'];
  $basedn = $props['OpenQwaq.AD.BaseDN'];
  $script = $props['OpenQwaq.AD.AuthScript'];

  $command = $script .
    ' -s ' . escapeshellarg($server) .
    ' -b ' . escapeshellarg($basedn) .
    ' -u ' . escapeshellarg($login) .
    ' -p ' . escapeshellarg('********') .
    ' 2>&1 ';

  if($enabled <> 'true') {
    $response = "AD Authentication: DISABLED\n";
  } else {
    $response = "AD Authentication: Enabled\n";
  }
  $response = $response. "
Server: $server
Base DN: $basedn
Login (UPN): $login
Script: $script

$command

";

  $command = $script .
    ' -s ' . escapeshellarg($server) .
    ' -b ' . escapeshellarg($basedn) .
    ' -u ' . escapeshellarg($login) .
    ' -p ' . escapeshellarg($password) .
    ' 2>&1 ';

  $output = array();
 exec($command, $output, $result);

 if($result <> 0) {
   $response = $response . "Script failed: Return value: $result\n\n";
 }

 $response = $response . htmlspecialchars(implode("\n", $output));
}

?>

<?php include('header.php'); ?>
<body id="edit_server" class="servers">
<?php make_navbar('Servers'); ?>

<div id="body" class="wrap">

<div class="section">

<form name="adTestForm" method="post" action="adtest.php" class="form">

<fieldset>
	<legend>Login Information</legend>
	<table style="border: 0px none ;">
	<tr>
	<td width="200px" style="background: white;">Login (email/upn):</td>
	<td style="background: white;"
	  <input type="text" name="login" value="<?php echo $login;?>" size="50">
	</td>
	</tr>
	<tr>
	<td width="200px" style="background: white;">Password:</td>
	<td style="background: white;"
	  <input type="password" name="password" value="<?php echo $password;?>" size="50">
	</td>
	</tr>
	</table>
<input type="submit" value="Authenticate">
<p>
</fieldset>


<fieldset>
	<legend>Response</legend>
<pre>
<?php echo $response; ?>
</pre>
<p>
</fieldset>
</form>

</div>
</div>
<?php include('footer.php') ?>
</body>
</html>
