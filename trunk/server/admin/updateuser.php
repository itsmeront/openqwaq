<?php

include('adminapi.php');

if($_POST['submit'] == "Send Welcome Email" || $_POST['submit'] == "Send Account Reminder") {
    return include "sendEmail.php";
}
if(isset($_GET['id'])) {
  // for an update op, prefetch the user data
  $id = $_GET['id'];
  $form = findUserById($id);
}

if($_POST['submit'] == "Delete User") {
  header("Location: deleteuser.php?id=$id");
  return;
}

if(isset($_POST["password"]))
  $form["password"] = $_POST["password"];
if(isset($_POST['email']))
   $form["email"] = $_POST["email"];

$form["status"] = $_POST["status"];
$form["firstName"] = $_POST["firstName"];
$form["lastName"] = $_POST["lastName"];
$form["company"] = $_POST["company"];
$form["comment"] = $_POST["comment"];

if(!isset($form['email'])) {
  $form['email'] = $_POST['name'];
}

if(isset($id)) {
  // update user info
  $pageTitle = "Unable to update user";
  if($_POST["name"] != $form["name"]) {
    // Oops, sorry but we really can't change user ids right now
    $pageInfo = "<span style=\"error\">User rename not implemented</span>";
  } else if($_POST["password"] <> $_POST["password2"]) {
    $pageInfo = "<span style=\"error\">Passwords do not match</span>";
  } else {

    // update the user data accordingly
    $rs = updateUser($id, $form);

    // In order to update the password the user needs to be activated
    if($rs && isset($form['password']) && !isUserActivated($form['name'])) {
      // password will be set in activation request; as well as status
      sendActivationRequest('', $form);
    }

    if($rs) {
      header("Location: edituser.php?id=$id");
      return;
    }
    $pageInfo = "<b>Update operation failed</b>";
  }
} else {
  // this is a user creation request; fill in name and status
  $pageTitle = "Unable to create user";
  $form["name"] = $_POST["name"];
  $form["status"] = "ActiveUser";
  if($_POST["password"] <> $_POST["password2"]) {
    $pageInfo = "<span style=\"error\">Passwords do not match</span>";
  } else {
    $userid = createUser($form);
    $user = findUserById($userid);
    $org = findOrgByName($_POST['orgName']);
    $group = findGroupByName($_POST['orgName']);
    if(isset($user) && isset($org)) {
      $acct["type"] = "user";
      $acct["userid"] = $user["id"];
      $acct["userName"] = $user["name"];
      $acct["orgid"] = $org["id"];
      $acct["orgName"] = $org["name"];
      $acct["role"] = $_POST["role"];
      $acct["status"] = "Invited";
      $id = createAccount($acct);
      if(isset($id)) {
	if(isset($_POST['activateUser']))
	  sendActivationRequest('', $form);
	header("Location: edituser.php?id=$userid");
	return;
      }
    }
    if(isset($user) && isset($group)) {
      // user in group membership
      $acct["type"] = "user";
      $acct["userid"] = $user["id"];
      $acct["userName"] = $user["name"];
      $acct["groupid"] = $group["id"];
      $acct["groupName"] = $group["name"];
      $id = createGroupAccount($acct);
      if(isset($id)) {
	if(isset($_POST['activateUser']))
	  sendActivationRequest('', $form);
	header("Location: edituser.php?id=$userid");
	return;
      }
    }
    $pageInfo = $lastError;
  }
}
?>

<?php include("errorframe.php"); ?>
