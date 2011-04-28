<?php
include("adminapi.php");

if(isset($_REQUEST["accountID"])) {
    // this is the response to an Ajax Request
    $id = $_REQUEST["accountID"];
    $newStatus = $_REQUEST["newStatus"];
    $account = findOrgAccountById($id);
    if(isset($account)) {
      $values["sfdc"]=$account["sfdc"];
      $values["role"]=$account["role"];
      $values["status"]=$newStatus;
      updateOrgAccount($id, $values);
      return header('HTTP/1.0 200 OK');
    }
    $account = findGroupAccountById($id);
    if(isset($account)) {
      $values["status"]=$newStatus;
      updateGroupAccount($id, $values);
      return header('HTTP/1.0 200 OK');
    }
    return header('HTTP/1.0 404 Not Found');
}
?>
