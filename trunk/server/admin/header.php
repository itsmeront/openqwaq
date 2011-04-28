<?php

function make_navbar($activeTab) {
  $selected_tab = $activeTab;
  include('navbar.php');
}

?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
<title>OpenQwaq Admin Interface</title>

<!-- CSS -->
<link href="/admin/css/screen.css" rel="stylesheet" type="text/css" media="screen" />
<link href="/admin/css/thickbox.css" rel="stylesheet" type="text/css" media="screen" />

<!-- Prototype -->
<script type="text/javascript" src="/admin/prototype-1.5.1.2.js" ></script>

<!-- jQuery -->
<script type="text/javascript" src="/admin/js/jquery-1.3.2.min.js"></script>
<script type="text/javascript" src="/admin/js/jquery.thickbox.min.js"></script>
<script type="text/javascript" src="/admin/js/jquery.listify.js"></script>
<script type="text/javascript" src="/admin/js/jquery.tooltip.js"></script>
<script type="text/javascript" src="/admin/js/jquery.color.js"></script>
<script type="text/javascript" src="/admin/js/jquery.textarearesizer.js"></script>

<!-- General JS -->
<script type="text/javascript" src="/admin/js/scripts.js"></script>
<script type="text/javascript" src="/admin/js/admin.js"></script>

<!--[if lt IE 8]>
<link href="/admin/css/ie.css" rel="stylesheet" type="text/css" />
<![endif]-->
</head>
