<?php

include("serverapi.php");

$server= $_GET['server'];
$key = $_GET['key'];

$data = getServerTemplateFile($server, $key);

?>
<?php include('header.php'); ?>
<body id="edit_user" class="users">
	<?php make_navbar('Servers'); ?>
	<div id="body" class="wrap">
		<div class="section">
			<?php
echo "<form method=\"POST\" action=\"updatetemplate.php?server=$server&key=$key\">";
echo "<textarea name=\"template\" rows=\"30\" cols=\"80\">";
echo htmlspecialchars($data, ENT_QUOTES);
echo "</textarea><p>";
echo "<input type=\"submit\" name=\"action\" value=\"Save Template\">";
echo "</form>";

?>
		</div>
	</div>
	<?php include('footer.php') ?>
</body>
</html>
