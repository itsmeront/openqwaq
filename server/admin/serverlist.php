<?php

header("Cache-Control: no-cache");
header("Expires: -1");

include("serverapi.php");
?>

<?php include('header.php'); ?>
<body id="server_list" class="servers">
<?php make_navbar('Servers'); ?>

<div id="body" class="wrap">
  <div id="server_info_table" class="section">
    <div class="table_header">
      <h3><span>Server Info</span></h3>
    </div>

<?php

$servers = listServers();

echo '<table cellpadding="0" cellspacing="0" class="alternate nolistify">';
echo '<tbody>
      <tr>
      <th>Date</th>
      <th>Internal</th>
      <th>External</th>
      <th>Role</th>
      <th>Status</th>
      <th class="actions">Actions</th></tr>';

foreach($servers as $form) {
  $created = explode(" ", $form["created"]);
  $created = $created[0];
  echo '
        <form method="POST" action="serveredit.php?action=update">
        <input type="hidden" name="id" value="' . $form['id'] . '">
        <tr>
			<td class="date">' . $created . '</td>';

  echo '
		<td><input name="internalName" value="' . $form['internalName'] . '" type="text" class="text"></td>
		<td><input name="externalName" value="' . $form['externalName'] . '" type="text" class="text"></td>';
		
  $isAllInOne = $isRouter = $isSP = $isApps = $isVideo = $isWebcast = $isProxy = "";
  if($form['role'] == "routerHost") $isRouter = "selected";
  if($form['role'] == "appHost") $isApps = "selected";
  if($form['role'] == "spHost") $isSP = "selected";
  if($form['role'] == "videoHost") $isVideo = "selected";
  if($form['role'] == "allInOne") $isAllInOne = "selected";
  if($form['role'] == "webcastHost") $isWebcast = "selected";
  if($form['role'] == "proxyHost") $isProxy = "selected";

  echo '
        <td><select name="role">
	    <option value="allInOne" ' . $isAllInOne . '>All-In-One</option>
	    <option value="spHost" ' . $isSP . '>Service Provider</option>
	    <option value="routerHost" ' . $isRouter . '>Router / Balancer</option>
	    <option value="appHost" ' . $isApps . '>Application Server</option>
	    <option value="videoHost" ' . $isVideo . '>Video Server</option>
	    <option value="webcastHost" ' . $isWebcast . '>Webcast Server</option>
	    <option value="proxyHost" ' . $isProxy . '>Proxy Client Host</option>
        </select></td>';

  $isActive = $isDisabled = "";
  if($form['status'] == "active") $isActive = "selected";
  else $isDisabled = "selected";

  echo '
        <td><select name="status">
	    <option value="active" ' . $isActive . '>Active</option>
	    <option value="disabled" ' . $isDisabled . '>Disabled</option>
        </select></td>';

  echo '
		<td class="actions">
			<input name="submit" type="image" value="Update" src="img/accept.png" class="update" title="Update" />
			<a href="serveredit.php?id=' . $form['id'] . '&amp;action=setup&amp;internalName=' . $form['internalName'] . '&amp;externalName=' . $form['externalName'] . '&amp;status=' . $form['status'] . '&amp;role=' . $form['role'] . '&amp;pool=' . $form['pool'] . '" class="setup" title="Setup">Setup</a>
			<a href="serveredit.php?id=' . $form['id'] . '&amp;action=delete" class="delete" title="Delete">Delete</a>
		</td>';

  echo '</tr></form>';
}

echo '
        <form method="POST" action="serveredit.php">
        <tr>
  	<td></td>
        <td><input type="text" name="internalName" value="" class="text"></td>
        <td><input type="text" name="externalName" value="" class="text"></td>
        <td><select name="role">
          <option value="allInOne" selected>All-In-One</option>
          <option value="spHost">Service Provider</option>
          <option value="routerHost">Router / Balancer</option>
          <option value="appHost">Application Server</option>
          <option value="videoHost">Video Server</option>
          <option value="webcastHost">Webcast Server</option>
          <option value="proxyHost">Proxy Client Host</option>
        </select></td>
        <td><select name="status">
          <option value="active" selected>Active</option>
          <option value="disabled">Disabled</option>
        </select></td>
        <td class="actions"><span class="button">
             <input type="submit" name="action" value="Add Server" src="img/add.png" class="add">
        </span></td>
        </tr>
        </form>';
  echo "</tbody></table>";

?>

</div>
</div>
<?php include('footer.php') ?>
</body>
</html>
