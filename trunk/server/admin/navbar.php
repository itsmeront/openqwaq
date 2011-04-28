<?php

if(!isset($selected_tab)) $selected_tab = '';

$dashboard_selected = '';
$users_selected = '';
$groups_selected = '';
$orgs_selected = '';
$servers_selected = '';
$tools_selected = '';

if($selected_tab == 'Dashboard') $dashboard_selected = 'class="selected"';
if($selected_tab == 'Users') $users_selected = 'class="selected"';
if($selected_tab == 'Groups') $groups_selected = 'class="selected"';
if($selected_tab == 'Organizations') $orgs_selected = 'class="selected"';
if($selected_tab == 'Servers') $servers_selected = 'class="selected"';
if($selected_tab == 'Tools') $tools_selected = 'class="selected"';

include("features.php");

?>

<div id="header">
  <div class="wrap">
  <h1><a href="index.php" title="Dashboard">OpenQwaq Admin Interface</a></h1>
<ul id="account_info">
  <li><a href="editlicense.php"><b><?php echo getLicenseOwner() ?></b></a></li>
  <li><a href="/docs/adminguide/">Help</a></li>
</ul>
<ul id="navigation">
  <li id="dashboard_tab" <?php echo $dashboard_selected; ?>><a href="index.php">Dashboard</a></li>
  <li id="users_tab" <?php echo $users_selected; ?>><a href="finduser.php">Users</a></li>
  <li id="groups_tab" <?php echo $groups_selected; ?>><a href="findgroup.php">Groups</a></li>
  <li id="organizations_tab" <?php echo $orgs_selected; ?>><a href="findorg.php">Organizations</a></li>
  <li id="servers_tab" <?php echo $servers_selected; ?>><a href="serverlist.php">Servers</a></li>
  <?php if(0) {?>
  <li id="tools_tab" <?php echo $tools_selected; ?>><a href="tools.php">Tools</a></li>    
  <?php } ?>
  </ul>
  </div>
</div>
