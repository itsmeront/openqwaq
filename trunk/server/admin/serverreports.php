<?php
include('serverapi.php'); 

$path = $_SERVER['DOCUMENT_ROOT'].'/admin/reports/';
$files = array();
$dir = opendir($path);
while(($file = readdir($dir)) !== false) {
  if(filetype($path . $file) == "file") {
    $key = substr($file, strpos($file, "-")+1);
    $key = substr($key, 0, strpos($key, "."));
    $files[$key][] = $file;
  }
}
?>

<?php include('header.php'); ?>
<body id="server_info">
<?php make_navbar('Servers'); ?>

<div id="body" class="wrap">
  <div class="section">
    <div class="table_header">
  <h3>Server Reports</h3>
  </div>
  <table cellpadding="0" cellspacing="0" class="alternate"><tbody><tr>
    <th>HTML Report</th><th>Total</th><th>Organizations</th><th>Users</th><th>Forums</th>
  </tr>

<?php
  foreach($files as $key => $fileSet) {
    $htmlFile = "report-$key.html";
    $totalFile = "total-$key.csv";
    $orgsFile = "orgs-$key.csv";
    $usersFile = "users-$key.csv";
    $forumsFile = "forums-$key.csv";
    echo "<tr>";
    echo "<td><a href=\"reports/$htmlFile\">$htmlFile</a></td>";
    echo "<td><a href=\"reports/$totalFile\">$totalFile</a></td>";
    echo "<td><a href=\"reports/$orgsFile\">$orgsFile</a></td>";
    echo "<td><a href=\"reports/$usersFile\">$usersFile</a></td>";
    echo "<td><a href=\"reports/$forumsFile\">$forumsFile</a></td>";
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
