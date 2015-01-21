	<script type="text/javascript">
		function _synd_bandwidth_onload() {
			var iStop = new Date().getTime();
			var oImage = document.getElementById('synd_bandwidth_image');
			var iSpeed = (undefined != oImage.fileSize ? oImage.fileSize : 25000) / (iStop - oImage.iStart) * 1000 * 8;
			oImage.src = '<?= tpl_view('system','setEnvironment',array('senv'=>array('bandwidth'=>null))) ?>'+iSpeed;
		}

		if (document.getElementById) {
			var oImage = document.createElement('img');
			oImage.id = 'synd_bandwidth_image';
			oImage.width = '2';
			oImage.height = '1';

			var oBody = document.getElementsByTagName('body').item(0);
			oBody.appendChild(oImage);

			oImage.onload = _synd_bandwidth_onload;
			oImage.iStart = new Date().getTime();
			oImage.src = '<?= tpl_design_uri('core/lib/bandwidth.jpg').'?rand='.rand() ?>';
		}
	</script>
