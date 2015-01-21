<? $num = ord($text[0])%5; ?>
<table width="100%" style="margin:0; margin-bottom:5px;">
<tr>
	<? for ($i=0; $i<$num || $i<2; $i++) { ?>
	<td class="page_section"><img src="/image/spacer.gif" width="<?= ($i*$num)%3?10-$i*$num-1:1 ?>" /></td>
	<td><img src="/image/spacer.gif" width="1" /></td>
	<? } ?>
	<td width="99%" class="page_section" style="border-top:0; border-right:0;"><?= tpl_text($text) ?> ::</td>
</tr>
</table>
