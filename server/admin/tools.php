<?php
   
include('./charts/chartQueries/performance.php');
include('chartData.php');
$pageTitle="Tools";
$pageInfo="Please prvovide the following information to get network latency graphs"

?>
<?php include('header.php'); ?>
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
<script type="text/javascript" src="js/chart.js"></script>
<?php make_navbar('Tools'); ?>

	<div id="body" class="wrap">
		<h3><?php echo $pageTitle?></h3>
   
		<form name="calendar" onsubmit="return false">
                  <fieldset>
                    <legend>Instructions</legend>
                          To see a chart of meetings: select date, optionally org and hit update
                       <br><br>
                          To see a chart comparing user network latencies within a meeting: 
                select date, org, forum, and time and hit update. User and timezone are optional
                       <br><br>
                   </fieldset>
			<fieldset>
				<legend>Session Information</legend>
				<ul>
					<li>
						<label class="required">Date  &nbsp;&nbsp;<a id="ancorCal" name="ancorCal" title="Select Date" onclick="cal.select(document.forms['calendar'].calDate,'ancorCal','MM/dd/yyyy'); document.forms['calendar'].calDate.focus(); return false;" href="#">select</a></label>
						<input type="text" class="calendar" id="selectionDate" name="calDate">
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
						<label class="required">Time Zone Offset *</label>
						<input type="text" class=calendar name=utcOffset id=utcOffset value=<?php   echo file_get_contents("./chartTzOffset", true) ?> \>
						<p class="note">* +-(hours):(minutes)<br>
						eg. -07:00</p>
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
		<div class="button submit">
			<input type=button id=updateChart onclick='updateChart();' title='Update Chart' value='Update Chart' />
                        
                </div>
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
			'FlashVars', 'library_path=./charts/charts_library&xml_source=createChart.php', 
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
		</div><!-- End Chart -->
	</div><!-- End wrap -->
	<div id="site_info">
		<div class="wrap">
			<p>&copy;2006-2011 Teleplace, inc. all rights reserved</p>
		</div>
	</div>
</body>
</html>
