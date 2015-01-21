<script type="text/javascript">
<!--
	function attempt_check_progress(oForm) {
		var bBreak = false;
		var oList = new Array(
			oForm.getElementsByTagName('input'),
			oForm.getElementsByTagName('select'));
		for (var i=0; i<oList.length && !bBreak; i++) {
			for (var j=0; j<oList[i].length; j++) {
				if (-1 != oList[i][j].name.indexOf('[answer]') && '' == oList[i][j].value) {
					if (true == confirm('<?= tpl_text('You should answer all the questions, skipped questions will count as failed! \nPress Ok to do so, press Cancel to continue anyway.') ?>'))
						return null;
					else {
						bBreak = true;
						break;
					}
				}
			}
		}
		oForm.submit();
	}
//-->
</script>