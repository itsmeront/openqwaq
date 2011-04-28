<?php
include("serverapi.php");
$xml = runServerTest();
if($xml->getName() <> 'ok') {
  $reason = (string)$xml;
  echo "<p><strong>Error running test:</strong> $reason</p>";
 } else{
  foreach($xml->children() as $xmlForm) {
    if($xmlForm->getName() == 'test') {
      $testName = (string)$xmlForm->name;
      $testResult = (string)$xmlForm->result;
      $comment = (string)$xmlForm->comment;
      echo "<p><strong>$testName</strong><br/>";
      echo "<strong>Description:</strong> $comment<br/>";
      echo "<strong>Result:</strong>";
      if($testResult == 'ok') {
	echo "<span class=\"pass\"> PASS</span></p>";
      } else {
	$details = (string)$xmlForm->details;
	echo "<span class=\"fail\"> FAIL</span><br/>";
	echo "<strong>Details:</strong> $details</p>";
      }
    }
  }
 }
?>
