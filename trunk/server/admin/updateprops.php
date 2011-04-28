<?php

include('propsapi.php');

if(isset($_GET['id'])) {
  $orgid = $_GET['id'];
  $target = "editorg.php?id=$orgid";
 } else {
  $orgid = '';
  $target = "editorg.php";
 }

if($_POST['action'] == 'Reset To Defaults') {
  $result = resetForumsProperties($orgid);
  header("Location: $target");
  return;
 }

$props = array();
foreach($_POST as $key => $value) {
  if($key <> 'action') {
    $key = str_replace('_','.',$key);
    $props[$key] = $value;
  }
}
$result = putForumsProperties($orgid, $props);
if($result) {
  header("Location: $target");
  return;
}

$pageInfo = $lastError;
?>

<?php include('errorframe.php'); ?>
