<?php

include("serverapi.php");
$id = $_GET['id'];
$result = killRouter($id);
header("Location: serverinfo.php");
?>