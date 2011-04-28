<?php
include("adminapi.php");

$id = $_GET['id'];
if(!isset($_POST["id"])) {
  if(!isset($_GET["id"])) {
    return header("Location: index.php");
  }
} else {
  $rs = deleteOrg($id);
  if($rs == true) {
    header("Location: index.php");
  }
}

$form = findOrgById($id);
$pageTitle="Deleting Organization: {$form['name']}";
$pageInfo="";
$submitPage=$_SERVER["REQUEST_URI"];
$submitLabel="Delete";
?>
<?php include('header.php'); ?>
<body id="edit_organization" class="organizations">
	<?php make_navbar('Organizations'); ?>
	<div id="body" class="wrap">
		<h3><?php echo $pageTitle; ?></h3>
		<form name="viewOrgForm" method="post" action="<?php echo $submitPage; ?>">
			<input type="hidden" name="id" value="<?php echo $form['id'] ?>">
			<table border="0">
				<tr>
					<td><label class="required">Name:</label></td>
					<td><?php echo $form['name'] ?></td>
				</tr>
				<tr>
					<td><label class="required">Seats:</label></td>
					<td><?php echo $form['seats'] ?></td>
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
