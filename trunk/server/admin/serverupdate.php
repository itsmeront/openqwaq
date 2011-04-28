<?php
include("serverapi.php");
$server = $_GET['server'];

// Process the core properties first
$props = array();
foreach($_POST as $key => $value) {
  $key = str_replace('_','.',$key);
  $props[$key] = $value;
}
$result = putServerConfigProperties($server, $props);

// Now the email templates
foreach($_FILES as $key => $file) {
  $key = str_replace('_','.',$key);
  $tmpName = $file['tmp_name'];
  $data = file_get_contents($tmpName);
  if($tmpName == '') continue;
  $result = putServerTemplateFile($server, $key, $data);
}

header("Location: serverconf.php?server=$server");
?>