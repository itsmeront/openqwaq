<?php
include('serverapi.php'); 
$server = $_GET['server'];
$index = strpos($server, ":");
if($index > 0) $server = substr($server, 0, $index);
$logs = getServerLogFiles($server);
?>

<?php include('header.php'); ?>
<body id="server_info">
<?php make_navbar('Servers'); ?>

<div id="body" class="wrap">
  <div class="section">
    <div class="table_header">
  <h3>Server Logs: <?php echo $server; ?></h3>
  </div>
<?php
  echo '<table cellpadding="0" cellspacing="0" class="alternate"><thead><tr>';
  foreach($logs->header->label as $label) {
    echo '<th>'.(string)$label.'</th>';
  }
  echo "</tr></thead></tbody>";
  foreach($logs->row as $row) {
    echo "<tr>";
    foreach($row->file as $file) {
      echo "<td><a href=\"showlog.php?server=$server&file=$file\">$file</a></td>";
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
