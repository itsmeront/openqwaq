<?php

include("serverapi.php");

$server= $_GET['server'];
$key = $_GET['key'];
$data = $_POST['template'];

putServerTemplateFile($server, $key, $data);

header("Location: edittemplate.php?server=$server&key=$key");
?>