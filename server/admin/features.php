<?php

include_once("serverapi.php");
include("config.php");

$serverAddr = $allServers[0][2];

function getFeature($featureName) {
  $license = file_get_contents("http://localhost/admin/license.php", 0);
  $xml = new SimpleXMLElement($response);
  return $xml->featureName;
}

function printFeature($name, $valid, $info) {
    echo "<tr><td width='200'>$name</td>";
    echo "<td>";
    if(!$valid) {
      echo '<span style="color:red"><b>';
      echo $info;
      echo '</b></span>';
    } else echo $info;
    echo "</td></tr>";
}

function getLicenseOwner() {
  @$response = file_get_contents("http://localhost/admin/license.php", 0);
  if($response) {
    try {
      $features = @new SimpleXMLElement($response);
    } catch (Exception $e) {
      // nuttin
    }
  }
  if(isset($features)) {
    return "Registered To ".$features->RegisteredTo;
  }
  return '<span style="color: red">UNLICENSED COPY</span>';
}

function printFeatureInfo() {
  @$response = file_get_contents("http://localhost/admin/license.php", 0);
  if(!$response) {
    $response = "Unable to retrieve license file";
  } else {
    try {
      $features = @new SimpleXMLElement($response);
    } catch (Exception $e) {
      // nuttin
    }
  }
  if(!isset($features)) {
    printFeature("License", false, $response);
    return;
  }

  printFeature("Registered To", true, $features->RegisteredTo);

  $expressMax = $features->Express;
  $spMax = $features->ServiceProvider;
  $routerMax = $features->Router;
  $appMax = $features->AppServer;
  $videoMax = $features->VideoServer;
  $rtspMax = $features->WebcastServer;
  $expDate = strtotime($features->ContractEndDate);

  $curDate = strtotime("now");
  # Assume license is outdated
  $valid = false;
  if ( $expDate >= $curDate ) $valid = true;
  printFeature("Expiration Date", $valid, date('d M Y', $expDate));
  
  $expressCount = 0;
  $spCount = 0;
  $routerCount = 0;
  $appCount = 0;
  $videoCount = 0;
  $rtspCount = 0;

  foreach(listServers() as $server) {
    if($server['status'] == 'active') {
      switch($server['role']) {
      case 'allInOne': $expressCount++; break;
      case 'spHost': $spCount++; break;
      case 'routerHost': $routerCount++; break;
      case 'appHost': $appCount++; break;
      case 'videoHost': $videoCount++; break;
      case 'rtspHost': $rtspCount++; break;
      case 'webcastHost': $rtspCount++; break;
      }
    }
  }

  $namedSeats = totalUserCount();
  $props = getServerConfigProperties('localhost', array("OpenQwaq.AD.Enabled"));
  $hasAD = ($props['OpenQwaq.AD.Enabled']) == 'true';

  $roles = array(
		 array("Named Seats", $namedSeats, $features->NamedSeats),
		 array("Express Edition", $expressCount, $expressMax),
		 array("Service Provider", $spCount, $spMax),
		 array("Router / Balancer", $routerCount, $routerMax),
		 array("Application Server", $appCount, $appMax),
		 array("Video Server", $videoCount, $videoMax),
		 array("Webcast Server", $rtspCount, $rtspMax),
		 );

  foreach($roles as $role) {
    if($role[1] > 0 or $role[2] > 0) {
      printFeature($role[0], $role[1] <= $role[2], $role[1]." / ".$role[2]);
    }
  }
}

?>
