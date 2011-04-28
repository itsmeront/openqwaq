<?php
include("serverapi.php");
$data = file_get_contents("OpenQwaq-license");
$base64 = chunk_split(base64_encode($data));
?>
<?php include('header.php'); ?>
<body id="edit_server" class="servers">
	<?php make_navbar('Servers'); ?>
	<div id="body" class="wrap">
		<div class="section">
			<table cellpadding="0" cellspacing="0" class="alternate">
				<thead>
					<tr>
						<th colspan="2">Your Current Server License</th>
					</tr>
				</thead>
				<tbody>
					<?php printFeatureInfo() ?>
				</tbody>
			</table>
		</div>
		<div class="section">
			<form method="POST" action="updatelicense.php">
				<table cellpadding="0" cellspacing="0">
					<thead>
						<tr>
							<th>Update License Block</th>
						</tr>
					</thead>
					<tbody>
						<tr>
							<td><textarea name="license" rows="10" style="width:100%; border: none"><?php echo "$base64"; ?></textarea>
					</tbody>
				</table>
				<div class="button submit">
					<input type="submit" name="action" value="Update License">
				</div>
			</form>
		</div>
	</div>
	<?php include('footer.php') ?>
</body>
</html>
