<?php

include('propsapi.php');

$server = $_GET['server'];

$sections = array(
   "Email Support" => array(
       "OpenQwaq.SP.SupportEmail" => "QFES Support Email",
       "OpenQwaq.BugMail" => "QFES Bug Reports"
       ),

   "Smtp Settings" => array(
       "OpenQwaq.Smtp.Host" => "SMTP Host",
       "OpenQwaq.Smtp.Port" => "SMTP Port",
       "OpenQwaq.Smtp.User" => "SMTP Login",
       "OpenQwaq.Smtp.Password" => "SMTP Password"
       ),


   "Email Templates" => array(
       "OpenQwaq.Coal.Welcome.AdminTemplate" => "Welcome (admin)",
       "OpenQwaq.Coal.Welcome.UserTemplate" => "Welcome (user)",
       "OpenQwaq.Coal.Invitation.Template" => "Invitation (new user)",
       "OpenQwaq.Coal.Invitation.NotifyTemplate" => "Invitation (existing user)",
       "OpenQwaq.Coal.AccountReminder.AdminTemplate" => "Account reminder (admin)",
       "OpenQwaq.Coal.AccountReminder.UserTemplate" => "Account reminder (user)",
       "OpenQwaq.Coal.ResetPassword.Template" => "Reset Password",
       // Leave those out for QFES
       // "OpenQwaq.Coal.SeatLimitNotice.LimitCloseTemplate"=>"Seat Limit Near",
       // "OpenQwaq.Coal.SeatLimitNotice.OverLimitTemplate"=>"Seat Limit Exceeded",
       ),

   "Active Directory" => array(
       "OpenQwaq.AD.Enabled" => "Delegated Authentication",
       "OpenQwaq.AD.ServerName" => "LDAP Server",
       "OpenQwaq.AD.BaseDN" => "Base DN",
       ),

   "External Authentication" => array(
       "OpenQwaq.SP.AuthScript" => "Authentication Script",
       "OpenQwaq.SP.AuthArgs" => "Authentication Args",
       ),

   "Webcast Support" => array(
       "OpenQwaq.RTSP.ExternalHostName" => "Webcast IP"
       ),

   "Client Options" => array(
       // Leave this option off until the client will actually
       // accept a url from the server - right now it is hard-coded
       //"OpenQwaq.SP.ClientDownload" => "Download Location",

       "OpenQwaq.SP.MinimumClientVersion" => "Min. Client Version",
       "OpenQwaq.SP.MaximumClientVersion" => "Max. Client Version",
       ),

   "Application Server" => array(
       "OpenQwaq.Apps.MaxApps" => "Max. Applications",
       "OpenQwaq.Apps.HomeDirectoryPath" => "Home Path",
       "OpenQwaq.Apps.TempDirectoryPath" => "Temp Path",
       ),

   "VNC Remote Access" => array(
       "OpenQwaq.VNC.Active" => "Enable VNC Access",
       "OpenQwaq.VNC.Password" => "VNC Password"
       ),

   "Forum Pages" => array(
       "OpenQwaq.SP.Web.ExternalHostName" => "HTTP Host Name",
       "OpenQwaq.SP.Web.HTTPSHostName" => "Secure (HTTPS) Host Name"
       ),

   "Database Settings" => array(
       "OpenQwaq.SP.ODBC.DSN" => "ODBC Data Source Name",
       "OpenQwaq.SP.ODBC.User" => "ODBC Login",
       "OpenQwaq.SP.ODBC.Password" => "ODBC Password",
       )
   );

?>
<?php include('header.php'); ?>
<body id="edit_server" class="servers">
<?php make_navbar('Servers'); ?>

<div id="body" class="wrap">

<div class="section">

<?php
echo "<form method=\"POST\" action=\"serverupdate.php?server=$server\">";
foreach($sections as $label => $props) {
  makePropSection($server, $label, $props);
}
echo '<input type="submit" value="Update">';
echo "</form>";

echo "<p><a href=\"serverconfraw.php?server=$server\">(edit server.conf directly)</a>";
?>
</div>
</div>
<?php include('footer.php') ?>
</body>
</html>
