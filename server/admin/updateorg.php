<?php
include('adminapi.php');

if(isset($_GET['id'])) {
  // this is an update operation
  $id = $_GET['id'];
  $form = findOrgById($id);
}

if($_POST['submit'] == "delete") {
  header("Location: deleteorg.php?id=$id");
  return;
}

$form["status"] = $_POST["status"];
$form["seats"] = $_POST["seats"];
if (!isset($_POST['tier'])){
    $_POST['tier']="";
}
$form["tier"] = $_POST["tier"];
$form["comment"] = $_POST["comment"];

if(isset($id)) {
  // this is an update operation
  $pageTitle="Unable to update Organization";

  if($_POST["server"] != $form["server"]) {
    // Moving orgs between servers is unsupported
    $pageInfo = "<span style=\"error\">Cannot move orgs</span>";
  } else {
    // update the data base accordingly
    $rs = true;
    if($_POST["name"] != $form["name"]) {
      // rename the org
      $rs = renameOrg($form["server"], $id, $_POST["name"]);
      if(!$rs) $pageInfo = $lastError;
    }
    // make sure we have the up-to-date name here
    $form["name"] = $_POST["name"];
    if($rs) {
      // update the org values
      $rs = updateOrg($id, $form);
    }
    if($rs) {
      header("Location: editorg.php?id=$id");
      return;
    }
    $pageInfo = "<b>Update operation failed</b>";
  }
} else {
  // this is an org creation request; fill in name and server
  $pageTitle="Unable to create Organization";
  $form["name"] = $_POST["name"];
  // $form["server"] = $_POST["server"];
  $id = createOrg($form);
  // redirect to edit org with the new id
  if($id) {
    header("Location: editorg.php?id=$id");
    return;
  }
  $pageInfo = $lastError;
}

?>

<?php include("errorframe.php"); ?>
