<?php
include('serverapi.php'); 
$server = $_GET['server'];
$file = $_GET['file'];
$logfile = getServerLogNamed($server, $file);
?>

<?php include('header.php'); ?>
<body id="server_info">
<?php make_navbar('Servers'); ?>

<div id="body" class="wrap">
  <div class="section">
    <div class="table_header">
  <h3>Log File: <?php echo $server."/". $file; ?></h3>
  </div>
<?php
  echo '<table cellpadding="0" cellspacing="0" class="log_table alternate" width="600"><thead><tr>';
  echo '<th>Time</th><th>Type</th><th>Source</th><th>Description</th>';
  echo "</tr></thead></tbody>";
  foreach($logfile->children() as $entry) {
    $date = substr((string)$entry->Date, 11, 8);
    echo "\n<tr>";
    if($entry->getName() == 'Info' || $entry->getName() == 'Warning') {
      echo "<td>".$date."</td><td>".$entry->getName()."</td><td>".
	$entry->Source."</td><td>".
	htmlspecialchars($entry->Description, ENT_QUOTES)."</td>";
    }
    if($entry->getName() == 'Error') {
      echo "<td>".$date."</td><td>".$entry->getName()."</td><td>".
	$entry->Source."</td><td>".$entry->Description."<br><pre>".
	htmlspecialchars($entry->Details, ENT_QUOTES)."</pre></td>";
    }
    if($entry->getName() == 'Activity') {
      echo "<b>";
      echo "<td>".$date."</td><td>".$entry->getName()."</td><td>".
	$entry->Action."</td><td>User: ".$entry->User.
	"<br>Extra: ".htmlspecialchars($entry->Extra, ENT_QUOTES)."</td>";
      echo "</b>";
    }
    echo "</tr>";
  }
  echo "</tbody></table>";
?>
    </div>
  </div>
</div>
<?php include('footer.php') ?>
</body>
</html>
