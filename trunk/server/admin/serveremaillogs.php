<?php
include("serverapi.php");
$server = $_GET['server'];
$xml = emailLogs();
if($xml->getName() <> 'ok') {
     /* This does not necessarily mean there was a failure.  We could have 
        simply timed out waiting for a response from the server (there might
        be some huge log files needing gathering which takes longer than our
        default timeout).  So, tell the user that the email is on the way
        with a generic message.  
      */
     echo "<p><strong>The support address should soon receive an email with the log files attached.  If the email does not arrive in a few minutes, you can find the file at server/bin/forums/logs.zip</strong></p>";
} else {
    /* For now, the xml document returned is a little complicated only containing
       a message describing what happened.  It looks like:
        <ok><results><info> a message </info></results></ok>
       This message will include the email address that should receive the
       zip file.
     */
    $children = $xml->children();
    $resultsNode = $children[0];
    $results = (string)$resultsNode->info;
    echo "<p><strong>$results</strong></p>";
}
?>
