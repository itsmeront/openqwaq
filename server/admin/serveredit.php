<?php
include("serverapi.php");

if(isset($_REQUEST['id'])) {
  $id = $_REQUEST['id'];
}

$form["internalName"] = $_REQUEST["internalName"];
$form["externalName"] = $_REQUEST["externalName"];
$form["status"] = $_REQUEST["status"];
$form["role"] = $_REQUEST["role"];
$form["pool"] = ""; // $_REQUEST["pool"];

if($_REQUEST['action'] == 'delete') {
  header("Location: deleteserver.php?id=$id");
  return;
}

if($_REQUEST['action'] == 'update') {
  // Update a server; go back to serverlist.php
  updateServer($id, $form);
 }

if($_REQUEST['action'] == 'setup') {
  // Redirect to serverconf.php
  header("Location: serverconf.php?server={$form['internalName']}");
  return;
 }

if($_REQUEST['action'] == 'test') {
  // Redirect to serverconf.php
  header("Location: servertest.php?server={$form['internalName']}");
  return;
 }

if($_REQUEST['action'] == 'Add Server') {
  // Add new server; go back to serverlist.php
  $id = createServer($form);
 }

header("Location: serverlist.php");

?>