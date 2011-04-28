<?php
include("serverapi.php");
$server = $_GET['server'];
$result = putServerConfigFile($server, $_POST['config']);
if($result) {
  header("Location: serverconfraw.php?server=$server");
  return;
}
$pageTitle = "Failed to update server configuration";
$pageInfo = $lastError;
?>

<?php include("errorframe.php"); ?>
