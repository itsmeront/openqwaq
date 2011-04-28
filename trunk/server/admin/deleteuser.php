<?php
include("adminapi.php");

$id = $_GET['id'];
if(!isset($_POST["id"])) {
  if(!isset($_GET["id"])) {
    return header("Location: index.php");
  }
} else {
  $rs = deleteUser($id);
  echo "deleting user";
  if($rs == true) {
    header("Location: index.php");
  }
}

$form = findUserById($id);
$pageTitle="Deleting User: {$form['name']}";
$pageInfo="";
$submitPage=$_SERVER["REQUEST_URI"];
$submitLabel="Delete User";
?>

<?php include('header.php'); ?>
<body id="edit_user" class="users">
<?php make_navbar('Users'); ?>

<div id="body" class="wrap">
  <h3><?php echo $pageTitle; ?></h3>


<form name="viewUserForm" method="post" action="<?php echo $submitPage; ?>">
  <input type="hidden" name="id" value="<?php echo $form['id'] ?>">
  <table border="0">
    <tr>
      <td><label class="required">User ID (login):</label></td>
      <td><?php echo $form['name'] ?></td>
    </tr>
    <tr>
      <td><label class="required">Email:</label></td>
      <td><?php echo $form['email'] ?></td>
    </tr>
  </table>
  <div class="button submit">
  	<input type="submit" value="<?php echo $submitLabel ?>">
	</div>
</form>
</div>
<?php include('footer.php') ?>
</body>
</html>

