<?php

header('Content-Type: text/xml'); 

$putload = new DOMDocument("1.0", "iso-8859-1");
$putload->formatOutput = false;

$myDate = date(DATE_ATOM);
$cpuCnt = exec('cat /proc/cpuinfo|grep processor|wc -l');
$loads = exec('uptime|sed -ne \'s/\(.*\)average:[[:space:]]\(.*\)/\2/p\'');
$splitLoads = explode( ", ", $loads );
$myName = exec('hostname -f');
$netBytes = exec('cat /proc/net/dev|grep eth0|sed -ne \'s/[[:space:]]*eth0:\([0-9]*\)[[:space:]].*/\1/p\'');

exec('ps aux|grep squeak|grep -v grep|awk \'{print $2}\'', $squeakpids);

$host = $putload->createElement( "host" );

$hostName = $putload->createElement( "hostName", $myName );
$host->appendChild( $hostName);

$systemLoad = $putload->createElement( "sysLoad" );
$sysLoad1 = $putload->createElement( "oneMin", ($splitLoads[0]/$cpuCnt) );
$sysLoad5 = $putload->createElement( "fiveMin", ($splitLoads[1]/$cpuCnt) );
$sysLoad15 = $putload->createElement( "fifteenMin", ($splitLoads[2]/$cpuCnt) );
$systemLoad->appendChild( $sysLoad1 );
$systemLoad->appendChild( $sysLoad5 );
$systemLoad->appendChild( $sysLoad15 );
$host->appendChild( $systemLoad );

$myTime = $putload->createElement( "time", $myDate );
$host->appendChild( $myTime );

$netb = $putload->createElement( "netBytes", $netBytes );
$host->appendChild( $netb );

foreach ( $squeakpids as $pid ) {
  $p = (int)escapeshellcmd($pid);
  $pCmd = `ps -o args $p|tail -n 1`;
  $pMem = `ps -o size $p|tail -n 1`;
  $pCpu = `ps -o pcpu $p|tail -n 1`;
  $pTime = `ps -o time $p|tail -n 1`;
  $procCmd = $putload->createElement( "cmd", $pCmd );
  $procMem = $putload->createElement( "mem", $pMem );
  $procCpu = $putload->createElement( "cpu", $pCpu );
  $procTime = $putload->createElement( "time", $pTime );
  $procPid = $putload->createElement( "pid", $pid );
  $proc = $putload->createElement( "proc" );
  $proc->appendChild( $procPid );
  $proc->appendChild( $procCpu );
  $proc->appendChild( $procTime );
  $proc->appendChild( $procMem );
  $proc->appendChild( $procCmd );
  $host->appendChild ( $proc );
}

$putload->appendChild( $host );

echo $putload->saveXML();
?>
