<h1><?= tpl_text('Mark invoices as paid') ?></h1>

<form method="post">
	<div class="RequiredField">
		<?= tpl_form_textarea('invoices',$request['invoices']) ?>
		<div class="Info"><?= tpl_text("Supply payment export data containing invoice numbers and amounts paid. The format is either PlusGirot export data or CSV like <em>'&lt;invoice number&gt;,&lt;amount&gt;,&lt;date&gt;</em>'. Date defaults to today if not otherwise defined. For example:") ?></div>
		<code style="margin-left:1em; display:block;">
			80081451413,1000<br />
			19701011413,800,2005-01-01
		</code>
	</div>

<? if (!empty($invoices) || !empty($errors)) { ?>
	<? include tpl_design_path('gui/errors.tpl'); ?>

	<? if (!empty($invoices)) { ?>
		<? $this->display('model/node/invoice/table.tpl',array('list'=>$invoices,'hideCheckbox'=>true)) ?>
	<? } ?>

	<br />
	<? if (empty($errors)) { ?>
		<? if (!empty($invoices)) { ?>
		<div class="OptionalField">
			<h3><?= tpl_text('Comment to append to invoices') ?></h3>
			<?= tpl_form_textarea('comment',isset($request['comment'])?
				$request['comment']:tpl_text('Marked as paid'),array('cols'=>50)) ?>
		</div>
		<? } ?>
		<input type="submit" value="<?= tpl_text('Search again &gt;&gt;&gt;') ?>" />
		<input type="submit" name="confirm" value="<?= tpl_text('Mark as paid') ?>" />
	<? } else { ?>
		<input type="submit" value="<?= tpl_text('Search again &gt;&gt;&gt;') ?>" />
	<? } ?>
<? } else { ?>
	<input type="submit" value="<?= tpl_text('Find invoices &gt;&gt;&gt;') ?>" />
<? } ?>
</form>
