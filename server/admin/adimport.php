<?php

include("serverapi.php");

$user=$_POST["user"];
$password=$_POST["password"];
$group=$_POST["group"];
$orgid=$_POST["orgid"];

$props = array("OpenQwaq.AD.ServerName", "OpenQwaq.AD.BaseDN", "OpenQwaq.AD.ListScript");
$props = getServerConfigProperties("localhost", $props);

$server= $props['OpenQwaq.AD.ServerName'];
$basedn= $props['OpenQwaq.AD.BaseDN'];
$script= $props['OpenQwaq.AD.ListScript'];

$org = findOrgById($orgid);
if(!isset($org)) {
  echo "No organization specified";
  return;
 }
var_dump($org);

$command = $script .
    ' -u ' . escapeshellarg($user) .
    ' -p ' . escapeshellarg($password) .
    ' -s ' . escapeshellarg($server) .
    ' -g ' . escapeshellarg($group);

var_dump($command);

$output = array();
exec($command, $output, $result);
var_dump($output);

if($result == 0) {
  // parse script response from output
  $response = implode("\n", $output);
 } else {
  $response = "<failed>Script failed: Return value: $result</failed>";
 }

$xml = new SimpleXMLElement($response);
$success = $xml->getName();
if($success == 'ok') {
  echo "<table><tr>";
  echo "<th>Login Name</th>";
  echo "<th>Email Address</th>";
  echo "<th>First Name</th>";
  echo "<th>Last Name</th>";
  echo "</tr>";
  foreach($xml->children() as $node) {
    if($node->getName() == "user") {
      $form = array(
		    "name" => (string)$node->loginName,
		    "password" => "",
		    "status" => "ActiveUser",
		    "email" => (string)$node->email,
		    "firstName" => (string)$node->firstName,
		    "lastName" => (string)$node->lastName,
		    "fullName" => (string)$node->displayName,
		    "company" => (string)$node->company,
		    "comment" => "Imported from Active Directory"
		    );

      $user = findUserByName($form['name']);
      if(!isset($user)) {
	// Create the user in OpenQwaq
	$uid = createUser($form);
	$user = $form;
      } else {
	$uid = $user['id'];
      }

      $acct = findAccountByUserAndOrg($uid, $orgid);
      if(isset($acct)) {
	// skip it
	var_dump($acct);
      } else {
	$acct["userid"] = $user["id"];
	$acct["userName"] = $user["name"];
	$acct["orgid"] = $org["id"];
	$acct["orgName"] = $org["name"];
	$acct["role"] = $_POST["role"];
	$acct["status"] = "Invited";
	$id = createAccount($acct);

	echo "<tr>";
	echo "<td>{$form['name']}</td>";
	echo "<td>{$form['email']}</td>";
	echo "<td>{$form['firstName']}</td>";
	echo "<td>{$form['lastName']}</td>";
	echo "</tr>";
      }
    }
  }
  echo "</table>";
 } else {
  echo (string)$xml;
 }

?>
