<?php
include("adminapi.php");

$id = $_GET['id'];
if(!isset($_POST["id"])) {
  if(!isset($_GET["id"])) {
    return header("Location: index.php");
  }
} else {
  $rs = deleteGroup($id);
  if($rs == true) {
    header("Location: index.php");
  }
}

$form = findGroupById($id);
$pageTitle="Deleting Group: {$form['name']}";
$pageInfo="";
$submitPage=$_SERVER["REQUEST_URI"];
$submitLabel="Delete Group";
?>
<?php include('header.php'); ?>
<body id="edit_organization" class="organizations">
	<?php make_navbar('Groups'); ?>
	<div id="body" class="wrap">
		<h3><?php echo $pageTitle; ?></h3>
		<form name="viewGroupForm" method="post" action="<?php echo $submitPage; ?>">
			<input type="hidden" name="id" value="<?php echo $form['id'] ?>">
			<table border="0">
				<tr>
					<td><label class="required">Name:</label></td>
					<td><?php echo $form['name'] ?></td>
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
