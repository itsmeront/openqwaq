<?php
/* Configurable settings for the admin console */

$odbcData="OpenQwaqData";
$odbcUser="openqwaq";
$odbcPass="openqwaq";

$odbcLogData="OpenQwaqActivityLog";
$odbcLogUser="openqwaq";
$odbcLogPass="openqwaq";

/* Location of the license file */
$LicFile="/home/openqwaq/server/admin/OpenQwaq-license";

/* Available servers for the orgs */
$allServers = array(
    array("", "Default", "http://localhost:9991"),
);

/* Available tiers of service for the admin console */
$allTiers = array(
   array("", "-- None --"),
);

?>
