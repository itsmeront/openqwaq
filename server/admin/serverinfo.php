<?php
 include('adminapi.php'); 
 include('status.php');
 $statusDoc = getStatus();
?>

<?php include('header.php'); ?>
<body id="server_info">
<?php make_navbar('Servers'); ?>

<div id="body" class="wrap">
  <div class="section">
    <div class="table_header">
<?php
  include("config.php");
  $temp = $allServers[0][2];
  $temp = explode(":", $temp);
  $temp = explode("//", $temp[1]);
  $serverAddr = $temp[1];
  if (empty($serverAddr)) {
    $serverAddr=localhost;
  }
  echo '<h3>Forum Status '.
  ' (<a href="serverlogs.php?type=sp&server='.$serverAddr.'">logs</a>,
     <a href="serverreports.php">reports</a>,
     <a href="servertest.php?height=300&width=500" class="thickbox test" title=Tests>Tests</a>,
     <a href="serveremaillogs.php?height=300&width=500" class="thickbox test" title="Email Recent Logs">email recent log activity</a>)'.
  '</h3>';
?>
  </div>
  <div class="sub_header">
    <p></p>
  </div>
  <table cellpadding="0" cellspacing="0" class="alternate">
    <tbody>
  <tr>
  <th colspan="2">Overall</th>
  </tr>
  <tr>
  <td>Total Users</td>
  <td><?php echo($statusDoc->activeUsers);?></td>
  </tr>
  <tr>
  <td>Server Version</td>
  <td><?php echo($statusDoc->version);?> </td>
  </tr>
  <tr>
  <td>Server Build</td>
  <td><?php echo($statusDoc->build);?> </td>
  </tr>
  <tr>
  <td>Query Stamp</td>
  <td><?php echo($statusDoc->timestamp);?></td>
  </tr>
  <tr>
  <td>Total Memory</td>
  <td><?php echo($statusDoc->memory);?></td>
  </tr>
  <tr>
  <td>Full GCs</td>
  <td><?php echo($statusDoc->fullGCs);?></td>
  </tr>
  <tr>
  <td>Tenures</td>
  <td><?php echo($statusDoc->tenures);?></td>
  </tr>
  </tbody>
  </table>
  
  </div>
<?php

  foreach($statusDoc->spHost as $spHost) {
    /* Service Provider Reporting */
    foreach($spHost->attributes() as $key=>$value){
      $attr[$key]=$value;
    }
    $rHostname = $attr["name"];
    echo '
  <div class="section">
    <div class="table_header">
      <h3>Service Provider: '.$rHostname.
      ' (<a href="serverlogs.php?type=sp&server='.$rHostname.'">logs</a>)'.
      '</h3>
       </div>
    <div class="sub_header">
      <p></p>
    </div>';

    if(!isset($spHost->ok)) {
      echo '<table cellpadding="0" cellspacing="0">';
      echo '<td><span style="color:red">HOST IS DOWN: '.
	(string)$spHost->failed.'</span></td></table>';
      echo '</div>';
    } else {
      echo('<table cellpadding="0" cellspacing="0" class="alternate">
      <tbody><tr>
          <th>Source</th><th>User</th><th>Organization</th><th>Duration</th>
        </tr>');
      foreach($spHost->ok->connections->client as $client) {
	echo '<tr>
            <td>'.$client->ip.'</td>
            <td>'.$client->name.'</td>
            <td>'.$client->org.'</td>
            <td>'.$client->duration.'</td>
        </tr>';
      }
      echo ('
      </tbody>
    </table>
    </div>');
    }
  }

/* Router Reporting */

foreach($statusDoc->routerHost as $rhost) {
  foreach($rhost->attributes() as $key=>$value){
    $attr[$key]=$value;
  }
  $rHostname = $attr["name"];
  echo '
  <div class="section">
    <div class="table_header">
      <h3>Router: '.$rHostname.
  ' (<a href="serverlogs.php?type=router&server='.$rHostname.'">logs</a>)'.
  '</h3>
    </div>
    <div class="sub_header">
      <p>Active Sessions: '.$rhost->ok->activeIslands. ' / Active Clients: '.$rhost->ok->activeClients.'</p>
    </div>';

  if(!isset($rhost->ok)) {
    echo '<table cellpadding="0" cellspacing="0">';
    echo '<td><span style="color:red">HOST IS DOWN: '.
      (string)$rhost->failed.'</span></td></table>';
    echo '</div>';
  } else {
    echo('<table cellpadding="0" cellspacing="0" class="alternate">
      <tbody>
        <tr>
          <th>Organization</th><th>Forum</th><th>Users</th><th></th>
        </tr>');
    foreach($rhost->ok->forum as $forum) {
      $list = explode('/', $forum->name);
      $org = findOrgById($list[1]);
      if(isset($org)) $org = $org['name'];
      else $org = $list[1];
      $id = $forum->id;
      echo '<tr><td>'.$org.'</td>';
      echo '<td>'. $list[2]. '</td>';
      echo '<td>'. $forum->attributes()->users. ': '. $forum->users . '</td>';
      echo '<td>';
      if($id <> '') {
	echo "<form method=\"POST\" action=\"killrouter.php?id=$id\">";
	echo '<input type="submit" value="kill session "></form>';
      }
      echo '</td></tr>';
    };
    echo ('
      </tbody>
    </table>
    </div>
    <div class="section">
    <table cellpadding="0" cellspacing="0" class="alternate">
      <tbody>
        <tr>
          <th>Source</th><th>User</th><th>Duration</th>
        </tr>');
    foreach($rhost->ok->client as $client) {
      $id = $client->id;
      echo '<tr>';
      echo '<td>'.$client->ip.'</td>';
      echo '<td>'.$client->name.'</td>';
      echo '<td>'.$client->duration.'</td>';
      echo '</tr>';
    };
    echo ('
      </tbody>
    </table>
  </div>');

  }
}

/* App Host Reporting */

foreach($statusDoc->appHost as $ahost) {
  foreach($ahost->attributes() as $key=>$value){
    $attr[$key]=$value;
  }
  $aHostname = $attr["name"];
  echo('
  <div class="section">
    <div class="table_header">
      <h3>App Host: '.$aHostname.
  ' (<a href="serverlogs.php?type=appHost&server='.$aHostname.'">logs</a>)'.
  '</h3>
    </div>
    <div class="sub_header">
      <p>Screens (active / total): '.$ahost->ok->activeScreens. ' / '. $ahost->ok->totalScreens.'</p>
    </div>');

  if(!isset($ahost->ok)) {
    echo '<table cellpadding="0" cellspacing="0">';
    echo '<td><span style="color:red">HOST IS DOWN: '.
      (string)$ahost->failed.'</span></td></table>';
    echo '</div>';
  } else {

    echo('<table cellpadding="0" cellspacing="0" class="alternate">
      <tbody>
        <tr>
          <th>Screen</th><th>Application</th><th>Organization</th><th>Forum</th>
        </tr>');
    foreach($ahost->ok->screen as $screen) {
      $list = explode('/', $screen->description);
      $org = findOrgById($list[1]);
      if(isset($org)) $org = $org['name'];
      else $org = $list[1];
      echo '<tr>';
      echo '<td>'.$screen->attributes()->number.'</td>';
      echo '<td>'.$screen->name.'</td>';
      echo '<td>'.$org.'</td>';
      echo '<td>'.$list[2].'</td>';
      echo '</tr>';
    };
    echo ('
      </tbody>
    </table>
  </div>');
  }
}

/* Video Host reporting */
foreach($statusDoc->netvidHost as $ahost) {
  foreach($ahost->attributes() as $key=>$value){
    $attr[$key]=$value;
  }
  $aHostname = $attr["name"];
  echo('
  <div class="section">
    <div class="table_header">
      <h3>Video Host: '.$aHostname.
  ' (<a href="serverlogs.php?type=netVidHost&server='.$aHostname.'">logs</a>)'.
  '</h3>
    </div>
    <div class="sub_header">
      <p>Streams (loaded / live): '.$ahost->ok->assetCount. ' / '. $ahost->ok->serverCount.'</p>
    </div>');

  if(!isset($ahost->ok)) {
    echo '<table cellpadding="0" cellspacing="0">';
    echo '<td><span style="color:red">HOST IS DOWN: '.
      (string)$ahost->failed.'</span></td></table>';
    echo '</div>';
  } else {

    echo ('<table cellpadding="0" cellspacing="0" class="alternate">
      <tbody>
        <tr>
          <th align="center">State</th>
          <th align="center">Stream</th>
        </tr>');
    foreach($ahost->ok->server as $detail) {
      echo '<tr><td>'.$detail->state.'</td><td>'.$detail->asset.'</td></tr>';
    };
    echo ('
      </tbody>
    </table>
  </div>');

  }
}

/* Webcast Host reporting */
foreach($statusDoc->webcastHost as $whost) {
  foreach($whost->attributes() as $key=>$value){
    $attr[$key]=$value;
  }
  $whostname = $attr["name"];
  echo('
  <div class="section">
    <div class="table_header">
      <h3>Webcast Host: '.$whostname.
  ' (<a href="serverlogs.php?type=webcastHost&server='.$whostname.'">logs</a>)'.
  '</h3>
    </div>
    <div class="sub_header">
      <p>Webcasts: '.$whost->ok->webcastCount. '  Sessions: '. $whost->ok->sessionCount. '</p>
    </div>');

  if(!isset($whost->ok)) {
    echo '<table cellpadding="0" cellspacing="0">';
    echo '<td><span style="color:red">HOST IS DOWN: '.
      (string)$whost->failed.'</span></td></table>';
    echo '</div>';
  } else {

    foreach($whost->ok->webcasts->webcast as $webcast) {
        echo ('<table cellpadding="0" cellspacing="0" class="alternate">
          <tbody>
            <tr>
              <th align="center">Webcast Name</th>
              <th align="center">Org</th>
              <th align="center">Forum</th>
              <th align="center">User</th>
              <th align="center">Age</th>
              <th align="center">Session Count</th>
            </tr>');

        echo '<tr><td>'.$webcast->name.'</td><td>'.$webcast->properties->orgName.'</td><td>'.$webcast->properties->service.'</td><td>'.$webcast->properties->userName.'</td><td>'.$webcast->age.'</td><td>'.$webcast->sessionCount.'</td></tr>';

	echo('</tbody></table>');

        echo ('<table cellpadding="0" cellspacing="0" class="alternate">
          <tbody>
            <tr>
              <th align="center">Age</th>
              <th align="center">State</th>
              <th align="center">Remote IP Address</th>
            </tr>');

	foreach($webcast->sessions->session as $websession) {
	  echo '<tr><td>'.$websession->age.'</td><td>'.$websession->state.'</td><td>'.$websession->remoteIP.'</td></tr>';
	}
	echo('</tbody></table>');

	echo('<br><br>');
    };


    echo ('</div>');

  }
}

/* Proxy Host reporting */
foreach($statusDoc->proxyHost as $phost) {
  foreach($phost->attributes() as $key=>$value){
    $attr[$key]=$value;
  }
  $phostname = $attr["name"];
  echo('
  <div class="section">
    <div class="table_header">
      <h3>Proxy Host: '.$phostname.
  ' (<a href="serverlogs.php?type=proxyHost&server='.$phostname.'">logs</a>)'.
  '</h3>
    </div>
    <div class="sub_header">
      <p></p>
    </div>');

  if(!isset($phost->ok)) {
    echo '<table cellpadding="0" cellspacing="0">';
    echo '<td><span style="color:red">HOST IS DOWN: '.
      (string)$phost->failed.'</span></td></table>';
    echo '</div>';
  } else {

    echo ('<table cellpadding="0" cellspacing="0" class="alternate">
      <tbody>
        <tr>
          <th colspan="2">Launcher</th>
        </tr>');
    echo ('
        <tr>
          <td>Server Build</td>
          <td>'.$phost->ok->version.'</td>
        </tr>
        <tr>
          <td>CPU load (1, 5, and 15 minutes)</td>
          <td>'.$phost->ok->loadAverages.'</td>
        </tr>
        <tr>
          <td>clients</td>
          <td>'.$phost->ok->nClients.'</td>
        </tr>');
    echo ('
      </tbody>
    </table>
    </div>
    <div class="section">
    <table cellpadding="0" cellspacing="0" class="alternate">
      <tbody>
        <tr>
          <th>user</th><th>% cpu</th><th>% memory</th><th>duration</th><th></th>
        </tr>');
    foreach($phost->ok->client as $client) {
      $list = explode(' ', $client);
      $id = $list[1];
      echo '<tr>
            <td>'.$list[14].'</td>
            <td>'.$list[2].'</td>
            <td>'.$list[3].'</td>
            <td>'.$list[9].'</td>
            <td>';
	if($id <> '') {
	  echo "<form method=\"POST\" action=\"killproxyclient.php?pid=$id&server=$phostname\">";
	  echo '<input type="submit" value="kill session "></form>';
	}
      echo '</td>
        </tr>';
    };
    echo ('
      </tbody>
    </table>
  </div>');
  }
}

?>

</div>
<?php include('footer.php') ?>
</body>
</html>
