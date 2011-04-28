<?php
/**
 * PHP Template.
 */
$pageTitle = "Could Not Send Email";
  include_once("adminapi.php");
  $id = $_GET["id"];
  if (!isset($id)) {
     header("Location: edituser.php");
     return;
  }
  $form = findUserById($id);
  $result = False;
  if(!isset($_POST["server"])) $_POST["server"] = "";
  if($_POST['submit'] == "Send Welcome Email") {
    $result = sendActivationRequest($_POST["server"], $form);
  }
  if ($_POST['submit'] == "Send Account Reminder") { 
    $result = sendPasswordReminderRequest($_POST["server"], $form["email"]);
  }
  if($result) {
      header("Location: edituser.php?id=$id");
      return;
  }
  $pageInfo = $lastError;
?>

<?php include('errorframe.php') ?>
