<?php
   
include('./charts/chartQueries/performance.php');
include('chartData.php');
include('chartTesetData.php');
$pageTitle="Tools";
$pageInfo="Please prvovide the following information to get network latency graphs"
?>
<script language="javascript">AC_FL_RunContent = 0;</script>
<script language="javascript"> DetectFlashVer = 0; </script>
<script src="./charts/AC_RunActiveContent.js" language="javascript"></script>
<script src="js/CalendarPopup.js" language="javascript"></script>
<script language="javascript">
	var cal = new CalendarPopup();
</script>
<script language="JavaScript" type="text/javascript">
<!--
var requiredMajorVersion = 9;
var requiredMinorVersion = 0;
var requiredRevision = 45;
-->
</script>

<html>
<body>
<?php include('header.php'); ?>
<script type="text/javascript" src="js/chart.js"></script>
<?php make_navbar('Tools'); ?>
<div id="body" class="wrap">
  <h3><?php echo $pageTitle?></h3>
<form name="calendar" onSubmit="return false">
    <fieldset>
        <legend>Latency Graphs</legend>
        <ul>  
            <li>
              <label class="required">Date</label>
              <input type="text" class="calendar" id="selectionDate" name="calDate">   
            </li>
           
            <li>
                <label>&nbsp;</label>
                <a id="ancorCal" name="ancorCal" title="Select Date" onClick="cal.select(document.forms['calendar'].calDate,'ancorCal','MM/dd/yyyy'); return false;" href="#">select</a>
            </li>
             <li>
                <label class="required">Organization</label>
                <div id="organization">
                    <select name='org' id='org'>
                      <option>(All)</option>
                    </select>
                </div>
            </li>
            <li>
                <label class="required">Forum</label>
                <div id="Room">
                    <select name='forum' id='forum'>
                      <option>(All)</option>
                    </select>
                </div>
            </li>
            <li>
                <label class="required">Time</label>
                <div id="Time">
                    <select name='time' id='time'>
                      <option>(All)</option>
                    </select>
                </div>
            </li>
            <li>
                <label class="required">Participant</label>
                <div id="Participant">
                    <select name='participant' id='participant'>
                      <option>(All)</option>
                    </select>
                </div>
            </li>
            </ul>
      
    </fieldset>
</form>
<input type=button id=updateChart onclick='updateChart();' title='Update Chart' value='Update Chart' />
<div id="chart">
    <script language="JavaScript" type="text/javascript">
<!--
if (AC_FL_RunContent == 0 || DetectFlashVer == 0) {
	alert("This page requires AC_RunActiveContent.js.");
} else {
	var hasRightVersion = DetectFlashVer(requiredMajorVersion, requiredMinorVersion, requiredRevision);
	if(hasRightVersion) { 
		AC_FL_RunContent(
			'codebase', 'http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=9,0,45,0',
			'width', '780',
			'height', '600',
			'scale', 'noscale',
			'salign', 'TL',
			'bgcolor', '#777788',
			'wmode', 'opaque',
			'movie', 'charts',
			'src', './charts/charts',
			'FlashVars', 'library_path=./charts/charts_library&xml_source=testChartData.php', 
			'id', 'my_chart',
			'name', 'my_chart',
			'menu', 'true',
			'allowFullScreen', 'true',
			'allowScriptAccess','sameDomain',
			'quality', 'high',
			'align', 'middle',
			'pluginspage', 'http://www.macromedia.com/go/getflashplayer',
			'play', 'true',
			'devicefont', 'false'
			); 
	} else { 
		var alternateContent = 'This content requires the Adobe Flash Player. '
		+ '<u><a href=http://www.macromedia.com/go/getflash/>Get Flash</a></u>.';
		document.write(alternateContent); 
	}
}
// -->
</script>
<noscript>
	<p>This content requires JavaScript.</p>
</noscript>
</div>
<br><br>
</div>

<div id="site_info">
  <div class="wrap">
    <p>&copy;2006-2011 Teleplace, inc. all rights reserved</p>
  </div>
</div>
</body>
</html>
