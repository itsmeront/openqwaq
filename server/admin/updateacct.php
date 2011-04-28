<?php
include("adminapi.php");

if(isset($_REQUEST["accountID"])) {
    // this is the response to an Ajax Request
    $id = $_REQUEST["accountID"];
    $account = findOrgAccountById($id);
    if(isset($account)) {
      $newRole = $_REQUEST["newRole"];
      $values["sfdc"]=$account["sfdc"];
      $values["role"]=$newRole;
      $values["status"]=$account["status"];
      updateOrgAccount($id, $values);
      return header('HTTP/1.0 200 OK');
    }
    $account = findGroupAccountById($id);
    if(isset($account)) {
      $values["status"]=$account["status"];
      updateGroupAccount($id, $values);
      return header('HTTP/1.0 200 OK');
    }
    return header('HTTP/1.0 404 Not Found');
}
    
if(isset($_GET["orgid"])) {
  // adds an account to an org (allows for both user and group)
  $id=$_GET["orgid"];
  $pageInfo="Organization does not exist";
  $org = findOrgByID($id);
  $user = findUserByName($_POST["userName"]);
  if(!isset($user)) {
    $group = findGroupByName($_POST["userName"]);
    if(!isset($group)) {
      $pageInfo="User/Group does not exist";
    }
  }
  $landingPage="editorg.php?id=$id";
}

if(isset($_GET["userid"])) {
  // adds an account to a user
  $id=$_GET["userid"];
  $pageInfo="User does not exist";
  $user = findUserByID($id);
  $org = findOrgByName($_POST["orgName"]);
  if(!isset($org)) {
    $group = findGroupByName($_POST["orgName"]);
    if(!isset($group)) {
      $pageInfo="Group/Organization does not exist";
    }
  }
  $landingPage="edituser.php?id=$id";
}

if(isset($_GET["groupid"])) {
  // adds an account to a group
  $id=$_GET["groupid"];
  $pageInfo="Group does not exist";
  $group = findGroupByID($id);
  if(isset($_POST['userName']))
    $user = findUserByName($_POST["userName"]);
  if(!isset($user)) {
    if(isset($_POST['orgName']))
      $org = findOrgByName($_POST["orgName"]);
    if(!isset($org)) {
      $pageInfo="User/Org does not exist";
    }
  }
  $landingPage="editgroup.php?id=$id";
}


if(isset($user) && isset($org)) {
  // user in org membership
  $acct["type"] = "user";
  $acct["userid"] = $user["id"];
  $acct["userName"] = $user["name"];
  $acct["orgid"] = $org["id"];
  $acct["orgName"] = $org["name"];
  $acct["role"] = $_POST["role"];
  $acct["status"] = "Invited";
  $id = createAccount($acct);
  if($id) {
    header("Location: $landingPage");
    return;
  }
  $pageInfo=$lastError;
}

if(isset($user) && isset($group)) {
  // user in group membership
  $acct["type"] = "user";
  $acct["userid"] = $user["id"];
  $acct["userName"] = $user["name"];
  $acct["groupid"] = $group["id"];
  $acct["groupName"] = $group["name"];
  $id = createGroupAccount($acct);
  if($id) {
    header("Location: $landingPage");
    return;
  }
  $pageInfo=$lastError;
}

if(isset($group) && isset($org)) {
  // group in org membership
  $acct["type"] = "group";
  $acct["userid"] = $group["id"];
  $acct["userName"] = $group["name"];
  $acct["orgid"] = $org["id"];
  $acct["orgName"] = $org["name"];
  $acct["role"] = $_POST["role"];
  $acct["status"] = "Invited";
  $id = createAccount($acct);
  if($id) {
    header("Location: $landingPage");
    return;
  }
  $pageInfo=$lastError;
}

$pageTitle="Failed to create account";
if(!isset($pageInfo)) $pageInfo="unknown reason";

?>

<?php include("errorframe.php"); ?>
