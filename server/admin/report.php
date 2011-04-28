<?php
include("serverapi.php");

$props = getServerConfigProperties("localhost", array("OpenQwaq.SP.SupportEmail"));
$email = $props["OpenQwaq.SP.SupportEmail"];

$hostname = `/bin/hostname`;
$hostname = rtrim($hostname);

function newDataSet() {
  return array(
    // total number of logins
    "logins" => 0,
    // new forums created
    "newForums" => 0,
    // active forums
    "forums" => array(),
    // number of apps launched
    "appsLaunched" => 0,
    // types of apps launched
    "apps" => array(),
    // Number of web downloads
    "webDownloads" => 0,
    // Number of web uploads
    "webUploads" => 0,
    // Max. number of concurrent users
    "maxConcurrentUsers" => 0,
    // Time spent alone
    "soloTime" => 0,
    // Time spent in group situations
    "groupTime" => 0,
    // Number of meetings held (>= 2 users)
    "meetingCount" => 0,
    // Users present
    "present" => array(),

    );
}

function sortByDate($a, $b) {
  return strcasecmp($a['date'], $b['date']);
}

function analyzeData($logStart, $logStop) {
  $forumData = array();
  $orgData = array();
  $userData = array();
  $globalData = newDataSet();
  $meetingPlace = array();

  $odbc = odbcConnect();

  $start = time();
  echo "\nCounting Total Users"; flush();
  $rs = odbc_exec($odbc, 
		  "SELECT count(*) from users"
		  );
  $count = odbc_result($rs, 1);
  odbc_close($odbc);

  $globalData["totalUsers"] = $count;

  $where = "WHERE ";

  // NOTE: The comparisons below are carefully chosen.
  // The reporing tool only runs *after* the period it reports on is
  // past, so it never includes the day that the script is executed on
  // In other words, a montly report run on October 1st, needs to report
  // the period starting from (and INCLUDING) Sep 1st through Sep 30st
  // and EXCLUDING Oct 1st.
  //
  // This is counter-intuitive if you want to run the report using -daily
  // since you won't get the stats for today. In order to get the stats
  // for today (which isn't over yet) you need to run the script with
  // tomorrow's date as an argument.

  if($logStart <> "") $where = $where . "date(logdate) >= '$logStart' AND ";
  if($logStop <> "")  $where = $where . "date(logdate) < '$logStop' AND ";

  $odbc = odbcLogConnect();

  // -------------------------------------------------------------
  // Meeting analysis
  // -------------------------------------------------------------

  // First we establish a timeline for all meetings that have
  // happened by collecting enter/leave events for each recorded
  // log event (forumsLogin, forumsLogout, teleport)

  echo " - ". (time() - $start) . " secs "; $start = time();
  echo "\nStarting global meeting analysis";
  $count = 0;
  $meetings = array();

  $rs = odbc_exec($odbc, 
		  "SELECT * ".
		  "FROM sp_activity_log ".
		  $where . "action in ".
		  "('forumsLogin', 'forumsLogout', 'teleport') "
		  );
  while(odbc_fetch_row($rs)) {
    if($count % 100 == 0) {
      echo "(".count($meetings).")";
      flush();
    }
    $count = $count + 1;

    $action = odbc_result($rs, "action");
    $user = odbc_result($rs, "user");
    $oid = odbc_result($rs, "org");
    $extra = odbc_result($rs, "extra");
    $date = odbc_result($rs, "logdate");

    $org = findOrgById($oid);
    if(isset($org)) $org = $org["name"];
    else $org = $oid;

    $forum = $org."/".$extra;

    if(!isset($userData[$user])) $userData[$user] = newDataSet();
    if(!isset($orgData[$org])) $orgData[$org] = newDataSet();
    if(!isset($forumData[$forum])) $forumData[$forum] = newDataSet();

    // echo "\n\t$action: $extra ($user)";

    // skip teleport within the same forum
    if($action == 'teleport' && 
       isset($meetingPlace[$user]) && 
       ($meetingPlace[$user] == $forum)) {
      // echo " (skipped) ";
      continue;
    }

    if($action == 'forumsLogin') {
      // Keep a record of the uses being present in the org/global
      $forumData[$forum]["present"][$user] = $user;
      $orgData[$org]["present"][$user] = $user;
      $globalData["present"][$user] = $user;

      // Count the number of logins
      $globalData["logins"] += 1;
      $orgData[$org]["logins"] += 1;
      $userData[$user]["logins"] += 1;
      $forumData[$forum]["logins"] += 1;

      // Update concurrent user count
      $orgData[$org]["maxConcurrentUsers"] = 
	max($orgData[$org]["maxConcurrentUsers"], 
	    count($orgData[$org]["present"]));

      $globalData["maxConcurrentUsers"] = 
	max($globalData["maxConcurrentUsers"], 
	    count($globalData["present"]));

      $forumData[$forum]["maxConcurrentUsers"] = 
	max($forumData[$forum]["maxConcurrentUsers"], 
	    count($forumData[$forum]["present"]));
    }

    if($action == 'forumsLogout') {
      // Clean out presence info for that user
      unset($orgData[$org]["present"][$user]);
      unset($globalData["present"][$user]);
      unset($forumData[$forum]["present"][$user]);
    }

    if($action == 'teleport' || $action == 'forumsLogout') {
      // process the last location for this user now
      // that we know where she went after being in her
      // previous meeting place.
      if(isset($meetingPlace[$user])) {
	$meeting = $meetingPlace[$user];
	$meetings[$meeting][] = array( 
	     "date" => $meetingStart[$user], 
	     "type" => "enter",
	     "user" => $user,
	     "org" => $org,
	     "forum" => $extra
	);
	$meetings[$meeting][] = array( 
	     "date" => $date, 
	     "type" => "leave",
	     "user" => $user,
	     "org" => $org,
	     "forum" => $extra
	);
      } else {
	// echo "\nFound $action without login: $org/$extra/$user\n";
      }
      unset($meetingStart[$user]);
      unset($meetingPlace[$user]);
    }

    if($action == 'teleport' || $action == 'forumsLogin') {
      $meetingStart[$user] = $date;
      $meetingPlace[$user] = $forum;

      // Count active forums
      if(isset($globalData["forums"][$forum]))
       $globalData["forums"][$forum] += 1;
      else
       $globalData["forums"][$forum] = 1;

      if(isset($userData[$user]["forums"][$forum]))
       $userData[$user]["forums"][$forum] += 1;
      else
       $userData[$user]["forums"][$forum] = 1;

      if(isset($orgData[$org]["forums"][$forum]))
       $orgData[$org]["forums"][$forum] += 1;
      else
       $orgData[$org]["forums"][$forum] = 1;
    }
  }

  echo " - ". (time() - $start) . " secs "; $start = time();

  //
  // Now that we have the events for each meeting do the analysis
  //

  foreach($meetings as $key => $events) {
    echo "\nAnalyzing Meetings in $key";
    usort($events, "sortByDate");
    $usersInForum = array();
    $usersInMeeting = array();
    $maxUsersInForum = 0;

    foreach($events as $action) {

      $user = $action['user'];
      $org = $action['org'];
      $forum = $action['org']."/".$action["forum"];

      // echo "{$action['type']}: $user / {$action['date']}\n";

      if($action['type'] == "enter") {
	$usersInForum[$user] = strtotime($action['date']);
	$usersInMeeting[$user] = $usersInForum[$user];

	if(count($usersInForum) == 2) {
	  // meeting starts

	  // update solo time for previous lone user
	  $soloTime = strtotime($action['date']) - $soloStart;
	  // echo "\n\tSolo Time for $user: $soloTime";
	  $userData[$soloUser]['soloTime'] += $soloTime;
	  $orgData[$org]['soloTime'] += $soloTime;
	  $globalData['soloTime'] += $soloTime;
	  $forumData[$forum]['soloTime'] += $soloTime;

	  // set meeting start date
	  $meetingStart = strtotime($action['date']);

	  // update the lone user's meeting start time
	  $usersInForum[$soloUser] = $meetingStart;
	  $usersInMeeting = $usersInForum;
	}
      }

      if($action['type'] == "leave") {

	if(count($usersInForum) > 1) {
	  // if there is at least one user left,
	  // update meeting time for the user leaving
	  $groupTime = strtotime($action['date']) - $usersInForum[$user];
	  $userData[$user]['groupTime'] += $groupTime;
	}

	unset($usersInForum[$user]);

	if(count($usersInForum) == 1) {
	  // meeting ends, update meeting end date for forum / org
	  $groupTime = strtotime($action['date']) - $meetingStart;

	  // echo "\n\tGroup Time for forum: $groupTime";

	  $globalData['groupTime'] += $groupTime;
	  $globalData['meetingCount'] += 1;

	  $orgData[$org]['groupTime'] += $groupTime;
	  $orgData[$org]['meetingCount'] += 1;

	  $forumData[$forum]['groupTime'] += $groupTime;
	  $forumData[$forum]['meetingCount'] += 1;

	  foreach(array_keys($usersInMeeting) as $user) {
	    $userData[$user]['meetingCount'] += 1;
	  }
	}

	if(count($usersInForum) == 1) {
	  // last user left; count solo time

	  $soloTime = strtotime($action['date']) - $soloStart;
	  // echo "\n\tSolo Time for $user: $soloTime";

	  $forumData[$forum]['soloTime'] += $soloTime;
	  $userData[$user]['soloTime'] += $soloTime;
	  $orgData[$org]['soloTime'] += $soloTime;
	  $globalData['soloTime'] += $soloTime;
	}

      }

      if(count($usersInForum) == 1) {
	// only one user left in forum after event
	// start counting solo time for that user
	$userNames = array_keys($usersInForum);
	$soloUser = $userNames[0];
	$soloStart = strtotime($action['date']);
      }

      // echo "\n\t\t{$action['type']}: ".implode(", ", array_keys($usersInForum));
    }
    echo " - ". (time() - $start) . " secs "; $start = time();
  }

  // -------------------------------------------------------------
  // Number of forums created
  // -------------------------------------------------------------

  echo " - ". (time() - $start) . " secs "; $start = time();
  echo "\nCounting New Forums"; flush();
  $rs = odbc_exec($odbc, 
		  "SELECT * ".
		  "FROM sp_activity_log ".
		  $where . "action = 'createService'"
		  );
  while(odbc_fetch_row($rs)) {
    $user = odbc_result($rs, "user");
    $org = odbc_result($rs, "org");
    $globalData["newForums"] +=1;
    if(!isset($userData[$user])) $userData[$user] = newDataSet();
    if(!isset($orgData[$org])) $orgData[$org] = newDataSet();
    $userData[$user]["newForums"] += 1;
    $orgData[$org]["newForums"] += 1;
  }

  // -------------------------------------------------------------
  // Number and types of apps used
  // -------------------------------------------------------------

  echo " - ". (time() - $start) . " secs "; $start = time(); 
  echo "\nCounting App Launches"; flush();
  $rs = odbc_exec($odbc, 
		  "SELECT * ".
		  "FROM sp_activity_log ".
		  $where . "action = 'launchApp'"
		  );
  while(odbc_fetch_row($rs)) {
    $user = odbc_result($rs, "user");
    $oid = odbc_result($rs, "org");
    $org = findOrgById($oid);
    $app = odbc_result($rs, "extra");
    if(isset($org)) $org = $org['name'];
    else $org = $oid;

    if(!isset($userData[$user])) $userData[$user] = newDataSet();
    if(!isset($orgData[$org])) $orgData[$org] = newDataSet();

    $globalData["appsLaunched"] +=1;
    $userData[$user]["appsLaunched"] += 1;
    $orgData[$org]["appsLaunched"] += 1;

    if(!isset($globalData["apps"][$app])) 
      $globalData["apps"][$app] = 1;
    else
      $globalData["apps"][$app] += 1;

    if(!isset($userData[$user]["apps"][$app])) 
      $userData[$user]["apps"][$app] = 1;
    else
      $userData[$user]["apps"][$app] += 1;

    if(!isset($orgData[$org]["apps"][$app])) 
      $orgData[$org]["apps"][$app] = 1;
    else
      $orgData[$org]["apps"][$app] += 1;
  }

  // -------------------------------------------------------------
  // Forum pages usage
  // -------------------------------------------------------------

  echo " - ". (time() - $start) . " secs "; $start = time();
  echo "\nCounting Web Downloads"; flush();
  $rs = odbc_exec($odbc, 
		  "SELECT * ".
		  "FROM sp_activity_log ".
		  $where . "action = 'webActivity' and extra like 'download:%'"
		  );
  while(odbc_fetch_row($rs)) {
    $user = odbc_result($rs, "user");
    $oid = odbc_result($rs, "org");
    $org = findOrgById($oid);
    if(isset($org)) $org = $org['name'];
    else $org = $oid;

    if(!isset($userData[$user])) $userData[$user] = newDataSet();
    if(!isset($orgData[$org])) $orgData[$org] = newDataSet();

    $globalData["webDownloads"] +=1;
    $userData[$user]["webDownloads"] += 1;
    $orgData[$org]["webDownloads"] += 1;
  }

  echo " - ". (time() - $start) . " secs "; $start = time();
  echo "\nCounting Web Uploads"; flush();
  $rs = odbc_exec($odbc, 
		  "SELECT * ".
		  "FROM sp_activity_log ".
		  $where . "action = 'upload'"
		  );
  while(odbc_fetch_row($rs)) {
    $user = odbc_result($rs, "user");
    $oid = odbc_result($rs, "org");
    $org = findOrgById($oid);
    if(isset($org)) $org = $org['name'];
    else $org = $oid;

    if(!isset($userData[$user])) $userData[$user] = newDataSet();
    if(!isset($orgData[$org])) $orgData[$org] = newDataSet();

    $globalData["webUploads"] +=1;
    $userData[$user]["webUploads"] += 1;
    $orgData[$org]["webUploads"] += 1;
  }

  echo " - ". (time() - $start) . " secs "; $start = time();
  echo "\nCounting QRL Launches"; flush();
  $rs = odbc_exec($odbc, 
		  "SELECT count(*) ".
		  "FROM sp_activity_log ".
		  $where . "action = 'qwq'"
		  );
  $count = odbc_result($rs, 1);
  $globalData["Web Launches (QRL)"] = $count;

  odbc_close($odbc);

  $results = array(
		   "global" => $globalData,
		   "orgs" => $orgData,
		   "users" => $userData,
		   "forums" => $forumData,
		   );
  return $results;
}

function printTime($seconds) {
  $mins = intval($seconds / 60);
  $hrs = intval($mins / 60);
  $days = intval($hrs / 24);
  if($days > 0) $days = "$days:";
  else $days = "";
  return 
    $days.
    str_pad($hrs % 24, 2, "0", STR_PAD_LEFT).":".
    str_pad($mins % 60, 2, "0", STR_PAD_LEFT).":".
    str_pad($seconds % 60, 2, "0", STR_PAD_LEFT);
}

// logins, newForums, forums, appsLaunched, downloads, uploads, soloTime,
// groupTime, meetings
function printCsvHeader($file, $label) {
  fprintf($file,'%s, Logins, New Forums, Active Forums, Apps Launched, Web Downloads, Web Uploads, Solo Time, Meeting Time, Meeting Count', $label);
  fprintf($file, "\n");
}

function printCsvRow($file, $label, $values) {
  fprintf($file, "%s, %d, %d, %d, %d, %d, %d, %s, %s, %d\n",
	  '"'.$label.'"',
	  $values['logins'],
	  $values['newForums'],
	  count($values['forums']),
	  $values['appsLaunched'],
	  $values['webDownloads'],
	  $values['webUploads'],
	  printTime($values['soloTime']),
	  printTime($values['groupTime']),
	  $values['meetingCount']
	  );
}

function printHtmlHeader($file, $name) {
  fprintf($file, "<th>$name</th><th>Logins</th><th>New Forums</th><th>Active Forums</th><th>Apps Launched</th><th>Web Downloads</th><th>Web Uploads</th><th>Solo Time</th><th>Meeting Time</th><th>Meeting Count</th>\n");
}

function printHtmlRow($file, $label, $values, $odd) {
  if($odd) {
    fprintf($file, '<tr style="background-color: #EEE;">');
  } else {
    fprintf($file, '<tr>');
  }
  fprintf($file,
    "<td>".$label."</td>".
    "<td>".$values['logins']."</td>".
    "<td>".$values['newForums']."</td>".
    "<td>".count($values['forums'])."</td>".
    "<td>".$values['appsLaunched']."</td>".
    "<td>".$values['webDownloads']."</td>".
    "<td>".$values['webUploads']."</td>".
    "<td>".printTime($values['soloTime'])."</td>".
    "<td>".printTime($values['groupTime'])."</td>".
    "<td>".$values['meetingCount']."</td>".
	  "</tr>\n");
}

function printForumHtmlHeader($file, $name) {
  fprintf($file, "<th>$name</th><th>Logins</th><th>Solo Time</th><th>Meeting Time</th><th>Meeting Count</th>\n");
}

function printForumHtmlRow($file, $label, $values, $odd) {
  if($odd) {
    fprintf($file, '<tr style="background-color: #EEE;">');
  } else {
    fprintf($file, '<tr>');
  }
  fprintf($file,
    "<td>".$label."</td>".
    "<td>".$values['logins']."</td>".
    "<td>".printTime($values['soloTime'])."</td>".
    "<td>".printTime($values['groupTime'])."</td>".
    "<td>".$values['meetingCount']."</td>".
	  "</tr>\n");
}

// -------------------------------------------------------------
// Command line processing
// -------------------------------------------------------------
$logDir = "/home/openqwaq/server/admin/reports/";

if(isset($argv[1])) $period = $argv[1];
 else $period = "-daily";
if(isset($argv[2])) $stop = strtotime($argv[2]);
 else $stop = time();

$pieces = getdate($stop);
switch($period) {
 case "-monthly":
   $periodName = "Monthly";
   $pieces["mon"] -= 1;
   break;
 case "-weekly":
   $periodName = "Weekly";
   $pieces["mday"] -= 7;
   break;
 case "-daily":
   $periodName  = "Daily";
   $pieces["mday"] -= 1;
   break;
 default:
   echo "Unknown time period specified: $period\n";
   return;
 }

$start = mktime($pieces["hours"], $pieces["minutes"], $pieces["seconds"],
		$pieces["mon"], $pieces["mday"], $pieces["year"]);
$start = strftime("%Y-%m-%d", $start);
$stop = strftime("%Y-%m-%d", $stop);
echo "Creating report covering $start to $stop\n";

$result = analyzeData($start, $stop);
$userData = $result["users"];
$orgData = $result["orgs"];
$globalData = $result["global"];
$forumData = $result["forums"];

function sortLogins($a, $b) {
  return $b["logins"] - $a["logins"];
}

uasort($userData, "sortLogins");
uasort($orgData, "sortLogins");
uasort($forumData, "sortLogins");


// ---------------------------------------------------------------------
// CSV reporting
// ---------------------------------------------------------------------

$csvFile = fopen($logDir . "total" . $period . "-" . $stop . ".csv", "wt");
printCsvHeader($csvFile, "Total");
printCsvRow($csvFile, "Total", $result["global"]);
fclose($csvFile);

$csvFile = fopen($logDir . "orgs" . $period . "-" . $stop . ".csv", "wt");
printCsvHeader($csvFile, "Total");
foreach($orgData as $org => $values) {
  printCsvRow($csvFile, $org, $values);
}
fclose($csvFile);

$csvFile = fopen($logDir . "users" . $period . "-" . $stop . ".csv", "wt");
printCsvHeader($csvFile, "Total");
foreach($userData as $user => $values) {
  printCsvRow($csvFile, $user, $values);
}
fclose($csvFile);

$csvFile = fopen($logDir . "forums" .$period . "-" . $stop . ".csv", "wt");
printCsvHeader($csvFile, "Total");
foreach($forumData as $forum => $values) {
  printCsvRow($csvFile, $forum, $values);
}
fclose($csvFile);

// ---------------------------------------------------------------------
// HTML reporting
// ---------------------------------------------------------------------

$html = fopen($logDir . "report" . $period . "-" . $stop . ".html", "wt");

fprintf($html, "\n\n");

fprintf($html, "%s", '<html><head>
<style type="text/css">

	
		table {
			border: 1px solid #43525C;
		}
	
		table tr th {
			background-color: #6A777F;
			border-right: 1px solid #43525C;
			border-bottom: 1px solid #43525C;
			padding: 11px;
			color: #FFF;
			font-size: 16px;
			font-weight: bold;
		}
		table tr td {
			border-right: 1px solid #43525C;
			border-collapse: collapse;
			height: 44px;
			line-height: 44px;
			vertical-align: middle;
			/*text-indent: 6px;*/padding: 0 6px;
		}

</style>
</head><body>');

fprintf($html, "<h2>$periodName OpenQwaq Usage Report</h2>");
fprintf($html, "<strong>Server: </strong>$hostname<br>");
fprintf($html, "<strong>Period: </strong>$start to $stop");

fprintf($html, "<h3>Total Server Usage</h3>");
fprintf($html, '<table cellpadding="0" cellspacing="0"><tbody>');

printHtmlHeader($html, "Server");
printHtmlRow($html, "Server", $result["global"], false);
fprintf($html, "</tbody></table>");

fprintf($html, "<h3>Top 20 Organizations</h3>");
fprintf($html, '<table cellpadding="0" cellspacing="0"><tbody>');

printHtmlHeader($html, "Organization");
$index = 1;
foreach($orgData as $org => $values) {
  if($index++ > 20) break;
  printHtmlRow($html, $org, $values, $index % 2 == 1);
}
if($index == 1) printHtmlRow($html, "none", newDataSet(), false);

fprintf($html, "</tbody></table>");

fprintf($html, "<h3>Top 20 Users</h3>");
fprintf($html, '<table cellpadding="0" cellspacing="0"><tbody>');

printHtmlHeader($html, "User");
$index = 1;
foreach($userData as $user => $values) {
  if($index++ > 20) break;
  printHtmlRow($html, $user, $values, $index % 2 == 1);
}
if($index == 1) printHtmlRow($html, "none", newDataSet(), false);
fprintf($html, "</tbody></table>");

fprintf($html, "<h3>Top 20 Forums</h3>");
fprintf($html, '<table cellpadding="0" cellspacing="0"><tbody>');

printForumHtmlHeader($html, "Org/Forum");
$index = 1;
foreach($forumData as $forum => $values) {
  $forum = preg_replace("/ & /", " &amp; ", $forum);
  if($index++ > 20) break;
  printForumHtmlRow($html, $forum, $values, $index % 2 == 1);
}
if($index == 1) printForumHtmlRow($html, "none", newDataSet(), false);
fprintf($html, "</tbody></table>");

fprintf($html, "</body></html>");
fclose($html);

if($email != "") {
  echo "\nEmailing report to $email\n";
  $message = file_get_contents($logDir . "report" . $period . "-" . $stop . ".html");
  sendMail("localhost", $email, 
	   "$periodName OpenQwaq Usage Report ($start to $stop) Server: $hostname", 
	   $message, array("ContentType" => "text/html"));
}

?>
