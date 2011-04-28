<?php

include_once("adminapi.php");
include("config.php");

$serverAddr = $allServers[0][2];

// Kill a particular router
function killRouter($id) {
  global $lastError;
  global $serverAddr;

  $urlBase = $serverAddr."/forums/killrouter?id=$id";
  //echo $urlBase;
  $doc = new DOMDocument("1.0", "UTF-8");
  $xml =  $doc->saveXML();
  $response = sendServerRequest($urlBase, $xml);
  $xml = new SimpleXMLElement($response);
  //echo (string)$xml;
  $success = $xml->getName();
  $reason = $response;
  if($success <> "ok") {
      $lastError = $reason;
      return False;
  }
  return True;
}

// Kill a proxy client
function killProxyclient($pid, $server) {
  global $lastError;
  global $serverAddr;

  $urlBase = $serverAddr."/forums/killproxyclient?pid=$pid&server=$server";
  //echo $urlBase;
  $doc = new DOMDocument("1.0", "UTF-8");
  $xml =  $doc->saveXML();
  $response = sendServerRequest($urlBase, $xml);
  $xml = new SimpleXMLElement($response);
  //echo (string)$xml;
  $success = $xml->getName();
  $reason = $response;
  if($success <> "ok") {
      $lastError = $reason;
      return False;
  }
  return True;
}

// Retrieve a server.conf file from the given host
function getServerTemplateFile($serverName, $key) {
  global $lastError;

  $urlBase = "http://$serverName:9991/forums/getTemplateFile?key=$key";
  $doc = new DOMDocument("1.0", "UTF-8");
  $xml =  $doc->saveXML();
  $response = sendServerRequest($urlBase, $xml);

  $xml = new SimpleXMLElement($response);
  $success = $xml->getName();
  $reason = $response;
  if($success <> "ok") {
      $lastError = $reason;
      return;
  } else {
    return (string)$xml;
  }
}

function sendMail($server, $to, $subject, $message, $other) {
  global $lastError;

  $urlBase = "http://$server:9991/forums/sendMail";

  $doc = new DOMDocument("1.0", "UTF-8");

  $top = $doc->createElement("sendMail", "");
  $doc->appendChild($top);
  $node = $doc->createElement("to", $to);
  $top->appendChild($node);
  $node = $doc->createElement("subject", $subject);
  $top->appendChild($node);
  $node = $doc->createElement("body", $message);
  $top->appendChild($node);
  foreach($other as $key => $value) {
    $node = $doc->createElement($key, $value);
    $top->appendChild($node);
  }
  $xml = $doc->saveXML();

  $response = sendServerRequest($urlBase, $xml);

  $xml = new SimpleXMLElement($response);
  $success = $xml->getName();
  $reason = $response;
  if($success <> "ok") {
      $lastError = $reason;
      return False;
  }
  return True;
}

function putServerTemplateFile($server, $key, $data) {
  global $lastError;

  $urlBase = "http://$server:9991/forums/putTemplateFile";

  $doc = new DOMDocument("1.0", "UTF-8");

  $top = $doc->createElement("putTemplateFile", "");
  $doc->appendChild($top);
  $node = $doc->createElement("key", $key);
  $top->appendChild($node);
  $node = $doc->createElement("data", $data);
  $top->appendChild($node);
  $xml = $doc->saveXML();

  $response = sendServerRequest($urlBase, $xml);
  $xml = new SimpleXMLElement($response);
  $success = $xml->getName();
  $reason = $response;
  if($success <> "ok") {
      $lastError = $reason;
      return False;
  }
  return True;
}

// Retrieve a server.conf file from the given host
function getServerConfigProperties($serverName, $options) {
  global $lastError;

  $urlBase = "http://$serverName:9991/forums/getConfigProperties";

  $doc = new DOMDocument("1.0", "UTF-8");
  $xml =  $doc->saveXML();

  $top = $doc->createElement("getConfigProperties", "");
  $doc->appendChild($top);
  $count = count($options);
  for($i=0; $i<$count; $i++) {
    $node = $doc->createElement("property", '');
    $node->setAttribute("key", $options[$i]);
    $top->appendChild($node);
  }
  $xml = $doc->saveXML();

  $response = sendServerRequest($urlBase, $xml);

  $xml = new SimpleXMLElement($response);
  $success = $xml->getName();
  $reason = $response;
  if($success <> "ok") {
      $lastError = $reason;
      return;
  }
  $result = array();
  foreach($xml->children() as $form) {
    if($form->getName() == "property") {
      $attrs = $form->attributes();
      $key = (string)$attrs['key'];
      $value = (string) $form;
      $result[$key] = $value;
    }
  }
  return $result;
}

function putServerConfigProperties($serverName, $options) {
  global $lastError;

  $urlBase = "http://$serverName:9991/forums/putConfigProperties";

  $doc = new DOMDocument("1.0", "UTF-8");
  $xml =  $doc->saveXML();

  $top = $doc->createElement("setConfigProperties", "");
  $doc->appendChild($top);
  foreach($options as $key => $value) {
    $node = $doc->createElement("property", $value);
    $node->setAttribute("key", $key);
    $top->appendChild($node);
  }
  $xml = $doc->saveXML();

  $response = sendServerRequest($urlBase, $xml);

  $xml = new SimpleXMLElement($response);
  $success = $xml->getName();
  $reason = $response;
  if($success <> "ok") {
      $lastError = $reason;
      return False;
  }
  return True;
}


// Retrieve a server.conf file from the given host
function getServerConfigFile($serverName) {
  global $lastError;

  $urlBase = "http://$serverName:9991/forums/getConfigFile";
  $doc = new DOMDocument("1.0", "UTF-8");
  $xml =  $doc->saveXML();
  $response = sendServerRequest($urlBase, $xml);

  $xml = new SimpleXMLElement($response);
  $success = $xml->getName();
  $reason = $response;
  if($success <> "ok") {
      $lastError = $reason;
      return;
  } else {
    return (string)$xml;
  }
}

function putServerConfigFile($serverName, $contents) {
  global $lastError;

  $urlBase = "http://$serverName:9991/forums/putConfigFile";

  $doc = new DOMDocument("1.0", "UTF-8");
  $top = $doc->createElement("setConfigFile", $contents);
  $doc->appendChild($top);
  $xml = $doc->saveXML();

  $response = sendServerRequest($urlBase, $xml);

  $xml = new SimpleXMLElement($response);
  $success = $xml->getName();
  $reason = $response;
  if($success <> "ok") {
      $lastError = $reason;
      return False;
  }
  return True;
}

// Retrieve a server.conf file from the given host
function runServerTest() {
  global $lastError, $serverAddr;
  $urlBase = $serverAddr."/forums/testServer";
  $doc = new DOMDocument("1.0", "UTF-8");
  $xml =  $doc->saveXML();
  $response = sendServerRequest($urlBase, $xml);
  $xml = new SimpleXMLElement($response);
  return $xml;
}

function emailLogs() {
  global $lastError, $serverAddr;
  $urlBase = $serverAddr."/forums/emaillogs";
  $doc = new DOMDocument("1.0", "UTF-8");
  $xml =  $doc->saveXML();
  $response = sendServerRequest($urlBase, $xml, 200);
  $xml = new SimpleXMLElement($response);
  return $xml;
}
/*
   * listServers(): List all the available servers
   * createServer(values): Create a server entry
   * updateServer(id, values): Update server info
   * deleteServer(id): Delete a server.
*/

function listServers() {
  $odbc = odbcConnect();
  $sql = "SELECT * from servers";
  $rs = odbc_exec($odbc, $sql);
  $result = array();
  while(odbc_fetch_row($rs)) {
    $result[] = array(
	"created" => odbc_result($rs, "created"),
	"id" => odbc_result($rs, "id"),
	"internalName" => odbc_result($rs, "internal_name"),
	"externalName" => odbc_result($rs, "external_name"),
	"pool" => odbc_result($rs, "pool"),
	"status" => odbc_result($rs, "status"),
	"role" => odbc_result($rs, "role"),
	);
  }
  odbc_close($odbc);
  if(isset($result)) return $result;
}

function createServer($values) {
  global $lastError;
  $odbc = odbcConnect();

  $stmt = odbc_prepare($odbc, "INSERT into servers(role, internal_name, external_name, status, pool, created) VALUES(?, ?, ?, ?, ?, CURRENT_TIMESTAMP())");
  $rs = odbc_execute($stmt, array($values["role"], $values["internalName"], $values["externalName"], $values["status"], $values["pool"]));
  odbc_close($odbc);
  return true;
}

function updateServer($id, $values) {
  $odbc = odbcConnect();
  $stmt = odbc_prepare($odbc, "UPDATE servers SET internal_name=?, external_name=?, status=?, role=?, pool=? WHERE id=?");
  $rs = odbc_execute($stmt, array($values["internalName"], $values["externalName"], $values["status"], $values["role"], $values["pool"], $id));
  odbc_close($odbc);
  return $rs;
}

function deleteServer($id) {
  $odbc = odbcConnect();
  $stmt = odbc_prepare($odbc, "DELETE FROM servers WHERE id = ?");
  $rs = odbc_execute($stmt, array($id));
  odbc_close($odbc);
  return $rs;
}

function findServerById($id) {
  $odbc = odbcConnect();
  $sql = "SELECT * from servers where id='$id'";
  $rs = odbc_exec($odbc, $sql);
  if(odbc_fetch_row($rs)) {
    $result = array(
	"created" => odbc_result($rs, "created"),
	"id" => odbc_result($rs, "id"),
	"internalName" => odbc_result($rs, "internal_name"),
	"externalName" => odbc_result($rs, "external_name"),
	"status" => odbc_result($rs, "status"),
	"role" => odbc_result($rs, "role"),
	);
  }
  odbc_close($odbc);
  if(isset($result)) return $result;
}

// Retrieve the log files from the server
function getServerLogFiles($serverName) {
  global $lastError;

  $urlBase = "http://$serverName:9991/forums/logs/";
  $doc = new DOMDocument("1.0", "UTF-8");
  $xml =  $doc->saveXML();
  $response = sendServerRequest($urlBase, $xml);

  $xml = new SimpleXMLElement($response);
  return $xml;
}

// Retrieve the log files from the server
function getServerLogNamed($serverName, $file) {
  global $lastError;

  $urlBase = "http://$serverName:9991/forums/logs/$file";
  $doc = new DOMDocument("1.0", "UTF-8");
  $xml =  $doc->saveXML();
  $response = sendServerRequest($urlBase, $xml);

  $xml = new SimpleXMLElement($response);
  return $xml;
}


?>
