<?php

include("config.php");
include("SFUpdateDB.php");

$lastError = "";

// Sends the XML request to the specified url 
function sendServerRequest($url, $xml, $timeout = 10) {
  $tokens = @explode("/", $url, 4); 
  $page = '/'.$tokens[3]; 
  $tokens = @explode(":", $tokens[2], 2); 
  $host = $tokens[0]; 
  $port = $tokens[1]; 
  $ip = gethostbyname($host); 
  $data = "POST $page HTTP/1.0\r\n"; 
  $data .= "Host: $host:$port\r\n"; 
  $data .= "Connection: Close\r\n"; 
  $data .= "Content-Type: text/xml\r\n"; 
  $data .= "Content-Length: ".strlen($xml)."\r\n\r\n"; 
  $data .= $xml; 
 
  $handle = @fsockopen($ip,$port, $errno, $errstr, $timeout); 
  if(!$handle) { 
    return "<failed>Unable to connect to $url</failed>"; 
  } 
  fputs($handle, $data); 
  $response = stream_get_contents($handle); 
  fclose($handle); 

  // Check the response code. If its a 200 return the contents, 
  // otherwise construct an error response from the reason string. 
  $response = @explode("\r\n\r\n",$response,2); 
  $line = @explode("\r\n", $response[0], 2); 
  // syslog(LOG_INFO, "HTTP response: {$line[0]}"); 
  $tokens = @explode(" ", $line[0], 3); 
  if($tokens[1] == "200") { 
    return $response[1]; 
  } else {
    return '<failed>'.$line[0].'</failed>'; 
  } 
}
 
function parseServerResponse($response) { 
  $doc = new DOMDocument("1.0", "UTF-8"); 
  #$old = libxml_use_internal_errors(true); 
  $doc->loadXML($response); 
  #libxml_use_internal_errors($old); 
  return $doc; 
} 

/**************************************************************************/

// Encapsulates connection settings etc.
function odbcConnect() {
  global $odbcData, $odbcUser, $odbcPass;
  return odbc_connect($odbcData, $odbcUser, $odbcPass);
}

function odbcLogConnect() {
  global $odbcLogData, $odbcLogUser, $odbcLogPass;
  return odbc_connect($odbcLogData, $odbcLogUser, $odbcLogPass);  
}

// Creates the actual stuff on disk
function setupOrgDir($serverName, $orgName) {
  global $lastError, $allServers;

  // Setup the files on disk for the organization
  foreach($allServers as $server) {
    if($serverName == $server[0]) $urlBase=$server[2];
  }

  $setupOrgUrl = $urlBase . "/coal/createOrg";
  // We don't have enough info at the php level to do the actual
  // org creation to punt and task the SP with it.
  $doc = new DOMDocument("1.0", "UTF-8");
  $top = $doc->createElement("createOrg", "");
  $doc->appendChild($top);
  $node = $doc->createElement("orgName");
  $node->appendChild($doc->createTextNode($orgName));
  $top->appendChild($node);
  $xml = $doc->saveXML();

  $response = sendServerRequest($setupOrgUrl, $xml);
  $xml = new SimpleXMLElement($response);
  $success = $xml->getName();
  $reason = $response;
  $loc = $xml->location;
  if($success <> "ok") {
    $lastError = $reason;
  } else {
    return $loc;
  }
}

// Creates the actual stuff on disk
function setupUserDir($serverName, $userName) {
  global $lastError, $allServers;

  // Setup the files on disk for the organization
  foreach($allServers as $server) {
    if($serverName == $server[0]) $urlBase=$server[2];
  }

  $setupOrgUrl = $urlBase . "/coal/createUser";
  // We don't have enough info at the php level to do the actual
  // creation to punt and task the SP with it.
  $doc = new DOMDocument("1.0", "UTF-8");
  $top = $doc->createElement("createUser", "");
  $doc->appendChild($top);
  $node = $doc->createElement("userName");
  $node->appendChild($doc->createTextNode($userName));
  $top->appendChild($node);
  $xml = $doc->saveXML();
  $response = sendServerRequest($setupOrgUrl, $xml);

  $xml = new SimpleXMLElement($response);
  $success = $xml->getName();
  $reason = $response;
  $loc = $xml->location;

  if($success <> "ok") {
    $lastError = $reason;
  } else {
    return $loc;
  }
}

// Creates an activation request this sends the welcome email
function sendActivationRequest($serverName, $form) {
    global $lastError, $allServers;

    foreach($allServers as $server) {
       if($serverName == $server[0]) $urlBase=$server[2];
    }

    $activateUrl = $urlBase . "/coal/activate";
    $xml = createCommonRequest($form, "activate");

    $response = sendServerRequest($activateUrl, $xml);

    $xml = new SimpleXMLElement($response);
    $success = $xml->getName();
    $reason = $response;
    if($success <> "ok") {
       $lastError = $reason;
       return False;
    } 
      return True;
}

// Creates a password reminder request
function sendPasswordReminderRequest($serverName, $email) {
  global $lastError, $allServers;
  foreach($allServers as $server) {
      if($serverName == $server[0]) $urlBase=$server[2];
  }  
  $reminderURL = $urlBase . "/coal/emailpassword";  
  $doc = new DOMDocument("1.0", "UTF-8");
  $top = $doc->createElement("emailPassword", "");
  $doc->appendChild($top);
  $node = $doc->createElement("email");
  $node->appendChild($doc->createTextNode($email));
  $top->appendChild($node);
  $xml =  $doc->saveXML();
  
  $response = sendServerRequest($reminderURL, $xml);

  $xml = new SimpleXMLElement($response);
  $success = $xml->getName();
  $reason = $response;
  if($success <> "ok") {
      $lastError = $reason;
      return False;
  } else {
     return True;
  }
}

// Creates a common activation / signup request
function createCommonRequest($form, $tag) {
  $doc = new DOMDocument("1.0", "UTF-8");
  $top = $doc->createElement($tag, "");
  $doc->appendChild($top);
  $node = $doc->createElement("firstName");
  $node->appendChild($doc->createTextNode($form["firstName"]));
  $top->appendChild($node);
  $node = $doc->createElement("lastName");
  $node->appendChild($doc->createTextNode($form["lastName"]));
  $top->appendChild($node);
  $node = $doc->createElement("company");
  $node->appendChild($doc->createTextNode($form["company"]));
  $top->appendChild($node);
  $node = $doc->createElement("email");
  $node->appendChild($doc->createTextNode($form["email"]));
  $top->appendChild($node);
  if(isset($form["password"])) {
    $node = $doc->createElement("password");
    $node->appendChild($doc->createTextNode($form["password"]));
    $top->appendChild($node);
  }
  return $doc->saveXML();
}

// Renames an org
function renameOrg($server, $orgid, $newName) {
  global $lastError, $allServers;

  // Setup the files on disk for the organization
  foreach($allServers as $server) {
    if($serverName == $server[0]) $urlBase=$server[2];
  }

  $renameOrgUrl = $urlBase . "/coal/renameOrg";
  // We don't have enough info at the php level to do the actual
  // org rename to punt and task the SP with it.
  echo "orgName is: ".$newName;
  $doc = new DOMDocument("1.0", "UTF-8");
  $top = $doc->createElement("renameOrg", "");
  $doc->appendChild($top);
  $node = $doc->createElement("orgid", $orgid);
  $top->appendChild($node);
  $node = $doc->createElement("orgName");
  $node->appendChild($doc->createTextNode($newName));
  $top->appendChild($node);
  $xml = $doc->saveXML();
  echo $xml;  
  $response = sendServerRequest($renameOrgUrl, $xml);
  echo $response;
  if(true) {
    $xml = new SimpleXMLElement($response);
    $success = $xml->getName();
    $reason = $response;
  } else {
    $doc = parseServerResponse($response);
    $success = $doc->firstChild->nodeName;
    $reason = $doc->firstChild->nodeValue;
  }
  $lastError = $reason;
  return $success == "ok";
}

function uuid() {
  $odbc = odbcConnect();
  $rs = odbc_exec($odbc, 'select UUID()');
  if (odbc_fetch_row($rs)) {
    return odbc_result($rs,'UUID()');
  } else {
    return sprintf( '%04x%04x-%04x-%04x-%04x-%04x%04x%04x',
        mt_rand( 0, 0xffff ), mt_rand( 0, 0xffff ), mt_rand( 0, 0xffff ),
        mt_rand( 0, 0x0fff ) | 0x4000,
        mt_rand( 0, 0x3fff ) | 0x8000,
        mt_rand( 0, 0xffff ), mt_rand( 0, 0xffff ), mt_rand( 0, 0xffff ) );
  }
}


/* Operations:
   * createUser(values): Create new user; return id.
   * updateUser(id, values): Update user information.
   * deleteUser(id): Delete a user.
   * findUserById(id): return user info.
   * findUsersByMatch(query, limit): Find users by match.
*/

function createUser($values) {
  global $lastError;

  if($values["name"] == '') {
    $lastError = "No user name provided";
    return;
  }

  $odbc = odbcConnect();
  // Check if this user already exists and fail if so
  $stmt = odbc_prepare($odbc, "SELECT * FROM users WHERE name = ?");
  $rs = odbc_execute($stmt, array($values["name"]));
  if(!$rs) {
    odbc_close($odbc);
    return;
  }
  if(odbc_fetch_row($stmt)) {
    odbc_close($odbc);
    $lastError = "<b>Error:</b> User {$values['name']} already exists";
    return;
  }

  // Setup the files on disk
  if(!isset($values["server"])) $values["server"] = "";
  $userDir = setupUserDir($values["server"], $values["name"]);
  if($userDir == "") {
    odbc_close($odbc);
    return;
  }
  $id = uuid();

  /* create digest entry first */
  $stmt = odbc_prepare($odbc, "INSERT into user_digests(name, digest, updated) VALUES(?, ?, CURRENT_TIMESTAMP())");
  $rs = odbc_execute($stmt, array($values["name"], ""));

  $stmt = odbc_prepare($odbc, "INSERT into users(id, name, email, status, first_name, last_name, company, comment, created, files) VALUES(?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP(), ?)");
  $rs = odbc_execute($stmt, array($id, $values["name"], $values["email"], $values["status"], $values["firstName"], $values["lastName"], $values["company"], $values["comment"], $userDir));
  odbc_close($odbc);
  if($rs) return $id;
}

function updateUser($id, $values) {
  if ($values['sfdc'] != "") {
      updateSF('users', $id, $values['sfdc']);
  }
  $odbc = odbcConnect();

  /* update password / digest first */
  if($values["password"] <> "") {
    $md5 = md5($values["name"] . ":" . $values["password"]);
    $stmt = odbc_prepare($odbc, "UPDATE user_passwords SET password=?, updated=CURRENT_TIMESTAMP() WHERE name=?");
    $rs = odbc_execute($stmt, array($values["password"], $values["name"]));
    $stmt = odbc_prepare($odbc, "UPDATE user_digests SET digest=?, updated=CURRENT_TIMESTAMP() where name=?");
    $rs = odbc_execute($stmt, array($md5, $values["name"]));
  }

  // update the data base accordingly
  $stmt = odbc_prepare($odbc, "UPDATE users SET email=?, first_name=?, last_name=?, company=?, comment=?, status=? WHERE id=?");
  $rs = odbc_execute($stmt, array(
				  $values["email"], 
				  $values["firstName"], 
				  $values["lastName"], 
				  $values["company"], 
				  $values["comment"],
                                  $values["status"],
				  $id));
  odbc_close($odbc);
  return $rs;
}

function deleteUser($id) {
  $form = findUserById($id);
  if(!isset($form)) return;
  if ($form['sfdc'] != "") {
      updateSF('users', $id, $form['sfdc']);
      $accounts = findAccountsByUser($id);
      foreach ($accounts as $account) {
        if ($account['sfdc'] != "") {
            updateSF('org_members', $account['id'], $account['sfdc']);
        }
      }
  }
  $odbc = odbcConnect();
  $stmt = odbc_prepare($odbc, "DELETE FROM org_members WHERE member_id = ? AND member_type = ?");
  $rs = odbc_execute($stmt, array($id, "user"));
  $stmt = odbc_prepare($odbc, "DELETE FROM group_members WHERE member_id = ? AND member_type = ?");
  $rs = odbc_execute($stmt, array($id, "user"));
  $stmt = odbc_prepare($odbc, "DELETE FROM users WHERE id = ?");
  $rs = odbc_execute($stmt, array($id));
  $stmt = odbc_prepare($odbc, "DELETE FROM user_passwords WHERE name = ?");
  $rs = odbc_execute($stmt, array($form["name"]));
  $stmt = odbc_prepare($odbc, "DELETE FROM user_digests WHERE name = ?");
  $rs = odbc_execute($stmt, array($form["name"]));
  odbc_close($odbc);
  return $rs;
}

function findUserById($id) {
  $odbc = odbcConnect();
  $sql = "SELECT * from users where id='$id'";
  $rs = odbc_exec($odbc, $sql);
  if(odbc_fetch_row($rs)) {
    $result = array(
	"id" => odbc_result($rs, "id"),
	"name" => odbc_result($rs, "name"),
	"password" => "",
	"email" => odbc_result($rs, "email"),
	"status" => odbc_result($rs, "status"),
	"firstName" => odbc_result($rs, "first_name"),
	"lastName" => odbc_result($rs, "last_name"),
	"company" => odbc_result($rs, "company"),
	"comment" => odbc_result($rs, "comment"),
        "sfdc"      => odbc_result($rs, "sfdc_id")
	);
  }
  odbc_close($odbc);
  if(isset($result)) return $result;
}

// Returns true if the user has been previously activated
function isUserActivated($name) {
  $odbc = odbcConnect();
  $rs=odbc_exec($odbc, "SELECT count(*) from user_digests where name='$name'");
  $result = odbc_result($rs,1);
  odbc_close($odbc);
  return $result > 0;
}

function findUserByName($name) {
  $odbc = odbcConnect();
  $sql = "SELECT * from users where name='$name'";
  $rs = odbc_exec($odbc, $sql);
  if(odbc_fetch_row($rs)) {
    $result = array(
	"id" => odbc_result($rs, "id"),
	"name" => odbc_result($rs, "name"),
	"password" => "",
	"email" => odbc_result($rs, "email"),
	"status" => odbc_result($rs, "status"),
	"firstName" => odbc_result($rs, "first_name"),
	"lastName" => odbc_result($rs, "last_name"),
	"company" => odbc_result($rs, "company"),
	"comment" => odbc_result($rs, "comment")
	);
  }
  odbc_close($odbc);
  if(isset($result)) return $result;
}

function findAllUserNames() {
  $odbc = odbcConnect();
  $rs = odbc_exec($odbc, "SELECT name from users");
  $result = array();
  while(odbc_fetch_row($rs)) {
    $result[] = odbc_result($rs, "name");
  }
  odbc_close($odbc);
  natcasesort($result);
  return $result;
}

function totalUserCount() {
  $odbc = odbcConnect();
  $rs = odbc_exec($odbc, "SELECT count(*) from users");
  $total = odbc_result($rs,1);
  odbc_close($odbc);
  return $total;
}

function findUsersByMatch($query, $limit, $page=1) {
  $odbc = odbcConnect();
  $rs = odbc_exec($odbc, "SELECT count(*) from users where name like '%$query%' or first_name like '%$query%' or last_name like '%$query%' or company like '$query%'");
  $total = odbc_result($rs,1);
  $rs = odbc_exec($odbc, "SELECT * from users where name like '%$query%' or email like '%$query%' or first_name like '%$query%' or last_name like '%$query%' or company like '%$query%' order by created desc");
  $result = array();
  $result["total"] = $total;
  $count = ($limit*($page-1))+1;
  $rows = 0;
  while(odbc_fetch_row($rs, $count) && ($rows < $limit) ) {
    $id = odbc_result($rs, "id");
    $count += 1;
    $rows += 1;
    $result[] = array(
	"id"      => $id,
    	"created" => odbc_result($rs, "created"),
	"name"    => odbc_result($rs, "name"),
    	"email"   => odbc_result($rs, "email"),
        "firstName" => odbc_result($rs, "first_name"),
        "lastName"  => odbc_result($rs, "last_name"),
    	"company"   => odbc_result($rs, "company"),
    	"sfdc"      => odbc_result($rs, "sfdc_id")
    );
  }
  $count = $count - 1; //we didn't use the last count
  $count = $count - ($limit*($page-1));
  $result["count"] = $count;
  odbc_close($odbc);
  return $result;
}


/*
   * createGroup(values): Create new group; return id.
   * updateGroup(id,values): Update group information.
   * deleteGroup(id): Delete a group.
   * findGroupById(id): return group info.
   * findGroupsByMatch(query, limit): Find group by match.
*/

function createGroup($values) {
  global $lastError;

  if($values["name"] == '') {
    $lastError = "No group name provided";
    return;
  }

  $odbc = odbcConnect();
  $groupName = $values["name"];
  // Check if this group already exists and fail if so
  $stmt = odbc_prepare($odbc, "SELECT * FROM groups WHERE name = ?");
  $rs = odbc_execute($stmt, array($groupName));
  if(!$rs) {
    odbc_close($odbc);
    return;
  }
  if(odbc_fetch_row($stmt)) {
    odbc_close($odbc);
    $lastError= "<b>Error:</b> Group '$groupName' already exists";
    return;
  }

  // Create the group
  $id = uuid();
  $stmt = odbc_prepare($odbc, "INSERT into groups(id, name, status, comment, created) VALUES(?, ?, ?, ?, CURRENT_TIMESTAMP())");
  $rs = odbc_execute($stmt, array($id, $values["name"], $values["status"], $values["comment"]));
  odbc_close($odbc);
  if($rs) return $id;
}

function updateGroup($id, $values) {
  $odbc = odbcConnect();
  $stmt = odbc_prepare($odbc, "UPDATE groups SET name=?, status=?, comment=? WHERE id=?");
  $rs = odbc_execute($stmt, array($values["name"], $values["status"], $values["comment"], $id));
  odbc_close($odbc);
  return $rs;
}

function deleteGroup($id) {
  $group = findGroupById($id);
  if (!isset($group)) return;  
  $odbc = odbcConnect();

  $stmt = odbc_prepare($odbc, "DELETE FROM group_members WHERE group_id = ?");
  $rs = odbc_execute($stmt, array($id));
  if(!$rs) {
    odbc_close($odbc);
    return false;
  }

  $stmt = odbc_prepare($odbc, "DELETE FROM org_members WHERE member_type = 'group' and member_id = ?");
  $rs = odbc_execute($stmt, array($id));
  if(!$rs) {
    odbc_close($odbc);
    return false;
  }

  $stmt = odbc_prepare($odbc, "DELETE FROM groups WHERE id = ?");
  $rs = odbc_execute($stmt, array($id));
  odbc_close($odbc);
  return $rs;
}

function findGroupById($id) {
  $odbc = odbcConnect();
  $sql = "SELECT * from groups where id='$id'";
  $rs = odbc_exec($odbc, $sql);
  if(odbc_fetch_row($rs)) {
    $result = array(
	"id" => odbc_result($rs, "id"),
	"name" => odbc_result($rs, "name"),
	"status" => odbc_result($rs, "status"),
	"comment" => odbc_result($rs, "comment"),
	"created" => odbc_result($rs, "created"),
	);
  }
  odbc_close($odbc);
  if(isset($result)) return $result;
}

function findGroupByName($name) {
  $odbc = odbcConnect();
  $sql = "SELECT * from groups where name='$name'";
  $rs = odbc_exec($odbc, $sql);
  if(odbc_fetch_row($rs)) {
    $result = array(
	"id" => odbc_result($rs, "id"),
	"name" => odbc_result($rs, "name"),
	"status" => odbc_result($rs, "status"),
	"comment" => odbc_result($rs, "comment"),
    	"created" => odbc_result($rs, "created")
	);
  }
  odbc_close($odbc);
  if(isset($result)) return $result;
}

function findAllGroupNames() {
  $odbc = odbcConnect();
  $rs = odbc_exec($odbc, "SELECT name from groups");
  $result = array();
  while(odbc_fetch_row($rs)) {
    $result[] = odbc_result($rs, "name");
  }
  odbc_close($odbc);
  natcasesort($result);
  return $result;
}

function totalGroupCount() {
  $odbc = odbcConnect();
  $rs = odbc_exec($odbc, "SELECT count(*) from groups");
  $total = odbc_result($rs,1);
  odbc_close($odbc);
  return $total;
}

function findGroupsByMatch($query, $limit, $page=1) {
  $result = array();
  $odbc = odbcConnect();
  $rs = odbc_exec($odbc, "SELECT count(*) from groups where name like '%$query%'");
  $result["total"] = odbc_result($rs,1);
  $sql = "SELECT * from groups where name like '%$query%' order by created desc";
  $rs = odbc_exec($odbc, $sql);
  $count = ($limit*($page-1))+1;
  $rows = 0;
  while(odbc_fetch_row($rs, $count) && ($rows < $limit) ) {
    $id = odbc_result($rs, "id");
    $count += 1;
    $rows += 1;
    $result[] = array(
	"id" => $id,
	"created" => odbc_result($rs, "created"),
   	"name" => odbc_result($rs, "name"),
        "status" => odbc_result($rs, "status"),
    );
  }
  $count = $count - 1; //we didn't use the last count
  $count = $count - ($limit*($page-1));
  $result["count"] = $count;
  odbc_close($odbc);
  return $result;
}

function createGroupAccount($values) {
  global $lastError;
  $odbc = odbcConnect();

  // Check if this acct already exists and fail if so
  $stmt = odbc_prepare($odbc, "SELECT * FROM group_members WHERE member_id=? and group_id=? AND member_type=?");
  $rs = odbc_execute($stmt, array($values["userid"],$values["groupid"],$values["type"]));
  if(!$rs) {
    odbc_close($odbc);
    return;
  }
  if(odbc_fetch_row($stmt)) {
    odbc_close($odbc);
    $lastError= "<b>Error:</b> Membership for {$values['userName']} in {$values['groupName']} already exists";
    return;
  }

  $id = uuid();
  $stmt = odbc_prepare($odbc, "INSERT into group_members(id, member_type, member_id, member_name, group_id, group_name, created) VALUES(?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP())");
  $rs = odbc_execute($stmt, array($id, $values["type"], $values["userid"], $values["userName"], $values["groupid"], $values["groupName"]));
  odbc_close($odbc);
  if($rs) return $id;
}


/*
   * createOrg(values): Create new org; return id.
   * updateOrg(id,values): Update org information.
   * deleteOrg(id): Delete an organization.
   * findOrgById(id): return org info.
   * findOrgsByMatch(query, limit): Find orgs by match.
*/

function createOrg($values) {
  global $lastError;

  if($values["name"] == '') {
    $lastError = "No group name provided";
    return;
  }

  $odbc = odbcConnect();
  $orgName = $values["name"];
  // Check if this org already exists and fail if so
  $stmt = odbc_prepare($odbc, "SELECT * FROM organizations WHERE name = ?");
  $rs = odbc_execute($stmt, array($orgName));
  if(!$rs) {
    odbc_close($odbc);
    return;
  }
  if(odbc_fetch_row($stmt)) {
    odbc_close($odbc);
    $lastError= "<b>Error:</b> Organization '$orgName' already exists";
    return;
  }

  // Setup the files on disk for the organization
  if(!isset($values["server"])) $values["server"] = "";
  //$orgDir = setupOrgDir($values["server"], $orgName);
  //if($orgDir == "") {
  //odbc_close($odbc);
  //return;
  //}
  $orgDir = '';
  // Create the org in the DB
  $id = uuid();
  $stmt = odbc_prepare($odbc, "INSERT into organizations(id, name, seats, status, tier, comment, files, server, created) VALUES(?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP())");
  $rs = odbc_execute($stmt, array($id, $values["name"], $values["seats"], $values["status"], $values["tier"], $values["comment"], $orgDir, $values["server"]));
  odbc_close($odbc);
  if($rs) return $id;
}

function updateOrg($id, $values) {
  if ($values['sfdc'] != "") {
      updateSF('organizations', $id, $values['sfdc']);
  }      
  $odbc = odbcConnect();
  $stmt = odbc_prepare($odbc, "UPDATE organizations SET name=?, seats=?, status=?, tier=?, server=?, comment=? WHERE id=?");
  $rs = odbc_execute($stmt, array($values["name"], $values["seats"], $values["status"], $values["tier"], $values["server"], $values["comment"], $id));
  odbc_close($odbc);
  return $rs;
}

function deleteOrg($id) {
  $org = findOrgById($id);
  if (!isset($org)) return;  
  if ($org['sfdc'] != "") {
      updateSF('organizations', $id, $org['sfdc']);
      $accounts = findAccountsByOrg($id);
      foreach ($accounts as $account){
          if ($account['sfdc'] != ""){
              updateSF('org_members', $account['id'], $account['sfdc']);
          }
      }
  }      
  $odbc = odbcConnect();
  $stmt = odbc_prepare($odbc, "DELETE FROM org_members WHERE org_id = ?");
  $rs = odbc_execute($stmt, array($id));
  if(!$rs) {
    odbc_close($odbc);
    return false;
  }
  $stmt = odbc_prepare($odbc, "DELETE FROM organizations WHERE id = ?");
  $rs = odbc_execute($stmt, array($id));
  odbc_close($odbc);
  return $rs;
}

function findOrgById($id) {
  $odbc = odbcConnect();
  $sql = "SELECT * from organizations where id='$id'";
  $rs = odbc_exec($odbc, $sql);
  if(odbc_fetch_row($rs)) {
    $result = array(
	"id" => odbc_result($rs, "id"),
	"name" => odbc_result($rs, "name"),
	"status" => odbc_result($rs, "status"),
	"seats" => odbc_result($rs, "seats"),
	"tier" => odbc_result($rs, "tier"),
	"server" => odbc_result($rs, "server"),
	"comment" => odbc_result($rs, "comment"),
        "sfdc" => odbc_result($rs, "sfdc_id")
	);
  }
  odbc_close($odbc);
  if(isset($result)) return $result;
}

function findOrgByName($name) {
  $odbc = odbcConnect();
  $sql = "SELECT * from organizations where name='$name'";
  $rs = odbc_exec($odbc, $sql);
  if(odbc_fetch_row($rs)) {
    $result = array(
	"id" => odbc_result($rs, "id"),
	"name" => odbc_result($rs, "name"),
	"status" => odbc_result($rs, "status"),
	"seats" => odbc_result($rs, "seats"),
	"tier" => odbc_result($rs, "tier"),
	"server" => odbc_result($rs, "server"),
	"comment" => odbc_result($rs, "comment"),
    	"sfdc" => odbc_result($rs, "sfdc_id")
	);
  }
  odbc_close($odbc);
  if(isset($result)) return $result;
}

function findAllOrgNames() {
  $odbc = odbcConnect();
  $rs = odbc_exec($odbc, "SELECT name from organizations");
  $result = array();
  while(odbc_fetch_row($rs)) {
    $result[] = odbc_result($rs, "name");
  }
  odbc_close($odbc);
  natcasesort($result);
  return $result;
}

function totalOrgCount() {
  $odbc = odbcConnect();
  $rs = odbc_exec($odbc, "SELECT count(*) from organizations");
  $total = odbc_result($rs,1);
  odbc_close($odbc);
  return $total;
}

function findOrgsByMatch($query, $limit, $page=1) {
  $result = array();
  $odbc = odbcConnect();
  $rs = odbc_exec($odbc, "SELECT count(*) from organizations where name like '%$query%'");
  $result["total"] = odbc_result($rs,1);
  $sql = "SELECT * from organizations where name like '%$query%' order by created desc";
  $rs = odbc_exec($odbc, $sql);
  $count = ($limit*($page-1))+1;
  $rows = 0;
  while(odbc_fetch_row($rs, $count) && ($rows < $limit) ) {
    $id = odbc_result($rs, "id");
    $count += 1;
    $rows += 1;
    $result[] = array(
	"id" => $id,
	"created" => odbc_result($rs, "created"),
   	"name" => odbc_result($rs, "name"),
        "status" => odbc_result($rs, "status"),
    	"seats" => odbc_result($rs, "seats"),
    	"tier" => odbc_result($rs, "tier"),
    	"server" => odbc_result($rs, "server"),
    	"sfdc" => odbc_result($rs, "sfdc_id")
    );
  }
 $count = $count - 1; //we didn't use the last count
 $count = $count - ($limit*($page-1));
  $result["count"] = $count;
  odbc_close($odbc);
  return $result;
}

/*
   * createAccount(values): Create user account; return id.
   * updateAccount(id, values): Update account info.
   * deleteAccount(id): Delete an account.
   * findAccountsByUser(id): Find accounts belonging to user.
   * findAccountsByOrg(id): Find accounts belonging to org.
*/

function createAccount($values) {
  global $lastError;
  $odbc = odbcConnect();

  // Check if this acct already exists and fail if so
  $stmt = odbc_prepare($odbc, "SELECT * FROM org_members WHERE member_id=? and org_id=? AND member_type=?");
  $rs = odbc_execute($stmt, array($values["userid"],$values["orgid"],$values["type"]));
  if(!$rs) {
    odbc_close($odbc);
    return;
  }
  if(odbc_fetch_row($stmt)) {
    odbc_close($odbc);
    $lastError= "<b>Error:</b> Membership for {$values['userName']} in {$values['orgName']} already exists";
    return;
  }

  $id = uuid();
  $stmt = odbc_prepare($odbc, "INSERT into org_members(id, member_type, member_id, member_name, org_id, org_name, status, role_name, created) VALUES(?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP())");
  $rs = odbc_execute($stmt, array($id, $values["type"], $values["userid"], $values["userName"], $values["orgid"], $values["orgName"], $values["status"], $values["role"]));
  odbc_close($odbc);
  if($rs) return $id;
}

function updateOrgAccount($id, $values) {
  if ($values['sfdc'] != "") {
      updateSF('org_members', $id, $values['sfdc']);
  }
  $odbc = odbcConnect();
  $stmt = odbc_prepare($odbc, "UPDATE org_members SET status=?, role_name=? WHERE id=?");
  $rs = odbc_execute($stmt, array($values["status"], $values["role"], $id));
  odbc_close($odbc);
  return $rs;
}

function updateGroupAccount($id, $values) {
  $odbc = odbcConnect();
  $stmt = odbc_prepare($odbc, "UPDATE group_members SET status=? WHERE id=?");
  $rs = odbc_execute($stmt, array($values["status"], $id));
  odbc_close($odbc);
  return $rs;
}

function deleteGroupAccount($id) {
  $odbc = odbcConnect();
  $stmt = odbc_prepare($odbc, "DELETE FROM group_members WHERE id = ?");
  $rs = odbc_execute($stmt, array($id));
  odbc_close($odbc);
  return $rs;
}

function deleteOrgAccount($id) {
  $account = findOrgAccountById($id);
  if ($account['sfdc'] != "") {
    updateSF('org_members', $account['id'], $account['sfdc']);
  }
  $odbc = odbcConnect();
  $stmt = odbc_prepare($odbc, "DELETE FROM org_members WHERE id = ?");
  $rs = odbc_execute($stmt, array($id));
  odbc_close($odbc);
  return $rs;
}

function deleteAccount($id) {
  $acct = findOrgAccountById($id);
  if(isset($acct)) deleteOrgAccount($id);
  $acct = findGroupAccountById($id);
  if(isset($acct)) deleteGroupAccount($id);
}

function findAccountById($id) {
  $odbc = odbcConnect();
  $sql="SELECT * from org_members where id='$id'";
  $rs = odbc_exec($odbc, $sql);
  if(odbc_fetch_row($rs)) {
    $result = array(
	"id" => odbc_result($rs, "id"),
    	"created" => odbc_result($rs, "created"),
    	"type" => odbc_result($rs, "member_type"),
    	"userid" => odbc_result($rs, "member_id"),
    	"userName" => odbc_result($rs, "member_name"),
	"orgid" => odbc_result($rs,"org_id"),
	"orgName" => odbc_result($rs, "org_name"),
    	"status" => odbc_result($rs, "status"),
    	"role" => odbc_result($rs, "role_name"),
    	"sfdc" => odbc_result($rs, "sfdc_id")
    );
  }
  odbc_close($odbc);
  if(isset($result)) return $result;
}

function findOrgAccountById($id) {
  $odbc = odbcConnect();
  $sql="SELECT * from org_members where id='$id'";
  $rs = odbc_exec($odbc, $sql);
  if(odbc_fetch_row($rs)) {
    $result = array(
	"id" => odbc_result($rs, "id"),
    	"created" => odbc_result($rs, "created"),
    	"type" => odbc_result($rs, "member_type"),
    	"memberid" => odbc_result($rs, "member_id"),
    	"memberName" => odbc_result($rs, "member_name"),
	"orgid" => odbc_result($rs,"org_id"),
	"orgName" => odbc_result($rs, "org_name"),
    	"status" => odbc_result($rs, "status"),
    	"role" => odbc_result($rs, "role_name"),
    	"sfdc" => odbc_result($rs, "sfdc_id")
    );
  }
  odbc_close($odbc);
  if(isset($result)) return $result;
}
function findGroupAccountById($id) {
  $odbc = odbcConnect();
  $sql="SELECT * from group_members where id='$id'";
  $rs = odbc_exec($odbc, $sql);
  if(odbc_fetch_row($rs)) {
    $result = array(
	"id" => odbc_result($rs, "id"),
    	"created" => odbc_result($rs, "created"),
    	"type" => odbc_result($rs, "member_type"),
    	"memberid" => odbc_result($rs, "member_id"),
    	"memberName" => odbc_result($rs, "member_name"),
	"groupid" => odbc_result($rs,"group_id"),
	"groupName" => odbc_result($rs, "group_name"),
    );
  }
  odbc_close($odbc);
  if(isset($result)) return $result;
}

function findAccountByUserAndOrg($uid, $oid) {
  $odbc = odbcConnect();
  $sql="SELECT * from org_members where member_id='$uid' and org_id='$oid' and member_type='user'";
  $rs = odbc_exec($odbc, $sql);
  if(odbc_fetch_row($rs)) {
    $result = array(
	"id" => odbc_result($rs, "id"),
    	"created" => odbc_result($rs, "created"),
    	"type" => odbc_result($rs, "member_type"),
    	"userid" => odbc_result($rs, "member_id"),
    	"userName" => odbc_result($rs, "member_name"),
	"orgid" => odbc_result($rs,"org_id"),
	"orgName" => odbc_result($rs, "org_name"),
    	"status" => odbc_result($rs, "status"),
    	"role" => odbc_result($rs, "role_name"),
    	"sfdc" => odbc_result($rs, "sfdc_id")
    );
  }
  odbc_close($odbc);
  if(isset($result)) return $result;
}

function findAccountsByUser($id) {
  $odbc = odbcConnect();
  $sql="SELECT * from org_members where member_id='$id' and member_type = 'user' order by created desc";
  $rs = odbc_exec($odbc, $sql);
  $result = array();
  while(odbc_fetch_row($rs)) {
    $result[] = array(
	"id" => odbc_result($rs, "id"),
    	"created" => odbc_result($rs, "created"),
    	"type" => odbc_result($rs, "member_type"),
    	"userid" => odbc_result($rs, "member_id"),
    	"userName" => odbc_result($rs, "member_name"),
	"orgid" => odbc_result($rs,"org_id"),
	"orgName" => odbc_result($rs, "org_name"),
    	"status" => odbc_result($rs, "status"),
    	"role" => odbc_result($rs, "role_name"),
    	"sfdc" => odbc_result($rs, "sfdc_id")
    );
  }
  odbc_close($odbc);
  return $result;
}

function findGroupAccountsByUser($id) {
  $odbc = odbcConnect();
  $sql="SELECT * from group_members where member_id='$id' and member_type = 'user' order by created desc";
  $rs = odbc_exec($odbc, $sql);
  $result = array();
  while(odbc_fetch_row($rs)) {
    $result[] = array(
	"id" => odbc_result($rs, "id"),
    	"created" => odbc_result($rs, "created"),
    	"type" => odbc_result($rs, "member_type"),
    	"userid" => odbc_result($rs, "member_id"),
    	"userName" => odbc_result($rs, "member_name"),
	"orgid" => odbc_result($rs,"group_id"),
	"orgName" => odbc_result($rs, "group_name"),
    	"status" => odbc_result($rs, "status"),
    );
  }
  odbc_close($odbc);
  return $result;
}

function findGroupAccountsByGroupId($id) {
  $odbc = odbcConnect();
  $sql="SELECT * from group_members where group_id='$id'";
  $rs = odbc_exec($odbc, $sql);
  $result = array();
  while(odbc_fetch_row($rs)) {
    $result[] = array(
	"id" => odbc_result($rs, "id"),
    	"created" => odbc_result($rs, "created"),
    	"type" => odbc_result($rs, "member_type"),
    	"userid" => odbc_result($rs, "member_id"),
    	"userName" => odbc_result($rs, "member_name"),
	"groupid" => odbc_result($rs,"group_id"),
	"groupName" => odbc_result($rs, "group_name"),
    	"status" => odbc_result($rs, "status"),
    );
  }
  odbc_close($odbc);
  return $result;
}

function findOrgAccountsByGroupId($groupid) {
  $odbc = odbcConnect();
  $sql="SELECT * from org_members where member_id='$groupid' and member_type='group'";
  $rs = odbc_exec($odbc, $sql);
  $result = array();
  while(odbc_fetch_row($rs)) {
    $result[] = array(
	"id" => odbc_result($rs, "id"),
    	"created" => odbc_result($rs, "created"),
    	"type" => odbc_result($rs, "member_type"),
    	"userid" => odbc_result($rs, "member_id"),
    	"userName" => odbc_result($rs, "member_name"),
	"orgid" => odbc_result($rs,"org_id"),
	"orgName" => odbc_result($rs, "org_name"),
    	"status" => odbc_result($rs, "status"),
    	"role" => odbc_result($rs, "role_name"),
    	"sfdc" => odbc_result($rs, "sfdc_id")
    );
  }
  odbc_close($odbc);
  return $result;
}

function findAccountsByOrg($id) {
  $odbc = odbcConnect();
  $sql="SELECT * from org_members where org_id='$id' order by created desc";
  $rs = odbc_exec($odbc, $sql);
  $result = array();
  while(odbc_fetch_row($rs)) {
    $result[] = array(
	"id" => odbc_result($rs, "id"),
    	"created" => odbc_result($rs, "created"),
    	"type" => odbc_result($rs, "member_type"),
    	"userid" => odbc_result($rs, "member_id"),
    	"userName" => odbc_result($rs, "member_name"),
	"orgid" => odbc_result($rs,"org_id"),
	"orgName" => odbc_result($rs, "org_name"),
    	"status" => odbc_result($rs, "status"),
    	"role" => odbc_result($rs, "role_name"),
    	"sfdc" => odbc_result($rs, "sfdc_id")
    );
  }
  odbc_close($odbc);
  return $result;
}

function findTotalAccountsInOrg($id) {
  $odbc = odbcConnect();
  $sql="SELECT count(*) from org_members where org_id='$id'";
  $rs = odbc_exec($odbc, $sql);
  if(odbc_fetch_row($rs)) {
    $result = odbc_result($rs, "count(*)");
  } else {
    $result = 0;
  }
  odbc_close($odbc);
  return $result;
}



/****************************************************************************/

function makeOrgsTable($query, $limit, $showLimit, $page=1) {
  global $allTiers;
  $includeTier = (count($allTiers) > 1);
  $matches = findOrgsByMatch($query, $limit, $page);
  $total = $matches["total"];
  $count = $matches["count"];
  if($showLimit) {
    echo "<b>Total number of results:</b> $total";
    if($total > $limit) {
      echo " (first $limit matches shown below; show ";
      echo "<a href=\"findorg.php?limit=10\">10</a>, ";
      echo "<a href=\"findorg.php?limit=20\">20</a>, ";
      echo "<a href=\"findorg.php?limit=50\">50</a>, ";
      echo "<a href=\"findorg.php?limit=100\">100</a>";
      echo ")";
    }
  }
  echo '<table cellpadding="0" cellspacing="0" class="alternate">';
  echo "<tr> 
	<th>Date</th> 
	<th>Organization</th> 
	<th>Members</th> 
	<th>Seats</th> 
	<th>Status</th>";
    if ($includeTier) {
        echo "<th>Tier</th>";
    };        
  for($index=0; $index < $count; $index+=1) {
    $form = $matches[$index];
    $used = findTotalAccountsInOrg($form['id']);
    $created = explode(" ", $form["created"]);
    $created = $created[0];
    echo "<tr>
  	<td>$created</td>
  	<td><a href=\"editorg.php?id={$form['id']}\">{$form['name']}</a></td>
  	<td>$used</td>
  	<td>{$form['seats']}</td>
	<td>{$form['status']}</td>";
    if ($includeTier) {
  	echo "<td>{$form['tier']}</td>";
    }
  }
  echo "</table>";
  return $total;
}

function makeUsersTable($query, $limit, $showLimit, $page=1) {
  $matches = findUsersByMatch($query, $limit, $page);
  $count = $matches["count"];
  $total = $matches["total"];
  if($showLimit) {
    echo "<b>Total number of results:</b> $total";
    if($total > $limit) {
      echo " (first $limit matches shown below; show ";
      echo "<a href=\"finduser.php?limit=10\">10</a>, ";
      echo "<a href=\"finduser.php?limit=20\">20</a>, ";
      echo "<a href=\"finduser.php?limit=50\">50</a>, ";
      echo "<a href=\"finduser.php?limit=100\">100</a>";
      echo ")";
    }
  }
  echo '<table cellpadding="0" cellspacing="0" class="alternate">';
  echo "<tr> <th>Date</th> <th>User ID</th> <th>First</th> <th>Last</th> <th>Company</th> </tr>";
  for($index = 0; $index < $count; $index += 1) {
    $form = $matches[$index];
    $created = explode(" ", $form["created"]);
    $created = $created[0];
    $sfdc = $form["sfdc"];
    echo "<tr>
  	<td>$created</td>
  	<td><a href=\"edituser.php?id=${form['id']}\">{$form['name']}</a></td>
  	<td>{$form['firstName']}</td>
  	<td>{$form['lastName']}</td>
  	<td>{$form['company']}</td>";
  }
  echo "</table>";
  return $total;
}

function makeGroupsTable($query, $limit, $showLimit, $page=1) {
  $matches = findGroupsByMatch($query, $limit, $page);
  $count = $matches["count"];
  $total = $matches["total"];
  if($showLimit) {
    echo "<b>Total number of results:</b> $total";
    if($total > $limit) {
      echo " (first $limit matches shown below; show ";
      echo "<a href=\"findgroup.php?limit=10\">10</a>, ";
      echo "<a href=\"findgroup.php?limit=20\">20</a>, ";
      echo "<a href=\"findgroup.php?limit=50\">50</a>, ";
      echo "<a href=\"findgroup.php?limit=100\">100</a>";
      echo ")";
    }
  }
  echo '<table cellpadding="0" cellspacing="0" class="alternate">';
  echo "<tr> <th>Date</th> <th>Group Name</th> <th>Status</th> </tr>";
  for($index = 0; $index < $count; $index += 1) {
    $form = $matches[$index];
    $created = explode(" ", $form["created"]);
    $created = $created[0];
    echo "<tr>
  	<td>$created</td>
  	<td><a href=\"editgroup.php?id=${form['id']}\">{$form['name']}</a></td>
  	<td>{$form['status']}</td>";
  }
  echo "</table>";
  return $total;
}

/****************************************************************************/

function makeOrgAccountTable($orgid, $url) {
  $accounts = findAccountsByOrg($orgid);
  echo '<table cellpadding="0" cellspacing="0" class="alternate">';
  echo "<thead>";
  echo "<tr>
	<th>Created</th>
	<th>User ID</th> 
	<th>Role</th> 
	<th>Status</th>
        <th>Action</th>
  </tr>";
  echo '</thead><tbody>';
  echo "<form method=\"POST\" action=\"$url?orgid=$orgid\">";
  echo '<tr><td>';
  echo date("Y-m-d");
  echo '</td>';

  echo '<td><table style="border: 0px none ; border-spacing: 5px;"><tr><td style="background: white;">';
  if(totalUserCount() < 9999 and totalGroupCount() < 9999) {
    echo '<select id="userName" name="userName" style="width: 250px">';
    echo "<option value=\"\">-- Select User / Group --</option>";
    foreach(findAllGroupNames() as $name)
      echo "<option value=\"$name\">$name (group)</option>\n";
    echo "<option value=\"\">-- Registered Users --</option>";
    foreach(findAllUserNames() as $name)
      echo "<option>$name</option>\n";
    echo "</select>";
  } else {
    echo '<input type="text" name="userName" class="text">';
  }
  echo '</td><td style="background: white; "></td></tr></table>';

  echo '<td><select name="role" title="User Role">
  	<option value="admin">Administrator</option>
  	<option value="user">Member</option>
  	<option value="guest">Visitor</option>
	</select></td>';
  echo '<td>Invited</td>';
  echo '<td>
           <div class="button">
           <input type="submit" value="Add Member">
           </div>
        </td>';
  echo '</tr></form>';
  foreach($accounts as $form) {
    $created = explode(" ", $form["created"]);
    $created = $created[0];
    echo "<tr>\n<td>$created</td>\n";
    if($form['type'] == 'user') {
      echo "<td><a href=\"edituser.php?id={$form['userid']}\">{$form['userName']}</a></td>";
    } else {
      echo "<td><a href=\"editgroup.php?id={$form['userid']}\">{$form['userName']} (Group)</a></td>";
    }
    $isUser = $isGuest = $isAdmin = "";
    if($form['role'] == "user") $isUser = "selected";
    if($form['role'] == "guest") $isGuest = "selected";
    if($form['role'] == "admin") $isAdmin = "selected";
    echo "<td><select id=\"updateRole\" name=\"updateRole\" title=\"User Role\">
        <option value=\"{$form['id']}|admin\" $isAdmin>Administrator</option>
  	<option value=\"{$form['id']}|user\" $isUser>User</option>
  	<option value=\"{$form['id']}|guest\" $isGuest>Visitor</option>
	</select></td>";
    $isInvited = $isActive = $isDeleted = "";
    if($form['status'] == "Invited") $isInvited = "selected";
    if($form['status'] == "Active") $isActive = "selected";
    if($form['status'] == "Deleted") $isDeleted = "selected";
    echo "<td><select id=\"updateStatus\" name=\"updateStatus\" title=\"User Status\">
  	<option value=\"{$form['id']}|Invited\" $isInvited>Invited</option>
  	<option value=\"{$form['id']}|Active\" $isActive>Active</option>
  	<option value=\"{$form['id']}|Deleted\" $isDeleted>Deactivated</option>
	</select></td>";
    echo "<td>
            <form method=\"POST\" class=\"slimform\" action=\"deleteacct.php?id=${form['id']}&from=org\">
            <div class=\"button\">
            <input type=\"submit\" value=\"Remove Member\">
            </div></form>
         </td>";
    echo "</tr>";
  }
  echo "</tbody>";
  echo "</table>";
}

function makeGroupMemberTable($gid, $url) {
  $accounts = findGroupAccountsByGroupId($gid);
  echo '<table cellpadding="0" cellspacing="0" class="alternate">';
  echo "<thead>";
  echo "<tr>
	<th>Created</th>
	<th>Member</th> 
	<th>Status</th>
        <th>Action</th>
  </tr>";
  echo '</thead><tbody>';
  echo "<form method=\"POST\" action=\"$url?groupid=$gid\">";
  echo '<tr><td>';
  echo date("Y-m-d");
  echo '</td>';

  echo '<td><table style="border: 0px none ; border-spacing: 5px;"><tr><td style="background: white;">';
  if(totalUserCount() < 9999) {
    echo '<select id="userName" name="userName" style="width: 250px">';
    echo "<option value=\"\">-- Select User --</option>";
    foreach(findAllUserNames() as $name)
      echo "<option>$name</option>\n";
    echo "</select>";
    echo "</select>";
  } else {
    echo '<input type="text" name="userName" class="text">';
  }
  echo '</td><td style="background: white; "></td></tr></table>';

  echo '<td>Invited</td>';
  echo '<td>
           <div class="button">
           <input type="submit" value="Add Member">
           </div>
        </td>';
  echo '</tr></form>';
  foreach($accounts as $form) {
    $created = explode(" ", $form["created"]);
    $created = $created[0];
    echo "<tr>\n<td>$created</td>\n";
    if($form['type'] == 'user') {
      echo "<td><a href=\"edituser.php?id={$form['userid']}\">{$form['userName']}</a></td>";
    } else {
      echo "<td><a href=\"editgroup.php?id={$form['userid']}\">{$form['userName']} (Group)</a></td>";
    }
    $isInvited = $isActive = $isDeleted = "";
    if($form['status'] == "Invited") $isInvited = "selected";
    if($form['status'] == "Active") $isActive = "selected";
    if($form['status'] == "Deleted") $isDeleted = "selected";
    echo "<td><select id=\"updateStatus\" name=\"updateStatus\" title=\"User Status\">
  	<option value=\"{$form['id']}|Invited\" $isInvited>Invited</option>
  	<option value=\"{$form['id']}|Active\" $isActive>Active</option>
  	<option value=\"{$form['id']}|Deleted\" $isDeleted>Deactivated</option>
	</select></td>";
    echo "<td>
            <form method=\"POST\" class=\"slimform\" action=\"deleteacct.php?id=${form['id']}&from=group\">
            <div class=\"button\">
            <input type=\"submit\" value=\"Remove Member\">
            </div></form>
         </td>";
    echo "</tr>";
  }
  echo "</tbody>";
  echo "</table>";
}


function makeGroupOwnerTable($groupid, $url) {
  $groups = findOrgAccountsByGroupId($groupid);
  echo '<table cellpadding="0" cellspacing="0" class="alternate">';
  echo "<tr>
	<th>Created</th>
	<th>Organization</th> 
	<th>Role</th> 
	<th>Status</th>
        <th>Action</th>
  </tr>";

  echo "<form method=\"POST\" action=\"$url?groupid=$groupid\">";
  echo '<tr><td>';
  echo date("Y-m-d");
  echo '</td>';

  echo '<td><table style="border: 0px none ; border-spacing: 5px;"><tr><td style="background: white;">';
  if(totalOrgCount() < 9999) {
    echo '<select id="orgName" name="orgName" style="width: 250px">';
    echo "<option value=\"\">-- Select Organization --</option>";
    foreach(findAllOrgNames() as $name)
      echo "<option>$name</option>\n";
    echo "</select>";
  } else {
    echo '<input type="text" name="userName" class="text">';
  }
  echo '</td><td style="background: white; "></td></tr></table>';

  echo '<td><select name="role" title="User Role">
  	<option value="admin">Administrator</option>
  	<option value="user">Member</option>
  	<option value="guest">Visitor</option>
	</select></td>';
  echo '<td>Invited</td><td><div class="button"><input type="submit" value="Add Member"></div></td>';
  echo '</tr></form>';


  foreach($groups as $form) {
    $created = explode(" ", $form["created"]);
    $created = $created[0];
    echo "<tr>
  	<td>$created</td>
  	<td><a href=\"editorg.php?id={$form['orgid']}\">{$form['orgName']}</a></td>";
    $isUser = $isGuest = $isAdmin = "";
    if($form['role'] == "user") $isUser = "selected";
    if($form['role'] == "guest") $isGuest = "selected";
    if($form['role'] == "admin") $isAdmin = "selected";
    echo "<td><select id=\"updateRole\" name=\"updateRole\" title=\"User Role\">
  	<option value=\"{$form['id']}|admin\" $isAdmin>Administrator</option>
  	<option value=\"{$form['id']}|user\" $isUser>User</option>
  	<option value=\"{$form['id']}|guest\" $isGuest>Visitor</option>
	</select></td>";
    $isInvited = $isActive = $isDeleted = "";
    if($form['status'] == "Invited") $isInvited = "selected";
    if($form['status'] == "Active") $isActive = "selected";
    if($form['status'] == "Deleted") $isDeleted = "selected";
    //$statusClass = strtolower($form['status']);
    echo "<td><select id=\"updateStatus\" name=\"updateStatus\" title=\"User Status\">
  	<option value=\"{$form['id']}|Invited\" $isInvited>Invited</option>
  	<option value=\"{$form['id']}|Active\" $isActive>Active</option>
  	<option value=\"{$form['id']}|Deleted\" $isDeleted>Deactivated</option>
	</select></td>";

    echo "<td>
            <form method=\"POST\" class=\"slimform\" action=\"deleteacct.php?id=${form['id']}&from=group\">
            <div class=\"button\">
            <input type=\"submit\" value=\"Remove Member\">
            </div></form>
         </td>";
    echo "</tr>";
  }
  echo "</table>";
}


function makeUserAccountTable($userid, $url) {
  $accounts = findAccountsByUser($userid);
  $groups = findGroupAccountsByUser($userid);
  echo '<table cellpadding="0" cellspacing="0" class="alternate">';
  echo "<thead><tr>
	<th>Created</th>
	<th>Organization</th> 
	<th>Role</th> 
	<th>Status</th>
        <th>Action</th>
  </tr></thead><tbody>";

  echo "<form method=\"POST\" action=\"$url?userid=$userid\">";
  echo '<tr><td>';
  echo date("Y-m-d");
  echo '</td>';

  echo '<td><table style="border: 0px none ; border-spacing: 5px;"><tr><td style="background: white;">';
  if(totalOrgCount() < 9999 and totalGroupCount() < 9999) {
    echo '<select id="orgName" name="orgName" style="width: 250px">';
    echo "<option value=\"\">-- Select Group / Organization --</option>";
    foreach(findAllGroupNames() as $name)
      echo "<option value=\"$name\">$name (group)</option>\n";
    echo "<option value=\"\">-- Organizations --</option>";
    foreach(findAllOrgNames() as $name)
      echo "<option>$name</option>\n";
    echo "</select>";
  } else {
    echo '<input type="text" name="userName" class="text">';
  }
  echo '</td><td style="background: white; "></td></tr></table>';

  echo '<td><select name="role" title="User Role">
  	<option value="admin">Administrator</option>
  	<option value="user">Member</option>
  	<option value="guest">Visitor</option>
	</select></td>';
  echo '<td>Invited</td><td><div class="button"><input type="submit" value="Add Member"></div></td>';
  echo '</tr></form>';


  foreach($groups as $form) {
    $created = explode(" ", $form["created"]);
    $created = $created[0];
    echo "<tr>
  	<td>$created</td>
  	<td><a href=\"editgroup.php?id={$form['orgid']}\">{$form['orgName']} (Group)</a></td>";
    echo "<td></td>"; // skip role
    $isInvited = $isActive = $isDeleted = "";
    if($form['status'] == "Invited") $isInvited = "selected";
    if($form['status'] == "Active") $isActive = "selected";
    if($form['status'] == "Deleted") $isDeleted = "selected";
    echo "<td><select id=\"updateStatus\" name=\"updateStatus\" title=\"User Status\">
  	<option value=\"{$form['id']}|Invited\" $isInvited>Invited</option>
  	<option value=\"{$form['id']}|Active\" $isActive>Active</option>
  	<option value=\"{$form['id']}|Deleted\" $isDeleted>Deactivated</option>
	</select></td>";
    echo "<td class=\"actions\">
            <form method=\"POST\" class=\"slimform\" action=\"deleteacct.php?id=${form['id']}&from=user\">
            <div class=\"button\">
            <input type=\"submit\" value=\"Remove Member\">
            </div>
            </form>
         </td>";
    echo "</tr>";
  }

  foreach($accounts as $form) {
    $created = explode(" ", $form["created"]);
    $created = $created[0];
    echo "<tr>
  	<td>$created</td>
  	<td><a href=\"editorg.php?id={$form['orgid']}\">{$form['orgName']}</a></td>";
    $isUser = $isGuest = $isAdmin = "";
    if($form['role'] == "user") $isUser = "selected";
    if($form['role'] == "guest") $isGuest = "selected";
    if($form['role'] == "admin") $isAdmin = "selected";
    echo "<td><select id=\"updateRole\" name=\"updateRole\" title=\"User Role\">
  	<option value=\"{$form['id']}|admin\" $isAdmin>Administrator</option>
  	<option value=\"{$form['id']}|user\" $isUser>User</option>
  	<option value=\"{$form['id']}|guest\" $isGuest>Visitor</option>
	</select></td>";
    $isInvited = $isActive = $isDeleted = "";
    if($form['status'] == "Invited") $isInvited = "selected";
    if($form['status'] == "Active") $isActive = "selected";
    if($form['status'] == "Deleted") $isDeleted = "selected";
    //$statusClass = strtolower($form['status']);
    echo "<td><select id=\"updateStatus\" name=\"updateStatus\" title=\"User Status\">
  	<option value=\"{$form['id']}|Invited\" $isInvited>Invited</option>
  	<option value=\"{$form['id']}|Active\" $isActive>Active</option>
  	<option value=\"{$form['id']}|Deleted\" $isDeleted>Deactivated</option>
	</select></td>";

    echo "<td>
            <form method=\"POST\" class=\"slimform\" action=\"deleteacct.php?id=${form['id']}&from=user\">
            <div class=\"button\">
            <input type=\"submit\" value=\"Remove Member\">
            </div></form>
         </td>";
    echo "</tr>";
  }
  echo "</tbody></table>";
}

?>
