<?php

include('adminapi.php');

if(!isset($_GET['id'])) {
  // this is a hack but it'll do for now
  return header("Location: createuser.php");
}

$id = $_GET['id'];
$form = findUserById($id);
$pageTitle="User info: {$form['name']}";
$pageInfo="";
$submitPage="updateuser.php?id=$id";
$submitLabel="Update User";

?>
<?php include('header.php'); ?>
<body id="edit_user" class="users">
	<?php make_navbar('Users'); ?>
	<div id="body" class="wrap">
		<h3><?php echo $pageTitle ?></h3>
		<?php if(isset($id)) { ?>
		<ul id="options">
			<li id="add" class="button">
				<form method="post" action="createuser.php">
					<input type="submit" value="Add">
				</form>
			<li id="delete" class="button">
				<form method="post" action="<?php echo "deleteuser.php?id=$id";?>">
					<input type="submit" value="Delete">
				</form>
			<li id="sf" class="button">
				<input type="<?php 
           if($form['sfdc'])
              echo 'submit'; 
           else 
              echo 'hidden'; ?>" value="SalesForce.com">
			</li>
		</ul>
		<?php } ?>
		<form name="editUserForm" method="post" action="<?php echo $submitPage; ?>">
			<input type="hidden" name="sfdc" value="<?php echo $form['sfdc'] ?>" />
			<input type="hidden" name="userid" value="<?php echo @$_GET['id'] ?>" />
			<fieldset>
				<legend>Required Information</legend>
				<ul>
					<li>
						<label class="required">User ID (email)</label>
						<input name="name" value="<?php echo $form['name'] ?>" type="text" class="text">
					</li>
					<li>
						<label class="required">Password</label>
						<input name="password" value="<?php echo $form['password'] ?>" type="password">
					</li>
					<li>
						<label class="required">Repeat password</label>
						<input name="password2" value="<?php echo $form['password'] ?>" type="password">
					</li>
					<!--
      <li>
        <label class="required"></label>
        <input name="email" value="<?php echo $form['email'] ?>" type="type" class="text">
      </li>
-->
					<li>
						<label class="required">Status</label>
						<select name="status" title="User Status">
							<option <?php if ($form['status'] == "ActiveUser") {echo("selected");} ?> value="ActiveUser">Active</option>
							<option <?php if ($form['status'] == "InvitedUser") {echo("selected");} ?> value="InvitedUser">Invited</option>
							<option <?php if ($form['status'] == "InActiveUser") {echo("selected");} ?> value="InActiveUser">Inactive</option>
						</select>
					</li>
				</ul>
			</fieldset>
			<fieldset>
				<legend>Optional Information</legend>
				<ul>
					<li>
						<label class="optional">First Name</label>
						<input name="firstName" value="<?php echo $form['firstName'] ?>" type="text" class="text">
					</li>
					<li>
						<label class="optional">Last Name</label>
						<input name="lastName" value="<?php echo $form['lastName'] ?>" type="text" class="text">
					</li>
					<li>
						<label class="optional">Company</label>
						<input name="company" value="<?php echo $form['company'] ?>" type="text" class="text">
					</li>
					<li class="comment">
						<label class="optional">Comment</label>
						<textarea name="comment"><?php echo $form['comment'] ?></textarea>
					</li>
				</ul>
			</fieldset>
			<div class="submit button">
				<input name="submit" value="<?php echo $submitLabel ?>" type="submit" class="submit">
			</div>
			<div  class ="submit button">
				<input name="submit" type="submit" value="Send Account Reminder">
			</div>
		</form>
		<div id="organizations_table" class="section">
			<div class="table_header">
				<h3><span>Membership info</span></h3>
			</div>
			<?php makeUserAccountTable($id, "updateacct.php"); ?>
		</div>
	</div>
	<?php include('footer.php') ?>
</body>
</html>
