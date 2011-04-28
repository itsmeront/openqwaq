<?php
/**
 * PHP Template.
 */
 include('adminapi.php'); 
 include('status.php');
 $statusDoc = getStatus();

?>
<?php include('header.php'); ?>
<body id="dashboard">
	<?php make_navbar('Dashboard'); ?>
	<div id="body" class="wrap">
		<div id="server_info_table" class="section">
			<div class="table_header">
				<h3><span>Server Info</span></h3>
			</div>
			<table cellpadding="0" cellspacing="0" class="alternate">
				<thead>
					<tr>
						<th>Active Users</th>
						<th>Server Version</th>
						<th>Host Name</th>
					</tr>
				</thead>
				<tbody>
					<tr>
						<td><?php echo $statusDoc->activeUsers; ?></td>
						<td><a href="serverinfo.php"><?php echo $statusDoc->version; ?></a></td>
						<td><?php echo $statusDoc->host; ?></td>
					</tr>
				</tbody>
			</table>
		</div>
		<div class="section">
			<table cellpadding="0" cellspacing="0" class="alternate">
				<thead>
					<tr>
						<th colspan="2">Server License</th>
					</tr>
				</thead>
				<tbody>
					<?php printFeatureInfo() ?>
				</tbody>
			</table>
		</div>
		<div class="section">
			<table cellpadding="0" cellspacing="0" class="alternate">
				<tbody>
					<tr>
						<th>User</th>
						<th>Organization</th>
						<th>Duration</th>
					</tr>
					<?php
  foreach($statusDoc->spHost as $spHost)
     if(isset($spHost->ok))
       foreach($spHost->ok->connections->client as $client) {
	 echo '<tr>
            <td>'.$client->name.'</td>
            <td>'.$client->org.'</td>
            <td>'.$client->duration.'</td>
        </tr>';
 }
?>
				</tbody>
				</table>
		</div>
		<div id="users_table" class="section">
			<div class="table_header">
   <h3><span>Users</span> <span class="add"><a href="edituser.php">(Add</a>, <a href="importUsersForm.php">Import</a>)</span></h3>
				<form method="post" action="finduser.php" class="search">
					<input name="search" value="" type="text" class="text">
					<input value="Find User" type="submit" class="search">
				</form>
				<h4>Recently Added Users</h4>
			</div>
			<?php makeUsersTable("", 5, false); ?>
		</div>
		<div id="organizations_table" class="section">
			<div class="table_header">
				<h3><span>Organizations</span> <span class="add"><a href="editorg.php">(Add)</a></span></h3>
				<form method="post" action="findorg.php" class="search">
					<input name="search" value="" type="text" class="text">
					<input value="Find Organization" type="submit" class="search">
				</form>
				<h4>Recently Added Organizations</h4>
			</div>
			<?php makeOrgsTable("", 5, false);?>
		</div>
	</div>
	<?php include('footer.php') ?>
</body>
</html>
