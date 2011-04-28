<?php

include("serverapi.php");

$base64 = $_POST['license'];
$data = base64_decode($base64);

file_put_contents("OpenQwaq-license", $data);
header("Location: index.php");

?>
