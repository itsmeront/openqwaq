<?php
include('adminapi.php');
if(isset($_POST["search"])) {
  $query = $_POST["search"];
} else {
  $query = "";
  if(isset($_GET["searchString"])){
    $query = rawurldecode($_GET["searchString"]);
  }
}
if(isset($_GET["limit"])) {
  $limit = $_GET["limit"];
} else {
  $limit = 10;
}
if (isset($_GET["page"])) {
    $page = $_GET["page"];
} else {
    $page = 1;
}
?>

<?php include('header.php'); ?>
<body id="find_organization" class="organizations">
<?php make_navbar('Organizations'); ?>

<div id="body" class="wrap">
  <div id="organizations_table" class="section">
    <div class="table_header">
      <h3><span>OpenQwaq Organizations</span> <span class="add">
      (<a href="editorg.php">Add</a>)
      </span></h3>
      <form method="post" action="findorg.php?limit=<?php echo $limit?>">
        <input name="search" value="<?php echo $query ?>" type="text" class="text">
        <input value="Search" type="submit" class="search">
      </form>
    </div>
    <div class="pagination">
    <?php 
      $matches = findOrgsByMatch($query, $limit, $page);
      $count = $matches["count"];
      $total = $matches["total"];
      $nPages = ceil($total/$limit);
    ?>
      <p class="showing">Showing <?php echo(min ($page*$limit, $total));?> of <?php echo $total; ?></p>
      <select name="" onChange="location = this.options[this.selectedIndex].value">
        <option <?php if($limit == 10){echo('selected');}?> value="findorg.php?limit=10&searchString=<?php echo rawurlencode($query);?>">10 items per page</option>
        <option <?php if($limit == 20){echo('selected');}?> value="findorg.php?limit=20&searchString=<?php echo rawurlencode($query);?>">20 items per page</option>
        <option <?php if($limit == 30){echo('selected');}?> value="findorg.php?limit=30&searchString=<?php echo rawurlencode($query);?>">30 items per page</option>
        <option <?php if($limit == 50){echo('selected');}?> value="findorg.php?limit=50&searchString=<?php echo rawurlencode($query);?>">50 items per page</option>
        <option <?php if($limit == 100){echo('selected');}?> value="findorg.php?limit=100&searchString=<?php echo rawurlencode($query);?>">100 items per page</option>
      </select>
      <a class="first" href="findorg.php?limit=<?php echo $limit;?>&page=1&searchString=<?php echo rawurlencode($query);?>">&lt;&lt;</a> 
      <a class="prev" href="findorg.php?limit=<?php echo $limit;?>&page=<?php echo max(($page - 1),1); ?>&searchString=<?php echo rawurlencode($query);?>">Prev</a> 
      Page:
      <input type="text" class="text" name="page" value="<?php echo $page ?>" onChange="location='findorg.php?limit=<?php echo $limit; ?>&searchString=<?php echo rawurlencode($query);?>&page='+this.value;"/>
      of <?php echo $nPages;?>   
      <a class="next" href="findorg.php?limit=<?php echo $limit;?>&page=<?php echo min(($page + 1),$nPages); ?>&searchString=<?php echo rawurlencode($query);?>">Next</a> 
      <a class="last" href="findorg.php?limit=<?php echo $limit;?>&page=<?php echo $nPages;?>&searchString=<?php echo rawurlencode($query);?>">&gt;&gt;</a> </div>
    <?php makeOrgsTable($query, $limit, false, $page); ?>
    <div class="pagination">
         <p class="showing">Showing <?php echo(min ($page*$limit, $total));?> of <?php echo $total;?></p>
     <select name="" onChange="location = this.options[this.selectedIndex].value">
        <option <?php if($limit == 10){echo('selected');}?> value="findorg.php?limit=10&searchString=<?php echo rawurlencode($query);?>">10 items per page</option>
        <option <?php if($limit == 20){echo('selected');}?> value="findorg.php?limit=20&searchString=<?php echo rawurlencode($query);?>">20 items per page</option>
        <option <?php if($limit == 30){echo('selected');}?> value="findorg.php?limit=30&searchString=<?php echo rawurlencode($query);?>">30 items per page</option>
        <option <?php if($limit == 50){echo('selected');}?> value="findorg.php?limit=50&searchString=<?php echo rawurlencode($query);?>">50 items per page</option>
        <option <?php if($limit == 100){echo('selected');}?> value="findorg.php?limit=100&searchString=<?php echo rawurlencode($query);?>">100 items per page</option>
      </select>
      <a class="first" href="findorg.php?limit=<?php echo $limit;?>&page=1&searchString=<?php echo rawurlencode($query);?>">&lt;&lt;</a> 
      <a class="prev" href="findorg.php?limit=<?php echo $limit;?>&page=<?php echo max(($page - 1),1); ?>&searchString=<?php echo rawurlencode($query);?>">Prev</a> 
      Page:
      <input type="text" class="text" name="page" value="<?php echo $page ?>" onChange="location='findorg.php?limit=<?php echo $limit; ?>&searchString=<?php echo rawurlencode($query);?>&page='+this.value;"/>
      of <?php echo $nPages;?>   
      <a class="next" href="findorg.php?limit=<?php echo $limit;?>&page=<?php echo min(($page + 1),$nPages); ?>&searchString=<?php echo rawurlencode($query);?>">Next</a> 
      <a class="last" href="findorg.php?limit=<?php echo $limit;?>&page=<?php echo $nPages;?>&searchString=<?php echo rawurlencode($query);?>">&gt;&gt;</a> </div>
  </div>
</div>
<?php include('footer.php') ?>
</body>
</html>
