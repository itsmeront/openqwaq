<?php

include("serverapi.php");
$pid = $_GET['pid'];
$server = $_GET['server'];
$result = killProxyclient($pid, $server);
header("Location: serverinfo.php");
?>