<?php

include("serverapi.php");

// Retrieve forums.properties values from the local host
// Retrieve a server.conf file from the given host
function getForumsProperties($orgid, $options) {
  global $lastError;

  $urlBase = "http://localhost:9991/forums/getForumsProperties";

  $doc = new DOMDocument("1.0", "UTF-8");
  $xml =  $doc->saveXML();

  $top = $doc->createElement("getForumsProperties", "");
  $doc->appendChild($top);
  if($orgid <> '') {
    $node = $doc->createElement("orgid", $orgid);
    $top->appendChild($node);
  }
  $count = count($options);
  for($i=0; $i < $count; $i++) {
    $node = $doc->createElement("property", '');
    $node->setAttribute("key", $options[$i]);
    $top->appendChild($node);
  }
  $xml = $doc->saveXML();

  $response = sendServerRequest($urlBase, $xml);

  $xml = new SimpleXMLElement($response);
  $success = $xml->getName();
  $reason = $response;
  if($success <> "ok") {
      $lastError = $reason;
      echo $reason;
      return;
  }
  $result = array();
  foreach($xml->children() as $form) {
    if($form->getName() == "property") {
      $attrs = $form->attributes();
      $key = (string)$attrs['key'];
      $value = (string) $form;
      $result[$key] = $value;
    }
  }
  return $result;
}

function putForumsProperties($orgid, $options) {
  global $lastError;

  $urlBase = "http://localhost:9991/forums/putForumsProperties";

  $doc = new DOMDocument("1.0", "UTF-8");
  $xml =  $doc->saveXML();

  $top = $doc->createElement("putForumsProperties", "");
  $doc->appendChild($top);
  if($orgid <> '') {
    $node = $doc->createElement("orgid", $orgid);
    $top->appendChild($node);
  }
  foreach($options as $key => $value) {
    $node = $doc->createElement("property", $value);
    $node->setAttribute("key", $key);
    $top->appendChild($node);
  }
  $xml = $doc->saveXML();

  $response = sendServerRequest($urlBase, $xml);

  $xml = new SimpleXMLElement($response);
  $success = $xml->getName();
  $reason = $response;
  if($success <> "ok") {
      $lastError = $reason;
      return False;
  }
  return True;
}

function resetForumsProperties($orgid) {
  global $lastError;

  $urlBase = "http://localhost:9991/forums/resetForumsProperties";

  $doc = new DOMDocument("1.0", "UTF-8");
  $xml =  $doc->saveXML();

  $top = $doc->createElement("resetForumsProperties", "");
  $doc->appendChild($top);
  $node = $doc->createElement("orgid", $orgid);
  $top->appendChild($node);
  $xml = $doc->saveXML();

  $response = sendServerRequest($urlBase, $xml);

  $xml = new SimpleXMLElement($response);
  $success = $xml->getName();
  $reason = $response;
  if($success <> "ok") {
      $lastError = $reason;
      return False;
  }
  return True;
}

function makePropSection($server, $label, $props) {
  echo "<fieldset><legend>$label</legend>";
  echo '<table style="border: 0px none ;">';
  $values = getServerConfigProperties($server, array_keys($props));
  foreach($props as $key => $name) {
    $value = $values[$key];
    echo "<tr>";

    echo "<td width=\"200px\" style=\"background: white ;\">$name</td>";
    echo "<td style=\"background: white ;\">";

    if(stripos($key, "Template") !== False) {
      // This is an email template field
      echo "<input type=\"text\" name=\"$key\" value=\"$value\" size=\"50\" disabled>";
      echo " <a href=\"edittemplate.php?server=$server&key=$key\">Edit Template</a>";
      echo "</td></tr>\n";
      continue;
    }

    if(stripos($key,".password") !== False) {
      // Password field
      echo "<input type=\"password\" name=\"$key\" value=\"$value\" size=\"50\">";
      echo "</td></tr>\n";
      continue;
    }
    if($value == 'true' or $value == 'false') {
      // Checkbox field
      $on = $off = '';
      if($value == 'true') $on = 'selected';
      else $off = 'selected';
      echo "<select name=\"$key\">";
      echo "<option value=\"true\" $on>Enabled</option>";
      echo "<option value=\"false\" $off>Disabled</option>";
      echo "</select>";
      echo "</td></tr>\n";
      continue;
    };
    // Default text input field
    echo "<input type=\"text\" name=\"$key\" value=\"$value\" size=\"50\">";
    echo "</td></tr>\n";
  }
  echo "</table><p>\n";
  echo "</fieldset>";
}

?>
