<?php
/**
 * PHP Template.
 */
  $offset = $_GET['offSet'];
  global $utcOffset, $screenTimeOffset;
  unset($utcOffset);
  unset($screenOffset);
  file_put_contents("chartTzOffset", $offset);
?>
