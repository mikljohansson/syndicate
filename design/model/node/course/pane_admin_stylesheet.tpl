<script type="text/javascript">
<!--

function _callback_onkeypress(oEvent) {
	if (9 == oEvent.keyCode) {
		if (oEvent.stopPropagation) {
			oEvent.stopPropagation();
			oEvent.preventDefault();
		}
		else {
			oEvent.cancelBubble = true;
			return false;
		}
	}

	if ((115 == oEvent.keyCode || 83 == oEvent.keyCode) && oEvent.ctrlKey) {
		if (oEvent.stopPropagation) {
			oEvent.stopPropagation();
			oEvent.preventDefault();
			document.getElementById('stylesheet').submit();
		}
		else {
			oEvent.cancelBubble = true;
			document.getElementById('stylesheet').submit();
			return false;
		}
	}
}

//-->
</script>
<form action="<?= tpl_link_call($node->getHandler(),'edit',$node->nodeId) ?>" id="stylesheet" method="post">
	<div class="RequiredField">
		<?= tpl_form_textarea('data[INFO_STYLESHEET]',$node->data['INFO_STYLESHEET'],array('cols'=>75,
			'onkeydown'=>'return _callback_onkeypress(event);',
			'onkeypress'=>'return _callback_onkeypress(event);')) ?>
	</div>
	<div class="OptionalField">
		<?= tpl_form_checkbox('data[FLAG_STYLESHEET]',$node->data['FLAG_STYLESHEET']) ?>
			<label for="data[FLAG_STYLESHEET]"><?= tpl_text('Apply stylesheet for this course') ?></label>
	</div>
	
	<input type="submit" name="post" value="<?= tpl_text('Save') ?>" />

	<div class="Help Info" style="margin-top:1em;">
		<?= tpl_translate('The regular styles are contained in <a href="%s">course.css</a> and <a href="%s">screen.css</a>; the style specified here is loaded in addition to these stylesheets. If necessary the <b>\'!important\'</b> directive can be used to forcibly override these predefined styles. For example:', 
			tpl_design_uri('module/course/course.css'), tpl_design_uri('layout/screen.css')) ?>
		<br /><br />
		<code>
			html { <br />
			&nbsp;&nbsp;background: #ddd !important;<br />
			}
		</code>
	</div>
</form>
