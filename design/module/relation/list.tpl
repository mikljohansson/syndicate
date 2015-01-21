<table>
<tr>
	<td width="40%">
		<? $this->display(tpl_design_path('gui/pane/section.tpl'),array('text'=>'Available Sections')) ?>
		<div class="list">
			<? $this->iterate($lists,'list_view.tpl') ?>
		</div>
	</td>
	<? if (isset($selection) && count($selection)) { ?>
	<td width="1">&nbsp;</td>
	<td width="40%">
		<? $this->display(tpl_design_path('gui/pane/section.tpl'),array('text'=>'Selection')) ?>
		<div class="list">
			<? $this->iterate($selection,'list_view.tpl') ?>
		</div>
	</td>
	<? } ?>
	<td>&nbsp;</td>
</tr>
</table>