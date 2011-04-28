<?php
include('adminapi.php');

if(isset($_GET['id'])) {
  $id = $_GET['id'];
  $form = findGroupById($id);
  $pageTitle="Group Info: {$form['name']}";
  $pageInfo="";
  $submitPage="updategroup.php?id=$id";
  $submitLabel="Update Group Info";
} else {
  $form = array(
	"name" => "", 
	"status" => "Active", 
	"comment" => ""
  );
  $pageTitle="Create New Group";
  $pageInfo="Please provide the following information";
  $submitPage="updategroup.php";
  $submitLabel="Create Group";
}

?>
<?php include('header.php'); ?>
<body id="edit_organization" class="organizations">
<?php make_navbar('Groups'); ?>

<div id="body" class="wrap">
  <h3><?php echo $pageTitle; ?></h3>

<?php if(isset($id)) { ?>
  <ul id="options">
  <li id="add" class="button">
  <form method="post" action="editgroup.php">
  <input type="submit" value="Add">
  </form>
  <li id="delete" class="button">
  <form method="post" action="<?php echo "deletegroup.php?id=$id";?>">
  <input type="submit" value="Delete">
  </form>
  </ul>
<?php } ?>

  <form name="editGroupForm" method="post" action="<?php echo $submitPage; ?>">
    <fieldset>
    <legend>Required Information</legend>
    <ul>
      <li>
        <label class="required">Name</label>
        <input name="name" value="<?php echo $form['name']; ?>" type="text" class="text">
      </li>
      <li>
        <label class="required">Status</label>
        <select name="status" title="Group status">
          <option <?php if ($form['status'] == "Active") {echo("selected");} ?> value="Active">Active</option>
          <option <?php if ($form['status'] == "Disabled") {echo("selected");} ?> value="Disabled">Disabled</option>
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

<?php if(isset($id)) { ?>

  <div id="users_table" class="section">
    <div class="table_header">
      <h3><span>Members of <?php echo $form['name']; ?></span></h3>
    </div>
      <?php if(isset($id)) makeGroupMemberTable($id, "updateacct.php")?>
  </div>

  <div id="organizations_table" class="section">
  <div class="table_header">
      <h3><span>Membership info</span></h3>
  </div>
      <?php if(isset($id)) makeGroupOwnerTable($id, "updateacct.php"); ?>
  </div>

<?php } else echo '<div class="section"></div>'; ?>

</div>
<?php include('footer.php') ?>
</body>
</html>


