<?php
/**
 * PHP Template.
 */
    
# Return server value specified within the config.php file.
# If it is configured as localhost, rewrite w/server's name
# or IP if name is not available.

function getStatus() { 
    global $allServers;
    $baseUrl = $allServers[0][2];

    # Time out the request after 3 seconds
    $ctx = stream_context_create(array(
	      'http' => array(
			      'timeout' => 3
			      )
	      )
    );
    $status = @file_get_contents($baseUrl . '/forums/status', 0, $ctx);
    if($status == false) return;
 
    $statusXML = new SimpleXMLElement($status);
    if (preg_match('/(localhost)(:\d+)/', ($statusXML->host), $match)) {
	$statusXML->host = $_SERVER['SERVER_ADDR'] . $match[2];
    }

    return($statusXML);
}
?>
