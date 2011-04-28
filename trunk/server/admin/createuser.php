<?php

include('adminapi.php');

$form = array(
	"name" => "",
	"password" => "",
	"email" => "",
	"status" => "ActiveUser",
	"firstName" => "",
	"lastName" => "",
	"company" => "",
	"comment" => "",
	"created" => date('Y-m-d G-i-s'),
        "sfdc"    => ""
	);
$pageTitle="Create New User";
$pageInfo="Please fill in the following information";
$submitPage="updateuser.php";
$submitLabel="Create User";

?>
<?php include('header.php'); ?>
<body id="edit_user" class="users">
	<?php make_navbar('Users'); ?>
	<div id="body" class="wrap">
		<h3><?php echo $pageTitle ?></h3>
		<form name="createUserForm" method="post" onsubmit= "return validateNewUserForm()" action="<?php echo $submitPage; ?>">
			<fieldset>
				<legend>Required Information</legend>
				<ul>
					<li>
						<label class="required">User ID (email)</label>
						<input name="name" value="<?php echo $form['name'] ?>" type="text" class="text">
					</li>
					<li>
						<label class="required">Organization</label>
						<select id="orgName" name="orgName" style="width: 150px">
							<option value="">--- Choose One ---</option>
							<?php 
foreach(findAllGroupNames() as $name)
  echo "<option value=\"$name\">$name (group)</option>\n";
foreach(findAllOrgNames() as $name) 
  echo "<option value=\"$name\">$name</option>\n"; 
?>
						</select>
					</li>
					<li>
						<label class="required">Role</label>
						<select name="role" title="User Role" width="150px">
							<option value="admin">Administrator</option>
							<option value="user" selected>Member</option>
							<option value="guest">Visitor</option>
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
					<li class="checkbox">
						<input name="activateUser" value="activateUser" type="checkbox" checked>
						<label for="activeUser">Send Welcome Email</label>
					</li>
				</ul>
			</fieldset>
			<div class="submit button">
				<input name="submit" value="<?php echo $submitLabel ?>" type="submit" class="submit">
			</div>
		</form>
		<br>
	</div>
	<?php include('footer.php') ?>
</body>
</html>
