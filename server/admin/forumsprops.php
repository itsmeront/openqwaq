<?php

include('propsapi.php');

if(isset($_GET['id'])) {
  $orgid = $_GET['id'];
  $orgForm = findOrgById($orgid);
  $orgLabel = $orgForm['name'];
 } else {
  $orgid = '';
  $orgLabel = 'All Organizations';
 }

$sections = array(
   "Feature Enablements" => array(
      "EnabledFeatures.AnyUserCanInvite" => "Any User can Invite",
      "EnabledFeatures.InviteByRole" => "Invitation by role",
      "EnabledFeatures.WebCam" => "WebCam Input",
      "EnabledFeatures.PersonalShare" => "Personal Share",
      "EnabledFeatures.ForumPages" => "Forum Pages",
      "EnabledFeatures.OfflineUploads" => "Lobby Uploads",
      "EnabledFeatures.PublicChat" => "Public Text Chat",
      "EnabledFeatures.PrivateChat" => "Private Text Chat",
      // XXX: Not a shipping feature
      // "EnabledFeatures.DesktopEdition" => "Enable Miramar",
      "EnabledFeatures.SessionRecording" => "Session DVR",
      "EnabledFeatures.Webcast" => "Webcast",
      "EnabledFeatures.SecureVideo" => "Secure Video"
      ),
   "Forum Limits" => array(
      "Limits.MaxForumsPerOrg" => "Max. Number of Forums in Org",
      "Limits.MaxUsersPerForum" => "Max. Number of Users in Forum",
      "Limits.MaxAppsPerForum" => "Max. Number of Apps in Forum"
      ),
   "Jitter Buffer Defaults" => array(
      "SoundQuality.MinJitterBuffer" => "Min Jitter Buffer",
      "SoundQuality.MaxJitterBuffer" => "Max Jitter Buffer"
      ),
   "Streaming Video Limits" => array(
      "Limits.StreamingVideoBitrate" => "Max. Bit Rate",
      "Limits.StreamingVideoWidth" => "Max. Width of Video",
      "Limits.StreamingVideoHeight" => "Max. Height of Video"
      )
   );


function makeForumsProperties($id) {
  global $sections;
  if($id<>'') {
    echo "<form method=\"POST\" action=\"updateprops.php?id=$id\" class=\"form\">";
  } else {
    echo "<form method=\"POST\" action=\"updateprops.php\" class=\"form\">";
  }
  foreach($sections as $label => $props) {
    echo "<fieldset><legend>$label</legend><ul>";
    $originals = getForumsProperties('', array_keys($props));
    $values = getForumsProperties($id, array_keys($props));
    foreach($props as $key => $name) {
      $value = $values[$key];
      if($value == $originals[$key]) {
	echo "<li><label>$name</label>";
      } else {
	echo "<li><label><b>$name</b></label>";
      }
      if($value == 'true' or $value == 'false') {
	$on = $off = '';
	if($value == 'true') $on = 'selected';
	else $off = 'selected';
	echo "<select name=\"$key\">";
	echo "<option value=\"true\" $on>Enabled</option>";
	echo "<option value=\"false\" $off>Disabled</option>";
	echo "</select>";
      } else {
	echo "<input type=\"text\" name=\"$key\" value=\"$value\" size=\"4\">";
      }
    }
    echo "</ul></fieldset>\n";
  }

  echo '<div class="button submit">';
  echo '<input name="action" value="Update Properties" type="submit">';
  echo '</div>';
  echo '<div class="button submit">';
  echo '<input name="action" value="Reset To Defaults" type="submit">';
  echo '</div>';
  echo "</form>";
}


?>
