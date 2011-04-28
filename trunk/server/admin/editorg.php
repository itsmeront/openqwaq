<?php
#include('adminapi.php');
include('forumsprops.php');

if(isset($_GET['id'])) {
  $id = $_GET['id'];
  $form = findOrgById($id);
  $pageTitle="Org Info: {$form['name']}";
  $pageInfo="";
  $submitPage="updateorg.php?id=$id";
  $submitLabel="Update Organization";
} else {
  $form = array(
	"name" => "", 
	"status" => "ActiveOrg", 
	"seats" => "99", 
	"tier" => "", 
	"server" => "",
	"comment" => "",
        "sfdc_id" => ""
  );
  $pageTitle="Create New Organization";
  $pageInfo="Please provide the following information";
  $submitPage="updateorg.php";
  $submitLabel="Create Organization";
}

?>
<?php include('header.php'); ?>
<body id="edit_organization" class="organizations">
	<?php make_navbar('Organizations'); ?>
	<div id="body" class="wrap">
		<h3><?php echo $pageTitle; ?></h3>
		<?php if(isset($id)) { ?>
		<ul id="options">
			<li id="add" class="button">
				<form method="post" action="editorg.php">
					<input type="submit" value="Add">
				</form>
			<li id="delete" class="button">
				<form method="post" action="<?php echo "deleteorg.php?id=$id";?>">
					<input type="submit" value="Delete">
				</form>
				<?php 
      if (@$form['sfdc_id']){
          echo "    
            <li id='sf' class='button'>
              <input type='submit' value='SalesForce.com'>
            </li>";
          }
     ?>
		</ul>
		<?php } ?>
		<form name="editOrgForm" method="post" action="<?php echo $submitPage; ?>">
                <input type="hidden" name="server" value="<?php echo $form['server']; ?>">
			<fieldset>
				<legend>Required Information</legend>
				<ul>
					<li>
						<label class="required">Name</label>
						<input name="name" value="<?php echo $form['name']; ?>" type="text" class="text">
					</li>
					<li>
						<label class="required">Seats</label>
						<input name="seats" value="<?php echo $form['seats']; ?>" type="text" class="text">
					</li>
					<li>
						<label class="required">Tier</label>
						<select name="tier" title="Tier of Service">
							<option value="">-- None --</option>
							<option <?php if ($form['tier'] == "PersonalEdition") {echo("selected");} ?> value="PersonalEdition">Personal Edition</option>
							<option <?php if ($form['tier'] == "GroupEdition") {echo("selected");} ?> value="GroupEdition">Group Edition</option>
							<option <?php if ($form['tier'] == "EnterpriseEdition") {echo("selected");} ?> value="EnterpriseEdition">Enterprise Edition</option>
						</select>
					</li>
					<li>
						<label class="required">Status</label>
						<select name="status" title="Organization status">
							<option <?php if ($form['status'] == "ActiveOrg") {echo("selected");} ?> value="ActiveOrg">Active</option>
							<option <?php if ($form['status'] == "ExpiredOrg") {echo("selected");} ?> value="ExpiredOrg">Expired</option>
							<option <?php if ($form['status'] == "DeletedOrg") {echo("selected");} ?> value="DeletedOrg">Inactive</option>
						</select>
					</li>
				</ul>
			</fieldset>
			<fieldset>
				<legend>Optional Information</legend>
				<ul>
					<li class="comment">
						<label class="optional">Comment</label>
						<textarea name="comment"><?php echo($form['comment']); ?></textarea>
					</li>
				</ul>
			</fieldset>
			<div class="button submit">
				<input name="action" value="<?php echo $submitLabel ?>" type="submit">
			</div>
		</form>
		<?php if(isset($id)) {
echo "<h3>Properties</h3>";
makeForumsProperties($id);
?>
		<div id="users_table" class="section">
			<div class="table_header">
				<h3><span>Users in <?php echo $form['name']; ?></span></h3>
			</div>
			<?php if(isset($id)) makeOrgAccountTable($id, "updateacct.php")?>
		</div>
		<?php } else echo '<div class="section"></div>'; ?>
	</div>
	<?php include('footer.php') ?>
</body>
</html>
