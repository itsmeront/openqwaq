<?php
include("adminapi.php");

if(!isset($_GET["id"])) {
  return header("Location: index.php");
}
$id = $_GET['id'];
$form = findOrgAccountById($id);
if(!isset($form)) {
  $form = findGroupAccountById($id);
}

if(isset($_POST['id'])) {
  // Return to the 'proper' place we were coming from
  $from = $_GET['from'];
  if(!isset($from)) $from = '';
  $rs = deleteAccount($id);
  if($from == 'user') {
    header("Location: edituser.php?id=${form['memberid']}");
    return;
  }
  if($from == 'org') {
    header("Location: editorg.php?id=${form['orgid']}");
    return;
  }
  if($from == 'group') {
    if(isset($form['groupName'])) {
      header("Location: editgroup.php?id=${form['groupid']}");
    } else {
      header("Location: editgroup.php?id=${form['memberid']}");
    }
    return;
  }
  header("Location: index.php");
  return;
}

if(isset($form['groupName'])) $orgOrGroup = $form['groupName'];
if(isset($form['orgName'])) $orgOrGroup = $form['orgName'];

$pageTitle="Removing Member: {$form['memberName']} from $orgOrGroup";
$pageInfo="";
$submitPage=$_SERVER["REQUEST_URI"];
$submitLabel="Remove Member";
?>
<?php include('header.php'); ?>
<body id="edit_organization" class="organizations">
	<?php make_navbar(''); ?>
	<div id="body" class="wrap">
		<h3><?php echo $pageTitle; ?></h3>
		<form name="viewAcctForm" method="post" action="<?php echo $submitPage; ?>">
			<input type="hidden" name="id" value="<?php echo $form['id'] ?>">
			<table border="0">
				<tr>
					<td><label class="required">Member:</label></td>
					<td><?php echo $form['memberName'] ?></td>
				</tr>
				<tr>
					<td><label class="required">Organization/Group:</label></td>
					<td><?php echo $orgOrGroup; ?></td>
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
