<?php
include('adminapi.php');

if(isset($_GET['id'])) {
  // this is an update operation
  $id = $_GET['id'];
  $form = findGroupById($id);
}

if($_POST['submit'] == "Delete Group") {
  header("Location: deletegroup.php?id=$id");
  return;
}

$form["status"] = $_POST["status"];
$form["comment"] = $_POST["comment"];

if(isset($id)) {
  // this is an update operation
  $pageTitle="Unable to update Group";
  // update the data base accordingly
  $rs = true;
  // update the org values
  $rs = updateGroup($id, $form);
  if($rs) {
    header("Location: editgroup.php?id=$id");
    return;
  }
  $pageInfo = "<b>Update operation failed</b>";
} else {
  // this is a group creation request; fill in fields
  $pageTitle="Unable to create Group";
  $form["name"] = $_POST["name"];
  $id = createGroup($form);
  // redirect to edit group with the new id
  if($id) {
    header("Location: editgroup.php?id=$id");
    return;
  }
  $pageInfo = $lastError;
}

?>

<?php include("errorframe.php"); ?>
