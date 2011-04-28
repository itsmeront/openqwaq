<?php
/**
 * PHP Template.
 */
function updateSF($tableName, $rowID, $sfID){
  $odbc = odbcConnect();
  $stmt = odbc_prepare($odbc, "INSERT into SalesForceUpdateQueue (creationDate, mysqlTableName, mysqlRowID, salesForceID) VALUES(CURRENT_TIMESTAMP(), ?, ?, ?)");
  $rs = odbc_execute($stmt, array($tableName, $rowID, $sfID));
  odbc_close($odbc);
}

?>
