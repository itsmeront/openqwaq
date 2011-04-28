<?php
include("serverapi.php");

$id = $_GET['id'];
if(!isset($_POST["id"])) {
  if(!isset($_GET["id"])) {
    return header("Location: serverlist.php");
  }
} else {
  $rs = deleteServer($id);
  if($rs == true) {
    header("Location: serverlist.php");
  }
}

$form = findServerById($id);
$pageTitle="Deleting Server: {$form['internalName']}";
$pageInfo="";
$submitPage=$_SERVER["REQUEST_URI"];
$submitLabel="Delete Server";
?>
<?php include('header.php'); ?>
<body id="edit_organization" class="organizations">
	<?php make_navbar('Servers'); ?>
	<div id="body" class="wrap">
		<h3><?php echo $pageTitle; ?></h3>
		<form name="viewServerForm" method="post" action="<?php echo $submitPage; ?>">
			<input type="hidden" name="id" value="<?php echo $form['id'] ?>">
			<table border="0">
				<tr>
					<td><label class="required">Name:</label></td>
					<td><?php echo $form['internalName'] ?></td>
				</tr>
			</table>
			<div class="button submit">
				<input type="submit" value="<?php echo $submitLabel; ?>">
			</div>
		</form>
	</div>
	<?php include('footer.php') ?>
</body>
</html>
