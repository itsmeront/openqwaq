<?php $category="support"; ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
<meta name="description" content=" The following links and resources will help you learn more about Teleplace solutions and ongoing development in the world of virtual collaboration for the enterprise.">
<title>Teleplace Client Download</title>
<?php include('includes/head.php'); ?>
</head>
<body id="support" class="content_page">
<div id="wrapper">
 
  <div id="branding_nav">
   <?php include "includes/branding_nav.php"; ?>
  </div>
  <!-- #branding_nav -->
  <div id="content_secondary">
    <?php include "includes/content_secondary.php"; ?>
  </div>
  <!-- #sub_nav -->

  <div id="content_main">

<table border=0>
<tr>
<td>

<?php if (file_exists('custom-download.php')) {
        include ('custom-download.php');
} else { ?>

   <h2>Teleplace Client Download</h2>
	<ul>
	<li><a href="TeleplaceInstall-3.5.17.msi">Windows (XP or Vista)</a>
	<li><a href="TeleplaceInstall-3.5.17.dmg">Mac OS X (Intel cpu only)</a>
	</ul>
	
	<p>Please see the Welcome email you received for more information.

<?php } ?>

</td>
</tr>
</table>
  </div>

  <!-- #content_main -->

  <div id="footer">
   <?php include "includes/footer.php"; ?>
  </div>
  <!-- #footer -->
</div>
</body>
</html>
