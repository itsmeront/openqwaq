<?php include('adminapi.php'); ?>
<html><head>
    <TITLE>OpenQwaq Server Admin Page</TITLE>
</head><body>
<?php 
    include('navbar.php');
    include('status.php');
    $statusDoc = getStatus();
?>
<h3>Server Info</h3>
<table border="1">
  <tr><td>Active Users</td><td><?php echo $statusDoc->activeUsers; ?></td></tr>
  <tr><td>Server Version</td><td><?php echo $statusDoc->version; ?></td></tr>
  <tr><td>Host Name</td><td><?php echo $statusDoc->host; ?></td></tr>
</table>

<h3>Organizations (<a href="editorg.php">Create</a>)</h3>
<form method="POST" action="findorg.php">
<input type="text" name="search" value=""><input type="submit" value="Find Organization">
</form>
Recently added organizations:
<?php makeOrgsTable("", 5, false);?>

<h3>Users (<a href="edituser.php">Create</a>)</h3>
<form method="POST" action="finduser.php">
<input type="text" name="search" value=""><input type="submit" value="Find User">
</form>
Recently added users:
<?php makeUsersTable("", 5, false); ?>

</body>